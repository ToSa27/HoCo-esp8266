#include <HoCoPCF8591AIn.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8591AInClass::HoCoPCF8591AInClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
}

HoCoPCF8591AInClass::~HoCoPCF8591AInClass() {
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::SendStatus() {
	DEBUG("HoCoPCF8591AInClass::SendStatus");
	char d[8];
	dtostrf(_LastValue, 2, d);
	Publish((char*)"in", (char*)d, true);
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::SetConfig(char *Config) {
	DEBUG("HoCoPCF8591AInClass::SetConfig %s", Config);
	char *PCF8591Name = CppJson::jsonGetString(Config, "PCF8591");
	_PCF8591Device = (HoCoPCF8591Class*)HoCo::DeviceByName(PCF8591Name);
	delete PCF8591Name;
	_Channel = CppJson::jsonGetInt(Config, "channel");
	_Min = CppJson::jsonGetDouble(Config, "min");
	_Max = CppJson::jsonGetDouble(Config, "max");
	_Interval = CppJson::jsonGetInt(Config, "interval");
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::Start() {
	DEBUG("HoCoPCF8591AInClass::Start");
	_LastValue = PCF8591_INVALID_VAL;
	ets_timer_disarm(&LoopTimer);
	if (_Interval > 0) {
		ets_timer_setfn(&LoopTimer, (ETSTimerFunc *)TimerLoop, (void*)this);
		ets_timer_arm_new(&LoopTimer, _Interval, 1, 0);
	}
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::Stop() {
	DEBUG("HoCoPCF8591AInClass::Stop");
	ets_timer_disarm(&LoopTimer);
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8591AInClass::HandlePropertyMessage");
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::Read() {
	DEBUG("HoCoPCF8591AInClass::Read");
	uint8 val;
	if (_PCF8591Device->Read(_Channel, &val)) {
		_LastValue = (val * (_Max - _Min) / 255.0) + _Min;
		SendStatus();
	}
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::TimerLoop(void *data) {
	HoCoPCF8591AInClass* h = (HoCoPCF8591AInClass*)data;
	h->_TimerLoop();
}

void ICACHE_FLASH_ATTR HoCoPCF8591AInClass::_TimerLoop() {
	Read();
}
