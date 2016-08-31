#include <HoCoDIn.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>
#include <tick.h>

//void ICACHE_IRAM_ATTR HoCoDInClass::InterruptCallback(void *data) {
//	HoCoDInClass* c = (HoCoDInClass*)data;
//	c->_InterruptCallback();
//}

//void ICACHE_IRAM_ATTR HoCoDInClass::_InterruptCallback() {
//	if (millis() - _LastInterrupt > _Debounce) {
//		uint8_t v = digitalRead(_Pin);
////		if (v != _LastState) {
//			_LastState = v;
//			SendStatus();
////		}
//	}
//	_LastInterrupt = millis();
//}

HoCoDInClass::HoCoDInClass(char *Name, char *Config, subscribe_callback subscribe, publish_callback publish) : HoCoDeviceClass(Name, subscribe, publish) {
	SetConfig(Config);
}

HoCoDInClass::~HoCoDInClass() {
//	detachInterrupt(_Pin);
}

uint8_t ICACHE_FLASH_ATTR HoCoDInClass::Get() {
	uint8_t r = digitalRead(_Pin);
	if (_Inverted)
		r = 1 - r;
	return r;
}

//void ICACHE_FLASH_ATTR HoCoDInClass::SetStatus(char *Status) {
//}

void ICACHE_FLASH_ATTR HoCoDInClass::SendStatus() {
	DEBUG("HoCoDInClass::SendStatus");
//	char jt[25];
//	ets_sprintf(jt, "{\"in\":%d,\"triggered\":%d}", digitalRead(_Pin), _LastState);
//	OnPublish((char*)"status", jt);
	char jt[3];
	ets_sprintf(jt, "%d", _DebounceState);
	Publish((char*)"triggered", (char*)jt, true);
}

//char ICACHE_FLASH_ATTR *HoCoDInClass::DeviceType() {
//	return (char *)"DIn";
//}

void ICACHE_FLASH_ATTR HoCoDInClass::SetConfig(char *Config) {
	DEBUG("HoCoDInClass::SetConfig %s", Config);
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	pinMode(_Pin, INPUT);
	DEBUG("pin %d", _Pin);
	_Debounce = CppJson::jsonGetInt(Config, "debounce");
	_Trigger = CppJson::jsonGetInt(Config, "trigger");
//	_LastState = digitalRead(_Pin);
}

//void ICACHE_FLASH_ATTR HoCoDInClass::SendConfig() {
//	char jt[50];
//	ets_sprintf(jt, "{\"pin\":%d,\"debounce\":%d,\"trigger\":%d}", _Pin, _Debounce, _Trigger);
//	OnPublish((char*)"status", jt);
//}

void ICACHE_FLASH_ATTR HoCoDInClass::Start() {
	DEBUG("HoCoDInClass::Start");
//	GPIO_INT_TYPE git = GPIO_PIN_INTR_DISABLE;
//	switch (_Trigger) {
//	case 1:
//		git = GPIO_PIN_INTR_POSEDGE;
//		break;
//	case 2:
//		git = GPIO_PIN_INTR_NEGEDGE;
//		break;
//	case 3:
//		git = GPIO_PIN_INTR_ANYEDGE;
//		break;
//	case 4:
		_DebounceState = Get();
		_DebounceLastMillis = mymillis();
		ets_timer_disarm(&LoopTimer);
		ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
		ets_timer_arm_new(&LoopTimer, 50, 1, 0);
//		return;
//	}
//	attachInterrupt(_Pin, InterruptCallback, git, (void*)this);
}

void ICACHE_FLASH_ATTR HoCoDInClass::Stop() {
	DEBUG("HoCoDInClass::Stop");
//	detachInterrupt(_Pin);
}

void ICACHE_FLASH_ATTR HoCoDInClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoDInClass::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoDInClass::TimerLoop(void *data) {
	HoCoDInClass* h = (HoCoDInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoDInClass::_TimerLoop() {
//	DEBUG("%d", mymillis());
    if (mymillis() - _DebounceLastMillis >= _Debounce) {
    	uint8_t currentState = Get();
    	if (_DebounceState != currentState) {
    		_DebounceLastMillis = mymillis();
    		_DebounceState = currentState;
    		if (_Trigger == 2)
    			SendStatus();
    		else if (_Trigger == 0 && _DebounceState)
   				SendStatus();
    		else if (_Trigger == 1 && !_DebounceState)
   				SendStatus();
    	}
    }
}
