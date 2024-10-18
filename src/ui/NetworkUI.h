#ifndef GREENHOUSE_CONTROLLER_NETWORKUI_H
#define GREENHOUSE_CONTROLLER_NETWORKUI_H

#include <string>
#include <cstring>

#include "config.h"
#include "FreeRTOS.h"
#include "pico/stdlib.h"

#include "control/SettingsDispatcher.h"
#include "display/ssd1306os.h"

// custom enter key symbol for keyboard + palettes for colour inversion
const unsigned char enter_key[] = {0x00, 0x10, 0x38, 0x7C, 0x10, 0x10, 0x1E, 0x00};
const unsigned char palette_bw[] = {0x00, 0x01};
const unsigned char palette_wb[] = {0x01, 0x00};

class NetworkUI {
public:
    explicit NetworkUI(SettingsDispatcher *dispatcher);
    virtual void input(uint input);
    virtual void update_display(ssd1306os &display);
private:
    int m_selected_y;
    int m_selected_x;
    int m_stage;
    std::string m_input[2];
    char m_keyboard[6][16]{};

    SettingsDispatcher *m_dispatcher;
};
#endif //GREENHOUSE_CONTROLLER_NETWORKUI_H
