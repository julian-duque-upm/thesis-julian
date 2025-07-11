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
 - az iot hub generate-sas-token --hub-name iothub-thesis-julian --device-id esp32-gateway --login "HostName=iothub-thesis-julian.azure-devices.net;DeviceId=esp32-gateway;SharedAccessKey=LMtD5lKZZwXrCkK3QIOpo6nJNGEfyd+sXWiamV/3hX0=" --duration 2592000
