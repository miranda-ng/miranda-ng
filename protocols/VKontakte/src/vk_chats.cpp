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

enum
{
	IDM_NONE,
	IDM_TOPIC, IDM_INVITE, IDM_DESTROY,
	IDM_KICK, IDM_INFO, IDM_VISIT_PROFILE
};

static LPCTSTR sttStatuses[] = { LPGENT("Participants"), LPGENT("Owners") };

extern JSONNode nullNode;

CVkChatInfo* CVkProto::AppendChat(int id, const JSONNode &jnDlg)
{
	debugLog(_T("CVkProto::AppendChat"));
	if (id == 0)
		return NULL;

	MCONTACT chatContact = FindChat(id);
	if (chatContact)
		if (getBool(chatContact, "kicked", false))
			return NULL;

	CVkChatInfo *c = m_chats.find((CVkChatInfo*)&id);
	if (c != NULL)
		return c;

	CMString tszTitle;
	c = new CVkChatInfo(id);
	if (!jnDlg.isnull()) {
		tszTitle = jnDlg["title"].as_mstring();
		c->m_tszTopic = mir_tstrdup(!tszTitle.IsEmpty() ? tszTitle : _T(""));
	}

	CMString sid; 
	sid.Format(_T("%S_%d"), m_szModuleName, id);
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

	setDword(gci.hContact, "vk_chat_id", id);
	db_unset(gci.hContact, m_szModuleName, "off");

	if (jnDlg["left"].as_int() == 1) {
		setByte(gci.hContact, "off", 1);
		m_chats.remove(c);
		return NULL;
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

	CMString tszQuery;
	tszQuery.AppendFormat(_T("var ChatId=%d;"), cc->m_chatid);
	tszQuery += _T("var Info=API.messages.getChat({\"chat_id\":ChatId});");
	tszQuery += _T("var ChatUsers=API.messages.getChatUsers({\"chat_id\":ChatId,\"fields\":\"id,first_name,last_name\"});");

	if (!cc->m_bHistoryRead) {
		tszQuery += _T("var ChatMsg=API.messages.getHistory({\"chat_id\":ChatId,\"count\":20,\"rev\":0});");
		tszQuery += _T("var MsgUsers=API.users.get({\"user_ids\":ChatMsg.items@.user_id,\"fields\":\"id,first_name,last_name\"});");
	}

	tszQuery += _T("return {\"info\":Info,\"users\":ChatUsers");

	if (!cc->m_bHistoryRead)
		tszQuery += _T(",\"msgs\":ChatMsg,\"msgs_users\":MsgUsers");

	tszQuery +=_T("};");

	debugLogA("CVkProto::RetrieveChantInfo(%d)", cc->m_chatid);
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveChatInfo)
		<< TCHAR_PARAM("code", tszQuery)
		<< VER_API)->pUserInfo = cc;
}

void CVkProto::OnReceiveChatInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveChatInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	if (m_chats.indexOf(cc) == -1)
		return;

	const JSONNode &jnInfo = jnResponse["info"];
	if (!jnInfo.isnull()) {
		CMString tszTitle(jnInfo["title"].as_mstring());
		if (tszTitle == (cc->m_tszTopic ? cc->m_tszTopic : _T(""))) {		
			cc->m_tszTopic = mir_tstrdup(tszTitle);
			setTString(cc->m_hContact, "Nick", tszTitle);

			GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_CHANGESESSIONAME };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszText = tszTitle;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
		if (jnInfo["left"].as_int() == 1 || jnInfo["kicked"].as_int() == 1) {
			setByte(cc->m_hContact, "kicked", (int)true);
			LeaveChat(cc->m_chatid);
			return;
		}
		cc->m_admin_id = jnInfo["admin_id"].as_int();
	}

	const JSONNode &jnUsers = jnResponse["users"];
	if (!jnUsers.isnull()) {
		for (int i = 0; i < cc->m_users.getCount(); i++)
			cc->m_users[i].m_bDel = true;

		for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
			const JSONNode &jnUser = (*it);
			if (!jnUser)
				break;

			int uid = jnUser["id"].as_int();
			TCHAR tszId[20];
			_itot(uid, tszId, 10);

			bool bNew;
			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == NULL) {
				cc->m_users.insert(cu = new CVkChatUser(uid));
				bNew = true;
			}
			else 
				bNew = cu->m_bUnknown;
			cu->m_bDel = false;

			CMString fName(jnUser["first_name"].as_mstring());
			CMString lName(jnUser["last_name"].as_mstring());
			CMString tszNick = fName.Trim() + _T(" ") + lName.Trim();
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

	const JSONNode &jnMsgsUsers = jnResponse["msgs_users"];
	for (auto it = jnMsgsUsers.begin(); it != jnMsgsUsers.end(); ++it) {
		const JSONNode &jnUser = (*it);
		LONG uid = jnUser["id"].as_int();
		CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
		if (cu)
			continue;
		
		MCONTACT hContact = FindUser(uid);
		if (hContact)
			continue;

		hContact = SetContactInfo(jnUser, true);
		db_set_b(hContact, "CList", "Hidden", 1);
		db_set_b(hContact, "CList", "NotOnList", 1);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}

	const JSONNode &jnMsgs = jnResponse["msgs"];
	if (!jnMsgs.isnull()) {
		
		const JSONNode &jnItems = jnMsgs["items"];
		if (!jnItems.isnull()) {
			for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
				const JSONNode &jnMsg = (*it);
				if (!jnMsg)
					break;

				AppendChatMessage(cc->m_chatid, jnMsg, true);
			}
			cc->m_bHistoryRead = true;
		}
	}

	for (int j = 0; j < cc->m_msgs.getCount(); j++) {
		CVkChatMessage &p = cc->m_msgs[j];
		AppendChatMessage(cc, p.m_uid, p.m_date, p.m_tszBody, p.m_bHistory, p.m_bIsAction);
	}
	cc->m_msgs.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::AppendChatMessage(int id, const JSONNode &jnMsg, bool bIsHistory)
{
	debugLogA("CVkProto::AppendChatMessage");
	CVkChatInfo *cc = AppendChat(id, nullNode);
	if (cc == NULL)
		return;

	int mid = jnMsg["id"].as_int();
	int uid = jnMsg["user_id"].as_int();
	bool bIsAction = false;

	int msgTime = jnMsg["date"].as_int();
	time_t now = time(NULL);
	if (!msgTime || msgTime > now)
		msgTime = now;

	CMString tszBody(jnMsg["body"].as_mstring());
	
	const JSONNode &jnFwdMessages = jnMsg["fwd_messages"];
	if (!jnFwdMessages.isnull()){
		CMString tszFwdMessages = GetFwdMessages(jnFwdMessages, bbcNo);
		if (!tszBody.IsEmpty())
			tszFwdMessages = _T("\n") + tszFwdMessages;
		tszBody += tszFwdMessages;
	}

	const JSONNode &jnAttachments = jnMsg["attachments"];
	if (!jnAttachments.isnull()){
		CMString tszAttachmentDescr = GetAttachmentDescr(jnAttachments, bbcNo);
		if (!tszBody.IsEmpty())
			tszAttachmentDescr = _T("\n") + tszAttachmentDescr;
		tszBody +=  tszAttachmentDescr;
	}

	if (tszBody.IsEmpty() && !jnMsg["action"].isnull()){
		bIsAction = true;
		CMString tszAction = jnMsg["action"].as_mstring();
		
		if (tszAction.IsEmpty())
			tszBody = _T("...");
		else if (tszAction == _T("chat_create")) {
			CMString tszActionText = jnMsg["action_text"].as_mstring();
			tszBody.AppendFormat(_T("%s \"%s\""), TranslateT("create chat"), tszActionText.IsEmpty() ? _T(" ") : tszActionText);
		}
		else if (tszAction == _T("chat_kick_user")) {
			CMString tszActionMid = jnMsg["action_mid"].as_mstring();
			if (tszActionMid.IsEmpty())
				tszBody = TranslateT("kick user");
			else {
				CMString tszUid;
				tszUid.AppendFormat(_T("%d"), uid);
				if (tszUid == tszActionMid)
					tszBody.AppendFormat(_T(" (https://vk.com/id%s) %s"), tszUid, TranslateT("left chat"));
				else {
					int a_uid = 0;
					int iReadCount = _stscanf(tszActionMid, _T("%d"), &a_uid);
					if (iReadCount == 1) {
						CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&a_uid);
						if (cu == NULL)
							tszBody.AppendFormat(_T("%s (https://vk.com/id%d)"), TranslateT("kick user"), a_uid);
						else
							tszBody.AppendFormat(_T("%s %s (https://vk.com/id%d)"), TranslateT("kick user"), cu->m_tszNick, a_uid);
					}
					else 
						tszBody = TranslateT("kick user");
				}
			}
		}
		else if (tszAction == _T("chat_invite_user")) {
			CMString tszActionMid = jnMsg["action_mid"].as_mstring();
			if (tszActionMid.IsEmpty())
				tszBody = TranslateT("invite user");
			else {
				int a_uid = 0;
				int iReadCount = _stscanf(tszActionMid, _T("%d"), &a_uid);
				if (iReadCount == 1) {
					CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&a_uid);
					if (cu == NULL)
						tszBody.AppendFormat(_T("%s (https://vk.com/id%d)"), TranslateT("invite user"), a_uid);
					else
						tszBody.AppendFormat(_T("%s %s (https://vk.com/id%d)"), TranslateT("invite user"), cu->m_tszNick, a_uid);
				}
				else
					tszBody = TranslateT("invite user");
			}
		}
		else if (tszAction == _T("chat_title_update")) {
			CMString tszTitle = jnMsg["action_text"].as_mstring();
			tszBody.AppendFormat(_T("%s \"%s\""), TranslateT("change chat title to"), tszTitle.IsEmpty() ? _T(" ") : tszTitle);

			if (!bIsHistory && tszTitle != (cc->m_tszTopic ? cc->m_tszTopic : _T(""))) {
				cc->m_tszTopic = mir_tstrdup(tszTitle);
				setTString(cc->m_hContact, "Nick", tszTitle);

				GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_CHANGESESSIONAME };
				GCEVENT gce = { sizeof(GCEVENT), &gcd };
				gce.ptszText = tszTitle;
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
			}
		}
		else
			tszBody.AppendFormat(_T(": %s (%s)"), TranslateT("chat action not supported"), tszAction);
	}

	if (cc->m_bHistoryRead)
		AppendChatMessage(cc, uid, msgTime, tszBody, bIsHistory, bIsAction);
	else {
		CVkChatMessage *cm = cc->m_msgs.find((CVkChatMessage *)&mid);
		if (cm == NULL)
			cc->m_msgs.insert(cm = new CVkChatMessage(mid));

		cm->m_uid = uid;
		cm->m_date = msgTime;
		cm->m_tszBody = mir_tstrdup(tszBody);
		cm->m_bHistory = bIsHistory;
		cm->m_bIsAction = bIsAction;
	}
}

void CVkProto::AppendChatMessage(CVkChatInfo *cc, int uid, int msgTime, LPCTSTR ptszBody, bool bIsHistory, bool bIsAction)
{
	debugLogA("CVkProto::AppendChatMessage2");
	MCONTACT hContact = FindUser(uid);
	CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
	if (cu == NULL) {
		cc->m_users.insert(cu = new CVkChatUser(uid));
		cu->m_tszNick = mir_tstrdup(hContact ? ptrT(db_get_tsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown"));
		cu->m_bUnknown = true;
	}

	TCHAR tszId[20];
	_itot(uid, tszId, 10);

	GCDEST gcd = { m_szModuleName, cc->m_tszId, bIsAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	gce.bIsMe = (uid == m_myUserId);
	gce.ptszUID = tszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.ptszNick = cu->m_tszNick ? mir_tstrdup(cu->m_tszNick) : mir_tstrdup(hContact ? ptrT(db_get_tsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown"));
	gce.ptszText = IsEmpty((TCHAR *)ptszBody) ? mir_tstrdup(_T("...")) : mir_tstrdup(ptszBody);
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatInfo* CVkProto::GetChatById(LPCTSTR ptszId)
{
	for (int i = 0; i < m_chats.getCount(); i++)
		if (!mir_tstrcmp(m_chats[i].m_tszId, ptszId))
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

	if (mir_strcmpi(gch->pDest->pszModule, m_szModuleName))
		return 0;

	CVkChatInfo *cc = GetChatById(gch->pDest->ptszID);
	if (cc == NULL)
		return 0;

	switch (gch->pDest->iType) {
	case GC_USER_MESSAGE:
		if (IsOnline() && mir_tstrlen(gch->ptszText) > 0) {
			TCHAR *buf = NEWTSTR_ALLOCA(gch->ptszText);
			rtrimt(buf);
			UnEscapeChatTags(buf);
			AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, AsyncHttpRequest::rpHigh)
				<< INT_PARAM("chat_id", cc->m_chatid) 
				<< CHAR_PARAM("message", T2Utf(buf))
				<< VER_API;
			pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			Push(pReq);
		}
		break;
	case GC_USER_PRIVMESS:
		{
			MCONTACT hContact = FindUser(_ttoi(gch->ptszUID));
			if (hContact == NULL) {
				hContact = FindUser(_ttoi(gch->ptszUID), true);
				db_set_b(hContact, "CList", "Hidden", 1);
				db_set_b(hContact, "CList", "NotOnList", 1);
				db_set_dw(hContact, "Ignore", "Mask1", 0);
				RetrieveUserInfo(_ttoi(gch->ptszUID));
			}
			CallService(MS_MSG_SENDMESSAGET, hContact, 0);
		}
		break;

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
		JSONNode jnRoot;
		CheckJsonResponse(pReq, reply, jnRoot);
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
	if (!IsOnline())
		return;

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
			code.Format("API.messages.removeChatUser({\"chat_id\":%d, \"user_id\":%d});"
				"var Hist = API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
				"var countMsg = Hist.count;var itemsMsg = Hist.items@.id; "
				"while (countMsg > 0) { API.messages.delete({\"message_ids\":itemsMsg});"
				"Hist=API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
				"countMsg = Hist.count;itemsMsg = Hist.items@.id;}; return 1;", cc->m_chatid, m_myUserId, cc->m_chatid, cc->m_chatid);
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnChatDestroy)
				<< CHAR_PARAM("code", code)
				<< VER_API)->pUserInfo = cc;
		}
		break;
	}
}

INT_PTR __cdecl CVkProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnJoinChat");
	if (!IsOnline())
		return 1;

	if (getBool(hContact, "kicked", false))
		return 1;
	
	int chat_id = getDword(hContact, "vk_chat_id", -1);
	
	if (chat_id == -1)
		return 1;

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, AsyncHttpRequest::rpHigh)
		<< INT_PARAM("chat_id", chat_id)
		<< TCHAR_PARAM("message", TranslateT("I'm back"))
		<< VER_API;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	Push(pReq);
	db_unset(hContact, m_szModuleName, "off");
	return 0;
}

INT_PTR __cdecl CVkProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnLeaveChat");
	if (!IsOnline())
		return 1;

	ptrT tszChatID(getTStringA(hContact, "ChatRoomID"));
	if (tszChatID == NULL)
		return 1;

	CVkChatInfo *cc = GetChatById(tszChatID);
	if (cc == NULL)
		return 1;
	
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnChatLeave)
		<< INT_PARAM("chat_id", cc->m_chatid)
		<< INT_PARAM("user_id", m_myUserId)
		<< VER_API)->pUserInfo = cc;

	return 0;
}

void CVkProto::LeaveChat(int chat_id, bool close_window, bool delete_chat)
{
	debugLogA("CVkProto::LeaveChat");
	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&chat_id);
	if (cc == NULL)
		return;

	GCDEST gcd = { m_szModuleName, cc->m_tszId, GC_EVENT_QUIT };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, close_window? SESSION_TERMINATE:SESSION_OFFLINE, (LPARAM)&gce);
	if (delete_chat)
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)cc->m_hContact, 0);
	else
		setByte(cc->m_hContact, "off", (int)true);
	m_chats.remove(cc);
}

void CVkProto::KickFromChat(int chat_id, int user_id, const JSONNode &jnMsg)
{
	debugLogA("CVkProto::KickFromChat (%d)", user_id);

	MCONTACT chatContact = FindChat(chat_id);
	if (chatContact)
		if (getBool(chatContact, "off", false))
			return;

	if (user_id == m_myUserId)
		LeaveChat(chat_id);
	
	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&chat_id);
	if (cc == NULL)
		return;

	MCONTACT hContact = FindUser(user_id, false);
	CMString msg(jnMsg["body"].as_mstring());
	if (msg.IsEmpty()) {
		msg = TranslateT("You've been kicked by ");
		if (hContact != NULL)
			msg += ptrT(db_get_tsa(hContact, m_szModuleName, "Nick"));
		else
			msg += TranslateT("(Unknown contact)");
	}
	else 
		AppendChatMessage(chat_id, jnMsg, false);

	MsgPopup(hContact, msg, TranslateT("Chat"));
	setByte(cc->m_hContact, "kicked", 1);
	LeaveChat(chat_id);
}

void CVkProto::OnChatLeave(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatLeave %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;
	
	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_chatid);
}

INT_PTR __cdecl CVkProto::SvcDestroyKickChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcDestroyKickChat");
	if (!IsOnline())
		return 1;

	if (!getBool(hContact, "off", false))
		return 1;
		
	int chat_id = getDword(hContact, "vk_chat_id", -1);
	if (chat_id == -1) 
		return 1;
	
	CMStringA code;
	code.Format("var Hist = API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
		"var countMsg = Hist.count;var itemsMsg = Hist.items@.id; "
		"while (countMsg > 0) { API.messages.delete({\"message_ids\":itemsMsg});"
		"Hist=API.messages.getHistory({\"chat_id\":%d, \"count\":200});"
		"countMsg = Hist.count;itemsMsg = Hist.items@.id;}; return 1;", chat_id, chat_id);
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("code", code)
		<< VER_API);

	CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);

	return 0;
}

void CVkProto::OnChatDestroy(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatDestroy %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_chatid, true, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::NickMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	CVkChatUser* cu = cc->GetUserById(gch->ptszUID);
	MCONTACT hContact;
	if (cu == NULL)
		return;

	char szUid[20], szChatId[20];
	_itoa(cu->m_uid, szUid, 10);
	_itoa(cc->m_chatid, szChatId, 10);

	switch (gch->dwData) {
	case IDM_INFO:
		hContact = FindUser(cu->m_uid);
		if (hContact == NULL) {
			hContact = FindUser(cu->m_uid, true);
			db_set_b(hContact, "CList", "Hidden", 1);
			db_set_b(hContact, "CList", "NotOnList", 1);
			db_set_dw(hContact, "Ignore", "Mask1", 0);
		}
		CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		break;

	case IDM_VISIT_PROFILE:
		hContact = FindUser(cu->m_uid);
		if (hContact == NULL) {
			CMString tszUrl(FORMAT, _T("http://vk.com/id%d"), cu->m_uid);
			CallService(MS_UTILS_OPENURL, (WPARAM)OUF_TCHAR, (LPARAM)tszUrl.GetBuffer());
		} else 
			SvcVisitProfile(hContact, 0);
		break;
		
	case IDM_KICK:
		if (!IsOnline())
			return;

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
	{ LPGENT("Visit profile"), IDM_VISIT_PROFILE, MENU_ITEM },
	{ LPGENT("&Kick"), IDM_KICK, MENU_ITEM }
};

int CVkProto::OnGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
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
		if (mir_strcmp(proto, ppro->m_szModuleName) || ppro->isChatRoom(hContact))
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
				GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
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
	if (!IsOnline())
		return 1;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GC_CREATE), NULL, GcCreateDlgProc, (LPARAM)this);
	return 0;
}

void CVkProto::CreateNewChat(LPCSTR uids, LPCTSTR ptszTitle)
{
	if (!IsOnline())
		return;
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

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	int chat_id = jnResponse.as_int();
	if (chat_id != 0)
		AppendChat(chat_id, nullNode);
}