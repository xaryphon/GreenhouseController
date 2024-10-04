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
#endif

#ifndef GREENHOUSE_CONTROLLER_API_H
#define GREENHOUSE_CONTROLLER_API_H

#include <string.h>
#include <time.h>

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

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
} TLS_CLIENT_T;
/*
TLS_CLIENT_T* tls_client_init(void);
bool network_connect(const char *ssid, const char *pwd);
void tls_request(TLS_CLIENT_T *client, const char *request);
*/
#endif //GREENHOUSE_CONTROLLER_API_H
