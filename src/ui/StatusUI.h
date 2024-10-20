#ifndef GREENHOUSE_CONTROLLER_STATUSUI_H
#define GREENHOUSE_CONTROLLER_STATUSUI_H

#include <string>
#include <cstring>

#include "config.h"
#include "FreeRTOS.h"
#include "pico/stdlib.h"

#include "sensor/Atmosphere.h"
#include "sensor/Co2Probe.h"
#include "control/Controller.h"
#include "control/Motor.h"
#include "display/ssd1306os.h"
#include "hardware/adc.h"

class StatusUI {
public:
    StatusUI(Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller);
    virtual void input(uint input);
    virtual void update_display(ssd1306os &display);
private:
    int m_selected;

    Co2Probe *m_co2_probe;
    Motor *m_motor;
    Atmosphere *m_atmo;
    Controller *m_controller;
};

#endif //GREENHOUSE_CONTROLLER_STATUSUI_H
