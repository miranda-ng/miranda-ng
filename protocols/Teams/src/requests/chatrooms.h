/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

struct CreateChatroomRequest : public AsyncHttpRequest
{
	CreateChatroomRequest(const LIST<char> &skypenames, CTeamsProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/threads")
	{
		//{"members":[{"id":"8:user3","role":"User"},{"id":"8:user2","role":"User"},{"id":"8:user1","role":"Admin"}]}
		JSONNode node;
		JSONNode members(JSON_ARRAY); members.set_name("members");

		for (auto &it : skypenames) {
			JSONNode member;
			member << CHAR_PARAM("id", it) << CHAR_PARAM("role", !mir_strcmpi(it, ppro->m_szSkypename) ? "Admin" : "User");
			members << member;
		}
		node << members;
		m_szParam = node.write().c_str();
	}
};

struct GetChatMembersRequest : public AsyncHttpRequest
{
	GetChatMembersRequest(const LIST<char> &ids, SESSION_INFO *si) :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/profiles", &CTeamsProto::OnGetChatMembers)
	{
		JSONNode node, mris(JSON_ARRAY); mris.set_name("mris");
		for (auto &it : ids)
			mris.push_back(JSONNode("", it));
		node << mris << CHAR_PARAM("locale", "en-US");
		m_szParam = node.write().c_str();

		pUserInfo = si;
	}
};

struct GetChatInfoRequest : public AsyncHttpRequest
{
	GetChatInfoRequest(const wchar_t *chatId) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, 0, &CTeamsProto::OnGetChatInfo)
	{
		m_szUrl.AppendFormat("/threads/%S", chatId);

		this << CHAR_PARAM("view", "msnp24Equivalent");
	}
};

struct InviteUserToChatRequest : public AsyncHttpRequest
{
	InviteUserToChatRequest(const char *chatId, const char *skypename, const char *role) :
		AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT)
	{
		m_szUrl.AppendFormat("/threads/%s/members/%s", chatId, skypename);

		JSONNode node;
		node << CHAR_PARAM("role", role);
		m_szParam = node.write().c_str();
	}
};

struct SetChatPropertiesRequest : public AsyncHttpRequest
{
	SetChatPropertiesRequest(const char *chatId, const char *propname, const char *value) :
		AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT)
	{
		m_szUrl.AppendFormat("/threads/%s/properties?name=%s", chatId, propname);

		JSONNode node;
		node << CHAR_PARAM(propname, value);
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_CHATS_H_