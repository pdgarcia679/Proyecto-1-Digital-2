#ifndef DHT_H_
#define DHT_H_

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "BitwiseManager/BitwiseManager_328PB.h"

#define max_wait_time		255		// Maximum time to wait response
#define dht_pulse_width		30		// Necessary to verify if data is 0 or 1

extern uint8_t DHT_pin;				// Allows you to use any pin

void DHT_init(uint8_t data_pin);	// Pin mode and timer configuration
void DHT_request();					// Request data procedure
uint8_t DHT_response();				// Verify response of DHT module
uint8_t DHT_capture();				// Capture 8 bits of data

uint8_t DHT_read(float *temperature, float *humidity); // Reading 40 bits of data: temp(int), temp(dec), hum(int), hum(dec), checksum

#endif /* DHT_H_ */