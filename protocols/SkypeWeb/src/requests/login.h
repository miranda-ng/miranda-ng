/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

#ifndef _SKYPE_REQUEST_LOGIN_H_
#define _SKYPE_REQUEST_LOGIN_H_

class LoginOAuthRequest : public HttpRequest
{
public:
	LoginOAuthRequest(const char *username, const char *password) :
		HttpRequest(REQUEST_POST, "api.skype.com/login/skypetoken")
	{
		CMStringA str(::FORMAT, "%s\nskyper\n%s", username, password);

		BYTE digest[16];

		mir_md5_hash((BYTE*)str.GetString(), str.GetLength(), digest);

		char *hash = mir_base64_encode(digest, sizeof(digest));

		Body
			<< CHAR_VALUE("scopes", "client")
			<< CHAR_VALUE("clientVersion", "0/7.4.85.102/259/")
			<< CHAR_VALUE("username", username)
			<< CHAR_VALUE("passwordHash", hash);
	}
};

class LoginOAuthRPSRequest : public HttpRequest
{
public:
	LoginOAuthRPSRequest(const char *token) :
		HttpRequest(REQUEST_POST, "api.skype.com/login/skypetoken")
	{
		Body
			<< CHAR_VALUE("scopes", "client")
			<< CHAR_VALUE("clientVersion", "0/7.4.85.102/259/")
			<< CHAR_VALUE("access_token", token)
			<< INT_VALUE("partner", 999);
	}
};

class LoginMSRequest : public HttpRequest
{
public:
	LoginMSRequest() : HttpRequest(REQUEST_GET, "login.live.com/oauth20_authorize.srf")
	{

		Url
			<< CHAR_VALUE("client_id", "00000000480BC46C")
			<< CHAR_VALUE("scope", "service::skype.com::MBI_SSL")	
			<< CHAR_VALUE("response_type", "token")
			<< CHAR_VALUE("redirect_uri", ptrA(mir_urlEncode("https://login.live.com/oauth20_desktop.srf")));
	}

	LoginMSRequest(const char *ppft, const char* login, const char* password, const char *cookies) : 
		HttpRequest(REQUEST_POST, "https://login.live.com/ppsecure/post.srf")
	{
		Headers 
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded")
			<< CHAR_VALUE("Cookie", cookies);

		Body 
			<< CHAR_VALUE("PPFT", ptrA(mir_urlEncode(ppft)))
			<< CHAR_VALUE("login", ptrA(mir_urlEncode(login)))
			<< CHAR_VALUE("passwd", ptrA(mir_urlEncode(password)));
	}
};

#endif //_SKYPE_REQUEST_LOGIN_H_
