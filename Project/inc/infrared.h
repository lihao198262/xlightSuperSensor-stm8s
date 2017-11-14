

//******************************************************************************        
//name:         infrared.h        
//introduce:    红外发射    
//author:       lihao      
//changetime:   2017.06.02     
//email:        lihao198262@163.com    
//******************************************************************************


#ifndef __INFRARED_H
#define __INFRARED_H

#include "stm8s.h"

#define CHANNEL1

#define HAIER_CON 0xA1

#define MEDIA_CON 0xA2

#define AIRCON_MEDIA
//#define AIRCON_HAIER
#ifdef AIRCON_MEDIA
extern char mediaoff[4];
extern char media_last_on_status[4];
#endif
#ifdef AIRCON_HAIER
extern char haieroff[15];
extern char haier_last_on_status[15];
#endif



/*******************************************************************************
 * 名称: Haier_Infrared_Send
 * 功能: 红外发射 类型海尔
 * 形参: unsigned long data
 * 返回: 无
 * 说明: 无 
 ******************************************************************************/
void Haier_Infrared_Send(uint8_t data[], int len);

/*******************************************************************************
 * 名称: NEC_Infrared_Send
 * 功能: 红外发射 类型NEC
 * 形参: unsigned long data
 * 返回: 无
 * 说明: 无 
 ******************************************************************************/
void NEC_Infrared_Send(unsigned long data);

/*******************************************************************************
 * 名称: Media_Infrared_Send
 * 功能: 红外发射
 * 形参: uint8_t base 
 * 形参: uint8_t high 
 * 形参: uint8_t low 
 * 返回: 无
 * 说明: 无 
 ******************************************************************************/
void Media_Infrared_Send(uint8_t base, uint8_t high, uint8_t low);


/*******************************************************************************
 * 名称: Infrared_Send
 * 功能: 红外发射
 * 形参: unsigned long data
 * 返回: 无
 * 说明: 无 
 ******************************************************************************/
void Infrared_Send(unsigned long data);

/*******************************************************************************
 * 名称: Infrared_Init()
 * 功能: 初始化函数
 * 形参: 无
 * 返回: 无
 * 说明: 无 
 ******************************************************************************/
void Infrared_Init(void);

extern u16 ir_send_delay;

bool Set_Send_Buf(u32 *buf, u8 len);

bool Set_AC_Buf(uint8_t *buf, u8 len);

bool Set_AC_Media_Buf(uint8_t *buf, u8 len);

void IR_Send();

#endif


