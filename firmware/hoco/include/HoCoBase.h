#pragma once

#include <esp8266.h>
#include <HoCo.h>
#include <Vector.h>
#include <HoCoDevice.h>

class HoCo
{
private:
	static bool isInitialized;
	static bool isConnected;
	static Vector<HoCoDeviceClass*> Devices;
	static void InitDevice(char *config);
	static subscribe_callback SubscribeCb;
	static publish_callback PublishCb;
protected:
//	static void PublishStatus();
public:
	~HoCo();
	static void Init(char *config, subscribe_callback subscribe, publish_callback publish);
	static void SetConnected(bool connected);
//	static bool HandleMessage(char *topic, char *data);
	static bool HandleNodeMessage(char *subtopic, char *data);
	static bool HandleDeviceMessage(char *subtopic, char *data);
	static void Start();
	static void Stop();
//	static void Publish(char *deviceid, char* topic, char *data);
//	static void Execute(char *fn, char *param);
	static HoCoDeviceClass* DeviceByName(char *name);
};
