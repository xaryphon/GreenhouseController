//
// Created by Noa Storm on 30/09/2024.
//

#ifndef GREENHOUSE_CONTROLLER_UI_H
#define GREENHOUSE_CONTROLLER_UI_H

#include <string>
#include <cstring>
#include <vector>
#include <map>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "config.h"
#include "display/ssd1306os.h"

class UI{
public:
    UI(std::string name_, QueueHandle_t *queue_);
private:
    void run();
    static void runner(void *params);
    void update_display(ssd1306os &display);
    void input(uint input);
    void main(uint input);
    void settings(uint input);
    void co2(uint input);
    void network(uint input);
    const std::string m_name;
    QueueHandle_t *m_queue;
    TaskHandle_t m_handle;
    std::map<std::string, std::vector<std::string>> m_menu;
    std::string m_current;
    uint m_selected_y;
    uint m_selected_x;
    uint m_target;
    std::string m_text[2];
    char m_keyboard[6][16];
};

#endif //GREENHOUSE_CONTROLLER_UI_H
