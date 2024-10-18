#include "SettingsUI.h"
#include "UI.h"

SettingsUI::SettingsUI()
: m_selected(0)
{
}

void SettingsUI::input(uint input) {
    switch (input) {
        // enter selected menu
        case ROT_SW_PIN:
        case BTN1_PIN:
            // 0 == STATUS, 1 == CO2, 2 == NETWORK
            UI::m_current = m_selected;
            break;
        // scroll through settings, 0-2
        case ROT_A_PIN:
        case ROT_B_PIN:
        case BTN0_PIN:
        case BTN2_PIN:
            m_selected = ((input == ROT_A_PIN) || (input == BTN0_PIN) ? m_selected + 1 : m_selected > 0 ? m_selected - 1 : 2) % 3;
            break;
        default:
            break;
    }
}

void SettingsUI::update_display(ssd1306os &display) {
    char text[8][17] = {};
    display.fill(0);

    sprintf(text[0], "Settings");
    sprintf(text[3], "Back");
    sprintf(text[4], "CO2");
    sprintf(text[5], "Network");

    display.rect((16 - (strlen(text[m_selected + 3]))) * 4, (m_selected + 3) * 8, strlen(text[m_selected + 3]) * 8, 8, 1, true);
    for (int i = 0; i < 8; ++i) {
        display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, i == m_selected + 3 ? 0 : 1);
    }
    display.show();
}
