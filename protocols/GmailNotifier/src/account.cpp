/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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
	hContact(_1)
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
#define GOOGLE_AUTH GOOGLE_OAUTH "/auth?response_type=code&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fgmail.readonly&access_type=offline&prompt=consent&redirect_uri=https%3A%2F%2Foauth.miranda-ng.org%2Fverification&client_id=" GOOGLE_APP_ID

class CRegisterDlg : public CDlgBase
{
	Account *pAcc;
	CCtrlEdit edtCode;
	CCtrlHyperlink m_link;

public:
	CRegisterDlg(Account *_1) :
		CDlgBase(g_plugin, IDD_OAUTH),
		pAcc(_1),
		m_link(this, IDC_OAUTH_AUTHORIZE, GOOGLE_AUTH),
		edtCode(this, IDC_OAUTH_CODE)
	{}

	bool OnApply()
	{
		ptrW szCode(edtCode.GetText());
		if (!mir_wstrlen(szCode))
			return false;

		MHttpRequest request(REQUEST_POST);
		request.m_szUrl = GOOGLE_API_OAUTH "/token";
		request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
		request << CHAR_PARAM("redirect_uri", "https://oauth.miranda-ng.org/verification") << CHAR_PARAM("client_id", GOOGLE_APP_ID)
			<< CHAR_PARAM("client_secret", GOOGLE_CLIENT_SECRET) << CHAR_PARAM("grant_type", "authorization_code") << WCHAR_PARAM("code", szCode);

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
};

void Account::Register()
{
	CRegisterDlg(this).DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////

bool Account::RefreshToken()
{
	if (!szAccessToken.IsEmpty())
		if (g_plugin.getDword(hContact, "ExpiresIn") < time(0))
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
