/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

//////////////////////////// History services ///////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcGetAllServerHistoryForContact(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetAllServerHistoryForContact");
	if (!IsOnline())
		return 0;
	LPCTSTR str = TranslateT("Are you sure to reload all messages from vk.com?\nLocal contact history will be deleted and reloaded from the server.\nIt may take a long time.\nDo you want to continue?");
	if (IDNO == MessageBox(NULL, str, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1 || userID == VK_FEED_USER)
		return 0;

	MEVENT hDBEvent = db_event_first(hContact);
	while (hDBEvent) {
		MEVENT hDBEventNext = db_event_next(hContact, hDBEvent);
		db_event_delete(hContact, hDBEvent);
		hDBEvent = hDBEventNext;
	}

	m_bNotifyForEndLoadingHistory = true;

	db_unset(hContact, m_szModuleName, "lastmsgid");
	GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, 0);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetAllServerHistory(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcGetAllServerHistory start");
	if (!IsOnline())
		return 0;
	LPCTSTR str = TranslateT("Are you sure you want to reload all messages for all contacts from vk.com?\nLocal contact history will be deleted and reloaded from the server.\nIt may take a very long time and/or corrupt miranda database.\nWe recommend check your database before reloading messages and after it (Miranda32.exe /svc:dbchecker or Miranda64.exe /svc:dbchecker).\nDo you want to continue?");
	if (IDNO == MessageBox(NULL, str, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG userID = getDword(hContact, "ID", -1);
		if (userID == -1 || userID == VK_FEED_USER)
			continue;

		
		MEVENT hDBEvent = db_event_first(hContact);
		while (hDBEvent) {
			MEVENT hDBEventNext = db_event_next(hContact, hDBEvent);
			db_event_delete(hContact, hDBEvent);
			hDBEvent = hDBEventNext;
		}

		{
			mir_cslock lck(m_csLoadHistoryTask);
			m_iLoadHistoryTask++;
			m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = true;
			debugLogA("CVkProto::SvcGetAllServerHistory for ID=%d m_iLoadHistoryTask=%d", userID, m_iLoadHistoryTask);
		}
		
		db_unset(hContact, m_szModuleName, "lastmsgid");
		GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, 0);
	
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GetServerHistoryLastNDay(MCONTACT hContact, int NDay)
{
	debugLogA("CVkProto::SvcGetServerHistoryLastNDay %d", NDay);

	time_t tTime = time(NULL) - 60 * 60 * 24 * NDay;

	MEVENT hDBEvent = db_event_first(hContact);
	while (hDBEvent) {
		MEVENT hDBEventNext = db_event_next(hContact, hDBEvent);
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDBEvent, &dbei);
		if (dbei.timestamp > tTime)
			db_event_delete(hContact, hDBEvent);
		hDBEvent = hDBEventNext;
	}

	{
		mir_cslock lck(m_csLoadHistoryTask);
		m_iLoadHistoryTask++;
		if (NDay > 3)
			m_bNotifyForEndLoadingHistory = true;
	}

	db_unset(hContact, m_szModuleName, "lastmsgid");
	GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, tTime, 0);
}

void CVkProto::GetServerHistory(MCONTACT hContact, int iOffset, int iCount, int iTime, int iLastMsgId, bool once)
{
	debugLogA("CVkProto::GetServerHistory %d %d %d %d %d", iOffset, iCount, iTime, iLastMsgId, (int)once);
	if (!IsOnline())
		return;

	LONG userID = getDword(hContact, "ID", -1);
	if (-1 == userID || userID == VK_FEED_USER)
		return;

	CMStringA code(FORMAT, "var iOffset=%d;var iReqCount=%d;var userID=%d;var iTime=%d;var lastMid=%d;"
		"var Hist=API.messages.getHistory({\"user_id\":userID,\"count\":iReqCount,\"offset\":iOffset});"
		"var ext=Hist.items.length;var index=0;"
		"while(ext!=0){if(Hist.items[index].date>iTime){if(Hist.items[index].id>lastMid)"
		"{index=index+1;ext=ext-1;}else ext=0;}else ext=0;};"
		"var ret=Hist.items.slice(0,index);"
		"var FMsgs=ret@.fwd_messages;var Idx=0;var Uids=[];while(Idx<FMsgs.length){"
		"var Jdx=0;var CFMsgs=parseInt(FMsgs[Idx].length);while(Jdx<CFMsgs){"
		"Uids.unshift(FMsgs[Idx][Jdx].user_id);Jdx=Jdx+1;};Idx=Idx+1;};"
		"var FUsers=API.users.get({\"user_ids\":Uids,\"name_case\":\"gen\"});"
		"return{\"count\":index,\"datetime\":iTime,\"items\":ret,\"fwd_users\":FUsers,\"once\":%d,\"rcount\":iReqCount};", 
		iOffset, iCount, userID, iTime, iLastMsgId, (int)once);
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveHistoryMessages)
		<< CHAR_PARAM("code", code)
		<< VER_API)->pUserInfo = new CVkSendMsgParam(hContact, iLastMsgId, iOffset);
}

void CVkProto::GetHistoryDlg(MCONTACT hContact, int iLastMsg)
{
	debugLogA("CVkProto::GetHistoryDlg %d", iLastMsg);
	int lastmsgid = -1;
	switch (m_iSyncHistoryMetod) {
	case syncAuto:
		lastmsgid = getDword(hContact, "lastmsgid", -1);
		if (lastmsgid == -1 || !IsOnline()) {
			setDword(hContact, "lastmsgid", iLastMsg);
			return;
		}
		m_bNotifyForEndLoadingHistory = false;
		GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, lastmsgid);
		break;
	case sync1Days:
		GetServerHistoryLastNDay(hContact, 1);
		break;
	case sync3Days:
		GetServerHistoryLastNDay(hContact, 3);
		break;
	}
}

void CVkProto::OnReceiveHistoryMessages(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveHistoryMessages %d", reply->resultCode);
	if (reply->resultCode != 200 || !pReq->pUserInfo) {
		mir_cslock lck(m_csLoadHistoryTask);
		if (m_iLoadHistoryTask > 0)
			m_iLoadHistoryTask--;
		debugLog(_T("CVkProto::OnReceiveHistoryMessages error m_iLoadHistoryTask=%d"), m_iLoadHistoryTask);
		MsgPopup(NULL, TranslateT("Error loading history message from server"), TranslateT("Error"), true);
		return;
	}

	JSONNode jnRoot;
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse) {
		if (!pReq->bNeedsRestart || m_bTerminated) {
			mir_cslock lck(m_csLoadHistoryTask);
			if (m_iLoadHistoryTask > 0) 
				m_iLoadHistoryTask--;
			
			ptrT ptszNick(db_get_tsa(param->hContact, m_szModuleName, "Nick"));
			CMString str(FORMAT, _T("%s for %s"), TranslateT("Error loading history message from server"), ptszNick);

			MsgPopup(param->hContact, str, TranslateT("Error"), true);
			debugLog(_T("CVkProto::OnReceiveHistoryMessages error for %s m_iLoadHistoryTask=%d"), ptszNick, m_iLoadHistoryTask);

			if (m_iLoadHistoryTask == 0 && m_bNotifyForEndLoadingHistoryAllContact) {
				MsgPopup(param->hContact, TranslateT("Loading messages for all contacts was complited"), TranslateT("Loading history"));
				m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = false;
			}

			delete param;
			pReq->pUserInfo = NULL;
		}
		return;
	}

	int iTime = jnResponse["datetime"].as_int(); 
	const JSONNode &jnMsgs = jnResponse["items"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];

	int iLastMsgId = getDword(param->hContact, "lastmsgid", -1);
	time_t tLastReadMessageTime = 0;
	int count = 0;

	for (auto it = jnMsgs.rbegin(); it != jnMsgs.rend(); ++it) {
		const JSONNode &jnMsg = (*it);

		int mid = jnMsg["id"].as_int();
		if (iLastMsgId < mid) 
			iLastMsgId = mid;

		char szMid[40];
		_itoa(mid, szMid, 10);

		CMString tszBody(jnMsg["body"].as_mstring());
		int datetime = jnMsg["date"].as_int();
		int isOut = jnMsg["out"].as_int();
		int isRead = jnMsg["read_state"].as_int(); 
		int uid = jnMsg["user_id"].as_int(); 

		const JSONNode &jnFwdMessages = jnMsg["fwd_messages"];
		if (jnFwdMessages) {
			CMString tszFwdMessages = GetFwdMessages(jnFwdMessages, jnFUsers, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszFwdMessages = _T("\n") + tszFwdMessages;
			tszBody += tszFwdMessages;
		}

		const JSONNode &jnAttachments = jnMsg["attachments"];
		if (jnAttachments) {
			CMString tszAttachmentDescr = GetAttachmentDescr(jnAttachments, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszAttachmentDescr = _T("\n") + tszAttachmentDescr;
			tszBody += tszAttachmentDescr;
		}

		T2Utf pszBody(tszBody);
		MCONTACT hContact = FindUser(uid, true);
		PROTORECVEVENT recv = { 0 };
		if (isRead)
			recv.flags |= PREF_CREATEREAD;
		if (isOut)
			recv.flags |= PREF_SENT;
		recv.timestamp = datetime;
		recv.szMessage = pszBody;
		recv.lParam = isOut;
		recv.pCustomData = szMid;
		recv.cbCustomDataSize = (int)mir_strlen(szMid);
		ProtoChainRecvMsg(hContact, &recv);

		if (isRead && isOut && datetime > tLastReadMessageTime)
			tLastReadMessageTime = datetime;

		count++;
	}
	setDword(param->hContact, "lastmsgid", iLastMsgId);

	if (ServiceExists(MS_MESSAGESTATE_UPDATE)) {
		MessageReadData data(tLastReadMessageTime, MRD_TYPE_MESSAGETIME);
		CallService(MS_MESSAGESTATE_UPDATE, param->hContact, (LPARAM)&data);
	}

	int once = jnResponse["once"].as_int();
	int iRCount = jnResponse["rcount"].as_int();
	if (count == iRCount && once == 0)
		GetServerHistory(param->hContact, param->iCount + count, iRCount, iTime, param->iMsgID);
	else {
		mir_cslock lck(m_csLoadHistoryTask);
		if (m_iLoadHistoryTask > 0)
			m_iLoadHistoryTask--;

		ptrT ptszNick(db_get_tsa(param->hContact, m_szModuleName, "Nick"));
		CMString str(FORMAT, TranslateT("Loading messages for %s was complited"), ptszNick);
		debugLog(_T("CVkProto::OnReceiveHistoryMessages for %s m_iLoadHistoryTask=%d"), ptszNick, m_iLoadHistoryTask);

		if (m_bNotifyForEndLoadingHistory)
			MsgPopup(param->hContact, str, TranslateT("Loading history"));

		if (m_iLoadHistoryTask == 0 && m_bNotifyForEndLoadingHistoryAllContact) {
			MsgPopup(param->hContact, TranslateT("Loading messages for all contacts was complited"), TranslateT("Loading history"));
			m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = false;		
		}
	}

	if (!pReq->bNeedsRestart || m_bTerminated) {
		delete param;
		pReq->pUserInfo = NULL;
	}
}