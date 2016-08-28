#pragma once

#include <esp8266.h>
#include <Vector.h>
#include <Hang.h>

#include <HangDevice.h>

class Hang
{
private:
	static bool isInitialized;
	static char* NodeId;
	static Vector<HangDeviceClass*> Devices;
////	static HttpFirmwareUpdate* Ota;
////	static bool OtaRunning;
	static void InitDevice(char *config);
	static subscribe_callback SubscribeCb;
	static publish_callback PublishCb;
protected:
////	static void OtaStatus(bool success);
////	static void OtaInit(String message);
//	static void PublishStatus();
public:
	~Hang();
	static void Init(char *config, subscribe_callback subscribe, publish_callback publish);
//	static bool HandleMessage(char *topic, char *data);
//	static bool HandleNodeMessage(char *tp, char *data);
	static bool HandleDeviceMessage(char *tp, char *data);
	static void Start();
	static void Stop();
//	static void Publish(char *deviceid, char* topic, char *data);
//	static void Execute(char *fn, char *param);
	static HangDeviceClass* DeviceByName(char *name);
};
