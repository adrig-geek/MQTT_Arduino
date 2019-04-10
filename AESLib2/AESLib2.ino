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

// Topics of MQTT
const char* topicTx = "test";
const char* topicRx = "test";
const char* topicRfid = "rfid";

MFRC522 rfid(53, 39);

EthernetClient ethClient;
PubSubClient client(ethClient);

AESLib aesLib;
char cleartext[256];
char ciphertext[512];

// AES Encryption Key
byte aes_key[] = { 0x15, 0x2B, 0x7E, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Generate IV (once)
void aes_init() {
  aesLib.gen_iv(aes_iv);
  // workaround for incorrect B64 functionality on first run...
  encrypt("HELLO WORLD!", aes_iv);
}

String encrypt(char * msg, byte iv[]) {  
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen];
  aesLib.encrypt64(msg, encrypted, aes_key, iv);  
  return String(encrypted);
}

String decrypt(char * msg, byte iv[]) {
  unsigned long ms = micros();
  int msgLen = strlen(msg);
  char decrypted[msgLen]; // half may be enough
  aesLib.decrypt64(msg, decrypted, aes_key, iv);  
  return String(decrypted);
}

void setup() {
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

  //Aes
  aes_init();
}

void loop() {

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
      char rfidCard[message.length()];
      message.toCharArray(rfidCard, message.length() + 1);
      while (message.length() < 16){
        message = message + " ";
      }
      Serial.print("READED -> ");
      Serial.println(rfidCard);

      byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
      String encrypted = encrypt(rfidCard, enc_iv);
      sprintf(ciphertext, "%s", encrypted.c_str());
      Serial.print("Ciphertext: ");
      Serial.println(encrypted);
      char enc_rfidCard[encrypted.length()];
      encrypted.toCharArray(enc_rfidCard, encrypted.length() + 1);
      client.publish(topicRfid, enc_rfidCard); //Encrypted
    }
  }


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
  byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
  String decrypted = decrypt(aux, dec_iv);
  Serial.print("DECRYPTED -> ");
  Serial.println(aux);
}
