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
, m_target(850) // 1500 + 200 / 2 = 850, midpoint of co2 target levels
{
    int c = 32;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (isprint(c)) m_keyboard[i][j] = c;
            ++c;
        }
    }

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
            m_menu[m_current][m_selected_y] == "Back" ? m_current = "status" : m_current = m_menu[m_current][m_selected_y];
            m_selected_y = 0;
            m_selected_x = 0;
            if (m_current == "CO2") m_target = 850;
            if (m_current == "Network") m_target = 0;
            break;
        // maybe separate rot_a and rot_b for clarity
        case ROT_A_PIN:
        case ROT_B_PIN:
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
            // send m_target to controller
            m_current = "settings";
            m_target = 850;
            m_selected_y = 0;
            m_selected_x = 0;
            break;
        case ROT_A_PIN:
            if (m_target < 1500) m_target < 1490 ? m_target += 10 : m_target = 1500;
            break;
        case ROT_B_PIN:
            if (m_target > 200) m_target > 210 ? m_target -= 10 : m_target = 200;
            break;
        case BTN0_PIN:
            if (m_target > 200) m_target -= 1;
            break;
        case BTN1_PIN:
            m_current = "settings";
            m_target = 850;
            m_selected_y = 0;
            m_selected_x = 0;
            break;
        case BTN2_PIN:
            if (m_target < 1500) m_target += 1;
            break;
        default:
            break;
    }
}


void UI::network(uint input){
    switch (input) {
        case ROT_SW_PIN:
            if (m_selected_y == 5 && m_selected_x == 15) {
                if (m_target == 0) m_target = 1;
                else if (m_target == 1) {
                    printf("sending data\n");
                    //send data and exit network
                    m_current = "settings";
                    m_text[0] = "";
                    m_text[1] = "";
                    m_selected_y = 0;
                    m_selected_x = 0;
                    m_target = 0;
                }
            }
            else {
                m_text[m_target].push_back(m_keyboard[m_selected_y][m_selected_x]);
            }
            break;
        case ROT_A_PIN:
            // 16 chars per row
            if (m_selected_x < 15) m_selected_x += 1;
            break;
        case ROT_B_PIN:
            if (m_selected_x > 0) m_selected_x -= 1;
            break;
        case BTN0_PIN:
            // move down
            // 6 rows for keyboard
            if (m_selected_y < 5) m_selected_y += 1;
            break;
        case BTN1_PIN:
            if (m_selected_y == 5 && m_selected_x == 15) {
                m_current = "settings";
                m_text[0] = "";
                m_text[1] = "";
                m_selected_y = 0;
                m_selected_x = 0;
                m_target = 0;
            }
            else {
                m_text[m_target].pop_back();
            }
            break;
        case BTN2_PIN:
            // move up
            if (m_selected_y > 0) m_selected_y -= 1;
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
        // get sensor data here somehow
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
        sprintf(text[1], "Max:1500, Min:200");
        sprintf(text[4], "CO2 target: %d", m_target);
    }
    else if (m_current == "Network") {
        sprintf(text[0], "%s %d %d %d %s", m_current.c_str(), m_selected_y, m_selected_x, m_target, m_target == 0 ? "SSID" : "PWD");
        sprintf(text[1], "%s", m_text[m_target].substr(m_text[m_target].size() < 15 ? 0 : m_text[m_target].size() - 15).c_str());
        sprintf(text[2], "%s", m_keyboard[0]);
    }

    if (m_current != "Network") {
        // display with centering for menu items
        for (int i = 0; i < 8; ++i) {
            display.rect((16 - (strlen(text[i]))) * 4, i * 8, strlen(text[i]) * 8, 8, i == m_selected_y + 3 ? 1 : 0, true);
            display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, i == m_selected_y + 3 ? 0 : 1);
        }
    }
    else {
        // keyboard display in network
        display.text(text[0], 0 * 8, 0 * 8, 1);
        display.text(text[1], 1 * 8, 1 * 8, 1);
        if (m_text[m_target].size() > 15) {
            display.rect(0, 1 * 8, 8, 8, 1, true);
            display.text("<", 0, 1 * 8, 0);
        }
        for (int i = 2; i < 8; ++i) {
            for (int j = 0; j < 16; ++j) {
                display.rect(j * 8, i * 8, 8, 8, i == m_selected_y + 2 ? j == m_selected_x ? 1 : 0 : 0, true);
                char c[1] = {text[i][j]};
                display.text(c, j * 8, i * 8, i == m_selected_y + 2 ? j == m_selected_x ? 0 : 1 : 1);
            }
        }
    }
    display.show();
}