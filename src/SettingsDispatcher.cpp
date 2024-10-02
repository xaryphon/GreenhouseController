#include "SettingsDispatcher.h"

void SettingsDispatcher::SetTargetPPM(uint16_t ppm) {
    m_eeprom->QueueTargetPPM(ppm);
}

void SettingsDispatcher::SetNetworkCredentials(const char *ssid, const char *password) {
    m_eeprom->QueueNetworkCredentials(ssid, password);
}

