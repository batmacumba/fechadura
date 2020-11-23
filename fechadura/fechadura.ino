#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define RST_PIN  D2 
#define SS_PIN  D4
#define STASSID "Obi Wan Kenobi_IoT"
#define STAPSK  "oladobomdaforca" // minha senha do wifi, uhuu
#define MSG_BUFFER_SIZE  (50)
#define MAX_OCUPANTES 64
#define NTP_OFFSET   -3 * 60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

WiFiClient espClient;
PubSubClient client(espClient);
MFRC522 mfrc522(SS_PIN, RST_PIN);
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* mqtt_server = "192.168.15.49";
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
String authorizedIDs = "";
int value = 0;
char *current[MAX_OCUPANTES];
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

String 
dump_byte_array(byte *buffer, byte bufferSize) 
{
    String rfid_uid = "";
    for (byte i = 0; i < bufferSize; i++) {
        String uid_part = String(buffer[i], HEX); 
        rfid_uid += uid_part;
    }
    return rfid_uid;
}

String
readRFID()
{
    if (!mfrc522.PICC_IsNewCardPresent()) {
      delay(50);
      return "";
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      delay(50);
      return "";
    }
    String rfid_uid = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    return rfid_uid;
}

void 
setup_wifi() 
{

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void 
callback(char* topic, byte* payload, unsigned int length) 
{
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char) payload[i];
    }
    if (strncmp(topic, "autorizados", 11) == 0) authorizedIDs = message;
}

void 
reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("autorizados");
      // TODO: pegar a ocupacao atual em caso de power off
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool 
isAuthorizedID(String rfid_uid)
{
    if (authorizedIDs.indexOf(rfid_uid) > 0) return true;
    return false;
}

bool
notInCurrent(String rfid_uid)
{
    for (int i = 0; i < MAX_OCUPANTES; i++) 
        if (strncmp(current[i], rfid_uid.c_str(), 8) == 0)  return false;
    return true;
}

void
addToCurrent(String rfid_uid)
{
    for (int i = 0; i < MAX_OCUPANTES; i++) {
        if (strlen(current[i]) == 0) {
            strncpy(current[i], rfid_uid.c_str(), 9);
            return;
        }
    }
}

void
shiftCurrent()
{
    for (int i = 0; i < MAX_OCUPANTES; i++) {
        // Troca o zero no i pelo ID no i+1 e zera o i+1
        if (strlen(current[i]) == 0 && strlen(current[i + 1]) > 0) {
            strncpy(current[i], current[i + 1], 9);
            *current[i + 1] = '\0';
        }
        // Dois zeros seguidos significa que não é mais necessário continuar o shift
        if (strlen(current[i]) == 0 && strlen(current[i + 1]) == 0) 
            return;
    }
}

int
removeFromCurrent(String rfid_uid)
{
    for (int i = 0; i < MAX_OCUPANTES; i++) {
        if (strncmp(current[i], rfid_uid.c_str(), 8) == 0) {
            *current[i] = '\0';
            // Vamos evitar "buracos" no array
            shiftCurrent();
            return 0;
        }
    }
    return -1;
}

void
initializeCurrent()
{
    for (int i = 0; i < MAX_OCUPANTES; i++) {
        current[i] = (char *) malloc(sizeof(char) * 9);
        *current[i] = '\0';
    }
}

String
formatCurrent()
{
    String formatted = "";
    for (int i = 0; i < MAX_OCUPANTES; i++) {
        if (strlen(current[i]) > 0) {
          if (strlen(current[i + 1]) == 0 || i == MAX_OCUPANTES - 1) formatted += String(current[i]);
          else formatted += String(current[i]) + ", "; 
        }
    }
    return formatted;
}

void 
setup() 
{
    Serial.begin(115200);    
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    SPI.begin();           
    mfrc522.PCD_Init();   
    initializeCurrent();
    timeClient.begin();TO
}

void 
loop() 
{ 
    if (!client.connected()) reconnect();
    client.loop();
    timeClient.update();

    // TODO: só vai ler o RFID quando a porta estiver fechada
    String rfid_uid = readRFID();
    if (rfid_uid.length() > 0) {
        if (isAuthorizedID(rfid_uid)) {
            if (notInCurrent(rfid_uid)) {
                // Entrada
                addToCurrent(rfid_uid);
                String message = "{ \"id\": \"" + rfid_uid + "\", \"timestamp\": \"" + 
                                timeClient.getFormattedDate() + "\" }";
                client.publish("entrada", message.c_str());
                delay(1000);
            }
            else {
                // Saída
                removeFromCurrent(rfid_uid);
                String message = "{ \"id\": \"" + rfid_uid + "\", \"timestamp\": \"" + 
                                timeClient.getFormattedDate() + "\" }";
                client.publish("saida", message.c_str());
                delay(1000);
            }
        }
    }
  
    unsigned long now = millis();
    if (now - lastMsg > 10000) {
        lastMsg = now;
        ++value;
        String message = "{ \"ocupantes\": \"" + formatCurrent() + "\", \"timestamp\": \"" + 
                              timeClient.getFormattedDate() + "\" }";
        client.publish("ocupacao", message.c_str());
    }
}
