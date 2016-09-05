#include <HoCoDevice.h>

#include <debug.h>

HoCoDeviceClass::HoCoDeviceClass(char *Name) {
	DeviceId = new char[ets_strlen(Name) + 1];
	ets_memcpy(DeviceId, Name, ets_strlen(Name));
	DeviceId[ets_strlen(Name)] = '\0';
	Subscribe((char*)"$config/$set");
}

HoCoDeviceClass::~HoCoDeviceClass() {
	delete(DeviceId);
}

void ICACHE_FLASH_ATTR HoCoDeviceClass::HandleDeviceMessage(char *Topic, char *Data) {
	DEBUG("HoCoDOutClass::HandleDeviceMessage");
	if (ets_strstr(Topic, "$config/$set"))
		SetConfig(Data);
	else
		HandlePropertyMessage(Topic, Data);
}

void ICACHE_FLASH_ATTR HoCoDeviceClass::Subscribe(char *Topic) {
	char *t = new char[ets_strlen(DeviceId) + ets_strlen(Topic) + 2];
	ets_sprintf(t, "%s/%s", DeviceId, Topic);
	mqtt_subscribe(t);
}

void ICACHE_FLASH_ATTR HoCoDeviceClass::Publish(char *Topic, char *Data, bool Retain) {
	char *t = new char[ets_strlen(DeviceId) + ets_strlen(Topic) + 2];
	ets_sprintf(t, "%s/%s", DeviceId, Topic);
	mqtt_publish(t, Data, Retain);
}
