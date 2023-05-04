#include <FirebaseESP32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 20, 4);

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

//firebase
#define FIREBASE_HOST "https://sistem-tertanam-22040-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "wWSciIi7240J3USleVcHnwTjXCsSkKlAKd2NIION"

//Relay
#define  relayPin 12

//ldr
const int ldrPin = 34;
const int doPin = 14;
const float Gamma = 0.7;
const float rl10 = 50;

FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);
  delay(10);

  LCD.init();
  LCD.backlight();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }
  Serial.println("WiFi Terhubung");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  pinMode(relayPin, OUTPUT);
  pinMode(doPin, INPUT);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  int doValue = digitalRead(doPin);

  // Serial.print("LDR value: ");
  // Serial.println(ldrValue);
  // Serial.print("DO value: ");
  // Serial.println(doValue);

  ldrValue = map(ldrValue, 4095, 0, 1024, 0); //ubah nilai sensor ke adc
  float voltase = ldrValue / 1024.*5;
  float resistance = 2000 * voltase / (1 - voltase / 5);
  float kecerahan = pow(rl10 * 1e3 * pow(10, Gamma) / resistance, (1 / Gamma));
  kecerahan = round(kecerahan * 10) / 10.0;
  Serial.print("Kecerahan = ");
  Serial.println(kecerahan);
  LCD.setCursor(0, 0);
  LCD.print("Kecerahan: ");
  LCD.print(kecerahan);
  Firebase.setFloat(firebaseData, "/kecerahan", kecerahan);

  if (Firebase.getBool(firebaseData, "/relayState")) {
    bool relayState = firebaseData.boolData();
    if (kecerahan < 100) {
      relayState = 1;
    }
    digitalWrite(relayPin, relayState);
    if (relayState == 0) {
      Serial.println("Kondisi lampu mati");
      LCD.setCursor(0, 1);
      LCD.print("Lampu mati ");
    } else {
      Serial.println("Kondisi lampu hidup");
      LCD.setCursor(0, 1);
      LCD.print("Lampu hidup ");
    }
    Serial.println("State: " + String(relayState));
  } else {
    Serial.println("Gagal menerima data dari firebase");
  }

  if (kecerahan < 100 && Firebase.getBool(firebaseData, "/relayState") == 0) {
    delay(10000);
    if (kecerahan < 100) {
      digitalWrite(relayPin, 1);
      Serial.println("Lampu harus hidup karena kecerahan rendah");
    }
  }

  delay(1000);

}
