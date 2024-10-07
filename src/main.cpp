#include <iostream>
#include <sstream>
#include <pico/cyw43_arch.h>
#include "Co2Probe.h"
#include "Controller.h"
#include "FreeRTOS.h"
#include "PicoI2C.h"
#include "RotaryDecoder.h"
#include "SettingsDispatcher.h"
#include "eeprom.h"
#include "task.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include "PicoOsUart.h"
#include "ModbusClient.h"

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

struct task_params {
    QueueHandle_t queue;
    uint pin;
};

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

int main()
{
    stdio_init_all();
    printf("\nBoot\n");

    QueueHandle_t input_queue = xQueueCreate(5, sizeof(uint));
    gpio_set_irq_callback(irq_callback);
    static RotaryDecoder rotary_decoder(input_queue, ROT_A_PIN, ROT_B_PIN, 4);
    g_rotary_decoder = &rotary_decoder;
    irq_set_enabled(IO_IRQ_BANK0, true);

    static Button btn0("BTN0", BTN0_PIN, &input_queue);
    static Button btn1("BTN1", BTN1_PIN, &input_queue);
    static Button btn2("BTN2", BTN2_PIN, &input_queue);
    static Button btnr("BTNR", ROT_SW_PIN, &input_queue);
    static UI ui("UI", &input_queue);
    xTaskCreate(network_task, "NETWORK_TASK", 6000, (void *) nullptr, tskIDLE_PRIORITY + 1, nullptr);

    static auto uart = std::make_shared<PicoOsUart>(1, UART1_TX_PIN, UART1_RX_PIN, UART1_BAUD_RATE, UART1_STOP_BITS);
    static auto modbus = std::make_shared<ModbusClient>(uart);
    static Co2Probe co2_probe { modbus };
    static Motor motor { modbus };
    static PicoI2C i2c_0 { 0 };
    static Eeprom eeprom { i2c_0 };
    static Controller controller { &eeprom, CO2_DISSIPATOR_PIN, &co2_probe, &motor };
    static SettingsDispatcher settings { &eeprom, &controller };

    vTaskStartScheduler();

    while(true){};
}

