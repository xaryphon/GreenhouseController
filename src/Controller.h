#pragma once

#include "Motor.h"
#include "Co2Probe.h"
#include "eeprom.h"
#include "pico/types.h"

class SettingsDispatcher;
class Controller {
public:
    Controller(Eeprom *, uint co2_dissipator_pin, Co2Probe *, Motor *);

    void SetTargetPPM(uint16_t target) {
        m_target_ppm = target;
    }

private:
    static void entry(void*);
    void run();

    void set_fan_speed(uint permille);
    void gas_for_ms(uint ms);

    SettingsDispatcher *m_settings;
    Eeprom *m_eeprom;
    uint m_co2_dissipator_pin;
    Co2Probe *m_co2_probe;
    Motor *m_motor;
    uint16_t m_target_ppm;

    friend class SettingsDispatcher;
};

