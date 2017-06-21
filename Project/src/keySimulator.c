#include <stm8s.h>
#include "keySimulator.h"

keyBuffer_t gKeyBuf[KEY_OP_MAX_BUFFERS];

void keySimulator_init() {
  for( u8 i = 0; i < KEY_OP_MAX_BUFFERS; i++ ) {
    gKeyBuf[i].target = 0;
    gKeyBuf[i].keyNum = 0;    
    memset(gKeyBuf[i].keys, 0x00, sizeof(keyStyle_t) * KEY_OP_MAX_KEYS);
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
          gKeyBuf[i].keys[_keyNum].delay = _keyString[j];
          if( ++j >= _len ) break;
        }
        
        gKeyBuf[i].keys[_keyNum].op = _keyString[j];
        if( ++j >= _len ) break;
        gKeyBuf[i].keys[_keyNum].keyID = _keyString[j];
        if( ++_keyNum >= KEY_OP_MAX_KEYS ) break;
      }
      gKeyBuf[i].target = _target;
      gKeyBuf[i].keyNum = _keyNum;
      return TRUE;
    }
  }
  return FALSE;
}