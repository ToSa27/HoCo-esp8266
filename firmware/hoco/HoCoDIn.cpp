#include <HoCoDIn.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>
#include <tick.h>

HoCoDInClass::HoCoDInClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoDInClass::~HoCoDInClass() {
}

uint8_t ICACHE_FLASH_ATTR HoCoDInClass::Get() {
	uint8_t r = digitalRead(_Pin);
	if (_Inverted)
		r = 1 - r;
	return r;
}

void ICACHE_FLASH_ATTR HoCoDInClass::SendStatus() {
	DEBUG("HoCoDInClass::SendStatus");
	char jt[3];
	ets_sprintf(jt, "%d", _DebounceState);
	Publish((char*)"triggered", (char*)jt, true);
}

void ICACHE_FLASH_ATTR HoCoDInClass::SetConfig(char *Config) {
	DEBUG("HoCoDInClass::SetConfig %s", Config);
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	pinMode(_Pin, INPUT);
	_Sample = CppJson::jsonGetInt(Config, "sample");
	_Debounce = CppJson::jsonGetInt(Config, "debounce");
	_Trigger = CppJson::jsonGetChar(Config, "trigger");
}

void ICACHE_FLASH_ATTR HoCoDInClass::Start() {
	DEBUG("HoCoDInClass::Start");
	_DebounceState = Get();
	_DebounceLastMillis = mymillis();
	ets_timer_disarm(&LoopTimer);
	if (_Sample > 0) {
		ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
		ets_timer_arm_new(&LoopTimer, _Sample, 1, 0);
	}
}

void ICACHE_FLASH_ATTR HoCoDInClass::Stop() {
	DEBUG("HoCoDInClass::Stop");
	ets_timer_disarm(&LoopTimer);
}

void ICACHE_FLASH_ATTR HoCoDInClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoDInClass::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoDInClass::TimerLoop(void *data) {
	HoCoDInClass* h = (HoCoDInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoDInClass::_TimerLoop() {
    if (mymillis() - _DebounceLastMillis >= _Debounce) {
    	uint8_t currentState = Get();
    	if (_DebounceState != currentState) {
    		_DebounceLastMillis = mymillis();
    		_DebounceState = currentState;
    		if (_Trigger == 'a')
    			SendStatus();
    		else if (_Trigger == 'r' && _DebounceState)
   				SendStatus();
    		else if (_Trigger == 'f' && !_DebounceState)
   				SendStatus();
    	}
    }
}
