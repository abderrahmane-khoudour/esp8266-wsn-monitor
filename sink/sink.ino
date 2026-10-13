// TP WSN - ESP8266 n° 2 : sink + serveur Web
// Recoit les mesures par ESP-NOW et les publie sur http://192.168.4.1

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>
#include <user_interface.h>

ESP8266WebServer server(80);

struct SensorData {
  uint32_t nodeId;
  float temperature;
  float humidity;
  uint32_t counter;
};

volatile float temperature = 0;
volatile float humidity = 0;
volatile uint32_t packetCounter = 0;
volatile unsigned long lastReceived = 0;
volatile bool dataAvailable = false;

const char* apName = "WSN-SINK";
const char* apPassword = "WSN12345";

const char page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WSN Monitor</title>
<style>
body{font-family:Arial;background:#eef2f7;margin:0;padding:20px;color:#172033}
main{max-width:650px;margin:auto}
header,.card{background:white;padding:20px;margin-bottom:15px;border-radius:14px;box-shadow:0 3px 12px #0001}
h1{color:#075985}.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:15px}
.label{color:#64748b}.value{font-size:2rem;font-weight:bold;margin-top:10px}
.temp{color:#ea580c}.hum{color:#0284c7}#status{font-weight:bold}
@media(max-width:400px){.grid{grid-template-columns:1fr}}
</style></head><body><main>
<header><h1>WSN Monitor</h1>
<p>Supervision du réseau de capteurs ESP8266</p><b>Sink : ESP8266 n° 2</b></header>
<section class="grid">
<div class="card"><div class="label">Température</div>
<div class="value temp"><span id="temp">--</span> °C</div></div>
<div class="card"><div class="label">Humidité</div>
<div class="value hum"><span id="hum">--</span> %</div></div>
</section>
<section class="card"><h3>État du réseau</h3>
<p>Capteur : <span id="status">En attente</span></p>
<p>Dernier paquet : <span id="count">0</span></p>
<p>Dernière réception : <span id="age">--</span></p>
<p id="updated" class="label">Initialisation...</p></section>
</main><script>
async function updateData(){
 try{
  const response=await fetch('/data',{cache:'no-store'});
  if(!response.ok)throw new Error('HTTP');
  const d=await response.json();
  document.getElementById('temp').textContent=d.valid?d.temperature.toFixed(1):'--';
  document.getElementById('hum').textContent=d.valid?d.humidity.toFixed(1):'--';
  document.getElementById('count').textContent=d.counter;
  document.getElementById('age').textContent=d.age<0?'Aucune mesure':d.age+' s';
  const online=d.valid&&d.age<=15;
  document.getElementById('status').textContent=online?'Connecté':'Hors ligne';
  document.getElementById('status').style.color=online?'green':'red';
  document.getElementById('updated').textContent='Actualisation : '+new Date().toLocaleTimeString();
 }catch(e){document.getElementById('status').textContent='Erreur de communication';}
}
updateData();setInterval(updateData,2000);
</script></body></html>
)rawliteral";

void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len != sizeof(SensorData)) return;
  SensorData msg;
  memcpy(&msg, data, sizeof(msg));
  temperature = msg.temperature;
  humidity = msg.humidity;
  packetCounter = msg.counter;
  lastReceived = millis();
  dataAvailable = true;
  Serial.printf("Noeud %lu | T=%.1f C | H=%.1f %% | N=%lu\n",
    (unsigned long)msg.nodeId, msg.temperature, msg.humidity,
    (unsigned long)msg.counter);
}

void handleData() {
  bool valid = dataAvailable;
  float t = temperature, h = humidity;
  uint32_t count = packetCounter;
  unsigned long received = lastReceived;
  long age = valid ? (long)((millis() - received) / 1000) : -1;

  String json = "{";
  json += "\"valid\":"; json += valid ? "true" : "false";
  json += ",\"temperature\":"; json += String(t, 1);
  json += ",\"humidity\":"; json += String(h, 1);
  json += ",\"counter\":"; json += String(count);
  json += ",\"age\":"; json += String(age);
  json += "}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName, apPassword, 1);
  wifi_set_channel(1);

  Serial.println();
  Serial.println("WSN SINK + SERVEUR WEB");
  Serial.print("Wi-Fi : "); Serial.println(apName);
  Serial.print("Adresse Web : http://"); Serial.println(WiFi.softAPIP());
  Serial.print("MAC station du sink : "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Erreur initialisation ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);

  server.on("/", []() { server.send_P(200, "text/html", page); });
  server.on("/data", handleData);
  server.begin();
  Serial.println("Serveur Web pret.");
}

void loop() {
  server.handleClient();
}
