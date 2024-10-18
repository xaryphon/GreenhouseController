#ifndef GREENHOUSE_CONTROLLER_SETTINGSUI_H
#define GREENHOUSE_CONTROLLER_SETTINGSUI_H

#include <string>
#include <cstring>

#include "config.h"
#include "FreeRTOS.h"
#include "pico/stdlib.h"

#include "display/ssd1306os.h"

class SettingsUI {
public:
    explicit SettingsUI();
    virtual void input(uint input);
    virtual void update_display(ssd1306os &display);
private:
    int m_selected;
};

#endif //GREENHOUSE_CONTROLLER_SETTINGSUI_H
