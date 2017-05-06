#include "U8glib.h"
#include <Arduino.h>
#include <eRCaGuy_analogReadXXbit.h>

U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8);    // SPI Com: SCK = 13, MOSI = 11, CS = 10, CD = 9, RST = 8

// temperatureSensor + oversampling
eRCaGuy_analogReadXXbit adc;
const uint8_t temperatureSensorPin = A1; //analogRead pin
const uint8_t bits_of_precision = 12;
const unsigned long num_samples = 10;
const float MAX_READING_12_bit = 2^bits_of_precision;

// temperatureSensor
long tempSensorValue = 0;
int tempSensorAnalogPin = A1;
float currentTemperature = 0.0;

//joystick
int joystickValue = 0;
int joystickAnalogPin = A0;

//relay
int relayPin = 4;

//target temp
int targetTemperature = 60;

//timer
long timerTimestamp = 0;

//offset
double offset=0.5;

//resistance calculation
const int Vin= 5;
float Vout= 0;
const float Rknown= 116.5;
float buffer= 0;

void setup() 
{
  setupScreen();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // HIGH to wyłączony
//  Serial.begin(9600);
}

void loop() 
{
  joystickValue = analogRead(joystickAnalogPin);
  tempSensorValue = (long)getAnalogInput();
  currentTemperature = tempSensorToCelsius(tempSensorValue);
  switchRelayIfTargetReached();
  listenJoystickForTargetTemperature();

  u8g.firstPage(); 
  do {
    draw();
  }while( u8g.nextPage() );
  delay(200);
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
  u8g.print("RAWTEMP10: ");
  u8g.print(tempSensorValue);
  if(isTempSensorConnected()){
    u8g.print(currentTemperature);
  }else{
    u8g.print("OFF");
  }
  u8g.setPrintPos( 0, 40 );
  u8g.print("VOLTS: ");
  u8g.print(tempSensorValue/MAX_READING_12_bit*5.0);

  u8g.setPrintPos( 0, 50 );
  u8g.print("OHMS: ");
  u8g.print(resistance(tempSensorValue));

  u8g.setPrintPos( 0, 60 );
  u8g.print("TEMP: ");
  u8g.print(currentTemperature);

  //timer
  u8g.setPrintPos( 0, 40 );
  u8g.print("RUNNING: ");
  u8g.print((millis()-timerTimestamp)/1000/60);
  u8g.print(":");
  if((millis()-timerTimestamp)/1000%60 < 10){
    u8g.print("0");
  }
  u8g.print((millis()-timerTimestamp)/1000%60);
}

float resistanceToTemperature(float input){
  return 2.6048*input-260.84;
}
float resistance(long raw_input){
  buffer = raw_input * Vin;
  Vout = buffer/(2^bits_of_precision);
  buffer = (Vin/Vout) -1;
  return Rknown * buffer;
}

float getAnalogInput(){
  //float V = analog_reading/MAX_READING_12_bit*5.0; //voltage
  return adc.analogReadXXbit(temperatureSensorPin,bits_of_precision,num_samples);
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

bool relayStatus(void){
  if(digitalRead(relayPin) == 0){
    return true;
  }else{
    return false;
  }
}

bool isTempSensorConnected(void){
  return (currentTemperature > 0 && currentTemperature < 120);
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


float tempSensorToCelsius(float input){
  return resistanceToTemperature(resistance(input));
}
