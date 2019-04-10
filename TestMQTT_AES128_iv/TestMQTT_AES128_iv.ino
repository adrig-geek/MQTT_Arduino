//AES
#include <AESLib.h>

//Ethernet
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

//AES Key
uint8_t key256[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
uint8_t key128[] = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5};
//uint8_t key128[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
uint8_t iv[]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Topics of MQTT
const char* topicTx = "test";
const char* topicRx = "test";
const char* topicRfid = "rfid";

MFRC522 rfid(53, 39);

EthernetClient ethClient;
PubSubClient client(ethClient);

void setup(){
  Serial.begin(57600);
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
      while (message.length() < 16){
        message = message + " ";
      }
      message[15] = '\0';
      char rfidCard[message.length()];
      message.toCharArray(rfidCard, message.length() + 1);

      Serial.print("READED -> ");
      Serial.println(rfidCard);
      aes128_cbc_enc(key128, iv, rfidCard, sizeof(rfidCard));
      Serial.print("encrypted:");
      Serial.println(rfidCard);
      client.publish(topicRfid, rfidCard); //Encrypted
      //client.publish(topicRfid, rfidCard); //Decrypted
    }
  }
  
  // Tiempo entre envios (en ms)
  delay(500);

  
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  char aux[16];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    aux[i] = (char)payload[i];
  }
  Serial.println();
  aes128_cbc_dec(key128, iv, aux, sizeof(aux));
  Serial.print("DECRYPTED -> ");
  Serial.println(aux);
}
