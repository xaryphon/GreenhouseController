#pragma once

#include "eeprom.h"
#include <cstdint>

class SettingsDispatcher {
public:
    SettingsDispatcher(Eeprom *eeprom)
    : m_eeprom(eeprom)
    {}

    void SetTargetPPM(uint16_t ppm);

    // NOTE: SSID and password will get trimmed to NETWORK_SSID_MAX_LENGTH and NETWORK_PASSWORD_MAX_LENGTH respectively
    void SetNetworkCredentials(const char *ssid, const char *password);

private:
    Eeprom *m_eeprom;
};

