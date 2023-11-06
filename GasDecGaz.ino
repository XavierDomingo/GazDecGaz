#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


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

Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);

void setup() {
  Serial.begin(9600);
  pinMode(ledPIN, OUTPUT);
  pinMode(buzPIN, OUTPUT);
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
  display.println("V.0.1");
  display.println("ENS150-Q120");
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
  display.println("Organic Comp.");
  display.println("CO2 concentr.");
  display.println("Gas Status Flag");
  display.display();
  delay(5000);
  tone(buzPIN, 1000, 200);
  delay (500);
  noTone(buzPIN);
  
}

void loop() {
  digitalWrite(ledPIN, LOW); // led OFF
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
  tone(buzPIN, 880, 200);
  delay(200);
  noTone(buzPIN);
}
