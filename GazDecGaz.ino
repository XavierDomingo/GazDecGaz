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
#include <DFRobot_ENS160.h>

ErriezBMX280 bmx280 = ErriezBMX280(0x76);
DFRobot_ENS160_I2C ENS160(&Wire, /*I2CAddr*/ 0x53);

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
bool alert = false; // bocinazo + flash comtrol de tiempo millis
bool ackhPa = false; // piloto ha apretado el boton de aceptado en hPa alarm
bool ackCo2 = false; // piloto ha apretado el boton de aceptado en Co2alarm
bool emerTemp = false; // se ha activado emergencia por Temteratura
bool emerhPa= false; // se ha activado emergencia por altura -> O2
bool emerAQI= false; // alerta > 3 activa Air quality index alert
bool emerTVOC= false; //Total volatile organic compounds > 750
bool emerCo2= false; // > 1000 activa alerta
int maxhPa = 1044; //max presión registrada
int minhPa = 700; //presion a 3000 metros HIPOXIA
int curTemp = 0; //temperatura actual
int curHumi = 0; //humedad actual
int curhPa = 0; //hPa actual
int curCo2 = 0; //Co2 level actual
int curQAI = 0; // Quality Air indicator
int curTVOC = 0; //calidad particulas 

Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);


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
  while( NO_ERR != ENS160.begin() ){
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
   }
  ENS160.setPWRMode(ENS160_STANDARD_MODE);
   /**
   * Users write ambient temperature and relative humidity into ENS160 for calibration and compensation of the measured gas data.
   * ambientTemp Compensate the current ambient temperature, float type, unit: C
   * relativeHumidity Compensate the current ambient humidity, float type, unit: %rH
   */
   Serial.print(F("Temperature inicial : "));
   Serial.print(bmx280.readTemperature());
   Serial.println(" C");
   curTemp=bmx280.readTemperature();
   if (bmx280.getChipID() == CHIP_ID_BME280) {
      Serial.print(F("Humidity inicial:    "));
      Serial.print(bmx280.readHumidity());
      Serial.println(" %");
      curHumi=bmx280.readHumidity();
      }
  ENS160.setTempAndHum(curTemp, curHumi);
    
}//setup
//---------------------------------------------------------

void loop() {
  leebme280();
  leeens160();
  ackButton();
  if ((emerhPa==true)||(emerCo2==true)){ //si hay emergencia 
     if ((ackhPa==false)&&(ackCo2==false)){ //si no hay ack's del Piloto
       flash(); //pita y flash
     }
  }
  muestra();
}

void  flash() {
    //{period}: Periodo de Tiempo en el cual se va a ejecutar esta tarea
    static long ultimo_cambio = 0; 
    alert = false;
    if(((millis()- ultimo_cambio) > 1000)){//si priodo y el piloto no ha dicho que basta
      ultimo_cambio= millis();
      if (alert=false){
        digitalWrite(ledPIN, HIGH); // led ON
        tone(buzPIN, 880, 200);
        alert=true;
      }
      else {
        noTone(buzPIN);
        digitalWrite(ledPIN, LOW); // led Off
        alert=false;
      }
    }  //millis
} // fin flash

void leebme280(){
    static long ultimo_cambio = 0; 
    if((millis()- ultimo_cambio)>17000){//leer cada 17 segundos
      ultimo_cambio= millis();
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
      Serial.println(" hPa");
      curhPa=bmx280.readPressure() / 100.0F;
      // test emergencia
      if (curTemp < -5 && curTemp > 40){
        emerTemp = true; // se ha activado emergencia por Temteratura inf -5 o sup 40 Cº
      }
      else {
        emerTemp = false;
      }
      if (curhPa < minhPa && curhPa > maxhPa){
        emerhPa = true; // se ha activado emergencia por presión
      }
      else {
        emerhPa = false;
        ackhPa =false;
      }
    } //millis
}// finlee 280
//----------------------------------
void leeens160(){
   static long ultimo_cambio = 0; 
    if((millis()- ultimo_cambio)>23000){//leer cada 23 segundos
    ultimo_cambio= millis();
     /**
     * Get the sensor operating status
     * Return value: 0-Normal operation, 
     *         1-Warm-Up phase, first 3 minutes after power-on.
     *         2-Initial Start-Up phase, first full hour of operation after initial power-on. Only once in the sensor’s lifetime.
     * note: Note that the status will only be stored in the non-volatile memory after an initial 24h of continuous
     *       operation. If unpowered before conclusion of said period, the ENS160 will resume "Initial Start-up" mode
     *       after re-powering.
     */
  uint8_t Status = ENS160.getENS160Status();
  Serial.print("Sensor operating status : ");
  Serial.println(Status);

  /**
   * Get the air quality index
   * Return value: 1-Excellent, 2-Good, 3-Moderate, 4-Poor, 5-Unhealthy
   */
  curQAI= ENS160.getAQI();
  Serial.print("Air quality index : ");
  Serial.println(curQAI);

  /**
   * Get TVOC concentration
   * Return value range: 0–65000, unit: ppb
   */
  curTVOC= ENS160.getTVOC();
  Serial.print("Concentration of total volatile organic compounds : ");
  Serial.print(curQAI);
  Serial.println(" ppb");

  /**
   * Get CO2 equivalent concentration calculated according to the detected data of VOCs and hydrogen (eCO2 – Equivalent CO2)
   * Return value range: 400–65000, unit: ppm
   * Five levels: Excellent(400 - 600), Good(600 - 800), Moderate(800 - 1000), 
   *               Poor(1000 - 1500), Unhealthy(> 1500)
   */
  curCo2= ENS160.getECO2();
  Serial.print("Carbon dioxide equivalent concentration : ");
  Serial.print(curCo2);
  Serial.println(" ppm");
   
    // EValuar emergencia 
    //bool emerAQI= 0; // alerta >= 3 activa Air quality index alert
    //bool emerTVOC= 0; //Total volatile organic compounds > 750
    //bool emerCo2= 0; // > 1000 activa alerta  
    if (curCo2 > 1000) {
      emerCo2=true;
    }
    else{
      emerCo2=false;
      ackCo2=false;
      }
    if (curQAI > 2) {
      emerAQI=true;
    }
    else{
      emerAQI=0;
      
      }
    if (curTVOC > 750) {
      emerTVOC=true;
      }
    else{
      emerTVOC=false;
      }  
  }//millis
}// leeens160
//---------------------------------------------

void ackButton(){  
  estaon = digitalRead(butPIN);
  if (estaon == LOW) {
    // ESPERAMOS ANTES DE COMPROBAR NUEVAMENTE
   delay(50);
   if (estaon == LOW ) {
     if (emerhPa==true)  {
     ackhPa=true;
     }
     else  {
       ackhPa=false;
       if (emerCo2==true){
        ackCo2=true;  
       }
       else {
         ackCo2=false;
       }
     }
   }
  }
}// ackButton

void muestra(){
  if ((emerhPa==true)&&(ackhPa==false)){ // pantalla emergencia Oxigeno
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
   if ((emerCo2==1)&&(ackCo2==0)){ //pantalla emergencia CO2
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
       if (emerCo2==true){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("C02: ");
       display.println(curCo2);
       //-----------------
       if (emerTemp==false){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("Temp: ");
       display.print(curTemp);
       display.println(" Cº");
     //-----------------
       display.print("Hume: ");
       display.print(curHumi);
       display.println(" %");
     //-----------------
       if (emerAQI==true){
         display.setTextColor(WHITE, BLACK);}
       else {
         display.setTextColor(BLACK);
       }
       display.print("AirQI: ");
       display.println(curQAI);
    //-----------------
       if (emerAQI==true){
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
