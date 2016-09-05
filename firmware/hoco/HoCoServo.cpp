#include <HoCoServo.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>

HoCoServoClass::HoCoServoClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
	Subscribe((char*)"pos/$set");
}

HoCoServoClass::~HoCoServoClass() {
}

void ICACHE_FLASH_ATTR HoCoServoClass::Set(uint8_t Value) {
	DEBUG("HoCoServoClass::Set %d", Value);
	if (Value < 2) {
		digitalWrite(_Pin, _Inverted ? 1 - Value : Value);
		SendStatus();
	}
}

void ICACHE_FLASH_ATTR HoCoServoClass::SendStatus() {
	char jt[10];
	ets_sprintf(jt, "%d", Get());
	Publish((char*)"on", jt, true);
}

void ICACHE_FLASH_ATTR HoCoServoClass::SetConfig(char *Config) {
	DEBUG("HoCoServoClass::SetConfig %s", Config);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	digitalWrite(_Pin, 0);
	pinMode(_Pin, OUTPUT);
}

void ICACHE_FLASH_ATTR HoCoServoClass::Start() {
	DEBUG("HoCoServoClass::Start");
}

void ICACHE_FLASH_ATTR HoCoServoClass::Stop() {
	DEBUG("HoCoServoClass::Stop");
}

void ICACHE_FLASH_ATTR HoCoServoClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoServoClass::HandlePropertyMessage");
	if (ets_strstr(Topic, "pos/$set"))
		Set(atoi(Data));
}
