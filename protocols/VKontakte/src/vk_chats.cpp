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
	IDM_NONE,
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
	gci.pszID = sid;
	gci.Flags = GCF_BYID | GCF_HCONTACT;
	CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci);
	c->m_hContact = gci.hContact;

	setTString(gci.hContact, "Nick", tszTitle);
	m_chats.insert(c);

	GCDEST gcd = { m_szModuleName, sid, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (int i = SIZEOF(sttStatuses)-1; i >= 0; i--) {
		gce.ptszStatus = TranslateTS(sttStatuses[i]);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	gce.ptszStatus = 0;
	CallServiceSync(MS_GC_EVENT, (m_bHideChats) ? WINDOW_HIDDEN : SESSION_INITDONE, (LPARAM)&gce);
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
	szQuery.AppendFormat("\"users\": API.messages.getChatUsers({\"chat_id\":%d, \"fields\":\"id,first_name,last_name\"})", cc->m_chatid);

	if (!cc->m_bHistoryRead)
		szQuery.AppendFormat(",\"msgs\": API.messages.getHistory({\"chat_id\":%d, \"count\":20, \"rev\":0})", cc->m_chatid);

	szQuery.Append("};");

	debugLogA("CVkProto::RetrieveChantInfo(%d)", cc->m_chatid);

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveChatInfo) 
		<< CHAR_PARAM("code", szQuery)
		<< VER_API )->pUserInfo = cc;
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
			setTString(cc->m_hContact, "Nick", tszTitle);

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_CHANGESESSIONAME };
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

			int uid = json_as_int(json_get(pUser, "id"));
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
			cu->m_tszNick = mir_tstrdup(tszNick);
			cu->m_bUnknown = false;
			
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

	JSONNODE *msgs = json_get(pResponse, "msgs");
	if (msgs != NULL) {
		int numMessages = json_as_int(json_get(msgs, "count"));
		msgs = json_as_array(json_get(msgs, "items"));
		if (msgs != NULL){
			for (int i = 0; i < numMessages; i++) {
				JSONNODE *pMsg = json_at(msgs, i);
				if (pMsg == NULL)
					break;

				AppendChatMessage(cc->m_chatid, pMsg, true);
			}
			cc->m_bHistoryRead = true;
		}
	}

	for (int j = 0; j < cc->m_msgs.getCount(); j++) {
		CVkChatMessage &p = cc->m_msgs[j];
		AppendChatMessage(cc, p.m_mid, p.m_uid, p.m_date, p.m_tszBody, p.m_bHistory);
	}
	cc->m_msgs.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::AppendChatMessage(int id, JSONNODE *pMsg, bool bIsHistory)
{
	CVkChatInfo *cc = AppendChat(id, NULL);
	if (cc == NULL)
		return;

	int mid = json_as_int(json_get(pMsg, "id"));
	int isOut = json_as_int(json_get(pMsg, "out"));
	int uid = json_as_int(json_get(pMsg, "user_id"));

	int msgTime = json_as_int(json_get(pMsg, "date"));
	time_t now = time(NULL);
	if (!msgTime || msgTime > now)
		msgTime = now;

	ptrT tszBody(json_as_string(json_get(pMsg, "body")));
	JSONNODE *pAttachments = json_get(pMsg, "attachments");
	if (pAttachments != NULL)
		tszBody = mir_tstrdup(CMString(tszBody) + GetAttachmentDescr(pAttachments));

	if (cc->m_bHistoryRead)
		AppendChatMessage(cc, mid, uid, msgTime, tszBody, bIsHistory);
	else {
		CVkChatMessage *cm = cc->m_msgs.find((CVkChatMessage *)&mid);
		if (cm == NULL)
			cc->m_msgs.insert(cm = new CVkChatMessage(mid));

		cm->m_uid = uid;
		cm->m_date = msgTime;
		cm->m_tszBody = tszBody.detouch();
		cm->m_bHistory = bIsHistory;
	}
}

void CVkProto::AppendChatMessage(CVkChatInfo *cc, int mid, int uid, int msgTime, LPCTSTR ptszBody, bool bIsHistory)
{
	CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
	if (cu == NULL) {
		cc->m_users.insert(cu = new CVkChatUser(uid));
		cu->m_tszNick = mir_tstrdup(TranslateT("Unknown"));
		cu->m_bUnknown = true;
	}

	TCHAR tszId[20];
	_itot(uid, tszId, 10);

	GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	gce.bIsMe = uid == m_myUserId;
	gce.ptszUID = tszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.ptszNick = cu->m_tszNick;
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

void CVkProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrT tszChatID(getTStringA(hContact, "ChatRoomID"));
	if (tszChatID == NULL)
		return;

	CVkChatInfo *cc = GetChatById(tszChatID);
	if (cc == NULL)
		return;

	GCDEST gcd = { m_szModuleName, tszChatID, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE, (LPARAM)&gce);
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
			AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true,  &CVkProto::OnSendChatMsg)
				<< INT_PARAM("chat_id", cc->m_chatid) 
				<< CHAR_PARAM("message", mir_utf8encodeT(buf))
				<< VER_API;
			pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			Push(pReq);
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
			for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
				TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
				int idx = SendMessage(hwndCombo, CB_ADDSTRING, 0, LPARAM(ptszNick));
				SendMessage(hwndCombo, CB_SETITEMDATA, idx, hContact);
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

LPTSTR CVkProto::ChangeChatTopic(CVkChatInfo *cc)
{
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_MULTILINE;
	pForm.caption = TranslateT("Enter new chat title");
	pForm.ptszInitVal = cc->m_tszTopic;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "gctopic_";
	return (!EnterString(&pForm)) ? NULL : pForm.ptszResult;
}

void CVkProto::LogMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	MCONTACT hContact;

	switch (gch->dwData) {
	case IDM_TOPIC:
		if (LPTSTR ptszNew = ChangeChatTopic(cc)) {
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.editChat.json", true, &CVkProto::OnReceiveSmth)
				<< TCHAR_PARAM("title", ptszNew) 
				<< INT_PARAM("chat_id", cc->m_chatid)
				<< VER_API);
			mir_free(ptszNew);
		}
		break;

	case IDM_INVITE:
		hContact = (MCONTACT)DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INVITE), NULL, InviteDlgProc, (LPARAM)this);
		if (hContact != NULL) {
			int uid = getDword(hContact, "ID", -1);
			if (uid != -1)
				Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth)
					<< INT_PARAM("user_id", uid) 
					<< INT_PARAM("chat_id", cc->m_chatid)
					<< VER_API);
		}
		break;

	case IDM_DESTROY:
		if (IDYES == MessageBox(NULL,
			TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"),
			TranslateT("Warning"), MB_YESNOCANCEL | MB_ICONQUESTION))
		{
			CMStringA code;
			code.Format("var Hist = API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
				"var countMsg = Hist.count;var itemsMsg = Hist.items@.id; "
				"while (countMsg > 1) { API.messages.delete({\"message_ids\":itemsMsg});"
				"Hist=API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
				"countMsg = Hist.count;itemsMsg = Hist.items@.id;}; return 1;", cc->m_chatid, cc->m_chatid);
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnChatDestroy)
				<< CHAR_PARAM("code", code)
				<< VER_API)->pUserInfo = cc;
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
	case IDM_INFO:
		if (MCONTACT hContact = FindUser(cu->m_uid))
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		break;
		
	case IDM_KICK:
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnReceiveSmth)
			<< INT_PARAM("chat_id", cc->m_chatid) 
			<< INT_PARAM("user_id", cu->m_uid)
			<< VER_API);
		cu->m_bUnknown = true;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENT("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ LPGENT("View/change &title"), IDM_TOPIC, MENU_ITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("&Destroy room"), IDM_DESTROY, MENU_ITEM }
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

/////////////////////////////////////////////////////////////////////////////////////////

static void FilterContacts(HWND hwndDlg, CVkProto *ppro)
{
	HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (lstrcmpA(proto, ppro->m_szModuleName) || ppro->isChatRoom(hContact))
			if (HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_FINDCONTACT, hContact, 0))
				SendMessage(hwndClist, CLM_DELETEITEM, (WPARAM)hItem, 0);
	}
}

static void ResetOptions(HWND hwndDlg)
{
	HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
	SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	SendMessage(hwndClist, CLM_GETHIDEOFFLINEROOT, 1, 0);
}

static INT_PTR CALLBACK GcCreateDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CVkProto *ppro = (CVkProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	NMCLISTCONTROL* nmc;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CVkProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
			SetWindowLongPtr(hwndClist, GWL_STYLE,
				GetWindowLongPtr(hwndClist, GWL_STYLE) |  CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
			SendMessage(hwndClist, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

			ResetOptions(hwndDlg);
		}
		return TRUE;

	case WM_NOTIFY:
		nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == IDC_CLIST && nmc->hdr.code == CLN_LISTREBUILT)
			FilterContacts(hwndDlg, ppro);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
			CMStringA uids;
			for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
				if (ppro->isChatRoom(hContact))
					continue;

				if (int hItem = SendMessage(hwndClist, CLM_FINDCONTACT, hContact, 0)) {
					if (SendMessage(hwndClist, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
						int uid = ppro->getDword(hContact, "ID", 0);
						if (uid != NULL) {
							if (!uids.IsEmpty())
								uids.AppendChar(',');
							uids.AppendFormat("%d", uid);
						}
					}
				}
			}

			TCHAR tszTitle[1024];
			GetDlgItemText(hwndDlg, IDC_TITLE, tszTitle, SIZEOF(tszTitle));
			ppro->CreateNewChat(uids, tszTitle);
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}

INT_PTR CVkProto::SvcCreateChat(WPARAM, LPARAM)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GC_CREATE), NULL, GcCreateDlgProc, (LPARAM)this);
	return 0;
}

void CVkProto::CreateNewChat(LPCSTR uids, LPCTSTR ptszTitle)
{
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.createChat.json", true, &CVkProto::OnCreateNewChat)
		<< TCHAR_PARAM("title", ptszTitle) 
		<< CHAR_PARAM("user_ids", uids)
		<< VER_API);
}

void CVkProto::OnCreateNewChat(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnCreateNewChat %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	int chat_id = json_as_int(pResponse);
	if (chat_id != NULL)
		AppendChat(chat_id, NULL);
}
