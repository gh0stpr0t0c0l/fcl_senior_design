#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stdbool.h>
// #include "telemetry.h"
#include "esp_err.h"

#define WIFI_SSID "DRONE_WIFI"
#define WIFI_PASS "password"
#define UDP_PORT 1234
#define UNICAST_IP "192.168.4.2"
//#define BROADCAST_IP "192.168.4.255"
// #define TELEMETRY_MAX_LEN 128
// #define TELEMETRY_QUEUE_LEN 16
//#define MULTICAST_IP "239.1.1.1"

void wifi_init(void);

void wifi_stop(void);

esp_err_t wifi_start_udp_broadcast(void);

bool wifi_is_connected(void);

#endif /* WIFI_H */
