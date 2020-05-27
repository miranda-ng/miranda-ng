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

#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

struct SendMessageParam
{
	MCONTACT hContact;
	DWORD hMessage;
};

struct SendMessageRequest : public AsyncHttpRequest
{
	SendMessageRequest(const char *username, time_t timestamp, const char *message, CSkypeProto *ppro, const char *MessageType = nullptr) :
	  AsyncHttpRequest(REQUEST_POST, 0, &CSkypeProto::OnMessageSent)
	{
		m_szUrl.Format("/users/ME/conversations/8:%s/messages", username);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		JSONNode node;
		node  << JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", MessageType ? MessageType : "Text")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message);
		m_szParam = node.write().c_str();
	}
};

struct SendActionRequest : public AsyncHttpRequest
{
	SendActionRequest(const char *username, time_t timestamp, const char *message, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST, 0, &CSkypeProto::OnMessageSent)
	{
		m_szUrl.Format("/users/ME/conversations/8:%s/messages", username);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		CMStringA content;
		content.AppendFormat("%s %s", ppro->m_szSkypename.c_str(), message);

		JSONNode node;
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", "RichText")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", content)
			<< JSONNode("skypeemoteoffset", ppro->m_szSkypename.GetLength() + 1);
		m_szParam = node.write().c_str();
	}
};

struct SendTypingRequest : public AsyncHttpRequest
{
	SendTypingRequest(const char *username, int iState, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST)
	{
		m_szUrl.Format("/users/ME/conversations/8:%s/messages", mir_urlEncode(username).c_str());

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		const char *state = (iState == PROTOTYPE_SELFTYPING_ON) ? "Control/Typing" : "Control/ClearTyping";

		JSONNode node;
		node
			<< JSONNode("clientmessageid", (long)time(NULL))
			<< JSONNode("messagetype", state)
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", "");
		m_szParam = node.write().c_str();
	}
};

struct MarkMessageReadRequest : public AsyncHttpRequest
{
	MarkMessageReadRequest(const char *username, LONGLONG /*msgId*/, LONGLONG msgTimestamp, bool isChat, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_PUT)
	{
		m_szUrl.Format("/users/ME/conversations/%d:%s/properties?name=consumptionhorizon", !isChat ? 8 : 19, username);

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

		//"lastReadMessageTimestamp;modificationTime;lastReadMessageId"

		JSONNode node(JSON_NODE);
		node << JSONNode("consumptionhorizon", CMStringA(::FORMAT, "%lld000;%lld000;%lld000", msgTimestamp, time(NULL), msgTimestamp));
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
