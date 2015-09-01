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

class LoadChatsRequest : public HttpRequest
{
public:
	LoadChatsRequest(LoginInfo &li) :
	  HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/conversations", li.endpoint.szServer)
	{
		Url
			<< INT_VALUE("startTime", 0)
			<< INT_VALUE("pageSize", 100)
			<< CHAR_VALUE("view", "msnp24Equivalent")
			<< CHAR_VALUE("targetType", "Thread");

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");
	}
};

class SendChatMessageRequest : public HttpRequest
{
public:
	SendChatMessageRequest(const char *to, ULONGLONG timestamp, const char *message, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/19:%s/messages", li.endpoint.szServer, to)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");
		JSONNode node;
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp)) 
			<< JSONNode("messagetype", "RichText") 
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message);

		Body << VALUE(node.write().c_str());
	}
};

class SendChatActionRequest : public HttpRequest
{
public:
	SendChatActionRequest(const char *to, ULONGLONG timestamp, const char *message, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/19:%s/messages", li.endpoint.szServer, to)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node(JSON_NODE);
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", "RichText")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message)
			<< JSONNode("skypeemoteoffset", 4);

		Body << VALUE(node.write().c_str());
	}
};

class CreateChatroomRequest : public HttpRequest
{
public:
	CreateChatroomRequest(const LIST<char> &skypenames, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/threads", li.endpoint.szServer)
	{
		//{"members":[{"id":"8:user3","role":"User"},{"id":"8:user2","role":"User"},{"id":"8:user1","role":"Admin"}]}

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);

		JSONNode node;
		JSONNode members(JSON_ARRAY); members.set_name("members");

		for (int i = 0; i < skypenames.getCount(); i++)
		{
			JSONNode member;
			member 
				<< JSONNode("id", CMStringA(::FORMAT, "8:%s", skypenames[i]).GetBuffer())
				<< JSONNode("role", !mir_strcmpi(skypenames[i], li.szSkypename) ? "Admin" : "User");
			members << member;
		}
		node << members;

		Body << VALUE(node.write().c_str());
	}
};

class GetChatInfoRequest : public HttpRequest
{
public:
	GetChatInfoRequest(const char *chatId, LoginInfo &li) :
	  HttpRequest(REQUEST_GET, FORMAT, "%s/v1/threads/19:%s", li.endpoint.szServer, chatId)
	{
		Url << CHAR_VALUE("view", "msnp24Equivalent");

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);
	}
};

class InviteUserToChatRequest : public HttpRequest
{
public:
	InviteUserToChatRequest(const char *chatId, const char *skypename, const char* role, LoginInfo &li) :
	  HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/threads/19:%s/members/8:%s", li.endpoint.szServer, chatId, skypename)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);

		JSONNode node;

		node << JSONNode("role", role);

		Body << VALUE(node.write().c_str());
	}
};

class KickUserRequest : public HttpRequest
{
public:
	KickUserRequest(const char *chatId, const char *skypename, LoginInfo &li) :
	  HttpRequest(REQUEST_DELETE, FORMAT, "%s/v1/threads/19:%s/members/8:%s", li.endpoint.szServer, chatId, skypename)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);
	}
};

class SetChatPropertiesRequest : public HttpRequest
{
public:
	SetChatPropertiesRequest(const char *chatId, const char *propname, const char *value, LoginInfo &li) :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/threads/19:%s/properties?name=%s", li.endpoint.szServer, chatId, propname)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);

		JSONNode node;
		node << JSONNode(propname, value);

		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_CHATS_H_