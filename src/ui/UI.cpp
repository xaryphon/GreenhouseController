#include "UI.h"

UI::UI(QueueHandle_t *queue_, SettingsDispatcher *dispatcher, Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller)
: StatusUI(co2_probe, motor, atmo, controller)
, CO2UI(controller, dispatcher)
, NetworkUI(dispatcher)
, SettingsUI()
, m_queue(queue_)
, m_i2c(std::make_shared<PicoI2C>(1))
{
    xTaskCreate(UI::runner, "UI", 256, (void *) this, TASK_UI_PRIORITY, nullptr);
}

int UI::m_current = STATUS;

void UI::run() {
    ssd1306os display(m_i2c);

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
    if (m_current == STATUS) StatusUI::input(input);
    else if (m_current == CO2) CO2UI::input(input);
    else if (m_current == NETWORK) NetworkUI::input(input);
    else if (m_current == SETTINGS) { SettingsUI::input(input); if(m_current == CO2) get_target(); } // get_target is for co2ui to default to current target when opened
}

void UI::update_display(ssd1306os &display) {
    if (m_current == STATUS) StatusUI::update_display(display);
    else if (m_current == CO2) CO2UI::update_display(display);
    else if (m_current == NETWORK) NetworkUI::update_display(display);
    else if (m_current == SETTINGS) SettingsUI::update_display(display);
}
