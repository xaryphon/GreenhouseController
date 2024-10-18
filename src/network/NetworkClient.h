#ifndef GREENHOUSE_CONTROLLER_NETWORKCLIENT_H
#define GREENHOUSE_CONTROLLER_NETWORKCLIENT_H

#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cJSON.h"

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
} TLS_CLIENT_T;

#endif //GREENHOUSE_CONTROLLER_NETWORKCLIENT_H
