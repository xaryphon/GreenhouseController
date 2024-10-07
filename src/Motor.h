#pragma once

#include "ModbusClient.h"
#include "ModbusRegister.h"
#include <memory>

class Motor {
public:
    Motor(std::shared_ptr<ModbusClient> modbus);

    void Write(uint16_t speed);
    uint16_t GetPower();

private:
    ModbusRegister m_ao1;
    uint16_t m_power;
};

