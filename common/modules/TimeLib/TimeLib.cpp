/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/

//#if ARDUINO >= 100
//#include <Arduino.h>
//#else
//#include <WProgram.h>
//#endif

#include <TimeLib.h>

static int32_t localOffset = 0;

static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds

void refreshCache(time_t t) {
  if (t != cacheTime) {
    breakTime(t, tm);
    cacheTime = t;
  }
}

int hour() { // the hour now
  return hour(now());
}

int ICACHE_FLASH_ATTR hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;
}

int ICACHE_FLASH_ATTR hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now());
}

int ICACHE_FLASH_ATTR hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t ICACHE_FLASH_ATTR isAM() { // returns true if time now is AM
  return !isPM(now());
}

uint8_t ICACHE_FLASH_ATTR isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);
}

uint8_t ICACHE_FLASH_ATTR isPM() { // returns true if PM
  return isPM(now());
}

uint8_t ICACHE_FLASH_ATTR isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12);
}

int ICACHE_FLASH_ATTR minute() {
  return minute(now());
}

int ICACHE_FLASH_ATTR minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;
}

int ICACHE_FLASH_ATTR second() {
  return second(now());
}

int ICACHE_FLASH_ATTR second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int ICACHE_FLASH_ATTR day(){
  return(day(now()));
}

int ICACHE_FLASH_ATTR day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int ICACHE_FLASH_ATTR weekday() {   // Sunday is day 1
  return  weekday(now());
}

int ICACHE_FLASH_ATTR weekday(time_t t) {
  refreshCache(t);
  return tm.Wday;
}

int ICACHE_FLASH_ATTR month(){
  return month(now());
}

int ICACHE_FLASH_ATTR month(time_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int ICACHE_FLASH_ATTR year() {  // as in Processing, the full four digit year: (2009, 2010 etc)
  return year(now());
}

int ICACHE_FLASH_ATTR year(time_t t) { // the year for the given time
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void ICACHE_FLASH_ATTR breakTime(time_t timeInput, tmElements_t &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput + localOffset;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

/*=====================================================*/
/* Low level system time functions  */

static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync
#endif


time_t ICACHE_FLASH_ATTR now() {
	// calculate number of seconds passed since last call to now()
  while (mymillis() - prevMillis >= 1000) {
		// millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    sysTime++;
    prevMillis += 1000;
#ifdef TIME_DRIFT_INFO
    sysUnsyncedTime++; // this can be compared to the synced time to measure long term drift
#endif
  }
  if (nextSyncTime <= sysTime) {
    if (getTimePtr != 0) {
      time_t t = getTimePtr();
      if (t != 0) {
        setTime(t);
      } else {
        nextSyncTime = sysTime + syncInterval;
        Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
      }
    }
  }
  return (time_t)sysTime;
}

void ICACHE_FLASH_ATTR setTime(time_t t) {
#ifdef TIME_DRIFT_INFO
 if(sysUnsyncedTime == 0)
   sysUnsyncedTime = t;   // store the time of the first call to set a valid Time
#endif

  sysTime = (uint32_t)t;
  nextSyncTime = (uint32_t)t + syncInterval;
  Status = timeSet;
  prevMillis = mymillis();  // restart counting from now (thanks to Korman for this fix)
}

void ICACHE_FLASH_ATTR setLocalOffset(int32 o) {
	if (localOffset != o) {
		localOffset = o;
		refreshCache(now());
	}
}

int32 ICACHE_FLASH_ATTR getLocalOffset() {
	return localOffset;
}

void ICACHE_FLASH_ATTR adjustTime(long adjustment) {
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t ICACHE_FLASH_ATTR timeStatus() {
  now(); // required to actually update the status
  return Status;
}

void ICACHE_FLASH_ATTR setSyncProvider( getExternalTime getTimeFunction){
  getTimePtr = getTimeFunction;
  nextSyncTime = sysTime;
  now(); // this will sync the clock
}

void ICACHE_FLASH_ATTR setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = sysTime + syncInterval;
}
