/*
 * Sketch       : alarma_wemos_d1_v2.ino
 * Descripción  : Alarma por detección de movimiento con envío en tiempo real
 *                de alerta al teléfono móvil. 
 * Autor        : Juan Pedro Serna
 * Versión      : 2.0.0
 * Placa        : Wemos D1 R2
 * Placa IDE    : LOLIN WEMOS D1 R2 & Mini
 *
 */

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

// Definir RGB LED PINS

#define BLUEPIN D3
#define GREENPIN D4
#define REDPIN D5

// Variables Wifi
const char* ssid = "***";
const char* password = "***";

// Variables servicio pushbullet.com

const char* host = "api.pushbullet.com";
const char* apiKey = "***";

// Variables zumbador pasivo

const int BUZZERPIN = D1;
const int DURACION = 3000;
const int FRECUENCIA = 440;

// Variables pulsador

const int PULSADORPIN = D2;

int valorPulsador;

// Variables sensor PIR HC-SR501

const int PIRPIN = D6;

int sensorON;       // Para activar/desactivar el sensor
int valorSensorPIR = 1;

ESP8266WiFiMulti wifiMulti;

void setup() {

  Serial.begin(9600);
  Serial.setDebugOutput(true);

  // Iniciar conexión WiFi

  wifiMulti.addAP(ssid, password);
  
  // RGB LED

  pinMode(BLUEPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);

  // Pulsador

  pinMode(PULSADORPIN, INPUT);

  // PIR HC-SR501
  // Recomendable un tiempo de calentamiento al inicio

  pinMode(PIRPIN, INPUT);
  Serial.println("Calentando sensor PIR HC-SR501");
  rgbLED(4);                      // LED MAGENTA: indica tiempo de espera mientras sensor calienta
  delay(10000);
  Serial.println("Sensor OK.");
  rgbLED(3);                      // LED AZUL: indica estado de sensor listo para ser activado
}

void loop() {

  wifiMulti.run();
  
  // sensorON = 1 --> Alarma activada

  if (sensorON == 1) {
    valorSensorPIR = digitalRead(PIRPIN);
    if (valorSensorPIR == HIGH) {
      rgbLED(1);
      sendMsg("ALARMA","Presencia detectada.");
      tone(BUZZERPIN, FRECUENCIA);
      delay(DURACION);
      noTone(BUZZERPIN);
      rgbLED(0);                  // LED EN REPOSO, SIN COLOR
    } else {
      rgbLED(0);
    }
  }

  // Iniciar sistema de alarma

  valorPulsador = digitalRead(PULSADORPIN);

  if (valorPulsador == HIGH) {
    rgbLED(2);                    // LED VERDE: tiempo para "huir" de la acción del sensor
    sensorON = 1;
    delay(10000);                 // 10 segundos para que sensor comience a detectar
    Serial.println("**** SENSOR PIR HC SR501 ACTIVADO ****");
  }
}

void rgbLED (int color) {

  switch (color) {
    case 0:
      digitalWrite(REDPIN, 0);
      digitalWrite(GREENPIN, 0);
      digitalWrite(BLUEPIN, 0);
      break;
    case 1:
      digitalWrite(REDPIN, 255);
      digitalWrite(GREENPIN, 0);
      digitalWrite(BLUEPIN, 0);
      break;
    case 2:
      digitalWrite(REDPIN, 0);
      digitalWrite(GREENPIN, 255);
      digitalWrite(BLUEPIN, 0);
      break;
    case 3:
      digitalWrite(REDPIN, 0);
      digitalWrite(GREENPIN, 0);
      digitalWrite(BLUEPIN, 255);
      break;
    case 4: // Color MAGENTA
      digitalWrite(REDPIN, 255);
      digitalWrite(GREENPIN, 0);
      digitalWrite(BLUEPIN, 255);
      break;
    default:
      digitalWrite(REDPIN, 0);
      digitalWrite(GREENPIN, 0);
      digitalWrite(BLUEPIN, 0);
      break;
  } 
}

void sendMsg(String titulo,String mensaje) {
  
  WiFiClientSecure client;
  client.setInsecure();

  if(!client.connect(host,443)) {
    Serial.println("No se pudo conectar con el servidor");
    return;
  }

  String url = "/v2/pushes";
  String message = "{\"type\": \"note\", \"title\": \"" + titulo + "\", \"body\": \"" + mensaje + "\"}\r\n";

  Serial.print("requesting URL: ");
  Serial.println(url);

  // Enviar mensaje a móvil mediante PUSHBULLET.COM (tipo "nota")

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Access-Token: " + apiKey + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " +
               String(message.length()) + "\r\n\r\n");

  client.print(message);

  delay(2000);

  while (client.available() == 0);

  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }  
}
