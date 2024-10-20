#pragma once

#include "ModbusRegister.h"

class Co2Probe {
public:
    Co2Probe(std::shared_ptr<ModbusClient> modbus);

    uint ReadPPM();

    uint GetLastPPM() {
        return m_last_read;
    }

private:
    ModbusRegister m_register;
    uint m_last_read;
};

