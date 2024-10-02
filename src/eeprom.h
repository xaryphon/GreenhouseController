#pragma once

#include "PicoI2C.h"
#include <memory>

struct EepromData;
class Eeprom {
public:
    Eeprom(PicoI2C &i2c);
    ~Eeprom();

    void LoadBlocking();

    void QueueTargetPPM(uint16_t ppm);
    void QueueNetworkCredentials(const char *ssid, const char *password);

private:
    static void task_entry(void *);
    void task();

    PicoI2C &m_i2c;
    SemaphoreHandle_t m_should_flush;
    Fmutex m_flush_mutex;
    Fmutex m_access_mutex;
    std::unique_ptr<EepromData> m_data;
};

