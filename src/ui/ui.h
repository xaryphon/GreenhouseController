//
// Created by Noa Storm on 30/09/2024.
//

#ifndef GREENHOUSE_CONTROLLER_UI_H
#define GREENHOUSE_CONTROLLER_UI_H

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include "sensor/Atmosphere.h"
#include "sensor/Co2Probe.h"
#include "control/Controller.h"
#include "control/Motor.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "config.h"
#include "display/ssd1306os.h"

// custom enter key symbol for keyboard + palettes for colour inversion
const unsigned char enter_key[] = {0x00, 0x10, 0x38, 0x7C, 0x10, 0x10, 0x1E, 0x00};
const unsigned char palette_bw[] = {0x00, 0x01};
const unsigned char palette_wb[] = {0x01, 0x00};

//m_menu["status"] = {"Status 1", "Status 2"};
//m_menu["settings"] = {"Back", "CO2", "Network"};
enum menu {
    STATUS = 0,
    SETTINGS = 1,
    STATUS_1 = 2,
    STATUS_2 = 3,
    BACK = 4,
    CO2 = 5,
    NETWORK = 6
};

class SettingsDispatcher;
class UI{
public:
    UI(std::string name_, QueueHandle_t *queue_, SettingsDispatcher *settings, Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller);
private:
    void run();
    static void runner(void *params);
    void update_display(ssd1306os &display);
    void input(uint input);
    void status(uint input);
    void settings(uint input);
    void co2(uint input);
    void network(uint input);
    const std::string m_name;
    QueueHandle_t *m_queue;
    SettingsDispatcher *m_settings;
    Co2Probe *m_co2_probe;
    Motor *m_motor;
    Atmosphere *m_atmo;
    Controller *m_controller;
    TaskHandle_t m_handle;
    std::map<std::string, std::vector<std::string>> m_menu;
    std::string m_current;
    uint m_selected_y;
    uint m_selected_x;
    uint m_target;
    std::string m_input[2];
    char m_keyboard[6][16];
};

#endif //GREENHOUSE_CONTROLLER_UI_H
