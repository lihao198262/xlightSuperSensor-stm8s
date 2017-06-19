
//adapted from http://www.amobbs.com/thread-5517880-1-1.html?_dsign=d9eb7efa//
#include "sen_dht.h"
#include "timer_4.h"


#define DHT_MA_NUM             20
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

u16 dht_mvTemData[DHT_MA_NUM] = {0};
u16 dht_mvHumData[DHT_MA_NUM] = {0};
u32 dht_tem_mvSum = 0;
u32 dht_hum_mvSum = 0;

RESULT DHT_GetData(u16 * t, u16 * h);
u8 DHT_ReadData(u8 *data);
u8 DHT_CheckSum(u8 * data);

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
      dht_tem_mvPtr = (dht_tem_mvPtr + 1) % DHT_MA_NUM;
      if( newHumData != dht_mvHumData[dht_hum_mvPtr] ) {
        dht_hum_mvSum += newHumData;
        dht_hum_mvSum -= dht_mvHumData[dht_hum_mvPtr];
        dht_mvHumData[dht_hum_mvPtr] = newHumData;
      }  
      dht_hum_mvPtr = (dht_hum_mvPtr + 1) % DHT_MA_NUM;
      if( !dht_tem_ready ) {
        dht_tem_ready = (dht_tem_mvPtr == 0);
      }
      if( !dht_hum_ready ) {
        dht_hum_ready = (dht_hum_mvPtr == 0);
      }
      if( dht_tem_ready )
      {
        dht_tem_value = dht_tem_mvSum / DHT_MA_NUM;
      }
      if( dht_hum_ready ) 
      {
        dht_hum_value = dht_hum_mvSum / DHT_MA_NUM;
      }
  }
  return dht_tem_ready || dht_hum_ready;
}

unsigned char U8FLAG,U8temp;

u8 DHT_ReadData(u8 *data)
{
    unsigned char i,j;

    DHT_OUT();
    DHT_Low();      //DHT11=0
    Delayms(20); 	  //delay 20ms
    DHT_High();     //DHT11=1
    Delay10Us(4);	  //delay 40us
    DHT_IN();       //DHT11_input
    U8FLAG=1;
    while ( !DHT_Read()/* && U8FLAG++ < 255*/); //wait DHT11 fist 80us low singal
    /*if(U8FLAG == 255) 
    {
      return 1;
    }*/
    while((DHT_Read())/*&& U8FLAG++ < 255*/); //wait DHT11 fist 80us high singal
    /*if(U8FLAG == 255) 
    {
      return 1;
    }*/
        for(j = 0; j<5; j++) { //read 5 bytes data
            for(i=0; i<8; i++) {
                //U8FLAG=1;
                while(!DHT_Read()/*&&U8FLAG++ < 255*/);//wait the first 50us low singal
                /*if(U8FLAG == 255) 
                {
                  return 1;
                }*/
                Delay10Us(3);
                U8temp=0;

                if(DHT_Read())
                    U8temp=1;//wait the high singal if over 30us the this bit set to 1
               
                data[j]<<=1;
                data[j]|=U8temp;
                while(DHT_Read());
            }

        }

        if(data[4] == (data[0] + data[1] + data[2] + data[3])) { // check data with check sum
            return 0;
        }
        return 0;
}

RESULT DHT_GetData(u16 * t, u16 * h)
{
  u8 tmp[5]={0};
  if (DHT_ReadData(tmp) == 1)
    return RESULT_ERRREAD;
  //if (tmp[4] == DHT_CheckSum(tmp))
  //{
    // decimal part process
    u8 dectmp = tmp[1];
    if (dectmp < 10 ) dectmp *= 10;
    else if(dectmp >=100) dectmp /= 10;
    *h = tmp[0]*100 + dectmp;
    dectmp = tmp[3];
    if (dectmp < 10 ) dectmp *= 10;
    else if(dectmp >=100) dectmp /= 10;
    *t = tmp[2]*100 + dectmp;
  //}
  //else
  //  return RESULT_ERRCHKSUM;
  return RESULT_OK;
}

u8 DHT_CheckSum(u8* data)
{
  u16 sum = 0;
  for (int i = 0; i < 4; i++)
    sum += *(data + i);
  return (u8)sum;
}