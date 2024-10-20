#include "Co2Probe.h"

Co2Probe::Co2Probe(std::shared_ptr<ModbusClient> modbus)
: m_register(modbus, 240, 256)
{
}

uint Co2Probe::ReadPPM()
{
    uint r = m_register.read();
    m_last_read = r;
    return r;
}

