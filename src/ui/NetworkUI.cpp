#include "NetworkUI.h"
#include "UI.h"

static mono_vlsb enter(enter_key, 8, 8);
static mono_vlsb pal_bw(palette_bw, 2, 1);
static mono_vlsb pal_wb(palette_wb, 2, 1);

NetworkUI::NetworkUI(SettingsDispatcher *dispatcher)
: m_dispatcher(dispatcher)
, m_stage(0)
, m_selected_y(0)
, m_selected_x(0)
{
    // 32 = first printable ascii character " " (space)
    for (int i = 0, c = 32; i < 6; ++i) {
        for (int j = 0; j < 16; ++j) {
            if (isprint(c)) m_keyboard[i][j] = c;
            ++c;
        }
    }
}


void NetworkUI::input(uint input) {
    switch (input) {
        // enter selected key
        case ROT_SW_PIN:
            // if enter pressed
            if (m_selected_y == 5 && m_selected_x == 15) {
                // ssid -> pwd
                if (m_stage == 0) {
                    m_stage = 1;
                    break;
                }
                // submit if both ssid and pwd entered
                else if (m_stage == 1) {
                    m_dispatcher->SetNetworkCredentials(m_input[0].c_str(), m_input[1].c_str());
                }
            }
            // if character pressed
            else {
                if (m_input[m_stage].size() < (m_stage == 0 ? NETWORK_SSID_MAX_LENGTH : NETWORK_PASSWORD_MAX_LENGTH)) {
                    m_input[m_stage].push_back(m_keyboard[m_selected_y][m_selected_x]);
                }
                break;
            }
        case BTN1_PIN:
            // return to settings and reset
            if (m_selected_y == 5 && m_selected_x == 15) {
                m_input[0] = "";
                m_input[1] = "";
                m_selected_y = 0;
                m_selected_x = 0;
                m_stage = 0;
                UI::m_current = SETTINGS;
            }
            // backspace
            else {
                if (!m_input[m_stage].empty()) m_input[m_stage].pop_back();
            }
            break;
        // move right
        case ROT_A_PIN:
            // 16 chars per row
            if (m_selected_x < 15) m_selected_x += 1;
            else m_selected_x = 0;
            break;
        // move left
        case ROT_B_PIN:
            if (m_selected_x > 0) m_selected_x -= 1;
            else m_selected_x = 15;
            break;
        // move down
        case BTN0_PIN:
            // 6 rows for keyboard
            if (m_selected_y < 5) m_selected_y += 1;
            else m_selected_y = 0;
            break;
        // move up
        case BTN2_PIN:
            if (m_selected_y > 0) m_selected_y -= 1;
            else m_selected_y = 5;
            break;
        default:
            break;
    }
}

void NetworkUI::update_display(ssd1306os &display) {
    char text[8][17] = {};
    display.fill(0);

    sprintf(text[0], "%s %s", "Network", m_stage == 0 ? "SSID" : "PWD");
    sprintf(text[1], "%s", m_input[m_stage].substr(m_input[m_stage].size() < 16 ? 0 : m_input[m_stage].size() - 16).c_str());
    sprintf(text[2], "%s", m_keyboard[0]);

    display.text(text[0], 0 * 8, 0 * 8, 1);
    display.text(text[1], 0 * 8, 1 * 8, 1);

    // if input is too long for screen
    if (m_input[m_stage].size() > 16) {
        display.rect(0, 1 * 8, 8, 8, 1, true);
        display.text("<", 0, 1 * 8, 0);
    }
    // keyboard
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 16; ++j) {
            display.rect(j * 8, (i + 2) * 8, 8, 8, i == m_selected_y ? j == m_selected_x ? 1 : 0 : 0, true);
            display.text(&m_keyboard[i][j], j * 8, (i + 2) * 8, i == m_selected_y ? j == m_selected_x ? 0 : 1 : 1);
        }
    }
    // enter key
    display.blit(enter, 15 * 8, 7 * 8, 0xFFF, m_selected_y == 5 && m_selected_x == 15 ? &pal_wb : &pal_bw);

    display.show();
}
