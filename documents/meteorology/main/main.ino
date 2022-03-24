

// ==================== Librerias para los componentes
#include <DHT.h>         // Libreria DHT
#include <ESP8266WiFi.h> // Libreria ESP8266
#include <Wire.h>        // Libreria para I2C

// ==================== Librerias para los display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <OLED.h>                 // Libreria para el display OLED
// #include <LiquidCrystal_I2C.h> // Libreria para el display LCD

// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
#include "config/global_variables.hpp"

// #define OLED_RESET 1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// #include <WifiLocation.h>

// ==================== Librerias para el manejo de fechas
#include <NTPClient.h>
#include <WiFiUdp.h>

// ==================== Librerias para la creacion de un servidor web
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// ==================== Librerias para coneccion a base datos de google
#include <FirebaseESP8266.h>    // Firebase para ESP8266
#include <addons/TokenHelper.h> // Generacion de token de acceso
#include <addons/RTDBHelper.h>

// ==================== Datos conexion API ThingSpeak
#define THINK_KEY "1VXGOVNMCWHWT3B2"      //  Ingrese su clave Write API de ThingSpeak
#define THINK_SERVER "api.thingspeak.com" // Servidos Thingspeak (Matlab)

//  ==================== Configuracion Firebase (NO MODIFICAR)
#define DB_ID "hidrologia-u-distrital"                                       // Identificador de la base de datos
#define DB_HOST "https://hidrologia-u-distrital-default-rtdb.firebaseio.com" // Direccion de la base datos
#define DB_API_KEY "AIzaSyC0v-XlIcscm_BlAKdyYxHPzp5CQpmxBgw"                 // Clave de acceso a la bas de datos
#define DB_REST_SECRET "tPtDKT4ZF0nanJ9QyJYZBRMsrnnc3P1Cj0CQPGWT"            // Clave de acceso a la bas de datos

// #define GOOGLE_API_GEOLOCATION_KEY "AIzaSyAor4_IQ6zbgIQ44djnjKo1EdsFD8CyqfQ" // Clave de acceso a la API de geolocalizacion
// #define KEY_GEOLOC "/geolocation/v1/geolocale?key=AIzaSyAor4_IQ6zbgIQ44djnjKo1EdsFD8CyqfQ"
// #define SERVER_GEOLOC "https://www.googleapis.com"

// ================================================================================= //
//   Para usar la base de datos de google debe solicitar la creacion del usuario a   //
//   efmarroquinb@udistrital.edu.co, enviando su correo, y datos del proyecto        //
// ================================================================================= //

// ==================== Configuracion Firebase (Datos del proyecto) entregado por el administrador del sistema
#define DB_NODE "edwin-marroquin"                              // Nombre del nodo donde se almacena la informacion
#define FIREBASE_EMAIL "efmarroquinb@correo.udistrital.edu.co" // Correo de validacion
#define FIREBASE_PASS "Maria-0227-"                            // Contraseña de validacion

// ==================== Configuracion del sensor DTH
#define DHTPIN D5         // Define el identificador del pin a usar en la placa ESP8266
#define DHTTYPE DHT11     // Define el tipo de sensor conectador a la placa ESP8266
DHT dht(DHTPIN, DHTTYPE); // Instaciacion de la clase para el Sensor DHT

// ==================== Configuracion placa ESP8266
#define ESP8266_LED 2 // Define el led a manupilar en la placa ESP8266

// ==================== Creacion del servidor local
#define SERVER_PORT 80
ESP8266WebServer server(SERVER_PORT);

// ==================== Conexión Wifi-Servidor
WiFiClient client;
WiFiUDP ntpUDP;
HTTPClient http;

// WifiLocation location(GOOGLE_API_GEOLOCATION_KEY);

// ==================== Configuracion Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;

// ==================== Inicializacion de variables
// String nodePath;

// float lat = 0.0;
// float lon = 0.0;

// float t = 0.0;
// float h = 0.0;

// int d = 10000;
// long dataTime = 0;
// int utcOffset = 0;
// Define fecha y hora segun uso horario
// Ajustado en segundos * El uso horario
// GMT+5 =  5
// GMT-5 =  5
// GMT-1 = -1
// GMT   =  0

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset * 3600);

// ==================== Incluir diseño webserver local
#include "views/index.h"
#include "views/inte.c"

void setup()
{

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.drawBitmap(4, 2, image_data_inteud, 120, 62, SSD1306_WHITE);
  display.display();
  delay(3000);

  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("WIFI: MeteoLab");
  display.println("PASS: hidro2021");
  display.println();
  display.println("Open in browser:");
  display.println("192.168.4.1");
  display.println("Connect local wifi");
  display.println("Enjoy!");
  display.display();

  pinMode(ESP8266_LED, OUTPUT); // Configuracion led de la placa ESP8266

  WiFi.mode(WIFI_STA); // Inicia la placa ESP8266 en modo
  // STATION       = WIFI_STA
  // ACCESS POINT  = WIFI_AP

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  WiFiManager wm;

  // ============ Se crea una red wifi provisional
  // ============ con el nombre de la red y la contraseña
  if (!wm.autoConnect("MeteoLab", "hidro2021"))
  {

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Error en la coneccion");
    display.println("Intente de nuevo");
    display.display();
    delay(2000);
    ESP.reset();
  }

  timeClient.begin();

  Serial.println("IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }

  // ============= Inicializacion del servidor local
  server.on("/", handle_OnConnect);
  server.on("/data", handle_ApiData);
  server.onNotFound(handle_NotFound);
  server.begin();

  // ============= Coneccion a FireBase
  config.api_key = DB_API_KEY;
  config.database_url = DB_HOST;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASS;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);

  // ============= Inicio de trabajo del |sensor DHT11
  dht.begin();
}

void loop()
{

  digitalWrite(ESP8266_LED, LOW); // Enciende el led de la placa ESP8266

  // ============= Toma de lecturas del sensor DHT
  h = dht.readHumidity();
  t = dht.readTemperature();

  server.handleClient();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println();
  display.println("OPEN: " + WiFi.localIP().toString());
  display.println();
  display.setTextSize(2);
  display.println("T: " + String(t));
  display.setTextSize(1);
  display.println();
  display.setTextSize(2);
  display.println("H: " + String(h));
  display.display();

  if (isnan(h) || isnan(t))
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Error de lectura");
    display.println("Revise el sistema");
    display.display();
    delay(1000);
    return;
  }

  if (client.connect(THINK_SERVER, SERVER_PORT))
  {

    String postStr = THINK_KEY;

    // ============= Creacion del nodo con una marca de tiempo para almacenar
    // ============= la nformacion en la base de datos de FireBase
    timeClient.update();
    dataTime = timeClient.getEpochTime();
    nodePath = (String)DB_NODE + "/" + String(dataTime);

    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);

    // ==================== Creacion de cabeceras REST para el envio de la informacion
    // ==================== a la pagina de thinkspeak.com
    Serial.println("// ============= Saving in ThinkSpeak");
    client.print("POST /update HTTP/1.1\n");
    client.print((String) "Host: " + THINK_SERVER + "\n");
    client.print("Connection: close\n");
    client.print((String) "X-THINGSPEAKAPIKEY: " + THINK_KEY + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("// ============= Saved in ThinkSpeak");
  }

  client.stop();
  client.flush();

  Serial.println("\n// ===========================================");
  Serial.println((String) "Sending temperature (°C): " + String(t));
  Serial.println((String) "Sending humidity     (%): " + String(h));

  if (client.connect(DB_HOST, 80))
  {
    Serial.println("\n// ============= Saving in Firebase");
    Serial.println(Firebase.setFloat(fbdo, nodePath + "/temp", t) ? "// ==== Saving Temperature" : fbdo.errorReason().c_str());
    Serial.println(Firebase.setFloat(fbdo, nodePath + "/humi", h) ? "// ==== Saving Humidity" : fbdo.errorReason().c_str());
    Serial.println("// ============= Saved in Firebase\n");
  }

  digitalWrite(ESP8266_LED, HIGH); // Apaga el led de la placa ESP8266

  // http.begin(client, SERVER_GEOLOC);
  // delay(100); // See if this prevents the problm with connection refused and deep sleep
  // http.addHeader("Content-Type", "application/json");
  // delay(100); // See if this prevents the problm with connection refused and deep sleep
  // int httpCode = http.POST(KEY_GEOLOC);
  // String payload = http.getString();
  // Serial.println(httpCode); // Print HTTP return code
  // Serial.println(payload);  // Print request response payload
  // http.end();

  // location_t loc = location.getGeoFromWiFi();

  // Serial.println("Location request data");
  // Serial.println(location.getSurroundingWiFiJson());
  // Serial.println("Latitude: " + String(loc.lat, 7));
  // Serial.println("Longitude: " + String(loc.lon, 7));
  // Serial.println("Accuracy: " + String(loc.accuracy));

  MDNS.update();
  delay(d); // Tiempo de retraso entre las lecturas
  // Se recomiendan minimo 10000 milisegundos entre lecturas
  // pero para no saturar la base de datos se recomiendan
  // lecturas entre 30000 milisegundos y 120000 milisegundos
}

void handle_OnConnect()
{
  String index = index_html;
  server.send(200, "text/html", index_html);
}

void handle_ApiData()
{
  String data = dataJson(dataTime, t, h);
  server.send(200, "application/json", data);
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Opps! regrese a la pagina anterior");
}

String dataJson(int d, float t, float h)
{
  String dataJ = "{ \"d\": ";
  dataJ += String(d);
  dataJ += " , \"t\" : ";
  dataJ += String(t);
  dataJ += " , \"h\" : ";
  dataJ += String(h);
  dataJ += "}";

  return dataJ;
}

// TODO:
//          1. Configuracion de la base de datos desde el dispositivo movil
//          2. Almacenamiento en Firebase de los datos en formato Json
