// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

WORD generate_flap_sequence()
{
	DWORD n = rand(), s = 0;

	for (DWORD i = n; i >>= 3; s += i);

	return (((0 - s) ^ (BYTE)n) & 7 ^ n) + 2;
}

void __fastcall init_generic_packet(icq_packet *pPacket, WORD wHeaderLen)
{
	pPacket->wPlace = 0;
	pPacket->wLen += wHeaderLen;
	pPacket->pData = (BYTE*)SAFE_MALLOC(pPacket->wLen);
}

void write_httphdr(icq_packet *pPacket, WORD wType, DWORD dwSeq)
{
	init_generic_packet(pPacket, 14);

	packWord(pPacket, (WORD)(pPacket->wLen - 2));
	packWord(pPacket, HTTP_PROXY_VERSION);
	packWord(pPacket, wType);
	packDWord(pPacket, 0); // Flags?
	packDWord(pPacket, dwSeq); // Connection sequence ?
}

void __fastcall write_flap(icq_packet *pPacket, BYTE byFlapChannel)
{
	init_generic_packet(pPacket, 6);

	pPacket->nChannel = byFlapChannel;

	packByte(pPacket, FLAP_MARKER);
	packByte(pPacket, byFlapChannel);
	packWord(pPacket, 0);                 // This is the sequence ID, it is filled in during the actual sending
	packWord(pPacket, (WORD)(pPacket->wLen - 6)); // This counter should not include the flap header (thus the -6)
}

void __fastcall serverPacketInit(icq_packet *pPacket, WORD wSize)
{
	pPacket->wLen = wSize;
	write_flap(pPacket, ICQ_DATA_CHAN);
}

void __fastcall directPacketInit(icq_packet *pPacket, DWORD dwSize)
{
	pPacket->wPlace = 0;
	pPacket->wLen   = (WORD)dwSize;
	pPacket->pData  = (BYTE *)SAFE_MALLOC(dwSize + 2);

	packLEWord(pPacket, pPacket->wLen);
}

void __fastcall serverCookieInit(icq_packet *pPacket, BYTE *pCookie, WORD wCookieSize)
{
	pPacket->wLen = (WORD)(wCookieSize + 8 + sizeof(CLIENT_ID_STRING) + 66);

	write_flap(pPacket, ICQ_LOGIN_CHAN);
	packDWord(pPacket, 0x00000001);
	packTLV(pPacket, 0x06, wCookieSize, pCookie);

	// Pack client identification details.
	packTLV(pPacket, 0x0003, (WORD)sizeof(CLIENT_ID_STRING)-1, (LPBYTE)CLIENT_ID_STRING);
	packTLVWord(pPacket, 0x0017, CLIENT_VERSION_MAJOR);
	packTLVWord(pPacket, 0x0018, CLIENT_VERSION_MINOR);
	packTLVWord(pPacket, 0x0019, CLIENT_VERSION_LESSER);
	packTLVWord(pPacket, 0x001a, CLIENT_VERSION_BUILD);
	packTLVWord(pPacket, 0x0016, CLIENT_ID_CODE);
	packTLVDWord(pPacket, 0x0014, CLIENT_DISTRIBUTION);
	packTLV(pPacket, 0x000f, 0x0002, (LPBYTE)CLIENT_LANGUAGE);
	packTLV(pPacket, 0x000e, 0x0002, (LPBYTE)CLIENT_COUNTRY);
	packDWord(pPacket, 0x00940001); // reconnect flag
	packByte(pPacket, 0);
	packTLVDWord(pPacket, 0x8003, 0x00100000); // Unknown
}

void __fastcall packByte(icq_packet *pPacket, BYTE byValue)
{
	pPacket->pData[pPacket->wPlace++] = byValue;
}

void __fastcall packWord(icq_packet *pPacket, WORD wValue)
{
	pPacket->pData[pPacket->wPlace++] = ((wValue & 0xff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (wValue & 0x00ff);
}

void __fastcall packDWord(icq_packet *pPacket, DWORD dwValue)
{
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0xff000000) >> 24);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x00ff0000) >> 16);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x0000ff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (BYTE) (dwValue & 0x000000ff);
}

void __fastcall packQWord(icq_packet *pPacket, DWORD64 qwValue)
{
	packDWord(pPacket, (DWORD)(qwValue >> 32));
	packDWord(pPacket, (DWORD)(qwValue & 0xffffffff));
}

void packTLV(icq_packet *pPacket, WORD wType, WORD wLength, const BYTE *pbyValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, wLength);
	packBuffer(pPacket, pbyValue, wLength);
}

void packTLVWord(icq_packet *pPacket, WORD wType, WORD wValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, 0x02);
	packWord(pPacket, wValue);
}

void packTLVDWord(icq_packet *pPacket, WORD wType, DWORD dwValue)
{
	packWord(pPacket, wType);
	packWord(pPacket, 0x04);
	packDWord(pPacket, dwValue);
}


void packTLVUID(icq_packet *pPacket, WORD wType, DWORD dwUin, const char *szUid)
{
	if (dwUin)
  {
    char szUin[UINMAXLEN];

    _ltoa(dwUin, szUin, 10);

    packTLV(pPacket, wType, getUINLen(dwUin), (BYTE*)szUin);
  }
  else if (szUid)
    packTLV(pPacket, wType, strlennull(szUid), (BYTE*)szUid);
}


// Pack a preformatted buffer.
// This can be used to pack strings or any type of raw data.
void packBuffer(icq_packet *pPacket, const BYTE* pbyBuffer, WORD wLength)
{
	while (wLength)
	{
		pPacket->pData[pPacket->wPlace++] = *pbyBuffer++;
		wLength--;
	}
}

// Pack a buffer and prepend it with the size as a LE WORD.
// Commented out since its not actually used anywhere right now.
//void packLEWordSizedBuffer(icq_packet* pPacket, const BYTE* pbyBuffer, WORD wLength)
//{
//
//  packLEWord(pPacket, wLength);
//  packBuffer(pPacket, pbyBuffer, wLength);
//
//}

int __fastcall getUINLen(DWORD dwUin)
{
  BYTE dwUinLen = 0;

  while(dwUin) {
	  dwUin /= 10;
	  dwUinLen += 1;
  }
  return dwUinLen;
}

int __fastcall getUIDLen(DWORD dwUin, const char *szUid)
{
	if (dwUin)
		return getUINLen(dwUin);
	else
		return strlennull(szUid);
}

void __fastcall packUIN(icq_packet *pPacket, DWORD dwUin)
{
	char pszUin[UINMAXLEN];
	BYTE nUinLen = getUINLen(dwUin);

	_ltoa(dwUin, pszUin, 10);

	packByte(pPacket, nUinLen);           // Length of user id
	packBuffer(pPacket, (LPBYTE)pszUin, nUinLen); // Receiving user's id
}

void __fastcall packUID(icq_packet *pPacket, DWORD dwUin, const char *szUid)
{
	if (dwUin)
		packUIN(pPacket, dwUin);
	else
	{
		BYTE nLen = strlennull(szUid);
		packByte(pPacket, nLen);
		packBuffer(pPacket, (LPBYTE)szUid, nLen);
	}
}


void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype)
{
  packFNACHeader(pPacket, wFamily, wSubtype, 0, wSubtype << 0x10);
}


void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype, WORD wFlags, DWORD dwSequence)
{
	WORD wSequence = (WORD)dwSequence & 0x7FFF; // this is necessary, if that bit is there we get disconnected

	packWord(pPacket, wFamily);   // Family type
	packWord(pPacket, wSubtype);  // Family subtype
	packWord(pPacket, wFlags);    // SNAC flags
	packWord(pPacket, wSequence); // SNAC request id (sequence)
	packWord(pPacket, (WORD)(dwSequence >> 0x10)); // SNAC request id (command)
}


void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype, WORD wFlags, DWORD dwSequence, WORD wVersion)
{
  packFNACHeader(pPacket, wFamily, wSubtype, wFlags | 0x8000, dwSequence);
  packWord(pPacket, 0x06);
  packTLVWord(pPacket, 0x01, wVersion);
}


void __fastcall packLEWord(icq_packet *pPacket, WORD wValue)
{
	pPacket->pData[pPacket->wPlace++] =  (wValue & 0x00ff);
	pPacket->pData[pPacket->wPlace++] = ((wValue & 0xff00) >> 8);
}

void __fastcall packLEDWord(icq_packet *pPacket, DWORD dwValue)
{
	pPacket->pData[pPacket->wPlace++] = (BYTE) (dwValue & 0x000000ff);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x0000ff00) >> 8);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0x00ff0000) >> 16);
	pPacket->pData[pPacket->wPlace++] = (BYTE)((dwValue & 0xff000000) >> 24);
}


/* helper function to place numerics to buffer */
static void packWord(PBYTE buf, WORD wValue)
{
  *(buf) = ((wValue & 0xff00) >> 8);
  *(buf + 1) = (wValue & 0x00ff);
}


static void packDWord(PBYTE buf, DWORD dwValue)
{
  *(buf) = (BYTE)((dwValue & 0xff000000) >> 24);
  *(buf + 1) = (BYTE)((dwValue & 0x00ff0000) >> 16);
  *(buf + 2) = (BYTE)((dwValue & 0x0000ff00) >> 8);
  *(buf + 3) = (BYTE) (dwValue & 0x000000ff);
}


static void packQWord(PBYTE buf, DWORD64 qwValue)
{
	packDWord(buf, (DWORD)(qwValue >> 32));
	packDWord(buf + 4, (DWORD)(qwValue & 0xffffffff));
}


void ppackByte(PBYTE *buf, int *buflen, BYTE byValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 1 + *buflen);
	*(*buf + *buflen) = byValue;
	++*buflen;
}


void ppackWord(PBYTE *buf, int *buflen, WORD wValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 2 + *buflen);
  packWord(*buf + *buflen, wValue);
	*buflen += 2;
}


void ppackLEWord(PBYTE *buf, int *buflen, WORD wValue)
{
	*buf=(PBYTE)SAFE_REALLOC(*buf, 2 + *buflen);
	*(PWORD)(*buf + *buflen) = wValue;
	*buflen+=2;
}


void ppackLEDWord(PBYTE *buf, int *buflen, DWORD dwValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 4 + *buflen);
	*(PDWORD)(*buf + *buflen) = dwValue;
	*buflen += 4;
}


void ppackLELNTS(PBYTE *buf, int *buflen, const char *str)
{
	WORD len = strlennull(str);
	ppackLEWord(buf, buflen, len);
	*buf = (PBYTE)SAFE_REALLOC(*buf, *buflen + len);
	memcpy(*buf + *buflen, str, len);
	*buflen += len;
}


void ppackBuffer(PBYTE *buf, int *buflen, WORD wLength, const BYTE *pbyValue)
{
  if (wLength)
  {
	  *buf = (PBYTE)SAFE_REALLOC(*buf, wLength + *buflen);
	  memcpy(*buf + *buflen, pbyValue, wLength);
	  *buflen += wLength;
  }
}


void ppackTLV(PBYTE *buf, int *buflen, WORD wType, WORD wLength, const BYTE *pbyValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 4 + wLength + *buflen);
	packWord(*buf + *buflen, wType);
	packWord(*buf + *buflen + 2, wLength);
  if (wLength)
	  memcpy(*buf + *buflen + 4, pbyValue, wLength);
	*buflen += 4 + wLength;
}


void ppackTLVByte(PBYTE *buf, int *buflen, WORD wType, BYTE byValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 5 + *buflen);
	packWord(*buf + *buflen, wType);
	packWord(*buf + *buflen + 2, 1);
  *(*buf + *buflen + 4) = byValue;
	*buflen += 5;
}


void ppackTLVWord(PBYTE *buf, int *buflen, WORD wType, WORD wValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 6 + *buflen);
	packWord(*buf + *buflen, wType);
	packWord(*buf + *buflen + 2, 2);
  packWord(*buf + *buflen + 4, wValue);
	*buflen += 6;
}


void ppackTLVDWord(PBYTE *buf, int *buflen, WORD wType, DWORD dwValue)
{
	*buf = (PBYTE)SAFE_REALLOC(*buf, 8 + *buflen);
	packWord(*buf + *buflen, wType);
	packWord(*buf + *buflen + 2, 4);
  packDWord(*buf + *buflen + 4, dwValue);
	*buflen += 8;
}


void ppackTLVDouble(PBYTE *buf, int *buflen, WORD wType, double dValue)
{
  DWORD64 qwValue;

  memcpy(&qwValue, &dValue, 8);

	*buf = (PBYTE)SAFE_REALLOC(*buf, 12 + *buflen);
	packWord(*buf + *buflen, wType);
	packWord(*buf + *buflen + 2, 8);
  packQWord(*buf + *buflen + 4, qwValue);
	*buflen += 12;
}


void ppackTLVUID(PBYTE *buf, int *buflen, WORD wType, DWORD dwUin, const char *szUid)
{
	if (dwUin)
  {
    char szUin[UINMAXLEN];

    _ltoa(dwUin, szUin, 10);

    ppackTLV(buf, buflen, wType, getUINLen(dwUin), (BYTE*)szUin);
  }
  else if (szUid)
    ppackTLV(buf, buflen, wType, strlennull(szUid), (BYTE*)szUid);
}


// *** TLV based (!!! WORDs and DWORDs are LE !!!)
void ppackLETLVByte(PBYTE *buf, int *buflen, BYTE byValue, WORD wType, BYTE always)
{
	if (!always && !byValue) return;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 5 + *buflen);
	*(PWORD)(*buf + *buflen) = wType;
	*(PWORD)(*buf + *buflen + 2) = 1;
	*(*buf + *buflen + 4) = byValue;
	*buflen += 5;
}


void ppackLETLVWord(PBYTE *buf, int *buflen, WORD wValue, WORD wType, BYTE always)
{
	if (!always && !wValue) return;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 6 + *buflen);
	*(PWORD)(*buf + *buflen) = wType;
	*(PWORD)(*buf + *buflen + 2) = 2;
	*(PWORD)(*buf + *buflen + 4) = wValue;
	*buflen += 6;
}


void ppackLETLVDWord(PBYTE *buf, int *buflen, DWORD dwValue, WORD wType, BYTE always)
{
	if (!always && !dwValue) return;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 8 + *buflen);
	*(PWORD)(*buf + *buflen) = wType;
	*(PWORD)(*buf + *buflen + 2) = 4;
	*(PDWORD)(*buf + *buflen + 4) = dwValue;
	*buflen += 8;
}


void packLETLVLNTS(PBYTE *buf, int *bufpos, const char *str, WORD wType)
{
	int len = strlennull(str) + 1;

	*(PWORD)(*buf + *bufpos) = wType;
	*(PWORD)(*buf + *bufpos + 2) = len + 2;
	*(PWORD)(*buf + *bufpos + 4) = len;
	memcpy(*buf + *bufpos + 6, str, len);
	*bufpos += len + 6;
}


void ppackLETLVLNTS(PBYTE *buf, int *buflen, const char *str, WORD wType, BYTE always)
{
	int len = strlennull(str) + 1;

	if (!always && len < 2) return;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 6 + *buflen + len);
	packLETLVLNTS(buf, buflen, str, wType);
}


void ppackLETLVWordLNTS(PBYTE *buf, int *buflen, WORD w, const char *str, WORD wType, BYTE always)
{
	int len = strlennull(str) + 1;

	if (!always && len < 2 && !w) return;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 8 + *buflen + len);
	*(PWORD)(*buf + *buflen) = wType;
	*(PWORD)(*buf + *buflen + 2) = len + 4;
	*(PWORD)(*buf + *buflen + 4) = w;
	*(PWORD)(*buf + *buflen + 6) = len;
	memcpy(*buf + *buflen + 8, str, len);
	*buflen += len + 8;
}


void ppackLETLVLNTSByte(PBYTE *buf, int *buflen, const char *str, BYTE b, WORD wType)
{
	int len = strlennull(str) + 1;

	*buf = (PBYTE)SAFE_REALLOC(*buf, 7 + *buflen + len);
	*(PWORD)(*buf + *buflen) = wType;
	*(PWORD)(*buf + *buflen + 2) = len + 3;
	*(PWORD)(*buf + *buflen + 4) = len;
	memcpy(*buf + *buflen + 6, str, len);
	*(*buf + *buflen + 6 + len) = b;
	*buflen += len + 7;
}


void CIcqProto::ppackLETLVLNTSfromDB(PBYTE *buf, int *buflen, const char *szSetting, WORD wType)
{
	char szTmp[1024];
	char *str = "";

	if (!getSettingStringStatic(NULL, szSetting, szTmp, sizeof(szTmp)))
		str = szTmp;

	ppackLETLVLNTS(buf, buflen, str, wType, 1);
}

void CIcqProto::ppackLETLVWordLNTSfromDB(PBYTE *buf, int *buflen, WORD w, const char *szSetting, WORD wType)
{
	char szTmp[1024];
	char *str = "";

	if (!getSettingStringStatic(NULL, szSetting, szTmp, sizeof(szTmp)))
		str = szTmp;

	ppackLETLVWordLNTS(buf, buflen, w, str, wType, 1);
}

void CIcqProto::ppackLETLVLNTSBytefromDB(PBYTE *buf, int *buflen, const char *szSetting, BYTE b, WORD wType)
{
	char szTmp[1024];
	char *str = "";

	if (!getSettingStringStatic(NULL, szSetting, szTmp, sizeof(szTmp)))
		str = szTmp;

	ppackLETLVLNTSByte(buf, buflen, str, b, wType);
}


void CIcqProto::ppackTLVStringFromDB(PBYTE *buf, int *buflen, const char *szSetting, WORD wType)
{
	char szTmp[1024];
	char *str = "";

	if (!getSettingStringStatic(NULL, szSetting, szTmp, sizeof(szTmp)))
		str = szTmp;

  ppackTLV(buf, buflen, wType, strlennull(str), (PBYTE)str);
}


void CIcqProto::ppackTLVStringUtfFromDB(PBYTE *buf, int *buflen, const char *szSetting, WORD wType)
{
	char *str = getSettingStringUtf(NULL, szSetting, NULL);

  ppackTLV(buf, buflen, wType, strlennull(str), (PBYTE)str);

  SAFE_FREE((void**)&str);
}


void CIcqProto::ppackTLVDateFromDB(PBYTE *buf, int *buflen, const char *szSettingYear, const char *szSettingMonth, const char *szSettingDay, WORD wType)
{
  SYSTEMTIME sTime = {0};
  double time = 0;

  sTime.wYear = getWord(szSettingYear, 0);
  sTime.wMonth = getByte(szSettingMonth, 0);
  sTime.wDay = getByte(szSettingDay, 0);
  if (sTime.wYear || sTime.wMonth || sTime.wDay)
  {
    SystemTimeToVariantTime(&sTime, &time);
    time -= 2;
  }

  ppackTLVDouble(buf, buflen, wType, time);
}


int CIcqProto::ppackTLVWordStringItemFromDB(PBYTE *buf, int *buflen, const char *szSetting, WORD wTypeID, WORD wTypeData, WORD wID)
{
  char szTmp[1024];
  char *str = NULL;

  if (!getSettingStringStatic(NULL, szSetting, szTmp, sizeof(szTmp)))
    str = szTmp;

  if (str)
  {
    WORD wLen = strlennull(str);

    ppackWord(buf, buflen, wLen + 0x0A);
	  ppackTLVWord(buf, buflen, wTypeID, wID);
    ppackTLV(buf, buflen, wTypeData, wLen, (PBYTE)str);

    return 1; // Success
  }

  return 0; // No data
}


int CIcqProto::ppackTLVWordStringUtfItemFromDB(PBYTE *buf, int *buflen, const char *szSetting, WORD wTypeID, WORD wTypeData, WORD wID)
{
  char *str = getSettingStringUtf(NULL, szSetting, NULL);

  if (str)
  {
    WORD wLen = strlennull(str);

    ppackWord(buf, buflen, wLen + 0x0A);
	  ppackTLVWord(buf, buflen, wTypeID, wID);
    ppackTLV(buf, buflen, wTypeData, wLen, (PBYTE)str);

    SAFE_FREE(&str);

    return 1; // Success
  }

  return 0; // No data
}


void ppackTLVBlockItems(PBYTE *buf, int *buflen, WORD wType, int *nItems, PBYTE *pBlock, WORD *wLength, BOOL bSingleItem)
{
  *buf = (PBYTE)SAFE_REALLOC(*buf, 8 + *buflen + *wLength);
  packWord(*buf + *buflen, wType);
  packWord(*buf + *buflen + 2, (bSingleItem ? 4 : 2) + *wLength);
  packWord(*buf + *buflen + 4, *nItems);
  if (bSingleItem)
    packWord(*buf + *buflen + 6, *wLength);
  if (*wLength)
    memcpy(*buf + *buflen + (bSingleItem ? 8 : 6), *pBlock, *wLength);
  *buflen += (bSingleItem ? 8 : 6) + *wLength;

  SAFE_FREE((void**)pBlock);
  *wLength = 0;
  *nItems = 0;
}


void ppackTLVBlockItem(PBYTE *buf, int *buflen, WORD wType, PBYTE *pItem, WORD *wLength)
{
  if (wLength)
  {
    *buf = (PBYTE)SAFE_REALLOC(*buf, 8 + *buflen + *wLength);
    packWord(*buf + *buflen, wType);
    packWord(*buf + *buflen + 2, 4 + *wLength);
    packWord(*buf + *buflen + 4, 1);
    packWord(*buf + *buflen + 6, *wLength);
    memcpy(*buf + *buflen + 8, *pItem, *wLength);
    *buflen += 8 + *wLength;
  }
  else
  {
    *buf = (PBYTE)SAFE_REALLOC(*buf, 6 + *buflen);
    packWord(*buf + *buflen, wType);
    packWord(*buf + *buflen + 2, 0x02);
    packWord(*buf + *buflen + 4, 0);
    *buflen += 6;
  }

  SAFE_FREE((void**)pItem);
  *wLength = 0;
}


void __fastcall unpackByte(BYTE **pSource, BYTE *byDestination)
{
	if (byDestination)
		*byDestination = *(*pSource)++;
	else
		*pSource += 1;
}

void __fastcall unpackWord(BYTE **pSource, WORD *wDestination)
{
	BYTE *tmp = *pSource;

	if (wDestination)
	{
		*wDestination  = *tmp++ << 8;
		*wDestination |= *tmp++;

		*pSource = tmp;
	}
	else
		*pSource += 2;
}

void __fastcall unpackDWord(BYTE **pSource, DWORD *dwDestination)
{
	BYTE *tmp = *pSource;

	if (dwDestination)
	{
		*dwDestination  = *tmp++ << 24;
		*dwDestination |= *tmp++ << 16;
		*dwDestination |= *tmp++ << 8;
		*dwDestination |= *tmp++;

		*pSource = tmp;
	}
	else
		*pSource += 4;
}

void __fastcall unpackQWord(BYTE **pSource, DWORD64 *qwDestination)
{
	DWORD dwData;

	if (qwDestination)
	{
		unpackDWord(pSource, &dwData);
		*qwDestination = ((DWORD64)dwData) << 32;
		unpackDWord(pSource, &dwData);
		*qwDestination |= dwData;
	}
	else
		*pSource += 8;
}

void __fastcall unpackLEWord(BYTE **buf, WORD *w)
{
	BYTE *tmp = *buf;

	if (w)
	{
		*w = (*tmp++);
		*w |= ((*tmp++) << 8);
	}
	else
		tmp += 2;

	*buf = tmp;
}

void __fastcall unpackLEDWord(BYTE **buf, DWORD *dw)
{
	BYTE *tmp = *buf;

	if (dw)
	{
		*dw = (*tmp++);
		*dw |= ((*tmp++) << 8);
		*dw |= ((*tmp++) << 16);
		*dw |= ((*tmp++) << 24);
	}
	else
		tmp += 4;

	*buf = tmp;
}

void unpackString(BYTE **buf, char *string, WORD len)
{
	BYTE *tmp = *buf;

	if (string)
	{
		while (len)  /* Can have 0x00 so go by len */
		{
			*string++ = *tmp++;
			len--;
		}
	}
	else
		tmp += len;

	*buf = tmp;
}

void unpackWideString(BYTE **buf, WCHAR *string, WORD len)
{
	BYTE *tmp = *buf;

	while (len > 1)
	{
		*string = (*tmp++ << 8);
		*string |= *tmp++;

		string++;
		len -= 2;
	}

	// We have a stray byte at the end, this means that the buffer had an odd length
	// which indicates an error.
	_ASSERTE(len == 0);
	if (len != 0)
	{
		// We dont copy the last byte but we still need to increase the buffer pointer
		// (we assume that 'len' was correct) since the calling function expects
		// that it is increased 'len' bytes.
		*tmp += len;
	}

	*buf = tmp;
}

void unpackTypedTLV(BYTE *buf, int buflen, WORD type, WORD *ttype, WORD *tlen, BYTE **ttlv)
{
	WORD wType, wLen;

NextTLV:
	// Unpack type and length
	unpackWord(&buf, &wType);
	unpackWord(&buf, &wLen);
	buflen -= 4;

	if (wType != type && buflen >= wLen + 4)
	{ // Not the right TLV, try next
		buflen -= wLen;
		buf += wLen;
		goto NextTLV;
	}
	// Check buffer size
	if (wLen > buflen) wLen = buflen;

	// Make sure we have a good pointer
	if (ttlv)
	{
		if (wLen)
		{ // Unpack and save value
			*ttlv = (BYTE*)SAFE_MALLOC(wLen + 1); // Add 1 for \0
			unpackString(&buf, (char*)*ttlv, wLen);
			*(*ttlv + wLen) = '\0';
		}
		else
			*ttlv = NULL;
	}

	// Save type and length
	if (ttype)
		*ttype = wType;
	if (tlen)
		*tlen = wLen;
}


BOOL CIcqProto::unpackUID(BYTE **ppBuf, WORD *pwLen, DWORD *pdwUIN, uid_str *ppszUID)
{
	BYTE nUIDLen;

	// sanity check
	if (!ppBuf || !pwLen || *pwLen < 1)
		return FALSE;

	// Sender UIN
	unpackByte(ppBuf, &nUIDLen);
	*pwLen -= 1;

	if ((nUIDLen > *pwLen) || (nUIDLen == 0))
		return FALSE;

	if (nUIDLen <= UINMAXLEN)
	{ // it can be uin, check
		char szUIN[UINMAXLEN+1];

		unpackString(ppBuf, szUIN, nUIDLen);
		szUIN[nUIDLen] = '\0';
		*pwLen -= nUIDLen;

		if (IsStringUIN(szUIN))
		{
			*pdwUIN = atoi(szUIN);
			return TRUE;
		}
		else
		{ // go back
			*ppBuf -= nUIDLen;
			*pwLen += nUIDLen;
		}
	}
	if (!m_bAimEnabled || !ppszUID || !(*ppszUID))
	{ // skip the UID data
		*ppBuf += nUIDLen;
		*pwLen -= nUIDLen;

		debugLogA("Malformed UIN in packet");
		return FALSE;
	}

	unpackString(ppBuf, *ppszUID, nUIDLen);
	*pwLen -= nUIDLen;
	(*ppszUID)[nUIDLen] = '\0';

	*pdwUIN = 0; // this is how we determine aim contacts internally

	return TRUE;
}
