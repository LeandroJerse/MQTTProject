#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// === CONFIGURAÇÃO DE REDE ===
const char* ssid = "LAMAU";
const char* password = "lamau_01";
const char* mqtt_server = "192.168.0.105";
const char* clientID = "ESP32Client";
const char* topic = "mensagem";

// === OBJETOS GLOBAIS ===
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MPU6050 mpu;

// === PINOS ===
const int LED1 = 33;
const int LED2 = 32;
const int BUZZER_PIN = 14;

// === CONFIG DE DETECÇÃO ===
const float GYRO_THRESHOLD = 22;
bool quedaDetectada = false;

// === TIMERS E FLAGS ===
unsigned long lastWifiAttempt = 0;
unsigned long lastMsg = 0;
unsigned long lastMQTTPublish = 0;
bool wifiConnected = false;

const unsigned long wifiReconnectInterval = 15000;
const unsigned long mqttPublishInterval = 5000;

// === SETUP ===
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(26, 25);

  if (!mpu.begin()) {
    Serial.println("MPU6050 não encontrado!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);

  Serial.println("\n🚀 Iniciando ESP32 MQTT + Queda...");

  setupWiFi();
  setupMQTT();
}

// === LOOP ===
void loop() {
  unsigned long currentMillis = millis();

  // ==== GESTÃO WI-FI ====
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    if (currentMillis - lastWifiAttempt >= wifiReconnectInterval) {
      lastWifiAttempt = currentMillis;
      setupWiFi();
    }
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("✅ Wi-Fi reconectado!");
    setupMQTT();
  }

  // ==== GESTÃO MQTT ====
  if (wifiConnected) {
    if (!client.connected()) reconnectMQTT();
    client.loop();
  }

  // ==== SENSOR DE QUEDA ====
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float acc_total = sqrt(a.acceleration.x * a.acceleration.x +
                         a.acceleration.y * a.acceleration.y +
                         a.acceleration.z * a.acceleration.z);
  float acc_dynamic = abs(acc_total - 9.81);

  if (acc_dynamic > GYRO_THRESHOLD) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(BUZZER_PIN, HIGH);

    if (!quedaDetectada && wifiConnected && client.connected()) {
      quedaDetectada = true;
      String alerta = "🚨 Queda detectada! acel: " + String(acc_dynamic, 2);
      client.publish(topic, alerta.c_str());
      Serial.println(alerta);
    }
  } else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    quedaDetectada = false;
  }

  delay(100);
}

// === CONECTAR AO WI-FI ===
void setupWiFi() {
  Serial.print("📡 Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiReconnectInterval) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi conectado!");
    Serial.print("📶 IP local: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println("\n❌ Falha na conexão Wi-Fi.");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
  }
}

// === SETUP MQTT ===
void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
}

// === RECONNECT MQTT ===
void reconnectMQTT() {
  Serial.print("🔁 Conectando ao MQTT...");
  if (client.connect(clientID)) {
    Serial.println("conectado!");
    client.subscribe(topic);
  } else {
    Serial.print("falhou. Código: ");
    Serial.print(client.state());
    Serial.println(" -> tentando novamente em breve.");
  }
}

// === CALLBACK ===
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
