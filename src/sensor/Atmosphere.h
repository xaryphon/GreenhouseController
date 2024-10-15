#pragma once

#include "ModbusClient.h"
#include "ModbusRegister.h"
#include <memory>

class Atmosphere {
public:
    Atmosphere(std::shared_ptr<ModbusClient> modbus);

    void Read();

    uint16_t GetRelativeHumidity() {
        return m_rh_value;
    }

    uint16_t GetTemperature() {
        return m_temp_value;
    }

private:
    ModbusRegister m_rh;
    ModbusRegister m_temp;
    uint16_t m_rh_value;
    uint16_t m_temp_value;
};

