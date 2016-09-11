#include <HoCoPCF8574DIn.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8574DInClass::HoCoPCF8574DInClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoPCF8574DInClass::~HoCoPCF8574DInClass() {
}

uint8_t ICACHE_FLASH_ATTR HoCoPCF8574DInClass::Get() {
	uint8_t r;
	_PCF8574Device->Read(_Channel, &r);
	if (_Inverted)
		r = 1 - r;
	return r;
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::SendStatus() {
	char jt[3];
	ets_sprintf(jt, "%d", _DebounceState);
	Publish((char*)"triggered", (char*)jt, true);
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::SetConfig(char *Config) {
	DEBUG("HoCoPCF8574DInClass::SetConfig %s", Config);
	char *PCF8574Name = CppJson::jsonGetString(Config, "PCF8574");
	_PCF8574Device = (HoCoPCF8574Class*)HoCo::DeviceByName(PCF8574Name);
	delete PCF8574Name;
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
	_Channel = CppJson::jsonGetInt(Config, "pin");
	_Sample = CppJson::jsonGetInt(Config, "sample");
	_Trigger = CppJson::jsonGetChar(Config, "trigger");
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::Start() {
	DEBUG("HoCoPCF8574DInClass::Start");
	ets_timer_disarm(&LoopTimer);
	if (_Sample > 0) {
		ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
		ets_timer_arm_new(&LoopTimer, _Sample, 1, 0);
	}
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::Stop() {
	DEBUG("HoCoPCF8574DInClass::Stop");
	ets_timer_disarm(&LoopTimer);
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8574DInClass::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::TimerLoop(void *data) {
	HoCoPCF8574DInClass* h = (HoCoPCF8574DInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoPCF8574DInClass::_TimerLoop() {
   	uint8_t currentState = Get();
   	if (_DebounceState != currentState) {
   		_DebounceState = currentState;
   		if (_Trigger == 'a')
   			SendStatus();
   		else if (_Trigger == 'r' && _DebounceState)
  				SendStatus();
   		else if (_Trigger == 'f' && !_DebounceState)
  				SendStatus();
   	}
}
