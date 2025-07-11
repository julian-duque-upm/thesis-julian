# Edge-to-Cloud IoT Robot Platform

Welcome to the repository for my Master’s Thesis project at **Universidad Politécnica de Madrid**: a modular, scalable edge-to-cloud IoT architecture for real-time robot telemetry, analytics, and cloud integration using **ESP32**, **Raspberry Pi 5**, **Docker**, **Node-RED**, and **Azure IoT Hub**.

<p align="center">
  <img src="images/architecture-diagram.png" alt="System Architecture" width="600"/>
</p>

---

## 🚗 Project Overview

This project demonstrates a complete IoT system for a line-following robot car, integrating edge computing and cloud services for real-time telemetry and analytics. Key components include:

- **ESP32 Robot Car**: Real-time line-following with telemetry (RPM, IR sensors, state) sent via Wi-Fi UDP.
- **Raspberry Pi 5 (Edge Node)**: Runs Dockerized **Mosquitto** (MQTT broker) and **Node-RED** for data ingestion, analytics, dashboard, and MQTT bridging.
- **ESP32 Gateway**: Bridges local MQTT telemetry to **Azure IoT Hub** using secure MQTT over TLS.
- **Azure IoT Hub**: Receives, stores, and visualizes telemetry for cloud-based analytics.

---

## 🗂️ Repository Structure

```bash
iot-stack/
├── docker-compose.yml         # Docker Compose configuration for Mosquitto and Node-RED
├── mosquitto/                # Mosquitto MQTT broker configuration and data
│   ├── config/               # Configuration files
│   ├── data/                 # Persistent data storage
│   └── logs/                 # Log files
├── nodered/                  # Node-RED configuration and flows
│   ├── flows.json            # Node-RED flow configuration
│   ├── flows_cred.json       # Node-RED credentials
│   └── ...                   # Additional Node-RED files
├── images/                   # Architecture and data flow diagrams
├── README.md                 # Project documentation (this file)
```

---

## 🛠️ Quick Start

Get the system up and running on your Raspberry Pi 5 with the following steps:

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/julian-duque-upm/thesis-julian.git
   cd thesis-julian/iot-stack
   ```

2. **Install Docker & Docker Compose on Raspberry Pi**:
   ```bash
   sudo apt update && sudo apt install -y docker.io docker-compose
   sudo usermod -aG docker $USER
   ```
   Log out and back in to apply group changes.

3. **Launch the Stack**:
   ```bash
   docker-compose up -d
   ```

4. **Access Services**:
   - **Node-RED Dashboard**: `http://<Pi_IP>:1880`
   - **Mosquitto MQTT Broker**: `mqtt://<Pi_IP>:1883`

---

## 🏗️ System Architecture

The architecture integrates edge and cloud components for seamless telemetry flow:

```mermaid
graph TD
    ESP32_Robot[ESP32 Robot Car] -- UDP --> Pi5[Raspberry Pi 5]
    Pi5 -- Node-RED + Mosquitto --> ESP32_Gateway[ESP32 Gateway]
    ESP32_Gateway -- MQTT/TLS --> Azure_IoT_Hub[Azure IoT Hub]
    Pi5 -- Dashboard --> User[User]
```

- **ESP32 Robot Car**: Sends telemetry (RPM, IR sensor data) via UDP to the Raspberry Pi.
- **Raspberry Pi 5**: Processes UDP data in Node-RED, publishes to Mosquitto MQTT, and hosts a dashboard.
- **ESP32 Gateway**: Subscribes to MQTT topics and forwards telemetry to Azure IoT Hub.
- **Azure IoT Hub**: Stores and visualizes telemetry for cloud-based analytics.

---

## 📝 Features

### Hardware & Firmware
- **Robot Car**: Dual-motor setup with encoders, 4 IR sensors, and ESP32 programmed in C with PID control.
- **UDP Telemetry**: High-frequency (200 Hz) data transmission to the Raspberry Pi.

### Edge Node (Raspberry Pi 5)
- **Dockerized Mosquitto**: Local MQTT broker for reliable messaging.
- **Node-RED**: Handles UDP ingestion, analytics, MQTT bridging, and a live dashboard.
- **Dashboard**: Displays real-time RPM gauges, IR sensor LEDs, and car direction ("Centered", "Left", "Right", "Lost").

### Cloud Integration
- **ESP32 Gateway**: Securely bridges MQTT telemetry to Azure IoT Hub using TLS and SAS tokens.
- **Azure IoT Hub**: Device registration, telemetry ingestion, and monitoring via Azure CLI or IoT Explorer.

### Edge Analytics
- **Node-RED**: Detects events like "robot stuck" or abnormal RPM and generates local alerts/logs.

### CI/CD
- **GitHub Actions**: Validates Docker Compose configs, builds, and deploys the stack.
- **Automated Deployment**: Optional SSH-based deployment to Raspberry Pi on push.

---

## 📊 Node-RED Dashboard Example

- **RPM Gauges**: Real-time left/right wheel speeds.
- **IR Sensor LEDs**: Visualizes line detection status.
- **Car Direction**: Displays "Centered", "Left", "Right", or "Lost" based on IR sensor data.

<p align="center">
  <img src="images/nodered-dashboard.png" alt="Node-RED Dashboard" width="600"/>
</p>

---

## ⚙️ Configuration Details

Below is the `docker-compose.yml` configuration for the edge node services:

<xaiArtifactInner>
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
    networks:
      - thesis-net

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
    networks:
      - thesis-net

networks:
  thesis-net:
    driver: bridge
</xaiArtifactInner>

---

## 🧑‍💻 Development & CI/CD

- **Local Git Workflow**: All project files are version-controlled and synced with GitHub.
- **CI**: GitHub Actions validates `docker-compose.yml` on each push.
- **CD**: Optional SSH-based deployment to auto-pull and restart the stack on the Raspberry Pi.
- **.gitignore**: Excludes runtime data, logs, and `node_modules`.

---

## 🏁 How to Replicate the System

1. **Assemble the Robot Car**: Build with ESP32, dual motors, encoders, and 4 IR sensors.
2. **Flash ESP32 Firmware**: Program the robot ESP32 with C firmware for PID control and UDP telemetry.
3. **Set Up Raspberry Pi 5**: Install Docker, Mosquitto, and Node-RED.
4. **Configure Node-RED**: Set up flows for UDP ingestion, MQTT bridging, and dashboard.
5. **Deploy ESP32 Gateway**: Use MicroPython or Arduino to subscribe to MQTT and forward telemetry to Azure IoT Hub with SAS token.
6. **Register Devices in Azure IoT Hub**: Monitor telemetry using Azure CLI or IoT Explorer.

---

## 🧪 Results & Evaluation

- **Latency & Reliability**: Measured via timestamps at each stage (robot → Pi → Azure).
- **Edge Analytics**: Local detection of "robot stuck" and abnormal RPM with alerts.
- **Privacy**: Telemetry-only system, no video or images.
- **CI/CD**: Automated validation, build, and deployment via GitHub Actions.

---

## 📚 Documentation & Evidence

- **Screenshots**: Node-RED flows, dashboards, and Azure CLI output in `/images`.
- **Config Files**: `docker-compose.yml`, Mosquitto config, Node-RED flows.
- **Code**: ESP32 C firmware, MicroPython/Arduino scripts for the gateway.
- **Replication Guide**: Detailed steps in this README.

---

## 🖼️ Architecture Diagram

See the `/images` directory for architecture and data flow diagrams.

---

## 🧪 How to Check Data Flow

- **Local MQTT**:
  ```bash
  mosquitto_sub -h localhost -t "robot/telemetry" -v
  ```

- **Azure IoT Hub**:
  ```bash
  az iot hub monitor-events --hub-name iothub-thesis-julian --device-id esp32-gateway
  ```

---

## 📝 References

- [MicroPython MQTT Library](https://micropython.org/)
- [Azure IoT Hub Documentation](https://docs.microsoft.com/azure/iot-hub/)
- [Node-RED Documentation](https://nodered.org/docs/)
- [Mosquitto MQTT](https://mosquitto.org/)

---

## 📋 To-Do / Roadmap

- [x] Robot car hardware & firmware
- [x] Local edge pipeline (UDP/MQTT)
- [x] Node-RED dashboard
- [x] MQTT bridge to ESP32 Gateway
- [x] ESP32 Gateway → Azure IoT Hub
- [x] Edge analytics in Node-RED
- [x] CI/CD for containers
- [x] Documentation & evidence
- [x] Architecture diagrams
- [ ] Results & discussion
- [ ] Final thesis report

---

## 🤝 Contributing

Contributions, suggestions, and issues are welcome! Please open an issue or submit a pull request.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

## 👤 Author

**Julian Duque**  
Universidad Politécnica de Madrid  
📧 [julian.duque@alumnos.upm.es](mailto:julian.duque@alumnos.upm.es)