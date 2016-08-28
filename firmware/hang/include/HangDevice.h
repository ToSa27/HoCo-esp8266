#pragma once

#include <esp8266.h>
#include <Hang.h>

class HangDeviceClass {
private:
	publish_callback _PublishCallback;
protected:
	void OnPublish(char *Topic, char *Data);
	virtual void SetStatus(char *Status) = 0;
	virtual void SetConfig(char *Config) = 0;
	virtual void SendConfig() = 0;
	virtual void ReceivedSub(char *Topic, char *Data) = 0;
public:
	HangDeviceClass(char *Name, publish_callback publish);
	virtual ~HangDeviceClass();
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void SendStatus() = 0;
	virtual char *DeviceType() = 0;
	char *DeviceId;
	void Received(char *Topic, char *Data);
};
