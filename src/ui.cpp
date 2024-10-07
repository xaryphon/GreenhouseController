//
// Created by Noa Storm on 30/09/2024.
//

#include "ui.h"
#include "SettingsDispatcher.h"
#include "hardware/adc.h"

// custom enter key symbol for keyboard + palettes for colour inversion
static mono_vlsb enter(enter_key, 8, 8);
static mono_vlsb pal_bw(palette_bw, 2, 1);
static mono_vlsb pal_wb(palette_wb, 2, 1);

UI::UI(std::string name_, QueueHandle_t *queue_, SettingsDispatcher *settings, Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller)
: m_name(name_)
, m_queue(queue_)
, m_settings(settings)
, m_co2_probe(co2_probe)
, m_motor(motor)
, m_atmo(atmo)
, m_controller(controller)
, m_current("status")
, m_selected_y(0)
, m_selected_x(0)
, m_target(850) // 1500 + 200 / 2 = 850, midpoint of co2 target levels
{
    // first printable ascii character " " (space)
    int c = 32;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (isprint(c)) m_keyboard[i][j] = c;
            ++c;
        }
    }
    
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    xTaskCreate(UI::runner, m_name.c_str(), 256, (void *) this, tskIDLE_PRIORITY + 1, nullptr);
}

void UI::run() {
    m_menu["status"] = {"Sensor Status", "System Status"};
    m_menu["Settings"] = {"Back", "CO2", "Network"};

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
    if (m_current == "status") status(input);
    else if (m_current == "Settings") settings(input);
    else if (m_current == "CO2") co2(input);
    else if (m_current == "Network") network(input);
}

// menu keybind logic
void UI::status(uint input) {
    switch (input) {
        case ROT_SW_PIN:
        case BTN1_PIN:
            m_current = "Settings";
            m_selected_y = 0;
            break;
        // do we leave unbound keys listed here for clarity or remove them for compactness
        case ROT_A_PIN:
        case ROT_B_PIN:
            break;
        case BTN0_PIN:
            m_selected_y == 0 ? m_selected_y = 1 : m_selected_y = 0;
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
        case BTN1_PIN:
            m_menu[m_current][m_selected_y] == "Back" ? m_current = "status" : m_current = m_menu[m_current][m_selected_y];
            m_selected_y = 0;
            m_selected_x = 0;
            if (m_current == "CO2") m_target = m_controller->GetTargetPPM();
            if (m_current == "Network") m_target = 0;
            break;
        case ROT_A_PIN:
        case ROT_B_PIN:
        case BTN0_PIN:
        case BTN2_PIN:
            m_selected_y = ((input == ROT_A_PIN) || (input == BTN0_PIN) ? m_selected_y + 1 : m_selected_y > 0 ? m_selected_y - 1 : 2) % m_menu["Settings"].size();
            break;
        default:
            break;
    }
}

void UI::co2(uint input){
    switch (input) {
        case ROT_SW_PIN:
            m_settings->SetTargetPPM(m_target);

        case BTN1_PIN:
            m_current = "Settings";
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
                if (m_target == 0) {
                    m_target = 1;
                    break;
                }
                else if (m_target == 1) {
                    m_settings->SetNetworkCredentials(m_input[0].c_str(), m_input[1].c_str());
                }
            }
            else {
                if (m_input[m_target].size() < (m_target == 0 ? NETWORK_SSID_MAX_LENGTH : NETWORK_PASSWORD_MAX_LENGTH)) {
                    m_input[m_target].push_back(m_keyboard[m_selected_y][m_selected_x]);
                }
                break;
            }
        case BTN1_PIN:
            if (m_selected_y == 5 && m_selected_x == 15) {
                m_current = "Settings";
                m_input[0] = "";
                m_input[1] = "";
                m_selected_y = 0;
                m_selected_x = 0;
                m_target = 0;
            }
            else {
                if (!m_input[m_target].empty()) m_input[m_target].pop_back();
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
        case BTN2_PIN:
            // move up
            if (m_selected_y > 0) m_selected_y -= 1;
            break;
        default:
            break;
    }
}

void UI::update_display(ssd1306os &display) {
    char text[8][17] = {};
    display.fill(0);

    if (m_current == "status") {
        sprintf(text[0], "%s", m_menu[m_current][m_selected_y].c_str());
        if (m_menu[m_current][m_selected_y] == "Sensor Status") {
            sprintf(text[2], "  CO2 %3u   ppm", +m_co2_probe->GetLastPPM());
            sprintf(text[3], "   RH %5.1f %%RH", m_atmo->GetRelativeHumidity() / 10.f);
            sprintf(text[4], " Temp %5.1f C  ", m_atmo->GetTemperature() / 10.f);
        }
        else if (m_menu[m_current][m_selected_y] == "System Status") {
            uint16_t temp_raw = adc_read();
            float temp = 27.f - (temp_raw / 4095.0f * 3.3f - 0.706f) / 0.001721f;
            sprintf(text[2], "Tcore %5.1f C  ", temp);
            sprintf(text[3], "Motor %5.1f %%  ", m_motor->GetPower() / 10.f);
        }
    }
    else if (m_current == "Settings") {
        sprintf(text[0], "%s", m_current.c_str());
        sprintf(text[3], "%s", m_menu[m_current][0].c_str());
        sprintf(text[4], "%s", m_menu[m_current][1].c_str());
        sprintf(text[5], "%s", m_menu[m_current][2].c_str());
    }
    else if (m_current == "CO2") {
        sprintf(text[0], "%s", m_current.c_str());
        sprintf(text[1], "Max:        1500");
        sprintf(text[2], "Min:         200");
        sprintf(text[3], "Current:    %4d", +m_controller->GetTargetPPM());
        sprintf(text[4], "CO2 target: %4d", m_target);
    }
    else if (m_current == "Network") {
        sprintf(text[0], "%s %s", m_current.c_str(), m_target == 0 ? "SSID" : "PWD");
        sprintf(text[1], "%s", m_input[m_target].substr(m_input[m_target].size() < 16 ? 0 : m_input[m_target].size() - 16).c_str());
        sprintf(text[2], "%s", m_keyboard[0]);
    }

    if (m_current != "Network") {
        // display with centering for menu items
        for (int i = 0; i < 8; ++i) {
            display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, 1);
        }
        if (m_current == "Settings") {
            display.rect((16 - (strlen(text[m_selected_y + 3]))) * 4, (m_selected_y + 3) * 8, strlen(text[m_selected_y + 3]) * 8, 8, 1, true);
            display.text(text[m_selected_y + 3], (16 - (strlen(text[m_selected_y + 3]))) * 4, (m_selected_y + 3) * 8, 0);
        }
    }
    else {
        // keyboard display in network
        display.text(text[0], 0 * 8, 0 * 8, 1);
        display.text(text[1], 0 * 8, 1 * 8, 1);
        if (m_input[m_target].size() > 16) {
            display.rect(0, 1 * 8, 8, 8, 1, true);
            display.text("<", 0, 1 * 8, 0);
        }
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 16; ++j) {
                display.rect(j * 8, (i + 2) * 8, 8, 8, i == m_selected_y ? j == m_selected_x ? 1 : 0 : 0, true);
                display.text(&m_keyboard[i][j], j * 8, (i + 2) * 8, i == m_selected_y ? j == m_selected_x ? 0 : 1 : 1);
            }
        }
        display.blit(enter, 15 * 8, 7 * 8, 0xFFF, m_selected_y == 5 && m_selected_x == 15 ? &pal_wb : &pal_bw);
    }
    display.show();
}
