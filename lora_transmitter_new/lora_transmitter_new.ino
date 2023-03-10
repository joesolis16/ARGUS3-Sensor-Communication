/*  
  =========================================================================
  Utilization: ARGUS III Transmitter LoRa Module
  Programmer 1: Anthony B. Duyan Jr. (RedID: 824809096)
  Programmer 2: Joseph Solis (RedID: 824761763)
  Date: 11/18/22
  =========================================================================
*/
#include "max6675.h" 
#include <Wire.h>

int SO = 12;
int CS = 10;
int sck = 13;
MAX6675 module(sck, CS, SO);

const int flamePin1 = 8;
const int flamePin2 = 7;
const int flamePin3 = 2;
int Flame1 = HIGH;
int Flame2 = HIGH;
int Flame3 = HIGH;


//  Setup code to run once.
void setup() 
{
  

  Serial.begin(115200);     // Default baudrate of module is 115200
  delay(2000);              // Wait for Lora device to be ready

  Serial.print("AT\r\n");
  delay(1000);

  Serial.print("AT+PARAMETER=10,7,1,7\r\n");      //For Less than 3Kms
  //Serial.print("AT+PARAMETER=10,7,1,7\r\n");    //For More than 3Kms
  delay(100);               //wait for module to respond

  Serial.print("AT+BAND=915000000\r\n");          //Bandwidth set to 915MHz
  delay(100);               //wait for module to respond

  Serial.print("AT+ADDRESS=115\r\n");             //needs to be unique
  delay(100);               //wait for module to respond

  Serial.print("AT+NETWORKID=8\r\n");             //needs to be same for receiver and transmitter
  delay(100);               //wait for module to respond

  // Serial.print("AT+PARAMETER?\r\n");              //prints the current parameters
  // delay(100);   //wait for module to respond

  // Serial.print("AT+BAND?\r\n");                   //prints the current bandwidth
  // delay(100);   //wait for module to respond

  // Serial.print("AT+NETWORKID?\r\n");              //prints the network id
  // delay(100);   //wait for module to respond

  // Serial.print("AT+ADDRESS?\r\n");                //prints the address
  // delay(100);   //wait for module to respond

  //Initialize I2C Communication
  Wire.begin();
  //Configure HDC1080
  Wire.beginTransmission(0x40);
  Wire.write(0x02);
  Wire.write(0x90);
  Wire.write(0x00);
  Wire.endTransmission();
  //Delay for Startup of HDC1080
  delay(20);

  pinMode(flamePin1, INPUT);
  pinMode(flamePin2, INPUT);
  pinMode(flamePin3, INPUT);

}

void loop() 
{
  // Wait 5s between measurements.

  delay(5000);

  // Flame sensors
  String condition;

  Flame1 = digitalRead(flamePin1);
  Flame2 = digitalRead(flamePin2);
  Flame3 = digitalRead(flamePin3);

  if ((Flame1 == LOW) || (Flame2 == LOW) || (Flame3 == LOW))
  {
    condition = "FIRE";
  }
  else
  {
    condition = "No fire";
  }

  // HDC1080
  double temp;
  double h;
  h = readSensor(&temp);

  String humidity = String(h);
  String temperature = String(temp);
  
  // Thermocoupler
  float couplerNum = module.readCelsius();
  String coupler =  String(couplerNum);

  String values = "Humidity is " + humidity + ", Temperature is " + temperature + "C ("  
                + "F) \nCondition: " + condition + " Thermalcoupler outside temp: " + coupler 
                + "C";
  String msgLength = String(values.length());

  // DATA TRANSMISSION
  Serial.print("AT+SEND=117," + msgLength + "," + values + "\r\n");

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
  delay(1);
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