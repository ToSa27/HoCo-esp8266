#include <HoCoBase.h>

#include <TimeLib.h>
#include <CppJson.h>
#include <debug.h>
#include <HoCoScheduler.h>
#include <HoCoDevice.h>
#include <HoCoDIn.h>
#include <HoCoDOut.h>

bool HoCo::isInitialized = false;
bool HoCo::isConnected = false;
Vector<HoCoDeviceClass*> HoCo::Devices;
//subscribe_callback HoCo::SubscribeCb = NULL;
//publish_callback HoCo::PublishCb = NULL;

HoCo::~HoCo() {
	Devices.clear();
}

// config json format:
// {"n":"...","d":[{...},{...},{...}]}
void ICACHE_FLASH_ATTR HoCo::Init(char *config) {
	DEBUG("HoCo::Init");
	DEBUG("config: %s", config);
//	SubscribeCb = subscribe;
//	PublishCb = publish;
	if (ets_strlen(config) > 0)
	{
		char *d = CppJson::jsonGet(config, "d");
		DEBUG("HoCo::Devices: %s", d);
		char *di;
		char *pos = CppJson::jsonGetArrayFirst(d, di);
		DEBUG("HoCo::FirstDevice: %s", di);
		while (di != NULL) {
			DEBUG("HoCo::Device: %s", di);
			InitDevice(di);
			delete(di);
			pos = CppJson::jsonGetArrayNext(pos, di);
		}
		delete(d);
		DEBUG("DeviceCount: %d", Devices.count());
	//	HangScheduler::Init();
		isInitialized = true;
	}
}

// config json format:
// {"t":"...","n":"...","c":{...}}
void ICACHE_FLASH_ATTR HoCo::InitDevice(char *config) {
	DEBUG("HoCo::InitDevice");
	DEBUG("HoCo::InitDevice-config: %s", config);
	char *t = CppJson::jsonGetString(config, "t");
	char *n = CppJson::jsonGetString(config, "n");
	char *c = CppJson::jsonGet(config, "c");
	DEBUG("HoCo::InitDevice - Type: %s / Name: %s", t, n);
	if (ets_strstr(t, "DOut") == t)
		Devices.add(new HoCoDOutClass(n, c));
//		Devices.add(new HoCoDOutClass(n, c, SubscribeCb, PublishCb));
	else if (ets_strstr(t, "DIn") == t)
		Devices.add(new HoCoDInClass(n, c));
//		Devices.add(new HoCoDInClass(n, c, SubscribeCb, PublishCb));
//	else if (ets_strstr(t, "DS1820") == t)
//		Devices.add(new HangDS1820Class(n, c, Publish));
//	else if (ets_strstr(t, "Oled") == t)
//		Devices.add(new HangOledClass(n, c, Publish));
//	else if (ets_strstr(t, "RgbOut") == t)
//		Devices.add(new HangRgbOutClass(n, c, Publish));
//	else if (ets_strstr(t, "AIn") == t)
//		Devices.add(new HangAInClass(n, c, Publish));
//	else if (ets_strstr(t, "AOut") == t)
//		Devices.add(new HangAOutClass(n, c, Publish));
//	else if (ets_strstr(t, "Fn") == t) {
//		char *tfn = t + 2;
//		if (ets_strstr(tfn, "Watering") == tfn)
//			Devices.add(new HangFnWatering(n, c, Publish));
//	}
	delete(c);
	delete(n);
	delete(t);
}

void ICACHE_FLASH_ATTR HoCo::SetConnected(bool connected) {
	if (connected != isConnected) {
		if (connected)
			Start();
		isConnected = connected;
	}
}

void ICACHE_FLASH_ATTR HoCo::HandleBroadcastMessage(char *subtopic, char *data) {
	DEBUG("HoCo::HandleBroadcastMessage");
	DEBUG("  t: %s", subtopic);
	DEBUG("  d: %s", data);
	char *subtopicBuf = subtopic;
	if (subtopicBuf[0] == '$') {
		subtopicBuf += 1;
		if (ets_strstr(subtopicBuf, "time") == subtopicBuf) {
			char *subdataBuf = data;
			uint32_t t = atoi(subdataBuf);
			subdataBuf = ets_strstr(subdataBuf, "/") + 1;
			int32_t o = atoi(subdataBuf);
			setTimeAndOffset(t, o);
		} else if (ets_strstr(subtopicBuf, "dates") == subtopicBuf) {
			HoCoScheduler::UpdateDates(data);
		}
	}
}

void ICACHE_FLASH_ATTR HoCo::HandleMessage(char *subtopic, char *data) {
	DEBUG("HoCo::HandleMessage");
	DEBUG("  t: %s", subtopic);
	DEBUG("  d: %s", data);
	if (subtopic[0] == '$') {
		subtopic++;
		if (ets_strstr(subtopic, "event/") == subtopic) {
			subtopic += 6;
			if (ets_strstr(subtopic, "/$set") > subtopic && ets_strstr(subtopic, "/$set") < subtopic + 3)
				HoCoScheduler::UpdateSchedule(atoi(subtopic), data);
		}
	} else {
		for (uint8_t i = 0; i < Devices.count(); i++) {
			if (ets_strstr(subtopic, Devices[i]->DeviceId) == subtopic) {
				char *subtopic2 = subtopic + ets_strlen(Devices[i]->DeviceId);
				if (ets_strstr(subtopic2, "/") == subtopic2) {
					subtopic2++;
					Devices[i]->HandleDeviceMessage(subtopic2, data);
				}
			}
		}
	}
}

void ICACHE_FLASH_ATTR HoCo::Start() {
	DEBUG("HoCo::Start");
	if (!isInitialized)
		return;
//	char nt[100];
//	ets_sprintf(nt, "/hang/%s/to/#", NodeId);
//	SubscribeCb(mqttClient, (char*)nt, 0);
	for (uint8_t i = 0; i < Devices.count(); i++)
		Devices[i]->Start();
	HoCoScheduler::Init();
	mqtt_subscribe((char*)"$event/+/$set");
//	PublishStatus();
}

void ICACHE_FLASH_ATTR HoCo::Stop() {
	DEBUG("HoCo::Stop");
	for (uint8_t i = 0; i < Devices.count(); i++)
		Devices[i]->Stop();
}

HoCoDeviceClass* ICACHE_FLASH_ATTR HoCo::DeviceByName(char *name) {
	for (uint8_t i = 0; i < Devices.count(); i++)
		if (ets_strstr(name, Devices[i]->DeviceId) == name)
			return Devices[i];
	return NULL;
}
