#pragma once

#include "ModbusRegister.h"
#include "pico/types.h"

class Co2Probe {
public:
    Co2Probe(std::shared_ptr<ModbusClient> modbus);

    uint ReadPPM();
    uint GetLastPPM() {
        return m_last_read;
    }

private:
    ModbusClient *m_client;
    ModbusRegister m_register;
    uint m_last_read;
};

