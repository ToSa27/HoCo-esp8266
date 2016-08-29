#pragma once

#include <esp8266.h>
#include <HoCo.h>

class HoCoDeviceClass {
private:
	subscribe_callback _SubscribeCallback;
	publish_callback _PublishCallback;
protected:
	void Subscribe(char *Topic);
	void Publish(char *Topic, char *Data, bool Retain);
//	virtual void SetStatus(char *Status) = 0;
	virtual void SetConfig(char *Config) = 0;
//	virtual void SendConfig() = 0;
//	virtual void ReceivedSub(char *Topic, char *Data) = 0;
	virtual void HandlePropertyMessage(char *Topic, char *Data) = 0;
public:
	HoCoDeviceClass(char *Name, subscribe_callback subscribe, publish_callback publish);
	virtual ~HoCoDeviceClass();
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void SendStatus() = 0;
//	virtual char *DeviceType() = 0;
	char *DeviceId;
	void HandleDeviceMessage(char *Topic, char *Data);
};
