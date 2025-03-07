#include "PWM1.h"

float map(float x, float in_min, float in_max, float out_min, float out_max) {
	return ((x - in_min)*(out_max - out_min)/(in_max - in_min)) + out_min;
}

uint8_t adc_value;

void initFastPWM1(void){
	//TCCR1A |= (1 << WGM11);			// PWM con ICR (valMax)
	//TCCR1B |= (1 << WGM12) | (1 << WGM13);

	DDRB |= (1<<DDRB1)|(1<<DDRB2);
	TCNT1 = 0; // reset
	ICR1 = 39999; // TOP
	TCCR1A =  (1 << COM1A1) | (1 << COM1B1) | (0 << COM1A0) ; // low --> Compare Match
	TCCR1A |=  (1 << WGM11) | (0 << WGM10) ; // Fast PWM TOP ICR1
	TCCR1B = (1 << WGM13) | (1 << WGM12); // Fast PWM TOP ICR1
	TCCR1B |= (0 << CS12) | (1 << CS11) | ( 0 << CS10 ); // Prescaler 8

}

void Prescaler(uint8_t prescaler){
	if (prescaler == 8){
		TCCR1B |= (1 << CS11);//Prescaler 8
	}
	else if (prescaler == 1024){
		TCCR1B |= (1 << CS12) | (1 << CS10);//Prescaler 12
	}
}

void ConfCanalNinverted(void){
	TCCR1A |= (1 << COM1B1)|(1<< COM1A1);
}

void Servo1(uint16_t Servo_ang) {
	//OCR1B = (200/12)*ADCH+1000;
	//uint16_t duty_cycle = map(Servo_ang, 0, 180, 1000, 4800);
	//OCR1B = ((adc_value * 200) / 1023) + 1000;
	//OCR1B = map(Servo_ang, 0, 180, 0, 7000);
	OCR1B = map(Servo_ang, 0, 180, 1080, 4000);

}

void Servo2(uint16_t Servo_ang){
	OCR1A = map(Servo_ang, 0, 180, 1080, 4000);
}