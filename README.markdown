<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
</head>
<body>

  <h1>📦 Autonomous IoT Robot System – Privacy-Focused Edge-to-Cloud Architecture</h1>
  <p>Este repositorio contiene la implementación completa de un sistema robótico autónomo modular y respetuoso con la privacidad, construido con ESP32, Raspberry Pi 5, Node-RED y Azure IoT Hub. El sistema se enfoca en sensores no visuales, telemetría en tiempo real e integración híbrida edge-cloud.</p>

  <h2>📐 System Architecture</h2>
  <p>La plataforma sigue una arquitectura de tres niveles:</p>
  <pre>
[Robot Layer (ESP32)] --> [Edge Node (Raspberry Pi 5)] --> [Cloud (Azure IoT Hub)]
  </pre>
  <ul>
    <li><strong>Robot Layer:</strong> Robot autónomo basado en ESP32, equipado con sensores IR, codificadores y un controlador PID. Envía telemetría en tiempo real vía UDP.</li>
    <li><strong>Edge Layer:</strong> Raspberry Pi 5 ejecuta Docker con Node-RED y Mosquitto para analizar, visualizar y enrutar la telemetría vía MQTT.</li>
    <li><strong>Cloud Layer:</strong> Un segundo ESP32 actúa como gateway hacia Azure IoT Hub. Los datos se almacenan en Blob Storage, con soporte futuro para Stream Analytics.</li>
  </ul>

  <h2>⚙️ Features</h2>
  <ul>
    <li>✅ Robot seguidor de línea basado en PID</li>
    <li>✅ Diseño modular de sensores basados en GPIO</li>
    <li>✅ Telemetría en tiempo real a 200Hz vía UDP</li>
    <li>✅ Panel local de Node-RED</li>
    <li>✅ Telemetría vía MQTT reenviada a Azure</li>
    <li>✅ Servicios edge dockerizados</li>
    <li>✅ Pipeline de despliegue CI/CD con GitHub Actions</li>
  </ul>

  <h2>🔧 Hardware</h2>
  <table border="1" cellpadding="5">
    <thead>
      <tr>
        <th>Componente</th>
        <th>Cant.</th>
        <th>Descripción</th>
      </tr>
    </thead>
    <tbody>
      <tr><td>ESP32 DevKit v1</td><td>2</td><td>MCU del robot + Gateway</td></tr>
      <tr><td>Raspberry Pi 5 (8GB)</td><td>1</td><td>Nodo de computación edge</td></tr>
      <tr><td>Sensores IR (TCRT5000)</td><td>4</td><td>Seguimiento de línea</td></tr>
      <tr><td>Encoders rotatorios</td><td>2</td><td>Feedback de ruedas</td></tr>
      <tr><td>Driver de motor L298N</td><td>1</td><td>Control en puente H</td></tr>
      <tr><td>Motores DC + Ruedas</td><td>2</td><td>Sistema de tracción</td></tr>
      <tr><td>Baterías Li-ion 18650</td><td>4</td><td>Alimentación</td></tr>
      <tr><td>Chasis + Monturas</td><td>—</td><td>Fabricado en laboratorio, impresión 3D</td></tr>
    </tbody>
  </table>

  <h2>📁 Repository Structure</h2>
  <pre>
├── firmware/
│   ├── main.c
│   ├── ir_sensors.c
│   ├── motor_driver.c
│   ├── encoder.h
│   ├── pid.c
│   └── wifi_udp.c
│
├── edge/
│   ├── docker-compose.yml
│   └── node-red-flow.json
│
├── cloud/
│   ├── micropython_gateway.py
│   └── azure_device_config.json
│
├── robocar/
│   └── (componentes específicos del robot)
│
├── gateway/
│   └── (código específico del gateway ESP32)
│
├── .github/
│   └── workflows/
│       └── deploy.yml
│
└── README.md
  </pre>

  <h2>🚀 Deployment Instructions</h2>
  <ol>
    <li><strong>Flash Robot Firmware</strong>
      <pre><code>
idf.py build
idf.py -p /dev/ttyUSB0 flash
      </code></pre>
    </li>
    <li><strong>Start Edge Services on Raspberry Pi</strong>
      <pre><code>
cd edge/
docker-compose up -d
      </code></pre>
    </li>
    <li><strong>Push Telemetry to Azure (ESP32 Gateway)</strong>
      <ul>
        <li>Flashea <code>micropython_gateway.py</code> usando ampy o rshell</li>
        <li>Configura las credenciales de Azure en <code>secrets.py</code></li>
      </ul>
    </li>
  </ol>

  <h2>🔍 Diagnostic & Access Commands</h2>
  <ul>
    <li><strong>Verifica si Azure está recibiendo datos del gateway:</strong>
      <pre><code>az iot hub monitor-events --hub-name iothub-thesis-julian --device-id esp32-gateway</code></pre>
    </li>
    <li><strong>Verifica si la Raspberry Pi recibe datos por MQTT:</strong>
      <pre><code>mosquitto_sub -h localhost -t "robot/telemetry" -v</code></pre>
    </li>
    <li><strong>Conéctate a la Raspberry Pi vía SSH y accede a Node-RED:</strong>
      <pre><code>ssh -L 1880:127.0.0.1:1880 jdapi@192.168.1.143</code></pre>
    </li>
    <li><strong>Login en Azure CLI:</strong>
      <pre><code>az login</code></pre>
    </li>
    <li><strong>Generar SAS Token válido por 30 días:</strong>
      <pre><code>az iot hub generate-sas-token --device-id esp32-gateway --hub-name iothub-thesis-julian --duration 2592000</code></pre>
    </li>
  </ul>

  <h2>🔐 Privacy by Design</h2>
  <ul>
    <li>❌ Sin cámaras ni micrófonos</li>
    <li>❌ No se almacena PII</li>
    <li>✅ Toda la telemetría es anónima y cifrada</li>
    <li>✅ Cumple con principios de GDPR y CCPA desde el diseño</li>
  </ul>

</body>
</html>
