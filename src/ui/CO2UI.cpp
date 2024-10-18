#include "CO2UI.h"
#include "UI.h"

CO2UI::CO2UI(Controller *controller, SettingsDispatcher *dispatcher)
: m_controller(controller)
, m_dispatcher(dispatcher)
, m_target(0)
{
}

void CO2UI::input(uint input) {
    switch (input) {
        // save new target
        case ROT_SW_PIN:
            m_dispatcher->SetTargetPPM(m_target);
        // return to settings
        case BTN1_PIN:
            UI::m_current = SETTINGS;
            break;
        // adjust new target
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

void CO2UI::update_display(ssd1306os &display) {
    char text[8][17] = {};
    display.fill(0);

    sprintf(text[0], "CO2");
    sprintf(text[1], "Max:        1500");
    sprintf(text[2], "Min:         200");
    sprintf(text[3], "Current:    %4d", +m_controller->GetTargetPPM());
    sprintf(text[5], "New target: %4d", m_target);

    for (int i = 0; i < 8; ++i) {
        display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, 1);
    }

    display.show();
}
