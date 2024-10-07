//
// Created by Noa Storm on 30/09/2024.
//

#define TLS_CLIENT_SERVER        "api.thingspeak.com"
#define TLS_TALKBACK             "POST /update.json?field1=70&api_key=B7C4VLKMNBYD4HLJ&talkback_key=K4J932OFNIJEGD9A HTTP/1.1\r\n" \
                                 "Host: " TLS_CLIENT_SERVER "\r\n" \
                                 "Connection: close\r\n" \
                                 "\r\n"
#define TLS_CLIENT_TIMEOUT_SECS  10
#if 0
#define WIFI_SSID "SmartIotMQTT"
#define WIFI_PASSWORD "SmartIot"
#else
#define WIFI_SSID "STORM's A15"
#define WIFI_PASSWORD "zaym8p9b8a44nqe"
#define WIFI_SSID2 "/dev/null"
#define WIFI_PASSWORD2 "fs4j7irntkxfqzd"
#endif

#ifndef GREENHOUSE_CONTROLLER_API_H
#define GREENHOUSE_CONTROLLER_API_H

#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cJSON.h"
#include "network_client.h"


class Network{
public:
    explicit Network(std::string name_);
    void set_creds(const char *ssid, const char *pwd);
private:
    void run();
    static void runner(void *params);
    bool connect();
    const std::string m_name;
    TLS_CLIENT_T *m_client;
    struct altcp_tls_config *m_tls_config;
    std::string m_ssid;
    std::string m_pwd;
    std::stringstream m_request;
    bool m_connected;
};
#endif //GREENHOUSE_CONTROLLER_API_H
