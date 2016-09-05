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

void ICACHE_FLASH_ATTR pinMode(uint8_t pin, uint8_t mode);
void ICACHE_FLASH_ATTR pullup(uint16_t pin);
void ICACHE_FLASH_ATTR pulldown(uint16_t pin);
void ICACHE_FLASH_ATTR noPullup(uint16_t pin);

void ICACHE_FLASH_ATTR digitalWrite(uint8_t pin, uint8_t val);
uint8_t ICACHE_FLASH_ATTR digitalRead(uint8_t pin);
void ICACHE_FLASH_ATTR analogWrite(uint8_t pin, int duty);
uint16_t ICACHE_FLASH_ATTR analogRead();

typedef void (*InterruptCallback)(void *);
void ICACHE_FLASH_ATTR attachInterrupt(uint8_t pin, InterruptCallback callback, GPIO_INT_TYPE mode, void *data);
void ICACHE_FLASH_ATTR detachInterrupt(uint8_t pin);
void ICACHE_FLASH_ATTR noInterrupts();
void ICACHE_FLASH_ATTR interrupts();

#ifdef __cplusplus
}
#endif

#endif
