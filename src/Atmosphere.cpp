#include "Atmosphere.h"

Atmosphere::Atmosphere(std::shared_ptr<ModbusClient> modbus)
: m_rh(modbus, 241, 256)
, m_temp(modbus, 241, 257)
{
}

void Atmosphere::Read() {
    m_rh_value = m_rh.read();
    m_temp_value = m_temp.read();
}

