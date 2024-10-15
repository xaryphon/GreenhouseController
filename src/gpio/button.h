//
// Created by Noa Storm on 25/09/2024.
//

#ifndef GREENHOUSE_CONTROLLER_BUTTON_H
#define GREENHOUSE_CONTROLLER_BUTTON_H
#include <string>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class Button {
public:
    Button(std::string name_, int pin_, QueueHandle_t *queue_);
private:
    void run();
    static void runner(void *params);
    const std::string m_name;
    int m_pin;
    int m_debounce = 0;
    QueueHandle_t *m_queue;
    TaskHandle_t m_handle;
};

#endif //GREENHOUSE_CONTROLLER_BUTTON_H
