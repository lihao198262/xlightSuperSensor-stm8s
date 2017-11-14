#ifndef __KEY_SIMULATOR_H
#define __KEY_SIMULATOR_H

#define KEY_OP_MAX_BUFFERS      2
#define KEY_OP_MAX_KEYS         6
#define KEY_OP_MAX_CON_KEYS     4               // Max concurrent keys

#define CURTAIN_ON    "p1"
#define CURTAIN_OFF   "p2"
#define CURTAIN_SW_LEN 2

typedef struct
{
  u16 delay;
  u8 op;
  u8 keyID[KEY_OP_MAX_CON_KEYS];
} keyStyle_t;

typedef struct
{
  u8 target;
  u8 keyNum;
  u8 ptr;
  u16 tick;
  bool key_on;
  u8 key_tick;
  u8 key_delay;
  u8 key_dbl_step;
  keyStyle_t keys[KEY_OP_MAX_KEYS];
} keyBuffer_t;

extern keyBuffer_t gKeyBuf[KEY_OP_MAX_BUFFERS];

void keySimulator_init();
bool ProduceKeyOperation(u8 _target, const char *_keyString, u8 _len);
u8 SimulateKeyPress(u8 _target, u8 _op, u8 *_keys);
bool FinishKeyPress(u8 _target, u8 _op, u8 *_keys, u8 _step);
void ScanKeyBuffer(u8 _idx);

#endif /* __KEY_SIMULATOR_H */