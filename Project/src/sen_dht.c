//adapted from http://www.amobbs.com/thread-5517880-1-1.html?_dsign=d9eb7efa//
#include "sen_dht.h"
#include "timer_2.h"

//#define DHT21
#define DHT11

static uint16_t collect_times = 0;
static uint16_t collect_times_success = 0;
static uint16_t collect_times_fail = 0;
static uint16_t dht11_timeout;

#define F_MASTER_MHZ    16
#define set_tmo_us(time)  dht11_timeout = (uint16_t)(F_MASTER_MHZ * time)

u8 wait_low(uint16_t timeout)
{
    set_tmo_us(timeout);
    while( !DHT_Read && --dht11_timeout);
    if(!dht11_timeout) return 1;
    return 0;
}
u8 wait_high(uint16_t timeout)
{
  set_tmo_us(timeout);
  while( DHT_Read && --dht11_timeout);
  if(!dht11_timeout) return 1;
  return 0;
}

#define DHT_TEM_MA_NUM         10
#define DHT_HUM_MA_NUM         40
#define DHT_TEM_MAX            50
#define DHT_HUM_MAX            90

bool dht_tem_ready = FALSE;
bool dht_hum_ready = FALSE;
bool dht_alive = FALSE;
// value = integer part * 100 + decimal part 
u16 dht_tem_value;
u16 dht_hum_value;

// Moving average
u8 dht_tem_mvPtr = 0;
u8 dht_hum_mvPtr = 0;

u16 dht_mvTemData[DHT_TEM_MA_NUM] = {0};
u16 dht_mvHumData[DHT_HUM_MA_NUM] = {0};
u32 dht_tem_mvSum = 0;
u32 dht_hum_mvSum = 0;

RESULT DHT_GetData(u16 * t, u16 * h);
u8 DHT_ReadData(u8 *data);

void DHT_init()
{
  dht_tem_mvSum = 0;
  dht_hum_mvSum = 0;
  dht_tem_mvPtr = 0;
  dht_hum_mvPtr = 0;
  dht_tem_value = 0;
  dht_hum_value = 0;
  dht_tem_ready = FALSE;
  dht_hum_ready = FALSE;  
  memset(dht_mvTemData, 0x00, sizeof(u16) * DHT_TEM_MA_NUM);
  memset(dht_mvHumData, 0x00, sizeof(u16) * DHT_HUM_MA_NUM);
  
  // Init Timer
  TIM2_Init();
}

bool DHT_checkData()
{
  u16 newTemData = 0;
  u16 newHumData = 0;
  if (DHT_GetData(&newTemData,&newHumData) == RESULT_OK)
  {
      if( newTemData > DHT_TEM_MAX * 100 ) newTemData = DHT_TEM_MAX * 100;
      if( newHumData > DHT_HUM_MAX * 100 ) newHumData = DHT_HUM_MAX * 100;
      
      dht_alive = TRUE;
      if( newTemData != dht_mvTemData[dht_tem_mvPtr] ) {
        dht_tem_mvSum += newTemData;
        dht_tem_mvSum -= dht_mvTemData[dht_tem_mvPtr];
        dht_mvTemData[dht_tem_mvPtr] = newTemData;
      }  
      dht_tem_mvPtr = (dht_tem_mvPtr + 1) % DHT_TEM_MA_NUM;
      
      if( newHumData != dht_mvHumData[dht_hum_mvPtr] ) {
        dht_hum_mvSum += newHumData;
        dht_hum_mvSum -= dht_mvHumData[dht_hum_mvPtr];
        dht_mvHumData[dht_hum_mvPtr] = newHumData;
      }  
      dht_hum_mvPtr = (dht_hum_mvPtr + 1) % DHT_HUM_MA_NUM;
      
      if( !dht_tem_ready ) {
        dht_tem_ready = (dht_tem_mvPtr == 0);
      }
      if( !dht_hum_ready ) {
        dht_hum_ready = (dht_hum_mvPtr == 0);
      }
      if( dht_tem_ready )
      {
        dht_tem_value = dht_tem_mvSum / DHT_TEM_MA_NUM;
        // Adjust according to sensor (+4 degree)
        dht_tem_value+= 400;
      }
      if( dht_hum_ready ) 
      {
        dht_hum_value = dht_hum_mvSum / DHT_HUM_MA_NUM;
      }
  }
  return dht_tem_ready || dht_hum_ready;
}

unsigned char U8FLAG, U8temp;

u8 DHT_ReadData(u8 *data)
{
    u8 i,j = 0;
    
    DHT_OUT;
    DHT_Low;      //DHT11=0
    Delayms(20);        //delay 20ms
    
    disableInterrupts();
    
    DHT_High;     //DHT11=1
    Delay10Us(4);	  //delay 40us
    DHT_IN;       //DHT11_input
    
    U8FLAG=0;
    if( wait_low(200) > 0) return RESULT_ERRREAD; //wait DHT11 fist 80us low singal  response
    if( wait_high(200) > 0) return RESULT_ERRREAD; //wait DHT11 fist 80us high singal   prepare
    for(j = 0; j<5; j++) { //read 5 bytes data
        for(i=0; i<8; i++) {
            wait_low(100);//wait the fist 50us low singal
            U8temp=0;
            Delay10Us(3);
            if(DHT_Read)
                U8temp=1;//wait the high singal if over 30us the this bit set to 1          
            data[j]<<=1;
            data[j]|=U8temp;
            wait_high(100);
        }
    }
    
    enableInterrupts();
    
    if( (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
      return RESULT_ERRCHKSUM;
    }
    
    return RESULT_OK;
}

RESULT DHT_GetData(u16 * t, u16 * h)
{
   collect_times++;
  /*if(collect_times == 600)
  {
     return RESULT_OK;
  }*/
  u8 tmp[5]={0};
  
  u8 rc = DHT_ReadData(tmp);
  if( rc == RESULT_OK )
  {
#ifdef DHT11
    *h = tmp[0]*100;
    *t = tmp[2]*100;
#endif
    
#ifdef DHT21
    // decimal part process
    u8 dectmp = tmp[1];
    if (dectmp < 10 ) dectmp *= 10;
    else if(dectmp >=100) dectmp /= 10;
    *h = tmp[0]*100 + dectmp;
    dectmp = tmp[3];
    if (dectmp < 10 ) dectmp *= 10;
    else if(dectmp >=100) dectmp /= 10;
    *t = tmp[2]*100 + dectmp;
#endif
    
    collect_times_success++;
    /*if(collect_times_success == 500)
    {
      //printf("oh,no");
      return RESULT_OK;
    }*/
  }
  else
  {
    collect_times_fail++;
    /*if(collect_times_fail == 500)
    {
      //printf("oh,no");
      return RESULT_ERRCHKSUM;
    }*/
  }
  
  return rc;
}
