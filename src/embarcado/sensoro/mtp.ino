#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Configurações WiFi e MQTT ---
const char* ssid = "FAMILIAPAVAN";
const char* password = "virus2424";
const char* mqtt_server = "192.168.3.55";

WiFiClient esp_client;
PubSubClient client(esp_client);

// --- Definições de pinos para NodeMCU ESP8266 ---
#define PIN_CARREGADOR    D6
#define PIN_DESCARREGADOR D7
#define PIN_MEDIDOR       A0

// --- Constantes para medição ---
#define CHARGED        1023
#define TIME_ON_OFF    10
#define DELTA_T        1000
#define THRESHOLD_UP   900
#define THRESHOLD_DOWN 10
#define RESISTOR_OHM   1000

// Macros para controle dos sinais dos pinos digitais
#define ON(p)  digitalWrite(p, HIGH)
#define OFF(p) digitalWrite(p, LOW)

// --- Máquina de estados ---
enum Estado {
  CARREGAR = 0,
  DESCARREGAR,
  MEDIR,
  ESPERAR,
  NUMERO_DE_ESTADOS
};

typedef Estado (*FuncaoDeEstado)();

// Variáveis globais
Estado estado_atual = CARREGAR;
FuncaoDeEstado maquina_de_estados[NUMERO_DE_ESTADOS];
float ultima_capacitancia = 0;

// --- Funções dos estados ---
Estado carregar() {
  Serial.println("Carregando o capacitor...");
  OFF(PIN_DESCARREGADOR);
  delay(TIME_ON_OFF);
  ON(PIN_CARREGADOR);
  while (analogRead(PIN_MEDIDOR) < CHARGED) {}
  return DESCARREGAR;
}

Estado descarregar() {
  Serial.println("Descarregando o capacitor...");
  OFF(PIN_CARREGADOR);
  delay(TIME_ON_OFF);
  ON(PIN_DESCARREGADOR);
  return MEDIR;
}

Estado medir() {
  float leitura = analogRead(PIN_MEDIDOR);
  unsigned long dt;
  unsigned long t0, t1;

  // Espera atingir THRESHOLD_UP
  t0 = millis();
  while (leitura >= THRESHOLD_UP) {
    leitura = analogRead(PIN_MEDIDOR);
    if (millis() - t0 > 2000) break; // timeout de segurança
  }
  t0 = millis();
  while (leitura >= THRESHOLD_DOWN) {
    leitura = analogRead(PIN_MEDIDOR);
    if (millis() - t0 > 2000) break; // timeout de segurança
  }
  t1 = millis();
  dt = t1 - t0;
  float capacitance = dt / ((float)RESISTOR_OHM);
  ultima_capacitancia = capacitance;
  Serial.print("Capacitancia medida: ");
  Serial.println(capacitance);
  return ESPERAR;
}

Estado esperar() {
  Serial.println("Esperando para a próxima medição...");
  delay(DELTA_T);
  return CARREGAR;
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  pinMode(PIN_CARREGADOR, OUTPUT);
  pinMode(PIN_DESCARREGADOR, OUTPUT);
  pinMode(PIN_MEDIDOR, INPUT);

  maquina_de_estados[CARREGAR] = carregar;
  maquina_de_estados[DESCARREGAR] = descarregar;
  maquina_de_estados[MEDIR] = medir;
  maquina_de_estados[ESPERAR] = esperar;

  // Garantir que o capacitor está descarregado
  maquina_de_estados[DESCARREGAR]();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  client.setServer(mqtt_server, 1883);
}

// --- Loop principal ---
void loop() {
  // Garantir conexão MQTT
  while (!client.connected()) {
    if (client.connect("NodeMCU")) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 3 segundos");
      delay(3000);
    }
  }
  client.loop();

  // Máquina de estados
  estado_atual = maquina_de_estados[estado_atual]();

  // Publica valor da capacitância medida
  char payload[32];
  dtostrf(ultima_capacitancia, 6, 2, payload);
  client.publish("dados/sensoro", payload);

  delay(2000); // Pequeno delay para evitar flooding
}
