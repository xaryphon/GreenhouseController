#include "Motor.h"

Motor::Motor(std::shared_ptr<ModbusClient> modbus)
: m_ao1(modbus, 1, 0x40000)
, m_ai1(modbus, 1, 0x30000)
{}

void Motor::Write(uint16_t power)
{
    m_ao1.write(power);
}

uint16_t Motor::Read()
{
    return m_ai1.read();
}

