/*
Copyright (C) 2012-26 Miranda NG team (https://miranda-ng.org)

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

Account::Account(MCONTACT _1) :
	hContact(_1),
	arEmails(1)
{
	szName = g_plugin.getMStringA(hContact, "name");
	szRefreshToken = g_plugin.getMStringA(hContact, "TokenSecret");
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
}

Account::~Account()
{}

/////////////////////////////////////////////////////////////////////////////////////////

#define GOOGLE_OAUTH "https://accounts.google.com/o/oauth2/v2"

#define GOOGLE_API "https://www.googleapis.com"
#define GOOGLE_API_OAUTH GOOGLE_API "/oauth2/v4"

#include "../../../miranda-private-keys/Gmail/client_secret.h"

const char szOk[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>You can close this window now</h1>";

class CRegisterDlg : public CDlgBase
{
	Account *pAcc;
	CCtrlEdit edtCode;
	CCtrlHyperlink m_link;

	CMStringA m_szUrl, m_szUri;
	NETLIBBIND nlb;

	static void onNewConn(HNETLIBCONN hNewConnection, uint32_t, void *pExtra)
	{
		auto *pDlg = (CRegisterDlg *)pExtra;
		ptrA buf((char*)mir_alloc(10000+1));

		int result = Netlib_Recv(hNewConnection, buf, 10000);
		if (result != SOCKET_ERROR) {
			buf[result] = 0;
			if (char *p = strchr(buf, '\n'))
				*p = 0;
			if (char *p = strrchr(buf, ' '))
				*p = 0;

			CMStringA szUrl(buf);
			int idx = szUrl.Find('?');
			if (idx != -1)
				szUrl.Delete(0, idx + 1);

			idx = 0;
			while (true) {
				auto arg = szUrl.Tokenize("&", idx);
				if (idx == -1)
					break;

				if (arg.Left(5) == "code=") {
					pDlg->edtCode.SetText(_A2T(arg.Mid(5)));
					SetForegroundWindow(pDlg->GetHwnd());
				}
			}

			Netlib_Send(hNewConnection, szOk, (int)strlen(szOk));
		}

		Netlib_CloseHandle(hNewConnection);
	}

public:
	CRegisterDlg(Account *_1) :
		CDlgBase(g_plugin, IDD_OAUTH),
		pAcc(_1),
		m_link(this, IDC_OAUTH_AUTHORIZE, ""),
		edtCode(this, IDC_OAUTH_CODE)
	{
		memset(&nlb, 0, sizeof(nlb));
		nlb.pfnNewConnection = &CRegisterDlg::onNewConn;
		nlb.pExtra = this;
		Netlib_BindPort(hNetlibUser, &nlb);

		m_szUri.Format("http://127.0.0.1:%d", nlb.wExPort);

		m_szUrl.Append("response_type=code&");
		m_szUrl.Append("scope=https://www.googleapis.com/auth/gmail.readonly&");
		m_szUrl.Append("access_type=offline&");
		m_szUrl.Append("prompt=consent&");
		m_szUrl.AppendFormat("redirect_uri=%s&", m_szUri.c_str());
		m_szUrl.Append("client_id=" GOOGLE_APP_ID);
		m_szUrl.Replace(":", "%3A");
		m_szUrl = GOOGLE_OAUTH "/auth?" + m_szUrl;
		m_link.SetUrl(m_szUrl);
	}

	bool OnApply()
	{
		ptrW szCode(edtCode.GetText());
		if (!mir_wstrlen(szCode))
			return false;

		MHttpRequest request(REQUEST_POST);
		request.m_szUrl = GOOGLE_API_OAUTH "/token";
		request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
		request << CHAR_PARAM("client_id", GOOGLE_APP_ID) << CHAR_PARAM("client_secret", GOOGLE_CLIENT_SECRET) 
			<< CHAR_PARAM("redirect_uri", m_szUri) << CHAR_PARAM("grant_type", "authorization_code") << WCHAR_PARAM("code", szCode);

		NLHR_PTR response(Netlib_HttpTransaction(hNetlibUser, &request));
		if (response == nullptr || response->resultCode != 200)
			return false;

		JSONNode root = JSONNode::parse(response->body);
		if (root.empty())
			return false;

		JSONNode node = root.at("error_description");
		if (!node.isnull())
			return false;

		pAcc->szAccessToken = root["access_token"].as_string().c_str();
		pAcc->szRefreshToken = root["refresh_token"].as_string().c_str();

		g_plugin.setDword(pAcc->hContact, "ExpiresIn", time(0) + root["expires_in"].as_int());
		g_plugin.setString(pAcc->hContact, "TokenSecret", pAcc->szRefreshToken);
		return true;
	}

	void OnDestroy() override
	{
		Netlib_CloseHandle(&nlb);
	}
};

void Account::Register()
{
	CRegisterDlg(this).DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////

bool Account::RefreshToken()
{
	if (!szAccessToken.IsEmpty())
		if (g_plugin.getDword(hContact, "ExpiresIn") > time(0))
			return true;

	if (szRefreshToken.IsEmpty())
		return false;

	MHttpRequest request(REQUEST_POST);
	request.m_szUrl = GOOGLE_API_OAUTH "/token";
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request << CHAR_PARAM("client_id", GOOGLE_APP_ID) << CHAR_PARAM("client_secret", GOOGLE_CLIENT_SECRET)
		<< CHAR_PARAM("grant_type", "refresh_token") << CHAR_PARAM("refresh_token", szRefreshToken);

	NLHR_PTR response(Netlib_HttpTransaction(hNetlibUser, &request));
	if (response == nullptr || response->resultCode != 200)
		return false;

	JSONNode root = JSONNode::parse(response->body);
	if (root.empty())
		return false;

	szAccessToken = root["access_token"].as_string().c_str();
	return !szAccessToken.IsEmpty();
}

void Account::Unregister()
{
	if (szRefreshToken.IsEmpty())
		return;

	MHttpRequest request(REQUEST_POST);
	request.m_szUrl.Format(GOOGLE_OAUTH "/revoke?token=%s", szRefreshToken.c_str());
	NLHR_PTR response(Netlib_HttpTransaction(hNetlibUser, &request));

	szAccessToken.Empty();
	szRefreshToken.Empty();
	g_plugin.delSetting(hContact, "TokenSecret");
}
