#include "stm8s.h"

#define     DHT_Low()                GPIO_WriteLow(GPIOC , GPIO_PIN_1)
#define     DHT_High()               GPIO_WriteHigh(GPIOC , GPIO_PIN_1)
#define     DHT_Read()               GPIO_ReadInputPin(GPIOC , GPIO_PIN_1)
#define     DHT_OUT()                GPIO_Init(GPIOC , GPIO_PIN_1 , GPIO_MODE_OUT_OD_LOW_FAST); //set DHT11 output
#define     DHT_IN()                 GPIO_Init(GPIOC , GPIO_PIN_1 , GPIO_MODE_IN_PU_NO_IT);  //set DHT11 input


typedef enum
{
  RESULT_OK =           0,      // OK
  RESULT_ERRREAD =      1,      // Error reading
  RESULT_ERRCHKSUM =    2,      // Error check sum
} RESULT;

RESULT DHT_GetData(u16 * t, u16 * h);
u8 DHT_ReadData(u8 *data);
u8 DHT_CheckSum(u8 * data);


bool DHT_checkData();

extern bool dht_tem_ready;
extern bool dht_hum_ready;
extern bool dht_alive;
extern u16 dht_tem_value;
extern u16 dht_hum_value;
