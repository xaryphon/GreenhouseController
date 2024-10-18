#include "Button.h"

Button::Button(const std::string& name, int pin, QueueHandle_t *queue)
: m_pin(pin)
, m_queue(queue)
{
    gpio_init(m_pin);
    gpio_set_dir(m_pin, GPIO_IN);
    gpio_pull_up(m_pin);
    xTaskCreate(Button::runner, name.c_str(), 256, (void *) this, TASK_BUTTON_PRIORITY, nullptr);
}

void Button::run() {
    while (true) {
        if (!gpio_get(m_pin)) {
            if (m_debounce < 3) {
                if (m_debounce == 2) {
                    xQueueSendToBack(*m_queue, static_cast<void *>(&m_pin), pdMS_TO_TICKS(500));
                }
                ++m_debounce;
            }
        } else {
            m_debounce = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void Button::runner(void *params) {
    Button *instance = static_cast<Button *>(params);
    instance->run();
}

