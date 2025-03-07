#include <avr/io.h>
#include <avr/interrupt.h>
#include "BitwiseManager/BitwiseManager_328PB.h"
#include "ADC/ADC.h"
#include "PWM1/PWM1.h"

#define STATUS_SR_SLA_ACK	0x60			// Own SLA+W has been received;	ACK has been returned
#define STATUS_SR_DATA_ACK	0x80			// Previously addressed with own SLA+W;	data has been received;	ACK has been returned
#define STATUS_ST_SLA_ACK	0xA8			// Own SLA+R has been	received; ACK has been returned
#define STATUS_ST_DATA_ACK	0xB8			// Data byte in TWDRn has been transmitted;	ACK has been received

ISR(TIMER4_COMPA_vect);
ISR(TWI0_vect);

void InitTimer4();
void INIT_TWI(uint8_t twi_address);
void initFastPWM1(void);

uint8_t response_data = 0;

volatile uint8_t process_data = 0;
volatile uint8_t received_data = 0;
volatile uint8_t update_gas = 0;

uint8_t gas_level = 0;

int main(void)
{
    cli();
	InitTimer4();
	initFastPWM1();
	INIT_TWI(0x42);
	SettingADC(AVcc, LeftJustified, ADC_Prescaler128, FreeRunning);
	sei();
	
    while (1) 
    {
		if (update_gas) {
			update_gas = 0;
			gas_level = AnalogRead(ChannelADC7);
		}
		if (process_data) {
			process_data = 0;
			switch (received_data) {
				case 0xA1:
					response_data = gas_level;
				break;
				case 0xB1:
					Servo2(0);
				break;
				case 0xB2:
					Servo2(180);
				break;
				default:
					response_data = '!';
				break;
			}
		}
    }
}

void InitTimer4() {
	TCCR4A = 0;						// Normal port operation of OC4A/OC4B
	TCCR4B |= (1<<WGM42);			// Mode 4: CTC with OCR4A as TOP
	TCCR4B |= (1<<CS42 | 1<<CS40);	// Prescaler /1024.
	TIMSK4 |= (1<<OCIE4A);			// Enable interrupt
	OCR4A = 15623;					// Frequency: 1 Hz (no output compare)
}

void INIT_TWI(uint8_t twi_address) {
	/* (1) The device's own slave address has been received.
	*  (2) The TWEN bit enables TWI n operation and activates the TWI n interface.
	*  (3) the TWI n interrupt request will be activated for as long as the TWCRn.TWINT Flag is high. */
	TWAR0 = (twi_address << 1);
	TWCR0 = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
}

ISR(TWI0_vect) {
	switch (TWSR0 & 0xF8) {
		case STATUS_SR_SLA_ACK:
			TWCR0 = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		case STATUS_SR_DATA_ACK:
			received_data = TWDR0;
			process_data = 1;
			TWCR0 = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		case STATUS_ST_SLA_ACK:
			TWDR0 = response_data;
			TWCR0 = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		case STATUS_ST_DATA_ACK:
			TWDR0 = response_data;
			TWCR0 = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		default:
			TWCR0 = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
	}
}

ISR(TIMER4_COMPA_vect) {
	update_gas = 1;
}