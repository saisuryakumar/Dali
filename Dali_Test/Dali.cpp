
#include <inttypes.h>
#include "esp32-hal-gpio.h"
#include "hal/gpio_hal.h"
#include "soc/soc_caps.h"

#include "Dali.h"

#include <Arduino.h>

uint8_t Dali_rxPin = 0;
uint8_t Dali_txPin = 0;
uint8_t bitArray[50] = {0};
unsigned long timeArray[51] = {0};
uint8_t i=0;
unsigned long bitStartTime = 0;
uint8_t rxMode = 0;
uint8_t bitCount = 0;


static void sendZero(void)
{
#ifdef DALI_TX_ACTIVE_LOW
  digitalWrite(Dali_txPin, LOW);
  delayMicroseconds(416);
  digitalWrite(Dali_txPin, HIGH);
  delayMicroseconds(416);
#else
  digitalWrite(Dali_txPin, HIGH);
  delayMicroseconds(416);
  digitalWrite(Dali_txPin, LOW);
  delayMicroseconds(416);
#endif //DALI_TX_ACTIVE_LOW

}


static void sendOne(void)
{
#ifdef DALI_TX_ACTIVE_LOW
  digitalWrite(Dali_txPin, HIGH);
  delayMicroseconds(416);
  digitalWrite(Dali_txPin, LOW);
  delayMicroseconds(416);
#else
  digitalWrite(Dali_txPin, LOW);
  delayMicroseconds(416);
  digitalWrite(Dali_txPin, HIGH);
  delayMicroseconds(416);
#endif //DALI_TX_ACTIVE_LOW
}


static void sendStopBit()
{
#ifdef DALI_TX_ACTIVE_LOW
  digitalWrite(Dali_txPin, LOW);
#else
  digitalWrite(Dali_txPin, HIGH);
#endif //DALI_TX_ACTIVE_LOW
}

static void sendBit(uint8_t b)
{
  if (b){sendOne();}
  else {sendZero();}   
}
static void sendByte(uint8_t b)
{
  for (int i = 7;  i >= 0; i--) 
  {
    sendBit((b >> i) & 1);
  }
}


void Dali_Init(uint8_t rxPin, uint8_t txPin)
{
  Dali_rxPin = rxPin;
  Dali_txPin = txPin;
  pinMode(Dali_rxPin, INPUT_PULLUP);
  pinMode(Dali_txPin, OUTPUT);
}

unsigned long int Dali_Receive(uint8_t rxWaitTimeout)
{
  unsigned long daliWaiTime = micros();
  unsigned long int cmd = 0; 
#if DALI_RX_ACTIVE_LOW
  uint8_t pinState = 1;
#else
  uint8_t pinState = 0;
#endif //DALI_RX_ACTIVE_LOW  
  while((micros() - daliWaiTime) < (rxWaitTimeout*1000))
  //while(1)
  {
    if(pinState != digitalRead(Dali_rxPin))
    {
      rxMode = 1;
      timeArray[i] = micros() - bitStartTime;
      bitStartTime = micros();
      pinState = digitalRead(Dali_rxPin);
 #if DALI_RX_ACTIVE_LOW
      bitArray[i] = pinState;
 #else
      bitArray[i] = !pinState;
 #endif //DALI_RX_ACTIVE_LOW  
      bitCount++; 
      i++;
      if(i>50){i=0;}
    }

    if(rxMode == 1)
    { 
      if((micros() - bitStartTime) > 2000)
      {
        for(uint8_t i=0; i<bitCount-1; i++)
        {
          timeArray[i] = timeArray[i+1];
        }
         timeArray[bitCount] = 0;

#if DALI_DEBUG         
         Serial.print("bitArray------>");
         for(uint8_t i=0; i<bitCount; i++)
         {
           Serial.print("[ ");Serial.print(bitArray[i]);Serial.print(" ]");
         }
         Serial.println();
         Serial.print("timeArray----->");
         for(uint8_t i=0; i<bitCount; i++)
         {
           Serial.print("[");Serial.print(timeArray[i]);Serial.print("]");
         }
         Serial.println(bitCount);
#endif //DALI_DEBUG

        for(uint8_t i=0; i<bitCount; i++)
        {
          if(timeArray[i] > DALI_PERIOD)
          {
            for(uint8_t j=bitCount; j>i; j--)
            {
              timeArray[j] = timeArray[j-1];
              bitArray[j] = bitArray[j-1];
            }
            bitCount++;
            timeArray[i] = timeArray[i] / 2;
            timeArray[i+1] = timeArray[i];
          }
        }

#if DALI_DEBUG 
         Serial.print("after added slot bitArray------>");
         for(uint8_t i=0; i<bitCount; i++)
         {
           Serial.print("[ ");Serial.print(bitArray[i]);Serial.print(" ]");
         }
         Serial.println();
         Serial.print("after added slot timeArray----->");
         for(uint8_t i=0; i<bitCount; i++)
         {
           Serial.print("[");Serial.print(timeArray[i]);Serial.print("]");
         }
         Serial.println();
         Serial.println(bitCount);
#endif //DALI_DEBUG
        
        uint8_t index = 0;
        for(uint8_t i=2; i<bitCount-1; i=i+2)
        {
          if((bitArray[i] == 0) && (bitArray[i+1] == 1))
          {
            //Serial.print("[1]");
            cmd = (cmd<<1) + 1;
          }
          else
          {
            //Serial.print("[0]");
            cmd = (cmd<<1);
          }
        }
#if DALI_DEBUG
        Serial.println(cmd, HEX);
#endif //DALI_DEBUG
        
        rxMode = 0; 
        i = 0;
        bitCount = 0;
        bitStartTime = 0;
        break;
      }
    }
  }
  
  return cmd;
}

int Dali_Send_Spl_Cmd(uint8_t cmd1, uint8_t cmd2)
{
  //Serial.print("cmd1: 0x"); Serial.println(cmd1, HEX);
  //Serial.print("cmd2: 0x"); Serial.println(cmd2, HEX);

  sendBit(1);
  sendByte(cmd1);
  sendByte(cmd2);
  sendStopBit();
  delayMicroseconds(6000);
  return 0;
}

int Dali_Send(uint8_t grp, uint8_t address, uint8_t cmdType, uint8_t dataByte)
{
  uint8_t msbByte = 0;
  uint8_t lsbByte = dataByte;
  if((grp > 1) || (cmdType > 1))
  {
    return -1;
  }
  if(address > 0x3F)
  {
    return -1;
  }
  //individual or Short Address
  //Group or Broadcast
  if(grp)
  {
    msbByte = msbByte | 0x80;
  }

  //00aa aaaa
  //0aaa aaa0
  address = address << 0x01;

  //gaaa aaa0
  msbByte = msbByte | address;

  //gaaa aaac
  msbByte = msbByte | cmdType;

  //Serial.print("msbByte: 0x"); Serial.println(msbByte, HEX);
  //Serial.print("lsbByte: 0x"); Serial.println(lsbByte, HEX);

  sendBit(1);
  sendByte(msbByte);
  sendByte(lsbByte);
  sendStopBit();
  delayMicroseconds(6000);
  return 0;

}
