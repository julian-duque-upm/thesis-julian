idf_component_register(
    SRCS
        "main.c"
        "motor_driver.c"
        "encoder.c"
        "ir_sensors.c"
        "wifi_udp.c"
        "pid.c"
    INCLUDE_DIRS
        "."                 # headers live alongside sources
    REQUIRES               # public dependencies (linked automatically)
        driver             # GPIO, LEDC
        esp_wifi           # Wi-Fi STA mode
        esp_netif          # TCP/IP stack glue
        esp_event          # event loop used by Wi-Fi
        nvs_flash          # Wi-Fi credentials persistence
        lwip               # BSD sockets / UDP
        freertos           # tasks & timing utilities
        esp_timer          # high-resolution timestamps
    PRIV_REQUIRES          # private deps (internal to this component)
        esp_driver_pcnt    # hardware quadrature encoder
)
