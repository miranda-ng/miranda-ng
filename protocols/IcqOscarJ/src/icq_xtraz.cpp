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
//  Internal Xtraz API
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::handleXtrazNotify(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szMsg, size_t nMsgLen, BOOL bThruDC)
{
	char *szNotify = strstrnull(szMsg, "<NOTIFY>");
	char *szQuery = strstrnull(szMsg, "<QUERY>");

	MCONTACT hContact = HContactFromUIN(dwUin, NULL);
	if (hContact) // user sent us xtraz, he supports it
		SetContactCapabilities(hContact, CAPF_XTRAZ);

	if (szNotify && szQuery) { // valid request
		char *szWork, *szEnd;
		int nNotifyLen, nQueryLen;

		szNotify += 8;
		szQuery += 7;
		szEnd = strstrnull(szMsg, "</NOTIFY>");
		if (!szEnd) szEnd = szMsg + nMsgLen;
		nNotifyLen = (szEnd - szNotify);
		szEnd = strstrnull(szMsg, "</QUERY>");
		if (!szEnd) szEnd = szNotify;
		szNotify = DemangleXml(szNotify, nNotifyLen);
		nQueryLen = (szEnd - szQuery);
		szQuery = DemangleXml(szQuery, nQueryLen);
		szWork = strstrnull(szQuery, "<PluginID>");
		szEnd = strstrnull(szQuery, "</PluginID>");

		debugLogA("Query: %s", szQuery);
		debugLogA("Notify: %s", szNotify);

		if (szWork && szEnd) { // this is our plugin
			szWork += 10;
			*szEnd = '\0';

			if (!mir_strcmpi(szWork, "srvMng") && strstrnull(szNotify, "AwayStat")) {
				char *szSender = strstrnull(szNotify, "<senderId>");
				char *szEndSend = strstrnull(szNotify, "</senderId>");

				if (szSender && szEndSend) {
					szSender += 10;
					*szEndSend = '\0';

					if ((DWORD)atoi(szSender) == dwUin) {
						BYTE dwXId = m_bXStatusEnabled ? getContactXStatus(NULL) : 0;

						if (dwXId && validateStatusMessageRequest(hContact, MTYPE_SCRIPT_NOTIFY)) { // apply privacy rules
							NotifyEventHooks(m_modeMsgsEvent, (WPARAM)MTYPE_SCRIPT_NOTIFY, (LPARAM)dwUin);

							char *tmp = getSettingStringUtf(NULL, DBSETTING_XSTATUS_NAME, "");
							char *szXName = MangleXml(tmp, mir_strlen(tmp));
							SAFE_FREE(&tmp);

							tmp = getSettingStringUtf(NULL, DBSETTING_XSTATUS_MSG, "");
							char *szXMsg = MangleXml(tmp, mir_strlen(tmp));
							SAFE_FREE(&tmp);

							size_t nResponseLen = 212 + mir_strlen(szXName) + mir_strlen(szXMsg) + UINMAXLEN + 2;
							char *szResponse = (char*)_alloca(nResponseLen + 1);
							// send response
							mir_snprintf(szResponse, nResponseLen,
								"<ret event=\"OnRemoteNotification\">"
								"<srv><id>cAwaySrv</id>"
								"<val srv_id=\"cAwaySrv\"><Root>"
								"<CASXtraSetAwayMessage></CASXtraSetAwayMessage>"
								"<uin>%d</uin>"
								"<index>%d</index>"
								"<title>%s</title>"
								"<desc>%s</desc></Root></val></srv></ret>",
								m_dwLocalUIN, dwXId, szXName, szXMsg);

							SAFE_FREE(&szXName);
							SAFE_FREE(&szXMsg);

							struct rates_xstatus_response : public rates_queue_item {
							protected:
								virtual rates_queue_item* copyItem(rates_queue_item *aDest = NULL)
								{
									rates_xstatus_response *pDest = (rates_xstatus_response*)aDest;
									if (!pDest)
										pDest = new rates_xstatus_response(ppro, wGroup);

									pDest->bThruDC = bThruDC;
									pDest->dwMsgID1 = dwMsgID1;
									pDest->dwMsgID2 = dwMsgID2;
									pDest->wCookie = wCookie;
									pDest->szResponse = null_strdup(szResponse);

									return rates_queue_item::copyItem(pDest);
								};
							public:
								rates_xstatus_response(CIcqProto *ppro, WORD wGroup) : rates_queue_item(ppro, wGroup), szResponse(NULL) {};
								virtual ~rates_xstatus_response() { if (bCreated) SAFE_FREE(&szResponse); };

								virtual void execute()
								{
									ppro->SendXtrazNotifyResponse(dwUin, dwMsgID1, dwMsgID2, wCookie, szResponse, mir_strlen(szResponse), bThruDC);
								};

								BOOL bThruDC;
								DWORD dwMsgID1;
								DWORD dwMsgID2;
								WORD wCookie;
								char *szResponse;
							};

							WORD wGroup;
							{
								mir_cslock rlck(m_ratesMutex);
								wGroup = m_rates->getGroupFromSNAC(ICQ_MSG_FAMILY, ICQ_MSG_RESPONSE);
							}

							rates_xstatus_response rr(this, wGroup);
							rr.hContact = hContact;
							rr.dwUin = dwUin;
							rr.bThruDC = bThruDC;
							rr.dwMsgID1 = dwMID;
							rr.dwMsgID2 = dwMID2;
							rr.wCookie = wCookie;
							rr.szResponse = szResponse;

							handleRateItem(&rr, RQT_RESPONSE, 0, !bThruDC);
						}
						else if (dwXId)
							debugLogA("Privacy: Ignoring XStatus request");
						else
							debugLogA("Error: We are not in XStatus, skipping");
					}
					else debugLogA("Error: Invalid sender information");
				}
				else debugLogA("Error: Missing sender information");
			}
			else debugLogA("Error: Unknown plugin \"%s\" in Xtraz message", szWork);
		}
		else debugLogA("Error: Missing PluginID in Xtraz message");

		SAFE_FREE(&szNotify);
		SAFE_FREE(&szQuery);
	}
	else debugLogA("Error: Invalid Xtraz Notify message");
}

void CIcqProto::handleXtrazNotifyResponse(MCONTACT hContact, WORD wCookie, char* szMsg)
{
	char *szMem, *szRes, *szEnd;
	size_t nResLen;

	debugLogA("Received Xtraz Notify Response");

	szRes = strstrnull(szMsg, "<RES>");
	szEnd = strstrnull(szMsg, "</RES>");

	if (szRes && szEnd) { // valid response
		char *szNode, *szWork;

		szRes += 5;
		nResLen = szEnd - szRes;

		szMem = szRes = DemangleXml(szRes, nResLen);

		debugLogA("Response: %s", szRes);

		ProtoBroadcastAck(hContact, ICQACKTYPE_XTRAZNOTIFY_RESPONSE, ACKRESULT_SUCCESS, (HANDLE)wCookie, (LPARAM)szRes);

	NextVal:
		szNode = strstrnull(szRes, "<val srv_id=");
		if (szNode) szEnd = strstrnull(szNode, ">"); else szEnd = NULL;

		if (szNode && szEnd) {
			*(szEnd - 1) = '\0';
			szNode += 13; //one more than the length of the string to skip ' or " too
			szWork = szEnd + 1;

			if (!mir_strcmpi(szNode, "cAwaySrv")) {
				int bChanged = FALSE;

				*szEnd = ' ';
				szNode = strstrnull(szWork, "<index>");
				szEnd = strstrnull(szWork, "</index>");
				if (szNode && szEnd) {
					szNode += 7;
					*szEnd = '\0';
					if (atoi(szNode) != getContactXStatus(hContact)) { // this is strange - but go on
						debugLogA("Warning: XStatusIds do not match!");
					}
					*szEnd = ' ';
				}
				szNode = strstrnull(szWork, "<title>");
				szEnd = strstrnull(szWork, "</title>");
				if (szNode && szEnd) { // we got XStatus title, save it
					char *szXName, *szOldXName;
					szNode += 7;
					*szEnd = '\0';
					szXName = DemangleXml(szNode, mir_strlen(szNode));
					// check if the name changed
					szOldXName = getSettingStringUtf(hContact, DBSETTING_XSTATUS_NAME, NULL);
					if (mir_strcmp(szOldXName, szXName))
						bChanged = TRUE;
					SAFE_FREE(&szOldXName);
					db_set_utf(hContact, m_szModuleName, DBSETTING_XSTATUS_NAME, szXName);
					SAFE_FREE(&szXName);
					*szEnd = ' ';
				}
				szNode = strstrnull(szWork, "<desc>");
				szEnd = strstrnull(szWork, "</desc>");
				if (szNode && szEnd) { // we got XStatus mode msg, save it
					char *szXMsg, *szOldXMsg;
					szNode += 6;
					*szEnd = '\0';
					szXMsg = DemangleXml(szNode, mir_strlen(szNode));
					// check if the decription changed
					szOldXMsg = getSettingStringUtf(hContact, DBSETTING_XSTATUS_NAME, NULL);
					if (mir_strcmp(szOldXMsg, szXMsg))
						bChanged = TRUE;
					SAFE_FREE(&szOldXMsg);
					db_set_utf(hContact, m_szModuleName, DBSETTING_XSTATUS_MSG, szXMsg);
					SAFE_FREE(&szXMsg);
				}
				ProtoBroadcastAck(hContact, ICQACKTYPE_XSTATUS_RESPONSE, ACKRESULT_SUCCESS, (HANDLE)wCookie, 0);
			}
			else {
				char *szSrvEnd = strstrnull(szEnd, "</srv>");

				if (szSrvEnd && strstrnull(szSrvEnd, "<val srv_id=")) { // check all values !
					szRes = szSrvEnd + 6; // after first value
					goto NextVal;
				}
				// no next val, we were unable to handle packet, write error
				debugLogA("Error: Unknown serverId \"%s\" in Xtraz response", szNode);
			}
		}
		else debugLogA("Error: Missing serverId in Xtraz response");

		SAFE_FREE(&szMem);
	}
	else debugLogA("Error: Invalid Xtraz Notify response");
}

static char* getXmlPidItem(const char* szData)
{
	const char *szPid = strstrnull(szData, "<PID>");
	const char *szEnd = strstrnull(szData, "</PID>");

	if (szPid && szEnd) {
		szPid += 5;
		return DemangleXml(szPid, szEnd - szPid);
	}
	return NULL;
}


void CIcqProto::handleXtrazInvitation(DWORD dwUin, char* szMsg, BOOL bThruDC)
{
	MCONTACT hContact = HContactFromUIN(dwUin, NULL);
	if (hContact) // user sent us xtraz, he supports it
		SetContactCapabilities(hContact, CAPF_XTRAZ);

	char *szPluginID = getXmlPidItem(szMsg);
	if (!mir_strcmp(szPluginID, "ICQChatRecv"))  // it is a invitation to multi-user chat
		;
	else 
		NetLog_Uni(bThruDC, "Error: Unknown plugin \"%s\" in Xtraz message", szPluginID);

	SAFE_FREE(&szPluginID);
}


void CIcqProto::handleXtrazData(DWORD dwUin, char* szMsg, BOOL bThruDC)
{
	MCONTACT hContact = HContactFromUIN(dwUin, NULL);
	if (hContact) // user sent us xtraz, he supports it
		SetContactCapabilities(hContact, CAPF_XTRAZ);

	char *szPluginID = getXmlPidItem(szMsg);
	if (!mir_strcmp(szPluginID, "viewCard")) { // it is a greeting card
		char *szWork, *szEnd, *szUrl, *szNum;

		szWork = strstrnull(szMsg, "<InD>");
		szEnd = strstrnull(szMsg, "</InD>");
		if (szWork && szEnd) {
			int nDataLen = szEnd - szWork;

			szUrl = (char*)_alloca(nDataLen);
			memcpy(szUrl, szWork + 5, nDataLen);
			szUrl[nDataLen - 5] = '\0';

			if (!_strnicmp(szUrl, "view_", 5)) {
				szNum = szUrl + 5;
				szWork = strstrnull(szUrl, ".html");
				if (szWork) {
					mir_strcpy(szWork, ".php");
					strcat(szWork, szWork + 5);
				}
				while (szWork = strstrnull(szUrl, "&amp;"))  // unescape &amp; code
					mir_strcpy(szWork + 1, szWork + 5);

				szWork = (char*)SAFE_MALLOC(nDataLen + MAX_PATH);
				ICQTranslateUtfStatic(LPGEN("Greeting card:"), szWork, MAX_PATH);
				strcat(szWork, "\r\nhttp://www.icq.com/friendship/pages/view_page_");
				strcat(szWork, szNum);

				// Create message to notify user
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = time(NULL);
				pre.szMessage = szWork;

				int bAdded;
				ProtoChainRecvMsg(HContactFromUIN(dwUin, &bAdded), &pre);

				SAFE_FREE(&szWork);
			}
			else NetLog_Uni(bThruDC, "Error: Non-standard greeting card message");
		}
		else NetLog_Uni(bThruDC, "Error: Malformed greeting card message");
	}
	else NetLog_Uni(bThruDC, "Error: Unknown plugin \"%s\" in Xtraz message", szPluginID);

	SAFE_FREE(&szPluginID);
}


// Functions really sending Xtraz stuff
DWORD CIcqProto::SendXtrazNotifyRequest(MCONTACT hContact, char* szQuery, char* szNotify, int bForced)
{
	DWORD dwUin;
	if (getContactUid(hContact, &dwUin, NULL))
		return 0; // Invalid contact

	if (!CheckContactCapabilities(hContact, CAPF_XTRAZ) && !bForced)
		return 0; // Contact does not support xtraz, do not send anything

	char  *szQueryBody = MangleXml(szQuery, mir_strlen(szQuery));
	char  *szNotifyBody = MangleXml(szNotify, mir_strlen(szNotify));
	size_t nBodyLen = mir_strlen(szQueryBody) + mir_strlen(szNotifyBody) + 41;
	char  *szBody = (char*)_alloca(nBodyLen);
	nBodyLen = mir_snprintf(szBody, nBodyLen, "<N><QUERY>%s</QUERY><NOTIFY>%s</NOTIFY></N>", szQueryBody, szNotifyBody);
	SAFE_FREE((void**)&szQueryBody);
	SAFE_FREE((void**)&szNotifyBody);

	// Set up the ack type
	cookie_message_data *pCookieData = CreateMessageCookie(MTYPE_SCRIPT_NOTIFY, ACKTYPE_CLIENT);
	DWORD dwCookie = AllocateCookie(CKT_MESSAGE, 0, hContact, (void*)pCookieData);

	// have we a open DC, send through that
	if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0))
		icq_sendXtrazRequestDirect(hContact, dwCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
	else
		icq_sendXtrazRequestServ(dwUin, dwCookie, szBody, nBodyLen, pCookieData);

	return dwCookie;
}


void CIcqProto::SendXtrazNotifyResponse(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szResponse, size_t nResponseLen, BOOL bThruDC)
{
	char *szResBody = MangleXml(szResponse, nResponseLen);
	size_t nBodyLen = mir_strlen(szResBody) + 21;
	char *szBody = (char*)_alloca(nBodyLen);
	MCONTACT hContact = HContactFromUIN(dwUin, NULL);

	if (hContact != INVALID_CONTACT_ID && !CheckContactCapabilities(hContact, CAPF_XTRAZ)) {
		SAFE_FREE(&szResBody);
		return; // Contact does not support xtraz, do not send anything
	}

	nBodyLen = mir_snprintf(szBody, nBodyLen, "<NR><RES>%s</RES></NR>", szResBody);
	SAFE_FREE(&szResBody);

	// Was request received thru DC and have we a open DC, send through that
	if (bThruDC && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0))
		icq_sendXtrazResponseDirect(hContact, wCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
	else
		icq_sendXtrazResponseServ(dwUin, dwMID, dwMID2, wCookie, szBody, nBodyLen, MTYPE_SCRIPT_NOTIFY);
}
