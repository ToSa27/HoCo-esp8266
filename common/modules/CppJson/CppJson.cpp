#include <CppJson.h>
#include <debug.h>
#include <helper.h>

char ICACHE_FLASH_ATTR *CppJson::jsonGetInternal(char *json, const char *name, bool trim) {
	if (json[0] != '{')
		return (char*)"";
	char *cp = json + 1;
	uint8_t bl = 1;
	while (bl > 0) {
		if (cp[0] == '{')
			bl++;
		else if (cp[0] == '}')
			bl--;
		else if (bl == 1) {
			if ((cp[-1] == '\"') && (ets_strstr(cp, name) == cp) && (cp[ets_strlen(name)] == '\"')) {
				cp = cp + ets_strlen(name) + 1;
				while (cp[0] != ':')
					cp++;
				cp++;
				while (cp[0] == ' ')
					cp++;
				char *sp = cp;
				char *ep;
				if (sp[0] == '{') {
					uint8_t vl = 1;
					while (vl > 0) {
						cp++;
						if (cp[0] == '{')
							vl++;
						else if (cp[0] == '}')
							vl--;
					}
					ep = cp;
				} else if (sp[0] == '[') {
					uint8_t vl = 1;
					while (vl > 0) {
						cp++;
						if (cp[0] == '[')
							vl++;
						else if (cp[0] == ']')
							vl--;
					}
					ep = cp;
				} else if (sp[0] == '\"') {
					cp++;
					while (cp[0] != '\"')
						cp++;
					ep = cp;
				} else {
					uint8_t vl = 1;
					while (vl > 0) {
						if ((cp[0] == ',') || (cp[0] == '}'))
							vl--;
						cp++;
					}
					ep = cp;
				}
				char *v;
				if (trim) {
					v = new char[ep - sp];
					ets_memcpy(v, sp + 1, ep - sp - 1);
					v[ep - sp - 1] = '\0';
				} else {
					v = new char[ep - sp + 2];
					ets_memcpy(v, sp, ep - sp + 1);
					v[ep - sp + 1] = '\0';
				}
				return v;
			}
		}
		cp++;
	}
	return (char*)"";
}

bool ICACHE_FLASH_ATTR CppJson::jsonHas(char *json, const char *name) {
	if (json[0] != '{')
		return false;
	char *cp = json + 1;
	uint8_t bl = 1;
	while (bl > 0) {
		if (cp[0] == '{')
			bl++;
		else if (cp[0] == '}')
			bl--;
		else if (bl == 1) {
			if ((cp[-1] == '\"') && (ets_strstr(cp, name) == cp) && (cp[ets_strlen(name)] == '\"'))
				return true;
		}
		cp++;
	}
	return false;
}

char ICACHE_FLASH_ATTR *CppJson::jsonGet(char *json, const char *name) {
	return jsonGetInternal(json, name, false);
}

char ICACHE_FLASH_ATTR CppJson::jsonGetChar(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, true);
	char c = v[0];
	delete(v);
	return c;
}

char ICACHE_FLASH_ATTR *CppJson::jsonGetString(char *json, const char *name) {
	return jsonGetInternal(json, name, true);
}

int ICACHE_FLASH_ATTR CppJson::jsonGetInt(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, false);
	int i = atoi(v);
	delete(v);
	return i;
}

double ICACHE_FLASH_ATTR CppJson::jsonGetDouble(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, false);
	double d = myatof(v);
	delete(v);
	return d;
}

uint32_t ICACHE_FLASH_ATTR CppJson::jsonGetTime(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, false);
	uint32_t t = atoi(v);
	delete(v);
	return t;
}

char ICACHE_FLASH_ATTR *CppJson::jsonGetArrayInternal(char *json, char front, char*& item) {
	item = NULL;
	char *cp = json;
	if (cp[0] != front)
		return NULL;
	cp++;
	uint8_t level = 0;
	char *ds = cp;
	while (cp < json + ets_strlen(json)) {
		if (cp[0] == '{') {
			level++;
		} else if (cp[0] == '}') {
			level--;
		} else if (level == 0) {
			if ((cp[0] == ',') || (cp[0] == ']')) {
				item = new char[cp - ds + 1];
				ets_memcpy(item, ds, cp - ds);
				item[cp - ds] = '\0';
				return cp;
			}
		}
		cp++;
	}
	return NULL;
}

char ICACHE_FLASH_ATTR *CppJson::jsonGetArrayFirst(char *json, char*& item) {
	return jsonGetArrayInternal(json, '[', item);
}

char ICACHE_FLASH_ATTR *CppJson::jsonGetArrayNext(char *json, char*& item) {
	return jsonGetArrayInternal(json, ',', item);
}
