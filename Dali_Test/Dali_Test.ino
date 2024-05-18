#include "Dali.h"

#define RX_MODE 0
#define TX_MODE 1

void setup() 
{
  Serial.begin(115200);
  Serial.println("Hello World");
  Dali_Init(6, 7);
}

void printDali(unsigned long int cmd)
{
  uint8_t cmdMSB = 0;
  uint8_t cmdLSB = 0;
  uint8_t address = 0;
  uint8_t cmdType = 0;
  uint8_t Data = 0;
  
  Serial.print("[");Serial.print(cmd,HEX);Serial.print("]:");
  cmdMSB = (cmd >> 0x08) & 0xFF;
  Serial.print("[");Serial.print(cmdMSB,HEX);Serial.print("]");
  cmdLSB = (cmd) & 0xFF;
  Serial.print("[");Serial.print(cmdLSB,HEX);Serial.print("]:");
  
  if(cmdMSB & 0x80){Serial.print("[1]");}
  else{Serial.print("[0]");}
  //GAAA AAAAC - DDDD DDDD
  address = cmdMSB & (0x7E);
  Serial.print("[");Serial.print(address, HEX);Serial.print("]");
  cmdType = cmdMSB & (0x01);
  Serial.print("[");Serial.print(cmdType);Serial.print("]");
  Data = (cmdLSB) & (0xFF);
  Serial.print("[");Serial.print(Data, HEX);Serial.println("]");
}

unsigned long int cmd = 0;



void loop() 
{
#if RX_MODE  
    cmd = Dali_Receive();
    if(cmd)
    {
      printDali(cmd);
      cmd = 0;
    }
#endif //RX_MODE


#if TX_MODE  

  Dali_Send(1,0x3F,1,CMD_RESET);
  Dali_Send(1,0x3F,1,CMD_RESET);
  Dali_Send_Spl_Cmd(INITIALISE,0x00);
  Dali_Send_Spl_Cmd(INITIALISE,0x00);


  //Dali_Send_Spl_Cmd(DTR0,1);
  //Dali_Send(1,0x3F,1,CMD_DTR_AS_FAIL);
  //Dali_Send(1,0x3F,1,CMD_DTR_AS_FAIL);

 
  uint16_t kelvin = 1000;
  
  while(1)
  {
    if(kelvin > 8000)
    {
      kelvin = 1000;
    }
    uint16_t val = 1000000/kelvin;
    uint8_t val1 = val & 0xFF;
    uint8_t val2 = (uint8_t)(val >> 0x08);
    Serial.print("[");Serial.print(kelvin);Serial.print("]");
    Serial.print("[");Serial.print(val);Serial.print("]");
    Serial.print("[");Serial.print(val, HEX);Serial.print("]");
    Serial.print("[");Serial.print(val1, HEX);Serial.print("]");
    Serial.print("[");Serial.print(val2, HEX);Serial.print("]");
    Serial.println("]");
    Dali_Send_Spl_Cmd(DTR0,val1);
    Dali_Send_Spl_Cmd(DTR1,val2);
    Dali_Send_Spl_Cmd(ENABLE_DEVICE_TYPE, 0x08);
    Dali_Send(1,0x3F,1,SET_TEMP_TEMPC);
    Dali_Send(1,0x3F,0,200);
    //cmd = Dali_Receive(100);
    //if(cmd)
    //{
    //  printDali(cmd);
    //  cmd = 0;
    //}
    delay(100);
    kelvin = kelvin + 50;
  }
  
//  for(uint8_t i=0; i<255; i=i+5)
//  {
//    Serial.print("TX :");Serial.println(i);
//    Dali_Send(1,0x3F,0,i);
//    delay(10);
//  }
//
//
//  delay(5000);
//  for(uint8_t i=255; i>0; i=i-5)
//  {
//    Serial.print("TX :");Serial.println(i);
//    Dali_Send(1,0x3F,0,i);
//    delay(10);
//  }
//  delay(10);
#endif //TX_MODE
}
