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

#ifndef _SKYPE_REQUEST_LOGIN_H_
#define _SKYPE_REQUEST_LOGIN_H_

struct LoginOAuthRequest : public AsyncHttpRequest
{
	LoginOAuthRequest(CMStringA username, const char *password) :
		AsyncHttpRequest(REQUEST_POST, HOST_API, "/login/skypetoken", &CSkypeProto::OnLoginOAuth)
	{
		username.MakeLower();
		const char *pszLogin = username;
		if (int iOffset = username.Find(':'))
			pszLogin += iOffset + 1;

		CMStringA hashStr(::FORMAT, "%s\nskyper\n%s", pszLogin, password);

		uint8_t digest[16];
		mir_md5_hash((const uint8_t*)hashStr.GetString(), hashStr.GetLength(), digest);

		this << CHAR_PARAM("scopes", "client")
			<< CHAR_PARAM("clientVersion", mir_urlEncode("0/7.4.85.102/259/").c_str())
			<< CHAR_PARAM("username", mir_urlEncode(pszLogin).c_str())
			<< CHAR_PARAM("passwordHash", mir_urlEncode(ptrA(mir_base64_encode(digest, sizeof(digest)))).c_str());
	}
};

#endif //_SKYPE_REQUEST_LOGIN_H_
