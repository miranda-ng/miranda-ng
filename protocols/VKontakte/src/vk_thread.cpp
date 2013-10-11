/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

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

static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR pObject, DWORD)
{
	CVkProto *ppro = (CVkProto*)pObject;
	ppro->SetServerStatus(ppro->m_iStatus);
}

void CVkProto::OnLoggedIn()
{
	m_bOnline = true;

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	// initialize online timer
	SetServerStatus(m_iStatus);
	m_timer = SetTimer(NULL, (UINT_PTR)this, 870000, TimerProc);

	HttpParam param = { "access_token", m_szAccessToken };
	PushAsyncHttpRequest(REQUEST_GET, "/method/getUserInfoEx.json", true, &CVkProto::OnReceiveMyInfo, 1, &param);

	SetAllContactStatuses(ID_STATUS_OFFLINE);
}

void CVkProto::OnLoggedOut()
{
	m_hWorkerThread = 0;
	m_bOnline = false;

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	KillTimer(NULL, m_timer);
}

int CVkProto::SetServerStatus(int iStatus)
{
	if (iStatus != ID_STATUS_OFFLINE && iStatus != ID_STATUS_INVISIBLE) {
		HttpParam param = { "access_token", m_szAccessToken };
		PushAsyncHttpRequest(REQUEST_GET, "/method/account.setOnline.json", true, NULL, 1, &param);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern CMStringA loginCookie;

static char VK_TOKEN_BEG[] = "access_token=";

void CVkProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *reply, void*)
{
	if (reply->resultCode == 302) { // manual redirect
		LPCSTR pszLocation = findHeader(reply, "Location");
		if (pszLocation) {
			if ( !_strnicmp(pszLocation, VK_REDIRECT_URL, sizeof(VK_REDIRECT_URL)-1)) {
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
				}
				else delSetting("AccessToken");

				OnLoggedIn();
			}
			else {
				AsyncHttpRequest *pReq = new AsyncHttpRequest();
				pReq->requestType = REQUEST_GET;
				pReq->flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_REMOVEHOST | NLHRF_SMARTREMOVEHOST;
				pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
				pReq->AddHeader("Referer", m_prevUrl);
				pReq->Redirect(reply);
				if (pReq->szUrl) {
					if ( strstr(pReq->szUrl, "login.vk.com"))
						pReq->AddHeader("Cookie", loginCookie);
					m_prevUrl = pReq->szUrl;
				}
				PushAsyncHttpRequest(pReq);
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

	if ( strstr(reply->pData, "Invalid login or password") ||
		  strstr(reply->pData, ptrA( mir_utf8encodecp("неверный логин или пароль", 1251))))
	{
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	// Application requests access to user's account
	if ( !strstr(reply->pData, "form method=\"post\""))
		goto LBL_NoForm;

	CMStringA szAction;
	CMStringA szBody = AutoFillForm(reply->pData, szAction);
	if (szAction.IsEmpty() || szBody.IsEmpty())
		goto LBL_NoForm;

	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->requestType = REQUEST_POST;
	pReq->flags = NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_REMOVEHOST | NLHRF_SMARTREMOVEHOST;
	pReq->pData = mir_strdup(szBody);
	pReq->dataLength = szBody.GetLength();
	pReq->szUrl = mir_strdup(szAction); m_prevUrl = pReq->szUrl;
	pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->Redirect(reply);
	PushAsyncHttpRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *reply, void*)
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::OnReceiveMyInfo %d", reply->resultCode);
	if (reply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONROOT pRoot(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response");
	if (pResponse == NULL)
		return;

	for (size_t i = 0; i < json_size(pResponse); i++) {
		JSONNODE *it = json_at(pResponse, i);
		LPCSTR id = json_name(it);
		if ( !_stricmp(id, "user_id")) {
			ptrT userid( json_as_string(it));
			m_myUserId = mir_t2a(userid);
			setTString("ID", userid);
		}
		else if ( !_stricmp(id, "user_name"))
			setTString("Nick", ptrT( json_as_string(it)));
		else if ( !_stricmp(id, "user_sex"))
			setByte("Gender", json_as_int(it) == 2 ? 'M' : 'F');
		else if ( !_stricmp(id, "user_bdate")) {
			ptrT date( json_as_string(it));
			int d, m, y;
			if ( _stscanf(date, _T("%d.%d.%d"), &d, &m, &y) == 3) {
				setByte("BirthDay", d);
				setByte("BirthMonth", m);
				setWord("BirthYear", y);
			}
		}
		else if ( !_stricmp(id, "user_photo"))
			setTString("Photo", ptrT( json_as_string(it)));
	}

	RetrieveUserInfo(m_myUserId);
	RetrieveFriends();
	RetrievePollingInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUserInfo(LPCSTR szUserId)
{
	HttpParam params[] = {
		{ "fields", "uid,first_name,last_name,photo,sex,bdate,city,relation" },
		{ "uids", szUserId },
		{ "access_token", m_szAccessToken }
	};
	PushAsyncHttpRequest(REQUEST_GET, "/method/getProfiles.json", true, &CVkProto::OnReceiveUserInfo, SIZEOF(params), params);
}

void CVkProto::OnReceiveUserInfo(NETLIBHTTPREQUEST *reply, void*)
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::OnReceiveUserInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response");
	if (pResponse == NULL)
		return;

	HANDLE hContact;
	for (size_t i = 0; i < json_size(pResponse); i++) {
		JSONNODE *it = json_at(pResponse, i);
		LPCSTR id = json_name(it);
		if ( !_stricmp(id, "user_id")) {
			ptrA userid( _T2A( json_as_string(it)));
			if ( !lstrcmpA(userid, m_myUserId))
				hContact = NULL;
			else if ((hContact = FindUser(userid, false)) == NULL)
				return;
		}
		else if ( !_stricmp(id, "user_name"))
			setTString(hContact, "Nick", ptrT( json_as_string(it)));
		else if ( !_stricmp(id, "user_sex"))
			setByte(hContact, "Gender", json_as_int(it) == 2 ? 'M' : 'F');
		else if ( !_stricmp(id, "user_bdate")) {
			ptrT date( json_as_string(it));
			int d, m, y;
			if ( _stscanf(date, _T("%d.%d.%d"), &d, &m, &y) == 3) {
				setByte(hContact, "BirthDay", d);
				setByte(hContact, "BirthMonth", m);
				setWord(hContact, "BirthYear", y);
			}
		}
		else if ( !_stricmp(id, "user_photo"))
			setTString(hContact, "Photo", ptrT( json_as_string(it)));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveFriends()
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::RetrieveFriends");

	HttpParam params[] = {
		{ "fields", "uid,first_name,last_name,photo,contacts" },
		{ "count", "1000" },
		{ "access_token", m_szAccessToken }
	};
	PushAsyncHttpRequest(REQUEST_GET, "/method/friends.get.json", true, &CVkProto::OnReceiveFriends, SIZEOF(params), params);
}

void CVkProto::OnReceiveFriends(NETLIBHTTPREQUEST *reply, void*)
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::OnReceiveFriends %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response"), *pInfo;
	if (pResponse == NULL)
		return;

	for (int i=0; (pInfo = json_at(pResponse, i)) != NULL; i++) {
		ptrT szValue( json_as_string( json_get(pInfo, "uid")));
		if (szValue == NULL)
			continue;

		HANDLE hContact = FindUser(_T2A(szValue), true);
		szValue = json_as_string( json_get(pInfo, "first_name"));
		if (szValue) setTString(hContact, "FirstName", szValue);

		szValue = json_as_string( json_get(pInfo, "last_name"));
		if (szValue) setTString(hContact, "LastName", szValue);

		szValue = json_as_string( json_get(pInfo, "photo"));
		if (szValue) setTString(hContact, "AvatarUrl", szValue);

		setWord(hContact, "Status", (json_as_int( json_get(pInfo, "online")) == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE); 

		szValue = json_as_string( json_get(pInfo, "mobile_phone"));
		if (szValue && *szValue) setTString(hContact, "Phone0", szValue);
		szValue = json_as_string( json_get(pInfo, "home_phone"));
		if (szValue && *szValue) setTString(hContact, "Phone1", szValue);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrievePollingInfo()
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::RetrievePollingInfo");

	HttpParam param = { "access_token", m_szAccessToken };
	PushAsyncHttpRequest(REQUEST_GET, "/method/messages.getLongPollServer.json", true, &CVkProto::OnReceivePollingInfo, 1, &param);
}

void CVkProto::OnReceivePollingInfo(NETLIBHTTPREQUEST *reply, void*)
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::OnReceivePollingInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response");
	if (pResponse == NULL)
		return;

	m_pollingTs = mir_t2a( ptrT( json_as_string( json_get(pResponse, "ts"))));
	m_pollingKey = mir_t2a( ptrT( json_as_string( json_get(pResponse, "key"))));
	m_pollingServer = mir_t2a( ptrT( json_as_string( json_get(pResponse, "server"))));
	if (m_pollingTs != NULL && m_pollingKey != NULL && m_pollingServer != NULL)
		ForkThread(&CVkProto::PollingThread, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::PollServer()
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::PollServer");

	HttpParam params[] = {
		{ "act", "a_check" },
		{ "key", m_pollingKey },
		{ "ts", m_pollingTs },
		{ "wait", "25" },
		{ "access_token", m_szAccessToken }
	};
	PushAsyncHttpRequest(REQUEST_GET, m_pollingServer, true, &CVkProto::OnReceivePolling, SIZEOF(params), params);
}

void CVkProto::OnReceivePolling(NETLIBHTTPREQUEST *reply, void*)
{
	Netlib_Logf(m_hNetlibUser, "CVkProto::OnReceivePolling %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response");
	if (pResponse != NULL)
		m_pollingTs = mir_t2a( ptrT( json_as_string( json_get(pResponse, "ts"))));
}

void CVkProto::PollingThread(void*)
{
	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.flags = NLOCF_SSL | NLOCF_HTTP | NLOCF_V2;
	nloc.szHost = VK_API_URL;
	nloc.wPort = 443;
	nloc.timeout = 60*1000;
	m_pollingConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
	if (m_pollingConn == NULL)
		return;

	while (!m_bTerminated)
		PollServer();

	CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)m_pollingConn, 0);
	m_pollingConn = NULL;
}
