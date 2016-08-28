#include <HangDOut.h>
#include <CppJson.h>
#include <debug.h>
#include <pin.h>

HangDOutClass::HangDOutClass(char *Name, char *Config, publish_callback publish) : HangDeviceClass(Name, publish) {
	SetConfig(Config);
}

HangDOutClass::~HangDOutClass() {
}

uint8_t ICACHE_FLASH_ATTR HangDOutClass::Get() {
	uint8_t r = digitalRead(_Pin);
	if (_Inverted)
		r = 1 - r;
	return r;
}

void ICACHE_FLASH_ATTR HangDOutClass::Set(uint8_t Value) {
	DEBUG("HangDOutClass::Set %d", Value);
	if (Value < 2) {
		digitalWrite(_Pin, _Inverted ? 1 - Value : Value);
		SendStatus();
	}
}

void ICACHE_FLASH_ATTR HangDOutClass::SetStatus(char *Status) {
	DEBUG("HangDOutClass::SetStatus %s", Status);
	uint8_t v = CppJson::jsonGetInt(Status, "out");
	Set(v);
}

void ICACHE_FLASH_ATTR HangDOutClass::SendStatus() {
	char jt[10];
	ets_sprintf(jt, "{\"out\":%d}", Get());
	OnPublish((char*)"status", jt);
}

char ICACHE_FLASH_ATTR *HangDOutClass::DeviceType() {
	return (char *)"DOut";
}

void ICACHE_FLASH_ATTR HangDOutClass::SetConfig(char *Config) {
	DEBUG("HangDOutClass::SetConfig %s", Config);
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
	_Pin = CppJson::jsonGetInt(Config, "pin");
	DEBUG("pin %d", _Pin);
	digitalWrite(_Pin, _Inverted ? 1 : 0);
	pinMode(_Pin, OUTPUT);
}

void ICACHE_FLASH_ATTR HangDOutClass::SendConfig() {
	char jt[50];
	ets_sprintf(jt, "{\"pin\":%d,\"inv\":%d}", _Pin, _Inverted);
	OnPublish((char*)"status", jt);
}

void ICACHE_FLASH_ATTR HangDOutClass::Start() {
}

void ICACHE_FLASH_ATTR HangDOutClass::Stop() {
}

void ICACHE_FLASH_ATTR HangDOutClass::ReceivedSub(char *Topic, char *Data) {
	DEBUG("HangDOutClass::ReceivedSub");
}
