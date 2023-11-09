// algoritmo de funcionamiento. lee cada 17 segundos 280 y cada 23 el EN160.
// si valores fuera de limites activa alarma ( buzzer y led ) hasta que piloto no ack
// mientras valor fuera de rango texto en fondo BLACK y sigue en estado emergencia. 
// si vuelve niveles normales se desactiva emenrgencia de ese parametro.
// CO2 y Oxigeno activan Pantalla Alarma fija

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Wire.h>
//#include <SPI.h>
#include <ErriezBMX280.h>
#include <ScioSense_ENS160.h>
ErriezBMX280 bmx280 = ErriezBMX280(0x76);

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
bool alert = 0; // bocinazo + flash comtrol de tiempo millis
bool ackhPa = 0; // piloto ha apretado el boton de aceptado en hPa alarm
bool ackCo2 = 0; // piloto ha apretado el boton de aceptado en Co2alarm
bool emerTemp = 0; // se ha activado emergencia por Temteratura
bool emerhPa= 0; // se ha activado emergencia por altura -> O2
bool emerAQI= 0; // alerta > 3 activa Air quality index alert
bool emerTVOC= 0; //Total volatile organic compounds > 750
bool emerCo2= 0; // > 1000 activa alerta
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
  // init DISPLAY NOKIA 5110
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
  digitalWrite(ledPIN, HIGH); // led ON
  tone(buzPIN, 1000, 200);
  delay (500);
  noTone(buzPIN);
  delay (10000); //10 segundos de screenshot credits 
  digitalWrite(ledPIN, LOW); // led ON 10 segundos
  // init BMP280
  while (!bmx280.begin()) {
        Serial.println(F("Error: Could not detect sensor"));
        delay(1000);
  }
  //init ens160
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
//---------------------------------------------------------

void loop() {
  leebme280();
  leeens160();
  ackButton();
  if ((emerhPa==1)||(emerCo2==1)){ //si hay emergencia 
     if ((ackhPa==0)&&(ackCo2==0){ //si no hay ack's del Piloto
       flash(); //pita y flash
     }
  }
  muestra();
}

void  flash() {
    //{period}: Periodo de Tiempo en el cual se va a ejecutar esta tarea
    unsigned long period=500; //En Milisegundos
    static unsigned long previousMillis=0;
    if(((millis()-previousMillis)>period){//si priodo y el piloto no ha dicho que basta
      if (alert=0){
        digitalWrite(ledPIN, HIGH); // led ON
        tone(buzPIN, 880, 200);
        alert=1;
      }
      else {
        noTone(buzPIN);
        digitalWrite(ledPIN, LOW); // led Off
        alert=0;
      }
        previousMillis += period;
    }  //millis
} // fin flash

void leebme280(){
    unsigned long period=17000; //En Milisegundos
    static unsigned long previousMillis=0;
    if((millis()-previousMillis)>period){//leer cada 5 segundos
      Serial.print(F("Temperature: "));
      Serial.print(bmx280.readTemperature());
      Serial.println(" C");
      curTemp=bmx280.readTemperature();
      if (bmx280.getChipID() == CHIP_ID_BME280) {
        Serial.print(F("Humidity:    "));
        Serial.print(bmx280.readHumidity());
        Serial.println(" %");
        curHumi=bmx280.readHumidity();
        }
      Serial.print(F("Pressure:    "));
      Serial.print(bmx280.readPressure() / 100.0F);
      Serial.println(" hPa")
      curhPa=bmx280.readPressure() / 100.0F;
      previousMillis += period;
      // test emergencia
      if (curTemp < -5 && curTemp > 40){
        emerTemp = 1; // se ha activado emergencia por Temteratura inf -5 o sup 40 Cº
      }
      else {
        emerTemp = 0;
      }
      if (curhPa < minhPa && curhPa > maxhPa){
        emerhPa = 1; // se ha activado emergencia por presión
      }
      else {
        emerhPa = 0;
        ackhPa =0;
      }
    } //millis
}// finlee 280
//----------------------------------
void leeens160(){
  unsigned long period=23000; //En Milisegundos
  static unsigned long previousMillis=0;
  if((millis()-previousMillis)>period){//leer cada 7 segundos
   if (ens160.available()) {
    ens160.measure(true);
    ens160.measureRaw(true);
    Serial.print("AQI: ");Serial.print(ens160.getAQI());Serial.print("\t");
    Serial.print("emerTVOC: ");Serial.print(ens160.getTVOC());Serial.print("ppb\t");
    Serial.print("eCO2: ");Serial.print(ens160.geteCO2());Serial.print("ppm\t");
    curCo2= ens160.getAQI();
    curQAI= ens160.getTVOC(); 
    curTVOC= ens160.geteCO2();
    }
    // EValuar emergencia 
    //bool emerAQI= 0; // alerta >= 3 activa Air quality index alert
    //bool emerTVOC= 0; //Total volatile organic compounds > 750
    //bool emerCo2= 0; // > 1000 activa alerta  
    if (curCo2 > 1000) {
      emerCo2=1;
    else{
      emerCo2=0;
      ackC02=0;
      }
    if (curQAI > 2) {
      emerQAI=1;
    else{
      emerQAI=0;
      }
    if (curTVOC > 750) {
      emerTVOC=1;
    else{
      emerTVOC=0;
      }      
    previousMillis += period;
  }//millis
}// leeens160
//---------------------------------------------

void ackButton(){  
  estaon = digitalRead(butPIN);
  if (estaon == LOW) {
    // ESPERAMOS ANTES DE COMPROBAR NUEVAMENTE
   delay(50);
   if (estaon == LOW ) {
     if (emerhPa==1)  {
     ackhPa=1;
     }
     else  {
       ackhPa=0;
       if (emerCo2==1){
        ackC02=1;  
       }
       else {
         ackCo2=0;
       }
     }
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
       display.println(curhPa);
       //---------
       if (emerCo2==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("C02: ");
       display.println(curCo2);
       //-----------------
       if (emerTemp==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("Temp: ");
       display.print(curTemp);
       display.println(" Cº");
     //-----------------
       display.print("Hume: ");
       display.print(curHume);
       display.println(" %");
     //-----------------
       if (emerAQI==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("AirQI: ");
       display.println(curAQI);
    //-----------------
       if (emerAQI==1){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("TVOC: ");
       display.println(emerTVOC);
     
       display.display();
   } 
  } 

}//fin muestra

