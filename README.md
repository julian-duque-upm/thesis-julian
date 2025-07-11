# thesis-julian
Check if azure is receving information from the Gateway:
 - az iot hub monitor-events --hub-name iothub-thesis-julian --device-id esp32-gateway

Check if the Pi is receving information from MQTT:
 - mosquitto_sub -h localhost -t "robot/telemetry" -v

Connect to the pi through ssh:
 - ssh -L 1880:127.0.0.1:1880 jdapi@192.168.1.143

Login to azure:
 - az login

Generate the SAS Token with 30 days:
 - az iot hub generate-sas-token --device-id esp32-gateway --hub-name iothub-thesis-julian --duration 2592000

<img width="970" height="694" alt="image" src="https://github.com/user-attachments/assets/89a60435-5dae-45e7-814f-072239edc7c9" />

<img width="1731" height="765" alt="image" src="https://github.com/user-attachments/assets/7d948dc9-0df2-4fb6-9edc-1b00f097c71f" />

