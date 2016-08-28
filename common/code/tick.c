#include <tick.h>
#include <user_interface.h>

// seems like sdk 2.0 includes its own millis() definition
//unsigned long millis() {
//	return system_get_time() / 1000UL;
//}

unsigned long micros() {
	return system_get_time();
}

void delay(uint32_t ms) {
	ets_delay_us(ms * 1000);
}
