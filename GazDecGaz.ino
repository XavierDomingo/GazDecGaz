#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

const int butPIN = 8;
const int ledPIN = 9;
const int buzPIN = 11;
const int PIN_RESET = 3;  // LCD1 Reset
const int PIN_SCE = 4;    // LCD2 Chip Select
const int PIN_DC = 5;     // LCD3 Dat/Command
const int PIN_SDIN = 6;   // LCD4 Data in
const int PIN_SCLK = 7;   // LCD5 Clk
              // LCD6 Vcc
              // LCD7 Vled
              // LCD8 Gnd
byte estaon = HIGH;
bool emergency = 0; // algo no va bien
bool alert = 0; // bocinazo + flash
bool ack = 0; // piloto ha apretado el botos de aceptado
bool emerTemp = 0; // se ha activado emergencia por Temteratura
bool emerHume = 0; // se ha actuvado emergencia por Humedad
bool emerhPa= 0; // se ha activado emergencia por altura -> O2
int minTemp = -5; //
int maxTemp = 45 ; // probleams de golpe de calor
int minHume = 10; // problemas de respiración
int maxHume = 60; // bochorno
int maxhPa = 1044; //max presiónregistrada
int minhPa = 700; //presion a 3000 metros HIPOXIA

Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);

void setup() {
  Serial.begin(9600);
  pinMode(ledPIN, OUTPUT); // pin led es salida
  pinMode(buzPIN, OUTPUT); // pin Buz en salida
  pinMode(butPIN, INPUT); // pin boton es entrada
  display.begin();
  // init done
  display.setContrast(50);
  display.setRotation(0);
  display.clearDisplay();   // clears the screen and buffer
  // text display tests
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("AIR Quality");
  display.setTextColor(WHITE, BLACK); // 
  display.println("Javier Domingo");
  display.setTextColor(BLACK); // 
  display.println("V.0.1 nov 2023");
  display.println("USE @ YOUR RISK");
  display.println("Nano ATmega328");
  display.display();
  
  delay(5000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("Oxigen");
  display.println("Temperature");
  display.println("Air Qty(1-5)");
  display.println("Organ  ic Comp.");
  display.println("CO2 concentr.");
  display.println("Gas Status Flag");
  display.display();
  delay(5000);
  tone(buzPIN, 1000, 200);
  delay (500);
  noTone(buzPIN);
  Serial.println(F("BME280 Sensor event test"));
  if (!bme.begin()) {
    Serial.println(F("No se ha encontrado el sensor BME280"));
    while (1) delay(10);
  }
  bme_temp->printSensorDetails();
  bme_pressure->printSensorDetails();
  bme_humidity->printSensorDetails();
  
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.println("O2: 2ppm EXCEL");
  display.println("T: 123456789C");
  display.println("AQi: 1 EXCELL");
  display.println("OrC: 3 LOW");
  display.println("CO2: 123456ppm");
  display.println("GasSt: 1 EXCELL");
  display.display();
  delay(2000);
  display.clearDisplay();
  //delay(1000);
  display.setCursor(0, 0);
  display.println("_____________");
  display.display();
  digitalWrite(ledPIN, HIGH); // led ON
  flash();
  estaon = digitalRead(butPIN);
  if (estaon == LOW) {
    // ESPERAMOS ANTES DE COMPROBAR NUEVAMENTE
   delay(50);
   if (estaon == LOW ) {
      // ENTONCES apagamso el  LED
      ack=1;
    }
  }
}

void  flash() {
    //{period}: Periodo de Tiempo en el cual se va a ejecutar esta tarea
    unsigned long period=500; //En Milisegundos
    static unsigned long previousMillis=0;
    if(((millis()-previousMillis)>period)&&(ack=0)){//si priodo y el piloto no ha dicho que basta
      if (alert=0){
        digitalWrite(ledPIN, HIGH); // led ON
        tone(buzPIN, 880, 200);
        alert=1;
      }
      else {
        noTone(buzPIN);
        digitalWrite(ledPIN, HIGH); // led ON
      }
        previousMillis += period;
    }  
} // fin flash

void leebme280(){
    unsigned long period=5000; //En Milisegundos
    static unsigned long previousMillis=0;
    if((millis()-previousMillis)>period){//leer cada 5 segundos
      sensors_event_t temp_event, pressure_event, humidity_event;
      bme_temp->getEvent(&temp_event);
      bme_pressure->getEvent(&pressure_event);
      bme_humidity->getEvent(&humidity_event);
      Serial.print(F("Temperatura = "));
      Serial.print(temp_event.temperature);
      Serial.println(" *C");
      Serial.print(F("Humedad = "));
      Serial.print(humidity_event.relative_humidity);
      Serial.println(" %");
      Serial.print(F("Presion = "));
      Serial.print(pressure_event.pressure);
      Serial.println(" hPa");
      Serial.println();
      previousMillis += period;
    } 
}// leebme220