#ifndef GREENHOUSE_CONTROLLER_CO2UI_H
#define GREENHOUSE_CONTROLLER_CO2UI_H

#include <string>
#include <cstring>

#include "config.h"
#include "FreeRTOS.h"
#include "pico/stdlib.h"

#include "control/Controller.h"
#include "control/SettingsDispatcher.h"
#include "display/ssd1306os.h"

class CO2UI {
public:
    CO2UI(Controller *controller, SettingsDispatcher *dispatcher);
    virtual void input(uint input);
    virtual void update_display(ssd1306os &display);
    void get_target() { m_target = m_controller->GetTargetPPM(); };
private:
    int m_target;

    Controller *m_controller;
    SettingsDispatcher *m_dispatcher;
};

#endif //GREENHOUSE_CONTROLLER_CO2UI_H
