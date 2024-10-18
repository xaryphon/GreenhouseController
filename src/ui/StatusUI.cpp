#include "StatusUI.h"
#include "UI.h"

StatusUI::StatusUI(Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller)
: m_selected(0)
, m_co2_probe(co2_probe)
, m_motor(motor)
, m_atmo(atmo)
, m_controller(controller)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}


void StatusUI::input(uint input) {
    switch (input) {
        // enter settings menu
        case ROT_SW_PIN:
        case BTN1_PIN:
            UI::m_current = SETTINGS;
            break;
        // toggle between status screens
        case BTN0_PIN:
            m_selected == 0 ? m_selected = 1 : m_selected = 0;
            break;
        // unbound inputs
        case ROT_A_PIN:
        case ROT_B_PIN:
            break;
        case BTN2_PIN:
            break;
        default:
            break;
    }
}

void StatusUI::update_display(ssd1306os &display) {
    char text[8][17] = {};
    display.fill(0);

    if (m_selected == 0) {
        sprintf(text[0], "Sensor Status");
        uint8_t pressure[2];
#if 1
        // last minute copy-paste from previous project for demo, worked fine??
        i2c_read_blocking(i2c1, 0x40, pressure, 2, false);
#else
        // proper way to do this with current library?? cant really test post-demo
        i2c->read(0x40, pressure, 2);
#endif
        auto read = (static_cast<int16_t>(pressure[0] << 8 | pressure[1])) / 240.f;
        sprintf(text[2], "  CO2 %3u   ppm", +m_co2_probe->GetLastPPM());
        sprintf(text[3], "   RH %5.1f %%RH", m_atmo->GetRelativeHumidity() / 10.f);
        sprintf(text[4], " Temp %5.1f C  ", m_atmo->GetTemperature() / 10.f);
        sprintf(text[5], "Pressure %.1f pa", read);
    }
    else if (m_selected == 1) {
        sprintf(text[0], "System Status");
        uint16_t temp_raw = adc_read();
        float temp = 27.f - (temp_raw / 4095.0f * 3.3f - 0.706f) / 0.001721f;
        sprintf(text[2], "Tcore %5.1f C  ", temp);
        sprintf(text[3], "Motor %5.1f %%  ", m_motor->GetPower() / 10.f);
        sprintf(text[4], "CO2 Target %4d", m_controller->GetTargetPPM());
    }

    for (int i = 0; i < 8; ++i) {
        display.text(text[i], (16 - (strlen(text[i]))) * 4, i * 8, 1);
    }

    display.show();
}
