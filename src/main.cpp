#include <iostream>
#include <sstream>
#include <pico/cyw43_arch.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include "PicoOsUart.h"
#include "ssd1306.h"

#include "config.h"
#include "button.h"
#include "network.h"
#include "ui.h"
#include "RotaryDecoder.h"

#include "hardware/timer.h"
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
TLS_CLIENT_T* tls_client_init(void);
bool network_connect(const char *ssid, const char *pwd);
int tls_request(TLS_CLIENT_T_ *client, const char *request, struct altcp_tls_config *conf);
}

#include "blinker.h"

SemaphoreHandle_t gpio_sem;

void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // signal task that a button was pressed
    xSemaphoreGiveFromISR(gpio_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

struct led_params{
    uint pin;
    uint delay;
};

struct task_params {
    QueueHandle_t queue;
    uint pin;
};

void serial_task(void *param)
{
    PicoOsUart u(0, 0, 1, 115200);
    Blinker blinky(20);
    uint8_t buffer[64];
    std::string line;
    while (true) {
        if(int count = u.read(buffer, 63, 30); count > 0) {
            u.write(buffer, count);
            buffer[count] = '\0';
            line += reinterpret_cast<const char *>(buffer);
            if(line.find_first_of("\n\r") != std::string::npos){
                u.send("\n");
                std::istringstream input(line);
                std::string cmd;
                input >> cmd;
                if(cmd == "delay") {
                    uint32_t i = 0;
                    input >> i;
                    blinky.on(i);
                }
                else if (cmd == "off") {
                    blinky.off();
                }
                line.clear();
            }
        }
    }
}

void modbus_task(void *param);
void display_task(void *param);
void i2c_task(void *param);

void network_task(void *param) {
    auto *tpr = (task_params *) param;

    cyw43_arch_init();
    bool connected = network_connect(WIFI_SSID, WIFI_PASSWORD);
    struct altcp_tls_config *tls_config = NULL;
    tls_config = altcp_tls_create_config_client(NULL, 0);
    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)tls_config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    TLS_CLIENT_T *client = tls_client_init();
    std::stringstream request;
    int test = 0;
    while(true) {
        /*
         * seems reconnecting is done automatically in the background and this is not needed
         * though connecting to another network might be a different case
        */
        while(!connected) {
            connected = network_connect(WIFI_SSID, WIFI_PASSWORD);
        }
        if (connected) {
            request << "POST /update.json"
                    << "?field1=" << ++test  // current co2 level
                    << "&field2=" << 12  // humidity
                    << "&field3=" << 55  // temp
                    << "&field4=" << 100 // fan speed
                    << "&field5=" << 0   // target co2 level
                    << "&api_key=B7C4VLKMNBYD4HLJ&talkback_key=K4J932OFNIJEGD9A HTTP/1.1\r\n"
                    << "Host:api.thingspeak.com\r\nConnection:keep-alive\r\n\r\n";
            printf("\n%s", request.str().c_str());
            printf("sending request...\n");
            int new_target_or_whatever = tls_request(client, request.str().c_str(), tls_config);
            request.str("");
            if (new_target_or_whatever == 45) {
                // need to test connecting to another wlan
                network_connect("another ssid", "another pwd");
            }
        }
        vTaskDelay(5000);
    }
}

static RotaryDecoder *g_rotary_decoder = nullptr;
static void irq_callback(uint pin, uint32_t event_mask) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (pin == ROT_A_PIN) {
        g_rotary_decoder->OnInterrupt(&xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


int main() {
    stdio_init_all();
    printf("\nBoot\n");

    gpio_sem = xSemaphoreCreateBinary();
    gpio_set_irq_callback(irq_callback);
    QueueHandle_t input_queue = xQueueCreate(5, sizeof(uint));
    static RotaryDecoder rotary_decoder(input_queue, ROT_A_PIN, ROT_B_PIN, 4);
    g_rotary_decoder = &rotary_decoder;
    irq_set_enabled(IO_IRQ_BANK0, true);

    static Button btn0("BTN0", BTN0_PIN, &input_queue);
    static Button btn1("BTN1", BTN1_PIN, &input_queue);
    static Button btn2("BTN2", BTN2_PIN, &input_queue);
    static Button btnr("BTNR", ROT_SW_PIN, &input_queue);

    static UI ui("UI", &input_queue);
    xTaskCreate(network_task, "NETWORK_TASK", 6000, (void *) nullptr, tskIDLE_PRIORITY + 1, nullptr);

    //xTaskCreate(serial_task, "UART1", 256, (void *) nullptr, tskIDLE_PRIORITY + 1, nullptr);
    //xTaskCreate(modbus_task, "Modbus", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, nullptr);
    //xTaskCreate(i2c_task, "i2c test", 512, (void *) nullptr, tskIDLE_PRIORITY + 1, nullptr);

    vTaskStartScheduler();

    while(true){};
}

#include <cstdio>
#include "ModbusClient.h"
#include "ModbusRegister.h"

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#endif

#define BAUD_RATE 9600
#define STOP_BITS 2 // for real system (pico simualtor also requires 2 stop bits)

#define USE_MODBUS

void modbus_task(void *param) {

    const uint led_pin = 22;
    const uint button = 9;

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);

    // Initialize chosen serial port
    //stdio_init_all();

    //printf("\nBoot\n");

#ifdef USE_MODBUS
    auto uart{std::make_shared<PicoOsUart>(UART_NR, UART_TX_PIN, UART_RX_PIN, BAUD_RATE, STOP_BITS)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};
    ModbusRegister rh(rtu_client, 241, 256);
    ModbusRegister t(rtu_client, 241, 257);
    ModbusRegister produal(rtu_client, 1, 0);
    produal.write(100);
    vTaskDelay((100));
    produal.write(0);
#endif

    while (true) {
#ifdef USE_MODBUS
        gpio_put(led_pin, !gpio_get(led_pin)); // toggle  led
        printf("RH=%5.1f%%\n", rh.read() / 10.0);
        vTaskDelay(5);
        printf("T =%5.1f%%\n", t.read() / 10.0);
        vTaskDelay(3000);
#endif
    }


}


void i2c_task(void *param) {
    auto i2cbus{std::make_shared<PicoI2C>(0, 100000)};

    const uint led_pin = 21;
    const uint delay = pdMS_TO_TICKS(250);
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    uint8_t buffer[64] = {0};
    i2cbus->write(0x50, buffer, 2);

    auto rv = i2cbus->read(0x50, buffer, 64);
    printf("rv=%u\n", rv);
    for(int i = 0; i < 64; ++i) {
        printf("%c", isprint(buffer[i]) ? buffer[i] : '_');
    }
    printf("\n");

    buffer[0]=0;
    buffer[1]=64;
    rv = i2cbus->transaction(0x50, buffer, 2, buffer, 64);
    printf("rv=%u\n", rv);
    for(int i = 0; i < 64; ++i) {
        printf("%c", isprint(buffer[i]) ? buffer[i] : '_');
    }
    printf("\n");

    while(true) {
        gpio_put(led_pin, 1);
        vTaskDelay(delay);
        gpio_put(led_pin, 0);
        vTaskDelay(delay);
    }


}
