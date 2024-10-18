#pragma once

#include <cstdio>

#include "config.h"
#include "FreeRTOS.h"
#include "queue.h"

#include <hardware/gpio.h>

class RotaryDecoder {
public:
    RotaryDecoder(QueueHandle_t input_queue, uint pin_a, uint pin_b, int scale);
    void OnInterrupt(BaseType_t *xHigherPriorityTaskWoken);

private:
    static void task_entry(void *);

    QueueHandle_t m_input_queue;
    QueueHandle_t m_interrupt_queue;
    uint m_pin_a;
    uint m_pin_b;
    int m_scale;
    int m_counter;
};

