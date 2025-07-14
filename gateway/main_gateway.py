import network
import time
import urequests
import json
from machine import Pin
from umqtt.simple import MQTTClient

# ==== USER SETTINGS ====
WIFI_SSID = 'DIGIFIBRA-X22D'
WIFI_PASSWORD = 'hYYYt32Ak2'

MQTT_BROKER = '192.168.1.143'
MQTT_PORT = 1883
MQTT_TOPIC = b'robot/telemetry'

IOTHUB_NAME = 'iothub-thesis-julian'
DEVICE_ID = 'esp32-gateway'
SAS_TOKEN = 'SharedAccessSignature sr=iothub-thesis-julian.azure-devices.net%2Fdevices%2Fesp32-gateway&sig=wcJsAXQLrbB7sVU0RvbrGgEySl32DUC80OpgGoE0pe4%3D&se=1752250242'
HEADERS = {
    'Authorization': SAS_TOKEN,
    'Content-Type': 'application/json'
}
URL = f'https://{IOTHUB_NAME}.azure-devices.net/devices/{DEVICE_ID}/messages/events?api-version=2021-04-12'

# ==== LED SETUP ====
led = Pin(2, Pin.OUT)

def led_on():
    led.value(1)

def led_off():
    led.value(0)

def led_blink(times, interval=0.3):
    for _ in range(times):
        led_off()
        time.sleep(interval)
        led_on()
        time.sleep(interval)

# ==== 1. Connect to Wi-Fi ====
def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    led_on()
    if not wlan.isconnected():
        print('Connecting to Wi-Fi...')
        wlan.connect(WIFI_SSID, WIFI_PASSWORD)
        timeout = 10
        start = time.time()
        while not wlan.isconnected() and (time.time() - start) < timeout:
            time.sleep(1)
        if not wlan.isconnected():
            raise Exception("Wi-Fi connection failed")
    print('Wi-Fi connected:', wlan.ifconfig())
    led_blink(2)
    led_on()

# ==== 2. MQTT Callback for Local Broker ====
def sub_cb(topic, msg):
    print(f'Received from robot: topic={topic}, msg={msg}')
    try:
        payload = json.loads(msg)
        send_to_azure(payload)
    except Exception as e:
        print('Failed to process or forward to Azure:', e)
        led_blink(3, 0.1)

# ==== 3. Send to Azure IoT Hub via HTTP ====
def send_to_azure(payload):
    print(f"Sending to Azure: {payload}")
    try:
        res = urequests.post(URL, headers=HEADERS, json=payload)
        print(f"Azure response: {res.status_code}")
        if res.status_code == 204:
            led_on()
        else:
            led_blink(3, 0.1)
        res.close()
    except Exception as e:
        print(f"Azure send failed: {e}")
        led_blink(3, 0.1)

# ==== 4. Main Loop ====
def main():
    connect_wifi()
    client = MQTTClient(client_id="esp32-gateway", server=MQTT_BROKER, port=MQTT_PORT)
    client.set_callback(sub_cb)
    try:
        client.connect()
        print(f"Connected to MQTT broker at {MQTT_BROKER}")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to {MQTT_TOPIC}")
        while True:
            client.check_msg()
            time.sleep(0.1)
    except Exception as e:
        print(f"Error in main loop: {e}")
        led_blink(5, 0.1)
        raise
    finally:
        client.disconnect()

# ==== 5. Run the Program ====
try:
    main()
except KeyboardInterrupt:
    print('Program interrupted')
except Exception as e:
    print(f'Program error: {e}')
finally:
    led_off()