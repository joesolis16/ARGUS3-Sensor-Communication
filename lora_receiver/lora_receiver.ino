/*  
  =========================================================================
  Utilization: ARGUS III Lora Transceiver Test (Receiver)
  Programmer 1: Anthony B. Duyan Jr. (RedID: 824809096)
  Programmer 2: Joseph Solis (RedID: 824761763)
  Date: 3/22/23
  =========================================================================
*/
String received;
const String msgRecv = "+RCV=";
//  Setup code to run once.
void setup() 
{
  
  Serial.begin(115200);     // Default baudrate of module is 115200
}

void loop() 
{

  
  if(Serial.available())
  {
    received = Serial.readString();
    if (received.startsWith(msgRecv))
      Serial.println(received);
  }
}
