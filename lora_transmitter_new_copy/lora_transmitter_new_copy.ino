
/*  
  ==================================================================== 
  Utilization: ARGUS III Lora Transceiver with GUI Test (Transmitter)
  Programmer 1: Anthony B. Duyan Jr. (RedID: 824809096)
  Programmer 2: Joseph Solis (RedID: 824761763)
  Date: 3/4/23
  ==================================================================== 
*/
#include <Wire.h>
#include "max6675.h"
#include <LowPower.h> 

int SO = 12;
int CS = 10;
int sck = 13;
MAX6675 module(sck, CS, SO);

const int flamePin1 = 6;
const int flamePin2 = 5;
const int flamePin3 = 4;
const int flamePin4 = 7;

int Flame1 = HIGH;
int Flame2 = HIGH;
int Flame3 = HIGH;
int Flame4 = HIGH;

// Timing delays without delay function
long previousMillis = 0;        // will store last time it was updated
long interval = 1000;           // interval at which to count (milliseconds)

const String sndMessage = "AT+SEND=0,";
//  Setup code to run once.
void setup() 
{
  
//Save Power by writing all Digital IO LOW 
  for (int i = 0; i < 20; i++)
  {
    pinMode(i, OUTPUT);
  }

  Serial.begin(115200);     // Default baudrate of module is 115200

  //Initialize I2C Communication
  Wire.begin();
  //Configure HDC1080
  Wire.beginTransmission(0x40);
  Wire.write(0x02);
  Wire.write(0x90);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(20);

  pinMode(flamePin1, INPUT);
  pinMode(flamePin2, INPUT);
  pinMode(flamePin3, INPUT);
  pinMode(flamePin4, INPUT);

}

void loop() 
{

  unsigned long currentMillis = millis();
  int flameCount = 0;
  
  if((currentMillis - previousMillis) > interval)
  {
    // Counter
    previousMillis = currentMillis;


    // Flame sensors
    String condition;
    

    Flame1 = digitalRead(flamePin1);
    Flame2 = digitalRead(flamePin2);
    Flame3 = digitalRead(flamePin3);
    Flame4 = digitalRead(flamePin4);
  


   

    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,SPI_OFF, USART0_OFF, TWI_OFF);
     if ((Flame1 == LOW) || (Flame2 == LOW) || (Flame3 == LOW) || (Flame4 == LOW))

    {
      condition = "FIRE";
      if (Flame1 == LOW)
        flameCount += 1;
      if (Flame2 == LOW)
        flameCount += 1;
      if (Flame3 == LOW)
        flameCount += 1;
      if (Flame4 == LOW) 
        flameCount += 1;        
                   
    }
    else
    {
      condition = "No fire";
      flameCount = 0;
    }

    // Thermocoupler
    float couplerNum = module.readCelsius();
    String coupler =  String(couplerNum);

    // HDC1080
    double temp;
    double h;
    h = readSensor(&temp);

    String humidity = String(h);
    String temperature = String(temp);
    String values = condition + " Temp:" + temperature + "C" + " Humidity:" + humidity
                  + "Coupler Temp:" + coupler + "C "
                  + "GUI:!" +  coupler + "@" + temperature + "$" + humidity + "&"
                  +  flameCount + "*";

    String msgLength = String(values.length());
    Serial.print(sndMessage + msgLength + "," + values + "\r\n");

  }
  
}

double readSensor(double* temperature)
{
  //holds 2 bytes of data from I2C Line
  uint8_t Byte[4];
  //holds the total contents of the temp register
  uint16_t temp;
  //holds the total contents of the humidity register
  uint16_t humidity;
  //Point to device 0x40 (Address for HDC1080)
  Wire.beginTransmission(0x40);
  //Point to register 0x00 (Temperature Register)
  Wire.write(0x00);
  //Relinquish master control of I2C line
  //pointing to the temp register triggers a conversion
  Wire.endTransmission();
  //delay to allow for sufficient conversion time
  delay(20);
  //Request four bytes from registers
  Wire.requestFrom(0x40, 4);

  //If the 4 bytes were returned sucessfully
  if (4 <= Wire.available())
  {
    //take reading
    //Byte[0] holds upper byte of temp reading
    Byte[0] = Wire.read();
    //Byte[1] holds lower byte of temp reading
    Byte[1] = Wire.read();
    //Byte[3] holds upper byte of humidity reading
    Byte[3] = Wire.read();
    //Byte[4] holds lower byte of humidity reading
    Byte[4] = Wire.read();
    //Combine the two bytes to make one 16 bit int
    temp = (((unsigned int)Byte[0] <<8 | Byte[1]));
    //Temp(C) = reading/(2^16)*165(C) – 40(C)
    *temperature = (double)(temp)/(65536)*165-40;
    //Combine the two bytes to make one 16 bit int
    humidity = (((unsigned int)Byte[3] <<8 | Byte[4]));
    //Humidity(%) = reading/(2^16)*100%
    return (double)(humidity)/(65536)*100;
  }
}
