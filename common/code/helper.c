#include <helper.h>
#include <math.h>

int ICACHE_FLASH_ATTR ltoa(long val, char* buffer, int base) {
	int i = 34;
	int p = 0;
	char buf[36] = {0};
	bool ngt = val < 0;
	if (ngt)
		val = -val;
	for (; val && i; --i, p++, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	if (p == 0)
		buf[i--] = '0'; // case for zero
	if (ngt)
		buf[i--] = '-';
	ets_strcpy(buffer, &buf[i+1]);
	return ets_strlen(buffer);
}

// Author zitron: http://forum.arduino.cc/index.php?topic=37391#msg276209
// removed dependncy to string.h
char ICACHE_FLASH_ATTR *dtostrf(double floatVar, int numDigitsAfterDecimal, char *outputBuffer) {
	if (outputBuffer == NULL)
		return NULL;
	*outputBuffer = 0;
	if (isnan(floatVar))
		ets_strcpy(outputBuffer, "NaN");
	else if (isinf(floatVar))
		ets_strcpy(outputBuffer, "Inf");
	else if (floatVar > 4294967040.0)
		ets_strcpy(outputBuffer, "OVF");  // constant determined empirically
	else if (floatVar <-4294967040.0)
		ets_strcpy(outputBuffer, "OVF");  // constant determined empirically
	if (*outputBuffer != 0)
		return outputBuffer;
	char temp[24];
	char *p = outputBuffer;
	if (floatVar < 0.0) {
		p[0] = '-';
		p++;
		floatVar *= -1;
	}
	if (numDigitsAfterDecimal == 0) {
		p += ltoa(round(floatVar), p, 10);
	} else {
		unsigned long frac;
		unsigned long mult = 1;
		int16_t  processedFracLen = numDigitsAfterDecimal;
		if (processedFracLen > 9)
			processedFracLen = 9; // Prevent overflow!
		int16_t padding = processedFracLen - 1;
		int16_t k = processedFracLen;
		while (k-- > 0)
			mult *= 10;
		floatVar += 0.5 / (float)mult;      // compute rounding factor
		p += ltoa(floor(floatVar), p, 10);  //prints the integer part without rounding
		p[0] = '.';
		p++;
		frac = (floatVar - floor(floatVar)) * mult;
		unsigned long frac1 = frac;
		while (frac1 /= 10)
			padding--;
		while (padding--) {
			p[0] = '0';
			p++;
		}
		p += ltoa(frac, p, 10);  // print fraction part
		k = numDigitsAfterDecimal - processedFracLen;
		while (k-- > 0) {
			p[0] = '0';
			p++;
		}
	}
	// generate width space padding
//	if ((minStringWidthIncDecimalPoint != 0) && (minStringWidthIncDecimalPoint >= p - outputBuffer)) {
//		int16_t J = minStringWidthIncDecimalPoint - (p - outputBuffer);
//		for (i = 0; i < J; i++) {
//			temp[i] = ' ';
//		}
//		temp[i++] = '\0';
//		strcat(temp, outputBuffer);
//		ets_strcpy(outputBuffer, temp);
//	}
	p[0] = '\0';
	return outputBuffer;
}

// Source:
// https://github.com/anakod/Sming/blob/master/Sming/system/stringconversion.cpp#L93
double ICACHE_FLASH_ATTR myatof(const char* s) {
	double result = 0;
	double factor = 1.0;
	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
		++s;
	if (*s == 0)
		return 0;
	if (*s == '-') {
		factor = -1.0;
		++s;
	}
	if (*s == '+')
		++s;
	bool decimals = false;
	char c;
	while (c = *s) {
		if (c == '.')
		{
			decimals = true;
			++s;
			continue;
		}
		int d = c - '0';
		if (d < 0 || d > 9)
			break;
		result = 10.0 * result + d;
		if (decimals)
			factor *= 0.1;
		++s;
	}
	return result * factor;
}
