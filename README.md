# Edge-to-Cloud IoT Robot Platform
Welcome to the repository for my Master’s Thesis project: a complete edge-to-cloud IoT architecture for real-time robot telemetry, analytics, and cloud integration using ESP32, Raspberry Pi 5, Docker, Node-RED, and Azure IoT Hub.

🚗 Project Overview
This project demonstrates a modular, scalable IoT system with:

ESP32 Robot Car: Real-time line-following, telemetry (RPM, IR sensors, state) via Wi-Fi UDP.

Raspberry Pi 5 (Edge Node): Runs Dockerized Mosquitto (MQTT broker) and Node-RED for data ingestion, analytics, dashboard, and MQTT bridging.

ESP32 Gateway: Bridges local MQTT telemetry to Azure IoT Hub using secure MQTT over TLS.

Azure IoT Hub: Receives, stores, and visualizes telemetry for cloud analytics.

🗂️ Repository Structure
iot-stack/
├── docker-compose.yml
├── mosquitto/
│   ├── config/
│   ├── data/
│   └── logs/
├── nodered/
│   ├── flows.json
│   ├── flows_cred.json
│   └── ...
├── README.md

🛠️ Quick Start
1. Clone the Repository
  git clone https://github.com/julian-duque-upm/thesis-julian.git
  cd thesis-julian/iot-stack
2. Set Up Docker & Docker Compose on Raspberry Pi
  sudo apt update && sudo apt install -y docker.io docker-compose
  sudo usermod -aG docker $USER
  # Log out and back in for group changes to take effect
3. Launch the Stack
  docker-compose up -d

4. Access Services
Node-RED Dashboard: http://<pi-ip>:1880

Mosquitto MQTT Broker: mqtt://<pi-ip>:1883

🏗️ System Architecture
graph TD
    ESP32_Robot --UDP--> Pi5
    Pi5 --Node-RED+Mosquitto--> ESP32_Gateway
    ESP32_Gateway --MQTT/TLS--> Azure_IoT_Hub
    Pi5 --Dashboard--> User

Robot ESP32: Sends telemetry via UDP to Pi.

Pi 5: UDP → Node-RED → MQTT (Mosquitto) → ESP32 Gateway.

ESP32 Gateway: Subscribes to MQTT, forwards to Azure IoT Hub.

Azure: Receives and stores telemetry for analysis.

📝 Features
Hardware & Firmware
Robot Car: Dual-motor, encoders, 4 IR sensors, ESP32 (C, PID control).

UDP Telemetry: High-frequency (200 Hz) data to Pi.

Edge Node (Raspberry Pi 5)
Dockerized Mosquitto: Local MQTT broker.

Node-RED: UDP ingestion, analytics, dashboard, MQTT bridge.

Dashboard: Live RPM gauges, IR sensor LEDs, car direction.

Cloud Integration
ESP32 Gateway: Secure MQTT bridge to Azure IoT Hub (TLS, SAS token).

Azure IoT Hub: Device registration, telemetry ingestion, CLI/Explorer monitoring.

Edge Analytics
Node-RED: Local event detection (e.g., robot stuck, abnormal RPM).

Alerts/Logs: Optionally generate local alerts or logs.

CI/CD
GitHub Actions: Automated Docker Compose validation, build, and deployment workflow.

Automated Deployment: SSH-based deployment to Raspberry Pi (optional).

📊 Node-RED Dashboard Example
RPM Gauges: Left/Right wheel speeds.

IR Sensor LEDs: Real-time line detection.

Car Direction: "Centered", "Left", "Right", "Lost".

🧩 How to Check Data Flow
Local MQTT
mosquitto_sub -h localhost -t "robot/telemetry" -v

mosquitto_sub -h localhost -t "robot/telemetry" -v
az iot hub monitor-events --hub-name iothub-thesis-julian --device-id esp32-gateway

⚙️ Configuration Details
docker-compose.yml
version: "3.8"
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: thesis-mosquitto
    restart: unless-stopped
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto/config:/mosquitto/config
      - ./mosquitto/data:/mosquitto/data
      - ./mosquitto/logs:/mosquitto/logs
    networks: [thesis-net]

  nodered:
    image: nodered/node-red:latest
    container_name: thesis-nodered
    restart: unless-stopped
    ports:
      - "1880:1880"
      - "12345:12345/udp"
    volumes:
      - ./nodered:/data
    environment:
      - TZ=Europe/Madrid
    depends_on:
      - mosquitto
    networks: [thesis-net]

networks:
  thesis-net:
    driver: bridge
🧑‍💻 Development & CI/CD
Local Git Workflow: All project files version-controlled and synced with GitHub.

CI: GitHub Actions workflow validates Docker Compose config on each push.

CD: Optional SSH deployment—auto-pulls and restarts stack on the Pi after each push.

.gitignore: Excludes runtime data, logs, and node_modules.

🏁 How to Replicate the System
Assemble the robot car with ESP32, encoders, IR sensors.

Flash ESP32 with PID firmware and UDP telemetry.

Set up Raspberry Pi 5 with Docker, Mosquitto, Node-RED.

Configure Node-RED flows for UDP ingest, MQTT bridge, dashboard.

Deploy ESP32 Gateway with MicroPython or Arduino, subscribe to MQTT, forward to Azure IoT Hub using SAS token.

Register devices in Azure IoT Hub and monitor telemetry.

📚 Documentation & Evidence
Screenshots: Node-RED flows, dashboards, Azure CLI output.

Config Files: docker-compose.yml, Mosquitto config, Node-RED flows.

Code: ESP32 C firmware, MicroPython/Arduino scripts for gateway.

Replication Guide: Step-by-step instructions included in this README.

🖼️ Architecture Diagram
See the repo's /images directory for architecture and data flow diagrams.

🧪 Results & Evaluation
Latency & Reliability: Measured using timestamps at each stage (robot, Pi, Azure).

Edge Analytics: Local stuck detection, abnormal RPM alerts.

Privacy: No video/images—telemetry only.

CI/CD: Automated build, validation, and deployment.

📝 References
Official MicroPython MQTT Library

Azure IoT Hub Documentation

Node-RED Docs

Mosquitto MQTT

📋 To-Do / Roadmap
 Robot car hardware & firmware

 Local edge pipeline (UDP/MQTT)

 Node-RED dashboard

 MQTT bridge to ESP32 Gateway

 ESP32 Gateway → Azure IoT Hub

 Edge analytics (Node-RED)

 CI/CD for containers

 Documentation & evidence

 Architecture diagrams

 Results & discussion

 Final thesis report

🤝 Contributing
Contributions, suggestions, and issues are welcome! Please open an issue or pull request.

📄 License
This project is released under the MIT License.

👤 Author
Julian Duque
Universidad Politécnica de Madrid
julian.duque@alumnos.upm.es





