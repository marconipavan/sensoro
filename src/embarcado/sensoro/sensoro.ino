#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "FAMILIAPAVAN";
const char* password = "virus2424";
const char* mqtt_server = "192.168.3.55"; // ip a | grep inet

WiFiClient esp_client;
PubSubClient client(esp_client);

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  client.setServer(mqtt_server, 1883);
  client.setCallback([](char* topic, byte* payload, unsigned int length){
    Serial.println("Mensagem rece");
  });

}

void loop() {

  while(!client.connected()){
    if(client.connect("NodeMCU")){
      Serial.println("Conectado com sucesso!");
    }
    else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 3 segundos");
      delay(3000);
    }
  }
  client.loop();

  client.publish("dados/sensoro", "amoeba");
  delay(2000);
}
