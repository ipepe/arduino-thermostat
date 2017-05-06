#include "U8glib.h"
#include <Arduino.h>
#include <eRCaGuy_NewAnalogRead.h>

 
U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8);    // SPI Com: SCK = 13, MOSI = 11, CS = 10, CD = 9, RST = 8

//joystick
int joystickValue = 0;
int joystickAnalogPin = A0;

//czujnik temperatury
long tempSensorValue = 0;
int tempSensorAnalogPin = A1;
double currentTemperature = 0.0;

//oversampling
byte bitsOfResolution = 12; //commanded oversampled resolution
unsigned long numSamplesToAvg = 1; //number of samples AT THE OVERSAMPLED RESOLUTION that you want to take and average
ADC_prescaler_t ADCSpeed = ADC_FAST;

//relay
int relayPin = 4;

//target temp
int targetTemperature = 60;

//timer
long timerTimestamp = 0;

//offset
double offset=0.5;

void setup(void) {  
  setupScreen();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // HIGH to wyłączony

  adc.setADCSpeed(ADCSpeed);
  adc.setBitsOfResolution(bitsOfResolution);
  adc.setNumSamplesToAvg(numSamplesToAvg);

  Serial.begin(9600);
}

void loop(void) {
  joystickValue = analogRead(joystickAnalogPin);
  tempSensorValue = adc.newAnalogRead(tempSensorAnalogPin);
  Serial.println(tempSensorValue);
  currentTemperature = tempSensorToCelsius(tempSensorValue);

  switchRelayIfTargetReached();
  listenJoystickForTargetTemperature();
  
  u8g.firstPage(); 
  do {
    draw();
  }while( u8g.nextPage() );
  delay(300);
}

void draw(void){
  //relayStatus
  u8g.setPrintPos(0, 10);
  u8g.print("RELAY: ");
  if(relayStatus()){
    u8g.print("ON");  
  }else{
    u8g.print("OFF");
  }
  
  //target temp
  u8g.setPrintPos( 0, 20 );
  u8g.print("TARGET: ");
  u8g.print(targetTemperature);

  //temp sensor
  u8g.setPrintPos( 0, 30 );
  u8g.print("TEMP: ");
  if(isTempSensorConnected()){
    u8g.print(currentTemperature);
  }else{
    u8g.print("OFF");
  }

  //joystick
  u8g.setPrintPos( 0, 40 );
  u8g.print("RUNNING: ");
  u8g.print((millis()-timerTimestamp)/1000/60);
  u8g.print(":");
  if((millis()-timerTimestamp)/1000%60 < 10){
    u8g.print("0");
  }
  u8g.print((millis()-timerTimestamp)/1000%60);

}

void listenJoystickForTargetTemperature(void){
  if(joystickValue > 700 && joystickValue < 900){
    targetTemperature++;
    delay(300);
  }else if (joystickValue > 300 && joystickValue < 500 ) {
    targetTemperature--;
    delay(300);
  }else if (joystickValue > 500 && joystickValue < 700 ) {
    targetTemperature = targetTemperature + 10;
    delay(300);
  }else if (joystickValue > 100 && joystickValue < 300 ) {
    resetTimer();
  }else if (joystickValue < 100){
    targetTemperature = targetTemperature - 10;
    delay(300);
  }
}

void resetTimer(void){
  timerTimestamp = millis();
}

void switchRelayIfTargetReached(void){
  if(isTempSensorConnected()){  
    if(currentTemperature > targetTemperature+offset){
      digitalWrite(relayPin, HIGH); //HIGH is OFF
    }else if (currentTemperature < targetTemperature-offset){
      digitalWrite(relayPin, LOW); //LOW is ON
    }
  }else{
    digitalWrite(relayPin, HIGH); //HIGH is OFF
  }
}

bool relayStatus(void){
  if(digitalRead(relayPin) == 0){
    return true;
  }else{
    return false;
  }
}

bool isTempSensorConnected(void){
  return (tempSensorValue > 400*4 && tempSensorValue < 700*4);
}

double tempSensorToCelsius(float input){
 input = (1023.0/(1023.0 - (input/4.0))-1.0)*100.0;
 return (((input/100)-1)/(3.9083*0.001))-12.7;
}


void setupScreen(void){
  u8g.setContrast(0); // Config the contrast to the best effect
  u8g.setRot180();// rotate screen, if require
  if ( u8g.getMode() == U8G_MODE_R3G3B2 )
  {u8g.setColorIndex(255);}else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
  {u8g.setColorIndex(3);}else if ( u8g.getMode() == U8G_MODE_BW ) 
  {u8g.setColorIndex(1);}else if ( u8g.getMode() == U8G_MODE_HICOLOR )
  {u8g.setHiColorByRGB(255,255,255);}
  u8g.setFont(u8g_font_7x13B);
}
 
