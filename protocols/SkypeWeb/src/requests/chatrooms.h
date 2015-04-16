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

#ifndef _SKYPE_REQUEST_CHATS_H_
#define _SKYPE_REQUEST_CHATS_H_

class CreateChatroomRequest : public HttpRequest
{
public:
	CreateChatroomRequest(const char *regToken, const LIST<char> &skypenames, const LIST<char> &roles, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/threads", server)
	{
		//{"members":[{"id":"8:user3","role":"User"},{"id":"8:user2","role":"User"},{"id":"8:user1","role":"Admin"}]}

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);

		CMStringA data = "{\"members\":[";
		for (int i = 0; i < skypenames.getCount(); i++)
			data.AppendFormat("{\"id\":\"8:%s\",\"role\":\"%s\"},", skypenames[i], roles[i]);
		data.Truncate(data.GetLength() - 1);
		data.Append("]}");

		Body << VALUE(data);
	}
};

class GetChatInfoRequest : public HttpRequest
{
public:
	GetChatInfoRequest(const char *regToken, const char *chatId, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/threads/19:%s", server, chatId)
	{
		Url << CHAR_VALUE("view", "msnp24Equivalent");

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);
	}
};

class InviteUserToChatRequest : public HttpRequest
{
public:
	InviteUserToChatRequest(const char *regToken, const char *chatId, const char *skypename, const char* role, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/threads/19:%s/members/8:%s", server, chatId, skypename)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);

		CMStringA data;
		data.AppendFormat("{\"role\":\"%s\"}", role);
		Body << VALUE(data);
	}
};

class KickUserRequest : public HttpRequest
{
public:
	KickUserRequest(const char *regToken, const char *chatId, const char *skypename, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_DELETE, FORMAT, "%s/v1/threads/19:%s/members/8:%s", server, chatId, skypename)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);
	}
};

#endif //_SKYPE_REQUEST_CHATS_H_