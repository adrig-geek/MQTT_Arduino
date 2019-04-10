#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define MQTT_PORT 1883

// Manual Assigned MAC
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Server's IP
IPAddress mqtt_server(172,16,2,153);

// Topics of MQTT
const char* topicName = "test";

EthernetClient ethClient;
PubSubClient client(ethClient);

void setup(){
  Serial.begin(9600);
  Serial.print("Configuring ...\n");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Failed");
  }
  client.setServer(mqtt_server, MQTT_PORT);
}

void loop(){
  //TODO Implementar tots els possibles casos
  if (client.state() == -1){
    client.connect("Arduino Client");
    Serial.println("Client connected");
  }

  // Envio
  Serial.print("Sending ...\n");
  client.publish(topicName, "test");
  
  // Tiempo entre envios (en ms)
  delay(5000);
}
