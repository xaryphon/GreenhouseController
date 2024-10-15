#pragma once

#include "sensor/Atmosphere.h"
#include "Motor.h"
#include "sensor/Co2Probe.h"
#include "eeprom.h"
#include "pico/types.h"

class SettingsDispatcher;
class Controller {
public:
    Controller(Eeprom *, uint co2_dissipator_pin, Co2Probe *, Motor *, Atmosphere *);

    void SetTargetPPM(uint16_t target) {
        m_target_ppm = target;
    }

    uint16_t GetTargetPPM() {
        return m_target_ppm;
    }

private:
    static void entry(void*);
    static void gas_entry(void*);
    void run();
    void gas_run();

    void set_fan_speed(uint permille);
    void gas_for_ms(uint ms);

    SettingsDispatcher *m_settings;
    Eeprom *m_eeprom;
    uint m_co2_dissipator_pin;
    Co2Probe *m_co2_probe;
    Motor *m_motor;
    Atmosphere *m_atmo;
    SemaphoreHandle_t m_gas_mutex;
    uint16_t m_gas_for_ms;
    uint16_t m_target_ppm;

    friend class SettingsDispatcher;
};

