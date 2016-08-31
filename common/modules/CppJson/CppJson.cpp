#include <CppJson.h>
#include <debug.h>

char *CppJson::jsonGetInternal(char *json, const char *name, bool trim) {
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

bool CppJson::jsonHas(char *json, const char *name) {
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

char *CppJson::jsonGet(char *json, const char *name) {
	return jsonGetInternal(json, name, false);
}

char *CppJson::jsonGetString(char *json, const char *name) {
	return jsonGetInternal(json, name, true);
}

int CppJson::jsonGetInt(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, false);
	int i = atoi(v);
	delete(v);
	return i;
}

uint32_t CppJson::jsonGetTime(char *json, const char *name) {
	char *v = jsonGetInternal(json, name, false);
	uint32_t t = atoi(v);
	delete(v);
	return t;
}

char *CppJson::jsonGetArrayInternal(char *json, char front, char*& item) {
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

char *CppJson::jsonGetArrayFirst(char *json, char*& item) {
	return jsonGetArrayInternal(json, '[', item);
}

char *CppJson::jsonGetArrayNext(char *json, char*& item) {
	return jsonGetArrayInternal(json, ',', item);
}
