#include "network.h"

static int target = 0;

static err_t tls_client_close(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending request\n");
    err = altcp_write(state->pcb, state->http_request, strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("timed out\n");
    state->error = PICO_ERROR_TIMEOUT;
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("tls_client_err %d\n", err);
    tls_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {
        /* For simplicity this examples creates a buffer on stack the size of the data pending here,
           and copies all the data to it in one go.
           Do be aware that the amount of data can potentially be a bit large (TLS record size can be 16 KB),
           so you may want to use a smaller fixed size buffer and copy the data to it using a loop, if memory is a concern */
        //char buf[p->tot_len + 1];
        char *buf= (char *) malloc(p->tot_len + 1);
        char raw_json[p->tot_len + 1];

        pbuf_copy_partial(p, buf, p->tot_len, 0);
        buf[p->tot_len] = 0;
        // {"executed_at":"2024-10-04T11:52:05Z","position":null,"id":44309941,"command_string":"target=500","created_at":"2024-10-04T11:51:55Z"}

        //printf("***\nnew data received from server:\n***\n%s\n", buf);

        if (sscanf(buf, "%*[^{]%s%*[\r]%*[\n]", raw_json) == 1) {
            const cJSON *json = cJSON_Parse(raw_json);
            const cJSON *command = cJSON_GetObjectItemCaseSensitive(json, "command_string");
            if (command != NULL) {
                printf("json found:\n%s\n", cJSON_Print(json));
                sscanf(command->valuestring, "target=%d", &target);
                printf("Target set to %d\n", target);
            }
        }
        else {
            printf("no command received\n");
        }
        free(buf);
        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);

    state->complete = true;
    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;
    u16_t port = 443;
    //u16_t port = 21883; // Joe's secure MQTT
    //u16_t port = 8883; // secure MQTT

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}


static bool tls_client_open(const char *hostname, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;


    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, state->timeout * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    /* Set SNI */
    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();

    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }


    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
TLS_CLIENT_T* tls_client_init(void) {
    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}
static void tlsdebug(void *ctx, int level, const char *file, int line, const char *message){
    fputs(message, stdout);
}

bool network_connect(const char *ssid, const char *pwd) {
    printf("SSID: %s\nPWD: %s\n", ssid, pwd);

    /*
     * arch_init needs to be done only once, so moved to parent task
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return false;
    }*/

    cyw43_arch_enable_sta_mode();
    int err = cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, CYW43_AUTH_WPA2_AES_PSK, 15000);
    if (err) {
        printf("failed to connect %d\n", err);
        return false;
    }

    printf("Wifi connected\n");
    return true;
}

// return atm is only co2 target as int, though we dont really need anything else
int tls_request(TLS_CLIENT_T *client, const char *request, struct altcp_tls_config *conf) {
    client->complete = false;
    client->pcb = altcp_tls_new(conf, IPADDR_TYPE_ANY);
    client->http_request = request;
    client->timeout = TLS_CLIENT_TIMEOUT_SECS;
    tls_client_open(TLS_CLIENT_SERVER, client);

    while(!client->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        //sleep_ms(1000);
        vTaskDelay(1000);
#endif
    }
    tls_client_close(client);
    return target;
}