/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"

/////////////////////////////////////////////////////////////////////////////////////////
// constructors and destructor

MimeHeaders::MimeHeaders() :
	mCount(0),
	mAllocCount(0),
	mVals(NULL)
{
}

MimeHeaders::MimeHeaders(unsigned iInitCount) :
	mCount(0)
{
	mAllocCount = iInitCount;
	mVals = (MimeHeader*)mir_alloc(iInitCount * sizeof(MimeHeader));
}

MimeHeaders::~MimeHeaders()
{
	clear();
	mir_free(mVals);
}

void MimeHeaders::clear(void)
{
	for (unsigned i = 0; i < mCount; i++) {
		MimeHeader& H = mVals[i];
		if (H.flags & 1) mir_free((void*)H.name);
		if (H.flags & 2) mir_free((void*)H.value);
	}
	mCount = 0;
}

unsigned MimeHeaders::allocSlot(void)
{
	if (++mCount >= mAllocCount) {
		mAllocCount += 10;
		mVals = (MimeHeader*)mir_realloc(mVals, sizeof(MimeHeader) * mAllocCount);
	}
	return mCount - 1;
}



/////////////////////////////////////////////////////////////////////////////////////////
// add various values

void MimeHeaders::addString(const char* name, const char* szValue, unsigned flags)
{
	if (szValue == NULL) return;

	MimeHeader& H = mVals[allocSlot()];
	H.name = name;
	H.value = szValue;
	H.flags = flags;
}

void MimeHeaders::addLong(const char* name, long lValue, unsigned flags)
{
	MimeHeader& H = mVals[allocSlot()];
	H.name = name;

	char szBuffer[20];
	_ltoa(lValue, szBuffer, 10);
	H.value = mir_strdup(szBuffer);
	H.flags = 2 | flags;
}

void MimeHeaders::addULong(const char* name, unsigned lValue)
{
	MimeHeader& H = mVals[allocSlot()];
	H.name = name;

	char szBuffer[20];
	_ultoa(lValue, szBuffer, 10);
	H.value = mir_strdup(szBuffer);
	H.flags = 2;
}

void MimeHeaders::addBool(const char* name, bool lValue)
{
	MimeHeader& H = mVals[allocSlot()];
	H.name = name;
	H.value = lValue ? "true" : "false";
	H.flags = 0;
}

char* MimeHeaders::flipStr(const char* src, size_t len, char* dest)
{
	if (len == -1) len = mir_strlen(src);

	if (src == dest) {
		const unsigned b = (unsigned)len-- / 2;
		for (unsigned i = 0; i < b; i++) {
			const char c = dest[i];
			dest[i] = dest[len - i];
			dest[len - i] = c;
		}
		++len;
	}
	else {
		for (unsigned i = 0; i < len; i++)
			dest[i] = src[len - 1 - i];
		dest[len] = 0;
	}

	return dest + len;
}

/////////////////////////////////////////////////////////////////////////////////////////
// write all values to a buffer

size_t MimeHeaders::getLength(void)
{
	size_t iResult = 0;
	for (unsigned i = 0; i < mCount; i++) {
		MimeHeader& H = mVals[i];
		iResult += mir_strlen(H.name) + mir_strlen(H.value) + 4;
	}

	return iResult + (iResult ? 2 : 0);
}

char* MimeHeaders::writeToBuffer(char* dest)
{
	for (unsigned i = 0; i < mCount; i++) {
		MimeHeader& H = mVals[i];
		if (H.flags & 4) {
			dest = flipStr(H.name, -1, dest);

			*(dest++) = ':';
			*(dest++) = ' ';

			dest = flipStr(H.value, -1, dest);

			*(dest++) = '\r';
			*(dest++) = '\n';
			*dest = 0;
		}
		else
			dest += sprintf(dest, "%s: %s\r\n", H.name, H.value); //!!!!!!!!!!!!
	}

	if (mCount) {
		*(dest++) = '\r';
		*(dest++) = '\n';
		*dest = 0;
	}

	return dest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// read set of values from buffer

char* MimeHeaders::readFromBuffer(char* src)
{
	clear();

	while (*src) {
		char* peol = strchr(src, '\n');

		if (peol == NULL)
			return strchr(src, 0);
		else if (peol == src)
			return src + 1;
		else if (peol == (src + 1) && *src == '\r')
			return src + 2;

		*peol = 0;

		char* delim = strchr(src, ':');
		if (delim) {
			*delim = 0;

			MimeHeader& H = mVals[allocSlot()];

			H.name = lrtrimp(src);
			H.value = lrtrimp(delim + 1);
			H.flags = 0;
		}

		src = peol + 1;
	}

	return src;
}

const char* MimeHeaders::find(const char* szFieldName)
{
	size_t i;
	for (i = 0; i < mCount; i++) {
		MimeHeader& MH = mVals[i];
		if (_stricmp(MH.name, szFieldName) == 0)
			return MH.value;
	}

	const size_t len = mir_strlen(szFieldName);
	char* szFieldNameR = (char*)alloca(len + 1);
	flipStr(szFieldName, len, szFieldNameR);

	for (i = 0; i < mCount; i++) {
		MimeHeader& MH = mVals[i];
		if (_stricmp(MH.name, szFieldNameR) == 0 && (MH.flags & 3) == 0) {
			mir_strcpy((char*)MH.name, szFieldNameR);
			flipStr(MH.value, -1, (char*)MH.value);
			return MH.value;
		}
	}

	return NULL;
}

static const struct _tag_cpltbl
{
	unsigned cp;
	const char* mimecp;
} cptbl[] =
{
	{ 37, "IBM037" },          // IBM EBCDIC US-Canada
	{ 437, "IBM437" },          // OEM United States
	{ 500, "IBM500" },          // IBM EBCDIC International
	{ 708, "ASMO-708" },        // Arabic (ASMO 708)
	{ 720, "DOS-720" },         // Arabic (Transparent ASMO); Arabic (DOS)
	{ 737, "ibm737" },          // OEM Greek (formerly 437G); Greek (DOS)
	{ 775, "ibm775" },          // OEM Baltic; Baltic (DOS)
	{ 850, "ibm850" },          // OEM Multilingual Latin 1; Western European (DOS)
	{ 852, "ibm852" },          // OEM Latin 2; Central European (DOS)
	{ 855, "IBM855" },          // OEM Cyrillic (primarily Russian)
	{ 857, "ibm857" },          // OEM Turkish; Turkish (DOS)
	{ 858, "IBM00858" },        // OEM Multilingual Latin 1 + Euro symbol
	{ 860, "IBM860" },          // OEM Portuguese; Portuguese (DOS)
	{ 861, "ibm861" },          // OEM Icelandic; Icelandic (DOS)
	{ 862, "DOS-862" },         // OEM Hebrew; Hebrew (DOS)
	{ 863, "IBM863" },          // OEM French Canadian; French Canadian (DOS)
	{ 864, "IBM864" },          // OEM Arabic; Arabic (864)
	{ 865, "IBM865" },          // OEM Nordic; Nordic (DOS)
	{ 866, "cp866" },           // OEM Russian; Cyrillic (DOS)
	{ 869, "ibm869" },          // OEM Modern Greek; Greek, Modern (DOS)
	{ 870, "IBM870" },          // IBM EBCDIC Multilingual/ROECE (Latin 2); IBM EBCDIC Multilingual Latin 2
	{ 874, "windows-874" },     // ANSI/OEM Thai (same as 28605, ISO 8859-15); Thai (Windows)
	{ 875, "cp875" },           // IBM EBCDIC Greek Modern
	{ 932, "shift_jis" },       // ANSI/OEM Japanese; Japanese (Shift-JIS)
	{ 936, "gb2312" },          // ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)
	{ 949, "ks_c_5601-1987" },  // ANSI/OEM Korean (Unified Hangul Code)
	{ 950, "big5" },            // ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)
	{ 1026, "IBM1026" },         // IBM EBCDIC Turkish (Latin 5)
	{ 1047, "IBM01047" },        // IBM EBCDIC Latin 1/Open System
	{ 1140, "IBM01140" },        // IBM EBCDIC US-Canada (037 + Euro symbol); IBM EBCDIC (US-Canada-Euro)
	{ 1141, "IBM01141" },        // IBM EBCDIC Germany (20273 + Euro symbol); IBM EBCDIC (Germany-Euro)
	{ 1142, "IBM01142" },        // IBM EBCDIC Denmark-Norway (20277 + Euro symbol); IBM EBCDIC (Denmark-Norway-Euro)
	{ 1143, "IBM01143" },        // IBM EBCDIC Finland-Sweden (20278 + Euro symbol); IBM EBCDIC (Finland-Sweden-Euro)
	{ 1144, "IBM01144" },        // IBM EBCDIC Italy (20280 + Euro symbol); IBM EBCDIC (Italy-Euro)
	{ 1145, "IBM01145" },        // IBM EBCDIC Latin America-Spain (20284 + Euro symbol); IBM EBCDIC (Spain-Euro)
	{ 1146, "IBM01146" },        // IBM EBCDIC United Kingdom (20285 + Euro symbol); IBM EBCDIC (UK-Euro)
	{ 1147, "IBM01147" },        // IBM EBCDIC France (20297 + Euro symbol); IBM EBCDIC (France-Euro)
	{ 1148, "IBM01148" },        // IBM EBCDIC International (500 + Euro symbol); IBM EBCDIC (International-Euro)
	{ 1149, "IBM01149" },        // IBM EBCDIC Icelandic (20871 + Euro symbol); IBM EBCDIC (Icelandic-Euro)
	{ 1250, "windows-1250" },    // ANSI Central European; Central European (Windows)
	{ 1251, "windows-1251" },    // ANSI Cyrillic; Cyrillic (Windows)
	{ 1252, "windows-1252" },    // ANSI Latin 1; Western European (Windows)
	{ 1253, "windows-1253" },    // ANSI Greek; Greek (Windows)
	{ 1254, "windows-1254" },    // ANSI Turkish; Turkish (Windows)
	{ 1255, "windows-1255" },    // ANSI Hebrew; Hebrew (Windows)
	{ 1256, "windows-1256" },    // ANSI Arabic; Arabic (Windows)
	{ 1257, "windows-1257" },    // ANSI Baltic; Baltic (Windows)
	{ 1258, "windows-1258" },    // ANSI/OEM Vietnamese; Vietnamese (Windows)
	{ 20127, "us-ascii" },        // US-ASCII (7-bit)
	{ 20273, "IBM273" },          // IBM EBCDIC Germany
	{ 20277, "IBM277" },          // IBM EBCDIC Denmark-Norway
	{ 20278, "IBM278" },          // IBM EBCDIC Finland-Sweden
	{ 20280, "IBM280" },          // IBM EBCDIC Italy
	{ 20284, "IBM284" },          // IBM EBCDIC Latin America-Spain
	{ 20285, "IBM285" },          // IBM EBCDIC United Kingdom
	{ 20290, "IBM290" },          // IBM EBCDIC Japanese Katakana Extended
	{ 20297, "IBM297" },          // IBM EBCDIC France
	{ 20420, "IBM420" },          // IBM EBCDIC Arabic
	{ 20423, "IBM423" },          // IBM EBCDIC Greek
	{ 20424, "IBM424" },          // IBM EBCDIC Hebrew
	{ 20838, "IBM-Thai" },        // IBM EBCDIC Thai
	{ 20866, "koi8-r" },          // Russian (KOI8-R); Cyrillic (KOI8-R)
	{ 20871, "IBM871" },          // IBM EBCDIC Icelandic
	{ 20880, "IBM880" },          // IBM EBCDIC Cyrillic Russian
	{ 20905, "IBM905" },          // IBM EBCDIC Turkish
	{ 20924, "IBM00924" },        // IBM EBCDIC Latin 1/Open System (1047 + Euro symbol)
	{ 20932, "EUC-JP" },          // Japanese (JIS 0208-1990 and 0121-1990)
	{ 21025, "cp1025" },          // IBM EBCDIC Cyrillic Serbian-Bulgarian
	{ 21866, "koi8-u" },          // Ukrainian (KOI8-U); Cyrillic (KOI8-U)
	{ 28591, "iso-8859-1" },      // ISO 8859-1 Latin 1; Western European (ISO)
	{ 28592, "iso-8859-2" },      // ISO 8859-2 Central European; Central European (ISO)
	{ 28593, "iso-8859-3" },      // ISO 8859-3 Latin 3
	{ 28594, "iso-8859-4" },      // ISO 8859-4 Baltic
	{ 28595, "iso-8859-5" },      // ISO 8859-5 Cyrillic
	{ 28596, "iso-8859-6" },      // ISO 8859-6 Arabic
	{ 28597, "iso-8859-7" },      // ISO 8859-7 Greek
	{ 28598, "iso-8859-8" },      // ISO 8859-8 Hebrew; Hebrew (ISO-Visual)
	{ 28599, "iso-8859-9" },      // ISO 8859-9 Turkish
	{ 28603, "iso-8859-13" },     // ISO 8859-13 Estonian
	{ 28605, "iso-8859-15" },     // ISO 8859-15 Latin 9
	{ 38598, "iso-8859-8-i" },    // ISO 8859-8 Hebrew; Hebrew (ISO-Logical)
	{ 50220, "iso-2022-jp" },     // ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)
	{ 50221, "csISO2022JP" },     // ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana)
	{ 50222, "iso-2022-jp" },     // ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI)
	{ 50225, "iso-2022-kr" },     // ISO 2022 Korean
	{ 50227, "ISO-2022-CN" },     // ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022)
	{ 50229, "ISO-2022-CN-EXT" }, // ISO 2022 Traditional Chinese
	{ 51932, "euc-jp" },          // EUC Japanese
	{ 51936, "EUC-CN" },          // EUC Simplified Chinese; Chinese Simplified (EUC)
	{ 51949, "euc-kr" },          // EUC Korean
	{ 52936, "hz-gb-2312" },      // HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)
	{ 54936, "GB18030" },         // Windows XP and later: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030)
};


static unsigned FindCP(const char* mimecp)
{
	unsigned cp = CP_ACP;
	for (unsigned i = 0; i < SIZEOF(cptbl); ++i) {
		if (_stricmp(mimecp, cptbl[i].mimecp) == 0) {
			cp = cptbl[i].cp;
			break;
		}
	}
	return cp;
}


static int SingleHexToDecimal(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static void  PQDecode(char* str)
{
	char* s = str, *d = str;

	while (*s) {
		switch (*s) {
		case '=':
		{
			int digit1 = SingleHexToDecimal(s[1]);
			if (digit1 != -1) {
				int digit2 = SingleHexToDecimal(s[2]);
				if (digit2 != -1) {
					s += 3;
					*d++ = (char)((digit1 << 4) | digit2);
				}
			}
			break;
		}

		case '_':
			*d++ = ' '; ++s;
			break;

		default:
			*d++ = *s++;
			break;
		}
	}
	*d = 0;
}

static size_t utf8toutf16(char* str, wchar_t* res)
{
	wchar_t *dec = mir_utf8decodeW(str);
	if (dec == NULL) dec = mir_a2u(str);
	mir_wstrcpy(res, dec);
	mir_free(dec);
	return mir_wstrlen(res);
}


wchar_t* MimeHeaders::decode(const char* val)
{
	size_t ssz = mir_strlen(val) * 2 + 1;
	char* tbuf = (char*)alloca(ssz);
	memcpy(tbuf, val, ssz);

	wchar_t* res = (wchar_t*)mir_alloc(ssz * sizeof(wchar_t));
	wchar_t* resp = res;

	char *p = tbuf;
	while (*p) {
		char *cp = strstr(p, "=?");
		if (cp == NULL) break;
		*cp = 0;

		size_t sz = utf8toutf16(p, resp);
		ssz -= sz; resp += sz;
		cp += 2;

		char *enc = strchr(cp, '?');
		if (enc == NULL) break;
		*(enc++) = 0;

		char *fld = strchr(enc, '?');
		if (fld == NULL) break;
		*(fld++) = 0;

		char *pe = strstr(fld, "?=");
		if (pe == NULL) break;
		*pe = 0;

		switch (*enc) {
		case 'b':
		case 'B':
		{
			char* dec = (char*)mir_base64_decode(fld, 0);
			mir_strcpy(fld, dec);
			mir_free(dec);
			break;
		}

		case 'q':
		case 'Q':
			PQDecode(fld);
			break;
		}

		if (_stricmp(cp, "UTF-8") == 0) {
			sz = utf8toutf16(fld, resp);
			ssz -= sz; resp += sz;
		}
		else {
			int sz = MultiByteToWideChar(FindCP(cp), 0, fld, -1, resp, (int)ssz);
			if (sz == 0)
				sz = MultiByteToWideChar(CP_ACP, 0, fld, -1, resp, (int)ssz);
			ssz -= --sz; resp += sz;
		}
		p = pe + 2;
	}

	utf8toutf16(p, resp);

	return res;
}


char* MimeHeaders::decodeMailBody(char* msgBody)
{
	char* res;
	const char *val = find("Content-Transfer-Encoding");
	if (val && _stricmp(val, "base64") == 0) {
		char *src = msgBody, *dst = msgBody;
		while (*src != 0) {
			if (isspace(*src)) ++src;
			else *(dst++) = *(src++);
		}
		*dst = 0;
		res = (char*)mir_base64_decode(msgBody, 0);
	}
	else {
		res = mir_strdup(msgBody);
		if (val && _stricmp(val, "quoted-printable") == 0)
			PQDecode(res);
	}
	return res;
}


int sttDivideWords(char* parBuffer, int parMinItems, char** parDest)
{
	int i;
	for (i = 0; i < parMinItems; i++) {
		parDest[i] = parBuffer;

		size_t tWordLen = strcspn(parBuffer, " \t");
		if (tWordLen == 0)
			return i;

		parBuffer += tWordLen;
		if (*parBuffer != '\0') {
			size_t tSpaceLen = strspn(parBuffer, " \t");
			memset(parBuffer, 0, tSpaceLen);
			parBuffer += tSpaceLen;
		}
	}

	return i;
}
