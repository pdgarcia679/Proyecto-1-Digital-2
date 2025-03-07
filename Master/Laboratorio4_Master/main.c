#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdlib.h>
#include "LCD/BitwiseManager/BitwiseManager_328PB.h"
#include "LCD/LiquidCrystal_1602A.h"
#include "USART/USART0.h"

/* Master wants to send data to a slave */
// (1) Master-transmitter sends a START condition and addresses the slave-receiver
// (2) Master-transmitter sends data to slave-receiver
// (3) Master-transmitter terminates the transfer with a STOP condition

/* Master wants to read data from a slave */
// (1) Master-receiver sends a START condition and addresses the slave-transmitter
// (2) Master-receiver sends the requested register to read to slave-transmitter
// (3) Master-receiver receives data from the slave-transmitter
// (4) Master-receiver terminates the transfer with a STOP condition

#define STATUS_START		0x08
#define STATUS_MT_SLA_ACK	0x18
#define STATUS_MT_DATA_ACK	0x28
#define STATUS_MR_SLA_ACK	0x40
#define STATUS_MR_DATA_ACK	0x50
#define STATUS_MR_DATA_NACK 0x58
#define STATUS_REP_START	0x10

#define LM75_address		0b01001001	// 0x49
#define Slave0_MQ_address	0x42
#define Slave1_DHT_address	0x36

/*
typedef enum {
	STATUS_START = 0x08,				// A START condition has been	transmitted
	STATUS_REP_START = 0x10,			// A repeated START condition	has been transmitted
	STATUS_MT_SLA_ACK = 0x18,			// SLA+W has been transmitted; ACK has been received
	STATUS_MT_SLA_NACK = 0x20,			// SLA+W has been transmitted; NOT ACK has been received
	STATUS_MT_DATA_ACK = 0x28,			// Data byte has been	transmitted; ACK has been received
	STATUS_MT_DATA_NACK = 0x30,			// Data byte has been	transmitted; NOT ACK has been received
	STATUS_MT_ARB_LOST = 0x38,			// Arbitration lost in SLA+R/W or data bytes

	STATUS_MR_SLA_ACK = 0x40,			// SLA+R has been transmitted; ACK has been received
	STATUS_MR_SLA_NACK = 0x48,			// SLA+R has been transmitted; NOT ACK has been received
	STATUS_MR_DATA_ACK = 0x50,			// Data byte has been received; ACK has been returned
	STATUS_MR_DATA_NACK = 0x58,			// Data byte has been received; NOT ACK has been returned

	STATUS_SR_SLA_ACK = 0x60,			// Own SLA+W has been received;	ACK has been returned
	STATUS_SR_ARB_LOST_SLA_ACK = 0x68,	// Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
	STATUS_SR_DATA_ACK = 0x80,			// Previously addressed with own SLA+W;	data has been received;	ACK has been returned
	STATUS_SR_DATA_NACK = 0x88,			// Previously addressed with own SLA+W;	data has been received;	NOT ACK has been returned
	STATUS_SR_STOP = 0xA0,				// A STOP condition or repeated	START condition has been received while still addressed	as Slave

	STATUS_ST_SLA_ACK = 0xA8,			// Own SLA+R has been	received; ACK has been returned
	STATUS_ST_ARB_LOST_SLA_ACK = 0xB0,	// Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
	STATUS_ST_DATA_ACK = 0xB8,			// Data byte in TWDRn has been transmitted;	ACK has been received
	STATUS_ST_DATA_NACK = 0xC0,			// Data byte in TWDRn has been transmitted; NOT ACK has been received
	STATUS_ST_LAST_DATA = 0xC8,			// Last data byte in TWDRn has been transmitted (TWEA =	“0”); ACK has been received

	STATUS_NO_INFO = 0xF8,				// No relevant state information available; TWINT = “0”
	STATUS_BUS_ERROR = 0x00				// Bus error due to an illegal START or STOP condition
};
*/
uint8_t TWI_WriteData(uint8_t twi_address, uint8_t twi_load);
uint8_t TWI_ReadData(uint8_t twi_address, uint8_t data_address, uint8_t* data_pointer);

ISR(USART0_RX_vect);
ISR(TIMER4_COMPA_vect);

void InitTimer4();
void Init_TWI();
void TWI_ACK();
void TWI_START();
void TWI_STOP();
char* IntToString(int IntToConvert);
volatile char character_rx;
volatile uint8_t fan_command = 0;
volatile uint8_t pause_offline = 0;
volatile uint8_t twi_busy = 0;

volatile uint8_t update_data;
uint8_t counter = 0;
// uint8_t TWI_STATUS();
/* Needs more information to define the specific trajectory and frame (big function)
*  User can use different functions and each pass the specific parameters to the big function */

uint8_t received_data_humidity;
uint8_t received_data_gas;
uint8_t received_data_temperature;

int main(void)
{
    cli();
	Init_TWI();
	InitTimer4();
	SerialBegin(Asynch, 9600, NormalSpeed, True, False);
	lcd_begin(D11, D10, D9, D8, D7, D6, D5, D4, D3, D2);
	lcd_clear();
	lcd_setcursor(1, 1);
	lcd_print("x42:0");
	lcd_setcursor(1, 9);
	lcd_print("x36:0");
	lcd_setcursor(2, 1);
	lcd_print("x49:0");
	sei();
						
	_delay_ms(10);
	
    while (1) 
    {	
		if (fan_command != 0 && !twi_busy) {
			if (fan_command == 1) {
				TWI_WriteData(Slave1_DHT_address, 0xB1);
				fan_command = 0;
			}
			if (fan_command == 2) {
				TWI_WriteData(Slave1_DHT_address, 0xB2);
				fan_command = 0;
			}
			fan_command = 0;
			pause_offline = 0;
		}
		if (!pause_offline && !twi_busy) {
			if (update_data) {
				twi_busy = 1;
				update_data = 0;
				/* Slave 0: sensor MQ-2 in address 0x42 */
				_delay_ms(2);
				if (TWI_ReadData(Slave0_MQ_address, 0xA1, &received_data_gas));
				_delay_ms(2);
				if (received_data_gas > 100)	TWI_WriteData(Slave0_MQ_address, 0xB2);
				else TWI_WriteData(Slave0_MQ_address, 0xB1);
			
				lcd_setcursor(1, 1);
				lcd_print("x42:");
				lcd_print("    ");
				lcd_setcursor(1, 5);
				lcd_print(IntToString(received_data_gas));
			
				/* Slave 1: sensor DHT-11 in address 0x36 */
				_delay_ms(2);
				if (TWI_ReadData(Slave1_DHT_address, 0xA1, &received_data_humidity));
				_delay_ms(2);
				if (received_data_humidity > 80)	TWI_WriteData(Slave1_DHT_address, 0xB2);
				else TWI_WriteData(Slave1_DHT_address, 0xB1);
			
				lcd_setcursor(1, 9);
				lcd_print("x36:");
				lcd_print("    ");
				lcd_setcursor(1, 13);
				lcd_print(IntToString(received_data_humidity));
				lcd_print("%");
			
				/* Slave 3: LM75 in address 0x49 */
				_delay_ms(2);
				if (TWI_ReadData(0x49, 0x00, &received_data_temperature));
				_delay_ms(2);
				if (received_data_temperature > 30)	TWI_WriteData(Slave1_DHT_address, 0xB4);
				else TWI_WriteData(Slave1_DHT_address, 0xB3);
			
				lcd_setcursor(2, 1);
				lcd_print("x49:");
				lcd_print("     ");
				lcd_setcursor(2, 5);
				lcd_print(IntToString(received_data_temperature));
				lcd_write_char(0xDF);
				lcd_print("C");
				
				twi_busy = 0;
			}
		}
	}
}



uint8_t TWI_WriteData(uint8_t twi_address, uint8_t twi_load) {
	TWI_START();
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_START)	{
		TWI_STOP();
		return 1;
	}
	
	TWDR0 = (twi_address << 1) | 0;
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MT_SLA_ACK)	{
		TWI_STOP();
		return 1;
	}
	
	TWDR0 = twi_load;
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MT_DATA_ACK)	{
		TWI_STOP();
		return 1;
	}
	
	TWI_STOP();
	return 0;
}

uint8_t TWI_ReadData(uint8_t twi_address, uint8_t data_address, uint8_t* data_pointer) {
	TWI_START();
	while (!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_START)	{
		TWI_STOP();
		return 1;
	}
	
	TWDR0 = (twi_address << 1) | 0;
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MT_SLA_ACK)	{
		TWI_STOP();
		return 1;
	}
	
	TWDR0 = data_address;
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MT_DATA_ACK)	{
		TWI_STOP();
		return 1;
	}
	
	TWI_START();
	while (!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_REP_START)	{
		TWI_STOP();
		return 1;
	}
	
	TWDR0 = (twi_address << 1) | 1;
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MR_SLA_ACK) {
		TWI_STOP();
		return 1;
	}
		
	TWCR0 = (1<<TWEN) | (1<<TWINT);
	while(!(TWCR0 & (1<<TWINT)));
	if ((TWSR0 & 0xF8) != STATUS_MR_DATA_NACK)	{	// Other does not verify this
		TWI_STOP();
		return 1;
	}
	*data_pointer = TWDR0;
	
	TWI_STOP();
	return 0;
}

void InitTimer4() {
	TCCR4A = 0;						// Normal port operation of OC4A/OC4B
	TCCR4B |= (1<<WGM42);			// Mode 4: CTC with OCR4A as TOP
	TCCR4B |= (1<<CS42 | 1<<CS40);	// Prescaler /1024.
	TIMSK4 |= (1<<OCIE4A);			// Enable interrupt
	OCR4A = 15623;					// Frequency: 1 Hz (no output compare)
}

void Init_TWI() {
	TWSR0 &= ~(1<<TWPS0);
	TWSR0 &= ~(1<<TWPS1);
	TWBR0 = 72;
	TWCR0 = (1<<TWEN);
}

void TWI_START() {
	/* (1) The TWI hardware checks if the bus is available, and generates a START condition on the bus if it is free.
	*  (2) The TWEN bit enables TWI operation and activates the TWI interface.
	*  (3) This bit is set by hardware when the TWI has finished its current job and expects application software response. */
	TWCR0 = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT);	// (1) | (2) | (3)
}

void TWI_STOP() {
	/* (1) Writing the TWSTO bit to one in Master mode will generate a STOP condition on the 2-wire Serial Bus TWI.
	*  (2) The TWEN bit enables TWI operation and activates the TWI interface.
	*  (3) This bit is set by hardware when the TWI has finished its current job and expects application software response. */
	TWCR0 |= (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);	// (1) | (2) | (3)
	while (TWCR0 & (1<<TWSTO));
}

/*
uint8_t TWI_STATUS() { // Needs to know: twi_mode, twi_load
	// (1) TWSTA must be cleared by	software when the START condition has been transmitted.
	// (2) The TWEN bit enables TWI operation and activates the TWI interface.
	// (3) The TWINT Flag must be cleared by software by writing a logic one to it.	
	// (4) Writing the TWSTO bit to one in Master mode will generate a STOP condition on the 2-wire Serial Bus TWI.
	twi_status = TWSR0 & 0xF8;
	switch (twi_status) {
		case STATUS_START:
			TWDR0 = (twi_address << 1) | twi_mode;
			TWCR0 = (1<<TWEN) | (1<<TWINT);
		break;
		case STATUS_REP_START:
			TWDR0 = (twi_address << 1) | twi_mode;
			TWCR0 = (1<<TWEN) | (1<<TWINT);		// ~(1) | (2) | (3)
		break;
		
		case STATUS_MT_SLA_ACK:
			TWDR0 = twi_load;
			TWCR0 = (1<<TWEN) | (1<<TWINT);		// ~(1) | (2) | (3)
		break;
		
		case STATUS_MT_SLA_NACK:
			TWI_STOP();
		break;
		
		case STATUS_MT_DATA_ACK:
			TWCR0 = (1<<TWEN) | (1<<TWINT);		// ~(1) | (2) | (3)
		break;
		
		case STATUS_MT_DATA_NACK:
			TWI_STOP();
		break;
	}
}
*/

ISR(USART0_RX_vect) {
	char character_rx = UDR0;
	if (character_rx == '\n' || character_rx == '\r') {
			return;
	}
	switch (character_rx) {
		case 'v':
		fan_command = 1;
		pause_offline = 1;
		break;
		
		case 'V':
		fan_command = 2;
		pause_offline = 1;
		break;
		default:
				
		break;
	}
}

ISR(TIMER4_COMPA_vect) {
	update_data = 1;
	counter++;
	if (counter >= 4) {
		counter = 0;
		/*
		char buffer[16];
		sprintf(buffer, "H:%d\n", received_data_humidity);
		SerialWriteText(buffer);
		*/
		char buffer[64];  // Buffer lo suficientemente grande para los 3 datos

		// Enviar los datos en líneas separadas con el formato esperado
		sprintf(buffer, "H:%d\nT:%d\nG:%d\n", received_data_humidity, received_data_temperature, received_data_gas);
	
		SerialWriteText(buffer);  // Enviar toda la cadena al ESP32
	}
}
