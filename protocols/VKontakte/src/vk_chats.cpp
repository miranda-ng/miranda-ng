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

	// retrieve title & owner id
	szQuery.AppendFormat("\"info\": API.messages.getChat({\"chat_id\":%d}),", cc->m_chatid);

	// retrieve users
	szQuery.AppendFormat("\"users\": API.messages.getChatUsers({\"chat_id\":%d, \"fields\":\"uid,first_name,last_name\"})", cc->m_chatid);

	if (!cc->m_bHistoryRead) {
		cc->m_bHistoryRead = true;
		szQuery.AppendFormat(",\"msgs\": API.messages.getHistory({\"chat_id\":%d, \"count\":\"20\", \"rev\":\"1\"})", cc->m_chatid);
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
		if (lstrcmp(tszTitle, cc->m_tszTitle)) {
			cc->m_tszTitle = mir_tstrdup(tszTitle);

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_TOPIC };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszText = tszTitle;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
		
		cc->m_admin_id = json_as_int(json_get(info, "admin_id"));
	}

	JSONNODE *users = json_as_array(json_get(pResponse, "users"));
	if (users != NULL) {
		for (int i = 0; i < cc->m_users.getCount(); i++)
			cc->m_users[i].m_bDel = true;

		for (int i = 0;; i++) {
			JSONNODE *pUser = json_at(users, i);
			if (pUser == NULL)
				break;

			int uid = json_as_int(json_get(pUser, "uid"));
			TCHAR tszId[20];
			_itot(uid, tszId, 10);

			bool bNew = false;
			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == NULL) {
				cc->m_users.insert(cu = new CVkChatUser(uid));
				bNew = true;
			}
			cu->m_bDel = false;

			ptrT fName(json_as_string(json_get(pUser, "first_name")));
			ptrT lName(json_as_string(json_get(pUser, "last_name")));
			CMString tszNick = CMString(fName).Trim() + _T(" ") + CMString(lName).Trim();
			cu->m_tszTitle = mir_tstrdup(tszNick);
			
			cu->m_tszImage = json_as_string(json_get(pUser, "photo"));

			if (bNew) {
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

		for (int i = cc->m_users.getCount() - 1; i >= 0; i--) {
			CVkChatUser &cu = cc->m_users[i];
			if (!cu.m_bDel)
				continue;

			TCHAR tszId[20];
			_itot(cu.m_uid, tszId, 10);

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_PART };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszUID = tszId;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
	}

	JSONNODE *msgs = json_as_array(json_get(pResponse, "msgs"));
	if (msgs != NULL) {
		for (int i = 1;; i++) {
			JSONNODE *pMsg = json_at(msgs, i);
			if (pMsg == NULL)
				break;

			AppendChatMessage(cc->m_chatid, pMsg, true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::AppendChatMessage(int id, JSONNODE *pMsg, bool bIsHistory)
{
	CVkChatInfo *cc = AppendChat(id, NULL);
	if (cc == NULL)
		return;

	int mid = json_as_int(json_get(pMsg, "mid"));
	int isOut = json_as_int(json_get(pMsg, "out"));
	if (mid && isOut)
		m_sendIds.insert((void*)mid);

	int uid = json_as_int(json_get(pMsg, "uid"));
	TCHAR tszId[20];
	_itot(uid, tszId, 10);

	int msgTime = json_as_int(json_get(pMsg, "date"));
	time_t now = time(NULL);
	if (!msgTime || msgTime > now)
		msgTime = now;

	CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
	if (cu == NULL) {
		cc->m_users.insert(cu = new CVkChatUser(uid));
		cu->m_tszTitle = mir_tstrdup(TranslateT("Unknown"));
	}

	ptrT tszBody(json_as_string(json_get(pMsg, "body")));
	JSONNODE *pAttachments = json_get(pMsg, "attachments");
	if (pAttachments != NULL)
		tszBody = mir_tstrdup(CMString(tszBody) + GetAttachmentDescr(pAttachments));

	GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	gce.bIsMe = uid == m_myUserId;
	gce.ptszUID = tszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.ptszNick = cu->m_tszTitle;
	gce.ptszText = tszBody;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
}

/////////////////////////////////////////////////////////////////////////////////////////

TCHAR* UnEscapeChatTags(TCHAR* str_in)
{
	TCHAR *s = str_in, *d = str_in;
	while (*s) {
		if (*s == '%' && s[1] == '%')
			s++;
		*d++ = *s++;
	}
	*d = 0;
	return str_in;
}

int CVkProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK* gch = (GCHOOK*)lParam;
	if (gch == NULL)
		return 0;

	if (lstrcmpiA(gch->pDest->pszModule, m_szModuleName))
		return 0;

	CVkChatInfo *cc = NULL;
	for (int i = 0; i < m_chats.getCount(); i++)
	if (!lstrcmp(m_chats[i].m_tszId, gch->pDest->ptszID)) {
		cc = &m_chats[i];
		break;
	}
	if (cc == NULL)
		return 0;

	switch (gch->pDest->iType) {
	case GC_USER_MESSAGE:
		if (m_bOnline && lstrlen(gch->ptszText) > 0) {
			TCHAR *buf = NEWTSTR_ALLOCA(gch->ptszText);
			rtrimt(buf);
			UnEscapeChatTags(buf);
			ptrA szMsg(mir_utf8encodeT(buf));
			
			char szId[20];
			_itoa(cc->m_chatid, szId, 10);

			HttpParam params[] = {
				{ "type", "1" },
				{ "chat_id", szId },
				{ "message", szMsg },
				{ "access_token", m_szAccessToken }
			};
			PushAsyncHttpRequest(REQUEST_GET, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, SIZEOF(params), params);
		}
		break;
	}
	return 0;
}

void CVkProto::OnSendChatMsg(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSendChatMsg %d", reply->resultCode);
	if (reply->resultCode == 200) {
		JSONROOT pRoot;
		JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
		if (pResponse != NULL)
			m_sendIds.insert((HANDLE)json_as_int(pResponse));
	}
}

