#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "list.h"

#define RST_PIN           D0  
#define SS_PIN            D8  
#define RELAY_PIN         D3  
#define SSID              "your-ssid"
#define PASS              "your-pass"       
#define MQTT_SERVER       "MQTT-SERVER-IP"
#define MAX_BUFFER_SIZE   256
#define NTP_OFFSET        -3 * 60 * 60            // In seconds
#define NTP_INTERVAL      60 * 1000               // In miliseconds
#define NTP_ADDRESS       "europe.pool.ntp.org"

WiFiClient espClient;
PubSubClient client(espClient);
MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiUDP ntpUDP; 
NTPClient time_client(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

bool current_not_restored = true;
List *authorized = list_new();
List *current = list_new();
char uid[MAX_BUFFER_SIZE];

int
str_cmp(void *a, void *b)
{
    char *str_a = (char *) a;
    char *str_b = (char *) b;
//    Serial.printf("compare %s %s\n", str_a, str_b);
    return strcmp(str_a, str_b);
}

void
readRFID()
{
    static unsigned long last_new_reading = 0;
    static char last_new_uid[MAX_BUFFER_SIZE];
    String str_uid = "";

    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
        goto zera_uid;
        
    for (byte i = 0; i <  mfrc522.uid.size; i++) {
        String uid_part = String(mfrc522.uid.uidByte[i], HEX); 
        str_uid += uid_part;
    }
    strncpy(uid, str_uid.c_str(), MAX_BUFFER_SIZE);
    // Leitura repetida
    if (strcmp(uid, last_new_uid) == 0 && millis() - last_new_reading < 3000)
        goto zera_uid;
    // Nova leitura
    else {
        strncpy(last_new_uid, uid, MAX_BUFFER_SIZE);
        last_new_reading = millis();
        return;
    }
    
    zera_uid:
        uid[0] = '\0';
}

void 
setup_wifi() 
{

    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

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
    if (strncmp(topic, "autorizados", 11) == 0) update_authorized(message);
    if (strncmp(topic, "ocupacao", 8) == 0) {
        restore_current(message);
        current_not_restored = false;
    }
}

void 
reconnect() 
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Fechadura-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        client.subscribe("autorizados");
        client.subscribe("ocupacao");

        unsigned long start = millis();
        unsigned long now = millis();
        while (current_not_restored) {
            client.loop();
            if (now - start > 2000) {
                Serial.println("Could not restore state from MQTT server! Trying again...");
                now = millis();
            }
            // Start from scratch
            if (millis() - start > 10000) break;
        }
        client.unsubscribe("ocupacao");
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

void
update_authorized(String message)
{
    if (authorized) list_destroy(authorized);
    authorized = list_new();
    char buffer[MAX_BUFFER_SIZE];
    strncpy(buffer, message.c_str(), MAX_BUFFER_SIZE);
    char *token = strtok(buffer, ",");
    while (token != NULL) {
        list_append(authorized, token, strlen(token) + 1);
        token = strtok(NULL, ",");
    }
}

void
restore_current(String message)
{
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message.c_str());
    if (error) {
        Serial.printf("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
    }
    const char *ocupantes = doc["ocupantes"];
    char buffer[MAX_BUFFER_SIZE];
    strncpy(buffer, ocupantes, MAX_BUFFER_SIZE);
    char *token = strtok(buffer, ", ");
    while (token != NULL) {
        list_append(current, token, strlen(token) + 1);
        token = strtok(NULL, ", ");
    }
}

void
format_current(char *buffer)
{
    String formatted = "";
    for (int i = 0; i < list_length(current); i++) {
        char *str = (char *) list_get(current, i);
        if (i == list_length(current) - 1) formatted += String(str);
        else formatted += String(str) + ", "; 
    }
    strcpy(buffer, formatted.c_str());
}

void
abre_trava_eletrica()
{
    digitalWrite(RELAY_PIN, LOW); 
    delay(1000);          
    digitalWrite(RELAY_PIN, HIGH);  
}

void 
setup() 
{
    Serial.begin(115200);    
    setup_wifi();
    client.setServer(MQTT_SERVER, 1883);
    client.setCallback(callback);
    pinMode(SS_PIN, OUTPUT);
    SPI.begin();       
    SPI.setDataMode(SPI_MODE3);    
    mfrc522.PCD_Init();  
    delay(4);       
    mfrc522.PCD_DumpVersionToSerial(); 
    time_client.begin();
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

}

void 
loop() 
{ 
    if (!client.connected()) reconnect();
    client.loop();
    time_client.update();
    bool abre = false;
    
    // TODO: só vai ler o RFID quando a porta estiver fechada
    /* Handler da leitura do cartão */
    char message[MAX_BUFFER_SIZE];
    readRFID();
    if (strlen(uid) > 0) {
        const char *topic;
        if (list_length(authorized) > 0 && list_contains(authorized, uid, &str_cmp)) {
            snprintf(message, MAX_BUFFER_SIZE, 
                    "{ \"id\": \"%s\", \"timestamp\": \"%s\" }",
                    uid, time_client.getFormattedDate().c_str());
            // Entrada
            if (!list_contains(current, uid, &str_cmp)) {
                list_append(current, uid, strlen(uid) + 1);
                topic = "entrada";
            }
            // TODO: possível bug: pessoa nao autorizada nao pode sair
            // Saída
            else {
                list_remove(current, uid, &str_cmp);
                topic = "saida";
            }
            abre = true;
            
        }
        else {
            strncpy(message, uid, MAX_BUFFER_SIZE);
            topic = "desconhecidos";
        }

        client.publish(topic, message);
    }

    /* Envio de mensagens periódicas para o tópico "ocupacao" */
    static unsigned long last_msg = 0;
    unsigned long now = millis();
    if (now - last_msg > 5000) {
       last_msg = now;
       char buffer[MAX_BUFFER_SIZE];
       format_current(buffer);
       snprintf(message, MAX_BUFFER_SIZE, 
                "{ \"ocupantes\": \"%s\", \"timestamp\": \"%s\" }",
                buffer, time_client.getFormattedDate().c_str());
       // Retain
       client.publish("ocupacao", message, 1);
    }

    if (abre) abre_trava_eletrica();
}
