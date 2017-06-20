#include <stm8s.h>
#include "ADC1Dev.h"

#include "sen_mic.h"

#define MIC_MA_NUM             40

bool mic_ready = FALSE;
bool mic_alive = FALSE;
u16 mic_value;

// Moving average
u8 mic_mvPtr = 0;
u16 mic_mvData[MIC_MA_NUM] = {0};
u32 mic_mvSum = 0;

bool mic_checkData()
{
  u16 newData = mic_read();
  if( newData > 3000 ) return mic_ready;
  
  mic_alive = TRUE;
  if( newData != mic_mvData[mic_mvPtr] ) {
    mic_mvSum += newData;
    mic_mvSum -= mic_mvData[mic_mvPtr];
    mic_mvData[mic_mvPtr] = newData;
  }  
  mic_mvPtr = (mic_mvPtr + 1) % MIC_MA_NUM;
  if( !mic_ready ) {
    mic_ready = (mic_mvPtr == 0);
  }
  
  if( mic_ready ) mic_value = mic_mvSum / MIC_MA_NUM;
    
  return mic_ready;
}


