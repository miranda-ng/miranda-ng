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

#ifndef _SKYPE_REQUEST_LOGINMS_H_
#define _SKYPE_REQUEST_LOGINMS_H_

class LoginMSRequest : public HttpRequest
{
public:
	LoginMSRequest() :
		HttpRequest(REQUEST_GET, "login.skype.com/login/oauth/microsoft")
	{
		flags |= NLHRF_REDIRECT;

		Url
			<< INT_VALUE  ("client_id", 578134)
			<< CHAR_VALUE ("redirect_uri", ptrA(mir_urlEncode("https://web.skype.com")));
	}
	LoginMSRequest(const char *login, const char *password, const char *cookies_str, const char *ppft) :
		HttpRequest(REQUEST_POST, "login.live.com/ppsecure/post.srf")
	{
		Url
			<< CHAR_VALUE ("wa", "wsignin1.0")
			<< CHAR_VALUE ("wreply", "https%3A%2F%2Fsecure.skype.com%2Flogin%2Foauth%2Fproxy%3Fclient_id%3D578134%26redirect_uri%3Dhttps%253A%252F%252Fweb.skype.com");
		
		Headers
			<< CHAR_VALUE ("Content-Type", "application/x-www-form-urlencoded")
			<< CHAR_VALUE ("Cookie", cookies_str);

		Body 
			<< CHAR_VALUE ("login", ptrA(mir_urlEncode(login)))
			<< CHAR_VALUE ("passwd", ptrA(mir_urlEncode(password)))
			<< CHAR_VALUE ("PPFT", ppft);
	}
	LoginMSRequest(const char *t) :
		HttpRequest(REQUEST_POST, "login.skype.com/login/oauth")
	{
		Url
			<< INT_VALUE  ("client_id", 578134)
			<< CHAR_VALUE ("redirect_uri", ptrA(mir_urlEncode("https://web.skype.com")));

		Headers
			<< CHAR_VALUE ("Content-Type", "application/x-www-form-urlencoded");

		Body
			<< CHAR_VALUE ("t", ptrA(mir_urlEncode(t)))
			<< INT_VALUE ("oauthPartner", 999)
			<< INT_VALUE  ("client_id", 578134)
			<< CHAR_VALUE ("redirect_uri", ptrA(mir_urlEncode("https://web.skype.com")));
	}

	LoginMSRequest(const char *url,const char *login, const char *cookies_str, const char *ppft, const char *code) :
		HttpRequest(REQUEST_POST, url)
	{
		Headers
			<< CHAR_VALUE ("Cookie", cookies_str);
		Body 
			<< CHAR_VALUE ("oct", code)
			<< INT_VALUE ("AdTD", 1)
			<< CHAR_VALUE ("login", ptrA(mir_urlEncode(login)))
			<< INT_VALUE ("type", 19)
			<< CHAR_VALUE ("PPFT", ptrA(mir_urlEncode(ppft)));
	}
};

#endif //_SKYPE_REQUEST_LOGINMS_H_
