#include "Co2Probe.h"

Co2Probe::Co2Probe(std::shared_ptr<ModbusClient> modbus)
: m_register(modbus, 240, 256)
{}

uint Co2Probe::ReadPPM()
{
    return m_register.read();
}

