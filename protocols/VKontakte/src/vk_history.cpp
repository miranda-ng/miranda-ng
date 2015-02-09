/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

INT_PTR __cdecl CVkProto::SvcGetAllServerHistory(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetAllServerHistory");
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

	db_unset(hContact, m_szModuleName, "lastmsgid");
	GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, 0);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetServerHistoryLast1Day(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetServerHistoryLast1Day");
	if (!IsOnline())
		return 0;

	GetServerHistoryLastNDay(hContact, 1);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetServerHistoryLast3Day(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetServerHistoryLast3Day");
	if (!IsOnline())
		return 0;

	GetServerHistoryLastNDay(hContact, 3);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetServerHistoryLast7Day(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetServerHistoryLast7Day");
	if (!IsOnline())
		return 0;

	GetServerHistoryLastNDay(hContact, 7);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetServerHistoryLast30Day(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetServerHistoryLast30Day");
	if (!IsOnline())
		return 0;

	GetServerHistoryLastNDay(hContact, 30);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetServerHistoryLast90Day(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetServerHistoryLast90Day");
	if (!IsOnline())
		return 0;

	GetServerHistoryLastNDay(hContact, 90);
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

	CMStringA code;

	code.AppendFormat("var iOffset=%d;var iReqCount=%d;var userID=%d;var iTime=%d;var lastMid=%d;"
		"var Hist=API.messages.getHistory({\"user_id\":userID,\"count\":iReqCount,\"offset\":iOffset});"
		"var allCount=Hist.count;var ext=Hist.items.length;var index=0;"
		"while(ext!=0){if(Hist.items[index].date>iTime){if(Hist.items[index].id>lastMid)"
		"{index=index+1;ext=ext-1;}else ext=0;}else ext=0;};"
		"var ret=Hist.items.slice(0,index); return{\"count\":index,\"datetime\":iTime,\"items\":ret,\"once\":%d,\"rcount\":iReqCount};", 
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
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	int iCount = json_as_int(json_get(pResponse, "count"));
	int iTime = json_as_int(json_get(pResponse, "datetime"));
	JSONNODE *pMsgs = json_get(pResponse, "items");
	int iLastMsgId = getDword(param->hContact, "lastmsgid", -1);
	int iIndex = iCount;
	int count = 0;

	for (iIndex = (iCount - 1); iIndex >= 0; iIndex--) {
		JSONNODE *pMsg = json_at(pMsgs, iIndex);
		if (pMsg == NULL)
			continue;

		int mid = json_as_int(json_get(pMsg, "id"));
		if (iLastMsgId < mid) 
			iLastMsgId = mid;	

		char szMid[40];
		_itoa(mid, szMid, 10);

		ptrT ptszBody(json_as_string(json_get(pMsg, "body")));
		int datetime = json_as_int(json_get(pMsg, "date"));
		int isOut = json_as_int(json_get(pMsg, "out"));
		int isRead = json_as_int(json_get(pMsg, "read_state"));
		int uid = json_as_int(json_get(pMsg, "user_id"));

		JSONNODE *pFwdMessages = json_get(pMsg, "fwd_messages");
		if (pFwdMessages != NULL){
			CMString tszFwdMessages = GetFwdMessages(pFwdMessages, m_iBBCForAttachments);
			if (!IsEmpty(ptszBody))
				tszFwdMessages = _T("\n") + tszFwdMessages;
			ptszBody = mir_tstrdup(CMString(ptszBody) + tszFwdMessages);
		}

		JSONNODE *pAttachments = json_get(pMsg, "attachments");
		if (pAttachments != NULL){
			CMString tszAttachmentDescr = GetAttachmentDescr(pAttachments, m_iBBCForAttachments);
			if (!IsEmpty(ptszBody))
				tszAttachmentDescr = _T("\n") + tszAttachmentDescr;
			ptszBody = mir_tstrdup(CMString(ptszBody) + tszAttachmentDescr);
		}

		MCONTACT hContact = FindUser(uid, true);
		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_TCHAR;
		if (isRead)
			recv.flags |= PREF_CREATEREAD;
		if (isOut)
			recv.flags |= PREF_SENT;
		recv.timestamp = datetime;
		recv.tszMessage = ptszBody;
		recv.lParam = isOut;
		recv.pCustomData = szMid;
		recv.cbCustomDataSize = (int)mir_strlen(szMid);
		ProtoChainRecvMsg(hContact, &recv);
		count++;
	}
	setDword(param->hContact, "lastmsgid", iLastMsgId);

	int once = json_as_int(json_get(pResponse, "once"));
	int iRCount = json_as_int(json_get(pResponse, "rcount"));
	if (count == iRCount && once == 0)
		GetServerHistory(param->hContact, param->iCount + count, iRCount, iTime, param->iMsgID);

	delete param;
}