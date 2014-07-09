// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
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

extern const int moodXStatus[];

/*****************************************************************************
*
*   Some handy extra pack functions for basic message type headers
*
*/

// This is the part of the message header that is common for all message channels
static void packServMsgSendHeader(icq_packet *p, DWORD dwSequence, DWORD dwID1, DWORD dwID2, DWORD dwUin, const char *szUID, WORD wFmt, WORD wLen)
{
	serverPacketInit(p, (WORD)(21 + getUIDLen(dwUin, szUID) + wLen));
	packFNACHeader(p, ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, 0, dwSequence | ICQ_MSG_SRV_SEND<<0x10);
	packLEDWord(p, dwID1);         // Msg ID part 1
	packLEDWord(p, dwID2);         // Msg ID part 2
	packWord(p, wFmt);             // Message channel
	packUID(p, dwUin, szUID);      // User ID
}


static void packServIcqExtensionHeader(icq_packet *p, CIcqProto *ppro, WORD wLen, WORD wType, WORD wSeq, WORD wCmd = ICQ_META_CLI_REQUEST)
{
	serverPacketInit(p, (WORD)(24 + wLen));
	packFNACHeader(p, ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQUEST, 0, wSeq | (wCmd << 0x10));
	packWord(p, 0x01);                // TLV type 1
	packWord(p, (WORD)(10 + wLen));   // TLV len
	packLEWord(p, (WORD)(8 + wLen));  // Data chunk size (TLV.Length-2)
	packLEDWord(p, ppro->m_dwLocalUIN); // My UIN
	packLEWord(p, wType);             // Request type
	packWord(p, wSeq);
}


static void packServIcqDirectoryHeader(icq_packet *p, CIcqProto *ppro, WORD wLen, WORD wType, WORD wCommand, WORD wSeq, WORD wSubCommand = ICQ_META_CLI_REQUEST)
{
	packServIcqExtensionHeader(p, ppro, wLen + 0x1E, CLI_META_INFO_REQ, wSeq, wSubCommand);
	packLEWord(p, wType);
	packLEWord(p, wLen + 0x1A);
	packFNACHeader(p, 0x5b9, wCommand, 0, 0, 2);
	packWord(p, 0);
	packWord(p, (WORD)GetACP());
	packDWord(p, 2);
}


static void packServTLV5HeaderBasic(icq_packet *p, WORD wLen, DWORD ID1, DWORD ID2, WORD wCommand, const plugin_guid pGuid)
{
	// TLV(5) header
	packWord(p, 0x05);                // Type
	packWord(p, (WORD)(26 + wLen));   // Len
	// TLV(5) data
	packWord(p, wCommand);            // Command
	packLEDWord(p, ID1);              // msgid1
	packLEDWord(p, ID2);              // msgid2
	packGUID(p, pGuid);               // capabilities (4 dwords)
}


static void packServTLV5HeaderMsg(icq_packet *p, WORD wLen, DWORD ID1, DWORD ID2, WORD wAckType)
{
	packServTLV5HeaderBasic(p, (WORD)(wLen + 10), ID1, ID2, 0, MCAP_SRV_RELAY_FMT);

	packTLVWord(p, 0x0A, wAckType);   // TLV: 0x0A Acktype: 1 for normal, 2 for ack
	packDWord(p, 0x000F0000);         // TLV: 0x0F empty
}


static void packServTLV2711Header(icq_packet *packet, WORD wCookie, WORD wVersion, BYTE bMsgType, BYTE bMsgFlags, WORD X1, WORD X2, int nLen)
{
	packWord(packet, 0x2711);            // Type
	packWord(packet, (WORD)(51 + nLen)); // Len
	// TLV(0x2711) data
	packLEWord(packet, 0x1B);            // Unknown
	packByte(packet, (BYTE)wVersion);    // Client (message) version
	packGUID(packet, PSIG_MESSAGE);
	packDWord(packet, CLIENTFEATURES);
	packDWord(packet, DC_TYPE);
	packLEWord(packet, wCookie);         // Reference cookie
	packLEWord(packet, 0x0E);            // Unknown
	packLEWord(packet, wCookie);         // Reference cookie again
	packDWord(packet, 0);                // Unknown (12 bytes)
	packDWord(packet, 0);                //  -
	packDWord(packet, 0);                //  -
	packByte(packet, bMsgType);          // Message type
	packByte(packet, bMsgFlags);         // Flags
	packLEWord(packet, X1);              // Accepted
	packWord(packet, X2);                // Unknown, priority?
}


static void packServDCInfo(icq_packet *p, CIcqProto* ppro, BOOL bEmpty)
{
	packTLVDWord(p, 0x03, bEmpty ? 0 : ppro->getDword("RealIP", 0)); // TLV: 0x03 DWORD IP
	packTLVWord(p, 0x05, (WORD)(bEmpty ? 0 : ppro->wListenPort));                 // TLV: 0x05 Listen port
}


static void packServChannel2Header(icq_packet *p, CIcqProto* ppro, DWORD dwUin, WORD wLen, DWORD dwID1, DWORD dwID2, DWORD dwCookie, WORD wVersion, BYTE bMsgType, BYTE bMsgFlags, WORD wPriority, int isAck, int includeDcInfo, BYTE bRequestServerAck)
{
	packServMsgSendHeader(p, dwCookie, dwID1, dwID2, dwUin, NULL, 0x0002, (WORD)(wLen + 95 + (bRequestServerAck ? 4 : 0) + (includeDcInfo ? 14 : 0)));

	packWord(p, 0x05);            // TLV type
	packWord(p, (WORD)(wLen + 91 + (includeDcInfo ? 14 : 0)));  /* TLV len */
	packWord(p, (WORD)(isAck ? 2 : 0));     /* not aborting anything */
	packLEDWord(p, dwID1);        // Msg ID part 1
	packLEDWord(p, dwID2);        // Msg ID part 2
	packGUID(p, MCAP_SRV_RELAY_FMT); /* capability (4 dwords) */
	packDWord(p, 0x000A0002);     // TLV: 0x0A WORD: 1 for normal, 2 for ack
	packWord(p, (WORD)(isAck ? 2 : 1));

	if (includeDcInfo)
		packServDCInfo(p, ppro, FALSE);

	packDWord(p, 0x000F0000);     // TLV: 0x0F empty

	packServTLV2711Header(p, (WORD)dwCookie, wVersion, bMsgType, bMsgFlags, (WORD)MirandaStatusToIcq(ppro->m_iStatus), wPriority, wLen);
}


static void packServAdvancedReply(icq_packet *p, DWORD dwUin, const char *szUid, DWORD dwID1, DWORD dwID2, WORD wCookie, WORD wLen)
{
	serverPacketInit(p, (WORD)(getUIDLen(dwUin, szUid) + 23 + wLen));
	packFNACHeader(p, ICQ_MSG_FAMILY, ICQ_MSG_RESPONSE, 0, ICQ_MSG_RESPONSE << 0x10 | (wCookie & 0x7FFF));
	packLEDWord(p, dwID1);        // Msg ID part 1
	packLEDWord(p, dwID2);        // Msg ID part 2
	packWord(p, 0x02);            // Channel
	packUID(p, dwUin, szUid);     // Contact UID
	packWord(p, 0x03);            // Msg specific formating
}


static void packServAdvancedMsgReply(icq_packet *p, DWORD dwUin, const char *szUid, DWORD dwID1, DWORD dwID2, WORD wCookie, WORD wVersion, BYTE bMsgType, BYTE bMsgFlags, WORD wLen)
{
	packServAdvancedReply(p, dwUin, szUid, dwID1, dwID2, wCookie, (WORD)(wLen + 51));

	packLEWord(p, 0x1B);          // Unknown
	packByte(p, (BYTE)wVersion);  // Protocol version
	packGUID(p, PSIG_MESSAGE);
	packDWord(p, CLIENTFEATURES);
	packDWord(p, DC_TYPE);
	packLEWord(p, wCookie);  // Reference
	packLEWord(p, 0x0E);     // Unknown
	packLEWord(p, wCookie);  // Reference
	packDWord(p, 0);         // Unknown
	packDWord(p, 0);         // Unknown
	packDWord(p, 0);         // Unknown
	packByte(p, bMsgType);   // Message type
	packByte(p, bMsgFlags);  // Message flags
	packLEWord(p, 0);        // Ack status code ( 0 = accepted, this is hardcoded because
	//                   it is only used this way yet)
	packLEWord(p, 0);        // Unused priority field
}


void packMsgColorInfo(icq_packet *packet)
{ // TODO: make configurable
	packLEDWord(packet, 0x00000000);   // Foreground colour
	packLEDWord(packet, 0x00FFFFFF);   // Background colour
}


void packEmptyMsg(icq_packet *packet)
{
	packLEWord(packet, 1);
	packByte(packet, 0);
}

/*****************************************************************************
*
*   Functions to actually send the stuff
*
*/

void CIcqProto::icq_sendCloseConnection()
{
	icq_packet packet;

	packet.wLen = 0;
	write_flap(&packet, ICQ_CLOSE_CHAN);
	sendServPacket(&packet);
}


void CIcqProto::icq_requestnewfamily(WORD wFamily, void (CIcqProto::*familyhandler)(HANDLE hConn, char* cookie, WORD cookieLen))
{
	icq_packet packet;
	cookie_family_request *request;
	int bRequestSSL = m_bSecureConnection && (wFamily != ICQ_AVATAR_FAMILY); // Avatar servers does not support SSL

	request = (cookie_family_request*)SAFE_MALLOC(sizeof(cookie_family_request));
	request->wFamily = wFamily;
	request->familyHandler = familyhandler;

	DWORD dwCookie = AllocateCookie(CKT_SERVICEREQUEST, ICQ_CLIENT_NEW_SERVICE, 0, request); // generate and alloc cookie

	serverPacketInit(&packet, 12 + (bRequestSSL ? 4 : 0));
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_NEW_SERVICE, 0, dwCookie);
	packWord(&packet, wFamily);
	if (bRequestSSL)
		packDWord(&packet, 0x008C0000); // use SSL

	sendServPacket(&packet);
}


void CIcqProto::icq_setidle(int bAllow)
{
	icq_packet packet;

	if (bAllow != m_bIdleAllow) {
		/* SNAC 1,11 */
		serverPacketInit(&packet, 14);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_IDLE);
		if (bAllow == 1)
			packDWord(&packet, 0x0000003C);
		else
			packDWord(&packet, 0x00000000);

		m_bIdleAllow = bAllow;
		sendServPacket(&packet);
	}
}


void CIcqProto::icq_setstatus(WORD wStatus, const char *szStatusNote)
{
	icq_packet packet;
	char *szCurrentStatusNote = szStatusNote ? getSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, NULL) : NULL;
	WORD wStatusMoodLen = 0, wStatusNoteLen = 0, wSessionDataLen = 0;
	char *szMoodData = NULL;

	if (szStatusNote && strcmpnull(szCurrentStatusNote, szStatusNote)) { // status note was changed, update now
		DBVARIANT dbv = { DBVT_DELETED };

		if (m_bMoodsEnabled && !getString(DBSETTING_STATUS_MOOD, &dbv))
			szMoodData = null_strdup(dbv.pszVal);

		db_free(&dbv);

		wStatusNoteLen = strlennull(szStatusNote);
		wStatusMoodLen = strlennull(szMoodData);

		wSessionDataLen = (wStatusNoteLen ? wStatusNoteLen + 4 : 0) + 4 + wStatusMoodLen + 4;
	}
	SAFE_FREE(&szCurrentStatusNote);

	// Pack data in packet
	serverPacketInit(&packet, (WORD)(18 + (wSessionDataLen ? wSessionDataLen + 4 : 0)));
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
	packWord(&packet, 0x06);                // TLV 6
	packWord(&packet, 0x04);                // TLV length
	packWord(&packet, GetMyStatusFlags());  // Status flags
	packWord(&packet, wStatus);             // Status
	if (wSessionDataLen) { // Pack session data
		packWord(&packet, 0x1D);              // TLV 1D
		packWord(&packet, wSessionDataLen);   // TLV length
		packWord(&packet, 0x02);              // Item Type
		if (wStatusNoteLen) {
			packWord(&packet, 0x400 | (WORD)(wStatusNoteLen + 4)); // Flags + Item Length
			packWord(&packet, wStatusNoteLen);  // Text Length
			packBuffer(&packet, (LPBYTE)szStatusNote, wStatusNoteLen);
			packWord(&packet, 0);               // Encoding not specified (utf-8 is default)
		}
		else
			packWord(&packet, 0);               // Flags + Item Length
		packWord(&packet, 0x0E);              // Item Type
		packWord(&packet, wStatusMoodLen);    // Flags + Item Length
		if (wStatusMoodLen)
			packBuffer(&packet, (LPBYTE)szMoodData, wStatusMoodLen); // Mood

		// Save current status note
		db_set_utf(NULL, m_szModuleName, DBSETTING_STATUS_NOTE, szStatusNote);
	}
	// Release memory
	SAFE_FREE(&szMoodData);

	// Send packet
	sendServPacket(&packet);
}


DWORD CIcqProto::icq_SendChannel1Message(DWORD dwUin, char *szUID, MCONTACT hContact, char *pszText, cookie_message_data *pCookieData)
{
	icq_packet packet;
	WORD wPacketLength;

	WORD wMessageLen = strlennull(pszText);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	if (pCookieData->nAckType == ACKTYPE_SERVER)
		wPacketLength = 25;
	else
		wPacketLength = 21;

	// Pack the standard header
	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, szUID, 1, (WORD)(wPacketLength + wMessageLen));

	// Pack first TLV
	packWord(&packet, 0x0002); // TLV(2)
	packWord(&packet, (WORD)(wMessageLen + 13)); // TLV len

	// Pack client features
	packWord(&packet, 0x0501); // TLV(501)
	packWord(&packet, 0x0001); // TLV len
	packByte(&packet, 0x1);    // Features, meaning unknown, duplicated from ICQ Lite

	// Pack text TLV
	packWord(&packet, 0x0101); // TLV(2)
	packWord(&packet, (WORD)(wMessageLen + 4)); // TLV len
	packWord(&packet, 0x0003); // Message charset number, again copied from ICQ Lite
	packWord(&packet, 0x0000); // Message charset subset
	packBuffer(&packet, (LPBYTE)pszText, (WORD)(wMessageLen)); // Message text

	// Pack request server ack TLV
	if (pCookieData->nAckType == ACKTYPE_SERVER)
		packDWord(&packet, 0x00030000); // TLV(3)

	// Pack store on server TLV
	packDWord(&packet, 0x00060000); // TLV(6)

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_SendChannel1MessageW(DWORD dwUin, char *szUID, MCONTACT hContact, WCHAR *pszText, cookie_message_data *pCookieData)
{
	icq_packet packet;
	WORD wMessageLen;
	DWORD dwCookie;
	WORD wPacketLength;
	WCHAR *ppText;
	int i;

	wMessageLen = strlennull(pszText) * (int)sizeof(WCHAR);
	dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	if (pCookieData->nAckType == ACKTYPE_SERVER)
		wPacketLength = 26;
	else
		wPacketLength = 22;

	// Pack the standard header
	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, szUID, 1, (WORD)(wPacketLength + wMessageLen));

	// Pack first TLV
	packWord(&packet, 0x0002); // TLV(2)
	packWord(&packet, (WORD)(wMessageLen + 14)); // TLV len

	// Pack client features
	packWord(&packet, 0x0501); // TLV(501)
	packWord(&packet, 0x0002); // TLV len
	packWord(&packet, 0x0106); // Features, meaning unknown, duplicated from ICQ 2003b

	// Pack text TLV
	packWord(&packet, 0x0101); // TLV(2)
	packWord(&packet, (WORD)(wMessageLen + 4)); // TLV len
	packWord(&packet, 0x0002); // Message charset number, again copied from ICQ 2003b
	packWord(&packet, 0x0000); // Message charset subset
	ppText = pszText;   // we must convert the widestring
	for (i = 0; i < wMessageLen; i += 2, ppText++)
		packWord(&packet, *ppText);

	// Pack request server ack TLV
	if (pCookieData->nAckType == ACKTYPE_SERVER)
		packDWord(&packet, 0x00030000); // TLV(3)

	// Pack store on server TLV
	packDWord(&packet, 0x00060000); // TLV(6)

	sendServPacket(&packet);
	return dwCookie;
}


DWORD CIcqProto::icq_SendChannel2Message(DWORD dwUin, MCONTACT hContact, const char *szMessage, int nBodyLen, WORD wPriority, cookie_message_data *pCookieData, char *szCap)
{
	icq_packet packet;

	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	// Pack the standard header
	packServChannel2Header(&packet, this, dwUin, (WORD)(nBodyLen + (szCap ? 53 : 11)), pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwCookie, ICQ_VERSION, (BYTE)pCookieData->bMessageType, 0,
		wPriority, 0, 0, (BYTE)((pCookieData->nAckType == ACKTYPE_SERVER) ? 1 : 0));

	packLEWord(&packet, (WORD)(nBodyLen + 1));            // Length of message
	packBuffer(&packet, (LPBYTE)szMessage, (WORD)(nBodyLen + 1)); // Message
	packMsgColorInfo(&packet);

	if (szCap) {
		packLEDWord(&packet, 0x00000026);                 // length of GUID
		packBuffer(&packet, (LPBYTE)szCap, 0x26);                 // UTF-8 GUID
	}

	// Pack request server ack TLV
	if (pCookieData->nAckType == ACKTYPE_SERVER)
		packDWord(&packet, 0x00030000); // TLV(3)

	sendServPacket(&packet);
	return dwCookie;
}


DWORD CIcqProto::icq_SendChannel2Contacts(DWORD dwUin, char *szUid, MCONTACT hContact, const char *pData, WORD wDataLen, const char *pNames, WORD wNamesLen, cookie_message_data *pCookieData)
{
	icq_packet packet;

	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, pCookieData);

	WORD wPacketLength = wDataLen + wNamesLen + 0x12;

	// Pack the standard header
	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, szUid, 2, (WORD)(wPacketLength + ((pCookieData->nAckType == ACKTYPE_SERVER) ? 0x22 : 0x1E)));

	packServTLV5HeaderBasic(&packet, wPacketLength, pCookieData->dwMsgID1, pCookieData->dwMsgID2, 0, MCAP_CONTACTS);

	packTLVWord(&packet, 0x0A, 1);              // TLV: 0x0A Acktype: 1 for normal, 2 for ack
	packDWord(&packet, 0x000F0000);             // TLV: 0x0F empty
	packTLV(&packet, 0x2711, wDataLen, (LPBYTE)pData);  // TLV: 0x2711 Content (Contact UIDs)
	packTLV(&packet, 0x2712, wNamesLen, (LPBYTE)pNames);// TLV: 0x2712 Extended Content (Contact NickNames)

	// Pack request ack TLV
	if (pCookieData->nAckType == ACKTYPE_SERVER) {
		packDWord(&packet, 0x00030000); // TLV(3)
	}

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_SendChannel4Message(DWORD dwUin, MCONTACT hContact, BYTE bMsgType, WORD wMsgLen, const char *szMsg, cookie_message_data *pCookieData)
{
	icq_packet packet;
	WORD wPacketLength;
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	if (pCookieData->nAckType == ACKTYPE_SERVER)
		wPacketLength = 28;
	else
		wPacketLength = 24;

	// Pack the standard header
	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, NULL, 4, (WORD)(wPacketLength + wMsgLen));

	// Pack first TLV
	packWord(&packet, 0x05);                 // TLV(5)
	packWord(&packet, (WORD)(wMsgLen + 16)); // TLV len
	packLEDWord(&packet, m_dwLocalUIN);        // My UIN
	packByte(&packet, bMsgType);             // Message type
	packByte(&packet, 0);                    // Message flags
	packLEWord(&packet, wMsgLen);            // Message length
	packBuffer(&packet, (LPBYTE)szMsg, wMsgLen);     // Message text
	packMsgColorInfo(&packet);

	// Pack request ack TLV
	if (pCookieData->nAckType == ACKTYPE_SERVER) {
		packDWord(&packet, 0x00030000); // TLV(3)
	}

	// Pack store on server TLV
	packDWord(&packet, 0x00060000); // TLV(6)

	sendServPacket(&packet);

	return dwCookie;
}


void CIcqProto::sendOwnerInfoRequest(void)
{
	icq_packet packet;

	if (m_bLegacyFix) {
		cookie_fam15_data *pCookieData = (cookie_fam15_data*)SAFE_MALLOC(sizeof(cookie_fam15_data));
		pCookieData->bRequestType = REQUESTTYPE_OWNER;
		DWORD dwCookie = AllocateCookie(CKT_FAMILYSPECIAL, 0, NULL, (void*)pCookieData);

		packServIcqExtensionHeader(&packet, this, 6, 0x07D0, (WORD)dwCookie);
		packLEWord(&packet, META_REQUEST_SELF_INFO);
		packLEDWord(&packet, m_dwLocalUIN);
	}
	else {
		cookie_directory_data *pCookieData = (cookie_directory_data*)SAFE_MALLOC(sizeof(cookie_directory_data));
		pCookieData->bRequestType = DIRECTORYREQUEST_INFOOWNER;

		DWORD dwCookie = AllocateCookie(CKT_DIRECTORY_QUERY, 0, NULL, (void*)pCookieData);
		WORD wDataLen = getUINLen(m_dwLocalUIN) + 4;

		packServIcqDirectoryHeader(&packet, this, wDataLen + 8, META_DIRECTORY_QUERY, DIRECTORY_QUERY_INFO, (WORD)dwCookie);
		packWord(&packet, 0x03); // with interests (ICQ6 uses 2 at login)
		packDWord(&packet, 0x01);
		packWord(&packet, wDataLen);

		packTLVUID(&packet, 0x32, m_dwLocalUIN, NULL);
	}

	sendServPacket(&packet);
}


DWORD CIcqProto::sendUserInfoMultiRequest(BYTE *pRequestData, WORD wDataLen, int nItems)
{
	icq_packet packet;

	cookie_directory_data *pCookieData = (cookie_directory_data*)SAFE_MALLOC(sizeof(cookie_directory_data));
	if (!pCookieData) return 0; // Failure
	pCookieData->bRequestType = DIRECTORYREQUEST_INFOMULTI;

	DWORD dwCookie = AllocateCookie(CKT_DIRECTORY_QUERY, 0, NULL, (void*)pCookieData);

	packServIcqDirectoryHeader(&packet, this, wDataLen + 2, META_DIRECTORY_QUERY, DIRECTORY_QUERY_MULTI_INFO, (WORD)dwCookie);
	packWord(&packet, nItems);
	packBuffer(&packet, pRequestData, wDataLen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendGetInfoServ(MCONTACT hContact, DWORD dwUin, int bManual)
{
	icq_packet packet;
	DWORD dwCookie = 0;

	if (IsServerOverRate(ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQUEST, bManual ? RML_IDLE_10 : RML_IDLE_50))
		return dwCookie;

	DBVARIANT infoToken = { DBVT_DELETED };
	BYTE *pToken = NULL;
	WORD cbToken = 0;

	if (!getSetting(hContact, DBSETTING_METAINFO_TOKEN, &infoToken)) { // retrieve user details using privacy token
		cbToken = infoToken.cpbVal;
		pToken = (BYTE*)_alloca(cbToken);
		memcpy(pToken, infoToken.pbVal, cbToken);

		db_free(&infoToken);
	}

	cookie_directory_data *pCookieData = (cookie_directory_data*)SAFE_MALLOC(sizeof(cookie_directory_data));

	if (m_bLegacyFix) {
		pCookieData->bRequestType = REQUESTTYPE_USERDETAILED;

		dwCookie = AllocateCookie(CKT_FAMILYSPECIAL, 0, hContact, (void*)pCookieData);

		packServIcqExtensionHeader(&packet, this, 6, CLI_META_INFO_REQ, (WORD)dwCookie);
		packLEWord(&packet, META_REQUEST_FULL_INFO);
		packLEDWord(&packet, dwUin);

		sendServPacket(&packet);

		return dwCookie;
	}

	pCookieData->bRequestType = DIRECTORYREQUEST_INFOUSER;

	dwCookie = AllocateCookie(CKT_DIRECTORY_QUERY, 0, hContact, (void*)pCookieData);
	WORD wDataLen = cbToken + getUINLen(dwUin) + (cbToken ? 8 : 4);

	packServIcqDirectoryHeader(&packet, this, wDataLen + 8, META_DIRECTORY_QUERY, DIRECTORY_QUERY_INFO, (WORD)dwCookie);
	packWord(&packet, 0x03);
	packDWord(&packet, 1);
	packWord(&packet, wDataLen);
	if (pToken)
		packTLV(&packet, 0x3C, cbToken, pToken);
	packTLVUID(&packet, 0x32, dwUin, NULL);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendGetAimProfileServ(MCONTACT hContact, char* szUid)
{
	icq_packet packet;
	BYTE bUIDlen = strlennull(szUid);

	if (IsServerOverRate(ICQ_LOCATION_FAMILY, ICQ_LOCATION_REQ_USER_INFO, RML_IDLE_10))
		return 0;

	cookie_fam15_data *pCookieData = (cookie_fam15_data*)SAFE_MALLOC(sizeof(cookie_fam15_data));
	pCookieData->bRequestType = REQUESTTYPE_PROFILE;

	DWORD dwCookie = AllocateCookie(CKT_FAMILYSPECIAL, ICQ_LOCATION_REQ_USER_INFO, hContact, (void*)pCookieData);

	serverPacketInit(&packet, (WORD)(13 + bUIDlen));
	packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_REQ_USER_INFO, 0, dwCookie);
	packWord(&packet, 0x01); // request profile info
	packByte(&packet, bUIDlen);
	packBuffer(&packet, (LPBYTE)szUid, bUIDlen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendGetAwayMsgServ(MCONTACT hContact, DWORD dwUin, int type, WORD wVersion)
{
	icq_packet packet;

	if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_IDLE_30))
		return 0;

	cookie_message_data *pCookieData = CreateMessageCookie(MTYPE_AUTOAWAY, (BYTE)type);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	packServChannel2Header(&packet, this, dwUin, 3, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwCookie, wVersion, (BYTE)type, 3, 1, 0, 0, 0);
	packEmptyMsg(&packet);    // Message
	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendGetAwayMsgServExt(MCONTACT hContact, DWORD dwUin, char *szUID, int type, WORD wVersion)
{
	icq_packet packet;

	if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_IDLE_30))
		return 0;

	cookie_message_data *pCookieData = CreateMessageCookie(MTYPE_AUTOAWAY, (BYTE)type);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, szUID, 2, 122 + getPluginTypeIdLen(type));

	// TLV(5) header
	packServTLV5HeaderMsg(&packet, 82 + getPluginTypeIdLen(type), pCookieData->dwMsgID1, pCookieData->dwMsgID2, 1);

	// TLV(0x2711) header
	packServTLV2711Header(&packet, (WORD)dwCookie, wVersion, MTYPE_PLUGIN, 0, 0, 0x100, 27 + getPluginTypeIdLen(type));
	//
	packLEWord(&packet, 0); // Empty msg

	packPluginTypeId(&packet, type);

	packLEDWord(&packet, 0x15);
	packLEDWord(&packet, 0);
	packLEDWord(&packet, 0x0D);
	packBuffer(&packet, (LPBYTE)"text/x-aolrtf", 0x0D);

	// Send the monster
	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendGetAimAwayMsgServ(MCONTACT hContact, char *szUID, int type)
{
	icq_packet packet;
	BYTE bUIDlen = strlennull(szUID);

	cookie_message_data *pCookieData = CreateMessageCookie(MTYPE_AUTOAWAY, (byte)type);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	serverPacketInit(&packet, (WORD)(13 + bUIDlen));
	packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_REQ_USER_INFO, 0, dwCookie);
	packWord(&packet, 0x03);
	packUID(&packet, 0, szUID);

	sendServPacket(&packet);

	return dwCookie;
}


void CIcqProto::icq_sendSetAimAwayMsgServ(const char *szMsg)
{
	icq_packet packet;
	WORD wMsgLen = strlennull(szMsg);

	DWORD dwCookie = GenerateCookie(ICQ_LOCATION_SET_USER_INFO);

	if (wMsgLen) {
		if (wMsgLen > 0x1000) wMsgLen = 0x1000; // limit length

		if (IsUSASCII(szMsg, wMsgLen)) {
			const char* fmt = "text/x-aolrtf; charset=\"us-ascii\"";
			const WORD fmtlen = (WORD)strlen(fmt);

			serverPacketInit(&packet, 23 + wMsgLen + fmtlen);
			packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO, 0, dwCookie);
			packTLV(&packet, 0x0f, 1, (LPBYTE)"\x02");
			packTLV(&packet, 0x03, fmtlen, (LPBYTE)fmt);
			packTLV(&packet, 0x04, wMsgLen, (LPBYTE)szMsg);
		}
		else {
			const char* fmt = "text/x-aolrtf; charset=\"unicode-2-0\"";
			const WORD fmtlen = (WORD)strlen(fmt);

			WCHAR *szMsgW = make_unicode_string(szMsg);
			wMsgLen = (WORD)strlennull(szMsgW) * sizeof(WCHAR);

			WCHAR *szMsgW2 = (WCHAR*)alloca(wMsgLen), *szMsgW3 = szMsgW;
			unpackWideString((BYTE**)&szMsgW3, szMsgW2, wMsgLen);
			SAFE_FREE(&szMsgW);

			serverPacketInit(&packet, 23 + wMsgLen + fmtlen);
			packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO, 0, dwCookie);
			packTLV(&packet, 0x0f, 1, (LPBYTE)"\x02");
			packTLV(&packet, 0x03, fmtlen, (LPBYTE)fmt);
			packTLV(&packet, 0x04, wMsgLen, (LPBYTE)szMsgW2);
		}
	}
	else {
		serverPacketInit(&packet, 19);
		packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO, 0, dwCookie);
		packTLV(&packet, 0x0f, 1, (LPBYTE)"\x02");
		packTLV(&packet, 0x04, 0, NULL);
	}

	sendServPacket(&packet);
}


void CIcqProto::icq_sendFileSendServv7(filetransfer* ft, const char *szFiles)
{
	icq_packet packet;
	WORD wDescrLen = 0, wFilesLen = 0;
	char *szFilesAnsi = NULL, *szDescrAnsi = NULL;

	if (!utf8_decode(szFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(szFiles);				// Legacy fix

	if (!utf8_decode(ft->szDescription, &szDescrAnsi))
		szDescrAnsi = _strdup(ft->szDescription);	// Legacy fix

	wFilesLen = strlennull(szFilesAnsi);
	wDescrLen = strlennull(szDescrAnsi);

	packServChannel2Header(&packet, this, ft->dwUin, (WORD)(18 + wDescrLen + wFilesLen), ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwCookie, ICQ_VERSION, MTYPE_FILEREQ, 0, 1, 0, 1, 1);

	packLEWord(&packet, (WORD)(wDescrLen + 1));
	packBuffer(&packet, (LPBYTE)szDescrAnsi, (WORD)(wDescrLen + 1));
	packLEDWord(&packet, 0);   // unknown
	packLEWord(&packet, (WORD)(wFilesLen + 1));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, (WORD)(wFilesLen + 1));
	packLEDWord(&packet, ft->dwTotalSize);
	packLEDWord(&packet, 0);   // unknown

	SAFE_FREE(&szFilesAnsi);
	SAFE_FREE(&szDescrAnsi);

	sendServPacket(&packet);
}


void CIcqProto::icq_sendFileSendServv8(filetransfer* ft, const char *szFiles, int nAckType)
{
	icq_packet packet;
	WORD wDescrLen = 0, wFilesLen = 0;
	char *szFilesAnsi = NULL, *szDescrAnsi = NULL;

	if (!utf8_decode(szFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(szFiles);				// Legacy fix

	if (!utf8_decode(ft->szDescription, &szDescrAnsi))
		szDescrAnsi = _strdup(ft->szDescription);	// Legacy fix

	wFilesLen = strlennull(szFilesAnsi);
	wDescrLen = strlennull(szDescrAnsi);

	// 202 + UIN len + file description (no null) + file name (null included)
	// Packet size = Flap length + 4
	WORD wFlapLen = 178 + wDescrLen + wFilesLen + (nAckType == ACKTYPE_SERVER ? 4 : 0);
	packServMsgSendHeader(&packet, ft->dwCookie, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwUin, NULL, 2, wFlapLen);

	// TLV(5) header
	packServTLV5HeaderMsg(&packet, (WORD)(138 + wDescrLen + wFilesLen), ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, 1);

	// Port & IP information
	packServDCInfo(&packet, this, FALSE);

	// TLV(0x2711) header
	packServTLV2711Header(&packet, (WORD)ft->dwCookie, ICQ_VERSION, MTYPE_PLUGIN, 0, (WORD)MirandaStatusToIcq(m_iStatus), 0x100, 69 + wDescrLen + wFilesLen);

	packEmptyMsg(&packet);  // Message (unused)

	packPluginTypeId(&packet, MTYPE_FILEREQ);

	packLEDWord(&packet, (WORD)(18 + wDescrLen + wFilesLen + 1)); // Remaining length
	packLEDWord(&packet, wDescrLen);          // Description
	packBuffer(&packet, (LPBYTE)szDescrAnsi, wDescrLen);
	packWord(&packet, 0x8c82); // Unknown (port?), seen 0x80F6
	packWord(&packet, 0x0222); // Unknown, seen 0x2e01
	packLEWord(&packet, (WORD)(wFilesLen + 1));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, (WORD)(wFilesLen + 1));
	packLEDWord(&packet, ft->dwTotalSize);
	packLEDWord(&packet, 0x0008c82); // Unknown, (seen 0xf680 ~33000)

	SAFE_FREE(&szFilesAnsi);
	SAFE_FREE(&szDescrAnsi);

	// Pack request server ack TLV
	if (nAckType == ACKTYPE_SERVER)
		packDWord(&packet, 0x00030000); // TLV(3)

	// Send the monster
	sendServPacket(&packet);
}


/* also sends rejections */
void CIcqProto::icq_sendFileAcceptServv8(DWORD dwUin, DWORD TS1, DWORD TS2, DWORD dwCookie, const char *szFiles, const char *szDescr, DWORD dwTotalSize, WORD wPort, BOOL accepted, int nAckType)
{
	icq_packet packet;
	WORD wDescrLen, wFilesLen;
	char *szFilesAnsi = NULL, *szDescrAnsi = NULL;

	/* if !accepted, szDescr == szReason, szFiles = "" */

	if (!accepted) szFiles = "";

	if (!utf8_decode(szFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(szFiles);	// Legacy fix

	if (!utf8_decode(szDescr, &szDescrAnsi))
		szDescrAnsi = _strdup(szDescr);	// Legacy fix

	wDescrLen = strlennull(szDescrAnsi);
	wFilesLen = strlennull(szFilesAnsi);

	// 202 + UIN len + file description (no null) + file name (null included)
	// Packet size = Flap length + 4
	WORD wFlapLen = 178 + wDescrLen + wFilesLen + (nAckType == ACKTYPE_SERVER ? 4 : 0);
	packServMsgSendHeader(&packet, dwCookie, TS1, TS2, dwUin, NULL, 2, wFlapLen);

	// TLV(5) header
	packServTLV5HeaderMsg(&packet, (WORD)(138 + wDescrLen + wFilesLen), TS1, TS2, 2);

	// Port & IP information
	packServDCInfo(&packet, this, !accepted);

	// TLV(0x2711) header
	packServTLV2711Header(&packet, (WORD)dwCookie, ICQ_VERSION, MTYPE_PLUGIN, 0, (WORD)(accepted ? 0 : 1), 0, 69 + wDescrLen + wFilesLen);
	//
	packEmptyMsg(&packet);    // Message (unused)

	packPluginTypeId(&packet, MTYPE_FILEREQ);

	packLEDWord(&packet, (WORD)(18 + wDescrLen + wFilesLen + 1)); // Remaining length
	packLEDWord(&packet, wDescrLen);          // Description
	packBuffer(&packet, (LPBYTE)szDescrAnsi, wDescrLen);
	packWord(&packet, wPort); // Port
	packWord(&packet, 0x00);  // Unknown
	packLEWord(&packet, (WORD)(wFilesLen + 1));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, (WORD)(wFilesLen + 1));
	packLEDWord(&packet, dwTotalSize);
	packLEDWord(&packet, (DWORD)wPort); // Unknown

	SAFE_FREE(&szFilesAnsi);
	SAFE_FREE(&szDescrAnsi);

	// Pack request server ack TLV
	if (nAckType == ACKTYPE_SERVER) {
		packDWord(&packet, 0x00030000); // TLV(3)
	}

	// Send the monster
	sendServPacket(&packet);
}


void CIcqProto::icq_sendFileAcceptServv7(DWORD dwUin, DWORD TS1, DWORD TS2, DWORD dwCookie, const char* szFiles, const char* szDescr, DWORD dwTotalSize, WORD wPort, BOOL accepted, int nAckType)
{
	icq_packet packet;
	WORD wDescrLen, wFilesLen;
	char *szFilesAnsi = NULL, *szDescrAnsi = NULL;

	/* if !accepted, szDescr == szReason, szFiles = "" */

	if (!accepted) szFiles = "";

	if (!utf8_decode(szFiles, &szFilesAnsi))
		szFilesAnsi = _strdup(szFiles);	// Legacy fix

	if (!utf8_decode(szDescr, &szDescrAnsi))
		szDescrAnsi = _strdup(szDescr);	// Legacy fix

	wDescrLen = strlennull(szDescrAnsi);
	wFilesLen = strlennull(szFilesAnsi);

	// 150 + UIN len + file description (with null) + file name (2 nulls)
	// Packet size = Flap length + 4
	WORD wFlapLen = 127 + wDescrLen + 1 + wFilesLen + (nAckType == ACKTYPE_SERVER ? 4 : 0);
	packServMsgSendHeader(&packet, dwCookie, TS1, TS2, dwUin, NULL, 2, wFlapLen);

	// TLV(5) header
	packServTLV5HeaderMsg(&packet, (WORD)(88 + wDescrLen + wFilesLen), TS1, TS2, 2);

	// Port & IP information
	packServDCInfo(&packet, this, !accepted);

	// TLV(0x2711) header
	packServTLV2711Header(&packet, (WORD)dwCookie, ICQ_VERSION, MTYPE_FILEREQ, 0, (WORD)(accepted ? 0 : 1), 0, 19 + wDescrLen + wFilesLen);
	//
	packLEWord(&packet, (WORD)(wDescrLen + 1));     // Description
	packBuffer(&packet, (LPBYTE)szDescrAnsi, (WORD)(wDescrLen + 1));
	packWord(&packet, wPort);   // Port
	packWord(&packet, 0x00);    // Unknown
	packLEWord(&packet, (WORD)(wFilesLen + 2));
	packBuffer(&packet, (LPBYTE)szFilesAnsi, (WORD)(wFilesLen + 1));
	packByte(&packet, 0);
	packLEDWord(&packet, dwTotalSize);
	packLEDWord(&packet, (DWORD)wPort); // Unknown

	SAFE_FREE(&szFilesAnsi);
	SAFE_FREE(&szDescrAnsi);

	// Pack request server ack TLV
	if (nAckType == ACKTYPE_SERVER) {
		packDWord(&packet, 0x00030000); // TLV(3)
	}

	// Send the monster
	sendServPacket(&packet);
}


void CIcqProto::icq_sendFileAcceptServ(DWORD dwUin, filetransfer *ft, int nAckType)
{
	char *szDesc = ft->szDescription;

	if (ft->bEmptyDesc) szDesc = ""; // keep empty if it originally was (Trillian workaround)

	if (ft->nVersion >= 8) {
		icq_sendFileAcceptServv8(dwUin, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwCookie, ft->szFilename, szDesc, ft->dwTotalSize, wListenPort, TRUE, nAckType);
		debugLogA("Sent file accept v%u through server, port %u", 8, wListenPort);
	}
	else {
		icq_sendFileAcceptServv7(dwUin, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwCookie, ft->szFilename, szDesc, ft->dwTotalSize, wListenPort, TRUE, nAckType);
		debugLogA("Sent file accept v%u through server, port %u", 7, wListenPort);
	}
}


void CIcqProto::icq_sendFileDenyServ(DWORD dwUin, filetransfer *ft, const char *szReason, int nAckType)
{
	if (ft->nVersion >= 8) {
		icq_sendFileAcceptServv8(dwUin, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwCookie, ft->szFilename, szReason, ft->dwTotalSize, wListenPort, FALSE, nAckType);
		debugLogA("Sent file deny v%u through server", 8);
	}
	else {
		icq_sendFileAcceptServv7(dwUin, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, ft->dwCookie, ft->szFilename, szReason, ft->dwTotalSize, wListenPort, FALSE, nAckType);
		debugLogA("Sent file deny v%u through server", 7);
	}
}


void CIcqProto::icq_sendAwayMsgReplyServ(DWORD dwUin, DWORD dwMsgID1, DWORD dwMsgID2, WORD wCookie, WORD wVersion, BYTE msgType, char** szMsg)
{
	MCONTACT hContact = HContactFromUIN(dwUin, NULL);

	if (validateStatusMessageRequest(hContact, msgType)) {
		NotifyEventHooks(m_modeMsgsEvent, (WPARAM)msgType, (LPARAM)dwUin);

		icq_lock l(m_modeMsgsMutex);

		if (szMsg && *szMsg) {
			char *pszMsg = NULL;
			WORD wReplyVersion = ICQ_VERSION;

			if (wVersion >= 9) {
				pszMsg = *szMsg;
				wReplyVersion = 9;
			}
			else { // only v9 protocol supports UTF-8 mode messagees
				WORD wMsgLen = strlennull(*szMsg) + 1;
				char *szAnsiMsg = (char*)_alloca(wMsgLen);

				utf8_decode_static(*szMsg, szAnsiMsg, wMsgLen);
				pszMsg = szAnsiMsg;
			}

			WORD wMsgLen = strlennull(pszMsg);

			// limit msg len to max snac size - we get disconnected if exceeded
			if (wMsgLen > MAX_MESSAGESNACSIZE)
				wMsgLen = MAX_MESSAGESNACSIZE;

			icq_packet packet;

			packServAdvancedMsgReply(&packet, dwUin, NULL, dwMsgID1, dwMsgID2, wCookie, wReplyVersion, msgType, 3, (WORD)(wMsgLen + 3));
			packLEWord(&packet, (WORD)(wMsgLen + 1));
			packBuffer(&packet, (LPBYTE)pszMsg, wMsgLen);
			packByte(&packet, 0);

			sendServPacket(&packet);
		}
	}
}


void CIcqProto::icq_sendAwayMsgReplyServExt(DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wCookie, WORD wVersion, BYTE msgType, char **szMsg)
{
	MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

	if (validateStatusMessageRequest(hContact, msgType)) {
		NotifyEventHooks(m_modeMsgsEvent, (WPARAM)msgType, (LPARAM)dwUin);

		icq_lock l(m_modeMsgsMutex);

		if (szMsg && *szMsg) {
			char *pszMsg = NULL;
			WORD wReplyVersion = ICQ_VERSION;

			if (wVersion >= 9)
				pszMsg = *szMsg;
			else { // only v9 protocol supports UTF-8 mode messagees
				wReplyVersion = 8;

				WORD wMsgLen = strlennull(*szMsg) + 1;
				char *szAnsiMsg = (char*)_alloca(wMsgLen);

				utf8_decode_static(*szMsg, szAnsiMsg, wMsgLen);
				pszMsg = szAnsiMsg;
			}
			// convert to HTML 
			char *mng = MangleXml(pszMsg, strlennull(pszMsg));
			pszMsg = (char*)SAFE_MALLOC(strlennull(mng) + 28);
			strcpy(pszMsg, "<HTML><BODY>"); /// TODO: add support for RTL & user customizable font
			strcat(pszMsg, mng);
			SAFE_FREE(&mng);
			strcat(pszMsg, "</BODY></HTML>");

			WORD wMsgLen = strlennull(pszMsg);

			// limit msg len to max snac size - we get disconnected if exceeded /// FIXME: correct HTML cutting
			if (wMsgLen > MAX_MESSAGESNACSIZE)
				wMsgLen = MAX_MESSAGESNACSIZE;

			icq_packet packet;

			packServAdvancedMsgReply(&packet, dwUin, szUID, dwMsgID1, dwMsgID2, wCookie, wReplyVersion, MTYPE_PLUGIN, 0, wMsgLen + 27 + getPluginTypeIdLen(msgType));
			packLEWord(&packet, 0);   // Message size
			packPluginTypeId(&packet, msgType);

			packLEDWord(&packet, wMsgLen + 21);
			packLEDWord(&packet, wMsgLen);
			packBuffer(&packet, (LPBYTE)pszMsg, wMsgLen);

			packLEDWord(&packet, 0x0D);
			packBuffer(&packet, (LPBYTE)"text/x-aolrtf", 0x0D);

			sendServPacket(&packet);
			SAFE_FREE(&pszMsg);
		}
	}
}


void CIcqProto::icq_sendAdvancedMsgAck(DWORD dwUin, DWORD dwTimestamp, DWORD dwTimestamp2, WORD wCookie, BYTE bMsgType, BYTE bMsgFlags)
{
	icq_packet packet;

	packServAdvancedMsgReply(&packet, dwUin, NULL, dwTimestamp, dwTimestamp2, wCookie, ICQ_VERSION, bMsgType, bMsgFlags, 11);
	packEmptyMsg(&packet);       // Status message
	packMsgColorInfo(&packet);

	sendServPacket(&packet);
}


void CIcqProto::icq_sendContactsAck(DWORD dwUin, char *szUid, DWORD dwMsgID1, DWORD dwMsgID2)
{
	icq_packet packet;

	packServMsgSendHeader(&packet, 0, dwMsgID1, dwMsgID2, dwUin, szUid, 2, 0x1E);
	packServTLV5HeaderBasic(&packet, 0, dwMsgID1, dwMsgID2, 2, MCAP_CONTACTS);

	sendServPacket(&packet);
}


// Searches

DWORD CIcqProto::SearchByUin(DWORD dwUin)
{
	WORD wInfoLen;
	icq_packet pBuffer; // I reuse the ICQ packet type as a generic buffer
	// I should be ashamed! ;)
	if (m_bLegacyFix) {
		// Calculate data size
		wInfoLen = 8;

		// Initialize our handy data buffer
		pBuffer.wPlace = 0;
		pBuffer.pData = (BYTE *)_alloca(wInfoLen);
		pBuffer.wLen = wInfoLen;

		// Initialize our handy data buffer
		packLEWord(&pBuffer, TLV_UIN);
		packLEWord(&pBuffer, 0x0004);
		packLEDWord(&pBuffer, dwUin);

		// Send it off for further packing
		return sendTLVSearchPacket(SEARCHTYPE_UID, (char*)pBuffer.pData, META_SEARCH_UIN, wInfoLen, FALSE);
	}

	// Calculate data size
	wInfoLen = 4 + getUINLen(dwUin);

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wInfoLen);
	pBuffer.wLen = wInfoLen;

	// Initialize our handy data buffer
	packTLVUID(&pBuffer, 0x32, dwUin, NULL);

	// Send it off for further packing
	return sendDirectorySearchPacket(pBuffer.pData, wInfoLen, 0, FALSE);
}


DWORD CIcqProto::SearchByNames(const char *pszNick, const char *pszFirstName, const char *pszLastName, WORD wPage)
{ // use directory search like ICQ6 does
	WORD wInfoLen = 0;
	WORD wNickLen, wFirstLen, wLastLen;
	icq_packet pBuffer; // I reuse the ICQ packet type as a generic buffer
	// I should be ashamed! ;)
	if (m_bLegacyFix) {
		// Legacy protocol uses ANSI-string searches

		char* pszNickAnsi = NULL;
		if (!utf8_decode(pszNick, &pszNickAnsi))
			pszNickAnsi = _strdup(pszNick);

		char* pszFirstNameAnsi = NULL;
		if (!utf8_decode(pszFirstName, &pszFirstNameAnsi))
			pszFirstNameAnsi = _strdup(pszFirstName);

		char* pszLastNameAnsi = NULL;
		if (!utf8_decode(pszLastName, &pszLastNameAnsi))
			pszLastNameAnsi = _strdup(pszLastName);

		wNickLen = strlennull(pszNickAnsi);
		wFirstLen = strlennull(pszFirstNameAnsi);
		wLastLen = strlennull(pszLastNameAnsi);

		_ASSERTE(wFirstLen || wLastLen || wNickLen);

		// Calculate data size
		if (wFirstLen > 0)
			wInfoLen = wFirstLen + 7;
		if (wLastLen > 0)
			wInfoLen += wLastLen + 7;
		if (wNickLen > 0)
			wInfoLen += wNickLen + 7;

		// Initialize our handy data buffer
		pBuffer.wPlace = 0;
		pBuffer.pData = (BYTE *)_alloca(wInfoLen);
		pBuffer.wLen = wInfoLen;

		int pBufferPos = 0;

		// Pack the search details
		if (wFirstLen > 0) {
			packLETLVLNTS(&pBuffer.pData, &pBufferPos, pszFirstNameAnsi, TLV_FIRSTNAME);
		}

		if (wLastLen > 0) {
			packLETLVLNTS(&pBuffer.pData, &pBufferPos, pszLastNameAnsi, TLV_LASTNAME);
		}

		if (wNickLen > 0) {
			packLETLVLNTS(&pBuffer.pData, &pBufferPos, pszNickAnsi, TLV_NICKNAME);
		}

		SAFE_FREE(&pszFirstNameAnsi);
		SAFE_FREE(&pszLastNameAnsi);
		SAFE_FREE(&pszNickAnsi);

		// Send it off for further packing
		return sendTLVSearchPacket(SEARCHTYPE_NAMES, (char*)pBuffer.pData, META_SEARCH_GENERIC, wInfoLen, FALSE);
	}

	wNickLen = strlennull(pszNick);
	wFirstLen = strlennull(pszFirstName);
	wLastLen = strlennull(pszLastName);

	_ASSERTE(wFirstLen || wLastLen || wNickLen);


	// Calculate data size
	if (wFirstLen)
		wInfoLen = wFirstLen + 4;
	if (wLastLen)
		wInfoLen += wLastLen + 4;
	if (wNickLen)
		wInfoLen += wNickLen + 4;

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wInfoLen);
	pBuffer.wLen = wInfoLen;

	// Pack the search details
	if (wNickLen)
		packTLV(&pBuffer, 0x78, wNickLen, (PBYTE)pszNick);

	if (wLastLen)
		packTLV(&pBuffer, 0x6E, wLastLen, (PBYTE)pszLastName);

	if (wFirstLen)
		packTLV(&pBuffer, 0x64, wFirstLen, (PBYTE)pszFirstName);

	// Send it off for further packing
	if (wInfoLen)
		return sendDirectorySearchPacket(pBuffer.pData, wInfoLen, wPage, FALSE);
	else
		return 0; // Failure
}


DWORD CIcqProto::SearchByMail(const char* pszEmail)
{
	DWORD dwCookie = 0;
	WORD wInfoLen = 0;
	WORD wEmailLen;
	BYTE *pBuffer;
	int pBufferPos;

	wEmailLen = strlennull(pszEmail);

	_ASSERTE(wEmailLen);

	if (wEmailLen > 0) {
		// Calculate data size
		wInfoLen = wEmailLen + 7;

		// Initialize our handy data buffer
		pBuffer = (BYTE *)_alloca(wInfoLen);
		pBufferPos = 0;

		// Pack the search details
		packLETLVLNTS(&pBuffer, &pBufferPos, pszEmail, TLV_EMAIL);

		// Send it off for further packing
		dwCookie = sendTLVSearchPacket(SEARCHTYPE_EMAIL, (char*)pBuffer, META_SEARCH_EMAIL, wInfoLen, FALSE);
	}

	return dwCookie;
}


DWORD CIcqProto::sendDirectorySearchPacket(const BYTE *pSearchData, WORD wDataLen, WORD wPage, BOOL bOnlineUsersOnly)
{
	icq_packet packet;
	DWORD dwCookie;

	_ASSERTE(pSearchData);
	_ASSERTE(wDataLen >= 4);

	cookie_directory_data *pCookieData = (cookie_directory_data*)SAFE_MALLOC(sizeof(cookie_directory_data));
	if (pCookieData) {
		pCookieData->bRequestType = DIRECTORYREQUEST_SEARCH;
		dwCookie = AllocateCookie(CKT_DIRECTORY_QUERY, 0, NULL, (void*)pCookieData);
	}
	else
		return 0;

	// Pack headers
	packServIcqDirectoryHeader(&packet, this, wDataLen + (bOnlineUsersOnly ? 14 : 8), META_DIRECTORY_QUERY, DIRECTORY_QUERY_INFO, (WORD)dwCookie);
	packWord(&packet, 0x02);

	// Pack requested page number
	packWord(&packet, wPage);

	// Pack search data
	packWord(&packet, 0x0001);
	packWord(&packet, wDataLen + (bOnlineUsersOnly ? 6 : 0));
	packBuffer(&packet, pSearchData, wDataLen);

	if (bOnlineUsersOnly) { // Pack "Online users only" flag
		packTLVWord(&packet, 0x136, 1);
	}

	// Go!
	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::sendTLVSearchPacket(BYTE bType, char* pSearchDataBuf, WORD wSearchType, WORD wInfoLen, BOOL bOnlineUsersOnly)
{
	icq_packet packet;
	cookie_search* pCookie;

	_ASSERTE(pSearchDataBuf);
	_ASSERTE(wInfoLen >= 4);

	pCookie = (cookie_search*)SAFE_MALLOC(sizeof(cookie_search));
	if (!pCookie)
		return 0;

	pCookie->bSearchType = bType;
	DWORD dwCookie = AllocateCookie(CKT_SEARCH, 0, 0, pCookie);

	// Pack headers
	packServIcqExtensionHeader(&packet, this, (WORD)(wInfoLen + (wSearchType == META_SEARCH_GENERIC ? 7 : 2)), CLI_META_INFO_REQ, (WORD)dwCookie);

	// Pack search type
	packLEWord(&packet, wSearchType);

	// Pack search data
	packBuffer(&packet, (LPBYTE)pSearchDataBuf, wInfoLen);

	if (wSearchType == META_SEARCH_GENERIC && bOnlineUsersOnly) { // Pack "Online users only" flag - only for generic search
		BYTE bData = 1;
		packTLV(&packet, TLV_ONLINEONLY, 1, &bData);
	}

	// Go!
	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendAdvancedSearchServ(BYTE* fieldsBuffer, int bufferLen)
{
	icq_packet packet;
	DWORD dwCookie;

	cookie_search *pCookie = (cookie_search*)SAFE_MALLOC(sizeof(cookie_search));
	if (pCookie) {
		pCookie->bSearchType = SEARCHTYPE_DETAILS;
		dwCookie = AllocateCookie(CKT_SEARCH, 0, 0, pCookie);
	}
	else
		return 0;

	packServIcqExtensionHeader(&packet, this, (WORD)bufferLen, CLI_META_INFO_REQ, (WORD)dwCookie);
	packBuffer(&packet, (LPBYTE)fieldsBuffer, (WORD)bufferLen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_searchAimByEmail(const char* pszEmail, DWORD dwSearchId)
{
	icq_packet packet;
	DWORD dwCookie;
	cookie_search* pCookie;
	WORD wEmailLen;

	if (!FindCookie(dwSearchId, NULL, (void**)&pCookie)) {
		dwSearchId = 0;
		pCookie = (cookie_search*)SAFE_MALLOC(sizeof(cookie_search));
		pCookie->bSearchType = SEARCHTYPE_EMAIL;
	}

	if (pCookie) {
		pCookie->dwMainId = dwSearchId;
		pCookie->szObject = null_strdup(pszEmail);
		dwCookie = AllocateCookie(CKT_SEARCH, ICQ_LOOKUP_REQUEST, 0, pCookie);
	}
	else
		return 0;

	wEmailLen = strlennull(pszEmail);
	serverPacketInit(&packet, (WORD)(10 + wEmailLen));
	packFNACHeader(&packet, ICQ_LOOKUP_FAMILY, ICQ_LOOKUP_REQUEST, 0, dwCookie);
	packBuffer(&packet, (LPBYTE)pszEmail, wEmailLen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_changeUserPasswordServ(const char *szPassword)
{
	icq_packet packet;
	WORD wPasswordLen = strlennull(szPassword);
	DWORD dwCookie = GenerateCookie(0);

	packServIcqExtensionHeader(&packet, this, (WORD)(wPasswordLen + 4), CLI_META_INFO_REQ, (WORD)dwCookie, ICQ_META_SRV_UPDATE);
	packLEWord(&packet, META_SET_PASSWORD_REQ);
	packLEWord(&packet, wPasswordLen);
	packBuffer(&packet, (BYTE*)szPassword, wPasswordLen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_changeUserDirectoryInfoServ(const BYTE *pData, WORD wDataLen, BYTE bRequestType)
{
	icq_packet packet;
	cookie_directory_data *pCookieData = (cookie_directory_data*)SAFE_MALLOC(sizeof(cookie_directory_data));
	pCookieData->bRequestType = bRequestType;
	DWORD dwCookie = AllocateCookie(CKT_DIRECTORY_UPDATE, 0, NULL, pCookieData);

	packServIcqDirectoryHeader(&packet, this, wDataLen + 4, META_DIRECTORY_UPDATE, DIRECTORY_SET_INFO, (WORD)dwCookie, ICQ_META_SRV_UPDATE);
	packWord(&packet, 0x0003);
	packWord(&packet, wDataLen);
	packBuffer(&packet, pData, wDataLen);

	sendServPacket(&packet);

	return dwCookie;
}


DWORD CIcqProto::icq_sendSMSServ(const char *szPhoneNumber, const char *szMsg)
{
	icq_packet packet;
	DWORD dwCookie;
	WORD wBufferLen;
	char* szBuffer = NULL;
	char* szMyNick = NULL;
	char szTime[30];
	time_t now;
	int nBufferSize;

	now = time(NULL);
	strftime(szTime, sizeof(szTime), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
	/* Sun, 00 Jan 0000 00:00:00 GMT */

	szMyNick = null_strdup((char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)(HANDLE)NULL, 0));
	nBufferSize = 1 + strlennull(szMyNick) + strlennull(szPhoneNumber) + strlennull(szMsg) + sizeof("<icq_sms_message><destination></destination><text></text><codepage>1252</codepage><encoding>utf8</encoding><senders_UIN>0000000000</senders_UIN><senders_name></senders_name><delivery_receipt>Yes</delivery_receipt><time>Sun, 00 Jan 0000 00:00:00 GMT</time></icq_sms_message>");

	if (szBuffer = (char *)_alloca(nBufferSize)) {

		wBufferLen = mir_snprintf(szBuffer, nBufferSize,
			"<icq_sms_message>"
			"<destination>"
			"%s"   /* phone number */
			"</destination>"
			"<text>"
			"%s"   /* body */
			"</text>"
			"<codepage>"
			"1252"
			"</codepage>"
			"<encoding>"
			"utf8"
			"</encoding>"
			"<senders_UIN>"
			"%u"  /* my UIN */
			"</senders_UIN>"
			"<senders_name>"
			"%s"  /* my nick */
			"</senders_name>"
			"<delivery_receipt>"
			"Yes"
			"</delivery_receipt>"
			"<time>"
			"%s"  /* time */
			"</time>"
			"</icq_sms_message>",
			szPhoneNumber, szMsg, m_dwLocalUIN, szMyNick, szTime);

		dwCookie = GenerateCookie(0);

		packServIcqExtensionHeader(&packet, this, (WORD)(wBufferLen + 27), CLI_META_INFO_REQ, (WORD)dwCookie);
		packWord(&packet, 0x8214);     /* send sms */
		packWord(&packet, 1);
		packWord(&packet, 0x16);
		packDWord(&packet, 0);
		packDWord(&packet, 0);
		packDWord(&packet, 0);
		packDWord(&packet, 0);
		packWord(&packet, 0);
		packWord(&packet, (WORD)(wBufferLen + 1));
		packBuffer(&packet, (LPBYTE)szBuffer, (WORD)(1 + wBufferLen));

		sendServPacket(&packet);
	}
	else {
		dwCookie = 0;
	}

	SAFE_FREE((void**)&szMyNick);
	return dwCookie;
}

void CIcqProto::icq_sendGenericContact(DWORD dwUin, const char *szUid, WORD wFamily, WORD wSubType)
{
	icq_packet packet;
	int nUinLen;

	nUinLen = getUIDLen(dwUin, szUid);

	serverPacketInit(&packet, (WORD)(nUinLen + 11));
	packFNACHeader(&packet, wFamily, wSubType);
	packUID(&packet, dwUin, szUid);

	sendServPacket(&packet);
}

void CIcqProto::icq_sendNewContact(DWORD dwUin, const char *szUid)
{
	/* Try to add to temporary buddy list */
	icq_sendGenericContact(dwUin, szUid, ICQ_BUDDY_FAMILY, ICQ_USER_ADDTOTEMPLIST);
}


void CIcqProto::icq_sendRemoveContact(DWORD dwUin, const char *szUid)
{
	/* Remove from temporary buddy list */
	icq_sendGenericContact(dwUin, szUid, ICQ_BUDDY_FAMILY, ICQ_USER_REMOVEFROMTEMPLIST);
}


// list==0: visible list
// list==1: invisible list
void CIcqProto::icq_sendChangeVisInvis(MCONTACT hContact, DWORD dwUin, char* szUID, int list, int add)
{ // TODO: This needs grouping & rate management
	// Tell server to change our server-side contact visbility list
	if (m_bSsiEnabled) {
		WORD wContactId;
		char* szSetting;
		WORD wType;

		if (list == 0) {
			wType = SSI_ITEM_PERMIT;
			szSetting = DBSETTING_SERVLIST_PERMIT;
		}
		else {
			wType = SSI_ITEM_DENY;
			szSetting = DBSETTING_SERVLIST_DENY;
		}

		if (add) {
			// check if we should make the changes, this is 2nd level check
			if (getWord(hContact, szSetting, 0) != 0)
				return;

			// Add
			wContactId = GenerateServerID(SSIT_ITEM, 0);

			icq_addServerPrivacyItem(hContact, dwUin, szUID, wContactId, wType);

			setWord(hContact, szSetting, wContactId);
		}
		else {
			// Remove
			wContactId = getWord(hContact, szSetting, 0);

			if (wContactId) {
				icq_removeServerPrivacyItem(hContact, dwUin, szUID, wContactId, wType);

				delSetting(hContact, szSetting);
			}
		}
	}

	// Notify server that we have changed
	// our client side visibility list
	{
		int nUinLen;
		icq_packet packet;
		WORD wSnac = 0;

		if (list && m_iStatus == ID_STATUS_INVISIBLE)
			return;

		if (!list && m_iStatus != ID_STATUS_INVISIBLE)
			return;


		if (list && add)
			wSnac = ICQ_CLI_ADDINVISIBLE;
		else if (list && !add)
			wSnac = ICQ_CLI_REMOVEINVISIBLE;
		else if (!list && add)
			wSnac = ICQ_CLI_ADDVISIBLE;
		else if (!list && !add)
			wSnac = ICQ_CLI_REMOVEVISIBLE;

		nUinLen = getUIDLen(dwUin, szUID);

		serverPacketInit(&packet, (WORD)(nUinLen + 11));
		packFNACHeader(&packet, ICQ_BOS_FAMILY, wSnac);
		packUID(&packet, dwUin, szUID);

		sendServPacket(&packet);
	}
}

void CIcqProto::icq_sendEntireVisInvisList(int list)
{
	if (list)
		sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDINVISIBLE, BUL_INVISIBLE);
	else
		sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDVISIBLE, BUL_VISIBLE);
}

void CIcqProto::icq_sendRevokeAuthServ(DWORD dwUin, char *szUid)
{
	icq_sendGenericContact(dwUin, szUid, ICQ_LISTS_FAMILY, ICQ_LISTS_REVOKEAUTH);
}

void CIcqProto::icq_sendGrantAuthServ(DWORD dwUin, const char *szUid, const char *szMsg)
{
	icq_packet packet;
	BYTE nUinlen;
	char *szUtfMsg = NULL;
	WORD nMsglen;

	nUinlen = getUIDLen(dwUin, szUid);

	// Prepare custom utf-8 message
	szUtfMsg = ansi_to_utf8(szMsg);
	nMsglen = strlennull(szUtfMsg);

	serverPacketInit(&packet, (WORD)(15 + nUinlen + nMsglen));
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_GRANTAUTH);
	packUID(&packet, dwUin, szUid);
	packWord(&packet, nMsglen);
	packBuffer(&packet, (LPBYTE)szUtfMsg, nMsglen);
	packWord(&packet, 0);

	SAFE_FREE((void**)&szUtfMsg);

	sendServPacket(&packet);
}

void CIcqProto::icq_sendAuthReqServ(DWORD dwUin, char *szUid, const char *szMsg)
{
	icq_packet packet;
	BYTE nUinlen;
	WORD nMsglen;

	nUinlen = getUIDLen(dwUin, szUid);
	nMsglen = strlennull(szMsg);

	serverPacketInit(&packet, (WORD)(15 + nUinlen + nMsglen));
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_REQUESTAUTH);
	packUID(&packet, dwUin, szUid);
	packWord(&packet, nMsglen);
	packBuffer(&packet, (LPBYTE)szMsg, nMsglen);
	packWord(&packet, 0);

	sendServPacket(&packet);
}

void CIcqProto::icq_sendAuthResponseServ(DWORD dwUin, char* szUid, int auth, const TCHAR *szReason)
{
	icq_packet packet;
	BYTE nUinLen = getUIDLen(dwUin, szUid);

	// Prepare custom utf-8 reason
	char *szUtfReason = tchar_to_utf8(szReason);
	WORD nReasonLen = strlennull(szUtfReason);

	serverPacketInit(&packet, (WORD)(16 + nUinLen + nReasonLen));
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_AUTHRESPONSE);
	packUID(&packet, dwUin, szUid);
	packByte(&packet, (BYTE)auth);
	packWord(&packet, nReasonLen);
	packBuffer(&packet, (LPBYTE)szUtfReason, nReasonLen);
	packWord(&packet, 0);

	SAFE_FREE(&szUtfReason);

	sendServPacket(&packet);
}

void CIcqProto::icq_sendYouWereAddedServ(DWORD dwUin, DWORD dwMyUin)
{
	icq_packet packet;
	DWORD dwID1;
	DWORD dwID2;

	dwID1 = time(NULL);
	dwID2 = RandRange(0, 0x00FF);

	packServMsgSendHeader(&packet, 0, dwID1, dwID2, dwUin, NULL, 0x0004, 17);
	packWord(&packet, 0x0005);      // TLV(5)
	packWord(&packet, 0x0009);
	packLEDWord(&packet, dwMyUin);
	packByte(&packet, MTYPE_ADDED);
	packByte(&packet, 0);           // msg-flags
	packEmptyMsg(&packet);          // NTS
	packDWord(&packet, 0x00060000); // TLV(6)

	sendServPacket(&packet);
}

void CIcqProto::icq_sendXtrazRequestServ(DWORD dwUin, DWORD dwCookie, char* szBody, int nBodyLen, cookie_message_data *pCookieData)
{
	icq_packet packet;
	WORD wCoreLen;

	wCoreLen = 11 + getPluginTypeIdLen(pCookieData->bMessageType) + nBodyLen;
	packServMsgSendHeader(&packet, dwCookie, pCookieData->dwMsgID1, pCookieData->dwMsgID2, dwUin, NULL, 2, (WORD)(99 + wCoreLen));

	// TLV(5) header
	packServTLV5HeaderMsg(&packet, (WORD)(55 + wCoreLen), pCookieData->dwMsgID1, pCookieData->dwMsgID2, 1);

	// TLV(0x2711) header
	packServTLV2711Header(&packet, (WORD)dwCookie, ICQ_VERSION, MTYPE_PLUGIN, 0, 0, 0x100, wCoreLen);
	//
	packEmptyMsg(&packet);

	packPluginTypeId(&packet, pCookieData->bMessageType);

	packLEDWord(&packet, nBodyLen + 4);
	packLEDWord(&packet, nBodyLen);
	packBuffer(&packet, (LPBYTE)szBody, (WORD)nBodyLen);

	// Pack request server ack TLV
	packDWord(&packet, 0x00030000); // TLV(3)

	// Send the monster
	sendServPacket(&packet);
}

void CIcqProto::icq_sendXtrazResponseServ(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szBody, int nBodyLen, int nType)
{
	icq_packet packet;

	packServAdvancedMsgReply(&packet, dwUin, NULL, dwMID, dwMID2, wCookie, ICQ_VERSION, MTYPE_PLUGIN, 0, (WORD)(getPluginTypeIdLen(nType) + 11 + nBodyLen));
	//
	packEmptyMsg(&packet);

	packPluginTypeId(&packet, nType);

	packLEDWord(&packet, nBodyLen + 4);
	packLEDWord(&packet, nBodyLen);
	packBuffer(&packet, (LPBYTE)szBody, (WORD)nBodyLen);

	// Send the monster
	sendServPacket(&packet);
}

void CIcqProto::icq_sendReverseReq(directconnect *dc, DWORD dwCookie, cookie_message_data *pCookie)
{
	icq_packet packet;

	packServMsgSendHeader(&packet, dwCookie, pCookie->dwMsgID1, pCookie->dwMsgID2, dc->dwRemoteUin, NULL, 2, 0x47);

	packServTLV5HeaderBasic(&packet, 0x29, pCookie->dwMsgID1, pCookie->dwMsgID2, 0, MCAP_REVERSE_DC_REQ);

	packTLVWord(&packet, 0x0A, 1);            // TLV: 0x0A Acktype: 1 for normal, 2 for ack
	packDWord(&packet, 0x000F0000);           // TLV: 0x0F empty
	packDWord(&packet, 0x2711001B);           // TLV: 0x2711 Content
	// TLV(0x2711) data
	packLEDWord(&packet, m_dwLocalUIN);         // Our UIN
	packDWord(&packet, dc->dwLocalExternalIP);// IP to connect to
	packLEDWord(&packet, wListenPort);        // Port to connect to
	packByte(&packet, DC_NORMAL);             // generic DC type
	packDWord(&packet, dc->dwRemotePort);     // unknown
	packDWord(&packet, wListenPort);          // port again ?
	packLEWord(&packet, ICQ_VERSION);         // DC Version
	packLEDWord(&packet, dwCookie);           // Req Cookie

	// Send the monster
	sendServPacket(&packet);
}

void CIcqProto::icq_sendReverseFailed(directconnect* dc, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwCookie)
{
	icq_packet packet;
	int nUinLen = getUINLen(dc->dwRemoteUin);

	serverPacketInit(&packet, (WORD)(nUinLen + 74));
	packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_RESPONSE, 0, ICQ_MSG_RESPONSE << 0x10 | (dwCookie & 0x7FFF));
	packLEDWord(&packet, dwMsgID1);   // Msg ID part 1
	packLEDWord(&packet, dwMsgID2);   // Msg ID part 2
	packWord(&packet, 0x02);
	packUIN(&packet, dc->dwRemoteUin);
	packWord(&packet, 0x03);
	packLEDWord(&packet, dc->dwRemoteUin);
	packLEDWord(&packet, dc->dwRemotePort);
	packLEDWord(&packet, wListenPort);
	packLEWord(&packet, ICQ_VERSION);
	packLEDWord(&packet, dwCookie);

	sendServPacket(&packet);
}


// OSCAR file-transfer packets starts here
//
void CIcqProto::oft_sendFileRequest(DWORD dwUin, char *szUid, oscar_filetransfer *ft, const char *pszFiles, DWORD dwLocalInternalIP)
{
	icq_packet packet;

	size_t size = strlennull(ft->szDescription) + strlennull(pszFiles) + 160;
	char *szCoolStr = (char *)_alloca(size);
	mir_snprintf(szCoolStr, size, "<ICQ_COOL_FT><FS>%s</FS><S>%I64u</S><SID>1</SID><DESC>%s</DESC></ICQ_COOL_FT>", pszFiles, ft->qwTotalSize, ft->szDescription);
	szCoolStr = MangleXml(szCoolStr, strlennull(szCoolStr));

	WORD wDataLen = 93 + strlennull(szCoolStr) + strlennull(pszFiles);
	if (ft->bUseProxy) wDataLen += 4;

	packServMsgSendHeader(&packet, ft->dwCookie, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, dwUin, szUid, 2, (WORD)(wDataLen + 0x1E));
	packServTLV5HeaderBasic(&packet, wDataLen, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, 0, MCAP_FILE_TRANSFER);

	packTLVWord(&packet, 0x0A, ++ft->wReqNum);        // Request sequence
	packDWord(&packet, 0x000F0000);                   // Unknown
	packTLV(&packet, 0x0D, 5, (LPBYTE)"utf-8");               // Charset
	packTLV(&packet, 0x0C, (WORD)strlennull(szCoolStr), (LPBYTE)szCoolStr); // User message (CoolData XML)
	SAFE_FREE(&szCoolStr);
	if (ft->bUseProxy) {
		packTLVDWord(&packet, 0x02, ft->dwProxyIP);     // Proxy IP
		packTLVDWord(&packet, 0x16, ft->dwProxyIP ^ 0x0FFFFFFFF);    // Proxy IP check
	}
	else {
		packTLVDWord(&packet, 0x02, dwLocalInternalIP);
		packTLVDWord(&packet, 0x16, dwLocalInternalIP ^ 0x0FFFFFFFF);
	}
	packTLVDWord(&packet, 0x03, dwLocalInternalIP);   // Client IP
	if (ft->bUseProxy) {
		packTLVWord(&packet, 0x05, ft->wRemotePort);
		packTLVWord(&packet, 0x17, (WORD)(ft->wRemotePort ^ 0x0FFFF));
		packDWord(&packet, 0x00100000);                 // Proxy flag
	}
	else {
		oscar_listener *pListener = (oscar_listener*)ft->listener;

		packTLVWord(&packet, 0x05, pListener->wPort);
		packTLVWord(&packet, 0x15, (WORD)((pListener->wPort) ^ 0x0FFFF));
	}
	{ // TLV(0x2711)
		packWord(&packet, 0x2711);
		packWord(&packet, (WORD)(9 + strlennull(pszFiles)));
		packWord(&packet, (WORD)(ft->wFilesCount == 1 ? 1 : 2));
		packWord(&packet, ft->wFilesCount);
		packDWord(&packet, (DWORD)ft->qwTotalSize);
		packBuffer(&packet, (LPBYTE)pszFiles, (WORD)(strlennull(pszFiles) + 1));
	}
	packTLV(&packet, 0x2712, 5, (LPBYTE)"utf-8");
	{ // TLV(0x2713)
		packWord(&packet, 0x2713);
		packWord(&packet, 8);
		packQWord(&packet, ft->qwTotalSize);
	}

	sendServPacket(&packet);                          // Send the monster
}


void CIcqProto::oft_sendFileReply(DWORD dwUin, char *szUid, oscar_filetransfer *ft, WORD wResult)
{
	icq_packet packet;

	packServMsgSendHeader(&packet, 0, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, dwUin, szUid, 2, 0x1E);
	packServTLV5HeaderBasic(&packet, 0, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, wResult, MCAP_FILE_TRANSFER);

	sendServPacket(&packet);
}


void CIcqProto::oft_sendFileAccept(DWORD dwUin, char *szUid, oscar_filetransfer *ft)
{
	oft_sendFileReply(dwUin, szUid, ft, 0x02);
}


void CIcqProto::oft_sendFileResponse(DWORD dwUin, char *szUid, oscar_filetransfer *ft, WORD wResponse)
{
	icq_packet packet;

	packServAdvancedReply(&packet, dwUin, szUid, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, 0, 4);
	packWord(&packet, 0x02);      // Length of following data
	packWord(&packet, wResponse); // Response code

	sendServPacket(&packet);
}


void CIcqProto::oft_sendFileDeny(DWORD dwUin, char *szUid, oscar_filetransfer *ft)
{
	if (dwUin) { // ICQ clients uses special deny file transfer
		oft_sendFileResponse(dwUin, szUid, ft, 0x01);
	}
	else
		oft_sendFileReply(dwUin, szUid, ft, 0x01);
}


void CIcqProto::oft_sendFileCancel(DWORD dwUin, char *szUid, oscar_filetransfer *ft)
{
	oft_sendFileReply(dwUin, szUid, ft, 0x01);
}


void CIcqProto::oft_sendFileRedirect(DWORD dwUin, char *szUid, oscar_filetransfer *ft, DWORD dwIP, WORD wPort, int bProxy)
{
	icq_packet packet;

	packServMsgSendHeader(&packet, 0, ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, dwUin, szUid, 2, (WORD)(bProxy ? 0x4a : 0x4e));
	packServTLV5HeaderBasic(&packet, (WORD)(bProxy ? 0x2C : 0x30), ft->pMessage.dwMsgID1, ft->pMessage.dwMsgID2, 0, MCAP_FILE_TRANSFER);
	// Connection point data
	packTLVWord(&packet, 0x0A, ++ft->wReqNum);                // Ack Type
	packTLVWord(&packet, 0x14, 0x0A);                         // Unknown ?
	packTLVDWord(&packet, 0x02, dwIP);                        // Internal IP / Proxy IP
	packTLVDWord(&packet, 0x16, dwIP ^ 0x0FFFFFFFF);          // IP Check ?
	if (!bProxy)
		packTLVDWord(&packet, 0x03, dwIP);
	packTLVWord(&packet, 0x05, wPort);                        // Listening Port
	packTLVWord(&packet, 0x17, (WORD)(wPort ^ 0x0FFFF));      // Port Check ?
	if (bProxy)
		packDWord(&packet, 0x00100000);                         // Proxy Flag

	sendServPacket(&packet);
}
