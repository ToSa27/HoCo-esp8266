#include <HoCoDevice.h>

#include <debug.h>

HoCoDeviceClass::HoCoDeviceClass(char *Name, publish_callback publish) {
	DeviceId = new char[ets_strlen(Name) + 1];
	ets_memcpy(DeviceId, Name, ets_strlen(Name));
	DeviceId[ets_strlen(Name)] = '\0';
	_PublishCallback = publish;
}

HoCoDeviceClass::~HoCoDeviceClass() {
	delete(DeviceId);
}

void ICACHE_FLASH_ATTR HoCoDeviceClass::OnPublish(char *Topic, char *Data) {
	DEBUG("OnPublish: %s = %s", Topic, Data);
	// ToDo : build (sub)topic based on DeviceId and Topic?
	if (_PublishCallback)
		_PublishCallback(Topic, Data, true);
}

void ICACHE_FLASH_ATTR HoCoDeviceClass::Received(char *Topic, char *Data) {
	DEBUG("HoCoDeviceClass::Received");
	if (ets_strstr(Topic, "get") == Topic)
		SendStatus();
	else if (ets_strstr(Topic, "set") == Topic) {
		SetStatus(Data);
	} else if (ets_strstr(Topic, "getConfig") == Topic)
		SendConfig();
	else if (ets_strstr(Topic, "setConfig") == Topic) {
		SetConfig(Data);
	} else
		ReceivedSub(Topic, Data);
}
