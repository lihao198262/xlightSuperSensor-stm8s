#include <stm8s.h>

#include "sen_irkey.h"

// IR Key sensor pin map
#define IRKEY_PORT              (GPIOB)
#define IRKEY_PIN1              (GPIO_PIN_5)
#define IRKEY_PIN2              (GPIO_PIN_4)
#define IRKEY_PIN3              (GPIO_PIN_3)

// Get Key sensor pin input
#define pinIRKey1               ((BitStatus)(IRKEY_PORT->IDR & (uint8_t)IRKEY_PIN1))
#define pinIRKey2               ((BitStatus)(IRKEY_PORT->IDR & (uint8_t)IRKEY_PIN2))
#define pinIRKey3               ((BitStatus)(IRKEY_PORT->IDR & (uint8_t)IRKEY_PIN3))

void irk_init()
{
  GPIO_Init(IRKEY_PORT, IRKEY_PIN1 | IRKEY_PIN2 | IRKEY_PIN3, GPIO_MODE_IN_PU_NO_IT);
}

u8 irk_read()
{
  return(pinIRKey1 | pinIRKey2 | pinIRKey3);
}