#include <string.h>

#include "h_util.h"

void placeString(char* pDest, char* pSrc, size_t pLen)
{
	size_t tLen = strlen(pSrc);
	if (tLen < pLen)
		strcpy(pDest, pSrc);
	else {
		memcpy(pDest, pSrc, pLen);
		pDest[pLen] = EOS;
	}
}

//================================================================================

char* rtrim(char *string)
{
	if (string == 0)
		return 0;

	char* p = string + strlen(string) - 1;

	while (p >= string) {
		if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
			*p-- = 0;
		else
			break;
	}
	return string;
}

char* ltrim(char* string)
{
	if (string == 0)
		return 0;

	char* p;

	for (p = string; *p && (*p == ' ' || *p == '\t' || *p == '\n'); p++)
		*p = 0;

	return p;
}

char* trim(char* string)
{
	return ltrim(rtrim(string));
}

char* strdel(char* string, size_t len)
{
	size_t slen = strlen(string);
	if (len > slen)
		len = slen;

	char* tempstr = newStr(string + len);
	char* p = strcpy(string, tempstr);
	delete tempstr;
	return p;
}

//================================================================================

char* rightJust(char *string, size_t l)
{
	size_t p = strlen(string) - 1;
	char* s = &(string[p]);

	while (*s == ' ' && p >= 0) { s--; p--; }
	p++;

	string[l] = 0;
	memmove(string + l - p, string, p);
	memset(string, ' ', l - p);

	return string;
}

char* leftJust(char *string, size_t l)
{
	char* s = string;
	size_t p = 0;

	while (*s == ' ' && p < l) { s++; p++; }

	strncpy(string, s, l - p + 1);

	return string;
}

const size_t maxCenterStrLen = 256;

char* centerStr(char* dst, const char* src, size_t len)
{
	size_t l = strlen(src);
	if (l > maxCenterStrLen)
		return strcpy(dst, src);

	char str[maxCenterStrLen + 1];
	char* tmpPtr = trim(strcpy(str, src));
	size_t sLen = strlen(tmpPtr);

	if (sLen < len && len != 0) {
		memset(dst, ' ', len);
		memcpy(dst + (len - sLen) / 2, tmpPtr, sLen);
	}
	else memcpy(dst, tmpPtr, sLen);

	dst[len] = 0;
	return dst;
}
