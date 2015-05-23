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
//  DESCRIPTION:
//
//  Handles packets from Family 4 ICBM Messages
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::handleMsgFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader)
{
	switch (pSnacHeader->wSubtype) {
	case ICQ_MSG_SRV_ERROR:          // SNAC(4, 0x01)
		handleRecvServMsgError(pBuffer, wBufferLength, pSnacHeader->dwRef);
		break;

	case ICQ_MSG_SRV_REPLYICBM:      // SNAC(4, 0x05) SRV_REPLYICBM
		handleReplyICBM();
		break;

	case ICQ_MSG_SRV_RECV:           // SNAC(4, 0x07)
		handleRecvServMsg(pBuffer, wBufferLength, pSnacHeader->dwRef);
		break;

	case ICQ_MSG_SRV_MISSED_MESSAGE: // SNAC(4, 0x0A)
		handleMissedMsg(pBuffer, wBufferLength);
		break;

	case ICQ_MSG_RESPONSE:           // SNAC(4, 0x0B)
		handleRecvMsgResponse(pBuffer, wBufferLength);
		break;

	case ICQ_MSG_SRV_ACK:            // SNAC(4, 0x0C) Server acknowledgements
		handleServerAck(pBuffer, wBufferLength, pSnacHeader->dwRef);
		break;

	case ICQ_MSG_MTN:                // SNAC(4, 0x14) Typing notifications
		handleTypingNotification(pBuffer, wBufferLength);
		break;

	case ICQ_MSG_SRV_OFFLINE_REPLY:  // SNAC(4, 0x17) Offline Messages response
		handleOffineMessagesReply(pSnacHeader->dwRef);
		break;

	default:
		debugLogA("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_MSG_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;
	}
}

static void setMsgChannelParams(CIcqProto *ppro, WORD wChan, DWORD dwFlags)
{
	icq_packet packet;

	// Set message parameters for channel wChan (CLI_SET_ICBM_PARAMS)
	serverPacketInit(&packet, 26);
	packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_SETPARAMS);
	packWord(&packet, wChan);               // Channel
	packDWord(&packet, dwFlags);            // Flags
	packWord(&packet, MAX_MESSAGESNACSIZE); // Max message snac size
	packWord(&packet, 0x03E7);              // Max sender warning level
	packWord(&packet, 0x03E7);              // Max receiver warning level
	packWord(&packet, CLIENTRATELIMIT);     // Minimum message interval in seconds
	packWord(&packet, 0x0000);              // Unknown
	ppro->sendServPacket(&packet);
}

void CIcqProto::handleReplyICBM()
{
	// we don't care about the stuff, just change the params
	DWORD dwFlags = 0x00000303;

	#ifdef DBG_CAPHTML
		dwFlags |= 0x00000400;
	#endif
	#ifdef DBG_CAPMTN
		dwFlags |= 0x00000008;
	#endif
	// Set message parameters for all channels (imitate ICQ 6)
	setMsgChannelParams(this, 0x0000, dwFlags);
}

void CIcqProto::handleRecvServMsg(BYTE *buf, size_t wLen, DWORD dwRef)
{
	DWORD dwUin;
	DWORD dwMsgID1;
	DWORD dwMsgID2;
	WORD wTLVCount;
	WORD wMessageFormat;
	uid_str szUID;

	if (wLen < 11) { // just do some basic packet checking
		debugLogA("Error: Malformed message thru server");
		return;
	}

	// These two values are some kind of reference, we need to save
	// them to send file request responses for example
	unpackLEDWord(&buf, &dwMsgID1); // TODO: msg cookies should be main
	wLen -= 4;
	unpackLEDWord(&buf, &dwMsgID2);
	wLen -= 4;

	// The message type used:
	unpackWord(&buf, &wMessageFormat); //  0x0001: Simple message format
	wLen -= 2;                         //  0x0002: Advanced message format
	//  0x0004: 'New' message format
	// Sender UIN
	if (!unpackUID(&buf, &wLen, &dwUin, &szUID)) return;

	if (dwUin && IsOnSpammerList(dwUin)) {
		debugLogA("Ignored Message from known Spammer");
		return;
	}

	if (wLen < 4) { // just do some basic packet checking
		debugLogA("Error: Malformed message thru server");
		return;
	}

	// Warning level?
	buf += 2;
	wLen -= 2;

	// Number of following TLVs, until msg-format dependant TLVs
	unpackWord(&buf, &wTLVCount);
	wLen -= 2;
	if (wTLVCount > 0) {
		// Save current buffer pointer so we can calculate
		// how much data we have left after the chain read.
		BYTE *pBufStart = buf;
		oscar_tlv_chain *chain = readIntoTLVChain(&buf, wLen, wTLVCount);

		// This chain contains info that is filled in by the server.
		// TLV(1): unknown
		// TLV(2): date: on since
		// TLV(3): date: on since
		// TLV(4): unknown, usually 0000. Not in file-req or auto-msg-req
		// TLV(6): sender's status
		// TLV(F): a time in seconds, unknown

		disposeChain(&chain);

		// Update wLen
		wLen -= buf - pBufStart;
	}


	// This is where the format specific data begins

	switch (wMessageFormat) {

	case 1: // Simple message format
		handleRecvServMsgType1(buf, wLen, dwUin, szUID, dwMsgID1, dwRef);
		break;

	case 2: // Encapsulated messages
		handleRecvServMsgType2(buf, wLen, dwUin, szUID, dwMsgID1, dwMsgID2);
		break;

	case 4: // Typed messages
		handleRecvServMsgType4(buf, wLen, dwUin, szUID, dwMsgID1, dwMsgID2, dwRef);
		break;

	default:
		debugLogA("Unknown format message thru server - Ref %u, Type: %u, UID: %s", dwRef, wMessageFormat, strUID(dwUin, szUID));
		break;
	}
}

char* CIcqProto::convertMsgToUserSpecificUtf(MCONTACT hContact, const char *szMsg)
{
	WORD wCP = getWord(hContact, "CodePage", m_wAnsiCodepage);
	char *usMsg = NULL;

	if (wCP != CP_ACP)
		usMsg = ansi_to_utf8_codepage(szMsg, wCP);

	return usMsg;
}

void CIcqProto::handleRecvServMsgType1(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwRef)
{
	WORD wTLVType;
	size_t wTLVLen;
	BYTE* pMsgTLV;

	if (wLen < 4) { // just perform basic structure check
		debugLogA("Message (format %u) - Ignoring empty message", 1);
		return;
	}

	// Unpack the first TLV(2)
	unpackTypedTLV(buf, wLen, 2, &wTLVType, &wTLVLen, &pMsgTLV);
	debugLogA("Message (format %u) - UID: %s", 1, strUID(dwUin, szUID));

	// It must be TLV(2)
	if (wTLVType == 2) {
		BYTE *pDataBuf = pMsgTLV;
		oscar_tlv_chain *pChain = readIntoTLVChain(&pDataBuf, wTLVLen, 0);

		// TLV(2) contains yet another TLV chain with the following TLVs:
		//   TLV(1281): Capability
		//   TLV(257):  This TLV contains the actual message (can be fragmented)

		if (pChain) {
			// Find the capability TLV
			oscar_tlv *pCapabilityTLV = pChain->getTLV(0x0501, 1);
			if (pCapabilityTLV && (pCapabilityTLV->wLen > 0)) {
				WORD wDataLen = pCapabilityTLV->wLen;
				if (wDataLen > 0)
					debugLogA("Message (format 1) - Message has %d caps.", wDataLen);
			}
			else debugLogA("Message (format 1) - No message cap.");

			{
				// Parse the message parts, usually only one 0x0101 TLV containing the message,
				// but in some cases there can be more 0x0101 TLVs containing message parts in
				// different encodings (just like the new format of Offline Messages).
				DWORD dwRecvTime;
				char* szMsg = NULL;
				bool bUtf8 = false;
				PROTORECVEVENT pre = { 0 };

				int bAdded;
				MCONTACT hContact = HContactFromUID(dwUin, szUID, &bAdded);

				WORD wMsgPart = 1;
				while (oscar_tlv *pMessageTLV = pChain->getTLV(0x0101, wMsgPart)) { // Loop thru all message parts
					if (pMessageTLV->wLen > 4) {
						char *szMsgPart = NULL;
						bool bMsgPartUnicode = false;

						// The message begins with a encoding specification
						// The first WORD is believed to have the following meaning:
						//  0x00: US-ASCII
						//  0x02: Unicode UCS-2 Big Endian encoding
						//  0x03: local 8bit encoding
						BYTE *pMsgBuf = pMessageTLV->pData;
						WORD wEncoding, wCodePage;
						unpackWord(&pMsgBuf, &wEncoding);
						unpackWord(&pMsgBuf, &wCodePage);

						WORD wMsgLen = pMessageTLV->wLen - 4;
						debugLogA("Message (format 1) - Part %d: Encoding is 0x%X, page is 0x%X", wMsgPart, wEncoding, wCodePage);

						switch (wEncoding) {
						case 2: // UCS-2
							{
								WCHAR *usMsgPart = (WCHAR*)SAFE_MALLOC(wMsgLen + 2);

								unpackWideString(&pMsgBuf, usMsgPart, wMsgLen);
								usMsgPart[wMsgLen / sizeof(WCHAR)] = 0;

								szMsgPart = make_utf8_string(usMsgPart);
								if (!IsUSASCII(szMsgPart, mir_strlen(szMsgPart)))
									bMsgPartUnicode = true;
								SAFE_FREE(&usMsgPart);
							}
							break;

						case 0: // us-ascii
						case 3: // ANSI
						default:
							// Copy the message text into a new proper string.
							szMsgPart = (char*)SAFE_MALLOC(wMsgLen + 1);
							memcpy(szMsgPart, pMsgBuf, wMsgLen);
							szMsgPart[wMsgLen] = '\0';
							break;
						}

						// Check if the new part is compatible with the message
						if (!bUtf8 && bMsgPartUnicode) { // make the resulting message utf-8 encoded - need to append utf-8 encoded part
							if (szMsg) { // not necessary to convert - appending first part, only set flags
								char *szUtfMsg = ansi_to_utf8_codepage(szMsg, getWord(hContact, "CodePage", m_wAnsiCodepage));

								SAFE_FREE(&szMsg);
								szMsg = szUtfMsg;
							}
							bUtf8 = true;
						}
						if (!bMsgPartUnicode && bUtf8) { // convert message part to utf-8 and append
							char *szUtfPart = ansi_to_utf8_codepage((char*)szMsgPart, getWord(hContact, "CodePage", m_wAnsiCodepage));
							SAFE_FREE(&szMsgPart);
							szMsgPart = szUtfPart;
						}
						// Append the new message part
						szMsg = (char*)SAFE_REALLOC(szMsg, mir_strlen(szMsg) + mir_strlen(szMsgPart) + 1);

						mir_strcat(szMsg, szMsgPart);
						SAFE_FREE(&szMsgPart);
					}
					wMsgPart++;
				}
				if (mir_strlen(szMsg)) {
					if (_strnicmp(szMsg, "<html>", 6) == 0) // strip HTML formating from AIM message
						szMsg = EliminateHtml(szMsg, mir_strlen(szMsg));

					if (!bUtf8 && !IsUSASCII(szMsg, mir_strlen(szMsg))) { // message is Ansi and contains national characters, create Unicode part by codepage
						char *usMsg = convertMsgToUserSpecificUtf(hContact, szMsg);
						if (usMsg) {
							SAFE_FREE(&szMsg);
							szMsg = usMsg;
							bUtf8 = true;
						}
					}

					dwRecvTime = (DWORD)time(NULL);

					{ // Check if the message was received as offline
						cookie_offline_messages *cookie;
						if (!(dwRef & 0x80000000) && FindCookie(dwRef, NULL, (void**)&cookie)) {
							cookie->nMessages++;

							WORD wTimeTLVType;
							size_t wTimeTLVLen;
							BYTE *pTimeTLV;
							unpackTypedTLV(buf, wLen, 0x16, &wTimeTLVType, &wTimeTLVLen, &pTimeTLV);
							if (pTimeTLV && wTimeTLVType == 0x16 && wTimeTLVLen == 4) { // found Offline timestamp
								BYTE *pBuf = pTimeTLV;

								unpackDWord(&pBuf, &dwRecvTime);
								debugLogA("Message (format %u) - Offline timestamp is %s", 1, time2text(dwRecvTime));
							}
							SAFE_FREE((void**)&pTimeTLV);
						}
					}
					// Create and send the message event
					pre.timestamp = dwRecvTime;
					pre.szMessage = (char *)szMsg;
					ProtoChainRecvMsg(hContact, &pre);

					debugLogA("Message (format 1) received");

					// Save tick value
					setDword(hContact, "TickTS", time(NULL) - (dwMsgID1 / 1000));
				}
				else debugLogA("Message (format %u) - Ignoring empty message", 1);

				SAFE_FREE(&szMsg);
			}

			// Free the chain memory
			disposeChain(&pChain);
		}
		else debugLogA("Failed to read TLV chain in message (format 1)");
	}
	else debugLogA("Unsupported TLV (%u) in message (format %u)", wTLVType, 1);

	SAFE_FREE((void**)&pMsgTLV);
}

void CIcqProto::handleRecvServMsgType2(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2)
{
	WORD wTLVType;
	size_t wTLVLen;
	BYTE *pDataBuf = NULL;

	if (wLen < 4) {
		debugLogA("Message (format %u) - Ignoring empty message", 2);
		return;
	}

	// Unpack the first TLV(5)
	unpackTypedTLV(buf, wLen, 5, &wTLVType, &wTLVLen, &pDataBuf);
	debugLogA("Message (format %u) - UID: %s", 2, strUID(dwUin, szUID));
	BYTE *pBuf = pDataBuf;

	// It must be TLV(5)
	if (wTLVType == 5) {
		WORD wCommand;
		oscar_tlv_chain* chain;
		oscar_tlv* tlv;
		DWORD q1, q2, q3, q4;

		if (wTLVLen < 26) { // just check if all basic data is there
			debugLogA("Message (format %u) - Ignoring empty message", 2);
			SAFE_FREE((void**)&pBuf);
			return;
		}

		unpackWord(&pDataBuf, &wCommand);
		wTLVLen -= 2;
		
		// Command 0x0000 - Normal message/file send request
		//         0x0001 - Abort request
		//         0x0002 - Acknowledge request
		debugLogA("Command is %u", wCommand); 

		// Some stuff we don't use
		pDataBuf += 8;  // dwID1 and dwID2 again
		wTLVLen -= 8;
		unpackDWord(&pDataBuf, &q1);
		unpackDWord(&pDataBuf, &q2);
		unpackDWord(&pDataBuf, &q3);
		unpackDWord(&pDataBuf, &q4); // Message Capability
		wTLVLen -= 16;

		if (CompareGUIDs(q1, q2, q3, q4, MCAP_SRV_RELAY_FMT)) { // we surely have at least 4 bytes for TLV chain
			MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

			if (wCommand == 1) {
				debugLogA("Cannot handle abort messages yet... :(");
				SAFE_FREE((void**)&pBuf);
				return;
			}

			if (wTLVLen < 4) { // just check if at least one tlv is there
				debugLogA("Message (format %u) - Ignoring empty message", 2);
				SAFE_FREE((void**)&pBuf);
				return;
			}

			// This TLV chain may contain the following TLVs:
			// TLV(A): Acktype 0x0000 - normal message
			//                 0x0001 - file request / abort request
			//                 0x0002 - file ack
			// TLV(F): Unknown
			// TLV(3): External IP
			// TLV(5): DC port (not to use for filetransfers)
			// TLV(0x2711): The next message level

			chain = readIntoTLVChain(&pDataBuf, wTLVLen, 0);
			if (!chain) { // sanity check
				debugLogA("Message (format %u) - Invalid data", 2);
				SAFE_FREE((void**)&pBuf);
				return;
			}

			WORD wAckType = chain->getWord(0x0A, 1);

			// Update the saved DC info (if contact already exists)
			if (hContact != INVALID_CONTACT_ID) {
				DWORD dwIP, dwExternalIP;
				WORD wPort;

				if (dwExternalIP = chain->getDWord(0x03, 1))
					setDword(hContact, "RealIP", dwExternalIP);
				if (dwIP = chain->getDWord(0x04, 1))
					setDword(hContact, "IP", dwIP);
				if (wPort = chain->getWord(0x05, 1))
					setWord(hContact, "UserPort", wPort);

				// Save tick value
				BYTE bClientID = getByte(hContact, "ClientID", 0);
				if (bClientID == CLID_GENERIC || bClientID == CLID_ICQ6)
					setDword(hContact, "TickTS", time(NULL) - (dwMsgID1 / 1000));
				else
					setDword(hContact, "TickTS", 0);
			}

			// Parse the next message level
			if (tlv = chain->getTLV(0x2711, 1))
				parseServRelayData(tlv->pData, tlv->wLen, hContact, dwUin, szUID, dwMsgID1, dwMsgID2, wAckType);
			else
				debugLogA("Warning, no 0x2711 TLV in message (format 2)");

			// Clean up
			disposeChain(&chain);
		}
		else if (CompareGUIDs(q1, q2, q3, q4, MCAP_REVERSE_DC_REQ)) { // Handle reverse DC request
			if (wCommand == 1) {
				debugLogA("Cannot handle abort messages yet... :(");
				SAFE_FREE((void**)&pBuf);
				return;
			}
			if (wTLVLen < 4) { // just check if at least one tlv is there
				debugLogA("Message (format %u) - Ignoring empty message", 2);
				SAFE_FREE((void**)&pBuf);
				return;
			}
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				SAFE_FREE((void**)&pBuf);
				return;
			}
			chain = readIntoTLVChain(&pDataBuf, wTLVLen, 0);
			if (!chain) { // Malformed packet
				debugLogA("Error: Malformed data in packet");
				SAFE_FREE((void**)&pBuf);
				return;
			}

			// Parse the next message level
			if (tlv = chain->getTLV(0x2711, 1)) {
				if (tlv->wLen == 0x1B) {
					BYTE *buf = tlv->pData;
					DWORD dwUin;

					unpackLEDWord(&buf, &dwUin);

					MCONTACT hContact = HContactFromUIN(dwUin, NULL);
					if (hContact == INVALID_CONTACT_ID)
						debugLogA("Error: %s from unknown contact %u", "Reverse Connect Request", dwUin);
					else {
						DWORD dwIp, dwPort;
						WORD wVersion;
						BYTE bMode;

						unpackDWord(&buf, &dwIp);
						unpackLEDWord(&buf, &dwPort);
						unpackByte(&buf, &bMode);
						buf += 4; // unknown
						if (dwPort)
							buf += 4;  // port, again?
						else
							unpackLEDWord(&buf, &dwPort);
						unpackLEWord(&buf, &wVersion);

						setDword(hContact, "IP", dwIp);
						setWord(hContact, "UserPort", (WORD)dwPort);
						setByte(hContact, "DCType", bMode);
						setWord(hContact, "Version", wVersion);
						if (wVersion > 6) {
							cookie_reverse_connect *pCookie = (cookie_reverse_connect*)SAFE_MALLOC(sizeof(cookie_reverse_connect));

							unpackLEDWord(&buf, (DWORD*)&pCookie->ft);
							pCookie->dwMsgID1 = dwMsgID1;
							pCookie->dwMsgID2 = dwMsgID2;

							OpenDirectConnection(hContact, DIRECTCONN_REVERSE, (void*)pCookie);
						}
						else debugLogA("Warning: Unsupported direct protocol version in %s", "Reverse Connect Request");
					}
				}
				else debugLogA("Malformed %s", "Reverse Connect Request");
			}
			else debugLogA("Warning, no 0x2711 TLV in message (format 2)");

			// Clean up
			disposeChain(&chain);
		}
		else if (CompareGUIDs(q1, q2, q3, q4, MCAP_FILE_TRANSFER)) // this is an OFT packet
			handleRecvServMsgOFT(pDataBuf, wTLVLen, dwUin, szUID, dwMsgID1, dwMsgID2, wCommand);

		else if (CompareGUIDs(q1, q2, q3, q4, MCAP_CONTACTS)) // this is Contacts Transfer
			handleRecvServMsgContacts(pDataBuf, wTLVLen, dwUin, szUID, dwMsgID1, dwMsgID2, wCommand);

		else // here should be detection of extra data streams (Xtraz)
			debugLogA("Unknown Message Format Capability");
	}
	else debugLogA("Unsupported TLV (%u) in message (format %u)", wTLVType, 2);

	SAFE_FREE((void**)&pBuf);
}


void CIcqProto::parseServRelayData(BYTE *pDataBuf, size_t wLen, MCONTACT hContact, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wAckType)
{
	WORD wId;

	if (wLen < 2) {
		debugLogA("Message (format %u) - Ignoring empty message", 2);
		return;
	}

	unpackLEWord(&pDataBuf, &wId); // Incorrect identification, but working
	wLen -= 2;

	// Only 0x1B are real messages
	if (wId == 0x001B) {
		WORD wVersion;
		WORD wCookie;
		DWORD dwGuid1, dwGuid2, dwGuid3, dwGuid4;

		if (wLen < 31) { // just check if we have data to work with
			debugLogA("Message (format %u) - Ignoring empty message", 2);
			return;
		}

		unpackLEWord(&pDataBuf, &wVersion);
		wLen -= 2;

		if (hContact != INVALID_CONTACT_ID)
			setWord(hContact, "Version", wVersion);

		unpackDWord(&pDataBuf, &dwGuid1); // plugin type GUID
		unpackDWord(&pDataBuf, &dwGuid2);
		unpackDWord(&pDataBuf, &dwGuid3);
		unpackDWord(&pDataBuf, &dwGuid4);
		wLen -= 16;

		// Skip lots of unused stuff
		pDataBuf += 9;
		wLen -= 9;

		unpackLEWord(&pDataBuf, &wId);
		wLen -= 2;

		unpackLEWord(&pDataBuf, &wCookie);
		wLen -= 2;

		if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PSIG_MESSAGE)) { // is this a normal message ?
			BYTE bMsgType;
			BYTE bFlags;
			WORD wStatus, wPritority;
			WORD wMsgLen;

			if (wLen < 20) { // check if there is everything that should be there
				debugLogA("Message (format %u) - Ignoring empty message", 2);
				return;
			}

			pDataBuf += 12;  /* all zeroes */
			wLen -= 12;
			unpackByte(&pDataBuf, &bMsgType);
			wLen -= 1;
			unpackByte(&pDataBuf, &bFlags);
			wLen -= 1;

			// Status
			unpackLEWord(&pDataBuf, &wStatus);
			wLen -= 2;

			// Priority
			unpackLEWord(&pDataBuf, &wPritority);
			wLen -= 2;
			debugLogA("Priority: %u", wPritority);

			// Message
			unpackLEWord(&pDataBuf, &wMsgLen);
			wLen -= 2;

			// HANDLERS
			switch (bMsgType) { // File messages, handled by the file module
			case MTYPE_FILEREQ:
				if (!dwUin) { // AIM cannot send this, just sanity
					debugLogA("Error: Malformed UIN in packet");
					return;
				}
				{
					char* szMsg = (char *)_alloca(wMsgLen + 1);
					memcpy(szMsg, pDataBuf, wMsgLen);
					szMsg[wMsgLen] = '\0';
					pDataBuf += wMsgLen;
					wLen -= wMsgLen;

					if (wAckType == 0 || wAckType == 1)
						// File requests 7
						handleFileRequest(pDataBuf, dwUin, wCookie, dwMsgID1, dwMsgID2, szMsg, 7, FALSE);
					else if (wAckType == 2)
						// File reply 7
						handleFileAck(pDataBuf, wLen, dwUin, wCookie, wStatus);
					else
						debugLogA("Ignored strange file message");
				}
				break;

			// Chat messages, handled by the chat module
			case MTYPE_CHAT:
				// TODO: this type is deprecated
				break;

			// Plugin messages, need further parsing
			case MTYPE_PLUGIN:
				if (wLen < wMsgLen) { // sanity check
					debugLogA("Error: Malformed server Greeting message");
					return;
				}

				parseServRelayPluginData(pDataBuf + wMsgLen, wLen - wMsgLen, dwUin, szUID, dwMsgID1, dwMsgID2, wAckType, bFlags, wStatus, wCookie, wVersion);
				break;

			// Everything else
			default:
				if (!dwUin) { // AIM cannot send this, just sanity
					debugLogA("Error: Malformed UIN in packet");
					return;
				}

				message_ack_params pMsgAck = { 0 };
				pMsgAck.bType = MAT_SERVER_ADVANCED;
				pMsgAck.dwUin = dwUin;
				pMsgAck.dwMsgID1 = dwMsgID1;
				pMsgAck.dwMsgID2 = dwMsgID2;
				pMsgAck.wCookie = wCookie;
				pMsgAck.msgType = bMsgType;
				pMsgAck.bFlags = bFlags;
				handleMessageTypes(dwUin, szUID, time(NULL), dwMsgID1, dwMsgID2, wCookie, wVersion, bMsgType, bFlags, wAckType, wLen, wMsgLen, (char*)pDataBuf, 0, &pMsgAck);
				break;
			}
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PSIG_INFO_PLUGIN)) { // info manager plugin - obsolete
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}

			BYTE bMsgType;
			BYTE bLevel;

			pDataBuf += 16;  /* unused stuff */
			wLen -= 16;
			unpackByte(&pDataBuf, &bMsgType);
			wLen -= 1;

			pDataBuf += 3; // unknown
			wLen -= 3;
			unpackByte(&pDataBuf, &bLevel);
			if (bLevel != 0 || wLen < 16) {
				debugLogA("Invalid %s Manager Plugin message from %u", "Info", dwUin);
				return;
			}
			unpackDWord(&pDataBuf, &dwGuid1); // plugin request GUID
			unpackDWord(&pDataBuf, &dwGuid2);
			unpackDWord(&pDataBuf, &dwGuid3);
			unpackDWord(&pDataBuf, &dwGuid4);
			wLen -= 16;

			if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PMSG_QUERY_INFO))
				debugLogA("User %u requests our %s plugin list. NOT SUPPORTED", dwUin, "info");
			else
				debugLogA("Unknown %s Manager message from %u", "Info", dwUin);
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PSIG_STATUS_PLUGIN)) { // status manager plugin - obsolete
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}

			BYTE bMsgType;
			BYTE bLevel;

			pDataBuf += 16;  /* unused stuff */
			wLen -= 16;
			unpackByte(&pDataBuf, &bMsgType);
			wLen -= 1;

			pDataBuf += 3; // unknown
			wLen -= 3;
			unpackByte(&pDataBuf, &bLevel);
			if (bLevel != 0 || wLen < 16) {
				debugLogA("Invalid %s Manager Plugin message from %u", "Status", dwUin);
				return;
			}
			unpackDWord(&pDataBuf, &dwGuid1); // plugin request GUID
			unpackDWord(&pDataBuf, &dwGuid2);
			unpackDWord(&pDataBuf, &dwGuid3);
			unpackDWord(&pDataBuf, &dwGuid4);
			wLen -= 16;

			if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PMSG_QUERY_STATUS))
				debugLogA("User %u requests our %s plugin list. NOT SUPPORTED", dwUin, "status");
			else
				debugLogA("Unknown %s Manager message from %u", "Status", dwUin);
		}
		else debugLogA("Unknown signature (%08x-%08x-%08x-%08x) in message (format 2)", dwGuid1, dwGuid2, dwGuid3, dwGuid4);
	}
	else debugLogA("Unknown wId1 (%u) in message (format 2)", wId);
}

void CIcqProto::parseServRelayPluginData(BYTE *pDataBuf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wAckType, BYTE bFlags, WORD wStatus, WORD wCookie, WORD wVersion)
{
	int nTypeId;
	WORD wFunction;

	debugLogA("Parsing Greeting message through server");

	// Message plugin identification
	if (!unpackPluginTypeId(&pDataBuf, &wLen, &nTypeId, &wFunction, FALSE)) return;

	if (wLen > 8) {
		size_t dwLengthToEnd, dwDataLen;

		// Length of remaining data
		unpackLEDWord(&pDataBuf, &dwLengthToEnd);

		// Length of message
		unpackLEDWord(&pDataBuf, &dwDataLen);
		wLen -= 8;

		if (dwDataLen > wLen)
			dwDataLen = wLen;

		if (nTypeId == MTYPE_FILEREQ && wAckType == 2) {
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}
			debugLogA("This is file ack");

			char *szMsg = (char *)_alloca(dwDataLen + 1);
			memcpy(szMsg, pDataBuf, dwDataLen);
			szMsg[dwDataLen] = '\0';
			pDataBuf += dwDataLen;
			wLen -= dwDataLen;

			handleFileAck(pDataBuf, wLen, dwUin, wCookie, wStatus);
		}
		else if (nTypeId == MTYPE_FILEREQ && wAckType == 1) {
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}
			debugLogA("This is a file request");

			char *szMsg = (char *)_alloca(dwDataLen + 1);
			memcpy(szMsg, pDataBuf, dwDataLen);
			szMsg[dwDataLen] = '\0';
			pDataBuf += dwDataLen;
			wLen -= dwDataLen;

			handleFileRequest(pDataBuf, dwUin, wCookie, dwMsgID1, dwMsgID2, szMsg, 8, FALSE);
		}
		else if (nTypeId == MTYPE_CHAT && wAckType == 1) { // TODO: this is deprecated
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}
			debugLogA("This is a chat request");

			char *szMsg = (char *)_alloca(dwDataLen + 1);
			memcpy(szMsg, pDataBuf, dwDataLen);
			szMsg[dwDataLen] = '\0';
			pDataBuf += dwDataLen;
			wLen -= dwDataLen;

			//    handleChatRequest(pDataBuf, wLen, dwUin, wCookie, dwMsgID1, dwMsgID2, szMsg, 8);
		}
		else if (nTypeId == MTYPE_STATUSMSGEXT && wFunction >= 1 && wFunction <= 3) { // handle extended status message request
			int nMsgType = 0;

			switch (wFunction) {
			case 1: // Away
				if (m_iStatus == ID_STATUS_ONLINE || m_iStatus == ID_STATUS_INVISIBLE)
					nMsgType = MTYPE_AUTOONLINE;
				else if (m_iStatus == ID_STATUS_AWAY)
					nMsgType = MTYPE_AUTOAWAY;
				else if (m_iStatus == ID_STATUS_FREECHAT)
					nMsgType = MTYPE_AUTOFFC;
				break;

			case 2: // Busy
				if (m_iStatus == ID_STATUS_OCCUPIED)
					nMsgType = MTYPE_AUTOBUSY;
				else if (m_iStatus == ID_STATUS_DND)
					nMsgType = MTYPE_AUTODND;
				break;

			case 3: // N/A
				if (m_iStatus == ID_STATUS_NA)
					nMsgType = MTYPE_AUTONA;
			}
			handleMessageTypes(dwUin, szUID, time(NULL), dwMsgID1, dwMsgID2, wCookie, wVersion, nMsgType, bFlags, wAckType, dwLengthToEnd, 0, (char*)pDataBuf, MTF_PLUGIN | MTF_STATUS_EXTENDED, NULL);
		}
		else if (nTypeId) {
			if (!dwUin) { // AIM cannot send this, just sanity
				debugLogA("Error: Malformed UIN in packet");
				return;
			}
			message_ack_params pMsgAck = { 0 };

			pMsgAck.bType = MAT_SERVER_ADVANCED;
			pMsgAck.dwUin = dwUin;
			pMsgAck.dwMsgID1 = dwMsgID1;
			pMsgAck.dwMsgID2 = dwMsgID2;
			pMsgAck.wCookie = wCookie;
			pMsgAck.msgType = nTypeId;
			pMsgAck.bFlags = bFlags;
			handleMessageTypes(dwUin, szUID, time(NULL), dwMsgID1, dwMsgID2, wCookie, wVersion, nTypeId, bFlags, wAckType, dwLengthToEnd, dwDataLen, (char*)pDataBuf, MTF_PLUGIN, &pMsgAck);
		}
		else debugLogA("Unsupported plugin message type %d", nTypeId);
	}
	else debugLogA("Error: Malformed server plugin message");
}

void CIcqProto::handleRecvServMsgContacts(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwID1, DWORD dwID2, WORD wCommand)
{
	MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

	if (wCommand == 0) { // received contacts
		if (wLen < 4) { // just check if at least one tlv is there
			debugLogA("Message (format %u) - Ignoring empty contacts message", 2);
			return;
		}
		oscar_tlv_chain *chain = readIntoTLVChain(&buf, wLen, 0);
		if (!chain) { // sanity check
			debugLogA("Message (format %u) - Invalid data", 2);
			return;
		}

		WORD wAckType = chain->getWord(0x0A, 1);

		if (wAckType == 1) { // it is really message containing contacts, parse them
			oscar_tlv *tlvUins = chain->getTLV(0x2711, 1);
			oscar_tlv *tlvNames = chain->getTLV(0x2712, 1);

			if (!tlvUins || tlvUins->wLen < 4) {
				debugLogA("Malformed '%s' message", "contacts");
				disposeChain(&chain);
				return;
			}
			int nContacts = 0x10, iContact = 0;
			ICQSEARCHRESULT **contacts = (ICQSEARCHRESULT**)SAFE_MALLOC(nContacts * sizeof(ICQSEARCHRESULT*));
			WORD wContactsGroup = 0;
			int valid = 1;
			BYTE *pBuffer = tlvUins->pData;
			int nLen = tlvUins->wLen;

			while (nLen > 2) { // parse UIDs
				if (!wContactsGroup) {
					WORD wGroupLen;

					unpackWord(&pBuffer, &wGroupLen);
					nLen -= 2;
					if (nLen >= wGroupLen + 2) {
						pBuffer += wGroupLen;
						unpackWord(&pBuffer, &wContactsGroup);
						nLen -= wGroupLen + 2;
					}
					else
						break;
				}
				else { // group parsed, UIDs waiting
					WORD wUidLen;

					unpackWord(&pBuffer, &wUidLen);
					nLen -= 2;
					if (nLen >= wUidLen) {
						char *szUid = (char*)SAFE_MALLOC(wUidLen + 1);
						unpackString(&pBuffer, szUid, wUidLen);
						nLen -= wUidLen;

						if (iContact >= nContacts) { // the list is too small, resize it
							nContacts += 0x10;
							contacts = (ICQSEARCHRESULT**)SAFE_REALLOC(contacts, nContacts * sizeof(ICQSEARCHRESULT*));
						}
						contacts[iContact] = (ICQSEARCHRESULT*)SAFE_MALLOC(sizeof(ICQSEARCHRESULT));
						contacts[iContact]->hdr.cbSize = sizeof(ICQSEARCHRESULT);
						contacts[iContact]->hdr.flags = PSR_TCHAR;
						contacts[iContact]->hdr.nick = null_strdup(_T(""));
						contacts[iContact]->hdr.id = ansi_to_tchar(szUid);

						if (IsStringUIN(szUid)) { // icq contact
							contacts[iContact]->uin = atoi(szUid);
							if (contacts[iContact]->uin == 0)
								valid = 0;
						}
						else { // aim contact
							if (!mir_strlen(szUid))
								valid = 0;
						}
						iContact++;

						SAFE_FREE(&szUid);
					}
					else {
						if (wContactsGroup) valid = 0;
						break;
					}

					wContactsGroup--;
				}
			}
			if (!iContact || !valid) {
				debugLogA("Malformed '%s' message", "contacts");
				disposeChain(&chain);
				for (int i = 0; i < iContact; i++) {
					SAFE_FREE(&contacts[i]->hdr.id);
					SAFE_FREE(&contacts[i]->hdr.nick);
					SAFE_FREE((void**)&contacts[i]);
				}
				SAFE_FREE((void**)&contacts);
				return;
			}
			nContacts = iContact;
			if (tlvNames && tlvNames->wLen >= 4) { // parse names, if available
				pBuffer = tlvNames->pData;
				nLen = tlvNames->wLen;
				iContact = 0;

				while (nLen > 2) { // parse Names
					if (!wContactsGroup) {
						WORD wGroupLen;

						unpackWord(&pBuffer, &wGroupLen);
						nLen -= 2;
						if (nLen >= wGroupLen + 2) {
							pBuffer += wGroupLen;
							unpackWord(&pBuffer, &wContactsGroup);
							nLen -= wGroupLen + 2;
						}
						else
							break;
					}
					else { // group parsed, Names waiting
						WORD wNickLen;

						unpackWord(&pBuffer, &wNickLen);
						nLen -= 2;
						if (nLen >= wNickLen) {
							WORD wNickTLV;
							size_t wNickTLVLen;
							char *pNick = NULL;

							unpackTypedTLV(pBuffer, wNickLen, 0x01, &wNickTLV, &wNickTLVLen, (LPBYTE*)&pNick);
							if (wNickTLV == 0x01) {
								SAFE_FREE(&contacts[iContact]->hdr.nick);
								contacts[iContact]->hdr.nick = utf8_to_tchar(pNick);
							}
							else
								SAFE_FREE(&pNick);
							pBuffer += wNickLen;
							nLen -= wNickLen;

							iContact++;
							if (iContact >= nContacts) break;
						}
						else
							break;

						wContactsGroup--;
					}
				}
			}

			if (!valid)
				debugLogA("Malformed '%s' message", "contacts");
			else {
				int bAdded;
				hContact = HContactFromUID(dwUin, szUID, &bAdded);

				// ack the message
				icq_sendContactsAck(dwUin, szUID, dwID1, dwID2);

				PROTORECVEVENT pre = { 0 };
				pre.timestamp = (DWORD)time(NULL);
				pre.szMessage = (char *)contacts;
				pre.lParam = nContacts;
				ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&pre);
			}

			for (int i = 0; i < iContact; i++) {
				SAFE_FREE(&contacts[i]->hdr.id);
				SAFE_FREE(&contacts[i]->hdr.nick);
				SAFE_FREE((void**)&contacts[i]);
			}
			SAFE_FREE((void**)&contacts);
		}
		else
			debugLogA("Error: Received unknown contacts message, ignoring.");
		// Clean up
		disposeChain(&chain);
	}
	else if (wCommand == 1) {
		debugLogA("Cannot handle abort messages yet... :(");
	}
	else if (wCommand == 2) { // acknowledgement
		DWORD dwCookie;
		MCONTACT hCookieContact;
		if (FindMessageCookie(dwID1, dwID2, &dwCookie, &hCookieContact, NULL)) {
			if (hCookieContact != hContact)
				debugLogA("Warning: Ack Contact does not match Cookie Contact(0x%x != 0x%x)", hContact, hCookieContact);

			ProtoBroadcastAck(hContact, ACKTYPE_CONTACTS, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);

			ReleaseCookie(dwCookie);
		}
		else debugLogA("Warning: Unexpected Contact Transfer ack from %s", strUID(dwUin, szUID));
	}
}

void CIcqProto::handleRecvServMsgType4(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwRef)
{
	WORD wTLVType;
	size_t wTLVLen;
	BYTE* pDataBuf;
	DWORD dwUin2;

	if (wLen < 2) {
		debugLogA("Message (format %u) - Ignoring empty message", 4);
		return;
	}

	// Unpack the first TLV(5)
	unpackTypedTLV(buf, wLen, 5, &wTLVType, &wTLVLen, &pDataBuf);
	debugLogA("Message (format %u) - UID: %s", 4, strUID(dwUin, szUID));

	// It must be TLV(5)
	if (wTLVType == 5) {
		BYTE bMsgType;
		BYTE bFlags;
		BYTE* pmsg = pDataBuf;
		size_t wMsgLen;

		unpackLEDWord(&pmsg, &dwUin2);

		if (dwUin2 == dwUin) {
			unpackByte(&pmsg, &bMsgType);
			unpackByte(&pmsg, &bFlags);
			unpackLEWord(&pmsg, &wMsgLen);

			if (bMsgType == 0 && wMsgLen == 1)
				debugLogA("User %u probably checks his ignore state.", dwUin);
			else {
				cookie_offline_messages *cookie;
				DWORD dwRecvTime = (DWORD)time(NULL);

				if (!(dwRef & 0x80000000) && FindCookie(dwRef, NULL, (void**)&cookie)) {
					WORD wTimeTLVType;
					size_t wTimeTLVLen;
					BYTE *pTimeTLV = NULL;

					cookie->nMessages++;

					unpackTypedTLV(buf, wLen, 0x16, &wTimeTLVType, &wTimeTLVLen, &pTimeTLV);
					if (pTimeTLV && wTimeTLVType == 0x16 && wTimeTLVLen == 4) { // found Offline timestamp
						BYTE *pBuf = pTimeTLV;
						unpackDWord(&pBuf, &dwRecvTime);
						debugLogA("Message (format %u) - Offline timestamp is %s", 4, time2text(dwRecvTime));
					}
					SAFE_FREE((void**)&pTimeTLV);
				}

				if (bMsgType == MTYPE_PLUGIN) {
					size_t wLen = wTLVLen - 8;
					int typeId;

					debugLogA("Parsing Greeting message through server");

					pmsg += wMsgLen;
					wLen -= wMsgLen;

					if (unpackPluginTypeId(&pmsg, &wLen, &typeId, NULL, FALSE) && wLen > 8) {
						size_t dwLengthToEnd, dwDataLen;

						// Length of remaining data
						unpackLEDWord(&pmsg, &dwLengthToEnd);

						// Length of message
						unpackLEDWord(&pmsg, &dwDataLen);
						wLen -= 8;

						if (dwDataLen > wLen)
							dwDataLen = wLen;

						if (typeId) {
							uid_str szUID;
							handleMessageTypes(dwUin, szUID, dwRecvTime, dwMsgID1, dwMsgID2, 0, 0, typeId, bFlags, 0, dwLengthToEnd, dwDataLen, (char*)pmsg, MTF_PLUGIN, NULL);
						}
						else debugLogA("Unsupported plugin message type %d", typeId);
					}
				}
				else {
					uid_str szUID;
					handleMessageTypes(dwUin, szUID, dwRecvTime, dwMsgID1, dwMsgID2, 0, 0, bMsgType, bFlags, 0, wTLVLen - 8, wMsgLen, (char*)pmsg, 0, NULL);
				}
			}
		}
		else debugLogA("Ignoring spoofed TYPE4 message thru server from %d", dwUin);
	}
	else debugLogA("Unsupported TLV (%u) in message (format %u)", wTLVType, 4);

	SAFE_FREE((void**)&pDataBuf);
}


//
// Helper functions
//

static int TypeGUIDToTypeId(DWORD dwGuid1, DWORD dwGuid2, DWORD dwGuid3, DWORD dwGuid4, WORD wType)
{
	int nTypeID = MTYPE_UNKNOWN;

	if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_STATUSMSGEXT)) {
		nTypeID = MTYPE_STATUSMSGEXT;
	}
	else if (wType == MGTYPE_UNDEFINED) {
		if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, PSIG_MESSAGE)) { // icq6 message ack
			nTypeID = MTYPE_PLAIN;
		}
	}
	else if (wType == MGTYPE_STANDARD_SEND) {
		if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_WEBURL)) {
			nTypeID = MTYPE_URL;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_CONTACTS)) {
			nTypeID = MTYPE_CONTACTS;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_CHAT)) {
			nTypeID = MTYPE_CHAT;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_FILE)) {
			nTypeID = MTYPE_FILEREQ;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_GREETING_CARD)) {
			nTypeID = MTYPE_GREETINGCARD;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_MESSAGE)) {
			nTypeID = MTYPE_MESSAGE;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_SMS_MESSAGE)) {
			nTypeID = MTYPE_SMS_MESSAGE;
		}
	}
	else if (wType == MGTYPE_CONTACTS_REQUEST) {
		if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_CONTACTS)) {
			nTypeID = MTYPE_REQUESTCONTACTS;
		}
		else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_XTRAZ_SCRIPT)) {
			nTypeID = MTYPE_SCRIPT_DATA;
		}
	}
	else if (CompareGUIDs(dwGuid1, dwGuid2, dwGuid3, dwGuid4, MGTYPE_XTRAZ_SCRIPT)) {
		if (wType == MGTYPE_SCRIPT_INVITATION) {
			nTypeID = MTYPE_SCRIPT_INVITATION;
		}
		else if (wType == MGTYPE_SCRIPT_NOTIFY) {
			nTypeID = MTYPE_SCRIPT_NOTIFY;
		}
	}

	return nTypeID;
}


int CIcqProto::unpackPluginTypeId(BYTE **pBuffer, size_t *pwLen, int *pTypeId, WORD *pFunctionId, BOOL bThruDC)
{
	size_t wLen = *pwLen, wInfoLen, dwPluginNameLen;
	DWORD q1, q2, q3, q4;
	WORD qt;

	if (wLen < 24)
		return 0; // Failure

	unpackLEWord(pBuffer, &wInfoLen);

	unpackDWord(pBuffer, &q1); // get data GUID & function id
	unpackDWord(pBuffer, &q2);
	unpackDWord(pBuffer, &q3);
	unpackDWord(pBuffer, &q4);
	unpackLEWord(pBuffer, &qt);
	wLen -= 20;

	if (pFunctionId) *pFunctionId = qt;

	unpackLEDWord(pBuffer, &dwPluginNameLen);
	wLen -= 4;

	if (dwPluginNameLen > wLen) { // check for malformed plugin name
		dwPluginNameLen = wLen;
		NetLog_Uni(bThruDC, "Warning: malformed size of plugin name.");
	}
	char *szPluginName = (char *)_alloca(dwPluginNameLen + 1);
	memcpy(szPluginName, *pBuffer, dwPluginNameLen);
	szPluginName[dwPluginNameLen] = '\0';
	wLen -= dwPluginNameLen;

	*pBuffer += dwPluginNameLen;

	int typeId = TypeGUIDToTypeId(q1, q2, q3, q4, qt);
	if (!typeId)
		NetLog_Uni(bThruDC, "Error: Unknown type {%08x-%08x-%08x-%08x:%04x}: %s", q1, q2, q3, q4, qt, szPluginName);

	if (wInfoLen >= 22 + dwPluginNameLen) { // sanity checking
		wInfoLen -= 22 + dwPluginNameLen;

		// check if enough data is available - skip remaining bytes of info block
		if (wLen >= wInfoLen) {
			*pBuffer += wInfoLen;
			wLen -= wInfoLen;
		}
	}

	*pwLen = wLen;
	*pTypeId = typeId;

	return 1; // Success
}


int getPluginTypeIdLen(int nTypeID)
{
	switch (nTypeID) {
	case MTYPE_SCRIPT_NOTIFY:
		return 0x51;

	case MTYPE_FILEREQ:
		return 0x2B;

	case MTYPE_AUTOONLINE:
	case MTYPE_AUTOAWAY:
	case MTYPE_AUTOBUSY:
	case MTYPE_AUTODND:
	case MTYPE_AUTOFFC:
		return 0x3C;

	case MTYPE_AUTONA:
		return 0x3B;

	default:
		return 0;
	}
}


void packPluginTypeId(icq_packet *packet, int nTypeID)
{
	switch (nTypeID) {
	case MTYPE_SCRIPT_NOTIFY:
		packLEWord(packet, 0x04f);                // Length

		packGUID(packet, MGTYPE_XTRAZ_SCRIPT);    // Message Type GUID
		packLEWord(packet, MGTYPE_SCRIPT_NOTIFY); // Function ID
		packLEDWord(packet, 0x002a);              // Request type string
		packBuffer(packet, (LPBYTE)"Script Plug-in: Remote Notification Arrive", 0x002a);

		packDWord(packet, 0x00000100);            // Unknown binary stuff
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packWord(packet, 0x0000);
		packByte(packet, 0x00);

		break;

	case MTYPE_FILEREQ:
		packLEWord(packet, 0x029);     // Length

		packGUID(packet, MGTYPE_FILE); // Message Type GUID
		packWord(packet, 0x0000);      // Unknown
		packLEDWord(packet, 0x0004);   // Request type string
		packBuffer(packet, (LPBYTE)"File", 0x0004);

		packDWord(packet, 0x00000100); // More unknown binary stuff
		packDWord(packet, 0x00010000);
		packDWord(packet, 0x00000000);
		packWord(packet, 0x0000);
		packByte(packet, 0x00);

		break;

	case MTYPE_AUTOONLINE:
	case MTYPE_AUTOAWAY:
	case MTYPE_AUTOFFC:
		packLEWord(packet, 0x03A);                // Length

		packGUID(packet, MGTYPE_STATUSMSGEXT);    // Message Type GUID

		packLEWord(packet, 1);                    // Function ID
		packLEDWord(packet, 0x13);                // Request type string
		packBuffer(packet, (LPBYTE)"Away Status Message", 0x13);

		packDWord(packet, 0x01000000);            // Unknown binary stuff
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packByte(packet, 0x00);

		break;

	case MTYPE_AUTOBUSY:
	case MTYPE_AUTODND:
		packLEWord(packet, 0x03A);                // Length

		packGUID(packet, MGTYPE_STATUSMSGEXT);    // Message Type GUID

		packLEWord(packet, 2);                    // Function ID
		packLEDWord(packet, 0x13);                // Request type string
		packBuffer(packet, (LPBYTE)"Busy Status Message", 0x13);

		packDWord(packet, 0x02000000);            // Unknown binary stuff
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packByte(packet, 0x00);

		break;

	case MTYPE_AUTONA:
		packLEWord(packet, 0x039);                // Length

		packGUID(packet, MGTYPE_STATUSMSGEXT);    // Message Type GUID

		packLEWord(packet, 3);                    // Function ID
		packLEDWord(packet, 0x12);                // Request type string
		packBuffer(packet, (LPBYTE)"N/A Status Message", 0x12);

		packDWord(packet, 0x03000000);            // Unknown binary stuff
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packDWord(packet, 0x00000000);
		packByte(packet, 0x00);

		break;
	}
}


void CIcqProto::handleStatusMsgReply(const char *szPrefix, MCONTACT hContact, DWORD dwUin, int bMsgType, WORD wCookie, const char *szMsg)
{
	if (hContact == INVALID_CONTACT_ID) {
		debugLogA("%sIgnoring status message from unknown contact %u", szPrefix, dwUin);
		return;
	}

	int status = AwayMsgTypeToStatus(bMsgType);
	if (status == ID_STATUS_OFFLINE) {
		debugLogA("%sIgnoring unknown status message from %u", szPrefix, dwUin);
		return;
	}

	// it is probably UTF-8 status reply
	PROTORECVEVENT pre = { 0 };
	pre.szMessage = (char*)szMsg;
	pre.timestamp = time(NULL);
	pre.lParam = wCookie;
	ProtoChainRecv(hContact, PSR_AWAYMSG, status, (LPARAM)&pre);
}


HANDLE CIcqProto::handleMessageAck(DWORD dwUin, char *szUID, WORD wCookie, int type, PBYTE buf, BYTE bFlags)
{
	if (bFlags == 3) {
		MCONTACT hCookieContact;
		cookie_message_data *pCookieData = NULL;

		MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

		if (!FindCookie(wCookie, &hCookieContact, (void**)&pCookieData)) {
			debugLogA("%sIgnoring unrequested status message from %u", "handleMessageAck: ", dwUin);

			ReleaseCookie(wCookie);
			return INVALID_HANDLE_VALUE;
		}

		if (hContact != hCookieContact) {
			debugLogA("%sAck Contact does not match Cookie Contact(0x%x != 0x%x)", "handleMessageAck: ", hContact, hCookieContact);

			ReleaseCookie(wCookie);
			return INVALID_HANDLE_VALUE;
		}
		ReleaseCookie(wCookie);

		handleStatusMsgReply("handleMessageAck: ", hContact, dwUin, type, wCookie, (char*)buf);
	}
	else // Should not happen
		debugLogA("%sIgnored type %u ack message (this should not happen)", "handleMessageAck: ", type);

	return INVALID_HANDLE_VALUE;
}


/* this function send all acks from handleMessageTypes */
void CIcqProto::sendMessageTypesAck(MCONTACT hContact, int bUnicode, message_ack_params *pArgs)
{
	if (pArgs) {
		if ((pArgs->msgType == MTYPE_PLAIN && !CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_MESSAGE))
			 || (pArgs->msgType == MTYPE_URL && !CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_URL))
			 || pArgs->msgType == MTYPE_CONTACTS) {
			if (pArgs->bType == MAT_SERVER_ADVANCED) { // Only ack message packets
				icq_sendAdvancedMsgAck(pArgs->dwUin, pArgs->dwMsgID1, pArgs->dwMsgID2, pArgs->wCookie, (BYTE)pArgs->msgType, pArgs->bFlags);
			}
			else if (pArgs->bType == MAT_DIRECT) { // Send acknowledgement
				icq_sendDirectMsgAck(pArgs->pDC, pArgs->wCookie, (BYTE)pArgs->msgType, pArgs->bFlags, bUnicode ? (char *)CAP_UTF8MSGS : NULL);
			}
		}
	}
}


/* this function also processes direct packets, so it should be bulletproof */
/* pMsg points to the beginning of the message */
void CIcqProto::handleMessageTypes(DWORD dwUin, char *szUID, DWORD dwTimestamp, DWORD dwMsgID, DWORD dwMsgID2, WORD wCookie, WORD wVersion, int type, int flags, WORD wAckType, size_t dwDataLen, size_t wMsgLen, char *pMsg, int nMsgFlags, message_ack_params *pAckParams)
{
	MCONTACT hContact = INVALID_CONTACT_ID;
	BOOL bThruDC = (nMsgFlags & MTF_DIRECT) == MTF_DIRECT;
	int bAdded;

	if (dwDataLen < wMsgLen) {
		NetLog_Uni(bThruDC, "Ignoring overflowed message");
		return;
	}

	if (wAckType == 2) {
		handleMessageAck(dwUin, szUID, wCookie, type, (LPBYTE)pMsg, (BYTE)flags);
		return;
	}

	char *szMsg = (char *)SAFE_MALLOC(wMsgLen + 1);
	if (wMsgLen > 0) {
		memcpy(szMsg, pMsg, wMsgLen);
		pMsg += wMsgLen;
		dwDataLen -= wMsgLen;
	}
	szMsg[wMsgLen] = '\0';


	char *pszMsgField[2 * MAX_CONTACTSSEND + 1];
	int nMsgFields = 0;

	pszMsgField[0] = szMsg;
	if (type == MTYPE_URL || type == MTYPE_AUTHREQ || type == MTYPE_ADDED || type == MTYPE_CONTACTS || type == MTYPE_EEXPRESS || type == MTYPE_WWP) {
		for (char *pszMsg = szMsg, nMsgFields = 1; *pszMsg; pszMsg++) {
			if ((BYTE)*pszMsg == 0xFE) {
				*pszMsg = '\0';
				pszMsgField[nMsgFields++] = pszMsg + 1;
				if (nMsgFields >= SIZEOF(pszMsgField))
					break;
			}
		}
	}

	switch (type) {
	case MTYPE_PLAIN:    /* plain message */
		{
			PROTORECVEVENT pre = { 0 };
			bool bUtf8 = false;

			// Check if this message is marked as UTF8 encoded
			if (dwDataLen > 12) {
				DWORD dwGuidLen = 0;
				int bDoubleMsg = 0;

				if (bThruDC) {
					DWORD dwExtraLen = *(DWORD*)pMsg;

					if (dwExtraLen < dwDataLen && !strncmp(szMsg, "{\\rtf", 5)) { // it is icq5 sending us crap, get real message from it
						WCHAR* usMsg = (WCHAR*)_alloca((dwExtraLen + 1)*sizeof(WCHAR));
						// make sure it is null-terminated
						wcsncpy(usMsg, (WCHAR*)(pMsg + 4), dwExtraLen);
						usMsg[dwExtraLen] = '\0';
						SAFE_FREE(&szMsg);
						szMsg = (char*)make_utf8_string(usMsg);

						if (!IsUnicodeAscii(usMsg, dwExtraLen))
							bUtf8 = true; // only mark real non-ascii messages as unicode

						bDoubleMsg = 1;
					}
				}

				if (!bDoubleMsg) {
					dwGuidLen = *(DWORD*)(pMsg + 8);
					dwDataLen -= 12;
					pMsg += 12;
				}

				while ((dwGuidLen >= 38) && (dwDataLen >= dwGuidLen)) {
					if (!strncmp(pMsg, CAP_UTF8MSGS, 38)) { // Found UTF8 cap, convert message to ansi
						bUtf8 = true;
						break;
					}
					else if (!strncmp(pMsg, CAP_RTFMSGS, 38)) { // Found RichText cap
						NetLog_Uni(bThruDC, "Warning: User %u sends us RichText.", dwUin);
						break;
					}

					dwGuidLen -= 38;
					dwDataLen -= 38;
					pMsg += 38;
				}
			}

			hContact = HContactFromUIN(dwUin, &bAdded);
			sendMessageTypesAck(hContact, bUtf8, pAckParams);

			if (!bUtf8 && !IsUSASCII(szMsg, mir_strlen(szMsg))) { // message is Ansi and contains national characters, create Unicode part by codepage
				char *usMsg = convertMsgToUserSpecificUtf(hContact, szMsg);
				if (usMsg) {
					SAFE_FREE(&szMsg);
					szMsg = (char*)usMsg;
				}
			}

			pre.timestamp = dwTimestamp;
			pre.szMessage = (char *)szMsg;
			ProtoChainRecvMsg(hContact, &pre);
		}
		break;

	case MTYPE_URL:
		if (nMsgFields < 2) {
			NetLog_Uni(bThruDC, "Malformed '%s' message", "URL");
			break;
		}

		hContact = HContactFromUIN(dwUin, &bAdded);
		sendMessageTypesAck(hContact, 0, pAckParams);
		{
			char *szTitle = ICQTranslateUtf(LPGEN("Incoming URL:"));
			char *szDataDescr = ansi_to_utf8(pszMsgField[0]);
			char *szDataUrl = ansi_to_utf8(pszMsgField[1]);
			char *szBlob = (char *)SAFE_MALLOC(mir_strlen(szTitle) + mir_strlen(szDataDescr) + mir_strlen(szDataUrl) + 8);
			mir_strcpy(szBlob, szTitle);
			mir_strcat(szBlob, " ");
			mir_strcat(szBlob, szDataDescr); // Description
			mir_strcat(szBlob, "\r\n");
			mir_strcat(szBlob, szDataUrl); // URL
			SAFE_FREE(&szTitle);
			SAFE_FREE(&szDataDescr);
			SAFE_FREE(&szDataUrl);

			PROTORECVEVENT pre = { 0 };
			pre.timestamp = dwTimestamp;
			pre.szMessage = (char *)szBlob;
			ProtoChainRecvMsg(hContact, &pre);
			SAFE_FREE(&szBlob);
		}
		break;

	case MTYPE_AUTHREQ:       /* auth request */
		/* format: nick FE first FE last FE email FE unk-char FE msg 00 */
		if (nMsgFields < 6) {
			debugLogA("Malformed '%s' message", "auth req");
			break;
		}
		{
			PROTORECVEVENT pre = { 0 };
			pre.timestamp = dwTimestamp;
			pre.lParam = sizeof(DWORD) * 2 + mir_strlen(pszMsgField[0]) + mir_strlen(pszMsgField[1]) + mir_strlen(pszMsgField[2]) + mir_strlen(pszMsgField[3]) + mir_strlen(pszMsgField[5]) + 5;

			// blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
			char *szBlob, *pCurBlob = szBlob = (char *)_alloca(pre.lParam);
			*(DWORD*)pCurBlob = dwUin; pCurBlob += sizeof(DWORD);
			*(DWORD*)pCurBlob = DWORD(hContact); pCurBlob += sizeof(DWORD);
			mir_strcpy((char*)pCurBlob, pszMsgField[0]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[1]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[2]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[3]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[5]);
			pre.szMessage = (char *)szBlob;
			ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
		}
		break;

	case MTYPE_ADDED:       /* 'you were added' */
		/* format: nick FE first FE last FE email 00 */
		if (nMsgFields < 4) {
			debugLogA("Malformed '%s' message", "you were added");
			break;
		}
		hContact = HContactFromUIN(dwUin, &bAdded);
		{
			// blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			size_t cbBlob = sizeof(DWORD) * 2 + mir_strlen(pszMsgField[0]) + mir_strlen(pszMsgField[1]) + mir_strlen(pszMsgField[2]) + mir_strlen(pszMsgField[3]) + 4;
			PBYTE pBlob, pCurBlob = pBlob = (PBYTE)_alloca(cbBlob);
			*(DWORD*)pCurBlob = dwUin; pCurBlob += sizeof(DWORD);
			*(DWORD*)pCurBlob = DWORD(hContact); pCurBlob += sizeof(DWORD);
			mir_strcpy((char*)pCurBlob, pszMsgField[0]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[1]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[2]); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
			mir_strcpy((char*)pCurBlob, pszMsgField[3]);
			AddEvent(NULL, EVENTTYPE_ADDED, dwTimestamp, 0, cbBlob, pBlob);
		}
		break;

	case MTYPE_CONTACTS:
		{
			char* pszNContactsEnd;
			int nContacts;
			if (nMsgFields < 3 ||
				 (nContacts = strtol(pszMsgField[0], &pszNContactsEnd, 10)) == 0 ||
				 pszNContactsEnd - pszMsgField[0] != (int)mir_strlen(pszMsgField[0]) ||
				 nMsgFields < nContacts * 2 + 1)
			{
				NetLog_Uni(bThruDC, "Malformed '%s' message", "contacts");
			}
			else {
				int valid = 1;
				ICQSEARCHRESULT** isrList = (ICQSEARCHRESULT**)_alloca(nContacts * sizeof(ICQSEARCHRESULT*));
				for (int i = 0; i < nContacts; i++) {
					isrList[i] = (ICQSEARCHRESULT*)SAFE_MALLOC(sizeof(ICQSEARCHRESULT));
					isrList[i]->hdr.cbSize = sizeof(ICQSEARCHRESULT);
					isrList[i]->hdr.flags = PSR_TCHAR;
					if (IsStringUIN(pszMsgField[1 + i * 2])) { // icq contact
						isrList[i]->uin = atoi(pszMsgField[1 + i * 2]);
						if (isrList[i]->uin == 0)
							valid = 0;
					}
					else { // aim contact
						if (!mir_strlen(pszMsgField[1 + i * 2]))
							valid = 0;
					}
					isrList[i]->hdr.id = ansi_to_tchar(pszMsgField[1 + i * 2]);
					isrList[i]->hdr.nick = ansi_to_tchar(pszMsgField[2 + i * 2]);
				}

				if (!valid)
					NetLog_Uni(bThruDC, "Malformed '%s' message", "contacts");
				else {
					hContact = HContactFromUIN(dwUin, &bAdded);
					sendMessageTypesAck(hContact, 0, pAckParams);

					PROTORECVEVENT pre = { 0 };
					pre.timestamp = dwTimestamp;
					pre.szMessage = (char *)isrList;
					pre.lParam = nContacts;
					ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&pre);
				}

				for (int i = 0; i < nContacts; i++) {
					SAFE_FREE(&isrList[i]->hdr.id);
					SAFE_FREE(&isrList[i]->hdr.nick);
					SAFE_FREE((void**)&isrList[i]);
				}
			}
		}
		break;

	case MTYPE_PLUGIN: // FIXME: this should be removed - it is never called
		hContact = NULL;

		switch (dwUin) {
		case 1111:    /* icqmail 'you've got mail' - not processed */
			break;
		}
		break;

	case MTYPE_SMS_MESSAGE:
		// it's a SMS message from a mobile - broadcast to SMS plugin
		if (dwUin != 1002) {
			NetLog_Uni(bThruDC, "Malformed '%s' message", "SMS Mobile");
			break;
		}
		debugLogA("Received SMS Mobile message");

		ProtoBroadcastAck(NULL, ICQACKTYPE_SMS, ACKRESULT_SUCCESS, NULL, (LPARAM)szMsg);
		break;

	case MTYPE_STATUSMSGEXT:
		// it's either extended StatusMsg reply from icq2003b or a IcqWebMessage
		if (dwUin == 1003)
			debugLogA("Received ICQWebMessage - NOT SUPPORTED");
		break;

	case MTYPE_WWP:
		// format: fromname FE FE FE fromemail FE unknownbyte FE 'Sender IP: xxx.xxx.xxx.xxx' 0D 0A body
		if (nMsgFields < 6)
			debugLogA("Malformed '%s' message", "web pager");
		else {
			// blob is: body(ASCIIZ), name(ASCIIZ), email(ASCIIZ)
			size_t cbBlob = mir_strlen(pszMsgField[0]) + mir_strlen(pszMsgField[3]) + mir_strlen(pszMsgField[5]) + 3;
			PBYTE pBlob, pCurBlob = pBlob = (PBYTE)_alloca(cbBlob);
			mir_strcpy((char *)pCurBlob, pszMsgField[5]); pCurBlob += mir_strlen((char *)pCurBlob) + 1;
			mir_strcpy((char *)pCurBlob, pszMsgField[0]); pCurBlob += mir_strlen((char *)pCurBlob) + 1;
			mir_strcpy((char *)pCurBlob, pszMsgField[3]);

			AddEvent(NULL, ICQEVENTTYPE_WEBPAGER, dwTimestamp, 0, cbBlob, pBlob);
		}
		break;

	case MTYPE_EEXPRESS:
		// format: fromname FE FE FE fromemail FE unknownbyte FE body
		if (nMsgFields < 6)
			debugLogA("Malformed '%s' message", "e-mail express");
		else {
			// blob is: body(ASCIIZ), name(ASCIIZ), email(ASCIIZ)
			size_t cbBlob = mir_strlen(pszMsgField[0]) + mir_strlen(pszMsgField[3]) + mir_strlen(pszMsgField[5]) + 3;
			PBYTE pBlob, pCurBlob = pBlob = (PBYTE)_alloca(cbBlob);
			mir_strcpy((char *)pCurBlob, pszMsgField[5]); pCurBlob += mir_strlen((char *)pCurBlob) + 1;
			mir_strcpy((char *)pCurBlob, pszMsgField[0]); pCurBlob += mir_strlen((char *)pCurBlob) + 1;
			mir_strcpy((char *)pCurBlob, pszMsgField[3]);

			AddEvent(NULL, ICQEVENTTYPE_EMAILEXPRESS, dwTimestamp, 0, cbBlob, pBlob);
		}
		break;

	case MTYPE_REQUESTCONTACTS:
		/* it's a contacts-request */
		NetLog_Uni(bThruDC, "Received %s from %u", "Request for Contacts", dwUin);
		break;

	case MTYPE_GREETINGCARD:
		/* it's a greeting card */
		NetLog_Uni(bThruDC, "Received %s from %u", "Greeting Card", dwUin);
		break;

	case MTYPE_SCRIPT_NOTIFY:
		/* it's a xtraz notify request */
		NetLog_Uni(bThruDC, "Received %s from %u", "Xtraz Notify Request", dwUin);
		handleXtrazNotify(dwUin, dwMsgID, dwMsgID2, wCookie, szMsg, wMsgLen, bThruDC);
		break;

	case MTYPE_SCRIPT_INVITATION:
		/* it's a xtraz invitation to session */
		NetLog_Uni(bThruDC, "Received %s from %u", "Xtraz Invitation", dwUin);
		handleXtrazInvitation(dwUin, szMsg, bThruDC);
		break;

	case MTYPE_SCRIPT_DATA:
		/* it's a xtraz data packet */
		NetLog_Uni(bThruDC, "Received %s from %u", "Xtraz data packet", dwUin);
		handleXtrazData(dwUin, szMsg, bThruDC);
		break;

	case MTYPE_AUTOONLINE:
	case MTYPE_AUTOAWAY:
	case MTYPE_AUTOBUSY:
	case MTYPE_AUTONA:
	case MTYPE_AUTODND:
	case MTYPE_AUTOFFC:
	{
		char **szMsg = MirandaStatusToAwayMsg(AwayMsgTypeToStatus(type));
		if (szMsg) {
			struct rates_status_message_response : public rates_queue_item
			{
			protected:
				virtual rates_queue_item* copyItem(rates_queue_item *aDest = NULL)
				{
					rates_status_message_response *pDest = (rates_status_message_response*)aDest;
					if (!pDest)
						pDest = new rates_status_message_response(ppro, wGroup);

					pDest->bExtended = bExtended;
					pDest->dwMsgID1 = dwMsgID1;
					pDest->dwMsgID2 = dwMsgID2;
					pDest->wCookie = wCookie;
					pDest->wVersion = wVersion;
					pDest->nMsgType = nMsgType;

					return rates_queue_item::copyItem(pDest);
				};
			public:
				rates_status_message_response(CIcqProto *ppro, WORD wGroup) : rates_queue_item(ppro, wGroup) {};
				virtual ~rates_status_message_response() {};

				virtual void execute()
				{
					char **pszMsg = ppro->MirandaStatusToAwayMsg(AwayMsgTypeToStatus(nMsgType));
					if (bExtended)
						ppro->icq_sendAwayMsgReplyServExt(dwUin, szUid, dwMsgID1, dwMsgID2, wCookie, wVersion, nMsgType, pszMsg);
					else if (dwUin)
						ppro->icq_sendAwayMsgReplyServ(dwUin, dwMsgID1, dwMsgID2, wCookie, wVersion, (BYTE)nMsgType, pszMsg);
					else
						ppro->debugLogA("Error: Malformed UIN in packet");
				};

				BOOL bExtended;
				DWORD dwMsgID1;
				DWORD dwMsgID2;
				WORD wCookie;
				WORD wVersion;
				int nMsgType;
			};

			WORD wGroup;
			{
				mir_cslock l(m_ratesMutex);
				wGroup = m_rates->getGroupFromSNAC(ICQ_MSG_FAMILY, ICQ_MSG_RESPONSE);
			}

			rates_status_message_response rr(this, wGroup);
			rr.bExtended = (nMsgFlags & MTF_STATUS_EXTENDED) == MTF_STATUS_EXTENDED;
			rr.hContact = hContact;
			rr.dwUin = dwUin;
			rr.szUid = szUID;
			rr.dwMsgID1 = dwMsgID;
			rr.dwMsgID2 = dwMsgID2;
			rr.wCookie = wCookie;
			rr.wVersion = wVersion;
			rr.nMsgType = type;

			handleRateItem(&rr, RQT_RESPONSE);
		}

		break;
	}

	case MTYPE_FILEREQ: // Never happens
	default:
		NetLog_Uni(bThruDC, "Unprocessed message type %d", type);
		break;

	}

	SAFE_FREE(&szMsg);
}

void CIcqProto::handleRecvMsgResponse(BYTE *buf, size_t wLen)
{
	DWORD dwCookie;
	WORD wMessageFormat;
	WORD wStatus;
	WORD bMsgType = 0;
	BYTE bFlags;
	WORD wLength;
	WORD wVersion = 0;
	cookie_message_data *pCookieData = NULL;

	DWORD dwMsgID1, dwMsgID2;
	unpackLEDWord(&buf, &dwMsgID1);  // Message ID
	unpackLEDWord(&buf, &dwMsgID2);
	wLen -= 8;

	unpackWord(&buf, &wMessageFormat);
	wLen -= 2;
	if (wMessageFormat != 2) {
		debugLogA("SNAC(4.B) Unknown type");
		return;
	}

	DWORD dwUin;
	uid_str szUid;
	if (!unpackUID(&buf, &wLen, &dwUin, &szUid))
		return;

	MCONTACT hContact = HContactFromUID(dwUin, szUid, NULL);

	buf += 2;   // 3. unknown
	wLen -= 2;

	MCONTACT hCookieContact;
	if (!FindMessageCookie(dwMsgID1, dwMsgID2, &dwCookie, &hCookieContact, &pCookieData)) {
		debugLogA("SNAC(4.B) Received an ack that I did not ask for from (%u)", dwUin);
		return;
	}

	if (IsValidOscarTransfer(pCookieData)) {
		// it is OFT response
		handleRecvServResponseOFT(buf, wLen, dwUin, szUid, pCookieData);
		return;
	}

	if (!dwUin) {
		// AIM cannot send this - just sanity
		debugLogA("Error: Invalid UID in message response.");
		return;
	}

	// Length of sub chunk?
	if (wLen >= 2) {
		unpackLEWord(&buf, &wLength);
		wLen -= 2;
	}
	else wLength = 0;

	if (wLength == 0x1b && pCookieData->bMessageType != MTYPE_REVERSE_REQUEST) {
		// this can be v8 greeting message reply
		unpackLEWord(&buf, &wVersion);
		buf += 27;  /* unknowns from the msg we sent */
		wLen -= 29;

		// Message sequence (SEQ2)
		WORD wCookie;
		unpackLEWord(&buf, &wCookie);
		wLen -= 2;

		// Unknown (12 bytes)
		buf += 12;
		wLen -= 12;

		// Message type
		unpackByte(&buf, (BYTE*)&bMsgType);
		unpackByte(&buf, &bFlags);
		wLen -= 2;

		// Status
		unpackLEWord(&buf, &wStatus);
		wLen -= 2;

		// Priority?
		buf += 2;
		wLen -= 2;

		if (!FindCookie(wCookie, &hCookieContact, (void**)&pCookieData)) {
			// use old reliable method
			debugLogA("Warning: Invalid cookie in %s from (%u)", "message response", dwUin);

			if (pCookieData->bMessageType != MTYPE_AUTOAWAY && bFlags == 3) { // most probably a broken ack of some kind (e.g. from R&Q), try to fix that
				bMsgType = pCookieData->bMessageType;
				bFlags = 0;

				debugLogA("Warning: Invalid message type in %s from (%u)", "message response", dwUin);
			}
		}
		else if (bMsgType != MTYPE_PLUGIN && pCookieData->bMessageType != MTYPE_AUTOAWAY) {
			// just because some clients break it...
			dwCookie = wCookie;

			if (bMsgType != pCookieData->bMessageType)
				debugLogA("Warning: Invalid message type in %s from (%u)", "message response", dwUin);

			bMsgType = pCookieData->bMessageType;
		}
		else if (pCookieData->bMessageType == MTYPE_AUTOAWAY && bMsgType != MTYPE_PLUGIN) {
			if (bMsgType != pCookieData->nAckType)
				debugLogA("Warning: Invalid message type in %s from (%u)", "message response", dwUin);
		}
	}
	else {
		bMsgType = pCookieData->bMessageType;
		bFlags = 0;
		wStatus = 0;
	}

	if (hCookieContact != hContact) {
		debugLogA("SNAC(4.B) Ack Contact does not match Cookie Contact(0x%x != 0x%x)", hContact, hCookieContact);
		ReleaseCookie(dwCookie); // This could be a bad idea, but I think it is safe
		return;
	}

	if (bFlags == 3)     // A status message reply
		handleStatusMsgReply("SNAC(4.B) ", hContact, dwUin, bMsgType, dwCookie, (char*)(buf + 2));
	else {
		// An ack of some kind
		int ackType;

		if (hContact == NULL || hContact == INVALID_CONTACT_ID) {
			debugLogA("SNAC(4.B) Message from unknown contact (%u)", dwUin);
			ReleaseCookie(dwCookie); // This could be a bad idea, but I think it is safe
			return;
		}

		WORD wMsgLen;
		switch (bMsgType) {
		case MTYPE_FILEREQ:
			// Message length
			unpackLEWord(&buf, &wMsgLen);
			wLen -= 2;
			{
				char *szMsg = (char *)_alloca(wMsgLen + 1);
				szMsg[wMsgLen] = '\0';
				if (wMsgLen > 0) {
					memcpy(szMsg, buf, wMsgLen);
					buf += wMsgLen;
					wLen -= wMsgLen;
				}
				handleFileAck(buf, wLen, dwUin, dwCookie, wStatus);
				// No success protoack will be sent here, since all file requests
				// will have been 'sent' when the server returns its ack
			}
			return;

		case MTYPE_PLUGIN:
			if (wLength != 0x1B) {
				debugLogA("Invalid Greeting %s", "message response");

				ReleaseCookie(dwCookie);
				return;
			}

			debugLogA("Parsing Greeting %s", "message response");

			// Message
			unpackLEWord(&buf, &wMsgLen);
			wLen -= 2;
			buf += wMsgLen;
			wLen -= wMsgLen;

			// This packet is malformed. Possibly a file accept from Miranda IM 0.1.2.1
			if (wLen < 20) {
				ReleaseCookie(dwCookie);
				return;
			}
			{
				WORD wFunctionId;
				int typeId;
				if (!unpackPluginTypeId(&buf, &wLen, &typeId, &wFunctionId, FALSE)) {
					ReleaseCookie(dwCookie);
					return;
				}

				if (wLen < 4) {
					debugLogA("Error: Invalid greeting %s", "message response");
					ReleaseCookie(dwCookie);
					return;
				}

				// Length of remaining data
				DWORD dwLengthToEnd;
				unpackLEDWord(&buf, &dwLengthToEnd);
				wLen -= 4;

				DWORD dwDataLen;
				if (wLen >= 4 && dwLengthToEnd > 0)
					unpackLEDWord(&buf, &dwDataLen); // Length of message
				else
					dwDataLen = 0;

				switch (typeId) {
				case MTYPE_PLAIN:
					if (pCookieData && pCookieData->bMessageType == MTYPE_AUTOAWAY && dwLengthToEnd >= 4) {
						// ICQ 6 invented this
						char *szMsg = (char*)_alloca(dwDataLen + 1);

						if (dwDataLen > 0)
							memcpy(szMsg, buf, dwDataLen);
						szMsg[dwDataLen] = '\0';
						handleStatusMsgReply("SNAC(4.B) ", hContact, dwUin, pCookieData->nAckType, dwCookie, szMsg);

						ReleaseCookie(dwCookie);
						return;
					}
					ackType = ACKTYPE_MESSAGE;
					break;

				case MTYPE_URL:
					ackType = ACKTYPE_URL;
					break;

				case MTYPE_CONTACTS:
					ackType = ACKTYPE_CONTACTS;
					break;

				case MTYPE_FILEREQ:
					debugLogA("This is file ack");
					{
						char *szMsg = (char *)_alloca(dwDataLen + 1);

						if (dwDataLen > 0)
							memcpy(szMsg, buf, dwDataLen);
						szMsg[dwDataLen] = '\0';
						buf += dwDataLen;
						wLen -= dwDataLen;

						handleFileAck(buf, wLen, dwUin, dwCookie, wStatus);
						// No success protoack will be sent here, since all file requests
						// will have been 'sent' when the server returns its ack
					}
					return;

				case MTYPE_SCRIPT_NOTIFY:
					{
						char *szMsg = (char*)_alloca(dwDataLen + 1);

						if (dwDataLen > 0)
							memcpy(szMsg, buf, dwDataLen);
						szMsg[dwDataLen] = '\0';

						handleXtrazNotifyResponse(hContact, (WORD)dwCookie, szMsg);

						ReleaseCookie(dwCookie);
					}
					return;

				case MTYPE_STATUSMSGEXT:
					{ // handle Away Message response (ICQ 6)
						char *szMsg = (char*)SAFE_MALLOC(dwDataLen + 1);

						if (dwDataLen > 0)
							memcpy(szMsg, buf, dwDataLen);
						szMsg[dwDataLen] = '\0';
						szMsg = EliminateHtml(szMsg, dwDataLen);

						handleStatusMsgReply("SNAC(4.B) ", hContact, dwUin, pCookieData->nAckType, (WORD)dwCookie, szMsg);

						SAFE_FREE(&szMsg);

						ReleaseCookie(dwCookie);
					}
					return;

				default:
					debugLogA("Error: Unknown plugin message response, type %d.", typeId);
					return;
				}
			}
			break;

		case MTYPE_PLAIN:
			ackType = ACKTYPE_MESSAGE;
			break;

		case MTYPE_URL:
			ackType = ACKTYPE_URL;
			break;

		case MTYPE_AUTHOK:
		case MTYPE_AUTHDENY:
			ackType = ACKTYPE_AUTHREQ;
			break;

		case MTYPE_ADDED:
			ackType = ACKTYPE_ADDED;
			break;

		case MTYPE_CONTACTS:
			ackType = ACKTYPE_CONTACTS;
			break;

		case MTYPE_REVERSE_REQUEST:
		{
			cookie_reverse_connect *pReverse = (cookie_reverse_connect*)pCookieData;

			if (pReverse->ft) {
				filetransfer *ft = (filetransfer*)pReverse->ft;

				ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			}
			debugLogA("Reverse Connect request failed");
			// Set DC status to failed
			setByte(hContact, "DCStatus", 2);

			ReleaseCookie(dwCookie);
		}
			return;

		case MTYPE_CHAT:
		default:
			debugLogA("SNAC(4.B) Unknown message type (%u) in switch", bMsgType);
			return;
		}

		if ((ackType == MTYPE_PLAIN && pCookieData && (pCookieData->nAckType == ACKTYPE_CLIENT)) || ackType != MTYPE_PLAIN)
			ProtoBroadcastAck(hContact, ackType, ACKRESULT_SUCCESS, (HANDLE)(WORD)dwCookie, 0);
	}

	ReleaseCookie(dwCookie);
}

// A response to a CLI_SENDMSG
void CIcqProto::handleRecvServMsgError(BYTE *buf, size_t wLen, DWORD dwSequence)
{
	WORD wError;
	char *pszErrorMessage;
	MCONTACT hContact;
	cookie_message_data *pCookieData = NULL;
	int nMessageType;


	if (wLen < 2)
		return;

	if (FindCookie((WORD)dwSequence, &hContact, (void**)&pCookieData)) {
		// all packet cookies from msg family has command 0 in the queue
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(hContact, &dwUin, &szUid)) {
			// Invalid contact
			FreeCookie((WORD)dwSequence);
			return;
		}

		// Error code
		unpackWord(&buf, &wError);

		if (wError == 9 && pCookieData->bMessageType == MTYPE_AUTOAWAY) {
			// we failed to request away message the normal way, try it AIM way
			icq_packet packet;

			serverPacketInit(&packet, 13 + getUINLen(dwUin));
			packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_REQ_USER_INFO, 0, (WORD)dwSequence);
			packWord(&packet, 0x03);
			packUIN(&packet, dwUin);

			sendServPacket(&packet);
			return;
		}

		// Not all of these are actually used in family 4
		// This will be moved into a special error handling function later
		switch (wError) {
		case 0x0002:     // Server rate limit exceeded
			pszErrorMessage = Translate("You are sending too fast. Wait a while and try again.\r\nSNAC(4.1) Error x02");
			break;

		case 0x0003:     // Client rate limit exceeded
			pszErrorMessage = Translate("You are sending too fast. Wait a while and try again.\r\nSNAC(4.1) Error x03");
			break;

		case 0x0004:     // Recipient is not logged in (resend in a offline message)
			if (pCookieData->bMessageType == MTYPE_PLAIN) {
				if (pCookieData->isOffline) {
					// offline failed - most probably to AIM contact
					pszErrorMessage = Translate("The contact does not support receiving offline messages.");
					break;
				}
				// TODO: this needs better solution
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
			}
			pszErrorMessage = Translate("The user has logged off. Select 'Retry' to send an offline message.\r\nSNAC(4.1) Error x04");
			break;

		case 0x0005:     // Requested service unavailable
			pszErrorMessage = Translate("The messaging service is temporarily unavailable. Wait a while and try again.\r\nSNAC(4.1) Error x05");
			break;

		case 0x0009:     // Not supported by client (resend in a simpler format)
			pszErrorMessage = Translate("The receiving client does not support this type of message.\r\nSNAC(4.1) Error x09");
			break;

		case 0x000A:     // Refused by client
			pszErrorMessage = Translate("You sent too long message. The receiving client does not support it.\r\nSNAC(4.1) Error x0A");
			break;

		case 0x000E:     // Incorrect SNAC format
			pszErrorMessage = Translate("The SNAC format was rejected by the server.\nSNAC(4.1) Error x0E");
			break;

		case 0x0013:     // User temporarily unavailable
			pszErrorMessage = Translate("The user is temporarily unavailable. Wait a while and try again.\r\nSNAC(4.1) Error x13");
			break;

		case 0x0001:     // Invalid SNAC header
		case 0x0006:     // Requested service not defined
		case 0x0007:     // You sent obsolete SNAC
		case 0x0008:     // Not supported by server
		case 0x000B:     // Reply too big
		case 0x000C:     // Responses lost
		case 0x000D:     // Request denied
		case 0x000F:     // Insufficient rights
		case 0x0010:     // In local permit/deny (recipient blocked)
		case 0x0011:     // Sender too evil
		case 0x0012:     // Receiver too evil
		case 0x0014:     // No match
		case 0x0015:     // List overflow
		case 0x0016:     // Request ambiguous
		case 0x0017:     // Server queue full
		case 0x0018:     // Not while on AOL
		default:
			if (pszErrorMessage = (char*)_alloca(256))
				mir_snprintf(pszErrorMessage, 256, Translate("SNAC(4.1) SENDMSG Error (x%02x)"), wError);
			break;
		}

		switch (pCookieData->bMessageType) {
		case MTYPE_PLAIN:
			nMessageType = ACKTYPE_MESSAGE;
			break;

		case MTYPE_CHAT:
			nMessageType = ACKTYPE_CHAT;
			break;

		case MTYPE_FILEREQ:
			nMessageType = ACKTYPE_FILE;
			break;

		case MTYPE_URL:
			nMessageType = ACKTYPE_URL;
			break;

		case MTYPE_CONTACTS:
			nMessageType = ACKTYPE_CONTACTS;
			break;

		default:
			nMessageType = -1;
			break;
		}

		if (nMessageType != -1)
			ProtoBroadcastAck(hContact, nMessageType, ACKRESULT_FAILED, (HANDLE)(WORD)dwSequence, (LPARAM)pszErrorMessage);
		else
			debugLogA("Error: Message delivery to %u failed: %s", dwUin, pszErrorMessage);

		FreeCookie((WORD)dwSequence);

		if (pCookieData->bMessageType != MTYPE_FILEREQ)
			SAFE_FREE((void**)&pCookieData);
	}
	else {
		unpackWord(&buf, &wError);
		LogFamilyError(ICQ_MSG_FAMILY, wError);
	}
}


void CIcqProto::handleServerAck(BYTE *buf, size_t wLen, DWORD dwSequence)
{
	DWORD dwUin;
	uid_str szUID;
	WORD wChannel;
	cookie_message_data *pCookieData;

	if (wLen < 13) {
		debugLogA("Ignoring SNAC(4,C) Packet to short");
		return;
	}

	buf += 8; // Skip first 8 bytes
	wLen -= 8;

	// Message channel
	unpackWord(&buf, &wChannel);
	wLen -= 2;

	// Sender
	if (!unpackUID(&buf, &wLen, &dwUin, &szUID)) return;

	MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

	if (FindCookie((WORD)dwSequence, NULL, (void**)&pCookieData)) {
		// If the user requested a full ack, the
		// server ack should be ignored here.
		if (pCookieData && (pCookieData->nAckType == ACKTYPE_SERVER)) {
			if ((hContact != NULL) && (hContact != INVALID_CONTACT_ID)) {
				int ackType;
				int ackRes = ACKRESULT_SUCCESS;

				switch (pCookieData->bMessageType) {
				case MTYPE_PLAIN:
					ackType = ACKTYPE_MESSAGE;
					break;

				case MTYPE_CONTACTS:
					ackType = ACKTYPE_CONTACTS;
					break;

				case MTYPE_URL:
					ackType = ACKTYPE_URL;
					break;

				case MTYPE_FILEREQ:
					ackType = ACKTYPE_FILE;
					ackRes = ACKRESULT_SENTREQUEST;
					// Note 1: We are not allowed to free the cookie here because it
					// contains the filetransfer struct that we will need later
					// Note 2: The cookiedata is NOT a message_cookie_data*, it is a
					// filetransfer*. IMPORTANT! (it's one of those silly things)
					break;

				default:
					ackType = -1;
					debugLogA("Error: Unknown message type %d in ack", pCookieData->bMessageType);
					break;
				}
				if (ackType != -1)
					ProtoBroadcastAck(hContact, ackType, ackRes, (HANDLE)(WORD)dwSequence, 0);

				if (pCookieData->bMessageType != MTYPE_FILEREQ)
					SAFE_FREE((void**)&pCookieData); // this could be a bad idea, but I think it is safe
			}
			FreeCookie((WORD)dwSequence);
		}
		else if (pCookieData && (pCookieData->nAckType == ACKTYPE_CLIENT))
			debugLogA("Received a server ack, waiting for client ack.");
		else
			debugLogA("Ignored a server ack I did not ask for");
	}
}


void CIcqProto::handleMissedMsg(BYTE *buf, size_t wLen)
{
	DWORD dwUin;
	uid_str szUid;
	WORD wChannel;
	WORD wWarningLevel;
	WORD wCount;
	WORD wError;
	WORD wTLVCount;

	if (wLen < 14)
		return; // Too short

	// Message channel?
	unpackWord(&buf, &wChannel);
	wLen -= 2;

	// Sender
	if (!unpackUID(&buf, &wLen, &dwUin, &szUid)) return;

	if (wLen < 8)
		return; // Too short

	// Warning level?
	unpackWord(&buf, &wWarningLevel);
	wLen -= 2;

	// TLV count
	unpackWord(&buf, &wTLVCount);
	wLen -= 2;

	// Read past user info TLVs
	oscar_tlv_chain *pChain = readIntoTLVChain(&buf, wLen - 4, wTLVCount);
	if (pChain)
		disposeChain(&pChain);

	if (wLen < 4)
		return; // Too short

	// Number of missed messages
	unpackWord(&buf, &wCount);
	wLen -= 2;

	// Error code
	unpackWord(&buf, &wError);
	wLen -= 2;

	{ // offline retrieval process in progress, note that we received missed message notification
		cookie_offline_messages *cookie;

		if (FindCookieByType(CKT_OFFLINEMESSAGE, NULL, NULL, (void**)&cookie))
			cookie->nMissed++;
	}

	switch (wError) {

	case 0: // The message was invalid
	case 1: // The message was too long
	case 2: // The sender has flooded the server
	case 4: // You are too evil
		break;

	default:
		// 3 = Sender too evil (sender warn level > your max_msg_sevil)
		return;
		break;
	}

	// Create event to notify user
	int bAdded;

	AddEvent(HContactFromUID(dwUin, szUid, &bAdded), ICQEVENTTYPE_MISSEDMESSAGE, time(NULL), 0, sizeof(wError), (PBYTE)&wError);
}


void CIcqProto::handleOffineMessagesReply(DWORD dwRef)
{
	cookie_offline_messages *cookie;

	if (FindCookie(dwRef, NULL, (void**)&cookie)) {
		debugLogA("End of offline msgs, %u received", cookie->nMessages);
		if (cookie->nMissed) {	// NASTY WORKAROUND!!
			// The ICQ server has a bug that causes offline messages to be received again and again when some
			// missed message notification is present (most probably it is not processed correctly and causes
			// the server to fail the purging process); try to purge them using the old offline messages
			// protocol.  2008/05/21
			debugLogA("Warning: Received %u missed message notifications, trying to fix the server.", cookie->nMissed);

			icq_packet packet;
			// This will delete the messages stored on server
			serverPacketInit(&packet, 24);
			packFNACHeader(&packet, ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQUEST);
			packWord(&packet, 1);             // TLV Type
			packWord(&packet, 10);            // TLV Length
			packLEWord(&packet, 8);           // Data length
			packLEDWord(&packet, m_dwLocalUIN); // My UIN
			packLEWord(&packet, CLI_DELETE_OFFLINE_MSGS_REQ); // Ack offline msgs
			packLEWord(&packet, 0x0000);      // Request sequence number (we dont use this for now)

			// Send it
			sendServPacket(&packet);
		}

		ReleaseCookie(dwRef);
	}
	else debugLogA("Error: Received unexpected end of offline msgs.");
}


void CIcqProto::handleTypingNotification(BYTE *buf, size_t wLen)
{
	if (wLen < 14) {
		debugLogA("Ignoring SNAC(4.x11) Packet to short");
		return;
	}

	// The message ID, unused?
	buf += 8;
	wLen -= 8;

	// Message channel, unused?
	WORD wChannel;
	unpackWord(&buf, &wChannel);
	wLen -= 2;

	// Sender
	DWORD dwUin;
	uid_str szUid;
	if (!unpackUID(&buf, &wLen, &dwUin, &szUid))
		return;

	MCONTACT hContact = HContactFromUID(dwUin, szUid, NULL);

	if (hContact == INVALID_CONTACT_ID) return;

	// Typing notification code
	WORD wNotification;
	unpackWord(&buf, &wNotification);
	wLen -= 2;

	SetContactCapabilities(hContact, CAPF_TYPING);

	// Notify user
	switch (wNotification) {
	case MTN_FINISHED:
	case MTN_TYPED:
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
		debugLogA("%s has stopped typing (ch %u).", strUID(dwUin, szUid), wChannel);
		break;

	case MTN_BEGUN:
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)60);
		debugLogA("%s is typing a message (ch %u).", strUID(dwUin, szUid), wChannel);
		break;

	case MTN_WINDOW_CLOSED:
		{
			char szFormat[MAX_PATH];
			char szMsg[MAX_PATH];
			char *nick = NickFromHandleUtf(hContact);

			mir_snprintf(szMsg, SIZEOF(szMsg), ICQTranslateUtfStatic(LPGEN("Contact \"%s\" has closed the message window."), szFormat, MAX_PATH), nick);
			ShowPopupMsg(hContact, ICQTranslateUtfStatic(LPGEN("ICQ Note"), szFormat, MAX_PATH), szMsg, LOG_NOTE);
			SAFE_FREE((void**)&nick);

			debugLogA("%s has closed the message window.", strUID(dwUin, szUid));
		}
		break;

	default:
		debugLogA("Unknown typing notification from %s, type %u (ch %u)", strUID(dwUin, szUid), wNotification, wChannel);
		break;
	}
}


void CIcqProto::sendTypingNotification(MCONTACT hContact, WORD wMTNCode)
{
	_ASSERTE((wMTNCode == MTN_FINISHED) || (wMTNCode == MTN_TYPED) || (wMTNCode == MTN_BEGUN) || (wMTNCode == MTN_WINDOW_CLOSED));

	DWORD dwUin;
	uid_str szUid;
	if (getContactUid(hContact, &dwUin, &szUid))
		return; // Invalid contact

	size_t wLen = getUIDLen(dwUin, szUid);

	icq_packet packet;
	serverPacketInit(&packet, 23 + wLen);
	packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_MTN);
	packLEDWord(&packet, 0x0000);          // Msg ID
	packLEDWord(&packet, 0x0000);          // Msg ID
	packWord(&packet, 0x01);               // Channel
	packUID(&packet, dwUin, szUid);        // User ID
	packWord(&packet, wMTNCode);           // Notification type

	sendServPacketAsync(&packet);
}
