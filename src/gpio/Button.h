#ifndef GREENHOUSE_CONTROLLER_BUTTON_H
#define GREENHOUSE_CONTROLLER_BUTTON_H

#include <string>

#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pico/stdlib.h"

class Button {
public:
    Button(const std::string& name_, int pin_, QueueHandle_t *queue_);
private:
    void run();
    static void runner(void *params);

    int m_pin;
    int m_debounce = 0;
    QueueHandle_t *m_queue;
};

#endif //GREENHOUSE_CONTROLLER_BUTTON_H
