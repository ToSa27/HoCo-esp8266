#include <HoCoScheduler.h>
#include <debug.h>
#include <HoCoBase.h>
#include <TimeLib.h>
#include <CppJson.h>

extern "C" {
#include <user_interface.h>
}

bool HoCoScheduler::IsInitialized = false;
ETSTimer HoCoScheduler::CheckEventsTimer;
//publish_callback HoCoScheduler::PublishCb = NULL;
/*
uint32_t HoCoScheduler::NextHoliday[MAX_HOLIDAY];
uint32_t HoCoScheduler::NextVacationStart = 0;
uint32_t HoCoScheduler::NextVacationEnd = 0;
*/
EVENTS HoCoScheduler::events;
uint32_t HoCoScheduler::LastExecution[EVT_MAX_EVENTS];

void ICACHE_FLASH_ATTR HoCoScheduler::Init() {
	LoadEvents(false);
	for (uint8_t i = 0; i < EVT_MAX_EVENTS; i++)
		LastExecution[i] = 0;
	setSyncInterval(1000);
	ets_timer_disarm(&CheckEventsTimer);
	ets_timer_setfn(&CheckEventsTimer, (ETSTimerFunc *)CheckEvents, NULL);
	ets_timer_arm_new(&CheckEventsTimer, 15000, 1, 0);
}

void ICACHE_FLASH_ATTR HoCoScheduler::UpdateDates(char* data) {
	DEBUG("HoCoScheduler::UpdateDates");
	uint8_t i = 0;
	char *h = CppJson::jsonGet(data, "h");
	if (h != NULL) {
		char *hi;
		char *pos = CppJson::jsonGetArrayFirst(h, hi);
		while (hi != NULL) {
			events.NextHoliday[i] = atoi(hi);
			i++;
			delete(hi);
			if (i == EVT_MAX_HOLIDAY)
				break;
			pos = CppJson::jsonGetArrayNext(pos, hi);
		}
		while (i < EVT_MAX_HOLIDAY) {
			events.NextHoliday[i] = 0;
			i++;
		}
	}
	char *v = CppJson::jsonGet(data, "v");
	if (v != NULL) {
		events.NextVacationStart = CppJson::jsonGetTime(v, "f");
		events.NextVacationEnd = CppJson::jsonGetTime(v, "t");
	}
}

/*
void ICACHE_FLASH_ATTR HoCoScheduler::UpdateTime(char* data) {
	time_t t = jsonGetTime(data, "n");
	setTime(t);
	debugf("HoCoScheduler::UpdateTime - %d", t);
	uint8_t i = 0;
	char *h = jsonGet(data, "h");
	if (h != NULL) {
		char *hi;
		char *pos = jsonGetArrayFirst(h, hi);
		while (hi != NULL) {
			NextHoliday[i] = atoi(hi);
			i++;
			delete(hi);
			if (i == MAX_HOLIDAY)
				break;
			pos = jsonGetArrayNext(pos, hi);
		}
		while (i < MAX_HOLIDAY) {
			NextHoliday[i] = 0;
			i++;
		}
	}
	char *v = jsonGet(data, "v");
	if (v != NULL) {
		NextVacationStart = jsonGetTime(v, "f");
		NextVacationEnd = jsonGetTime(v, "t");
	}
}

void ICACHE_FLASH_ATTR HoCoScheduler::PrintEvent(uint8_t index) {
	EVENT e = events.event[index];
	debugf("Schedule Event %d", index);
	debugf("  Enabled:     %d", e.Enabled);
	debugf("  MinuteOfDay: %d", e.MinuteOfDay);
	debugf("  WeekdayMask: %d", e.WeekdayMask);
	debugf("  Holiday:     %d", e.Holiday);
	debugf("  Vacation:    %d", e.Vacation);
	debugf("  Random:      %d", e.Random);
	debugf("  Action: %s", e.Action);
}
*/

void ICACHE_FLASH_ATTR HoCoScheduler::UpdateSchedule(uint8_t index, char *s) {
	DEBUG("HoCoScheduler::UpdateSchedule");
	// schedule json syntax:
	// {"en":...,"mod":...,"wd":...,"h":...,"v":...,"r":...,"a":...}
	EVENT e = events.event[index];
	if (CppJson::jsonHas(s, "en"))
		events.event[index].Enabled = CppJson::jsonGetInt(s, "en");
	if (CppJson::jsonHas(s, "int"))
		events.event[index].Interval = CppJson::jsonGetInt(s, "int");
	if (CppJson::jsonHas(s, "mod"))
		events.event[index].MinuteOfDay = CppJson::jsonGetInt(s, "mod");
	if (CppJson::jsonHas(s, "wd"))
		events.event[index].WeekdayMask = CppJson::jsonGetInt(s, "wd");
	if (CppJson::jsonHas(s, "h"))
		events.event[index].Holiday = CppJson::jsonGetInt(s, "h");
	if (CppJson::jsonHas(s, "v"))
		events.event[index].Vacation = CppJson::jsonGetInt(s, "v");
	if (CppJson::jsonHas(s, "r"))
		events.event[index].Random = CppJson::jsonGetInt(s, "r");
	if (CppJson::jsonHas(s, "a")) {
		char *a = CppJson::jsonGetString(s, "a");
		ets_memcpy(events.event[index].Action, a, EVT_MAX_ACTION_LEN);
		for (uint8_t i = 0; i < EVT_MAX_ACTION_LEN; i++)
			if (events.event[index].Action[i] == '\'')
				events.event[index].Action[i] = '\"';
	}
//	PrintEvent(index);
	SaveEvents();
	PublishEvent(index);
}

void ICACHE_FLASH_ATTR HoCoScheduler::LoadEvents(bool reset) {
	INFO("Loading event config.");
	ets_memset(&events, 0, sizeof(events));
	system_param_load(EVT_CONFIG_SECTOR, 0, (uint32*)&events, sizeof(events));
	if(events.magic != EVT_CONFIG_MAGIC || reset) {
		WARN("Initializing event config.");
		ets_memset(&events, 0, sizeof(events));
		// ToDo
		SaveEvents();
	}
}

void ICACHE_FLASH_ATTR HoCoScheduler::SaveEvents() {
	INFO("Saving event config.");
	events.magic = EVT_CONFIG_MAGIC;
	system_param_save_with_protect(EVT_CONFIG_SECTOR, (uint32*)&events, sizeof(events));
}

void ICACHE_FLASH_ATTR HoCoScheduler::CheckEvents(void *data) {
	DEBUG("HoCoScheduler::CheckEvents");
	if (timeStatus() != timeNotSet) {
		time_t t = now();
		uint16_t mod = minute(t) + (hour(t) * 60);
		uint8_t wd = weekday(t);
		DEBUG("  t: %d / mod: %d / wd: %d", t, mod, wd);
		bool IsHoliday = false;
		for (uint8_t i = 0; i < EVT_MAX_HOLIDAY; i++)
			if ((t > events.NextHoliday[i]) && (t < events.NextHoliday[i] + SECS_PER_DAY))
				IsHoliday = true;
		bool IsVacation = ((t > events.NextVacationStart) && (t < events.NextVacationEnd + SECS_PER_DAY));
		for (uint8_t i = 0; i < EVT_MAX_EVENTS; i++) {
			EVENT e = events.event[i];
			if (e.Enabled > 0) {
				if (e.Interval > 0) {
					if (LastExecution[i] <= t - (e.MinuteOfDay * 60)) {
						ExecuteEvent(e.Action);
						LastExecution[i] = t;
					}
				} else {
					if ((mod >= e.MinuteOfDay) && (mod < e.MinuteOfDay + 30))
					{
						if (LastExecution[i] < t - SECS_PER_HOUR)
						{
							bool exec = false;
							if (IsHoliday && e.Holiday)
								exec = true;
							else if (IsVacation && e.Vacation)
								exec = true;
							else if ((e.WeekdayMask & (1 << wd)) > 0)
								exec = true;
							if (exec) {
								ExecuteEvent(e.Action);
								LastExecution[i] = t;
							}
						}
					}
				}
			}
		}
	}
}

void ICACHE_FLASH_ATTR HoCoScheduler::ExecuteEvent(char *Action) {
	char *s = Action;
	while (s[0] != '|')
		s++;
	uint8_t l = s - Action;
	char *topic = new char[l + 1];
	ets_memcpy(topic, Action, l);
	topic[l] = '\0';
	s++;
	l = ets_strlen(Action) - (s - Action);
	char *data = new char[l + 1];
	ets_memcpy(data, s, l);
	data[l] = '\0';
	HoCo::HandleMessage(topic, data);
	delete data;
	delete topic;
}

void ICACHE_FLASH_ATTR HoCoScheduler::PublishEvents() {
	for (uint8_t i = 0; i < EVT_MAX_EVENTS; i++)
		PublishEvent(i);
}

void ICACHE_FLASH_ATTR HoCoScheduler::PublishEvent(uint8_t index) {
	EVENT e = events.event[index];
	if (e.Enabled > 0) {
		char t[12];
		ets_sprintf(t, "$event/%d", index);
		char a[EVT_MAX_ACTION_LEN];
		ets_memcpy(a, e.Action, EVT_MAX_ACTION_LEN);
		for (uint8_t i = 0; i < EVT_MAX_ACTION_LEN; i++)
			if (a[i] == '\"')
				a[i] = '\'';
		char jt[80 + EVT_MAX_ACTION_LEN];
		ets_sprintf(jt, "{\"en\":%d,\"mod\":%d,\"wd\":%d,\"h\":%d,\"v\":%d,\"r\":%d,\"a\":\"%s\"}", e.Enabled, e.MinuteOfDay, e.WeekdayMask, e.Holiday, e.Vacation, e.Random, a);
		mqtt_publish(t,jt, true);
//		if (PublishCb)
//			PublishCb(t, jt, true);
	}
}
