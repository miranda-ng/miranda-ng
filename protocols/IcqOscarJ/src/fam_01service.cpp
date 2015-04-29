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
//  Handles packets from Service family
// -----------------------------------------------------------------------------

#include "stdafx.h"

extern capstr capXStatus[];

void CIcqProto::handleServiceFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader, serverthread_info *info)
{
	icq_packet packet;

	switch (pSnacHeader->wSubtype) {

	case ICQ_SERVER_READY:
		debugLogA("Server is ready and is requesting my Family versions");
		debugLogA("Sending my Families");

		// This packet is a response to SRV_FAMILIES SNAC(1,3).
		// This tells the server which SNAC families and their corresponding
		// versions which the client understands. This also seems to identify
		// the client as an ICQ vice AIM client to the server.
		// Miranda mimics the behaviour of ICQ 6
		serverPacketInit(&packet, 54);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_FAMILIES);
		packDWord(&packet, 0x00220001);
		packDWord(&packet, 0x00010004);
		packDWord(&packet, 0x00130004);
		packDWord(&packet, 0x00020001);
		packDWord(&packet, 0x00030001);
		packDWord(&packet, 0x00150001);
		packDWord(&packet, 0x00040001);
		packDWord(&packet, 0x00060001);
		packDWord(&packet, 0x00090001);
		packDWord(&packet, 0x000a0001);
		packDWord(&packet, 0x000b0001);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_FAMILIES2:
		/* This is a reply to CLI_FAMILIES and it tells the client which families and their versions that this server understands.
		* We send a rate request packet */
		debugLogA("Server told me his Family versions");
		debugLogA("Requesting Rate Information");

		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQ_RATE_INFO);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_RATE_INFO:
		debugLogA("Server sent Rate Info");

		/* init rates management */
		m_rates = new rates(this, pBuffer, wBufferLength);

		/* ack rate levels */
		debugLogA("Sending Rate Info Ack");

		m_rates->initAckPacket(&packet);
		sendServPacket(&packet);

		/* CLI_REQINFO - This command requests from the server certain information about the client that is stored on the server. */
		debugLogA("Sending CLI_REQINFO");

		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQINFO);
		sendServPacket(&packet);

		if (m_bSsiEnabled) {
			cookie_servlist_action* ack;
			DWORD dwCookie;

			DWORD dwLastUpdate = getDword("SrvLastUpdate", 0);
			WORD wRecordCount = getWord("SrvRecordCount", 0);

			// CLI_REQLISTS - we want to use SSI
			debugLogA("Requesting roster rights");

			serverPacketInit(&packet, 16);
			packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQLISTS);
			packTLVWord(&packet, 0x0B, 0x000F); // mimic ICQ 6
			sendServPacket(&packet);

			if (!wRecordCount) { // CLI_REQROSTER
				// we do not have any data - request full list
				debugLogA("Requesting full roster");

				serverPacketInit(&packet, 10);
				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack) { // we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_REQUEST, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_REQUEST << 0x10;

				packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQUEST, 0, dwCookie);
				sendServPacket(&packet);
			}
			else { // CLI_CHECKROSTER
				debugLogA("Requesting roster check");

				serverPacketInit(&packet, 16);
				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack)  { // TODO: rewrite - use get list service for empty list
					// we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_CHECK, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_CHECK << 0x10;

				packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_CHECK, 0, dwCookie);
				// check if it was not changed elsewhere (force reload, set that setting to zero)
				if (IsServerGroupsDefined()) {
					packDWord(&packet, dwLastUpdate);  // last saved time
					packWord(&packet, wRecordCount);   // number of records saved
				}
				else { // we need to get groups info into DB, force receive list
					packDWord(&packet, 0);  // last saved time
					packWord(&packet, 0);   // number of records saved
				}
				sendServPacket(&packet);
			}
		}

		// CLI_REQLOCATION
		debugLogA("Requesting Location rights");

		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_CLI_REQ_RIGHTS);
		sendServPacket(&packet);

		// CLI_REQBUDDY
		debugLogA("Requesting Client-side contactlist rights");

		serverPacketInit(&packet, 16);
		packFNACHeader(&packet, ICQ_BUDDY_FAMILY, ICQ_USER_CLI_REQBUDDY);
		// Query flags: 1 = Enable Avatars
		//              2 = Enable offline status message notification
		//              4 = Enable Avatars for offline contacts
		//              8 = Use reject for not authorized contacts
		packTLVWord(&packet, 0x05, 0x0007);
		sendServPacket(&packet);

		// CLI_REQICBM
		debugLogA("Sending CLI_REQICBM");

		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQICBM);
		sendServPacket(&packet);

		// CLI_REQBOS
		debugLogA("Sending CLI_REQBOS");

		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_BOS_FAMILY, ICQ_PRIVACY_REQ_RIGHTS);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_PAUSE:
		debugLogA("Server is going down in a few seconds... (Flags: %u)", pSnacHeader->wFlags);
		// This is the list of groups that we want to have on the next server
		serverPacketInit(&packet, 30);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_PAUSE_ACK);
		packWord(&packet, ICQ_SERVICE_FAMILY);
		packWord(&packet, ICQ_LISTS_FAMILY);
		packWord(&packet, ICQ_LOCATION_FAMILY);
		packWord(&packet, ICQ_BUDDY_FAMILY);
		packWord(&packet, ICQ_EXTENSIONS_FAMILY);
		packWord(&packet, ICQ_MSG_FAMILY);
		packWord(&packet, 0x06);
		packWord(&packet, ICQ_BOS_FAMILY);
		packWord(&packet, ICQ_LOOKUP_FAMILY);
		packWord(&packet, ICQ_STATS_FAMILY);
		sendServPacket(&packet);

		debugLogA("Sent server pause ack");
		break;

	case ICQ_SERVER_MIGRATIONREQ:
		debugLogA("Server migration requested (Flags: %u)", pSnacHeader->wFlags);

		pBuffer += 2; // Unknown, seen: 0
		wBufferLength -= 2;
		{
			oscar_tlv_chain *chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

			if (info->cookieDataLen > 0)
				SAFE_FREE((void**)&info->cookieData);

			info->newServer = chain->getString(0x05, 1);
			info->newServerSSL = chain->getNumber(0x8E, 1);
			info->cookieData = (BYTE*)chain->getString(0x06, 1);
			info->cookieDataLen = chain->getLength(0x06, 1);

			disposeChain(&chain);

			if (!info->newServer || !info->cookieData) {
				icq_LogMessage(LOG_FATAL, LPGEN("A server migration has failed because the server returned invalid data. You must reconnect manually."));
				SAFE_FREE(&info->newServer);
				SAFE_FREE((void**)&info->cookieData);
				info->cookieDataLen = 0;
				info->isNewServerReady = false;
				return;
			}

			debugLogA("Migration has started. New server will be %s", info->newServer);

			m_iDesiredStatus = m_iStatus;
			SetCurrentStatus(ID_STATUS_CONNECTING); // revert to connecting state

			info->isNewServerReady = info->isMigrating = true;
		}
		break;

	case ICQ_SERVER_NAME_INFO: // This is the reply to CLI_REQINFO
		debugLogA("Received self info");
		{
			BYTE bUinLen;
			unpackByte(&pBuffer, &bUinLen);
			pBuffer += bUinLen;
			pBuffer += 4;      /* warning level & user class */
			wBufferLength -= 5 + bUinLen;

			// This is during the login sequence
			if (pSnacHeader->dwRef == ICQ_CLIENT_REQINFO << 0x10) {
				// TLV(x01) User type?
				// TLV(x0C) Empty CLI2CLI Direct connection info
				// TLV(x0A) External IP
				// TLV(x0F) Number of seconds that user has been online
				// TLV(x03) The online since time.
				// TLV(x0A) External IP again
				// TLV(x22) Unknown
				// TLV(x1E) Unknown: empty.
				// TLV(x05) Member of ICQ since.
				// TLV(x14) Unknown
				oscar_tlv_chain *chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

				// Save external IP
				DWORD dwValue = chain->getDWord(0x0A, 1);
				setDword("IP", dwValue);

				// Save member since timestamp
				dwValue = chain->getDWord(0x05, 1);
				if (dwValue) setDword("MemberTS", dwValue);

				dwValue = chain->getDWord(0x03, 1);
				setDword("LogonTS", dwValue ? dwValue : time(NULL));

				disposeChain(&chain);

				// If we are in SSI mode, this is sent after the list is acked instead
				// to make sure that we don't set status before seing the visibility code
				if (!m_bSsiEnabled || info->isMigrating)
					handleServUINSettings(wListenPort, info);
			}
		}
		break;

	case ICQ_SERVER_RATE_CHANGE:
		if (wBufferLength >= 2) {
			WORD wStatus, wClass;
			DWORD dwLevel;
			// We now have global rate management, although controlled are only some
			// areas. This should not arrive in most cases. If it does, update our
			// local rate levels & issue broadcast.
			unpackWord(&pBuffer, &wStatus);
			unpackWord(&pBuffer, &wClass);
			pBuffer += 20;
			unpackDWord(&pBuffer, &dwLevel);
			{
				mir_cslock l(m_ratesMutex);
				m_rates->updateLevel(wClass, dwLevel);
			}

			if (wStatus == 2 || wStatus == 3) {
				// this is only the simplest solution, needs rate management to every section
				ProtoBroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				if (wStatus == 2)
					debugLogA("Rates #%u: Alert", wClass);
				else
					debugLogA("Rates #%u: Limit", wClass);
			}
			else if (wStatus == 4) {
				ProtoBroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				debugLogA("Rates #%u: Clear", wClass);
			}
		}

		break;

	case ICQ_SERVER_REDIRECT_SERVICE: // reply to family request, got new connection point
	{
		oscar_tlv_chain *pChain = NULL;
		cookie_family_request *pCookieData;

		if (!(pChain = readIntoTLVChain(&pBuffer, wBufferLength, 0))) {
			debugLogA("Received Broken Redirect Service SNAC(1,5).");
			break;
		}

		// pick request data
		WORD wFamily = pChain->getWord(0x0D, 1);
		if ((!FindCookie(pSnacHeader->dwRef, NULL, (void**)&pCookieData)) || (pCookieData->wFamily != wFamily)) {
			disposeChain(&pChain);
			debugLogA("Received unexpected SNAC(1,5), skipping.");
			break;
		}

		FreeCookie(pSnacHeader->dwRef);

		// new family entry point received
		char *pServer = pChain->getString(0x05, 1);
		BYTE bServerSSL = pChain->getNumber(0x8E, 1);
		char *pCookie = pChain->getString(0x06, 1);
		WORD wCookieLen = pChain->getLength(0x06, 1);

		if (!pServer || !pCookie) {
			debugLogA("Server returned invalid data, family unavailable.");

			SAFE_FREE(&pServer);
			SAFE_FREE(&pCookie);
			SAFE_FREE((void**)&pCookieData);
			disposeChain(&pChain);
			break;
		}

		// Get new family server ip and port
		WORD wPort = info->wServerPort; // get default port
		parseServerAddress(pServer, &wPort);

		// establish connection
		NETLIBOPENCONNECTION nloc = { 0 };
		if (m_bGatewayMode)
			nloc.flags |= NLOCF_HTTPGATEWAY;
		nloc.szHost = pServer;
		nloc.wPort = wPort;

		HANDLE hConnection = NetLib_OpenConnection(m_hNetlibUser, wFamily == ICQ_AVATAR_FAMILY ? "Avatar " : NULL, &nloc);

		if (hConnection == NULL)
			debugLogA("Unable to connect to ICQ new family server.");
		// we want the handler to be called even if the connecting failed
		else if (bServerSSL) { /* Start SSL session if requested */
			debugLogA("(%p) Starting SSL negotiation", CallService(MS_NETLIB_GETSOCKET, (WPARAM)hConnection, 0));

			if (!CallService(MS_NETLIB_STARTSSL, (WPARAM)hConnection, 0)) {
				debugLogA("Unable to connect to ICQ new family server, SSL could not be negotiated");
				NetLib_CloseConnection(&hConnection, FALSE);
			}
		}

		(this->*pCookieData->familyHandler)(hConnection, pCookie, wCookieLen);

		// Free allocated memory
		// NOTE: "cookie" will get freed when we have connected to the avatar server.
		disposeChain(&pChain);
		SAFE_FREE(&pServer);
		SAFE_FREE((void**)&pCookieData);
		break;
	}

	case ICQ_SERVER_EXTSTATUS: // our session data
		debugLogA("Received owner session data.");

		while (wBufferLength > 4) { // loop thru all items
			WORD itemType = pBuffer[0] * 0x10 | pBuffer[1];
			BYTE itemFlags = pBuffer[2];
			size_t itemLen = pBuffer[3];

			if (itemType == AVATAR_HASH_PHOTO) { /// TODO: handle photo item
				// skip photo item
				debugLogA("Photo item recognized");
			}
			else if ((itemType == AVATAR_HASH_STATIC || itemType == AVATAR_HASH_FLASH) && (itemLen >= 0x10)) {
				debugLogA("Avatar item recognized");

				if (m_bAvatarsEnabled && !info->bMyAvatarInited) { // signal the server after login
					// this refreshes avatar state - it used to work automatically, but now it does not
					if (getByte("ForceOurAvatar", 0)) { // keep our avatar
						TCHAR *file = GetOwnAvatarFileName();
						SetMyAvatar(0, (LPARAM)file);
						SAFE_FREE(&file);
					}
					else { // only change avatar hash to the same one
						BYTE hash[0x14];
						memcpy(hash, pBuffer, 0x14);
						hash[2] = 1; // update image status
						updateServAvatarHash(hash, 0x14);
					}
					info->bMyAvatarInited = true;
					break;
				}
				// process owner avatar hash changed notification
				handleAvatarOwnerHash(itemFlags, pBuffer, itemLen + 4);
			}
			else if (itemType == 0x02) {
				debugLogA("Status message item recognized");
			}
			else if (itemType == 0x0E) {
				debugLogA("Status mood item recognized");
			}

			// move to next item
			if (wBufferLength >= itemLen + 4) {
				wBufferLength -= itemLen + 4;
				pBuffer += itemLen + 4;
			}
			else {
				pBuffer += wBufferLength;
				wBufferLength = 0;
			}
		}
		break;

	case ICQ_ERROR: // Something went wrong, probably the request for avatar family failed
	{
		WORD wError;
		if (wBufferLength >= 2)
			unpackWord(&pBuffer, &wError);
		else
			wError = 0;

		LogFamilyError(ICQ_SERVICE_FAMILY, wError);
	}
		break;

		// Stuff we don't care about
	case ICQ_SERVER_MOTD:
		debugLogA("Server message of the day");
		break;

	default:
		debugLogA("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_SERVICE_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;
	}
}


char* CIcqProto::buildUinList(int subtype, size_t wMaxLen, MCONTACT *hContactResume)
{
	MCONTACT hContact;
	WORD wCurrentLen = 0;
	int add;

	char *szList = (char*)SAFE_MALLOC(CallService(MS_DB_CONTACT_GETCOUNT, 0, 0) * UINMAXLEN);

	char szLen[2];
	szLen[1] = '\0';

	if (*hContactResume)
		hContact = *hContactResume;
	else
		hContact = db_find_first(m_szModuleName);

	while (hContact != NULL) {
		DWORD dwUIN;
		uid_str szUID;
		if (!getContactUid(hContact, &dwUIN, &szUID)) {
			szLen[0] = (char)mir_strlen(strUID(dwUIN, szUID));

			switch (subtype) {
			case BUL_VISIBLE:
				add = ID_STATUS_ONLINE == getWord(hContact, "ApparentMode", 0);
				break;

			case BUL_INVISIBLE:
				add = ID_STATUS_OFFLINE == getWord(hContact, "ApparentMode", 0);
				break;

			case BUL_TEMPVISIBLE:
				add = getByte(hContact, "TemporaryVisible", 0);
				// clear temporary flag
				// Here we assume that all temporary contacts will be in one packet
				setByte(hContact, "TemporaryVisible", 0);
				break;

			default:
				add = 1;

				// If we are in SS mode, we only add those contacts that are
				// not in our SS list, or are awaiting authorization, to our
				// client side list
				if (m_bSsiEnabled && getWord(hContact, DBSETTING_SERVLIST_ID, 0) &&
					 !getByte(hContact, "Auth", 0))
					 add = 0;

				// Never add hidden contacts to CS list
				if (db_get_b(hContact, "CList", "Hidden", 0))
					add = 0;

				break;
			}

			if (add) {
				wCurrentLen += szLen[0] + 1;
				if (wCurrentLen > wMaxLen) {
					*hContactResume = hContact;
					return szList;
				}

				strcat(szList, szLen);
				strcat(szList, szUID);
			}
		}

		hContact = db_find_next(hContact, m_szModuleName);
	}
	*hContactResume = NULL;

	return szList;
}

void CIcqProto::sendEntireListServ(WORD wFamily, WORD wSubtype, int listType)
{
	MCONTACT hResumeContact = NULL;

	do { // server doesn't seem to be able to cope with packets larger than 8k
		// send only about 100contacts per packet
		char *szList = buildUinList(listType, 0x3E8, &hResumeContact);

		size_t nListLen = mir_strlen(szList);
		if (nListLen) {
			icq_packet packet;
			serverPacketInit(&packet, nListLen + 10);
			packFNACHeader(&packet, wFamily, wSubtype);
			packBuffer(&packet, (LPBYTE)szList, nListLen);
			sendServPacket(&packet);
		}

		SAFE_FREE((void**)&szList);
	}
	while (hResumeContact);
}


static void packShortCapability(icq_packet *packet, WORD wCapability)
{
	// pack standard capability
	DWORD dwQ1 = 0x09460000 | wCapability;

	packDWord(packet, dwQ1);
	packDWord(packet, 0x4c7f11d1);
	packDWord(packet, 0x82224445);
	packDWord(packet, 0x53540000);
}

// CLI_SETUSERINFO
void CIcqProto::setUserInfo()
{
	icq_packet packet;
	size_t wAdditionalData = 0;
	BYTE bXStatus = getContactXStatus(NULL);

	if (m_bAimEnabled)
		wAdditionalData += 16;
#ifdef DBG_CAPMTN
	wAdditionalData += 16;
#endif
	if (m_bUtfEnabled)
		wAdditionalData += 16;
#ifdef DBG_NEWCAPS
	wAdditionalData += 16;
#endif
#ifdef DBG_CAPXTRAZ
	wAdditionalData += 16;
#endif
#ifdef DBG_OSCARFT
	wAdditionalData += 16;
#endif
	if (m_bAvatarsEnabled)
		wAdditionalData += 16;
	if (m_bXStatusEnabled && bXStatus != 0)
		wAdditionalData += 16;
#ifdef DBG_CAPHTML
	wAdditionalData += 16;
#endif
#ifdef DBG_AIMCONTACTSEND
	wAdditionalData += 16;
#endif

	wAdditionalData += CustomCapList.getCount() * 16;

	//MIM/PackName
	bool bHasPackName = false;
	DBVARIANT dbv;
	if (!db_get_s(NULL, "ICQCaps", "PackName", &dbv)) {
		//MIM/PackName
		bHasPackName = true;
		wAdditionalData += 16;
	}

	serverPacketInit(&packet, 62 + wAdditionalData);
	packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO);

	/* TLV(5): capability data */
	packWord(&packet, 0x0005);
	packWord(&packet, WORD(48 + wAdditionalData));

#ifdef DBG_CAPMTN
	packDWord(&packet, 0x563FC809); // CAP_TYPING
	packDWord(&packet, 0x0B6F41BD);
	packDWord(&packet, 0x9F794226);
	packDWord(&packet, 0x09DFA2F3);
#endif

	packShortCapability(&packet, 0x1349);  // AIM_CAPS_ICQSERVERRELAY

	// Broadcasts the capability to receive UTF8 encoded messages
	if (m_bUtfEnabled)
		packShortCapability(&packet, 0x134E);  // CAP_UTF8MSGS

#ifdef DBG_NEWCAPS
	// Tells server we understand to new format of caps
	packShortCapability(&packet, 0x0000);  // CAP_SHORTCAPS
#endif

#ifdef DBG_CAPXTRAZ
	packDWord(&packet, 0x1a093c6c); // CAP_XTRAZ
	packDWord(&packet, 0xd7fd4ec5); // Broadcasts the capability to handle
	packDWord(&packet, 0x9d51a647); // Xtraz
	packDWord(&packet, 0x4e34f5a0);
#endif

	if (m_bAvatarsEnabled)
		packShortCapability(&packet, 0x134C);  // CAP_DEVILS

#ifdef DBG_OSCARFT
	// Broadcasts the capability to receive Oscar File Transfers
	packShortCapability(&packet, 0x1343);  // CAP_AIM_FILE
#endif

	// Tells the server we can speak to AIM
	if (m_bAimEnabled)
		packShortCapability(&packet, 0x134D);  // CAP_AIM_COMPATIBLE

#ifdef DBG_AIMCONTACTSEND
	packShortCapability(&packet, 0x134B);  // CAP_SENDBUDDYLIST
#endif

	if (m_bXStatusEnabled && bXStatus != 0)
		packBuffer(&packet, capXStatus[bXStatus - 1], BINARY_CAP_SIZE);

	packShortCapability(&packet, 0x1344);      // CAP_ICQDIRECT

#ifdef DBG_CAPHTML
	packShortCapability(&packet, 0x0002);      // CAP_HTMLMSGS
#endif

	packDWord(&packet, 0x4D697261);   // Miranda Signature
	packDWord(&packet, 0x6E64614E);

	WORD v[4];
	CallService(MS_SYSTEM_GETFILEVERSION, 0, (LPARAM)v);
	packWord(&packet, v[0]);
	packWord(&packet, v[1]);
	packWord(&packet, v[2]);
	packWord(&packet, v[3]);

	//MIM/PackName
	if (bHasPackName) {
		packBuffer(&packet, (BYTE*)dbv.pszVal, 0x10);
		db_free(&dbv);
	}

	if (CustomCapList.getCount())
		for (int i = 0; i < CustomCapList.getCount(); i++)
			packBuffer(&packet, (PBYTE)CustomCapList[i].caps, 0x10);

	sendServPacket(&packet);
}


void CIcqProto::handleServUINSettings(int nPort, serverthread_info *info)
{
	setUserInfo();

	/* SNAC 3,4: Tell server who's on our list (deprecated) */
	/* SNAC 3,15: Try to add unauthorised contacts to temporary list */
	sendEntireListServ(ICQ_BUDDY_FAMILY, ICQ_USER_ADDTOTEMPLIST, BUL_ALLCONTACTS);

	if (m_iDesiredStatus == ID_STATUS_INVISIBLE) {
		/* Tell server who's on our visible list (deprecated) */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDVISIBLE, BUL_VISIBLE);
		else
			updateServVisibilityCode(3);
	}

	if (m_iDesiredStatus != ID_STATUS_INVISIBLE) {
		/* Tell server who's on our invisible list (deprecated) */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDINVISIBLE, BUL_INVISIBLE);
		else
			updateServVisibilityCode(4);
	}

	// SNAC 1,1E: Set status
	icq_packet packet;
	{
		DWORD dwDirectCookie = rand() ^ (rand() << 16);

		// Get status
		WORD wStatus = MirandaStatusToIcq(m_iDesiredStatus);

		// Get status note & mood
		char *szStatusNote = PrepareStatusNote(m_iDesiredStatus);
		BYTE bXStatus = getContactXStatus(NULL);
		char szMoodData[32];

		// prepare mood id
		if (m_bMoodsEnabled && bXStatus && moodXStatus[bXStatus - 1] != -1)
			mir_snprintf(szMoodData, SIZEOF(szMoodData), "icqmood%d", moodXStatus[bXStatus - 1]);
		else
			szMoodData[0] = '\0';

		//! Tricky code, this ensures that the status note will be saved to the directory
		SetStatusNote(szStatusNote, m_bGatewayMode ? 5000 : 2500, TRUE);

		size_t wStatusNoteLen = mir_strlen(szStatusNote);
		size_t wStatusMoodLen = mir_strlen(szMoodData);
		size_t wSessionDataLen = (wStatusNoteLen ? wStatusNoteLen + 4 : 0) + 4 + wStatusMoodLen + 4;

		serverPacketInit(&packet, 71 + (wSessionDataLen ? wSessionDataLen + 4 : 0));
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
		packDWord(&packet, 0x00060004);             // TLV 6: Status mode and security flags
		packWord(&packet, GetMyStatusFlags());      // Status flags
		packWord(&packet, wStatus);                 // Status
		packTLVWord(&packet, 0x0008, 0x0A06);       // TLV 8: Independent Status Messages
		packDWord(&packet, 0x000c0025);             // TLV C: Direct connection info
		packDWord(&packet, getDword("RealIP", 0));
		packDWord(&packet, nPort);
		packByte(&packet, DC_TYPE);                 // TCP/FLAG firewall settings
		packWord(&packet, ICQ_VERSION);
		packDWord(&packet, dwDirectCookie);         // DC Cookie
		packDWord(&packet, WEBFRONTPORT);           // Web front port
		packDWord(&packet, CLIENTFEATURES);         // Client features
		packDWord(&packet, 0x7fffffff);             // Abused timestamp
		packDWord(&packet, ICQ_PLUG_VERSION);       // Abused timestamp
		if (ServiceExists("SecureIM/IsContactSecured"))
			packDWord(&packet, 0x5AFEC0DE);           // SecureIM Abuse
		else
			packDWord(&packet, 0x00000000);           // Timestamp
		packWord(&packet, 0x0000);                  // Unknown
		packTLVWord(&packet, 0x001F, 0x0000);

		if (wSessionDataLen) { // Pack session data
			packWord(&packet, 0x1D);                  // TLV 1D
			packWord(&packet, WORD(wSessionDataLen));       // TLV length
			packWord(&packet, 0x02);                  // Item Type
			if (wStatusNoteLen) {
				packWord(&packet, 0x400 | WORD(wStatusNoteLen + 4)); // Flags + Item Length
				packWord(&packet, WORD(wStatusNoteLen));      // Text Length
				packBuffer(&packet, (LPBYTE)szStatusNote, wStatusNoteLen);
				packWord(&packet, 0);                   // Encoding not specified (utf-8 is default)
			}
			else
				packWord(&packet, 0);                   // Flags + Item Length
			packWord(&packet, 0x0E);                  // Item Type
			packWord(&packet, WORD(wStatusMoodLen));        // Flags + Item Length
			if (wStatusMoodLen)
				packBuffer(&packet, (LPBYTE)szMoodData, wStatusMoodLen); // Mood

			// Save current status note & mood
			db_set_utf(NULL, m_szModuleName, DBSETTING_STATUS_NOTE, szStatusNote);
			setString(DBSETTING_STATUS_MOOD, szMoodData);
		}
		// Release memory
		SAFE_FREE(&szStatusNote);

		sendServPacket(&packet);
	}

	/* SNAC 1,11 */
	serverPacketInit(&packet, 14);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_IDLE);
	packDWord(&packet, 0x00000000);

	sendServPacket(&packet);
	m_bIdleAllow = 0;

	// Change status
	SetCurrentStatus(m_iDesiredStatus);

	// Finish Login sequence
	serverPacketInit(&packet, 98);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_READY);
	packDWord(&packet, 0x00220001); // imitate ICQ 6 behaviour
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00010004);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00130004);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00020001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00030001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00150001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00040001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00060001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00090001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x000A0001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x000B0001);
	packDWord(&packet, 0x0110164f);

	sendServPacket(&packet);

	debugLogA(" *** Yeehah, login sequence complete");

	// login sequence is complete enter logged-in mode
	info->bLoggedIn = true;
	m_bConnectionLost = false;

	// enable auto info-update routine
	icq_EnableUserLookup(true);

	if (!info->isMigrating) {
		// Get Offline Messages Reqeust
		cookie_offline_messages *ack = (cookie_offline_messages*)SAFE_MALLOC(sizeof(cookie_offline_messages));
		if (ack) {
			DWORD dwCookie = AllocateCookie(CKT_OFFLINEMESSAGE, ICQ_MSG_CLI_REQ_OFFLINE, 0, ack);

			serverPacketInit(&packet, 10);
			packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQ_OFFLINE, 0, dwCookie);

			sendServPacket(&packet);
		}
		else icq_LogMessage(LOG_WARNING, LPGEN("Failed to request offline messages. They may be received next time you log in."));

		// Update our information from the server
		sendOwnerInfoRequest();

		// Request info updates on all contacts
		icq_RescanInfoUpdate();

		// Start sending Keep-Alive packets
		StartKeepAlive(info);

		if (m_bAvatarsEnabled) { // Send SNAC 1,4 - request avatar family 0x10 connection
			icq_requestnewfamily(ICQ_AVATAR_FAMILY, &CIcqProto::StartAvatarThread);

			m_avatarsConnectionPending = TRUE;
			debugLogA("Requesting Avatar family entry point.");
		}

		// Set last xstatus
		updateServerCustomStatus(TRUE);
	}
	info->isMigrating = false;

	if (m_bAimEnabled) {
		char **szAwayMsg = NULL;
		mir_cslock l(m_modeMsgsMutex);

		szAwayMsg = MirandaStatusToAwayMsg(m_iStatus);
		if (szAwayMsg)
			icq_sendSetAimAwayMsgServ(*szAwayMsg);
	}
}
