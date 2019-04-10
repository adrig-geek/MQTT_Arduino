#include <SPI.h>
#include <Ethernet.h>

//MQTT
#include <PubSubClient.h>

//RFID
#include <SPI.h>
#include <MFRC522.h>

#define MQTT_PORT 1883

// Manual Assigned MAC
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Server's IP
IPAddress mqtt_server(172,16,2,153);

// Topics of MQTT
const char* topicTx = "test";
const char* topicRx = "test";
const char* topicRfid = "rfid";

MFRC522 rfid(53, 39);

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

  //RFID
  SPI.begin ();  
  rfid.PCD_Init(); 
}

void loop(){
  
  //TODO Implementar totes les possibles respostes de MQTT
  if (client.state() == -1){
    if(client.connect("Arduino Client", "joan", "gomez")){
          client.subscribe(topicRx);
          client.subscribe(topicRfid);
    }
    Serial.println("Arduino connected");
  }

  //RFID
  if (rfid.PICC_IsNewCardPresent()){
    if (rfid.PICC_ReadCardSerial()){
      
      String message = String(rfid.uid.uidByte[0]) + ":" + String(rfid.uid.uidByte[1]) + ":" + String(rfid.uid.uidByte[2]) + ":" + String(rfid.uid.uidByte[3]);
      Serial.println (message);
      char rfidCard[message.length() + 1];
      message.toCharArray(rfidCard, message.length() + 1);
      
      client.publish(topicRfid, rfidCard);
    }
  }
  
  // Tiempo entre envios (en ms)
  delay(500);

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
