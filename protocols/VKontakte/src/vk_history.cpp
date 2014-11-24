/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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
	if (userID == -1)
		return 0;

	setByte(hContact, "ImportHistory", 1);
	setDword(hContact, "lastmsgid", 0);

	HANDLE hDBEvent = db_event_first(hContact);
	while (hDBEvent) {
		HANDLE hDBEventNext = db_event_next(hContact, hDBEvent);
		db_event_delete(hContact, hDBEvent);
		hDBEvent = hDBEventNext;
	}

	debugLogA("CVkProto::SvcGetAllServerHistory");
	GetHistoryDlgMessages(hContact, 0, INT_MAX, -1);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GetHistoryDlg(MCONTACT hContact, int iLastMsg)
{
	debugLogA("CVkProto::GetHistoryDlg %d", iLastMsg);
	int lastmsgid = getDword(hContact, "lastmsgid", -1);
	if (lastmsgid == -1 || !IsOnline()) {
		setDword(hContact, "lastmsgid", iLastMsg);
		return;
	}
	int maxOffset = iLastMsg - lastmsgid;
	setDword(hContact, "new_lastmsgid", iLastMsg);
	GetHistoryDlgMessages(hContact, 0, maxOffset, -1);
}

void CVkProto::GetHistoryDlgMessages(MCONTACT hContact, int iOffset, int iMaxCount, int lastcount)
{
	debugLogA("CVkProto::GetHistoryDlgMessages");
	LONG userID = getDword(hContact, "ID", -1);
	if (-1 == userID)
		return;

	if (lastcount == 0 || iMaxCount < 1 || !IsOnline()) {
		setDword(hContact, "lastmsgid", getDword(hContact, "new_lastmsgid", -1));
		db_unset(hContact, m_szModuleName, "new_lastmsgid");
		if (getBool(hContact, "ImportHistory", false))
			MsgPopup(hContact, TranslateT("History import is complete"), TranslateT("History import"));
		db_unset(hContact, m_szModuleName, "ImportHistory");
		return;
	}

	int iReqCount = iMaxCount > MAXHISTORYMIDSPERONE ? MAXHISTORYMIDSPERONE : iMaxCount;

	debugLogA("CVkProto::GetHistoryDlgMessages %d %d %d", userID, iOffset, iReqCount);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getHistory.json", true, &CVkProto::OnReceiveHistoryMessages)
		<< INT_PARAM("offset", iOffset)
		<< INT_PARAM("count", iReqCount)
		<< INT_PARAM("user_id", userID)
		<< VER_API;

	pReq->pUserInfo = new CVkSendMsgParam(hContact, iOffset, iMaxCount);
	Push(pReq);
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

	CMStringA mids;
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	int numMessages = json_as_int(json_get(pResponse, "count")),
		i = 0,
		lastmsgid = getDword(param->hContact, "lastmsgid", -1),
		mid = -1;

	JSONNODE *pMsgs = json_get(pResponse, "items");

	int new_lastmsgid = getDword(param->hContact, "new_lastmsgid", -1);
	int his = getByte(param->hContact, "ImportHistory", 0);

	for (i = 0; i < numMessages; i++) {
		JSONNODE *pMsg = json_at(pMsgs, i);
		if (pMsg == NULL)
			break;

		mid = json_as_int(json_get(pMsg, "id"));
		if (his && new_lastmsgid == -1) {
			new_lastmsgid = mid;
			setDword(param->hContact, "new_lastmsgid", mid);
		}
		if (mid <= lastmsgid)
			break;

		char szMid[40];
		_itoa(mid, szMid, 10);

		ptrT ptszBody(json_as_string(json_get(pMsg, "body")));
		int datetime = json_as_int(json_get(pMsg, "date"));
		int isOut = json_as_int(json_get(pMsg, "out"));
		int isRead = json_as_int(json_get(pMsg, "read_state"));
		int uid = json_as_int(json_get(pMsg, "user_id"));

		JSONNODE *pAttachments = json_get(pMsg, "attachments");
		if (pAttachments != NULL)
			ptszBody = mir_tstrdup(CMString(ptszBody) + GetAttachmentDescr(pAttachments));

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
		recv.cbCustomDataSize = (int)strlen(szMid);
		ProtoChainRecvMsg(hContact, &recv);
	}
	GetHistoryDlgMessages(param->hContact, param->iMsgID + i, param->iCount - i, i);
	delete param;
}

/////////////////////////////////////////////////////////////////////////////////////////