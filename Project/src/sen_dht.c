//adapted from http://www.amobbs.com/thread-5517880-1-1.html?_dsign=d9eb7efa//
#include "sen_dht.h"
#include "timer_2.h"

//#define DHT21
#define DHT11
//#define DHT22

//static uint16_t collect_times = 0;
//static uint16_t collect_times_success = 0;
//static uint16_t collect_times_fail = 0;
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
#define DHT_HUM_MA_NUM         20

#ifdef DHT11
#define DHT_TEM_MAX            50
#define DHT_TEM_MIN            0
#define DHT_HUM_MAX            90
#define DHT_HUM_MIN            20
#elif defined DHT22
#define DHT_TEM_MAX            80
#define DHT_TEM_MIN            -40
#define DHT_HUM_MAX            80
#define DHT_HUM_MIN            -20
#else
#define DHT_TEM_MAX            60
#define DHT_TEM_MIN            -40
#define DHT_HUM_MAX            100
#define DHT_HUM_MIN            -20
#endif

bool dht_tem_ready = FALSE;
bool dht_hum_ready = FALSE;
bool dht_alive = FALSE;
// value = integer part * 100 + decimal part 
s16 dht_tem_value;
s16 dht_hum_value;

// Moving average
u8 dht_tem_mvPtr = 0;
u8 dht_hum_mvPtr = 0;

s16 dht_mvTemData[DHT_TEM_MA_NUM] = {0};
s16 dht_mvHumData[DHT_HUM_MA_NUM] = {0};
s32 dht_tem_mvSum = 0;
s32 dht_hum_mvSum = 0;

RESULT DHT_GetData(s16 * t, s16 * h);
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
  memset(dht_mvTemData, 0x00, sizeof(s16) * DHT_TEM_MA_NUM);
  memset(dht_mvHumData, 0x00, sizeof(s16) * DHT_HUM_MA_NUM);
  
  // Init Timer
  TIM2_Init();
}

bool DHT_checkData()
{
  s16 newTemData = 0;
  s16 newHumData = 0;
  if (DHT_GetData(&newTemData,&newHumData) == RESULT_OK)
  {
      if( newTemData > DHT_TEM_MAX * 100 ) 
      {
        newTemData = DHT_TEM_MAX * 100;
      }
      else if( newTemData < DHT_TEM_MIN * 100 ) 
      {
        newTemData = DHT_TEM_MIN * 100;
      }
      if( newHumData > DHT_HUM_MAX * 100 ) 
      {
        newHumData = DHT_HUM_MAX * 100;
      }
      else if( newHumData < DHT_HUM_MIN * 100 ) 
      {
        newHumData = DHT_HUM_MIN * 100;
      }
      
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
    if( wait_low(200) > 0) goto failed; //wait DHT11 fist 80us low singal  response
    if( wait_high(200) > 0) goto failed; //wait DHT11 fist 80us high singal   prepare
    for(j = 0; j<5; j++) { //read 5 bytes data
        for(i=0; i<8; i++) {
            if( wait_low(100) > 0) goto failed;//wait the fist 50us low singal
            U8temp=0;
            Delay10Us(3);
            if(DHT_Read)
                U8temp=1;//wait the high singal if over 30us the this bit set to 1          
            data[j]<<=1;
            data[j]|=U8temp;
            if( wait_high(100)> 0) goto failed;
        }
    }
    
    enableInterrupts();
    
    if( (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
      return RESULT_ERRCHKSUM;
    }   
    return RESULT_OK;
failed:
    enableInterrupts();
    return RESULT_ERRREAD;
}

RESULT DHT_GetData(s16 * t, s16 * h)
{
   //collect_times++;
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

#ifdef DHT22
    int16_t hum10 = 0;
    int16_t tem10 = 0;
    if ((tmp[0] & 0x80) == 0x80) {
            hum10 |= (tmp[0] & 0x7F) << 8;
            hum10 |= tmp[1];
            hum10 *= -1;
    } else {
            hum10 |= tmp[0] << 8;
            hum10 |= tmp[1];
    }
    if ((tmp[2] & 0x80) == 0x80) {
            tem10 |= (tmp[2] & 0x7F) << 8;
            tem10 |= tmp[3];
            tem10 *= -1;
    } else {
            tem10 |= tmp[2] << 8;
            tem10 |= tmp[3];
    }
    *h = hum10 * 10;
    *t = tem10 * 10;
#endif
    
    //collect_times_success++;
  }
  /*else
  {
    collect_times_fail++;
  }*/
  /*printlog("t=");
  printnum(collect_times);
  printlog(",s=");
  printnum(collect_times_success);*/
  return rc;
}
