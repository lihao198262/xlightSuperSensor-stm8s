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

u8 irk_read()
{
  u8 keyBitmap = 0;
  BF_SET(keyBitmap, GPIO_ReadInputPin(IRKEY_PORT, IRKEY_PIN1), 0, 1);
  BF_SET(keyBitmap, GPIO_ReadInputPin(IRKEY_PORT, IRKEY_PIN2), 1, 1);
  BF_SET(keyBitmap, GPIO_ReadInputPin(IRKEY_PORT, IRKEY_PIN3), 2, 1);
  return(keyBitmap);
}