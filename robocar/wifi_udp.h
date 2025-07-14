#ifndef WIFI_UDP_H
#define WIFI_UDP_H

#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

/* Initialise NVS, join Wi-Fi (with dual-SSID fallback) and open a UDP socket.
 * Call this **once** at start-up (e.g. from `app_main()`).
 * The function blocks until the interface is up (or fails after retries).     */
void wifi_udp_init(void);

/* Returns `true` when the station is connected and the UDP socket is ready.  */
bool wifi_udp_ready(void);

/* Send an arbitrary buffer through the pre-opened UDP socket.
 * Returns the number of bytes sent, or ‑1 on error.                           */
int wifi_udp_send(const char *payload, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_UDP_H */
