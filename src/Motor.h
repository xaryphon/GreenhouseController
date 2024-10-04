#pragma once

#include "ModbusClient.h"
#include "ModbusRegister.h"
#include <memory>

class Motor {
public:
    Motor(std::shared_ptr<ModbusClient> modbus);

    void Write(uint16_t speed);
    uint16_t Read();

private:
    ModbusRegister m_ao1;
    ModbusRegister m_ai1;
};

