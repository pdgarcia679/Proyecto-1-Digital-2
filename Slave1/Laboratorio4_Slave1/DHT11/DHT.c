#include "DHT.h"

uint8_t DHT_pin;

void DHT_init(uint8_t data_pin) {
	DHT_pin = data_pin;											// Global variable
	TCCR3A &= ~(1<<COM3A0 | 1<<COM3A1 | 1<<WGM31 | 1<<WGM30);	// Normal port operation and mode bits XX00
	TCCR3B &= ~(1<<WGM33 | 1<<WGM32);							// Mode bits 0000 (0): normal with 0xFFFF as TOP
	TCCR3B |= (1<<CS31);										// Prescaler /8: Frequency 2 MHz (T = 500 ns)
	TCNT3 = 0;													// Restart timer
}

void DHT_request() {
	PIN_MODE(DHT_pin, OUTPUT);			// Set pin as output
	DIGITAL_WRITE(DHT_pin, LOW);		// Send start bit low
	_delay_ms(20);						// Hold for 20 ms
	DIGITAL_WRITE(DHT_pin, HIGH);		// Release bus
	_delay_us(20);						// Wait at least 20 us
	PIN_MODE(DHT_pin, INPUT_PULLDOWN);  // Set pin as input
	_delay_us(20);						// Wait next 20 us
}

uint8_t DHT_response() {
	TCNT3 = 0;															// Restart timer
	while((DIGITAL_READ(DHT_pin)) && !(TCNT3 > (max_wait_time * 2)));	// Continue if pin changes to low or timed out (1 us)
	if (TCNT3 > (max_wait_time * 2)) return 0;							// Exit if timed out
	
	TCNT3 = 0;															// Restart timer
	while(!(DIGITAL_READ(DHT_pin)) && !(TCNT3 > (max_wait_time * 2)));	// Continue if pin changes to high or timed out (1 us)
	if (TCNT3 > (max_wait_time * 2)) return 0;							// Exit if timed out
	
	return 1;															// Successful exit
}

uint8_t DHT_capture() {
	uint8_t i, result = 0;
	for (i = 0; i < 8; i++) {													// For each data bit
		TCNT3 = 0;																// Restart timer
		while((DIGITAL_READ(DHT_pin)) && !(TCNT3 > (max_wait_time * 2)));		// Continue if pin changes to low or timed out (1 us)
		TCNT3 = 0;																// Restart timer
		while(!(DIGITAL_READ(DHT_pin)) && !(TCNT3 > (max_wait_time * 2)));		// Continue if pin changes to high or timed out (1 us)
		TCNT3 = 0;																// Restart timer
		while(!(TCNT3 > (dht_pulse_width * 2)));								// Wait 30 us to verify type of bit
		if (DIGITAL_READ(DHT_pin)) result |= (1<<(7-i));						// Add bit, if continues high is 1 else is 0
	}
	return result;																// Return byte
}

uint8_t DHT_read(float *temperature, float *humidity) {							// Uses pointers to dump information
	uint8_t rh_int, rh_dec, temp_int, temp_dec, checksum;

	DHT_request();																// Step 1: request procedure
	if (!DHT_response()) return 0;												// Step 2: verify response

	rh_int = DHT_capture();														// Step 3: capture related humidity integer byte
	rh_dec = DHT_capture(); 													// Step 4: capture related humidity decimal byte
	temp_int = DHT_capture();  													// Step 5: capture temperature integer byte
	temp_dec = DHT_capture();  													// Step 6: capture temperature decimal byte
	checksum = DHT_capture();													// Step 7: capture the byte of sum previous bytes

	if ((rh_int + rh_dec + temp_int + temp_dec) != checksum) return 0;			// Step 8: check if sum is correct, exit if not

	*humidity = rh_int;															// Dump related humidity
	*temperature = temp_int;													// Dump temperature
	
	return 1;																	// Successful exit
}
