#include <tick.h>
#include <user_interface.h>

// seems like sdk 2.0 includes its own millis() definition
// but it seems to return seconds instead of milliseconds
unsigned long mymillis() {
	return system_get_time() / 1000UL;
}

unsigned long mymicros() {
	return system_get_time();
}

void delay(uint32_t ms) {
	ets_delay_us(ms * 1000);
}
