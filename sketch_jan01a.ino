#define BLYNK_TEMPLATE_ID   "TMPL3ojFr6Grn"
#define BLYNK_TEMPLATE_NAME "weather station"
#define BLYNK_AUTH_TOKEN    "NDuiRFu1BKWpZ41270W_x8iQ6q-RSBa6"


#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "p";          // your WiFi SSID
char pass[] = "12345678";   // your WiFi password

#define MQ135_PIN   34
#define YL83_PIN    32
#define DHTPIN      4
#define DHTTYPE     DHT11
#define LDR_PIN 35
#define RL_VALUE    10000
#define R0          2394288
#define VCC         3.3
#define R_FIXED 10000.0   // 10k ohm resistor
#define A 50000.0
#define GAMMA 0.8



DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Start WiFi + Blynk
  Blynk.begin(auth, ssid, pass);

  analogReadResolution(12);                 // 0–4095
  analogSetPinAttenuation(LDR_PIN, ADC_11db); // Allows full 0–3.3V range

  // Initialize sensors
  dht.begin();
}

void loop() {
  Blynk.run();   // keep Blynk connected

  //ldr
  int adcValu = analogRead(LDR_PIN);
  float voltage = adcValu * (VCC / 4095.0);  // Convert ADC to voltage
  if (voltage >= (VCC - 0.01))// Avoid division errors
  {  
    Serial.println("Too bright / invalid reading");
    delay(1000);
    return;
  }

  float rLDR = R_FIXED * (voltage / (VCC - voltage)); // Calculate LDR resistance
  float lux = pow((A / rLDR), (1.0 / GAMMA));  // Estimate light intensity in lux
  Serial.print(" ohm | Light Intensity: ");
  Serial.print(lux);
  Blynk.virtualWrite(V4, lux);
  
  // MQ135
  int adcValue = analogRead(MQ135_PIN);
  float vOut = (adcValue / 4095.0) * VCC;
  float Rs = RL_VALUE * ((VCC - vOut) / vOut);
  float ratio = Rs / R0;
  float ppm = pow(10, (-0.42 * log10(ratio) + 1.92));
  Serial.print("CO2 ppm: "); Serial.println(ppm);
  Blynk.virtualWrite(V3, ppm);

//ozone
  int R1=10;
  int Rp=10;
  Rs = Rp * (3.3 - vOut) / vOut;
  Serial.print(Rs);
  float rati = Rs / R1;
  float a = -0.45;   // Example slope (adjust from datasheet)
  float b = 2.3;     // Example intercept (adjust from datasheet)
  float pm = pow(10, (a * log10(rati) + b));
  Serial.print(" | Ozone (ppm): "); Serial.println(pm);
  Blynk.virtualWrite(V5, pm);  


  // YL83 Rain Sensor
  int yl83Value = analogRead(YL83_PIN);
  Serial.print(yl83Value);
  int level = map(yl83Value, 0, 4095, 0, 100);
  int leve = map(level, 100, 0, 0, 100);
  Serial.print("YL83 Rain: "); Serial.print(leve); Serial.println("%");
  Serial.print("YL83 Rain: "); Serial.print(level); Serial.println("%");
  Blynk.virtualWrite(V2, leve);

  // DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity: "); Serial.print(h); Serial.println(" %");
  Serial.print("Temperature: "); Serial.print(t); Serial.println(" *C");
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V0, t);

  delay(2000);
}
