#include "Motor.h"

Motor::Motor(std::shared_ptr<ModbusClient> modbus)
: m_ao1(modbus, 1, 0x40000)
, m_power(0)
{
}

void Motor::Write(uint16_t power)
{
    m_ao1.write(power);
    m_power = power;
}

uint16_t Motor::GetPower()
{
    return m_power;
}

