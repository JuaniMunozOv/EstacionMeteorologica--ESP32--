#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>

// Configuración de los sensores DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWireObjeto(ONE_WIRE_BUS);
DallasTemperature sensorDS18B20(&oneWireObjeto);

// Configuración del sensor DHT11
const int DHTPIN = 4;
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuración del sensor de humedad del suelo
#define sensorsuelo 34

// Pines para LEDs indicadores
const int greenLedPin = 26;  // Pin para el LED verde

const char *thingspeakServer = "https://api.thingspeak.com/update?api_key=74OG7DSH9WLJSAJV&field1=0";
const char *apiKey = "74OG7DSH9WLJSAJV";

const char *apSsid = "ESP32-EstacionMeteorologica"; // SSID para el punto de acceso WiFi
const char *apPassword = "12345678";                 // Contraseña para el punto de acceso WiFi

bool wifiConfigured = false; // Bandera para indicar si se ha configurado WiFi
bool wifiConnected = false;  // Bandera para indicar si se ha conectado a WiFi

const char *ssid = "";      // Variable para almacenar SSID de WiFi configurado
const char *password = "";  // Variable para almacenar contraseña de WiFi configurado

IPAddress staticIP(192, 168, 4, 4); // Configura la IP estática que desees
IPAddress subnet(255, 255, 255, 0);  // Configura la máscara de subred que desees
IPAddress gateway(192, 168, 4, 4);   // Configura la puerta de enlace que desees


AsyncWebServer server(80);


// Función para enviar datos a ThingSpeak
void sendToThingSpeak(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(thingspeakServer);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      Serial.print("Enviado a ThingSpeak. Código de respuesta: ");
      http.writeToStream(&Serial);
    } else {
      Serial.print("Error en la solicitud. Código de respuesta: ");
      Serial.println(httpResponseCode);
      digitalWrite(greenLedPin, LOW);  // Apagar LED verde en caso de error en la solicitud
      delay(1000);
      digitalWrite(greenLedPin, HIGH);
      delay(1000);
    }
    http.end();
  } else {
    Serial.println("Error en la conexión WiFi");
    digitalWrite(greenLedPin, LOW);  // Apagar LED verde en caso de error de conexión
    delay(1000);
    digitalWrite(greenLedPin, HIGH);
    delay(1000);
  }
}

void switchToStationMode() {
  WiFi.softAPdisconnect(true); // Desconectar el punto de acceso WiFi
  WiFi.begin(ssid, password);  // Cambiar a modo estación con los nuevos valores de SSID y contraseña
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  sensorDS18B20.begin();
  pinMode(greenLedPin, OUTPUT);

 if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.mode(WIFI_AP); // Configura el ESP32 en modo punto de acceso

  // Configura el punto de acceso WiFi
  WiFi.softAPConfig(staticIP, gateway, subnet);
  const char *apSsid = "ESP32-EstacionMeteorologica"; // SSID para el punto de acceso WiFi
  const char *apPassword = "12345678";                 // Contraseña para el punto de acceso WiFi
  WiFi.softAP(apSsid, apPassword);

  Serial.println("ESP32 en modo Punto de Acceso");
  Serial.print("SSID del Punto de Acceso: ");
  Serial.println(apSsid);
  Serial.print("Contraseña del Punto de Acceso: ");
  Serial.println(apPassword);
  Serial.print("IP del Punto de Acceso: ");
  Serial.println(WiFi.softAPIP());

  // Configura el servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/indexx.html", "text/html");
  });

   server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/styles.css", "text/css");
  });

   /*server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/indexx.html");
  });*/

  // Agrega otras rutas y manejadores aquí

  // Configura la ruta para manejar la solicitud de conexión a WiFi
  server.on("/configure", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid;
    String password;
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      // Intenta conectarse a la red WiFi ingresada
      WiFi.begin(ssid.c_str(), password.c_str());
      int retries = 0;
      while (WiFi.status() != WL_CONNECTED && retries < 30) {
        delay(1000);
        retries++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado a la red WiFi");
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        request->send(SPIFFS, "text/html", "Conexión exitosa. ESP32 conectado a la red WiFi.");
      } else {
        Serial.println("Error al conectarse a la red WiFi");
        request->send(SPIFFS, "text/html", "Error al conectarse a la red WiFi. Verifica los datos e inténtalo nuevamente.");
      }
    }
  });

  /* Configura la ruta para reiniciar el ESP32
  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    ESP.restart();
  });*/

  // Inicializa el servidor web
  server.begin(); 
}

/*
void connectToWifi() {
  int retries = 0;
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(1000);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConexión WiFi exitosa");
  } else {
    Serial.println("\nError en la conexión WiFi");
  }
}*/

unsigned long previousMillis = 0;
const unsigned long interval = 3000; // Intervalo de 3 segundos

void loop() {

  unsigned long currentMillis = millis();
  // Si la configuración WiFi está lista, cambia a modo estación
  if (wifiConfigured && !wifiConnected) {
    switchToStationMode();
    wifiConnected = true; // Marcar que se ha conectado a la red WiFi
  }
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

  sensorDS18B20.requestTemperatures();
  float temperatura1 = sensorDS18B20.getTempCByIndex(0);
  float temperatura2 = sensorDS18B20.getTempCByIndex(1);

  // Verificar lecturas válidas de los termómetros
  if (temperatura1 == -127.00 || temperatura2 == -127.00) {
    digitalWrite(greenLedPin, LOW);  // Apagar LED verde en caso de error en la lectura
    delay(1000);
    digitalWrite(greenLedPin, HIGH);
    delay(1000);
    return;
  }

  float humedad = dht.readHumidity();
  float IndicedeCalor = dht.computeHeatIndex(temperatura1, humedad, false);
  int porchumedad = map(analogRead(sensorsuelo), 0, 4095, 100, 0);

  String data = "&field6=" + String(temperatura1) + "&field2=" + String(temperatura2) + "&field3=" + String(humedad) + "&field4=" + String(IndicedeCalor) + "&field5=" + String(porchumedad);
  sendToThingSpeak(data);

  // Mostrar el uso de memoria disponible
  Serial.print("Memoria Libre: ");
  Serial.println(ESP.getFreeHeap());

  delay(3000);  // Espera 3 segundos antes de la siguiente lectura
}
}
