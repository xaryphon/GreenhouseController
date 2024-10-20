#pragma once

#include <cstdint>

#include "Controller.h"
#include "eeprom.h"
#include "Network.h"

class SettingsDispatcher {
public:
    SettingsDispatcher(Eeprom *eeprom, Controller *controller, Network *network);

    void SetTargetPPM(uint16_t ppm);

    // NOTE: SSID and password will get trimmed to NETWORK_SSID_MAX_LENGTH and NETWORK_PASSWORD_MAX_LENGTH respectively
    void SetNetworkCredentials(const char *ssid, const char *password);

private:
    Eeprom *m_eeprom;
    Controller *m_controller;
    Network *m_network;
};

