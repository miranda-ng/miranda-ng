// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
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

void EncryptDirectPacket(directconnect* dc, icq_packet* p);

void packEmptyMsg(icq_packet *packet);

static void packDirectMsgHeader(icq_packet* packet, size_t wDataLen, WORD wCommand, DWORD dwCookie, BYTE bMsgType, BYTE bMsgFlags, WORD wX1, WORD wX2)
{
	directPacketInit(packet, 29 + wDataLen);
	packByte(packet, 2);        /* channel */
	packLEDWord(packet, 0);     /* space for crypto */
	packLEWord(packet, wCommand);
	packLEWord(packet, 14);      /* unknown */
	packLEWord(packet, (WORD)dwCookie);
	packLEDWord(packet, 0);      /* unknown */
	packLEDWord(packet, 0);      /* unknown */
	packLEDWord(packet, 0);      /* unknown */
	packByte(packet, bMsgType);
	packByte(packet, bMsgFlags);
	packLEWord(packet, wX1);    /* unknown. Is 1 for getawaymsg, 0 otherwise */
	packLEWord(packet, wX2); // this is probably priority
}


void CIcqProto::icq_sendDirectMsgAck(directconnect* dc, WORD wCookie, BYTE bMsgType, BYTE bMsgFlags, char* szCap)
{
	icq_packet packet;
	packDirectMsgHeader(&packet, bMsgType == MTYPE_PLAIN ? (szCap ? 53 : 11) : 3, DIRECT_ACK, wCookie, bMsgType, bMsgFlags, 0, 0);
	packEmptyMsg(&packet);   /* empty message */

	if (bMsgType == MTYPE_PLAIN) {
		packMsgColorInfo(&packet);

		if (szCap) {
			packLEDWord(&packet, 0x26);     /* CLSID length */
			packBuffer(&packet, (LPBYTE)szCap, 0x26); /* GUID */
		}
	}
	EncryptDirectPacket(dc, &packet);
	sendDirectPacket(dc, &packet);

	NetLog_Direct("Sent acknowledgement thru direct connection");
}


DWORD CIcqProto::icq_sendGetAwayMsgDirect(MCONTACT hContact, int type)
{
	if (getWord(hContact, "Version", 0) >= 9)
		return 0; // v9 DC protocol does not support this message

	cookie_message_data *pCookieData = CreateMessageCookie(MTYPE_AUTOAWAY, (BYTE)type);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	icq_packet packet;
	packDirectMsgHeader(&packet, 3, DIRECT_MESSAGE, dwCookie, (BYTE)type, 3, 1, 0);
	packEmptyMsg(&packet);  // message

	return (SendDirectMessage(hContact, &packet)) ? dwCookie : 0;
}


void CIcqProto::icq_sendAwayMsgReplyDirect(directconnect* dc, WORD wCookie, BYTE msgType, const char** szMsg)
{
	if (validateStatusMessageRequest(dc->hContact, msgType)) {
		NotifyEventHooks(m_modeMsgsEvent, (WPARAM)msgType, (LPARAM)dc->dwRemoteUin);

		icq_lock l(m_modeMsgsMutex);

		if (szMsg && *szMsg) {
			// prepare Ansi message - only Ansi supported
			size_t wMsgLen = mir_strlen(*szMsg) + 1;
			char *szAnsiMsg = (char*)_alloca(wMsgLen);

			utf8_decode_static(*szMsg, szAnsiMsg, wMsgLen);
			wMsgLen = mir_strlen(szAnsiMsg);

			icq_packet packet;
			packDirectMsgHeader(&packet, 3 + wMsgLen, DIRECT_ACK, wCookie, msgType, 3, 0, 0);
			packLEWord(&packet, WORD(wMsgLen + 1));
			packBuffer(&packet, (LPBYTE)szAnsiMsg, wMsgLen + 1);
			EncryptDirectPacket(dc, &packet);

			sendDirectPacket(dc, &packet);
		}
	}
}


void CIcqProto::icq_sendFileAcceptDirect(MCONTACT hContact, filetransfer* ft)
{
	// v7 packet
	icq_packet packet;
	packDirectMsgHeader(&packet, 18, DIRECT_ACK, ft->dwCookie, MTYPE_FILEREQ, 0, 0, 0);
	packLEWord(&packet, 1);    // description
	packByte(&packet, 0);
	packWord(&packet, wListenPort);
	packLEWord(&packet, 0);
	packLEWord(&packet, 1);    // filename
	packByte(&packet, 0);     // TODO: really send filename
	packLEDWord(&packet, ft->dwTotalSize);  // file size 
	packLEDWord(&packet, wListenPort);    // FIXME: ideally we want to open a new port for this

	SendDirectMessage(hContact, &packet);

	NetLog_Direct("Sent file accept direct, port %u", wListenPort);
}


void CIcqProto::icq_sendFileDenyDirect(MCONTACT hContact, filetransfer *ft, const char *szReason)
{
	// v7 packet
	icq_packet packet;
	char *szReasonAnsi = NULL;
	if (!utf8_decode(szReason, &szReasonAnsi))
		szReasonAnsi = _strdup(szReason);		// Legacy fix
	size_t cbReasonAnsi = mir_strlen(szReasonAnsi);

	packDirectMsgHeader(&packet, 18 + cbReasonAnsi, DIRECT_ACK, ft->dwCookie, MTYPE_FILEREQ, 0, 1, 0);
	packLEWord(&packet, WORD(1 + cbReasonAnsi));  // description
	if (szReasonAnsi)
		packBuffer(&packet, (LPBYTE)szReasonAnsi, cbReasonAnsi);
	packByte(&packet, 0);
	packWord(&packet, 0);
	packLEWord(&packet, 0);
	packLEWord(&packet, 1);   // filename
	packByte(&packet, 0);     // TODO: really send filename
	packLEDWord(&packet, 0);  // file size 
	packLEDWord(&packet, 0);

	SAFE_FREE(&szReasonAnsi);

	SendDirectMessage(hContact, &packet);

	NetLog_Direct("Sent file deny direct.");
}


int CIcqProto::icq_sendFileSendDirectv7(filetransfer *ft, const char *pszFiles)
{
	char *szFilesAnsi = NULL;
	size_t wDescrLen = mir_strlen(ft->szDescription);

	if (!utf8_decode(pszFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(pszFiles);		// Legacy fix
	size_t wFilesLen = mir_strlen(szFilesAnsi);

	icq_packet packet;
	packDirectMsgHeader(&packet, 18 + wDescrLen + wFilesLen, DIRECT_MESSAGE, (WORD)ft->dwCookie, MTYPE_FILEREQ, 0, 0, 0);
	packLEWord(&packet, WORD(wDescrLen + 1));
	packBuffer(&packet, (LPBYTE)ft->szDescription, wDescrLen + 1);
	packLEDWord(&packet, 0);   // listen port
	packLEWord(&packet, WORD(wFilesLen + 1));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, wFilesLen + 1);
	packLEDWord(&packet, ft->dwTotalSize);
	packLEDWord(&packet, 0);    // listen port (again)

	SAFE_FREE(&szFilesAnsi);

	NetLog_Direct("Sending v%u file transfer request direct", 7);

	return SendDirectMessage(ft->hContact, &packet);
}


int CIcqProto::icq_sendFileSendDirectv8(filetransfer *ft, const char *pszFiles)
{
	char *szFilesAnsi = NULL;
	size_t wDescrLen = mir_strlen(ft->szDescription);

	if (!utf8_decode(pszFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(pszFiles);		// Legacy fix
	size_t wFilesLen = mir_strlen(szFilesAnsi)+1;

	icq_packet packet;
	packDirectMsgHeader(&packet, 0x2E + 22 + wDescrLen + wFilesLen, DIRECT_MESSAGE, (WORD)ft->dwCookie, MTYPE_PLUGIN, 0, 0, 0);
	packEmptyMsg(&packet);  // message
	packPluginTypeId(&packet, MTYPE_FILEREQ);

	packLEDWord(&packet, WORD(18 + wDescrLen + wFilesLen)); // Remaining length
	packLEDWord(&packet, DWORD(wDescrLen));          // Description
	packBuffer(&packet, (LPBYTE)ft->szDescription, wDescrLen);
	packWord(&packet, 0x8c82); // Unknown (port?), seen 0x80F6
	packWord(&packet, 0x0222); // Unknown, seen 0x2e01
	packLEWord(&packet, WORD(wFilesLen));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, wFilesLen);
	packLEDWord(&packet, ft->dwTotalSize);
	packLEDWord(&packet, 0x0008c82); // Unknown, (seen 0xf680 ~33000)

	SAFE_FREE(&szFilesAnsi);

	NetLog_Direct("Sending v%u file transfer request direct", 8);

	return SendDirectMessage(ft->hContact, &packet);
}


DWORD CIcqProto::icq_SendDirectMessage(MCONTACT hContact, const char *szMessage, size_t nBodyLength, WORD wPriority, cookie_message_data *pCookieData, char *szCap)
{
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	// Pack the standard header
	icq_packet packet;
	packDirectMsgHeader(&packet, nBodyLength + (szCap ? 53 : 11), DIRECT_MESSAGE, dwCookie, (BYTE)pCookieData->bMessageType, 0, 0, 0);

	packLEWord(&packet, WORD(nBodyLength+1));            // Length of message
	packBuffer(&packet, (LPBYTE)szMessage, nBodyLength+1); // Message
	packMsgColorInfo(&packet);
	if (szCap)
	{
		packLEDWord(&packet, 0x00000026);                    // length of GUID
		packBuffer(&packet, (LPBYTE)szCap, 0x26);                    // UTF-8 GUID
	}

	if (SendDirectMessage(hContact, &packet))
		return dwCookie; // Success
	
	FreeCookie(dwCookie); // release cookie
	return 0; // Failure
}

void CIcqProto::icq_sendXtrazRequestDirect(MCONTACT hContact, DWORD dwCookie, char* szBody, size_t nBodyLen, WORD wType)
{
	icq_packet packet;
	packDirectMsgHeader(&packet, 11 + getPluginTypeIdLen(wType) + nBodyLen, DIRECT_MESSAGE, dwCookie, MTYPE_PLUGIN, 0, 0, 1);
	packEmptyMsg(&packet);  // message (unused)
	packPluginTypeId(&packet, wType);

	packLEDWord(&packet, DWORD(nBodyLen + 4));
	packLEDWord(&packet, DWORD(nBodyLen));
	packBuffer(&packet, (LPBYTE)szBody, nBodyLen);

	SendDirectMessage(hContact, &packet);
}

void CIcqProto::icq_sendXtrazResponseDirect(MCONTACT hContact, WORD wCookie, char* szBody, size_t nBodyLen, WORD wType)
{
	icq_packet packet;
	packDirectMsgHeader(&packet, getPluginTypeIdLen(wType) + 11 + nBodyLen, DIRECT_ACK, wCookie, MTYPE_PLUGIN, 0, 0, 0);

	packEmptyMsg(&packet);  // Message (unused)

	packPluginTypeId(&packet, wType);

	packLEDWord(&packet, DWORD(nBodyLen + 4));
	packLEDWord(&packet, DWORD(nBodyLen));
	packBuffer(&packet, (LPBYTE)szBody, nBodyLen);

	SendDirectMessage(hContact, &packet);
}
