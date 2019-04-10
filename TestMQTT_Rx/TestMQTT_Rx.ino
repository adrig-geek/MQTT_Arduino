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
  Serial.begin(115200);
  Serial.print("Configuring ...\n");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Failed");
  }
  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(callback);
}

void loop(){
  //TODO Implementar tots els possibles casos
  if (client.state() == -1){
    if(client.connect("Arduino Client")){
          client.subscribe(topicName);
    }
    Serial.println("Arduino connected");
  }

  // Envio
  //Serial.print("Sending ...\n");
  //client.publish(topicName, "ARDUINO");
  
  // Tiempo entre envios (en ms)
  delay(5000);

  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
