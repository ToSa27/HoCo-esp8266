#include <HoCoCounter.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>
#include <tick.h>

HoCoCounterClass::HoCoCounterClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoCounterClass::~HoCoCounterClass() {
}

void ICACHE_FLASH_ATTR HoCoCounterClass::SendStatus() {
	DEBUG("HoCoCounterClass::SendStatus");
	char jt[3];
	ets_sprintf(jt, "%d", _Count);
	Publish((char*)"count", (char*)jt, true);
}

void ICACHE_FLASH_ATTR HoCoCounterClass::SetConfig(char *Config) {
	DEBUG("HoCoCounterClass::SetConfig %s", Config);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	pinMode(_Pin, INPUT);
	_Trigger = CppJson::jsonGetChar(Config, "trigger");
	_Interval = CppJson::jsonGetInt(Config, "interval");
}

void ICACHE_FLASH_ATTR HoCoCounterClass::Start() {
	DEBUG("HoCoCounterClass::Start");
	pinMode(_Pin, INPUT_PULLUP);
	GPIO_INT_TYPE git = GPIO_PIN_INTR_DISABLE;
	switch (_Trigger) {
	case 'r':
		git = GPIO_PIN_INTR_POSEDGE;
		break;
	case 'f':
		git = GPIO_PIN_INTR_NEGEDGE;
		break;
	case 'a':
		git = GPIO_PIN_INTR_ANYEDGE;
		break;
	}
	_Count = 0;
	ets_timer_disarm(&LoopTimer);
	ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
	ets_timer_arm_new(&LoopTimer, _Interval, 1, 0);
	attachInterrupt(_Pin, InterruptCallback, git, (void*)this);
}

void ICACHE_FLASH_ATTR HoCoCounterClass::Stop() {
	DEBUG("HoCoCounterClass::Stop");
	ets_timer_disarm(&LoopTimer);
	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HoCoCounterClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoCounterClass::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoCounterClass::TimerLoop(void *data) {
	HoCoCounterClass* h = (HoCoCounterClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoCounterClass::_TimerLoop() {
	SendStatus();
	_Count = 0;
}

void ICACHE_IRAM_ATTR HoCoCounterClass::InterruptCallback(void *data) {
	HoCoCounterClass* c = (HoCoCounterClass*)data;
	c->_InterruptCallback();
}

void ICACHE_IRAM_ATTR HoCoCounterClass::_InterruptCallback() {
	_Count++;
}
