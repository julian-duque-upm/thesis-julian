# thesis-julian
Check if azure is receving information from the Gateway:
      az iot hub monitor-events --hub-name iothub-thesis-julian --device-id esp32-gateway

Check if the Pi is receving information from MQTT:
      mosquitto_sub -h localhost -t "robot/telemetry" -v
