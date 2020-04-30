/*
- RTC 
- Display Time and temp
- Study Prescaler
- Add user input for setting min and max temp (using remote)
- Add light that dims and brightens according to brightness in room
*/

/*
DHT
Data - 2

RTC
SDA - A4
SCL - A5

RGB LED
Red - 3
Green -5
Blue - 6

LCD
RS - 13
RW - GND
E - 12
D4 - 11
D5 - 10
D6 - 9
D7 - 8
A - 220ohm pullup (5V)
K - GND
*/

#include <Arduino.h>
#include "DHT.h"
#include "LiquidCrystal.h"
#include "RTClib.h"

//DHT11 (read every 2 seconds)
#define DHTPIN 2     // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHT11);
unsigned long lastReadTime = 0; //stores time of last read
uint16_t readDHTInterval = 2000; //2s interval between reads
float currentTemp = 0;

//Reference temps for light colors, min is all blue, max is all red
uint16_t minTemp = 22;
uint16_t maxTemp = 27;

//RBG LED
#define LED_R 3
#define LED_G 5
#define LED_B 6

uint16_t redBrightness;
uint16_t greenBrightness;
uint16_t blueBrightness;

//LCD Display (1602A)
LiquidCrystal lcd(13,12,11,10,9,8); //initialize
char line0[17];
char line1[17];

//RTC (DS1307)
RTC_DS1307 rtc;


void setup() {
  Serial.begin(9600);
  dht.begin();

  //set RBG pins
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  //LCD Display
  lcd.begin(16,2);
  sprintf(line0, "Temp: ");
  lcd.print(line0);

  //Sets up RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is not running");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void updateLCD(){
  char strTemp[8];
  dtostrf(currentTemp, 5, 2, strTemp);
  sprintf(line0, "Temp: %s%cC",strTemp, (char)223);

  DateTime currentTime = rtc.now();
  sprintf(line1, "%02d:%02d:%02d", currentTime.hour(), currentTime.minute(), currentTime.second());
  Serial.println(line1);
  lcd.setCursor(0,0);
  lcd.print(line0);
  lcd.setCursor(0,1);
  lcd.print(line1);
}

void readTemp(){
    //Read temperature
    currentTemp = dht.readTemperature();
    //float h = dht.readHumidity();

    if (isnan(currentTemp)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      lastReadTime = millis();
      return;
    }

    Serial.print(F("Temperature: "));
    Serial.print(currentTemp);
    Serial.println(F("°C"));
}

void tempSim(){
  currentTemp += 0.10;

  if (currentTemp >= maxTemp + 3){
    currentTemp = 20;
  }

  Serial.print("Temperature: ");
  Serial.print(currentTemp);
  Serial.println("°C");
}

void adjustLED(){
  if (currentTemp < minTemp){ //if temp is less then min reference temp, set t0 be completely blue
    redBrightness = 0;
    greenBrightness = 0;
    blueBrightness = 255;

  }else if (currentTemp > maxTemp){ //if temp is greater than maxTemp, make LED completely red
    redBrightness = 255;
    greenBrightness = 0;
    blueBrightness = 0;

  }else{ //else make it come inbetween brightness
    redBrightness = (currentTemp - minTemp)/(maxTemp - minTemp) * 255;
    blueBrightness = 255 - redBrightness;
   // greenBrightness = 255 - redBrightness*0.25;
  }

  analogWrite(LED_R, redBrightness);
  analogWrite(LED_G, greenBrightness);
  analogWrite(LED_B, blueBrightness);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastReadTime >= readDHTInterval){
    readTemp();
    //tempSim();
    adjustLED();

    lastReadTime = millis();
  }

  updateLCD();
}