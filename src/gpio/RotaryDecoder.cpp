#include "RotaryDecoder.h"

RotaryDecoder::RotaryDecoder(QueueHandle_t input_queue, uint pin_a, uint pin_b, int scale)
: m_input_queue(input_queue)
, m_interrupt_queue(xQueueCreate(8, sizeof(uint8_t)))
, m_pin_a(pin_a)
, m_pin_b(pin_b)
, m_scale(scale)
, m_counter(0)
{
    vQueueAddToRegistry(m_interrupt_queue, "RotaryDecoder");
    gpio_init(pin_a);
    gpio_init(pin_b);
    gpio_set_irq_enabled(pin_a, GPIO_IRQ_EDGE_FALL, true);
    xTaskCreate(task_entry, "RotaryDecoder", 128, this, TASK_ROTARY_PRIORITY, nullptr);
}

void RotaryDecoder::OnInterrupt(BaseType_t *xHigherPriorityTaskWoken) {
    uint8_t b = gpio_get(m_pin_b);
    xQueueSendFromISR(m_interrupt_queue, &b, xHigherPriorityTaskWoken);
}

void RotaryDecoder::task_entry(void *param) {
    auto *d = (RotaryDecoder*)param;
    for (;;) {
        uint8_t event;
        xQueueReceive(d->m_interrupt_queue, &event, portMAX_DELAY);
        if (event) {
            if (d->m_counter > 0) {
                d->m_counter = 0;
            } else if (d->m_counter == -d->m_scale) {
                xQueueSend(d->m_input_queue, &d->m_pin_a, portMAX_DELAY);
                d->m_counter = 0;
            }
            d->m_counter -= 1;
        } else {
            if (d->m_counter < 0) {
                d->m_counter = 0;
            } else if (d->m_counter == d->m_scale) {
                xQueueSend(d->m_input_queue, &d->m_pin_b, portMAX_DELAY);
                d->m_counter = 0;
            }
            d->m_counter += 1;
        }
    }
}

