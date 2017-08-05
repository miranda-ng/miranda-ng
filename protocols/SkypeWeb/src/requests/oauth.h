/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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

class OAuthRequest : public HttpRequest
{
public:
	OAuthRequest() :
		HttpRequest(REQUEST_GET, "login.live.com/login.srf")
	{
		flags |= NLHRF_REDIRECT;

		Url
			<< CHAR_VALUE("wa", "wsignin1.0")
			<< CHAR_VALUE("wp", "MBI_SSL")
			<< CHAR_VALUE("wreply", "https%3A%2F%2Flw.skype.com%2Flogin%2Foauth%2Fproxy%3Fsite_name%3Dlw.skype.com")
			<< CHAR_VALUE("cobrandid", "90010");
	}

	OAuthRequest(const char *login, const char *password, const char *cookies, const char *ppft) :
		HttpRequest(REQUEST_POST, "login.live.com/ppsecure/post.srf")
	{
		Url
			<< CHAR_VALUE("wa", "wsignin1.0")
			<< CHAR_VALUE("wp", "MBI_SSL")
			<< CHAR_VALUE("wreply", "https%3A%2F%2Flw.skype.com%2Flogin%2Foauth%2Fproxy%3Fsite_name%3Dlw.skype.com")
			<< CHAR_VALUE("cobrandid", "90010");

		Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded")
			<< CHAR_VALUE("Cookie", cookies);

		Body
			<< CHAR_VALUE("login", ptrA(mir_urlEncode(login)))
			<< CHAR_VALUE("passwd", ptrA(mir_urlEncode(password)))
			<< CHAR_VALUE("PPFT", ppft);
	}

	OAuthRequest(const char *t) :
		HttpRequest(REQUEST_POST, "login.skype.com/login/microsoft")
	{
		Headers
			<< CHAR_VALUE ("Content-Type", "application/x-www-form-urlencoded");

		Body
			<< CHAR_VALUE ("t", ptrA(mir_urlEncode(t)))
			<< CHAR_VALUE("site_name", "lw.skype.com")
			<< INT_VALUE ("oauthPartner", 999);
	}
};

#endif //_SKYPE_REQUEST_OAUTH_H_
