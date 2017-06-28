#include <stm8s.h>
#include "sen_irkey.h"
#include "MyMessage.h"

// IR Key sensor pin map
#define IRKEY_PORT              (GPIOB)
#define IRKEY_PIN1              (GPIO_PIN_5)
#define IRKEY_PIN2              (GPIO_PIN_4)
#define IRKEY_PIN3              (GPIO_PIN_3)

void irk_init()
{
  GPIO_Init(IRKEY_PORT, IRKEY_PIN1 | IRKEY_PIN2 | IRKEY_PIN3, GPIO_MODE_IN_PU_NO_IT);
}

uint8_t irk_read()
{
  uint8_t keyBitmap = 0x00;
  uint8_t _temp = GPIO_ReadInputData(IRKEY_PORT);
  
  if( _temp & IRKEY_PIN1 ) 
    BF_SET(keyBitmap, 1, 0, 1);
  
  if( _temp & IRKEY_PIN2 )
    BF_SET(keyBitmap, 1, 1, 1);
  
  if(_temp & IRKEY_PIN3 )
    BF_SET(keyBitmap, 1, 2, 1);
  
  return(keyBitmap);
}