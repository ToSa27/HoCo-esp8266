#include <HoCoDIn.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>
#include <tick.h>

void ICACHE_IRAM_ATTR HoCoDInClass::InterruptCallback(void *data) {
	HoCoDInClass* c = (HoCoDInClass*)data;
	c->_InterruptCallback();
}

void ICACHE_IRAM_ATTR HoCoDInClass::_InterruptCallback() {
	if (millis() - _LastInterrupt > _Debounce) {
		uint8_t v = digitalRead(_Pin);
//		if (v != _LastState) {
			_LastState = v;
			SendStatus();
//		}
	}
	_LastInterrupt = millis();
}

HoCoDInClass::HoCoDInClass(char *Name, char *Config, publish_callback publish) : HoCoDeviceClass(Name, publish) {
	SetConfig(Config);
}

HoCoDInClass::~HoCoDInClass() {
	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HoCoDInClass::SetStatus(char *Status) {
}

void ICACHE_FLASH_ATTR HoCoDInClass::SendStatus() {
	char jt[25];
	ets_sprintf(jt, "{\"in\":%d,\"triggered\":%d}", digitalRead(_Pin), _LastState);
	OnPublish((char*)"status", jt);
}

char ICACHE_FLASH_ATTR *HoCoDInClass::DeviceType() {
	return (char *)"DIn";
}

void ICACHE_FLASH_ATTR HoCoDInClass::SetConfig(char *Config) {
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	_Debounce = CppJson::jsonGetInt(Config, "debounce");
	_Trigger = CppJson::jsonGetInt(Config, "trigger");
	_LastState = digitalRead(_Pin);
}

void ICACHE_FLASH_ATTR HoCoDInClass::SendConfig() {
	char jt[50];
	ets_sprintf(jt, "{\"pin\":%d,\"debounce\":%d,\"trigger\":%d}", _Pin, _Debounce, _Trigger);
	OnPublish((char*)"status", jt);
}

void ICACHE_FLASH_ATTR HoCoDInClass::Start() {
	GPIO_INT_TYPE git = GPIO_PIN_INTR_DISABLE;
	switch (_Trigger) {
	case 1:
		git = GPIO_PIN_INTR_POSEDGE;
		break;
	case 2:
		git = GPIO_PIN_INTR_NEGEDGE;
		break;
	case 3:
		git = GPIO_PIN_INTR_ANYEDGE;
		break;
	case 4:
		ets_timer_disarm(&LoopTimer);
		ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
		ets_timer_arm_new(&LoopTimer, _Debounce, 1, 0);
		return;
	}
	attachInterrupt(_Pin, InterruptCallback, git, (void*)this);
}

void ICACHE_FLASH_ATTR HoCoDInClass::Stop() {
	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HoCoDInClass::ReceivedSub(char *Topic, char *Data) {
	DEBUG("HoCoDInClass::ReceivedSub");
}

void ICACHE_FLASH_ATTR HoCoDInClass::TimerLoop(void *data) {
	HoCoDInClass* h = (HoCoDInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoDInClass::_TimerLoop() {
	if (_LastState != digitalRead(_Pin)) {
		_LastState = digitalRead(_Pin);
		SendStatus();
	}
}

