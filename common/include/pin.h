#ifndef __HOCO_PIN_H__
#define __HOCO_PIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>
#include <gpio.h>

#define PINCOUNT 16
#define ESP_MAX_INTERRUPTS 16

#define HIGH 					0x1
#define LOW  					0x0

#define INPUT 					0x0
#define OUTPUT 					0x1
#define INPUT_PULLUP 			0x2
#define INPUT_PULLDOWN 			0x3
#define OUTPUT_OPEN_DRAIN 		0x4

void pinMode(uint8_t pin, uint8_t mode);
void pullup(uint16_t pin);
void pulldown(uint16_t pin);
void noPullup(uint16_t pin);

void digitalWrite(uint8_t pin, uint8_t val);
uint8_t digitalRead(uint8_t pin);
void analogWrite(uint8_t pin, int duty);
uint16_t analogRead();

typedef void (*InterruptCallback)(void *);
void attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode, void *data);
void detachInterrupt(uint8_t pin);
void noInterrupts();
void interrupts();

#ifdef __cplusplus
}
#endif

#endif
