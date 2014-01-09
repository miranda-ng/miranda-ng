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

static LPCTSTR sttStatuses[] = { LPGENT("Participants"), LPGENT("Owners") };

CVkChatInfo* CVkProto::AppendChat(int id, JSONNODE *pDlg)
{
	if (id == 0)
		return NULL;

	CVkChatInfo *c = m_chats.find((CVkChatInfo*)&id);
	if (c != NULL)
		return c;

	ptrT tszTitle;
	c = new CVkChatInfo(id);
	if (pDlg != NULL) {
		tszTitle = json_as_string(json_get(pDlg, "title"));
		c->m_tszTitle = mir_tstrdup((tszTitle != NULL) ? tszTitle : _T(""));
	}

	CMString sid; sid.Format(_T("%S_%d"), m_szModuleName, id);
	c->m_tszId = mir_tstrdup(sid);

	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = tszTitle;
	gcw.ptszID = sid;
	CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw);

	GC_INFO gci = { 0 };
	gci.pszModule = m_szModuleName;
	gci.pszID = sid.GetBuffer();
	gci.Flags = BYID | HCONTACT;
	CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);
	c->m_hContact = gci.hContact;
	m_chats.insert(c);

	GCDEST gcd = { m_szModuleName, sid.GetBuffer(), GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (int i = SIZEOF(sttStatuses)-1; i >= 0; i--) {
		gce.ptszStatus = TranslateTS(sttStatuses[i]);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	gce.ptszStatus = 0;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	RetrieveChatInfo(c);
	return c;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveChatInfo(CVkChatInfo *cc)
{
	CMStringA szQuery("return { ");

	// retrieve title if empty
	if (!lstrlen(cc->m_tszTitle))
		szQuery.AppendFormat("\"info\": API.messages.getChat({\"chat_id\":%d}),", cc->m_chatid);

	// retrieve users
	szQuery.AppendFormat("\"users\": API.messages.getChatUsers({\"chat_id\":%d, \"fields\":\"uid,first_name,last_name,photo\"})", cc->m_chatid);

	if (!cc->m_bHistoryRead) {
		cc->m_bHistoryRead = true;
		szQuery.AppendFormat(",\"msgs\": API.messages.getHistory({\"chat_id\":%d, \"count\":\"20\", \"rev\":\"0\"})", cc->m_chatid);
	}

	szQuery.Append("};");

	debugLogA("CVkProto::RetrieveChantInfo(%d)", cc->m_chatid);

	HttpParam params[] = {
		{ "code", szQuery },
		{ "access_token", m_szAccessToken }
	};
	AsyncHttpRequest *pReq = PushAsyncHttpRequest(REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveChatInfo, SIZEOF(params), params);
	pReq->pUserInfo = cc;
}

void CVkProto::OnReceiveChatInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveChatInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	if (m_chats.indexOf(cc) == -1)
		return;

	JSONNODE *info = json_get(pResponse, "info");
	if (info != NULL) {
		ptrT tszTitle(json_as_string(json_get(info, "title")));
		cc->m_tszTitle = mir_tstrdup(tszTitle);

		cc->m_admin_id = json_as_int(json_get(info, "admin_id"));

		JSONNODE *userids = json_as_array(json_get(info, "users"));
		if (userids) {
			for (int i = 0;; i++) {
				int uid = json_as_int(json_at(userids, i));
				if (uid == 0)
					break;

				CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
				if (cu == NULL)
					cc->m_users.insert(cu = new CVkChatUser(uid));
			}
		}
	}

	JSONNODE *users = json_as_array(json_get(pResponse, "users"));
	if (users != NULL) {
		for (int i = 0;; i++) {
			JSONNODE *pUser = json_at(users, i);
			if (pUser == NULL)
				break;

			int uid = json_as_int(json_get(pUser, "uid"));
			TCHAR tszId[20];
			_itot(uid, tszId, 10);

			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == NULL)
				cc->m_users.insert(cu = new CVkChatUser(uid));

			ptrT fName(json_as_string(json_get(pUser, "first_name")));
			ptrT lName(json_as_string(json_get(pUser, "last_name")));
			CMString tszNick = CMString(fName).Trim() + _T(" ") + CMString(lName).Trim();
			cu->m_tszTitle = mir_tstrdup(tszNick);
			
			cu->m_tszImage = json_as_string(json_get(pUser, "photo"));

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_JOIN };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.bIsMe = uid == m_myUserId;
			gce.ptszUID = tszId;
			gce.ptszNick = tszNick;
			gce.ptszStatus = TranslateTS(sttStatuses[uid == cc->m_admin_id]);
			gce.dwItemData = (INT_PTR)cu;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
	}

	JSONNODE *msgs = json_as_array(json_get(pResponse, "msgs"));
	if (msgs != NULL) {
		for (int i = 1;; i++) {
			JSONNODE *pMsg = json_at(msgs, i);
			if (pMsg == NULL)
				break;

			int uid = json_as_int(json_get(pMsg, "from_id"));
			TCHAR tszId[20];
			_itot(uid, tszId, 10);

			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == NULL) {
				cc->m_users.insert(cu = new CVkChatUser(uid));
				cu->m_tszTitle = mir_tstrdup(TranslateT("Uknown"));
			}

			int iDate = json_as_int(json_get(pMsg, "date"));
			ptrT tszBody(json_as_string(json_get(pMsg, "body")));

			JSONNODE *pAttachments = json_get(pMsg, "attachments");
			if (pAttachments != NULL)
				tszBody = mir_tstrdup(CMString(tszBody) + GetAttachmentDescr(pAttachments));

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_MESSAGE };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.bIsMe = uid == m_myUserId;
			gce.ptszUID = tszId;
			gce.time = iDate;
			gce.ptszNick = cu->m_tszTitle;
			gce.ptszText = tszBody;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
	}
}
