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

enum
{
	IDM_TOPIC, IDM_INVITE, IDM_DESTROY,
	IDM_KICK, IDM_INFO
};

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
		c->m_tszTopic = mir_tstrdup((tszTitle != NULL) ? tszTitle : _T(""));
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
	CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci);
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

	if (!cc->m_bHistoryRead)
		szQuery.AppendFormat(",\"msgs\": API.messages.getHistory({\"chat_id\":%d, \"count\":\"20\", \"rev\":\"1\"})", cc->m_chatid);

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
		if (lstrcmp(tszTitle, cc->m_tszTopic)) {
			cc->m_tszTopic = mir_tstrdup(tszTitle);

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

			bool bNew;
			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == NULL) {
				cc->m_users.insert(cu = new CVkChatUser(uid));
				bNew = true;
			}
			else bNew = cu->m_bUnknown;
			cu->m_bDel = false;

			ptrT fName(json_as_string(json_get(pUser, "first_name")));
			ptrT lName(json_as_string(json_get(pUser, "last_name")));
			CMString tszNick = CMString(fName).Trim() + _T(" ") + CMString(lName).Trim();
			cu->m_tszTitle = mir_tstrdup(tszNick);
			cu->m_bUnknown = false;
			
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

			cc->m_users.remove(i);
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
		cc->m_bHistoryRead = true;
	}

	for (int j = 0; j < cc->m_msgs.getCount(); j++) {
		CVkChatMessage &p = cc->m_msgs[j];
		AppendChatMessage(cc, p.m_uid, p.m_date, p.m_tszBody, false);
	}
	cc->m_msgs.destroy();
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

	int msgTime = json_as_int(json_get(pMsg, "date"));
	time_t now = time(NULL);
	if (!msgTime || msgTime > now)
		msgTime = now;

	ptrT tszBody(json_as_string(json_get(pMsg, "body")));
	JSONNODE *pAttachments = json_get(pMsg, "attachments");
	if (pAttachments != NULL)
		tszBody = mir_tstrdup(CMString(tszBody) + GetAttachmentDescr(pAttachments));

	if (cc->m_bHistoryRead)
		AppendChatMessage(cc, uid, msgTime, tszBody, bIsHistory);
	else {
		CVkChatMessage *cm = new CVkChatMessage();
		cm->m_mid = mid;
		cm->m_uid = uid;
		cm->m_date = msgTime;
		cm->m_tszBody = tszBody.detouch();
		cc->m_msgs.insert(cm);
	}
}

void CVkProto::AppendChatMessage(CVkChatInfo *cc, int uid, int msgTime, LPCTSTR ptszBody, bool bIsHistory)
{
	TCHAR tszId[20];
	_itot(uid, tszId, 10);

	CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
	if (cu == NULL) {
		cc->m_users.insert(cu = new CVkChatUser(uid));
		cu->m_tszTitle = mir_tstrdup(TranslateT("Unknown"));
		cu->m_bUnknown = true;
	}

	GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	gce.bIsMe = uid == m_myUserId;
	gce.ptszUID = tszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.ptszNick = cu->m_tszTitle;
	gce.ptszText = ptszBody;
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatInfo* CVkProto::GetChatById(LPCTSTR ptszId)
{
	for (int i = 0; i < m_chats.getCount(); i++)
		if (!lstrcmp(m_chats[i].m_tszId, ptszId))
			return &m_chats[i];

	return NULL;
}

CVkChatUser* CVkChatInfo::GetUserById(LPCTSTR ptszId)
{
	int user_id = _ttoi(ptszId);
	return m_users.find((CVkChatUser*)&user_id);
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
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == NULL)
		return 0;

	if (lstrcmpiA(gch->pDest->pszModule, m_szModuleName))
		return 0;

	CVkChatInfo *cc = GetChatById(gch->pDest->ptszID);
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

	case GC_USER_LOGMENU:
		LogMenuHook(cc, gch);
		break;

	case GC_USER_NICKLISTMENU:
		NickMenuHook(cc, gch);
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

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK InviteDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CVkProto *ppro = (CVkProto*)lParam;
			HWND hwndCombo = GetDlgItem(hwndDlg, IDC_CONTACT);
			for (HANDLE hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
				TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
				int idx = SendMessage(hwndCombo, CB_ADDSTRING, 0, LPARAM(ptszNick));
				SendMessage(hwndCombo, CB_SETITEMDATA, idx, (LPARAM)hContact);
			}
			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			int idx = SendDlgItemMessage(hwndDlg, IDC_CONTACT, CB_GETCURSEL, 0, 0);
			if (idx != -1)
				EndDialog(hwndDlg, SendDlgItemMessage(hwndDlg, IDC_CONTACT, CB_GETITEMDATA, idx, 0));
			else
				EndDialog(hwndDlg, 0);
			return TRUE;
		}		
	}

	return 0;
}

void CVkProto::LogMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	HANDLE hContact;
	char szChatId[20];
	_itoa(cc->m_chatid, szChatId, 10);

	switch (gch->dwData) {
	case IDM_INVITE:
		hContact = (HANDLE)DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INVITE), NULL, InviteDlgProc, (LPARAM)this);
		if (hContact != NULL) {
			int uid = getDword(hContact, "ID", -1);
			if (uid == -1) break;

			char szUid[20];
			_itoa(uid, szUid, 10);

			HttpParam params[] = {
				{ "uid", szUid },
				{ "chat_id", szChatId },
				{ "access_token", m_szAccessToken }
			};
			PushAsyncHttpRequest(REQUEST_GET, "/method/messages.addChatUser.json", true, NULL, SIZEOF(params), params);
		}
		break;

	case IDM_DESTROY:
		if (IDYES == MessageBox(NULL,
			TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"),
			TranslateT("Warning"), MB_YESNOCANCEL | MB_ICONQUESTION))
		{
			HttpParam params[] = {
				{ "chat_id", szChatId },
				{ "access_token", m_szAccessToken }
			};
			AsyncHttpRequest *pReq = PushAsyncHttpRequest(REQUEST_GET, "/method/messages.deleteDialog.json", true, &CVkProto::OnChatDestroy, SIZEOF(params), params);
			pReq->pUserInfo = cc;
		}
		break;
	}
}

void CVkProto::OnChatDestroy(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatDestroy %d", reply->resultCode);
	if (reply->resultCode == 200) {
		CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;

		GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_QUIT };
		GCEVENT gce = { sizeof(GCEVENT), &gcd };
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

		CallService(MS_DB_CONTACT_DELETE, (WPARAM)cc->m_hContact, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::NickMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	CVkChatUser* cu = cc->GetUserById(gch->ptszUID);
	if (cu == NULL)
		return;

	char szUid[20], szChatId[20];
	_itoa(cu->m_uid, szUid, 10);
	_itoa(cc->m_chatid, szChatId, 10);

	switch (gch->dwData) {
	case IDM_KICK:
		HttpParam params[] = {
			{ "chat_id", szChatId },
			{ "uid", szUid },
			{ "access_token", m_szAccessToken }
		};
		PushAsyncHttpRequest(REQUEST_GET, "/method/messages.removeChatUser.json", true, NULL, SIZEOF(params), params);
		cu->m_bUnknown = true;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENT("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ LPGENT("View/change &topic"), IDM_TOPIC, MENU_POPUPITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("&Destroy room"), IDM_DESTROY, MENU_POPUPITEM }
};

static gc_item sttListItems[] =
{
	{ LPGENT("&User details"), IDM_INFO, MENU_ITEM },
	{ LPGENT("&Kick"), IDM_KICK, MENU_ITEM }
};

int CVkProto::OnGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (lstrcmpiA(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		gcmi->nItems = SIZEOF(sttLogListItems);
		gcmi->Item = sttLogListItems;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		gcmi->nItems = SIZEOF(sttListItems);
		gcmi->Item = sttListItems;
	}
	return 0;
}
