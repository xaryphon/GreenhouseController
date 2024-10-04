#pragma once

#include "ModbusRegister.h"
#include "pico/types.h"

class Co2Probe {
public:
    Co2Probe(std::shared_ptr<ModbusClient> modbus);

    uint ReadPPM();

private:
    ModbusClient *m_client;
    ModbusRegister m_register;
};

