#include <ESP8266WiFi.h>
#include "AdafruitIO_WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// --- CONFIGURACIÓN DE TU RED Y ADAFRUIT IO ---
#define WIFI_SSID       "ALFONSO"
#define WIFI_PASS       "CaterpillaR-2021"

#define AIO_USERNAME    "TU_USUARIO_ADAFRUIT"
#define AIO_KEY         "TU_ACTIVE_KEY_ADAFRUIT"

// --- CONFIGURACIÓN DEL SENSOR ---
#define ONE_WIRE_BUS    D7   // Pin D7 en la serigrafía del NodeMCU

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Inicialización del cliente Adafruit IO
AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, WIFI_SSID, WIFI_PASS);

// Feed donde enviaremos la temperatura
AdafruitIO_Feed *feedTemp = io.feed("terraza-temp");

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 30000; // Envío cada 30 segundos

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println("\n--- Iniciando NodeMCU ESP8266 + DS18B20 (Terraza) ---");

  // Iniciar bus DS18B20
  sensors.begin();

  // Conectar a la red Wi-Fi y Adafruit IO
  Serial.print("Conectando a Adafruit IO...");
  io.connect();

  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n¡Conectado exitosamente a Adafruit IO!");
}

void loop() {
  // Mantener viva la conexión con la nube
  io.run();

  // Comprobar temporizador de envío
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = millis();

    // Pedir temperatura al sensor DS18B20
    sensors.requestTemperatures(); 
    float temp = sensors.getTempCByIndex(0);

    // Validar si la lectura es correcta (-127 es error de conexión/resistencia)
    if (temp == DEVICE_DISCONNECTED_C) {
      Serial.println("Error: Sonda DS18B20 no detectada. Revisa conexiones y resistencia de 4.7k.");
      return;
    }

    Serial.printf("Temperatura Terraza Exterior -> %.2f °C\n", temp);

    // Enviar valor a Adafruit IO
    feedTemp->save(temp);
  }
}