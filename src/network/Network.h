#ifndef GREENHOUSE_CONTROLLER_API_H
#define GREENHOUSE_CONTROLLER_API_H

#include <sstream>

#include "config.h"
#include "FreeRTOS.h"
#include "task.h"

#include "NetworkClient.h"
#include "Co2Probe.h"
#include "Atmosphere.h"
#include "Motor.h"
#include "Controller.h"

class Network{
public:
    explicit Network(Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller);
    void set_creds(const char *ssid, const char *pwd);
private:
    void run();
    static void runner(void *params);
    bool connect();

    TLS_CLIENT_T *m_client;
    struct altcp_tls_config *m_tls_config;
    std::string m_ssid;
    std::string m_pwd;
    std::stringstream m_request;
    bool m_connected;

    Co2Probe *m_co2_probe;
    Motor *m_motor;
    Atmosphere *m_atmo;
    Controller *m_controller;
    SettingsDispatcher *m_settings;

    friend class SettingsDispatcher;
};
#endif //GREENHOUSE_CONTROLLER_API_H
