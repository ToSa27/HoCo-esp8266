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
//	static subscribe_callback SubscribeCb;
//	static publish_callback PublishCb;
protected:
//	static void PublishStatus();
public:
	~HoCo();
	static void Init(char *config);
	static void SetConnected(bool connected);
	static void HandleBroadcastMessage(char *subtopic, char *data);
	static void HandleMessage(char *subtopic, char *data);
	static void Start();
	static void Stop();
//	static void Publish(char *deviceid, char* topic, char *data);
//	static void Execute(char *fn, char *param);
	static HoCoDeviceClass* DeviceByName(char *name);
};
