#include <HoCoDOut.h>

#include <CppJson.h>
#include <debug.h>
#include <pin.h>

HoCoDOutClass::HoCoDOutClass(char *Name, char *Config, publish_callback publish) : HoCoDeviceClass(Name, publish) {
	SetConfig(Config);
}

HoCoDOutClass::~HoCoDOutClass() {
}

uint8_t ICACHE_FLASH_ATTR HoCoDOutClass::Get() {
	uint8_t r = digitalRead(_Pin);
	if (_Inverted)
		r = 1 - r;
	return r;
}

void ICACHE_FLASH_ATTR HoCoDOutClass::Set(uint8_t Value) {
	DEBUG("HoCoDOutClass::Set %d", Value);
	if (Value < 2) {
		digitalWrite(_Pin, _Inverted ? 1 - Value : Value);
		SendStatus();
	}
}

void ICACHE_FLASH_ATTR HoCoDOutClass::SetStatus(char *Status) {
	DEBUG("HoCoDOutClass::SetStatus %s", Status);
	uint8_t v = CppJson::jsonGetInt(Status, "out");
	Set(v);
}

void ICACHE_FLASH_ATTR HoCoDOutClass::SendStatus() {
	char jt[10];
	ets_sprintf(jt, "{\"out\":%d}", Get());
	OnPublish((char*)"status", jt);
}

char ICACHE_FLASH_ATTR *HoCoDOutClass::DeviceType() {
	return (char *)"DOut";
}

void ICACHE_FLASH_ATTR HoCoDOutClass::SetConfig(char *Config) {
	DEBUG("HoCoDOutClass::SetConfig %s", Config);
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	digitalWrite(_Pin, _Inverted ? 1 : 0);
	pinMode(_Pin, OUTPUT);
}

void ICACHE_FLASH_ATTR HoCoDOutClass::SendConfig() {
	char jt[50];
	ets_sprintf(jt, "{\"pin\":%d,\"inv\":%d}", _Pin, _Inverted);
	OnPublish((char*)"status", jt);
}

void ICACHE_FLASH_ATTR HoCoDOutClass::Start() {
}

void ICACHE_FLASH_ATTR HoCoDOutClass::Stop() {
}

void ICACHE_FLASH_ATTR HoCoDOutClass::ReceivedSub(char *Topic, char *Data) {
	DEBUG("HoCoDOutClass::ReceivedSub");
}
