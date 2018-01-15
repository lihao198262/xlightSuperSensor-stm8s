#ifndef __SEN_MULTI_H
#define __SEN_MULTI_H
extern uint16_t multi_sensor_alive_tick;
extern uint16_t pm25_value;
extern uint16_t pm10_value;
extern uint16_t tvoc_value;
extern uint16_t ch2o_value;
extern uint16_t co2_value;
extern int16_t tem_value;
extern int16_t hum_value;
void multi_init(void);

#endif /* __SEN_MULTI_H */