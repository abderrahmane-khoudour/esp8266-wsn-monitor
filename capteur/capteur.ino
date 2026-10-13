// TP WSN - ESP8266 n° 1 : noeud capteur
// Lit le DHT11 toutes les 5 s et envoie la mesure au sink par ESP-NOW

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>
#include <user_interface.h>

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MAC station REELLE du sink (affichee dans son moniteur serie)
uint8_t sinkAddress[] = {0xDC, 0x4F, 0x22, 0x3B, 0x16, 0x4A};

struct SensorData {
  uint32_t nodeId;
  float temperature;
  float humidity;
  uint32_t counter;
};

SensorData dataToSend;
uint32_t packetCounter = 0;
unsigned long previousMillis = 0;

void onDataSent(uint8_t *mac, uint8_t status) {
  Serial.print("Envoi : ");
  Serial.println(status == 0 ? "SUCCES" : "ECHEC");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_channel(1);

  Serial.println("\nWSN - NOEUD CAPTEUR");
  Serial.print("MAC du capteur : ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Erreur initialisation ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  if (esp_now_add_peer(sinkAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0) != 0) {
    Serial.println("Erreur ajout du sink");
    return;
  }
  esp_now_register_send_cb(onDataSent);
  Serial.println("Noeud capteur pret.");
}

void loop() {
  if (millis() - previousMillis >= 5000) {
    previousMillis = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("Erreur lecture DHT11");
      return;
    }

    dataToSend.nodeId = 1;
    dataToSend.temperature = t;
    dataToSend.humidity = h;
    dataToSend.counter = ++packetCounter;

    int result = esp_now_send(sinkAddress,
      (uint8_t *)&dataToSend, sizeof(dataToSend));
    Serial.printf("T=%.1f C, H=%.1f %%\n", t, h);
    if (result != 0) Serial.println("Erreur lors de l'envoi");
  }
}
