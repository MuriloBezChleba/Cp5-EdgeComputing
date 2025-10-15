 
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
 
// =========================== CONFIGURAÇÕES DE USUÁRIO ===========================
 
// --- Configurações Wi-Fi ---
const char* WIFI_SSID = "iPhone do batata";      
const char* WIFI_PASSWORD = "batata4289";        
 
// --- Configurações do Broker MQTT ---
const char* BROKER_MQTT = "18.231.120.18";  
const int BROKER_PORT = 1883;                
const char* ID_MQTT = "fiware_esp32_001";     
 
// --- Tópicos MQTT ---
const char* TOPICO_SUBSCRIBE = "/TEF/device001/cmd";
const char* TOPICO_PUBLISH_ATTRS = "/TEF/device001/attrs";      
const char* TOPICO_PUBLISH_LDR = "/TEF/device001/attrs/luz";    
const char* TOPICO_PUBLISH_TEMP = "/TEF/device001/attrs/temp";  
const char* TOPICO_PUBLISH_UMID = "/TEF/device001/attrs/umid";  
 
// --- Pinos ---
#define PIN_LED 2        
#define PIN_LDR 34       
#define PIN_DHT 4        
#define DHTTYPE DHT11
 
// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================
WiFiClient espClient;
PubSubClient MQTT(espClient);
DHT dht(PIN_DHT, DHTTYPE);
 
char EstadoSaida = '0';
unsigned long lastMsg = 0; 
int intervalo = 5000;      
 
// ============================================================================
// INICIALIZAÇÕES
// ============================================================================
void initSerial() {
  Serial.begin(115200);
  Serial.println("\n===== INICIALIZAÇÃO SERIAL =====");
}
 
void initWiFi() {
  Serial.println("Conectando-se ao Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
}
 
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}
 
void initHardware() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  dht.begin();
}
 
// ============================================================================
// CALLBACK MQTT - RECEBE COMANDOS
// ============================================================================
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
 
  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);
 

  if (msg.equalsIgnoreCase("on")) {
    digitalWrite(PIN_LED, HIGH);
    EstadoSaida = '1';
    MQTT.publish(TOPICO_PUBLISH_ATTRS, "s|on");
  } else if (msg.equalsIgnoreCase("off")) {
    digitalWrite(PIN_LED, LOW);
    EstadoSaida = '0';
    MQTT.publish(TOPICO_PUBLISH_ATTRS, "s|off");
  }
}
 
// ============================================================================
// RECONEXÕES
// ============================================================================
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Conectando ao broker MQTT em ");
    Serial.print(BROKER_MQTT);
    Serial.print("... ");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.print("Falha (rc=");
      Serial.print(MQTT.state());
      Serial.println("). Tentando novamente em 2s...");
      delay(2000);
    }
  }
}
 
void verificaConexoes() {
  if (!MQTT.connected()) reconnectMQTT();
  if (WiFi.status() != WL_CONNECTED) initWiFi();
}
 
// ============================================================================
// PUBLICAÇÃO DE DADOS
// ============================================================================
void publicarSensores() {
  // --- Leitura LDR ---
  int valorLDR = analogRead(PIN_LDR);
  int luminosidade = map(valorLDR, 0, 4095, 0, 100);
 
  // --- Leitura DHT11 ---
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
 
  // Verificação de erro DHT
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o sensor DHT11!");
    return;
  }
 
  // --- Publicações MQTT ---
  String msgLuz = String(luminosidade);
  String msgTemp = String(temperatura, 1);
  String msgUmid = String(umidade, 1);
 
  MQTT.publish(TOPICO_PUBLISH_LDR, msgLuz.c_str());
  MQTT.publish(TOPICO_PUBLISH_TEMP, msgTemp.c_str());
  MQTT.publish(TOPICO_PUBLISH_UMID, msgUmid.c_str());
 
  Serial.println("=== Dados enviados ao Broker ===");
  Serial.print("Luminosidade: "); Serial.println(msgLuz);
  Serial.print("Temperatura: "); Serial.println(msgTemp);
  Serial.print("Umidade: "); Serial.println(msgUmid);
  Serial.println("================================");
}
 
// ============================================================================
// LOOP PRINCIPAL
// ============================================================================
void setup() {
  initSerial();
  initHardware();
  initWiFi();
  initMQTT();
}
 
void loop() {
  verificaConexoes();
  MQTT.loop();
 
  unsigned long agora = millis();
  if (agora - lastMsg > intervalo) {
    lastMsg = agora;
    publicarSensores();
  }
}