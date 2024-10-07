#include "Controller.h"
#include "FreeRTOS.h"
#include "SettingsDispatcher.h"
#include "config.h"
#include "hardware/gpio.h"
#include "task.h"
#include <cstdio>

Controller::Controller(Eeprom *eeprom, uint co2_dissipator_pin, Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo)
: m_eeprom(eeprom)
, m_co2_dissipator_pin(co2_dissipator_pin)
, m_co2_probe(co2_probe)
, m_motor(motor)
, m_atmo(atmo)
{
    gpio_init(co2_dissipator_pin);
    gpio_set_dir(co2_dissipator_pin, GPIO_OUT);
    xTaskCreate(entry, "Controller", 512, this, TASK_CONTROLLER_PRIORITY, nullptr);
}

void Controller::entry(void *param)
{
    static_cast<Controller*>(param)->run();
}

void Controller::gas_for_ms(uint ms) {
    if (ms > 2000)
        ms = 2000;
    printf("Gassing for %u ms\n", ms);
    gpio_put(m_co2_dissipator_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_put(m_co2_dissipator_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void Controller::set_fan_speed(uint speed) {
    m_motor->Write(speed);
}

void Controller::run()
{
    m_eeprom->LoadBlocking(m_settings);
    while (true) {
        m_motor->Read();
        m_atmo->Read();
        float target = m_target_ppm;
        float co2 = m_co2_probe->ReadPPM();
        float diff = co2 - target;
        printf("co2 %f, target %f, diff %f\n", co2, target, diff);
        if (co2 > 2000) {
            set_fan_speed(1000);
        } else if (diff > 0) {
            if (diff > target * 0.1f) {
                float new_speed = diff / 200.f;
                if (new_speed > 1.f)
                    new_speed = 1.f;
                set_fan_speed((uint)(new_speed * 1000.f));
            } else {
                set_fan_speed(0);
            }
        } else {
            set_fan_speed(0);
            if (diff < -(target * 0.1f)) {
                gas_for_ms((uint)(-diff * 10.f));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

