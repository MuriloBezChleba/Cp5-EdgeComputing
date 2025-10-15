# 🌐 Vinheria – Telemetria IoT com ESP32 (DHT11 + LDR) via MQTT

## 📌 Descrição do Projeto
Este projeto implementa um **nó IoT com ESP32** para leitura de **temperatura, umidade (DHT11)** e **luminosidade (LDR)**, publicando os dados via **MQTT**.  
Ele pode ser integrado a um **FIWARE IoT Agent/Orion** ou consumido por dashboards e apps externos (ex.: **MyMQTT**, Node-RED).

O firmware inclui **logs seriais**, reconexão automática de **Wi-Fi/MQTT** e tópicos separados para cada métrica, além de um tópico de **comandos** para acender/apagar o LED do ESP32.

---

## 👥 Integrantes
- Caio Marques Lins | RM: 559805  
- Murilo Gonzalez Bez Chleba | RM: 566199  
- Bernardo Lozório Gomes Y Gomes | RM: 564943 

---

## 🏗️ Arquitetura Proposta
```mermaid
flowchart LR
    A[ESP32<br/>DHT11 + LDR] -->|MQTT publish| B[Broker Mosquitto]
    C[App MyMQTT<br/>(ou Node-RED)] -->|subscribe| B
    B --> D[FIWARE IoT Agent<br/>(opcional)]
    D --> E[Orion / QuantumLeap<br/>(opcional)]
```

**Componentes:**
- **ESP32** → lê DHT11 (temperatura/umidade) e LDR (luminosidade).  
- **Broker MQTT (Mosquitto)** → recebe telemetria e repassa aos assinantes.  
- **MyMQTT / Node-RED** → consomem as mensagens e exibem/roteiam.  
- **FIWARE (opcional)** → IoT Agent mapeia atributos e Orion armazena/consulta.

---

## ⚙️ Tecnologias Utilizadas
- **ESP32** (Arduino Core)  
- **Arduino IDE** (ou PlatformIO)  
- **Bibliotecas:**  
  - `PubSubClient` (MQTT)  
  - `DHT sensor library` + `Adafruit Unified Sensor` (DHT11)  
- **Mosquitto (MQTT Broker)**  
- **MyMQTT (Android)** para testes rápidos de subscribe/publish

---

## 🚀 Como Executar o Projeto

### 1) Configurar o firmware (Arduino IDE)
No arquivo principal, ajuste as credenciais e o broker:

```cpp
// Wi-Fi (troque pelos seus dados)
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// Broker MQTT
const char* BROKER_MQTT = "seu_broker"; // seu broker
const int   BROKER_PORT = 1883;
const char* ID_MQTT     = "seu_id"; // ID único do cliente
```

**Pinos de hardware (conforme seu circuito):**
- LED onboard: `GPIO 2`  
- LDR (com divisor de tensão): **nó** → `GPIO 34 (ADC1)`, **topo** → `3V3`, **base** → `resistor 10k → GND`  
- DHT11: `GPIO 4`

> Instale as libs em: **Ferramentas → Gerenciador de Bibliotecas**  
> – `PubSubClient` (Nick O’Leary)  
> – `DHT sensor library` + `Adafruit Unified Sensor` (Adafruit)

### 2) Compilar e gravar
- Placa: **ESP32 Dev Module** (ou a sua)  
- Porta: a serial do seu ESP32  
- **Baud do Monitor Serial:** `115200`  

### 3) Iniciar/validar o Broker (Mosquitto)
```bash
# Linux/macOS
mosquitto -v

# Windows (serviço)
net start mosquitto
# ou
mosquitto.exe -v
```

### 4) Testar no MyMQTT (Android)
- **Conexão:** host `18.231.120.18`, porta `1883`, sem TLS, sem usuário/senha.  
- **Assinar os tópicos de telemetria:**
  - `/TEF/device001/attrs/luz`
  - `/TEF/device001/attrs/temp`
  - `/TEF/device001/attrs/umid`
  - (ou **coringa**: `/TEF/device001/attrs/#`)

Você verá mensagens como:
```
/TEF/device001/attrs/temp  ->  24.7
/TEF/device001/attrs/umid  ->  53.0
/TEF/device001/attrs/luz   ->  72
```

### 5) Enviar comandos (opcional)
Assine `TOPICO_SUBSCRIBE = /TEF/device001/cmd` no código.  
Para ligar/desligar o LED do ESP32, **publique** neste tópico:
- Payload `on` → liga LED (ESP confirma em `/TEF/device001/attrs` com `s|on`)  
- Payload `off` → desliga LED (confirma com `s|off`)

---

## 🧪 Tópicos MQTT (Resumo)

| Função                | Tópico                               | Payload (exemplo) |
|----------------------|--------------------------------------|-------------------|
| **Temperatura**      | `/TEF/device001/attrs/temp`          | `24.7`            |
| **Umidade**          | `/TEF/device001/attrs/umid`          | `53.0`            |
| **Luminosidade**     | `/TEF/device001/attrs/luz`           | `72`              |
| **Status/eco**       | `/TEF/device001/attrs`               | `s|on` / `s|off`  |
| **Comando LED (IN)** | `/TEF/device001/cmd`                 | `on` / `off`      |

> Obs.: seu código publica **valores simples** (números/texto), não JSON.  
> Se for integrar ao **FIWARE IoT Agent JSON**, você pode:  
> - manter tópicos separados e mapear cada `object_id` manualmente, ou  
> - alterar o publish para **um único tópico** com **payload JSON** (se desejar evoluir).

---

## 📊 Logs & Diagnóstico
- Abra o **Monitor Serial (115200)** para ver mensagens do tipo:
  - Conexão Wi-Fi/MQTT, erros de reconexão
  - Leituras de sensores e tópicos publicados
- Se aparecer caracteres estranhos: verifique a **taxa (baud)** do Monitor.  
- Timeouts MQTT costumam ocorrer com **Wi-Fi fraco** (RSSI < −70 dBm). Aproxime do AP ou use um hotspot para testar.

---


## 📌 Observações
- **LDR** precisa de **divisor de tensão** (3V3—LDR—(nó)—10k—GND; nó → GPIO34).  
- **GPIO34 é só entrada** (ok para ADC); evite pinos de **ADC2** enquanto o Wi-Fi está ativo.  
- Para ambientes com firewall, garanta a abertura da **porta TCP 1883** no broker.  
- Se planejar histórico e consultas, considere integrar com **FIWARE (IoT Agent → Orion → QuantumLeap)**.
