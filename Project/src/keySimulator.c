#include <stm8s.h>
#include "keySimulator.h"
#include "xliNodeConfig.h"

keyBuffer_t gKeyBuf[KEY_OP_MAX_BUFFERS];

typedef struct {
  uint8_t target;
  uint8_t key;
  GPIO_TypeDef* port;
  GPIO_Pin_TypeDef pin;
} keyPinMap_t;

#define KEYMAP_TABLE_ROWS          3
const keyPinMap_t keyMapTable[] = {
  // target     key     port    pin
  {0,           1,      GPIOD,  GPIO_PIN_4},    // Up
  {0,           2,      GPIOD,  GPIO_PIN_3},    // Down
  {0,           3, 	GPIOD, 	GPIO_PIN_2}     // Stop
};

bool LookupKeyPinMap(uint8_t target, uint8_t key, GPIO_TypeDef **port, GPIO_Pin_TypeDef *pin)
{
  for( u8 i = 0; i < KEYMAP_TABLE_ROWS; i++ ) {
    if( target == keyMapTable[i].target || key == keyMapTable[i].key ) {
      *port = keyMapTable[i].port;
      *pin = keyMapTable[i].pin;
      return TRUE;
    }
  }
  return FALSE;
}

void keySimulator_init() {
  u8 i;
  for( i = 0; i < KEY_OP_MAX_BUFFERS; i++ ) {
    gKeyBuf[i].target = 0;
    gKeyBuf[i].keyNum = 0;    
    gKeyBuf[i].ptr = 0;
    gKeyBuf[i].tick = 0;
    memset(gKeyBuf[i].keys, 0x00, sizeof(keyStyle_t) * KEY_OP_MAX_KEYS);
  }
  
  for( i = 0; i < KEYMAP_TABLE_ROWS; i++ ) {
    GPIO_Init(keyMapTable[i].port, keyMapTable[i].pin, GPIO_MODE_OUT_PP_LOW_SLOW);
  }
}

// Parse ketstring and put operation series into an available buffer
bool ProduceKeyOperation(u8 _target, const char *_keyString, u8 _len) {
  u8 _keyNum = 0;
  for( u8 i = 0; i < KEY_OP_MAX_BUFFERS; i++ ) {
    if( gKeyBuf[i].keyNum == 0 ) {
      // Found available buffer
      for( u8 j = 0; j < _len; j++ ) {
        // Delay
        if( j == 0 ) gKeyBuf[i].keys[_keyNum].delay = 0;
        else {
          switch(_keyString[j]) {
          case KEY_DELI_NO_PAUSE:
            gKeyBuf[i].keys[_keyNum].delay = 0;
            break;
          case KEY_DELI_SMALL_PAUSE:
            gKeyBuf[i].keys[_keyNum].delay = 20;
            break;
          case KEY_DELI_NORMAL_PAUSE:
            gKeyBuf[i].keys[_keyNum].delay = 60;
            break;
          case KEY_DELI_LONG_PAUSE:
            gKeyBuf[i].keys[_keyNum].delay = 200;
            break;
          case KEY_DELI_VLONG_PAUSE:
            gKeyBuf[i].keys[_keyNum].delay = 500;
            break;
          case KEY_DELI_SAME_TIME:
            gKeyBuf[i].keys[_keyNum].delay = 0;
          default:
            gKeyBuf[i].keys[_keyNum].delay = 0;
            break;
          }
          if( ++j >= _len ) break;
        }
        
        gKeyBuf[i].keys[_keyNum].op = _keyString[j];
        if( ++j >= _len ) break;
        gKeyBuf[i].keys[_keyNum].keyID = _keyString[j];
        if( ++_keyNum >= KEY_OP_MAX_KEYS ) break;
      }
      gKeyBuf[i].target = _target;
      gKeyBuf[i].ptr = 0;
      gKeyBuf[i].tick = 0;
      gKeyBuf[i].keyNum = _keyNum;
      return TRUE;
    }
  }
  return FALSE;
}

void SimulateKeyPress(u8 _target, u8 _op, u8 _key) {
  GPIO_TypeDef *_port;
  GPIO_Pin_TypeDef _pin;
  if( LookupKeyPinMap(_target, _key, &_port, &_pin) ) {
    switch(_op) {
    case KEY_OP_STYLE_PRESS:
      relay_gpio_write_bit(_port, _pin, TRUE);
      break;
    case KEY_OP_STYLE_FAST_PRESS:
      break;
    case KEY_OP_STYLE_LONG_PRESS:
      break;
    case KEY_OP_STYLE_HOLD:
      break;
    case KEY_OP_STYLE_RELEASE:
      break;
    case KEY_OP_STYLE_DBL_CLICK:
      // ToDo:...
      break;
    }
  }
}

void ScanKeyBuffer(u8 _idx) {
  u8 _k = gKeyBuf[_idx].ptr;
  if( _k < gKeyBuf[_idx].keyNum ) {    
    // Tick
    if( ++gKeyBuf[_idx].tick > gKeyBuf[_idx].keys[_k].delay ) {
      SimulateKeyPress(gKeyBuf[_idx].target, gKeyBuf[_idx].keys[_k].op, gKeyBuf[_idx].keys[_k].keyID);
      gKeyBuf[_idx].tick = 0;
      gKeyBuf[_idx].ptr++;
    }
  }
}