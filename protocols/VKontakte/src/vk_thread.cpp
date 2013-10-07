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

void CVkProto::OnLoggedIn()
{
	m_bOnline = true;

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	HttpParam param = { "access_token", m_szAccessToken };
	PushAsyncHttpRequest(REQUEST_GET, "/method/getUserInfoEx.json", true, &CVkProto::OnReceiveMyInfo, 1, &param);
}

void CVkProto::OnLoggedOut()
{
	m_hWorkerThread = 0;
	m_bOnline = false;

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern CMStringA loginCookie;

static char VK_TOKEN_BEG[] = "access_token=";

void CVkProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *reply)
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

void CVkProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *reply)
{
	if (reply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONNODE *pRoot = json_parse(reply->pData);
	if ( !CheckJsonResult(pRoot))
		return;

	JSONNODE *pResponse = json_get(pRoot, "response");
	if (pResponse == NULL)
		return;

	for (size_t i = 0; i < json_size(pResponse); i++) {
		JSONNODE *it = json_at(pResponse, i);
		LPCSTR id = json_name(it);
		if ( !_stricmp(id, "user_id"))
			setTString("ID", ptrT( json_as_string(it)));
		else if ( !_stricmp(id, "user_name"))
			setTString("Nick", ptrT( json_as_string(it)));
		else if ( !_stricmp(id, "user_sex"))
			setByte("Gender", json_as_int(it) == 2 ? 'M' : 'F');
		else if ( !_stricmp(id, "user_bdate")) {
			ptrT date( json_as_string(it));
			int d, m, y;
			if ( _tscanf(date, _T("%d.%d.%d"), &d, &m, &y) == 3) {
				setByte("BirthDay", d);
				setByte("BirthMonth", m);
				setByte("BirthYear", y);
			}
		}
		else if ( !_stricmp(id, "user_photo"))
			setTString("Photo", ptrT( json_as_string(it)));
	}
}
