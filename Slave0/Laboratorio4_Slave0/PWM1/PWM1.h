#ifndef PWM1_H_
#define PWM1_H_

#include <avr/io.h>
#include <stdint.h>

void initFastPWM1(void);
void Prescaler(uint8_t prescaler);
void ConfCanalNinverted(void);
void Servo1(uint16_t Servo_ang);
void Servo2(uint16_t Servo_ang);

//uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
float map(float x, float in_min, float in_max, float out_min, float out_max);

#endif /* PWM1_H_ */