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

UINT_PTR CVkProto::m_timer;

char szBlankUrl[] = "http://api.vk.com/blank.html";

void CVkProto::ShutdownSession()
{
	if (m_hWorkerThread) {
		m_bTerminated = true;
		SetEvent(m_evRequestsQueue);
	}

	OnLoggedOut();
}

void CVkProto::ConnectionFailed(int iReason)
{
	delSetting("AccessToken");

	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, iReason);

	ShutdownSession();
}

static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	for (int i=0; i < vk_Instances.getCount(); i++)
		vk_Instances[i]->SetServerStatus(vk_Instances[i]->m_iStatus);
}

static void CALLBACK VKSetTimer(void *pObject)
{
	CVkProto::m_timer = SetTimer(NULL, 0, 60000, TimerProc);
}

static void CALLBACK VKUnsetTimer(void *pObject)
{
	CVkProto *ppro = (CVkProto*)pObject;
	if (CVkProto::m_timer)
		KillTimer(NULL, CVkProto::m_timer);
	CVkProto::m_timer = 0;
}

void CVkProto::OnLoggedIn()
{
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);

	// initialize online timer
	if (!m_timer)
		CallFunctionAsync(VKSetTimer, this);
}

void CVkProto::OnLoggedOut()
{
	m_bOnline = false;

	if (m_pollingConn)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConn, 0);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	bool bOnline = false;
	for (int i=0; i < vk_Instances.getCount(); i++)
		bOnline = bOnline&&vk_Instances[i]->IsOnline();
	if(!bOnline)
		CallFunctionAsync(VKUnsetTimer, this);
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	m_chats.destroy();
}

void CVkProto::SetServerStatus(int iNewStatus)
{
	if (!IsOnline() || iNewStatus < ID_STATUS_OFFLINE)
		return;

	int iOldStatus = m_iStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_iStatus = ID_STATUS_OFFLINE; 
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth)
			<< VER_API);
	}
	else if (iNewStatus != ID_STATUS_INVISIBLE) {
		m_iStatus = ID_STATUS_ONLINE; 
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOnline.json", true, &CVkProto::OnReceiveSmth)
			<< VER_API);
	}
	else m_iStatus = ID_STATUS_INVISIBLE; 

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

/////////////////////////////////////////////////////////////////////////////////////////

static char VK_TOKEN_BEG[] = "access_token=";

void CVkProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	GrabCookies(reply);

	if (reply->resultCode == 302) { // manual redirect
		LPCSTR pszLocation = findHeader(reply, "Location");
		if (pszLocation) {
			if (!_strnicmp(pszLocation, szBlankUrl, sizeof(szBlankUrl)-1)) {
				m_szAccessToken = NULL;
				LPCSTR p = strstr(pszLocation, VK_TOKEN_BEG);
				if (p) {
					p += sizeof(VK_TOKEN_BEG)-1;
					for (LPCSTR q = p+1; *q; q++) {
						if (*q == '&' || *q == '=' || *q == '\"') {
							m_szAccessToken = mir_strndup(p, q-p);
							break;
						}
					}
					if (m_szAccessToken == NULL)
						m_szAccessToken = mir_strdup(p);
					setString("AccessToken", m_szAccessToken);
					RetrieveMyInfo();
				}
				else {
					delSetting("AccessToken");
					ConnectionFailed(LOGINERR_NOSERVER);
				}
			}
			else {
				AsyncHttpRequest *pReq = new AsyncHttpRequest();
				pReq->requestType = REQUEST_GET;
				pReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
				pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
				pReq->AddHeader("Referer", m_prevUrl);
				pReq->Redirect(reply);
				if (pReq->m_szUrl) {
					ApplyCookies(pReq);
					m_prevUrl = pReq->m_szUrl;
				}
				Push(pReq);
			}
		}
		else ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	if (reply->resultCode != 200) { // something went wrong
LBL_NoForm:
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	if (strstr(reply->pData, "service_msg_warning")) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	// Application requests access to user's account
	if (!strstr(reply->pData, "form method=\"post\""))
		goto LBL_NoForm;

	CMStringA szAction, szBody;
	bool bSuccess = AutoFillForm(reply->pData, szAction, szBody);
	if (!bSuccess || szAction.IsEmpty() || szBody.IsEmpty()) {
		if (m_prevError)
			goto LBL_NoForm;
		m_prevError = true;
	}

	pReq = new AsyncHttpRequest();
	pReq->requestType = REQUEST_POST;
	pReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	pReq->pData = mir_strdup(szBody);
	pReq->dataLength = szBody.GetLength();
	pReq->m_szUrl = szAction; m_prevUrl = pReq->m_szUrl;
	pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->Redirect(reply);
	ApplyCookies(pReq);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveMyInfo()
{
	// Old method
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnReceiveMyInfo)
		<< VER_API);
}

void CVkProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveMyInfo %d", reply->resultCode);
	if (reply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;
	
	m_myUserId = json_as_int(json_get(json_at(pResponse, 0), "id"));
	setDword("ID", m_myUserId);
		
	OnLoggedIn();
	RetrieveUserInfo(m_myUserId);
	RetrieveFriends();
	RetrieveUnreadMessages();
	RetrievePollingInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUserInfo(LONG userID)
{
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnReceiveUserInfo)
		<< INT_PARAM("user_ids", userID) 
		<< CHAR_PARAM("fields", "id, first_name, last_name, photo_100, sex, country, timezone, contacts, online")
		<< CHAR_PARAM("name_case", "nom")
		<< VER_API);
}

void CVkProto::OnReceiveUserInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUserInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	for (size_t i=0; ; i++) {
		JSONNODE *pRecord = json_at(pResponse, i);
		if (pRecord == NULL) 
			break;

		LONG userid = json_as_int( json_get(pRecord, "id"));
		if (userid == 0)
			return;

		MCONTACT hContact;
		if (userid == m_myUserId)
			hContact = NULL;
		else if ((hContact = FindUser(userid, false)) == NULL)
			return;

		CMString tszNick;
		ptrT szValue( json_as_string( json_get(pRecord, "first_name")));
		if (szValue) {
			setTString(hContact, "FirstName", szValue);
			tszNick.Append(szValue);
			tszNick.AppendChar(' ');
		}

		if (szValue = json_as_string( json_get(pRecord, "last_name"))) {
			setTString(hContact, "LastName", szValue);
			tszNick.Append(szValue);
		}

		if (!tszNick.IsEmpty())
			setTString(hContact, "Nick", tszNick);
	
		setByte(hContact, "Gender", json_as_int( json_get(pRecord, "sex")) == 2 ? 'M' : 'F');
	
		if (szValue = json_as_string( json_get(pRecord, "bdate"))) {
			int d, m, y;
			if ( _stscanf(szValue, _T("%d.%d.%d"), &d, &m, &y) == 3) {
				setByte(hContact, "BirthDay", d);
				setByte(hContact, "BirthMonth", m);
				setWord(hContact, "BirthYear", y);
			}
		}

		szValue = json_as_string( json_get(pRecord, "photo_100"));
		SetAvatarUrl(hContact, szValue);
		
		setWord(hContact, "Status", (json_as_int(json_get(pRecord, "online")) == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveFriends()
{
	debugLogA("CVkProto::RetrieveFriends");
	// Old method
	// fields 'country' and 'city' replaced to objects
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.get.json", true, &CVkProto::OnReceiveFriends)
		<< INT_PARAM("count", 1000) 
		<< CHAR_PARAM("fields", "id, first_name, last_name, photo_100, sex, country, timezone, contacts, online")
		<<VER_API);
}

void CVkProto::OnReceiveFriends(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveFriends %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot), *pInfo;
	if (pResponse == NULL)
		return;

	bool bCleanContacts = getByte("AutoClean", 0) != 0;
	LIST<void> arContacts(10, PtrKeySortT);
	if (bCleanContacts)
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
			if (!isChatRoom(hContact))
				arContacts.insert((HANDLE)hContact);

	for (int i = 0; (pInfo = json_at(pResponse, i)) != NULL; i++) {
		ptrT szValue(json_as_string(json_get(pInfo, "id")));
		if (szValue == NULL)
			continue;

		CMString tszNick;
		MCONTACT hContact = FindUser(_ttoi(szValue), true);
		arContacts.remove((HANDLE)hContact);
		szValue = json_as_string(json_get(pInfo, "first_name"));
		if (szValue) {
			setTString(hContact, "FirstName", szValue);

			tszNick.Append(szValue);
			tszNick.AppendChar(' ');
		}

		if (szValue = json_as_string(json_get(pInfo, "last_name"))) {
			setTString(hContact, "LastName", szValue);
			tszNick.Append(szValue);
		}

		if (!tszNick.IsEmpty())
			setTString(hContact, "Nick", tszNick);

		szValue = json_as_string(json_get(pInfo, "photo_100"));
		SetAvatarUrl(hContact, szValue);

		setWord(hContact, "Status", (json_as_int(json_get(pInfo, "online")) == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE);

		int iValue = json_as_int(json_get(pInfo, "sex"));
		if (iValue)
			setByte(hContact, "Gender", (iValue == 2) ? 'M' : 'F');

		if ((iValue = json_as_int(json_get(pInfo, "timezone"))) != 0)
			setByte(hContact, "Timezone", iValue * -2);

		szValue = json_as_string(json_get(pInfo, "mobile_phone"));
		if (szValue && *szValue)
			setTString(hContact, "Cellular", szValue);
		szValue = json_as_string(json_get(pInfo, "home_phone"));
		if (szValue && *szValue)
			setTString(hContact, "Phone", szValue);
	}

	if (bCleanContacts)
		for (int i = 0; i < arContacts.getCount(); i++)
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)arContacts[i], 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::MarkMessagesRead(const CMStringA &mids)
{
	if (mids.IsEmpty())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("message_ids", mids) << VER_API);
}

void CVkProto::MarkMessagesRead(const MCONTACT hContact)
{
	LONG userID = getDword(hContact, "ID", -1);
	if (-1==userID)
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("peer_id", userID) << VER_API);
}

void CVkProto::RetrieveMessagesByIds(const CMStringA &mids)
{
	if (mids.IsEmpty())
		return;
	// Old method
	//  change 'message' object and mids => message_ids
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getById.json", true, &CVkProto::OnReceiveMessages)
		<< CHAR_PARAM("mids", mids));
}

void CVkProto::RetrieveUnreadMessages()
{
	debugLogA("CVkProto::RetrieveMessages");
	// Old method
	//  change 'message' and dialogs object and filters=1 not suported
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveMessages)
		<< CHAR_PARAM("code", "return { \"msgs\":API.messages.get({\"filters\":1}), \"dlgs\":API.messages.getDialogs() };"));
}

static char* szImageTypes[] = { "src_xxxbig", "src_xxbig", "src_xbig", "src_big", "src", "src_small" };

void CVkProto::OnReceiveMessages(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveMessages %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	JSONNODE *pDlgs = json_as_array(json_get(pResponse, "dlgs"));
	if (pDlgs != NULL) {
		int numDialogs = json_as_int(json_at(pDlgs, 0));
		for (int i = 1; i <= numDialogs; i++) {
			JSONNODE *pDlg = json_at(pDlgs, i);
			if (pDlg == NULL)
				continue;

			int chatid = json_as_int(json_get(pDlg, "chat_id"));
			if (chatid != 0){
				if (m_chats.find((CVkChatInfo*)&chatid) == NULL) 
					AppendChat(chatid, pDlg);	
			}else if (m_bAutoSyncHistory){
				int mid = json_as_int(json_get(pDlg, "mid"));
				int uid = json_as_int(json_get(pDlg, "uid"));
				MCONTACT hContact = FindUser(uid, true);	
				debugLogA("CVkProto::GetHistoryDlg %d", mid);
				GetHistoryDlg(hContact, mid);
			}
		}
	}

	CMStringA mids, lmids;
	bool bDirectArray = false;

	JSONNODE *pMsgs = json_as_array(json_get(pResponse, "msgs"));
	if (pMsgs == NULL) {
		pMsgs = pResponse;
		bDirectArray = true;
	}

	int numMessages = json_as_int(json_at(pMsgs, 0));
	for (int i = 1; i <= numMessages; i++) {
		JSONNODE *pMsg = json_at(pMsgs, i);
		if (pMsg == NULL)
			continue;

		UINT mid = json_as_int(json_get(pMsg, "mid"));

		char szMid[40];
		_itoa(mid, szMid, 10);
		if (!mids.IsEmpty())
			mids.AppendChar(',');
		mids.Append(szMid);

		int chat_id = json_as_int(json_get(pMsg, "chat_id"));
		if (chat_id != 0) {
			AppendChatMessage(chat_id, pMsg, false);
			continue;
		}

		// VK documentation lies: even if you specified preview_length=0, 
		// long messages get cut out. So we need to retrieve them from scratch
		ptrT ptszBody(json_as_string(json_get(pMsg, "body")));
		if (!bDirectArray && _tcslen(ptszBody) > 1000) {
			if (!lmids.IsEmpty())
				lmids.AppendChar(',');
			lmids.Append(szMid);
			continue;
		}

		int datetime = json_as_int(json_get(pMsg, "date"));
		int isOut = json_as_int(json_get(pMsg, "out"));
		int uid = json_as_int(json_get(pMsg, "uid"));
		int isRead = json_as_int(json_get(pMsg, "read_state"));

		JSONNODE *pAttachments = json_get(pMsg, "attachments");
		if (pAttachments != NULL)
			ptszBody = mir_tstrdup(CMString(ptszBody) + GetAttachmentDescr(pAttachments));

		MCONTACT hContact = FindUser(uid, true);

		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_TCHAR;
		if (isRead&&!m_bMesAsUnread)
			recv.flags |= PREF_CREATEREAD;
		if (isOut)
			recv.flags |= PREF_SENT;
		recv.timestamp = datetime;
		
		CMStringW szBody = ptszBody;
		MyHtmlDecode(szBody);
		recv.tszMessage = szBody.GetBuffer();
		
		recv.lParam = isOut;
		recv.pCustomData = szMid;
		recv.cbCustomDataSize = (int)strlen(szMid);
		Sleep(100);
		if (!CheckMid(mid)){
			ProtoChainRecvMsg(hContact, &recv);
			if (mid>getDword(hContact, "lastmsgid", -1))
				setDword(hContact, "lastmsgid", mid);
		}
	}

	if (!m_bMarkReadOnReply)
		MarkMessagesRead(mids);
	RetrieveMessagesByIds(lmids);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GetHistoryDlg(MCONTACT hContact, int iLastMsg)
{
	int lastmsgid = getDword(hContact, "lastmsgid", -1);
	if (-1==lastmsgid){
		setDword(hContact, "lastmsgid", iLastMsg);
		return;
	}
	int maxOffset = iLastMsg - lastmsgid;
	setDword(hContact, "new_lastmsgid", iLastMsg);
	GetHistoryDlgMessages (hContact, 0, maxOffset, -1);
}

void CVkProto::GetHistoryDlgMessages(MCONTACT hContact, int iOffset, int iMaxCount, int lastcount)
{
	LONG userID = getDword(hContact, "ID", -1);
	if (-1==userID)
		return;

	if ((0==lastcount)||(iMaxCount<1)){
		setDword(hContact, "lastmsgid", getDword(hContact, "new_lastmsgid", -1));
		db_unset(hContact, m_szModuleName, "new_lastmsgid");
		db_unset(hContact, m_szModuleName, "ImportHistory");
		return;
	}
	
	int iReqCount = iMaxCount>MAXHISTORYMIDSPERONE?MAXHISTORYMIDSPERONE:iMaxCount;
	
	debugLogA("CVkProto::GetHistoryDlgMessages %d %d %d", userID, iOffset, iReqCount);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getHistory.json", true, &CVkProto::OnReceiveHistoryMessages)
		<< INT_PARAM("offset", iOffset) 
		<< INT_PARAM("count", iReqCount) 
		<< INT_PARAM("user_id", userID) 
		<< VER_API;

	pReq->pUserInfo = new CVkSendMsgParam(hContact, iOffset);
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
		if (his&&(-1==new_lastmsgid)){
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
		
		CMStringW szBody = ptszBody;
		MyHtmlDecode(szBody);
		recv.tszMessage = szBody.GetBuffer();
		recv.lParam = isOut;
		recv.pCustomData = szMid;
		recv.cbCustomDataSize = (int)strlen(szMid);
		ProtoChainRecvMsg(hContact, &recv);
	}
	
	int inewCount = mid - getDword(param->hContact, "lastmsgid", -1);
	inewCount = (0>mid)?0:inewCount;
	GetHistoryDlgMessages(param->hContact, param->iMsgID+i, inewCount, i);
	delete param;
}

void CVkProto::RetrievePollingInfo()
{
	debugLogA("CVkProto::RetrievePollingInfo");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getLongPollServer.json", true, &CVkProto::OnReceivePollingInfo)
		<< VER_API);
}

void CVkProto::OnReceivePollingInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceivePollingInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	m_pollingTs = mir_t2a(ptrT(json_as_string(json_get(pResponse, "ts"))));
	m_pollingKey = mir_t2a(ptrT(json_as_string(json_get(pResponse, "key"))));
	m_pollingServer = mir_t2a(ptrT(json_as_string(json_get(pResponse, "server"))));
	if (!m_hPollingThread && m_pollingTs != NULL && m_pollingKey != NULL && m_pollingServer != NULL)
		m_hPollingThread = ForkThreadEx(&CVkProto::PollingThread, NULL, NULL);
}

INT_PTR __cdecl CVkProto::SvcGetAllServerHistory(WPARAM wParam, LPARAM)
{
	LPCWSTR str = TranslateT("Are you sure to reload all messages from vk.com?\nLocal contact history will be deleted and reloaded from the server.\nIt may take a long time.\nDo you want to continue?");
	if (IDNO==MessageBox(NULL, str, TranslateT("Attention!"), MB_ICONWARNING|MB_YESNO))
		return 0;
	
	LONG userID = getDword((MCONTACT)wParam, "ID", -1); 
	if (-1==userID)
		return 0;
	
	setByte((MCONTACT)wParam, "ImportHistory", 1);
	setDword((MCONTACT)wParam, "lastmsgid", 0);
	
	while (HANDLE hd = db_event_first((MCONTACT)wParam))
		db_event_delete((MCONTACT)wParam, hd);
	
	debugLogA("CVkProto::SvcGetAllServerHistory");
	GetHistoryDlgMessages((MCONTACT)wParam, 0, INT_MAX, -1);
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::PollUpdates(JSONNODE *pUpdates)
{
	debugLogA("CVkProto::PollUpdates");

	CMStringA mids;
	int msgid, uid, flags;
	MCONTACT hContact;

	JSONNODE *pChild;
	for (int i = 0; (pChild = json_at(pUpdates, i)) != NULL; i++) {
		switch (json_as_int(json_at(pChild, 0))) {
		case VKPOLL_MSG_ADDED: // new message
			msgid = json_as_int(json_at(pChild, 1));

			// skip outgoing messages sent from a client
			flags = json_as_int(json_at(pChild, 2));
			if ((flags & VKFLAG_MSGOUTBOX) && !(flags & VKFLAG_MSGCHAT))
				if (CheckMid(msgid))
					break;

			if (!mids.IsEmpty())
				mids.AppendChar(',');
			mids.AppendFormat("%d", msgid);
			break;

		case VKPOLL_USR_ONLINE:
			uid = -json_as_int(json_at(pChild, 1));
			if ((hContact = FindUser(uid)) != NULL)
				setWord(hContact, "Status", ID_STATUS_ONLINE);
			break;

		case VKPOLL_USR_OFFLINE:
			uid = -json_as_int(json_at(pChild, 1));
			if ((hContact = FindUser(uid)) != NULL)
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
			break;

		case VKPOLL_USR_UTN:
			uid = json_as_int(json_at(pChild, 1));
			if ((hContact = FindUser(uid)) != NULL)
				CallService(MS_PROTO_CONTACTISTYPING, hContact, 5);
			break;

		case VKPOLL_CHAT_CHANGED:
			int chat_id = json_as_int(json_at(pChild, 1));
			CVkChatInfo *cc = m_chats.find((CVkChatInfo*)&chat_id);
			if (cc)
				RetrieveChatInfo(cc);
			break;
		}
	}

	RetrieveMessagesByIds(mids);
}

int CVkProto::PollServer()
{
	debugLogA("CVkProto::PollServer");

	//need rework
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = NEWSTR_ALLOCA(CMStringA().Format("http://%s?act=a_check&key=%s&ts=%s&wait=25&access_token=%s", m_pollingServer, m_pollingKey, m_pollingTs, m_szAccessToken));
	req.flags = VK_NODUMPHEADERS | NLHRF_PERSISTENT;
	req.timeout = 30000;
	req.nlc = m_pollingConn;

	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (reply == NULL) {
		m_pollingConn = NULL;
		return 0;
	}

	int retVal = 0;
	if (reply->resultCode == 200) {
		JSONROOT pRoot(reply->pData);
		JSONNODE *pFailed = json_get(pRoot, "failed");
		if (pFailed != NULL && json_as_int(pFailed) == 2) {
			RetrievePollingInfo();
			retVal = -1;
			debugLogA("Polling key expired, restarting polling thread");
		}
		else if (CheckJsonResult(NULL, reply, pRoot)) {
			m_pollingTs = mir_t2a(ptrT(json_as_string(json_get(pRoot, "ts"))));
			JSONNODE *pUpdates = json_get(pRoot, "updates");
			if (pUpdates != NULL)
				PollUpdates(pUpdates);
			retVal = 1;
		}
	}

	m_pollingConn = reply->nlc;

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
	return retVal;
}

void CVkProto::PollingThread(void*)
{
	debugLogA("CVkProto::PollingThread: entering");

	while (!m_bTerminated)
		if (PollServer() == -1)
			break;

	m_hPollingThread = NULL;
	m_pollingConn = NULL;
	debugLogA("CVkProto::PollingThread: leaving");
}

CMString CVkProto::GetAttachmentDescr(JSONNODE *pAttachments)
{
	// need rework
	
	CMString res;
	res.AppendChar('\n');
	res += TranslateT("Attachments:");
	res.AppendChar('\n');
	JSONNODE *pAttach;
	for (int k = 0; (pAttach = json_at(pAttachments, k)) != NULL; k++) {
		res.AppendChar('\t');
		ptrT ptszType(json_as_string(json_get(pAttach, "type")));
		if (!lstrcmp(ptszType, _T("photo"))) {
			JSONNODE *pPhoto = json_get(pAttach, "photo");
			if (pPhoto == NULL) continue;

			ptrT ptszLink;
			for (int i = 0; i < SIZEOF(szImageTypes); i++) {
				JSONNODE *n = json_get(pPhoto, szImageTypes[i]);
				if (n != NULL) {
					ptszLink = json_as_string(n);
					break;
				}
			}

			int iWidth = json_as_int(json_get(pPhoto, "width"));
			int iHeight = json_as_int(json_get(pPhoto, "height"));
			res.AppendFormat(_T("%s: %s (%dx%d)"), TranslateT("Photo"), ptszLink, iWidth, iHeight);
		}
		else if (!lstrcmp(ptszType, _T("audio"))) {
			JSONNODE *pAudio = json_get(pAttach, "audio");
			if (pAudio == NULL) continue;

			int  aid = json_as_int(json_get(pAudio, "aid"));
			int  ownerID = json_as_int(json_get(pAudio, "owner_id"));
			ptrT ptszArtist(json_as_string(json_get(pAudio, "artist")));
			ptrT ptszTitle(json_as_string(json_get(pAudio, "title")));
			res.AppendFormat(_T("%s: (%s - %s) - http://vk.com/audio%d_%d"),
				TranslateT("Audio"), ptszArtist, ptszTitle, ownerID, aid);
		}
		else if (!lstrcmp(ptszType, _T("video"))) {
			JSONNODE *pVideo = json_get(pAttach, "video");
			if (pVideo == NULL) continue;

			ptrT ptszTitle(json_as_string(json_get(pVideo, "title")));
			int  vid = json_as_int(json_get(pVideo, "vid"));
			int  ownerID = json_as_int(json_get(pVideo, "owner_id"));
			res.AppendFormat(_T("%s: %s - http://vk.com/video%d_%d"),
				TranslateT("Video"), ptszTitle, ownerID, vid);
		}
		else if (!lstrcmp(ptszType, _T("doc"))) {
			JSONNODE *pDoc = json_get(pAttach, "doc");
			if (pDoc == NULL) continue;

			ptrT ptszTitle(json_as_string(json_get(pDoc, "title")));
			ptrT ptszUrl(json_as_string(json_get(pDoc, "url")));
			res.AppendFormat(_T("%s: (%s) - %s"),
				TranslateT("Document"), ptszTitle, ptszUrl);
		}
		else if (!lstrcmp(ptszType, _T("wall"))) {
			JSONNODE *pWall = json_get(pAttach, "wall");
			if (pWall == NULL) continue;
			
			ptrT ptszText(json_as_string(json_get(pWall, "text")));
			int  id = json_as_int(json_get(pWall, "id"));
			int  fromID = json_as_int(json_get(pWall, "from_id"));
			res.AppendFormat(_T("%s: %s - http://vk.com/wall%d_%d"), TranslateT("Wall post"), ptszText, fromID, id);
		}
		else res.AppendFormat(TranslateT("Unsupported or unknown attachment type: %s"), ptszType);

		res.AppendChar('\n');
	}

	return res;
}
