#include <avr/io.h>
#include <avr/interrupt.h>
#include "DHT11/BitwiseManager/BitwiseManager_328PB.h"
#include "DHT11/DHT.h"

#define STATUS_SR_SLA_ACK	0x60			// Own SLA+W has been received;	ACK has been returned
#define STATUS_SR_DATA_ACK	0x80			// Previously addressed with own SLA+W;	data has been received;	ACK has been returned
#define STATUS_ST_SLA_ACK	0xA8			// Own SLA+R has been	received; ACK has been returned
#define STATUS_ST_DATA_ACK	0xB8			// Data byte in TWDRn has been transmitted;	ACK has been received

#define FAN_PIN		D9
#define BUZZER_PIN	D10

ISR(TIMER4_COMPA_vect);
ISR(TWI0_vect);

void InitTimer4();
void INIT_TWI(uint8_t twi_address);

uint8_t response_data = 0;

volatile uint8_t process_data = 0;
volatile uint8_t received_data = 0;
volatile uint8_t request_humidity = 0;
	
float temperature = 0.00;
float humidity = 0.00;

int main(void)
{
    cli();
	InitTimer4();
	INIT_TWI(0x36);
	DHT_init(A7);
	sei();
	PIN_MODE(FAN_PIN, OUTPUT);
	PIN_MODE(BUZZER_PIN, OUTPUT);
	
    while (1) 
    {
		if (request_humidity) {
			request_humidity = 0;
			if (DHT_read(&temperature, &humidity));
		}
		if (process_data) {
			process_data = 0;
			switch (received_data) {
				case 0xA1:
					response_data = humidity;
				break;
				case 0xB1:
					DIGITAL_WRITE(FAN_PIN, LOW);
				break;
				case 0xB2:
					DIGITAL_WRITE(FAN_PIN, HIGH);
				break;
				case 0xB3:
					DIGITAL_WRITE(BUZZER_PIN, LOW);
				break;
				case 0xB4:
					DIGITAL_WRITE(BUZZER_PIN, HIGH);
				break;
				default:
					response_data = 0xFF;
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
	request_humidity = 1;
}