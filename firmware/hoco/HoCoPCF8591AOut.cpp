#include <HoCoPCF8591AOut.h>

#include <HoCoBase.h>
#include <CppJson.h>
#include <debug.h>
#include <tick.h>
#include <helper.h>

HoCoPCF8591AOutClass::HoCoPCF8591AOutClass(char *Name, char *Config) : HoCoDeviceClass(Name) {
	SetConfig(Config);
	Subscribe((char*)"out/$set");
}

HoCoPCF8591AOutClass::~HoCoPCF8591AOutClass() {
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::SendStatus() {
	DEBUG("HoCoPCF8591AOutClass::SendStatus");
	char d[8];
	dtostrf(_LastValue, 2, d);
	Publish((char*)"out", (char*)d, true);
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::SetConfig(char *Config) {
	DEBUG("HoCoPCF8591AOutClass::SetConfig %s", Config);
	char *PCF8591Name = CppJson::jsonGetString(Config, "PCF8591");
	_PCF8591Device = (HoCoPCF8591Class*)HoCo::DeviceByName(PCF8591Name);
	delete PCF8591Name;
	_Min = CppJson::jsonGetInt(Config, "min");
	_Max = CppJson::jsonGetInt(Config, "max");
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::Start() {
	DEBUG("HoCoPCF8591AOutClass::Start");
	_LastValue = PCF8591_INVALID_VAL;
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::Stop() {
	DEBUG("HoCoPCF8591AOutClass::Stop");
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::HandlePropertyMessage(char *Topic, char *Data) {
	DEBUG("HoCoPCF8591AOutClass::HandlePropertyMessage");
	if (ets_strstr(Topic, "out/$set"))
		Write(myatof(Data));
}

void ICACHE_FLASH_ATTR HoCoPCF8591AOutClass::Write(double val) {
	DEBUG("HoCoPCF8591AOutClass::Write");
	if (_PCF8591Device->Write((uint8)((val - _Min) / (_Max - _Min) * 255))) {
		_LastValue = val;
		SendStatus();
	}
}
