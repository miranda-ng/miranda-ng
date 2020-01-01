/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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
	LoginOAuthRequest(CMStringA username, const char *password) :
		HttpRequest(REQUEST_POST, "api.skype.com/login/skypetoken")
	{
		username.MakeLower();
		CMStringA hashStr(::FORMAT, "%s\nskyper\n%s", username.c_str(), password);

		BYTE digest[16];
		mir_md5_hash((const BYTE*)hashStr.GetString(), hashStr.GetLength(), digest);

		Body
			<< CHAR_VALUE("scopes", "client")
			<< CHAR_VALUE("clientVersion", mir_urlEncode("0/7.4.85.102/259/").c_str())
			<< CHAR_VALUE("username", mir_urlEncode(username).c_str())
			<< CHAR_VALUE("passwordHash", mir_urlEncode(ptrA(mir_base64_encode(digest, sizeof(digest)))).c_str());
	}
};

#endif //_SKYPE_REQUEST_LOGIN_H_
