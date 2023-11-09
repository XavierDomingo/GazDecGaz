// algoritmo de funcionamiento. lee cada 17 segundos 280 y cada 27 el EN160.
// si valores fuera de limites activa alarma ( buzzer y led ) hasta que piloto no ack
// mientras valor fuera de rango texto en fondo BLACK y sigue en estado emergencia. 
// si vuelve niveles normales se desactiva emenrgencia de ese parametro.
// CO2 y Oxigeno activan Pantalla Alarma fija

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_BME280.h>
#include <ScioSense_ENS160.h>

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
bool ackhPa = 0; // piloto ha apretado el botos de aceptado en hPa alarm
bool ackCo2 = 0; // piloto ha apretado el botos de aceptado en Co2alarm
bool emerTemp = 0; // se ha activado emergencia por Temteratura
bool emerHume = 0; // se ha actuvado emergencia por Humedad
bool emerhPa= 0; // se ha activado emergencia por altura -> O2
bool emerAQI= 0; // alerta > 3 activa Air quality index alert
bool emerTVOC= 0; //Total volatile organic compounds > 750
bool emereCO2= 0; // > 1000 activa alerta
int maxhPa = 1044; //max presión registrada
int minhPa = 700; //presion a 3000 metros HIPOXIA
int curTemp = 0; //temperatura actual
int curHumi = 0; //humedad actual
int curhPa = 0; //hPa actual
int curCo2 = 0; //Co2 level actual
int curQAI = 0; // Quality Air indicator
int curTVOC = 0; //calidad particulas 

Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);
ScioSense_ENS160 ens160(ENS160_I2CADDR_0);

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
  display.println("CO2 ALERT");
  display.println("HYPOXIA ALERT");
  display.setTextColor(WHITE, BLACK); // 
  display.println("Javier Domingo");
  display.setTextColor(BLACK); // 
  display.println("V.0.1 nov 2023");
  display.println("USE @ YOUR RISK");
  display.println("Nano ATmega328");
  display.display();
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

  Serial.println("------------------------------------------------------------");
  Serial.println("ENS160 - Digital air quality sensor");
  Serial.println();
  Serial.println("Sensor readout in standard mode");
  Serial.println();
  Serial.println("------------------------------------------------------------");
  delay(1000);

  Serial.print("ENS160...");
  ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");
  if (ens160.available()) {
    // Print ENS160 versions
    Serial.print("\tRev: "); Serial.print(ens160.getMajorRev());
    Serial.print("."); Serial.print(ens160.getMinorRev());
    Serial.print("."); Serial.println(ens160.getBuild());
    Serial.print("\tStandard mode ");
    Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
  }
}//setup


void loop() {
  leebme280();
  leeens160();
  flash();
  ackButton();
  muestra();
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
    }  //millis
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
      // test emergencia
      if (temp_event.temperature < -5 || temp_event.temperature > 40){
        emerTemp = 1; // se ha activado emergencia por Temteratura
      }
      else {
        emerTemp = 0;
      }
      if (pressure_event.pressure < minhPa || pressure_event.pressure > maxhPa){
        emerhPa = 1; // se ha activado emergencia por presión
      }
      else {
        emerhPa = 0;
      }
    } //millis
}// finlee 280

void ackButton(){  
  estaon = digitalRead(butPIN);
  if (estaon == LOW) {
    // ESPERAMOS ANTES DE COMPROBAR NUEVAMENTE
   delay(50);
   if (estaon == LOW ) {
      ack=1;
    }
  }
}// ackButton

void muestra(){
  if ((emerhPa==1)&&(ackhPa==0)){ // pantalla emergencia Oxigeno
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.setTextColor(WHITE, BLACK);
    display.println("OXIGEN");
    display.setTextColor(BLACK);
    display.setTextSize(2);
    display.print("hPa: ");
    display.println(curhPa);
    display.setTextSize(1);
    display.print("----ALERT----");
    display.display();
  }
  else{ 
   if ((emereCO2==1)&&(ackCo2==0)){ //pantalla emergencia CO2
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(2);
        display.setTextColor(WHITE, BLACK);
        display.println("-CO2-");
        display.setTextColor(BLACK);
        display.setTextSize(2);
        display.print("CO2: ");
        display.println(curCo2);
        display.setTextSize(1);
        display.print("----ALERT----");
        display.display();
   }
   else{ //pantalla default
       display.clearDisplay();
       display.setTextSize(1);
       display.setCursor(0,0);
       if (emerhPa==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("OXI: ");

     
       display.println("CO2: 123456ppm");
       if (emerTemp==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.println("Temp: 123456789C");
       display.println("AQi: 1 EXCELL");
       display.println("TVOC: 1 EXCELL");
       display.display();
   } 
  }
  
    

}//fin muestra

void leeens160(){
  unsigned long period=7000; //En Milisegundos
  static unsigned long previousMillis=0;
  if((millis()-previousMillis)>period){//leer cada 7 segundos
   if (ens160.available()) {
    ens160.measure(true);
    ens160.measureRaw(true);
    Serial.print("AQI: ");Serial.print(ens160.getAQI());Serial.print("\t");
    Serial.print("emerTVOC: ");Serial.print(ens160.getTVOC());Serial.print("ppb\t");
    Serial.print("eCO2: ");Serial.print(ens160.geteCO2());Serial.print("ppm\t");
    }
    previousMillis += period;
  }//millis
}// leeens160
