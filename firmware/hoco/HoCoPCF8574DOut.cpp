#include <HoCoPCF8574DOut.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8574DOutClass::HoCoPCF8574DOutClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
	Subscribe((char*)"on/$set");
}

HoCoPCF8574DOutClass::~HoCoPCF8574DOutClass() {
}

uint8_t ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::Get() {
	uint8_t r;
	_PCF8574Device->Read(_Channel, &r);
	if (_Inverted)
		r = 1 - r;
	return r;
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::Set(uint8_t Value) {
	DEBUG("HoCoDOutClass::Set %d", Value);
	if (Value < 2) {
		_PCF8574Device->Write(_Channel, _Inverted ? 1 - Value : Value);
		SendStatus();
	}
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::SendStatus() {
	char jt[10];
	ets_sprintf(jt, "%d", Get());
	Publish((char*)"on", jt, true);
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::SetConfig(char *Config) {
	DEBUG("HoCoPCF8574DOutClass::SetConfig %s", Config);
	char *PCF8574Name = CppJson::jsonGetString(Config, "PCF8574");
	_PCF8574Device = (HoCoPCF8574Class*)HoCo::DeviceByName(PCF8574Name);
	delete PCF8574Name;
	_Channel = CppJson::jsonGetInt(Config, "channel");
	_Inverted = (CppJson::jsonGetInt(Config, "inv") > 0);
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::Start() {
	DEBUG("HoCoPCF8574DOutClass::Start");
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::Stop() {
	DEBUG("HoCoPCF8574DOutClass::Stop");
}

void ICACHE_FLASH_ATTR HoCoPCF8574DOutClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8574DOutClass::HandlePropertyMessage");
	if (ets_strstr(Topic, "on/$set"))
		if (Data[0] =='t')	// toggle
			Set(1 - Get());
		else
			Set(atoi(Data));
}
