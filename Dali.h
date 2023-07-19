#ifndef _DALI_
#define _DALI_

#define DALI_PERIOD 700
#define DALI_TIMEOUT 20000
#define DALI_TX_ACTIVE_HIGH 1

#ifdef __cplusplus
extern "C" {
#endif

void Dali_Init(uint8_t rxPin, uint8_t txPin);
unsigned long int Dali_Receive();
int Dali_Send(uint8_t grp, uint8_t address, uint8_t cmdType, uint8_t dataByte);

#ifdef __cplusplus
}
#endif


#endif//_CONFIO_DALI_
