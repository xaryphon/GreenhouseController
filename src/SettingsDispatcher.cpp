#include "SettingsDispatcher.h"

SettingsDispatcher::SettingsDispatcher(Eeprom *eeprom, Controller *controller)
: m_eeprom(eeprom)
, m_controller(controller)
{
    m_controller->m_settings = this;
}

void SettingsDispatcher::SetTargetPPM(uint16_t ppm) {
    m_controller->SetTargetPPM(ppm);
    m_eeprom->QueueTargetPPM(ppm);
}

void SettingsDispatcher::SetNetworkCredentials(const char *ssid, const char *password) {
    m_eeprom->QueueNetworkCredentials(ssid, password);
}

