#pragma once

#include "ModbusClient.h"
#include "ModbusRegister.h"
#include <memory>

class Motor {
public:
    Motor(std::shared_ptr<ModbusClient> modbus);

    void Write(uint16_t speed);
    void Read();
    uint16_t GetRPM();

private:
    ModbusRegister m_ao1;
    ModbusRegister m_ai1;
    uint64_t m_last_read_us;
    uint16_t m_rpm;
};

