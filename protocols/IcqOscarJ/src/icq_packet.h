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

#ifndef __ICQ_PACKET_H
#define __ICQ_PACKET_H

typedef unsigned char  BYTE;
typedef unsigned short  WORD;
typedef unsigned long  DWORD;

/*---------* Structures *--------------*/

struct icq_packet
{
  WORD wPlace;
  BYTE nChannel;
  WORD wLen;
  BYTE *pData;
};

/*---------* Functions *---------------*/

WORD generate_flap_sequence();

void __fastcall init_generic_packet(icq_packet *pPacket, WORD wHeaderLen);
void write_httphdr(icq_packet *pPacket, WORD wType, DWORD dwSeq);
void __fastcall write_flap(icq_packet *pPacket, BYTE byFlapChannel);
void __fastcall serverPacketInit(icq_packet *pPacket, WORD wSize);
void __fastcall directPacketInit(icq_packet *pPacket, DWORD dwSize);

void __fastcall serverCookieInit(icq_packet *pPacket, BYTE *pCookie, WORD wCookieSize);

void __fastcall packByte(icq_packet *pPacket, BYTE byValue);
void __fastcall packWord(icq_packet *pPacket, WORD wValue);
void __fastcall packDWord(icq_packet *pPacket, DWORD dwValue);
void __fastcall packQWord(icq_packet *pPacket, DWORD64 qwValue);
void packTLV(icq_packet *pPacket, WORD wType, WORD wLength, const BYTE *pbyValue);
void packTLVWord(icq_packet *pPacket, WORD wType, WORD wData);
void packTLVDWord(icq_packet *pPacket, WORD wType, DWORD dwData);
void packTLVUID(icq_packet *pPacket, WORD wType, DWORD dwUin, const char *szUid);

void packBuffer(icq_packet *pPacket, const BYTE *pbyBuffer, WORD wLength);
//void packLEWordSizedBuffer(icq_packet* pPacket, const BYTE* pbyBuffer, WORD wLength);
int __fastcall getUINLen(DWORD dwUin);
int __fastcall getUIDLen(DWORD dwUin, const char *szUid);
void __fastcall packUIN(icq_packet *pPacket, DWORD dwUin);
void __fastcall packUID(icq_packet *pPacket, DWORD dwUin, const char *szUid);

void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype);
void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype, WORD wFlags, DWORD dwSequence);
void packFNACHeader(icq_packet *pPacket, WORD wFamily, WORD wSubtype, WORD wFlags, DWORD dwSequence, WORD wVersion);

void __fastcall packLEWord(icq_packet *pPacket, WORD wValue);
void __fastcall packLEDWord(icq_packet *pPacket, DWORD dwValue);

void packLETLVLNTS(PBYTE *buf, int *bufpos, const char *str, WORD wType);

void ppackByte(PBYTE *buf, int *buflen, BYTE byValue);
void ppackWord(PBYTE *buf, int *buflen, WORD wValue);
void ppackLEWord(PBYTE *buf, int *buflen, WORD wValue);
void ppackLEDWord(PBYTE *buf, int *buflen, DWORD dwValue);
void ppackLELNTS(PBYTE *buf, int *buflen, const char *str);
void ppackBuffer(PBYTE *buf, int *buflen, WORD wLength, const BYTE *pbyValue);

void ppackTLV(PBYTE *buf, int *buflen, WORD wType, WORD wLength, const BYTE *pbyValue);
void ppackTLVByte(PBYTE *buf, int *buflen, WORD wType, BYTE byValue);
void ppackTLVWord(PBYTE *buf, int *buflen, WORD wType, WORD wValue);
void ppackTLVDWord(PBYTE *buf, int *buflen, WORD wType, DWORD dwValue);
void ppackTLVDouble(PBYTE *buf, int *buflen, WORD wType, double dValue);
void ppackTLVUID(PBYTE *buf, int *buflen, WORD wType, DWORD dwUin, const char *szUid);

void ppackLETLVByte(PBYTE *buf, int *buflen, BYTE byValue, WORD wType, BYTE always);
void ppackLETLVWord(PBYTE *buf, int *buflen, WORD wValue, WORD wType, BYTE always);
void ppackLETLVDWord(PBYTE *buf, int *buflen, DWORD dwValue, WORD wType, BYTE always);
void ppackLETLVLNTS(PBYTE *buf, int *buflen, const char *str, WORD wType, BYTE always);
void ppackLETLVWordLNTS(PBYTE *buf, int *buflen, WORD w, const char *str, WORD wType, BYTE always);
void ppackLETLVLNTSByte(PBYTE *buf, int *buflen, const char *str, BYTE b, WORD wType);

void ppackTLVBlockItems(PBYTE *buf, int *buflen, WORD wType, int *nItems, PBYTE *pBlock, WORD *wLength, BOOL bSingleItem);
void ppackTLVBlockItem(PBYTE *buf, int *buflen, WORD wType, PBYTE *pItem, WORD *wLength);

void __fastcall unpackByte(BYTE **pSource, BYTE *byDestination);
void __fastcall unpackWord(BYTE **pSource, WORD *wDestination);
void __fastcall unpackDWord(BYTE **pSource, DWORD *dwDestination);
void __fastcall unpackQWord(BYTE **pSource, DWORD64 *qwDestination);
void unpackString(BYTE **buf, char *string, WORD len);
void unpackWideString(BYTE **buf, WCHAR *string, WORD len);
void unpackTypedTLV(BYTE *buf, int buflen, WORD type, WORD *ttype, WORD *tlen, BYTE **ttlv);
BOOL unpackUID(BYTE **ppBuf, WORD *pwLen, DWORD *pdwUIN, uid_str *ppszUID);

void __fastcall unpackLEWord(BYTE **buf, WORD *w);
void __fastcall unpackLEDWord(BYTE **buf, DWORD *dw);

#endif /* __ICQ_PACKET_H */
