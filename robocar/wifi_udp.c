/* Wi-Fi station + UDP helper for ESP-IDF
 * ─────────────────────────────────────────────────────────────
 * • Connects to a primary SSID; after MAXIMUM_RETRY attempts
 *   it automatically switches to a secondary SSID.
 * • Opens a datagram socket that sends telemetry to the
 *   Raspberry-Pi address defined in menuconfig.
 *
 * LED behaviour
 * ─────────────
 * • Solid ON (GPIO 2) while the station is trying to connect.
 * • Blink 3 × (0.5 s period) after a successful connection.
 * • Blink 5 × after exhausting all retries / both SSIDs.
 */

#include "wifi_udp.h"
#include "sdkconfig.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"

#include "nvs_flash.h"

#include "driver/gpio.h"          /* LED GPIO                          */
#include "freertos/FreeRTOS.h"    /* vTaskDelay                        */
#include "freertos/task.h"

#include <string.h>
#include <errno.h>

/* ─────────────────────────── fall-back defaults ───────────────────────── */

#ifndef CONFIG_ROBOT_WIFI_SSID1
#warning "CONFIG_ROBOT_WIFI_SSID1 undefined – using \"ssid-primary\""
#define CONFIG_ROBOT_WIFI_SSID1 "ssid-primary"
#endif
#ifndef CONFIG_ROBOT_WIFI_PASSWORD1
#define CONFIG_ROBOT_WIFI_PASSWORD1 ""
#endif
#ifndef CONFIG_ROBOT_WIFI_SSID2
#warning "CONFIG_ROBOT_WIFI_SSID2 undefined – using \"ssid-backup\""
#define CONFIG_ROBOT_WIFI_SSID2 "ssid-backup"
#endif
#ifndef CONFIG_ROBOT_WIFI_PASSWORD2
#define CONFIG_ROBOT_WIFI_PASSWORD2 ""
#endif
#ifndef CONFIG_ROBOT_PI_IP
#warning "CONFIG_ROBOT_PI_IP undefined – using 192.168.1.100"
#define CONFIG_ROBOT_PI_IP "192.168.1.100"
#endif
#ifndef CONFIG_ROBOT_UDP_PORT
#warning "CONFIG_ROBOT_UDP_PORT undefined – using 12345"
#define CONFIG_ROBOT_UDP_PORT 12345
#endif

/* ─────────────────────────── LED definitions ──────────────────────────── */

#define WIFI_LED_GPIO GPIO_NUM_2

static inline void led_init(void)
{
    gpio_reset_pin(WIFI_LED_GPIO);
    gpio_set_direction(WIFI_LED_GPIO, GPIO_MODE_OUTPUT);
}

static inline void led_on(void)  { gpio_set_level(WIFI_LED_GPIO, 1); }
static inline void led_off(void) { gpio_set_level(WIFI_LED_GPIO, 0); }

static void led_blink(int times)
{
    for (int i = 0; i < times; ++i) {
        led_on();
        vTaskDelay(pdMS_TO_TICKS(500));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ────────────────────────── user-editable constants ───────────────────── */

#define WIFI_SSID1       CONFIG_ROBOT_WIFI_SSID1
#define WIFI_PASS1       CONFIG_ROBOT_WIFI_PASSWORD1
#define WIFI_SSID2       CONFIG_ROBOT_WIFI_SSID2
#define WIFI_PASS2       CONFIG_ROBOT_WIFI_PASSWORD2
#define DEST_IP_STR      CONFIG_ROBOT_PI_IP
#define DEST_UDP_PORT    CONFIG_ROBOT_UDP_PORT
#define MAXIMUM_RETRY    5      /* retries before switching SSID */

static const char *TAG = "wifi_udp";

/* ───────────────────── internal bookkeeping & flags ───────────────────── */

static EventGroupHandle_t s_wifi_evt_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static uint8_t  s_current_ap = 0; /* 0 → primary, 1 → secondary */
static int      s_retry_num  = 0;
static int      s_sock       = -1; /* UDP socket handle         */
static struct sockaddr_in s_dest_addr;

static const char *const s_ssid[2] = { WIFI_SSID1, WIFI_SSID2 };

/* ───────────────────────────── prototypes ─────────────────────────────── */

static void      wifi_event_handler(void *, esp_event_base_t, int32_t, void *);
static esp_err_t udp_socket_create(void);

/* ───────────────────────────── API ────────────────────────────────────── */

bool wifi_udp_ready(void)
{
    return (xEventGroupGetBits(s_wifi_evt_group) & WIFI_CONNECTED_BIT) &&
           (s_sock >= 0);
}

int wifi_udp_send(const char *payload, size_t len)
{
    if (!wifi_udp_ready()) return -1;

    int sent = sendto(s_sock, payload, len, 0,
                      (struct sockaddr *)&s_dest_addr,
                      sizeof(s_dest_addr));
    if (sent < 0) ESP_LOGW(TAG, "sendto errno %d", errno);
    return sent;
}

void wifi_udp_init(void)
{
    /* 1. Hardware: initialise LED and turn it ON (connecting) */
    led_init();
    led_on();

    /* 2. NVS (required by Wi-Fi) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* 3. Net-IF & default event loop */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    /* 4. Wi-Fi driver */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    s_wifi_evt_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    /* 5. First attempt → primary SSID */
    wifi_config_t wifi_cfg = { 0 };
    strcpy((char *)wifi_cfg.sta.ssid,     WIFI_SSID1);
    strcpy((char *)wifi_cfg.sta.password, WIFI_PASS1);
    wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* 6. Wait for connection (or failure) */
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_evt_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE,
        portMAX_DELAY);

    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Wi-Fi: unable to connect to any configured SSID");
        led_blink(5);  /* final failure indication */
        led_off();
        return;
    }

    /* 7. Create UDP socket */
    ESP_ERROR_CHECK(udp_socket_create());
    /* Success LED sequence already happened in the event handler */
}

/* ─────────────────────── event callback (private) ─────────────────────── */

static void wifi_event_handler(void *arg,
                               esp_event_base_t base,
                               int32_t          id,
                               void            *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {

        esp_wifi_connect();

    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {

        if (s_retry_num++ < MAXIMUM_RETRY) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "retry %d/%d on SSID %s",
                     s_retry_num, MAXIMUM_RETRY, s_ssid[s_current_ap]);
        } else {
            /* Switch to secondary network once retries exhausted */
            if (s_current_ap == 0) {
                wifi_config_t cfg = { 0 };
                strcpy((char *)cfg.sta.ssid,     WIFI_SSID2);
                strcpy((char *)cfg.sta.password, WIFI_PASS2);
                cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));

                s_current_ap = 1;
                s_retry_num  = 0;
                ESP_LOGW(TAG, "switching to secondary SSID");
                esp_wifi_connect();
            } else {
                /* Both SSIDs failed → give up */
                xEventGroupSetBits(s_wifi_evt_group, WIFI_FAIL_BIT);
                if (s_sock >= 0) { close(s_sock); s_sock = -1; }
                xEventGroupClearBits(s_wifi_evt_group, WIFI_CONNECTED_BIT);
                led_blink(5);   /* failure indication */
                led_off();
            }
        }

    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "IP " IPSTR " on SSID %s",
                 IP2STR(&event->ip_info.ip), s_ssid[s_current_ap]);

        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_evt_group, WIFI_CONNECTED_BIT);

        led_blink(3);  /* success indication */
        led_off();
    }
}

/* ─────────────────────── UDP socket helper (private) ──────────────────── */

static esp_err_t udp_socket_create(void)
{
    s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s_sock < 0) {
        ESP_LOGE(TAG, "socket() failed errno %d", errno);
        return ESP_FAIL;
    }

    s_dest_addr.sin_family      = AF_INET;
    s_dest_addr.sin_port        = htons(DEST_UDP_PORT);
    s_dest_addr.sin_addr.s_addr = inet_addr(DEST_IP_STR);

    ESP_LOGI(TAG, "UDP target → %s:%d", DEST_IP_STR, DEST_UDP_PORT);
    return ESP_OK;
}
