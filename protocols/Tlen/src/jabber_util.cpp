/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "jabber.h"
#include "jabber_list.h"
#include <ctype.h>
#include <win2k.h>


HANDLE HookEventObj_Ex(const char *name, TlenProtocol *proto, MIRANDAHOOKOBJ hook)
{
	proto->hookNum ++;
	proto->hHooks = (HANDLE *) mir_realloc(proto->hHooks, sizeof(HANDLE) * (proto->hookNum));
	proto->hHooks[proto->hookNum - 1] = HookEventObj(name, hook, proto);
	return proto->hHooks[proto->hookNum - 1] ;
}


HANDLE CreateServiceFunction_Ex(const char *name, TlenProtocol *proto, MIRANDASERVICEOBJ service) {
	proto->serviceNum++;
	proto->hServices = (HANDLE *) mir_realloc(proto->hServices, sizeof(HANDLE) * (proto->serviceNum));
	proto->hServices[proto->serviceNum - 1] = CreateServiceFunctionObj(name, service, proto);
	return proto->hServices[proto->serviceNum - 1] ;
}

void UnhookEvents_Ex(TlenProtocol *proto) {
	unsigned int i;
	for (i=0; i<proto->hookNum; ++i) {
		if (proto->hHooks[i] != NULL) {
			UnhookEvent(proto->hHooks[i]);
		}
	}
	mir_free(proto->hHooks);
	proto->hookNum = 0;
	proto->hHooks = NULL;
}

void DestroyServices_Ex(TlenProtocol *proto) {
	unsigned int i;
	for (i=0; i<proto->serviceNum; ++i) {
		if (proto->hServices[i] != NULL) {
			DestroyServiceFunction(proto->hServices[i]);
		}
	}
	mir_free(proto->hServices);
	proto->serviceNum = 0;
	proto->hServices = NULL;
}

void JabberSerialInit(TlenProtocol *proto)
{
	InitializeCriticalSection(&proto->csSerial);
	proto->serial = 0;
}

void JabberSerialUninit(TlenProtocol *proto)
{
	DeleteCriticalSection(&proto->csSerial);
}


unsigned int JabberSerialNext(TlenProtocol *proto)
{
	unsigned int ret;

	EnterCriticalSection(&proto->csSerial);
	ret = proto->serial;
	proto->serial++;
	LeaveCriticalSection(&proto->csSerial);
	return ret;
}


void JabberLog(TlenProtocol *proto, const char *fmt, ...)
{
#ifdef ENABLE_LOGGING
	char *str;
	va_list vararg;
	int strsize;
	char *text;
	char *p, *q;
	int extra;

	va_start(vararg, fmt);
	str = (char *) mir_alloc(strsize=2048);
	while (_vsnprintf(str, strsize, fmt, vararg) == -1)
		str = (char *) mir_realloc(str, strsize+=2048);
	va_end(vararg);

	extra = 0;
	for (p=str; *p != '\0'; p++)
		if (*p == '\n' || *p == '\r')
			extra++;
	text = (char *) mir_alloc(strlen("TLEN")+2+strlen(str)+2+extra);
	sprintf(text, "[%s]", "TLEN");
	for (p=str,q=text+strlen(text); *p != '\0'; p++,q++) {
		if (*p == '\r') {
			*q = '\\';
			*(q+1) = 'r';
			q++;
		}
		else if (*p == '\n') {
			*q = '\\';
			*(q+1) = 'n';
			q++;
		}
		else
			*q = *p;
	}
	*q = '\n';
	*(q+1) = '\0';
	if (proto->hNetlibUser != NULL) {
		CallService(MS_NETLIB_LOG, (WPARAM) proto->hNetlibUser, (LPARAM) text);
	}
	//OutputDebugString(text);
	mir_free(text);
	mir_free(str);
#endif
}

// Caution: DO NOT use JabberSend() to send binary (non-string) data

int JabberSend(TlenProtocol *proto, const char *fmt, ...)
{
	char *str;
	int size;
	va_list vararg;
	int result = 0;

	EnterCriticalSection(&proto->csSend);

	va_start(vararg,fmt);
	size = 512;
	str = (char *) mir_alloc(size);
	while (_vsnprintf(str, size, fmt, vararg) == -1) {
		size += 512;
		str = (char *) mir_realloc(str, size);
	}
	va_end(vararg);

	JabberLog(proto, "SEND:%s", str);
	size = (int)strlen(str);
	if (proto->threadData != NULL) {
		if (proto->threadData->useAES) {
			result = JabberWsSendAES(proto, str, size, &proto->threadData->aes_out_context, proto->threadData->aes_out_iv);
		} else {
			result = JabberWsSend(proto, proto->threadData->s, str, size);
		}
	}
	LeaveCriticalSection(&proto->csSend);

	mir_free(str);
	return result;
}

char *JabberResourceFromJID(const char *jid2)
{
	char *p;
	char *nick;
	char* jid = mir_strdup(jid2);

	p=strchr(jid, '/');
	if (p != NULL && p[1] != '\0') {
		p++;
		if ((nick=(char *) mir_alloc(1+strlen(jid)-(p-jid))) != NULL) {
			strncpy(nick, p, strlen(jid)-(p-jid));
			nick[strlen(jid)-(p-jid)] = '\0';
		}
	}
	else {
		nick = mir_strdup(jid);
	}

	mir_free(jid);
	return nick;
}

char *JabberNickFromJID(const char *jid2)
{
	char *p;
	char *nick;
	char* jid = mir_strdup(jid2);

	if ((p=strchr(jid, '@')) == NULL)
		p = strchr(jid, '/');
	if (p != NULL) {
		if ((nick=(char *) mir_alloc((p-jid)+1)) != NULL) {
			strncpy(nick, jid, p-jid);
			nick[p-jid] = '\0';
		}
	}
	else {
		nick = mir_strdup(jid);
	}

	mir_free(jid);
	return nick;
}

char *JabberLoginFromJID(const char *jid2)
{
	char *p;
	char *nick;
	char* jid = mir_strdup(jid2);

	p = strchr(jid, '/');
	if (p != NULL) {
		if ((nick=(char *) mir_alloc((p-jid)+1)) != NULL) {
			strncpy(nick, jid, p-jid);
			nick[p-jid] = '\0';
		}
	}
	else {
		nick = mir_strdup(jid);
	}

	mir_free(jid);
	return nick;
}

char *JabberLocalNickFromJID(const char *jid)
{
	char *p;
	char *localNick;

	p = JabberNickFromJID(jid);
	localNick = JabberTextDecode(p);
	mir_free(p);
	return localNick;
}

char *JabberSha1(char *str)
{
	mir_sha1_ctx sha;
	DWORD digest[5];
	char* result;

	if ( str == NULL )
		return NULL;

	mir_sha1_init( &sha );
	mir_sha1_append( &sha, (BYTE* )str, (int)strlen( str ));
	mir_sha1_finish( &sha, (BYTE* )digest );
	if ((result=(char *)mir_alloc(41)) == NULL)
		return NULL;
	sprintf(result, "%08x%08x%08x%08x%08x", (int)htonl(digest[0]), (int)htonl(digest[1]), (int)htonl(digest[2]), (int)htonl(digest[3]), (int)htonl(digest[4]));
	return result;
}

char *TlenSha1(char *str, int len)
{
	mir_sha1_ctx sha;
	BYTE digest[20];
	char* result;
	int i;

	if ( str == NULL )
		return NULL;

	mir_sha1_init( &sha );
	mir_sha1_append( &sha, (BYTE* )str, len);
	mir_sha1_finish( &sha, digest );
	if (( result=( char* )mir_alloc( 20 )) == NULL )
		return NULL;
	for (i=0; i<20; i++)
		result[i]=digest[4*(i>>2)+(3-(i&0x3))];
	return result;
}

char *TlenPasswordHash(const char *str)
{
	int magic1 = 0x50305735, magic2 = 0x12345671, sum = 7;
	char *p, *res;

	if (str == NULL) return NULL;
	for (p=(char *)str; *p != '\0'; p++) {
		if (*p != ' ' && *p != '\t') {
			magic1 ^= (((magic1 & 0x3f) + sum) * ((char) *p)) + (magic1 << 8);
			magic2 += (magic2 << 8) ^ magic1;
			sum += ((char) *p);
		}
	}
	magic1 &= 0x7fffffff;
	magic2 &= 0x7fffffff;
	res = (char *) mir_alloc(17);
	sprintf(res, "%08x%08x", magic1, magic2);
	return res;
}

char *TlenUrlEncode(const char *str)
{
	char *p, *q, *res;
	unsigned char c;

	if (str == NULL) return NULL;
	res = (char *) mir_alloc(3*strlen(str) + 1);
	for (p=(char *)str,q=res; *p != '\0'; p++,q++) {
		if (*p == ' ') {
			*q = '+';
		}
		else if (*p < 0x20 || *p >= 0x7f || strchr("%&+:'<>\"", *p) != NULL) {
			// Convert first from CP1252 to ISO8859-2
			switch ((unsigned char) *p) {
			case 0xa5: c = (unsigned char) 0xa1; break;
			case 0x8c: c = (unsigned char) 0xa6; break;
			case 0x8f: c = (unsigned char) 0xac; break;
			case 0xb9: c = (unsigned char) 0xb1; break;
			case 0x9c: c = (unsigned char) 0xb6; break;
			case 0x9f: c = (unsigned char) 0xbc; break;
			default: c = (unsigned char) *p; break;
			}
			sprintf(q, "%%%02X", c);
			q += 2;
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
	return res;
}

void TlenUrlDecode(char *str)
{
	char *p, *q;
	unsigned int code;

	if (str == NULL) return;
	for (p=q=str; *p != '\0'; p++,q++) {
		if (*p == '+') {
			*q = ' ';
		}
		else if (*p == '%' && *(p+1) != '\0' && isxdigit(*(p+1)) && *(p+2) != '\0' && isxdigit(*(p+2))) {
			sscanf(p+1, "%2x", &code);
			*q = (char) code;
			// Convert from ISO8859-2 to CP1252
			switch ((unsigned char) *q) {
			case 0xa1: *q = (char) 0xa5; break;
			case 0xa6: *q = (char) 0x8c; break;
			case 0xac: *q = (char) 0x8f; break;
			case 0xb1: *q = (char) 0xb9; break;
			case 0xb6: *q = (char) 0x9c; break;
			case 0xbc: *q = (char) 0x9f; break;
			}
			p += 2;
		}
		else {
			*q = *p;
		}
	}
	*q = '\0';
}

char * TlenGroupDecode(const char *str)
{
	char *p, *q;
	if (str == NULL) return NULL;
	p = q = JabberTextDecode(str);
	for (; *p != '\0'; p++) {
		if (*p == '/') {
			*p = '\\';
		}
	}
	return q;
}

char * TlenGroupEncode(const char *str)
{
	char *p, *q;
	if (str == NULL) return NULL;
	p = q = mir_strdup(str);
	for (; *p != '\0'; p++) {
		if (*p == '\\') {
			*p = '/';
		}
	}
	p = JabberTextEncode(q);
	mir_free(q);
	return p;
}

char *JabberTextEncode(const char *str)
{
	char *s1;

	if (str == NULL) return NULL;
	if ((s1=TlenUrlEncode(str)) == NULL)
		return NULL;
	return s1;
}

char *JabberTextDecode(const char *str)
{
	char *s1;

	if (str == NULL) return NULL;
	s1 = mir_strdup(str);
	TlenUrlDecode(s1);
	return s1;
}

static char b64table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *JabberBase64Encode(const char *buffer, int bufferLen)
{
	int n;
	unsigned char igroup[3];
	char *p, *peob;
	char *res, *r;

	if (buffer == NULL || bufferLen <= 0) return NULL;
	if ((res=(char *) mir_alloc((((bufferLen+2)/3)*4) + 1)) == NULL) return NULL;

	for (p=(char*)buffer,peob=p+bufferLen,r=res; p<peob;) {
		igroup[0] = igroup[1] = igroup[2] = 0;
		for (n=0; n<3; n++) {
			if (p >= peob) break;
			igroup[n] = (unsigned char) *p;
			p++;
		}
		if (n > 0) {
			r[0] = b64table[ igroup[0]>>2 ];
			r[1] = b64table[ ((igroup[0]&3)<<4) | (igroup[1]>>4) ];
			r[2] = b64table[ ((igroup[1]&0xf)<<2) | (igroup[2]>>6) ];
			r[3] = b64table[ igroup[2]&0x3f ];
			if (n < 3) {
				r[3] = '=';
				if (n < 2)
					r[2] = '=';
			}
			r += 4;
		}
	}
	*r = '\0';

	return res;
}

static unsigned char b64rtable[256];

char *JabberBase64Decode(const char *str, int *resultLen)
{
	char *res;
	unsigned char *p, *r, igroup[4], a[4];
	int n, num, count;

	if (str == NULL || resultLen == NULL) return NULL;
	if ((res=(char *) mir_alloc(((strlen(str)+3)/4)*3)) == NULL) return NULL;

	for (n=0; n<256; n++)
		b64rtable[n] = (unsigned char) 0x80;
	for (n=0; n<26; n++)
		b64rtable['A'+n] = n;
	for (n=0; n<26; n++)
		b64rtable['a'+n] = n + 26;
	for (n=0; n<10; n++)
		b64rtable['0'+n] = n + 52;
	b64rtable['+'] = 62;
	b64rtable['/'] = 63;
	b64rtable['='] = 0;
	count = 0;
	for (p=(unsigned char *)str,r=(unsigned char *)res; *p != '\0';) {
		for (n=0; n<4; n++) {
			if ( *p == '\r' || *p == '\n' ) {
				n--; p++;
				continue;
			}

			if ( *p=='\0' ) {
				if ( n == 0 )
					goto LBL_Exit;
				mir_free( res );
				return NULL;
			}

			if ( b64rtable[*p] == 0x80 ) {
				mir_free( res );
				return NULL;
			}

			a[n] = *p;
			igroup[n] = b64rtable[*p];
			p++;
		}
		r[0] = igroup[0]<<2 | igroup[1]>>4;
		r[1] = igroup[1]<<4 | igroup[2]>>2;
		r[2] = igroup[2]<<6 | igroup[3];
		r += 3;
		num = ( a[2] == '='?1:( a[3] == '='?2:3 ));
		count += num;
		if ( num < 3 ) break;
	}
LBL_Exit:
	*resultLen = count;
	return res;
}

/*
 *	Apply Polish Daylight Saving Time rules to get "DST-unbiased" timestamp
 */

time_t  TlenTimeToUTC(time_t time) {
	struct tm *timestamp;
	timestamp = gmtime(&time);
	if ( (timestamp->tm_mon > 2 && timestamp->tm_mon < 9) ||
		 (timestamp->tm_mon == 2 && timestamp->tm_mday - timestamp->tm_wday >= 25) ||
		 (timestamp->tm_mon == 9 && timestamp->tm_mday - timestamp->tm_wday < 25)) {
		//time -= 3600;
	} else {
		//time += 3600;
	}
	return time;
}

time_t JabberIsoToUnixTime(char *stamp)
{
	struct tm timestamp;
	char date[9];
	char *p;
	int i, y;
	time_t t;

	if (stamp == NULL) return (time_t) 0;

	p = stamp;

	// Get the date part
	for (i=0; *p != '\0' && i<8 && isdigit(*p); p++,i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year (1970-2069)
		y = (date[0]-'0')*10 + (date[1]-'0');
		if (y < 70) y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0]-'0')*1000 + (date[1]-'0')*100 + (date[2]-'0')*10 + date[3]-'0';
		y -= 1900;
	}
	else
		return (time_t) 0;
	timestamp.tm_year = y;
	// Parse month
	timestamp.tm_mon = (date[i-4]-'0')*10 + date[i-3]-'0' - 1;
	// Parse date
	timestamp.tm_mday = (date[i-2]-'0')*10 + date[i-1]-'0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (sscanf(p, "%d:%d:%d", &(timestamp.tm_hour), &(timestamp.tm_min), &(timestamp.tm_sec)) != 3)
		return (time_t) 0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	_tzset();
	t = mktime(&timestamp);
	t -= _timezone;
	t = TlenTimeToUTC(t);

	if (t >= 0)
		return t;
	else
		return (time_t) 0;
}

void JabberStringAppend(char **str, int *sizeAlloced, const char *fmt, ...)
{
	va_list vararg;
	char *p;
	int size, len;

	if (str == NULL) return;

	if (*str == NULL || *sizeAlloced <= 0) {
		*sizeAlloced = size = 2048;
		*str = (char *) mir_alloc(size);
		len = 0;
	}
	else {
		len = (int)strlen(*str);
		size = *sizeAlloced - (int)strlen(*str);
	}

	p = *str + len;
	va_start(vararg, fmt);
	while (_vsnprintf(p, size, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *) mir_realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	va_end(vararg);
}

BOOL IsAuthorized(TlenProtocol *proto, const char *jid)
{
	JABBER_LIST_ITEM *item = JabberListGetItemPtr(proto, LIST_ROSTER, jid);
	if (item != NULL) {
		return item->subscription == SUB_BOTH || item->subscription == SUB_FROM;
	}
	return FALSE;
}


void TlenLogMessage(TlenProtocol *proto, HANDLE hContact, DWORD flags, const char *message)
{
	int size = (int)strlen(message) + 2;
	char *localMessage = (char *)mir_alloc(size);
	strcpy(localMessage, message);
	localMessage[size - 1] = '\0';
	JabberDBAddEvent(proto, hContact, EVENTTYPE_MESSAGE, flags, (PBYTE)message, (DWORD)size);
	mir_free(localMessage);
}
