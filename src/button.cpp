//
// Created by Noa Storm on 25/09/2024.
//

#include <iostream>
#include "button.h"

Button::Button(std::string name_, int pin_, QueueHandle_t *queue_) : m_name(name_), m_pin(pin_), m_queue(queue_) {
    gpio_init(m_pin);
    gpio_set_dir(m_pin, GPIO_IN);
    gpio_pull_up(m_pin);
    xTaskCreate(Button::runner, m_name.c_str(), 256, (void *) this, tskIDLE_PRIORITY + 1, &m_handle);
}

void Button::run() {
    while (true) {
        if (!gpio_get(m_pin)) {
            if (m_debounce < 3) {
                if (m_debounce == 2) {
                    printf("pressed %s\n", m_name.c_str());
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

