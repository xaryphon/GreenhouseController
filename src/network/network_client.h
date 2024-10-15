//
// Created by Noa Storm on 07/10/2024.
//

#ifndef GREENHOUSE_CONTROLLER_NETWORK_CLIENT_H
#define GREENHOUSE_CONTROLLER_NETWORK_CLIENT_H

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cJSON.h"

#define TLS_CLIENT_SERVER "api.thingspeak.com"
#define TLS_CLIENT_TIMEOUT_SECS  10


typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
} TLS_CLIENT_T;

#endif //GREENHOUSE_CONTROLLER_NETWORK_CLIENT_H
