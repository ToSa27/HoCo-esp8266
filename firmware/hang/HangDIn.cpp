#include <HangDIn.h>
#include <CppJson.h>
#include <debug.h>
#include <pin.h>
#include <tick.h>

void ICACHE_IRAM_ATTR HangDInClass::InterruptCallback(void *data) {
	HangDInClass* c = (HangDInClass*)data;
	c->_InterruptCallback();
}

void ICACHE_IRAM_ATTR HangDInClass::_InterruptCallback() {
	if (millis() - _LastInterrupt > _Debounce) {
		uint8_t v = digitalRead(_Pin);
//		if (v != _LastState) {
			_LastState = v;
			SendStatus();
//		}
	}
	_LastInterrupt = millis();
}

HangDInClass::HangDInClass(char *Name, char *Config, publish_callback publish) : HangDeviceClass(Name, publish) {
	SetConfig(Config);
}

HangDInClass::~HangDInClass() {
	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HangDInClass::SetStatus(char *Status) {
}

void ICACHE_FLASH_ATTR HangDInClass::SendStatus() {
	char jt[25];
	ets_sprintf(jt, "{\"in\":%d,\"triggered\":%d}", digitalRead(_Pin), _LastState);
	OnPublish((char*)"status", jt);
}

char ICACHE_FLASH_ATTR *HangDInClass::DeviceType() {
	return (char *)"DIn";
}

void ICACHE_FLASH_ATTR HangDInClass::SetConfig(char *Config) {
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	_Debounce = CppJson::jsonGetInt(Config, "debounce");
	_Trigger = CppJson::jsonGetInt(Config, "trigger");
	_LastState = digitalRead(_Pin);
}

void ICACHE_FLASH_ATTR HangDInClass::SendConfig() {
	char jt[50];
	ets_sprintf(jt, "{\"pin\":%d,\"debounce\":%d,\"trigger\":%d}", _Pin, _Debounce, _Trigger);
	OnPublish((char*)"status", jt);
}

void ICACHE_FLASH_ATTR HangDInClass::Start() {
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

void ICACHE_FLASH_ATTR HangDInClass::Stop() {
	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HangDInClass::ReceivedSub(char *Topic, char *Data) {
	DEBUG("HangDInClass::ReceivedSub");
}

void ICACHE_FLASH_ATTR HangDInClass::TimerLoop(void *data) {
	HangDInClass* h = (HangDInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HangDInClass::_TimerLoop() {
	if (_LastState != digitalRead(_Pin)) {
		_LastState = digitalRead(_Pin);
		SendStatus();
	}
}

