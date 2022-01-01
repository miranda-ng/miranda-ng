/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

#ifndef _SKYPE_REQUEST_OAUTH_H_
#define _SKYPE_REQUEST_OAUTH_H_

struct OAuthRequest : public AsyncHttpRequest
{
	OAuthRequest() :
		AsyncHttpRequest(REQUEST_GET, HOST_OTHER, "https://login.live.com/login.srf", &CSkypeProto::OnOAuthStart)
	{
		flags |= NLHRF_REDIRECT;

		this << CHAR_PARAM("wa", "wsignin1.0") << CHAR_PARAM("wp", "MBI_SSL")
			<< CHAR_PARAM("wreply", "https://lw.skype.com/login/oauth/proxy?site_name=lw.skype.com")
			<< CHAR_PARAM("cobrandid", "90010");
	}

	OAuthRequest(const char *login, const char *password, const char *cookies, const char *ppft) :
		AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://login.live.com/ppsecure/post.srf", &CSkypeProto::OnOAuthConfirm)
	{
		this << CHAR_PARAM("wa", "wsignin1.0") << CHAR_PARAM("wp", "MBI_SSL")
			<< CHAR_PARAM("wreply", "https://lw.skype.com/login/oauth/proxy?site_name=lw.skype.com")
			<< CHAR_PARAM("cobrandid", "90010");
		m_szUrl.AppendFormat("?%s", m_szParam.c_str());
		m_szParam.Empty();

		AddHeader("Cookie", cookies);

		if (auto *delim = strchr(login, ':'))
			login = delim + 1;

		this << CHAR_PARAM("login", login) << CHAR_PARAM("passwd", password) << CHAR_PARAM("PPFT", ppft);
	}

	OAuthRequest(const char *cookies, const char* ppft, const char* opid) :
		AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://login.live.com/ppsecure/post.srf", &CSkypeProto::OnOAuthAuthorize)
	{
		this << CHAR_PARAM("wa", "wsignin1.0") << CHAR_PARAM("wp", "MBI_SSL")
			<< CHAR_PARAM("wreply", "https://lw.skype.com/login/oauth/proxy?site_name=lw.skype.com")
			<< CHAR_PARAM("cobrandid", "90010")
			<< CHAR_PARAM("id", "293290")
			<< CHAR_PARAM("opid", opid);

		m_szUrl.AppendFormat("?%s", m_szParam.c_str());
		m_szParam.Empty();

		AddHeader("Cookie", cookies);

		this << CHAR_PARAM("type", "28") << CHAR_PARAM("PPFT", ppft);
	}

	OAuthRequest(const char *t) :
		AsyncHttpRequest(REQUEST_POST, HOST_LOGIN, "/login/microsoft", &CSkypeProto::OnOAuthEnd)
	{
		this << CHAR_PARAM ("t", t) << CHAR_PARAM("site_name", "lw.skype.com") << INT_PARAM ("oauthPartner", 999);
	}
};

#endif //_SKYPE_REQUEST_OAUTH_H_
