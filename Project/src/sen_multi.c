#include <stm8s.h>
#include "sen_multi.h"
#include "Uart2Dev.h"
#include "_global.h"

#define MULTI_MESSAGE_HEAD       0x01
#define MULTI_MESSAGE_CODE       0x03
#define UART_STEP_WAIT_HEAD_0           0
#define UART_STEP_WAIT_HEAD_1           1
#define UART_STEP_WAIT_LEN              2
#define UART_STEP_WAIT_PAYL             3
#define UART_STEP_WAIT_CHECKSUM0        4
#define UART_STEP_WAIT_CHECKSUM1        5

#define TEM_MAX            60
#define TEM_MIN            0
#define HUM_MAX            90
#define HUM_MIN            20
#define PM25_MIN           0
#define PM25_MAX           1000
#define PM10_MIN           0
#define PM10_MAX           1000
#define CO2_MIN            400
#define CO2_MAX            5000
#define TVOC_MIN           0
#define TVOC_MAX           1000
#define CH2O_MIN           0
#define CH2O_MAX           400

/*CRC 校验表高位*/ 
const unsigned char auchCRCHi[] = { 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 }; 
 
 
/* CRC 校验表低位    */ 
const unsigned char auchCRCLo[] = { 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40 }; 
 
 
/*CRC 校验函数，生成 CRC*/ 
bool CRC_Check(u8 * pushMsg,u8 usDataLen,u8 CRCH,u8 CRCL) 
{ 
  u8 uchCRCHi = 0xff;//高 CRC 字节初始化 
  u8 uchCRCLo = 0xff;//低 CRC 字节初始化 
  u8 uIndex; //CRC 循环中的索引 
  while(usDataLen--) { 
    uIndex = uchCRCLo^ *pushMsg++;//计算 CRC 
    uchCRCLo = uchCRCHi^ auchCRCHi[uIndex]; 
    uchCRCHi = auchCRCLo[uIndex]; 
  } 
  if(uchCRCHi == CRCH && uchCRCLo == CRCL) return TRUE;
  return FALSE;  
}  
#define MAX_UART_BUF_SIZE 19
#define MSG_NUM 1
uint8_t uart_step = UART_STEP_WAIT_HEAD_0;
uint8_t uartReceiveDataBuf[MSG_NUM][MAX_UART_BUF_SIZE];
uint8_t uartDataPtr = 0;
uint8_t msgWPtr = 0;
uint8_t msgRPtr = 0;
uint8_t msgLen = 0;
  
uint16_t co2Val=0;
uint16_t tvocVal=0;
uint16_t ch2oVal=0;
uint16_t pm25Val=0;
int16_t humVal=0;
int16_t temVal=0;
uint16_t pm10Val=0;

uint16_t multi_sensor_alive_tick = 0;

void multi_init()
{ 
  uartDataPtr = 0;
  uart_step = UART_STEP_WAIT_HEAD_0;
  memset(uartReceiveDataBuf,0,sizeof(uartReceiveDataBuf));
  // Init serial ports
  uart2_config(9600);
}

/////////////////////////////tem and hum///////////////////////////////////////
#define TEM_MA_NUM         10
#define HUM_MA_NUM         20

// value = integer part * 100 + decimal part 
int16_t tem_value;
int16_t hum_value;

// Moving average
u8 tem_mvPtr = 0;
u8 hum_mvPtr = 0;
u8 tem_mvLen = 0;
u8 hum_mvLen = 0;

int16_t mvTemData[TEM_MA_NUM] = {0};
int16_t mvHumData[HUM_MA_NUM] = {0};
int32_t tem_mvSum = 0;
int32_t hum_mvSum = 0;

void AddHumTemData(int16_t temData,int16_t humData)
{
    if( temData > TEM_MAX * 100 ) 
    {
      temData = TEM_MAX * 100;
    }
    else if( temData < TEM_MIN * 100 ) 
    {
      temData = TEM_MIN * 100;
    }
    if( humData > HUM_MAX * 100 ) 
    {
      humData = HUM_MAX * 100;
    }
    else if( humData < HUM_MIN * 100 ) 
    {
      humData = HUM_MIN * 100;
    }
    
    if( temData != mvTemData[tem_mvPtr] ) {
      tem_mvSum += temData;
      tem_mvSum -= mvTemData[tem_mvPtr];
      mvTemData[tem_mvPtr] = temData;
    }  
    tem_mvPtr = (tem_mvPtr + 1) % TEM_MA_NUM;
    if(tem_mvLen < TEM_MA_NUM) tem_mvLen++;
    else tem_mvLen = TEM_MA_NUM;
    
    if( humData != mvHumData[hum_mvPtr] ) {
      hum_mvSum += humData;
      hum_mvSum -= mvHumData[hum_mvPtr];
      mvHumData[hum_mvPtr] = humData;
    }  
    hum_mvPtr = (hum_mvPtr + 1) % HUM_MA_NUM;
    if(hum_mvLen < HUM_MA_NUM) hum_mvLen++;
    else hum_mvLen = HUM_MA_NUM;
    
    tem_value = tem_mvSum / tem_mvLen;
    hum_value = hum_mvSum / hum_mvLen;
}
/////////////////////////////tem and hum///////////////////////////////////////


/////////////////////////////pm25 and pm10///////////////////////////////////////
#define PM25_MA_NUM         20
#define PM10_MA_NUM         20

uint16_t pm25_value;
uint16_t pm10_value;

// Moving average
u8 pm25_mvPtr = 0;
u8 pm10_mvPtr = 0;
u8 pm25_mvLen = 0;
u8 pm10_mvLen = 0;

uint16_t mvPm25Data[PM25_MA_NUM] = {0};
uint16_t mvPm10Data[PM10_MA_NUM] = {0};
uint32_t pm25_mvSum = 0;
uint32_t pm10_mvSum = 0;

void AddPMData(uint16_t pm25Data,uint16_t pm10Data)
{
    if( pm25Data > PM25_MAX) 
    {
      pm25Data = PM25_MAX;
    }
    if( pm10Data > PM10_MAX ) 
    {
      pm10Data = PM10_MAX;
    }
    
    if( pm25Data != mvPm25Data[pm25_mvPtr] ) {
      pm25_mvSum += pm25Data;
      pm25_mvSum -= mvPm25Data[pm25_mvPtr];
      mvPm25Data[pm25_mvPtr] = pm25Data;
    }  
    pm25_mvPtr = (pm25_mvPtr + 1) % PM25_MA_NUM;
    if(pm25_mvLen < PM25_MA_NUM) pm25_mvLen++;
    else pm25_mvLen = PM25_MA_NUM;
    
    if( pm10Data != mvPm10Data[pm10_mvPtr] ) {
      pm10_mvSum += pm10Data;
      pm10_mvSum -= mvPm10Data[pm10_mvPtr];
      mvPm10Data[pm10_mvPtr] = pm10Data;
    }  
    pm10_mvPtr = (pm10_mvPtr + 1) % PM10_MA_NUM;
    if(pm10_mvLen < PM10_MA_NUM) pm10_mvLen++;
    else pm10_mvLen = PM10_MA_NUM;
    
    pm25_value = pm25_mvSum / pm25_mvLen;
    pm10_value = pm10_mvSum / pm10_mvLen;
}
/////////////////////////////pm25 and pm10///////////////////////////////////////

/////////////////////////////tvoc and ch2o and co2///////////////////////////////////////
#define TVOC_MA_NUM         50
#define CH2O_MA_NUM         50
#define CO2_MA_NUM          50

uint16_t tvoc_value=0;
uint16_t ch2o_value=0;
uint16_t co2_value=0;


// Moving average
u8 tvoc_mvPtr = 0;
u8 ch2o_mvPtr = 0;
u8 co2_mvPtr = 0;
u8 tvoc_mvLen = 0;
u8 ch2o_mvLen = 0;
u8 co2_mvLen = 0;


uint16_t mvTvocData[TVOC_MA_NUM] = {0};
uint16_t mvCh2oData[CH2O_MA_NUM] = {0};
uint16_t mvCo2Data[CO2_MA_NUM] = {0};
uint32_t tvoc_mvSum = 0;
uint32_t ch2o_mvSum = 0;
uint32_t co2_mvSum = 0;

void AddAirData(uint16_t tvocData,uint16_t ch2oData,uint16_t co2Data)
{
    if( tvocData > TVOC_MAX) 
    {
      tvocData = TVOC_MAX;
    }
    if( ch2oData > CH2O_MAX ) 
    {
      ch2oData = CH2O_MAX;
    }
    if( co2Data > CO2_MAX ) 
    {
      co2Data = CO2_MAX;
    }
    
    if( tvocData != mvTvocData[tvoc_mvPtr] ) {
      tvoc_mvSum += tvocData;
      tvoc_mvSum -= mvTvocData[tvoc_mvPtr];
      mvTvocData[tvoc_mvPtr] = tvocData;
    }  
    tvoc_mvPtr = (tvoc_mvPtr + 1) % TVOC_MA_NUM;
    if(tvoc_mvLen < TVOC_MA_NUM) tvoc_mvLen++;
    else tvoc_mvLen = PM25_MA_NUM;
    
    if( ch2oData != mvCh2oData[ch2o_mvPtr] ) {
      ch2o_mvSum += ch2oData;
      ch2o_mvSum -= mvCh2oData[ch2o_mvPtr];
      mvCh2oData[ch2o_mvPtr] = ch2oData;
    }  
    ch2o_mvPtr = (ch2o_mvPtr + 1) % TVOC_MA_NUM;
    if(ch2o_mvLen < TVOC_MA_NUM) ch2o_mvLen++;
    else ch2o_mvLen = TVOC_MA_NUM;
    
    if( co2Data != mvCo2Data[co2_mvPtr] ) {
      co2_mvSum += co2Data;
      co2_mvSum -= mvCo2Data[co2_mvPtr];
      mvCo2Data[co2_mvPtr] = co2Data;
    }  
    co2_mvPtr = (co2_mvPtr + 1) % CO2_MA_NUM;
    if(co2_mvLen < CO2_MA_NUM) co2_mvLen++;
    else co2_mvLen = CO2_MA_NUM;
    
    tvoc_value = tvoc_mvSum / tvoc_mvLen;
    ch2o_value = ch2o_mvSum / ch2o_mvLen;
    co2_value = co2_mvSum / co2_mvLen;
}
////////////////////////////////////////////////////////////////////

void PraseMultiSensorMsg()
{
  while(msgLen >0 || msgRPtr != msgWPtr)
  {
#ifdef TEST
  PB2_High;
#endif
    bool crcCheckRet = CRC_Check(uartReceiveDataBuf[msgRPtr],17,uartReceiveDataBuf[msgRPtr][18],uartReceiveDataBuf[msgRPtr][17]);
    if(crcCheckRet)
    {
      co2Val = ((uint16_t)uartReceiveDataBuf[msgRPtr][3]<<8 | uartReceiveDataBuf[msgRPtr][4]);
      tvocVal = ((uint16_t)uartReceiveDataBuf[msgRPtr][5]<<8 | uartReceiveDataBuf[msgRPtr][6]);
      ch2oVal = ((uint16_t)uartReceiveDataBuf[msgRPtr][7]<<8 | uartReceiveDataBuf[msgRPtr][8]);
      pm25Val = ((uint16_t)uartReceiveDataBuf[msgRPtr][9]<<8 | uartReceiveDataBuf[msgRPtr][10]);
      uint16_t humTem = ((uint16_t)uartReceiveDataBuf[msgRPtr][11]<<8 | uartReceiveDataBuf[msgRPtr][12]);
      humVal = -600+(((uint32_t)12500*humTem)>>16);
      uint16_t temTem = ((uint16_t)uartReceiveDataBuf[msgRPtr][13]<<8 | uartReceiveDataBuf[msgRPtr][14]);
      temVal = -4685+(((uint32_t)17572*temTem)>>16);
      pm10Val = ((uint16_t)uartReceiveDataBuf[msgRPtr][15]<<8 | uartReceiveDataBuf[msgRPtr][16]); 
      
      AddHumTemData(temVal,humVal);
      AddPMData(pm25Val,pm10Val);
      AddAirData(tvocVal,ch2oVal,co2Val);
      multi_sensor_alive_tick = 0;
    }   
    msgRPtr = (msgRPtr+1)%MSG_NUM;
    msgLen = 0;
#ifdef TEST
  PB2_Low;
#endif
  } 
  return;
}

INTERRUPT_HANDLER(UART2_RX_IRQHandler, 21)
{
  /* In order to detect unexpected events during development,
  it is recommended to set a breakpoint on the following instruction.
  */
  u8 data;
  if( UART2_GetITStatus(UART2_IT_RXNE) == SET ) {
    data = UART2_ReceiveData8();
    switch( uart_step ) {
    case UART_STEP_WAIT_HEAD_0:
      if( data == MULTI_MESSAGE_HEAD ) 
      {
        uart_step = UART_STEP_WAIT_HEAD_1;
        uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;
      }
      break;
    case UART_STEP_WAIT_HEAD_1:
      if( data == MULTI_MESSAGE_CODE )
      {
        uart_step = UART_STEP_WAIT_LEN;
        uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;
      }
      else {
        uartDataPtr = 0;
        uart_step = UART_STEP_WAIT_HEAD_0;
      }
      break;
    case UART_STEP_WAIT_LEN:
      if( data > 1 && data < MAX_UART_BUF_SIZE ) {
        //uartDataPtr = 0;
        uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;
        uart_step = UART_STEP_WAIT_PAYL;
      } else {
        uartDataPtr = 0;
        uart_step = UART_STEP_WAIT_HEAD_0;
      }
      break;
    case UART_STEP_WAIT_PAYL:   
      uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;    
      if( uartDataPtr == uartReceiveDataBuf[msgWPtr][2]+UART_STEP_WAIT_PAYL ) uart_step = UART_STEP_WAIT_CHECKSUM0;
      break;
    case UART_STEP_WAIT_CHECKSUM0:
      uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;
      uart_step = UART_STEP_WAIT_CHECKSUM1;
      break;
    case UART_STEP_WAIT_CHECKSUM1:
      uartReceiveDataBuf[msgWPtr][uartDataPtr++] = data;
      uart_step = UART_STEP_WAIT_CHECKSUM1;
      msgWPtr = (msgWPtr+1)%MSG_NUM;
      msgLen++;
      uartDataPtr = 0;
      uart_step = UART_STEP_WAIT_HEAD_0;
      break;
    default:
      uartDataPtr = 0;
      uart_step = UART_STEP_WAIT_HEAD_0;
      break;
    }
    UART2_ClearITPendingBit(UART2_IT_RXNE);
  }
}
