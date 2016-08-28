#include <HangDevice.h>
#include <debug.h>

HangDeviceClass::HangDeviceClass(char *Name, publish_callback publish) {
	DeviceId = new char[ets_strlen(Name) + 1];
	ets_memcpy(DeviceId, Name, ets_strlen(Name));
	DeviceId[ets_strlen(Name)] = '\0';
	_PublishCallback = publish;
}

HangDeviceClass::~HangDeviceClass() {
	delete(DeviceId);
}

void ICACHE_FLASH_ATTR HangDeviceClass::OnPublish(char *Topic, char *Data) {
	DEBUG("OnPublish: %s = %s", Topic, Data);
	// ToDo : build (sub)topic based on DeviceId and Topic?
	if (_PublishCallback)
		_PublishCallback(Topic, Data, true);
}

void ICACHE_FLASH_ATTR HangDeviceClass::Received(char *Topic, char *Data) {
	DEBUG("HangDeviceClass::Received");
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
