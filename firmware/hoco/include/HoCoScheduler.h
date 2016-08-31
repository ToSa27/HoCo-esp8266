#pragma once

#include <esp8266.h>
#include <HoCo.h>

#define EVT_CONFIG_SECTOR				0x30a
#define EVT_CONFIG_MAGIC				0x2334ae68

#define EVT_MAX_EVENTS					25
#define EVT_MAX_ACTION_LEN				150
#define EVT_MAX_HOLIDAY					2

typedef struct{
	uint16_t MinuteOfDay;
	uint8_t WeekdayMask;
	uint8_t Enabled:1;
	uint8_t Holiday:1;
	uint8_t Vacation:1;
	uint8_t Reserved:3;
	uint8_t Random:2;
	char Action[EVT_MAX_ACTION_LEN];
} EVENT;

typedef struct {
	uint32 magic;
	EVENT event[EVT_MAX_EVENTS];
	uint32_t NextHoliday[EVT_MAX_HOLIDAY];
	uint32_t NextVacationStart;
	uint32_t NextVacationEnd;
} EVENTS;


/*
#include <HoCo_config.h>
#include "Config.h"
#include "Time.h"
#include "HoCo.h"

typedef struct{
	uint16_t MinuteOfDay;
	uint8_t WeekdayMask;
	uint8_t Enabled:1;
	uint8_t Holiday:1;
	uint8_t Vacation:1;
	uint8_t Reserved:3;
	uint8_t Random:2;
	char Action[MAX_ACTION_LEN];
} EVENT;

typedef struct{
	uint32_t evt_holder;
	EVENT event[MAX_EVENTS];
} EVENTS;
*/

class HoCoScheduler
{
public:
	static void Init(publish_callback publish);
//	static void UpdateTime(char* Timestamp);
	static void PublishEvents();
	static void PublishEvent(uint8_t index);
	static void UpdateSchedule(uint8_t index, char *schedule);
private:
	static EVENTS events;
	static void LoadEvents(bool reset);
	static void SaveEvents();
	static void CheckEvents(void *data);
	static void ExecuteEvent(char *Action);
//	static void PrintEvent(uint8_t index);
	static bool IsInitialized;
//	static EVENTS events;
	static ETSTimer CheckEventsTimer;
//	static uint32_t NextHoliday[MAX_HOLIDAY];
//	static uint32_t NextVacationStart;
//	static uint32_t NextVacationEnd;
	static uint32_t LastExecution[EVT_MAX_EVENTS];
	static publish_callback PublishCb;
};

