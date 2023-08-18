#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de WiFi y ThingSpeak
const char* ssid = "CASA MNZ";
const char* password = "munos0610";
const char* server = "https://api.thingspeak.com/update?api_key=74OG7DSH9WLJSAJV&field1=0";
const char* apiKey = "74OG7DSH9WLJSAJV";

// Configuración de los sensores DS18B20
#define ONE_WIRE_BUS 14
OneWire oneWireObjeto(ONE_WIRE_BUS);
DallasTemperature sensorDS18B20(&oneWireObjeto);

// Configuración del sensor DHT11
const int DHTPIN = 4;
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// Pines para LEDs indicadores
const int greenLedPin = 26; // Pin para el LED verde

void setup() {
  Serial.begin(115200);
  dht.begin();
  sensorDS18B20.begin();
  pinMode(greenLedPin, OUTPUT);

  // Conexión WiFi
  connectToWiFi();
  Serial.println("Conectado a WiFi");
}

void loop() {
  sensorDS18B20.requestTemperatures();
  float temperatura1 = sensorDS18B20.getTempCByIndex(0);
  float temperatura2 = sensorDS18B20.getTempCByIndex(1);
  
  // Verificar lecturas válidas de los termómetros
  if (temperatura1 == -127.00 || temperatura2 == -127.00) {
    handleReadError();
    return;
  }
  
  float humedad = dht.readHumidity();
  float IndicedeCalor = dht.computeHeatIndex(temperatura1, humedad, false);

  String data = "&field6=" + String(temperatura1) + "&field2=" + String(temperatura2)+ "&field3=" + String(humedad) + "&field4=" + String(IndicedeCalor);
  sendToThingSpeak(data);

  // Mostrar datos en la consola
  Serial.print("T1: ");Serial.println(temperatura1);
  Serial.print("T2: ");Serial.println(temperatura2);
  Serial.print("hum: ");Serial.println(humedad);
  Serial.print("IC: ");Serial.println(IndicedeCalor);

  delay(3000); // Espera 3 segundos antes de la siguiente lectura
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(greenLedPin, LOW); // Apagar LED verde en caso de error de conexión
    delay(500);
    digitalWrite(greenLedPin, HIGH);
    delay(500);
  }
  digitalWrite(greenLedPin, HIGH); // Encender LED verde al conectar a WiFi y todo esté correcto
}

void handleReadError() {
  digitalWrite(greenLedPin, LOW); // Apagar LED verde en caso de error en la lectura
  delay(1000);
  digitalWrite(greenLedPin, HIGH);
  delay(1000);
}

void sendToThingSpeak(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      Serial.print("Enviado a ThingSpeak. Código de respuesta: ");
      http.writeToStream(&Serial);
    } else {
      handleSendError();
    }
    http.end();
  } else {
    handleWiFiError();
  }
}

void handleSendError() {
  Serial.print("Error en la solicitud. Código de respuesta: ");
  Serial.println(httpResponseCode);
  digitalWrite(greenLedPin, LOW); // Apagar LED verde en caso de error en la solicitud
  delay(1000);
  digitalWrite(greenLedPin, HIGH);
  delay(1000);
}

void handleWiFiError() {
  Serial.println("Error en la conexión WiFi");
  digitalWrite(greenLedPin, LOW); // Apagar LED verde en caso de error de conexión
  delay(1000);
  digitalWrite(greenLedPin, HIGH);
  delay(1000);
}
