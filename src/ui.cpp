//
// Created by Noa Storm on 30/09/2024.
//

#include "ui.h"

UI::UI(std::string name_, QueueHandle_t *queue_)
: m_name(name_)
, m_queue(queue_)
, m_current("status")
, m_selected_y(0)
, m_selected_x(0)
{
    xTaskCreate(UI::runner, m_name.c_str(), 256, (void *) this, tskIDLE_PRIORITY + 1, &m_handle);
}

void UI::run() {
    m_menu["status"] = {"Status 1", "Status 2"};
    m_menu["settings"] = {"Back", "CO2", "Network"};

    auto i2cbus{std::make_shared<PicoI2C>(1, 400000)};
    ssd1306os display(i2cbus);

    int value = 0;
    while (true) {
        if (xQueueReceive(*m_queue, static_cast<void *>(&value), pdMS_TO_TICKS(100)) == pdTRUE) {
            input(value);
        }
        update_display(display);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void UI::runner(void *params) {
    UI *instance = static_cast<UI *>(params);
    instance->run();
}

void UI::input(uint input) {
    if (m_current == "status") main(input);
    else if (m_current == "settings") settings(input);
    else if (m_current == "CO2") co2(input);
    else if (m_current == "Network") network(input);
}

// menu keybind logic
void UI::main(uint input) {
    switch (input) {
        case ROT_SW_PIN:
            m_current = "settings";
            m_selected_y = 0;
            break;
        // do we leave unbound keys listed here for clarity or remove them for compactness
        case ROT_A_PIN ... ROT_B_PIN:
            break;
        case BTN0_PIN:
            m_selected_y == 0 ? m_selected_y = 1 : m_selected_y = 0;
            break;
        case BTN1_PIN:
            break;
        case BTN2_PIN:
            break;
        default:
            break;
    }
}
void UI::settings(uint input){
    switch (input) {
        case ROT_SW_PIN:
            m_menu[m_current][m_selected_y] == "Back" ? m_current = "status": m_current = m_menu[m_current][m_selected_y];
            m_selected_y = 0;
            break;
        case ROT_A_PIN ... ROT_B_PIN:
            m_selected_y = (input == ROT_A_PIN ? m_selected_y + 1 : m_selected_y > 0 ? m_selected_y - 1 : 2) % m_menu["settings"].size();
            break;
        case BTN0_PIN:
            break;
        case BTN1_PIN:
            break;
        case BTN2_PIN:
            break;
        default:
            break;
    }

}
void UI::co2(uint input){
    switch (input) {
        case ROT_SW_PIN:
            break;
        case ROT_A_PIN ... ROT_B_PIN:
            break;
        case BTN0_PIN:
            m_current = "settings";
            break;
        case BTN1_PIN:
            break;
        case BTN2_PIN:
            break;
        default:
            break;
    }
}
void UI::network(uint input){
    switch (input) {
        case ROT_SW_PIN:
            m_current = "settings";
            break;
        case ROT_A_PIN ... ROT_B_PIN:
            break;
        case BTN0_PIN:
            m_current = "settings";
            break;
        case BTN1_PIN:
            break;
        case BTN2_PIN:
            break;
        default:
            break;
    }
}

void UI::update_display(ssd1306os &display) {
    char text[8][16] = {};
    display.fill(0);

    //m_menu["status"] = {"Status 1", "Status 2"};
    //m_menu["settings"] = {"Back", "CO2", "Network"};

    if (m_current == "status") {
        sprintf(text[0], "%s", m_menu[m_current][m_selected_y].c_str());
        if (m_menu[m_current][m_selected_y] == "Status 1") {
            sprintf(text[2], "some sensor %d", 69);
        }
        else if (m_menu[m_current][m_selected_y] == "Status 2") {
            sprintf(text[2], "another %d", 420);
        }
    }
    else if (m_current == "settings") {
        sprintf(text[0], "%s", m_current.c_str());
        sprintf(text[1], "%d", m_selected_y);
        sprintf(text[3], "%s", m_menu[m_current][0].c_str());
        sprintf(text[4], "%s", m_menu[m_current][1].c_str());
        sprintf(text[5], "%s", m_menu[m_current][2].c_str());
    }
    else if (m_current == "CO2") {
        sprintf(text[0], "%s", m_current.c_str());
        sprintf(text[1], "%d", m_selected_y);
    }
    else if (m_current == "Network") {
        sprintf(text[0], "%s", m_current.c_str());
        sprintf(text[1], "%d", m_selected_y);
    }

    for (int i = 0; i < 8; ++i) {
        display.rect((16 - (strlen(text[i]))) * 4, i * 8, strlen(text[i]) * 8, 8, i == m_selected_y + 3 ? 1 : 0, true);
        display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, i == m_selected_y + 3 ? 0 : 1);
    }
    display.show();
}