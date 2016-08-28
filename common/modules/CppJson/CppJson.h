#pragma once

#include <esp8266.h>

class CppJson {
private:
	static char *jsonGetInternal(char *json, const char *name, bool trim);
	static char *jsonGetArrayInternal(char *json, char front, char*& item);
public:
	static char *jsonGet(char *json, const char *name);
	static char *jsonGetString(char *json, const char *name);
	static int jsonGetInt(char *json, const char *name);
	static uint32_t jsonGetTime(char *json, const char *name);
	static char *jsonGetArrayFirst(char *json, char*& item);
	static char *jsonGetArrayNext(char *json, char*& item);
};
