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

#ifndef _SKYPE_REQUEST_CHATS_H_
#define _SKYPE_REQUEST_CHATS_H_

struct LoadChatsRequest : public AsyncHttpRequest
{
	LoadChatsRequest(CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_GET, "/users/ME/conversations", &CSkypeProto::OnLoadChats)
	{
		this << INT_PARAM("startTime", 0) << INT_PARAM("pageSize", 100) 
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Thread");

		AddHeader("Accept", "application/json, text/javascript");
		AddRegistrationToken(ppro);
		AddHeader("Content-Type", "application/json; charset = UTF-8");
	}
};

struct SendChatMessageRequest : public AsyncHttpRequest
{
	SendChatMessageRequest(const char *to, time_t timestamp, const char *message, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST)
	{
		m_szUrl.Format("/users/ME/conversations/19:%s/messages", to);

		AddHeader("Accept", "application/json, text/javascript");
		AddRegistrationToken(ppro);
		AddHeader("Content-Type", "application/json; charset=UTF-8");

		JSONNode node;
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu000", (ULONGLONG)timestamp)) 
			<< JSONNode("messagetype", "RichText") 
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message);
		m_szParam = node.write().c_str();
	}
};

struct SendChatActionRequest : public AsyncHttpRequest
{
	SendChatActionRequest(const char *to, time_t timestamp, const char *message, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST)
	{
		m_szUrl.Format("/users/ME/conversations/19:%s/messages", to);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node(JSON_NODE);
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu000", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", "RichText")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message)
			<< JSONNode("skypeemoteoffset", 4);
		m_szParam = node.write().c_str();
	}
};

struct CreateChatroomRequest : public AsyncHttpRequest
{
	CreateChatroomRequest(const LIST<char> &skypenames, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST, "/threads")
	{
		//{"members":[{"id":"8:user3","role":"User"},{"id":"8:user2","role":"User"},{"id":"8:user1","role":"Admin"}]}
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node;
		JSONNode members(JSON_ARRAY); members.set_name("members");

		for (auto &it : skypenames)
		{
			JSONNode member;
			member 
				<< JSONNode("id", CMStringA(::FORMAT, "8:%s", it).GetBuffer())
				<< JSONNode("role", !mir_strcmpi(it, ppro->m_szSkypename) ? "Admin" : "User");
			members << member;
		}
		node << members;
		m_szParam = node.write().c_str();
	}
};

struct GetChatInfoRequest : public AsyncHttpRequest
{
	GetChatInfoRequest(const char *chatId, const CMStringW topic, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_GET, 0, &CSkypeProto::OnGetChatInfo)
	{
		m_szUrl.Format("/threads/%s%s", ppro->m_szServer, strstr(chatId, "19:") == chatId ? "" : "19:", chatId);
		pUserInfo = topic.Detach();

		this << CHAR_PARAM("view", "msnp24Equivalent");

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);
	}
};

struct InviteUserToChatRequest : public AsyncHttpRequest
{
	InviteUserToChatRequest(const char *chatId, const char *skypename, const char* role, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_PUT)
	{
		m_szUrl.Format("/threads/19:%s/members/8:%s", chatId, skypename);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node;
		node << JSONNode("role", role);
		m_szParam = node.write().c_str();
	}
};

struct KickUserRequest : public AsyncHttpRequest
{
	KickUserRequest(const char *chatId, const char *skypename, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_DELETE)
	{
		m_szUrl.Format("/threads/19:%s/members/8:%s", chatId, skypename);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);
	}
};

struct SetChatPropertiesRequest : public AsyncHttpRequest
{
	SetChatPropertiesRequest(const char *chatId, const char *propname, const char *value, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_PUT)
	{
		m_szUrl.Format("/threads/19:%s/properties?name=%s", chatId, propname);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node;
		node << JSONNode(propname, value);
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_CHATS_H_