#include "Network.h"
#include "SettingsDispatcher.h"

extern "C" {
TLS_CLIENT_T* tls_client_init(void);
bool network_connect(const char *ssid, const char *pwd);
int tls_request(TLS_CLIENT_T_ *client, const char *request, struct altcp_tls_config *conf);
}

Network::Network(Co2Probe *co2_probe, Motor *motor, Atmosphere *atmo, Controller *controller)
: m_ssid(NETWORK_SSID_DEFAULT)
, m_pwd(NETWORK_PASSWORD_DEFAULT)
, m_connected(false)
, m_client(tls_client_init())
, m_tls_config(nullptr)
, m_co2_probe(co2_probe)
, m_motor(motor)
, m_atmo(atmo)
, m_controller(controller)
{
    xTaskCreate(Network::runner, "NETWORK", 6000, (void *) this, TASK_NETWORK_PRIORITY, nullptr);
}

void Network::run() {
    cyw43_arch_init();
    m_tls_config = altcp_tls_create_config_client(nullptr, 0);
    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)m_tls_config, MBEDTLS_SSL_VERIFY_OPTIONAL);

    while(true) {
        // connect to new network
        if (!m_connected && !m_ssid.empty() && !m_pwd.empty()) {
            m_connected = connect();
        }
        if (m_connected) {
            m_request << "POST /update.json"
                      << "?field1=" << m_co2_probe->GetLastPPM()  // current co2 level
                      << "&field2=" << m_atmo->GetRelativeHumidity() / 10.f  // humidity
                      << "&field3=" << m_atmo->GetTemperature() / 10.f  // temp
                      << "&field4=" << m_motor->GetPower() / 10.f // fan speed
                      << "&field5=" << m_controller->GetTargetPPM()   // target co2 level
                      << "&api_key=" API_KEY "&talkback_key=" TB_KEY " HTTP/1.1\r\n"
                      << "Host:" TLS_CLIENT_SERVER "\r\nConnection:keep-alive\r\n\r\n";
            //printf("\n%s", m_request.str().c_str());
            printf("sending request...\n");
            int new_target = tls_request(m_client, m_request.str().c_str(), m_tls_config);
            if (new_target != 0 && new_target != m_controller->GetTargetPPM()) {
                m_settings->SetTargetPPM(new_target);
            }
            m_request.str("");
        }
        vTaskDelay(5000);
    }
}

void Network::runner(void *params) {
    Network *instance = static_cast<Network *>(params);
    instance->run();
}

void Network::set_creds(const char *ssid, const char *pwd) {
    m_ssid = ssid;
    m_pwd = pwd;
    m_connected = false;
}

bool Network::connect() {
    return network_connect(m_ssid.c_str(), m_pwd.c_str());
}
