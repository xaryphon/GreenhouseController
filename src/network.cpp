//
// Created by Noa Storm on 07/10/2024.
//

#include "network.h"

extern "C" {
TLS_CLIENT_T* tls_client_init(void);
bool network_connect(const char *ssid, const char *pwd);
int tls_request(TLS_CLIENT_T_ *client, const char *request, struct altcp_tls_config *conf);
}

Network::Network(std::string name_)
: m_name(name_)
, m_ssid("")
, m_pwd("")
, m_connected(false)
, m_client(tls_client_init())
, m_tls_config(NULL)
{
    xTaskCreate(Network::runner, m_name.c_str(), 6000, (void *) this, tskIDLE_PRIORITY + 1, nullptr);
}

void Network::run() {
    cyw43_arch_init();
    m_tls_config = altcp_tls_create_config_client(NULL, 0);
    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)m_tls_config, MBEDTLS_SSL_VERIFY_OPTIONAL);

    int test = 0;
    while(true) {
        while(!m_connected && !m_ssid.empty() && !m_pwd.empty()) {
            m_connected = connect();
        }
        if (m_connected) {
            m_request << "POST /update.json"
                    << "?field1=" << ++test  // current co2 level
                    << "&field2=" << 12  // humidity
                    << "&field3=" << 55  // temp
                    << "&field4=" << 100 // fan speed
                    << "&field5=" << 0   // target co2 level
                    << "&api_key=B7C4VLKMNBYD4HLJ&talkback_key=K4J932OFNIJEGD9A HTTP/1.1\r\n"
                    << "Host:api.thingspeak.com\r\nConnection:keep-alive\r\n\r\n";
            printf("\n%s", m_request.str().c_str());
            printf("sending request...\n");
            int new_target_or_whatever = tls_request(m_client, m_request.str().c_str(), m_tls_config);
            // do check if target is new and within range
            printf("new target: %d", new_target_or_whatever);
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
