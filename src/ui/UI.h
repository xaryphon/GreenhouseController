#ifndef GREENHOUSE_CONTROLLER_UI_H
#define GREENHOUSE_CONTROLLER_UI_H

#include <string>

#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "StatusUI.h"
#include "CO2UI.h"
#include "NetworkUI.h"
#include "SettingsUI.h"

#include "sensor/Atmosphere.h"
#include "sensor/Co2Probe.h"
#include "control/Controller.h"
#include "control/Motor.h"
#include "pico/stdlib.h"

#include "display/ssd1306os.h"
#include "control/SettingsDispatcher.h"
#include "hardware/adc.h"

class UI : public virtual StatusUI, CO2UI, NetworkUI, SettingsUI {
public:
    UI(QueueHandle_t *queue_, SettingsDispatcher *settings, Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller);
    void input(uint input) override;
    void update_display(ssd1306os &display) override;
    static int m_current;
private:
    void run();
    static void runner(void *params);

    QueueHandle_t *m_queue;
    std::shared_ptr<PicoI2C> m_i2c;
};

#endif //GREENHOUSE_CONTROLLER_UI_H
