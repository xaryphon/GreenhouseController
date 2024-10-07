#include "Motor.h"
#include "hardware/timer.h"

Motor::Motor(std::shared_ptr<ModbusClient> modbus)
: m_ao1(modbus, 1, 0x40000)
, m_ai1(modbus, 1, 0x30000)
, m_last_read_us(0)
, m_rpm(0)
{
}

void Motor::Read() {
    uint64_t now_us = time_us_64();
    m_rpm = (uint64_t)m_ai1.read() * 60'000'000 / (now_us - m_last_read_us);
    m_last_read_us = now_us;
}

void Motor::Write(uint16_t power)
{
    m_ao1.write(power);
}

uint16_t Motor::GetRPM()
{
    return m_rpm;
}

