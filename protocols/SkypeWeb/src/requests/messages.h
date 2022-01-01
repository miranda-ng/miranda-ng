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

#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

struct SendMessageParam
{
	MCONTACT hContact;
	uint32_t hMessage;
};

struct SendMessageRequest : public AsyncHttpRequest
{
	SendMessageRequest(const char *username, time_t timestamp, const char *message, const char *MessageType = nullptr) :
	  AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, 0, &CSkypeProto::OnMessageSent)
	{
		m_szUrl.AppendFormat("/users/ME/conversations/%s/messages", mir_urlEncode(username).c_str());

		JSONNode node;
		node  << INT64_PARAM("clientmessageid", timestamp) << CHAR_PARAM("messagetype", MessageType ? MessageType : "Text")
			<< CHAR_PARAM("contenttype", "text") << CHAR_PARAM("content", message);
		m_szParam = node.write().c_str();
	}
};

struct SendActionRequest : public AsyncHttpRequest
{
	SendActionRequest(const char *username, time_t timestamp, const char *message, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, 0, &CSkypeProto::OnMessageSent)
	{
		m_szUrl.AppendFormat("/users/ME/conversations/%s/messages", mir_urlEncode(username).c_str());

		CMStringA content;
		content.AppendFormat("%s %s", ppro->m_szSkypename.c_str(), message);

		JSONNode node;
		node << INT64_PARAM("clientmessageid", timestamp) << CHAR_PARAM("messagetype", "RichText") << CHAR_PARAM("contenttype", "text")
			<< CHAR_PARAM("content", content) << INT_PARAM("skypeemoteoffset", ppro->m_szSkypename.GetLength() + 1);
		m_szParam = node.write().c_str();
	}
};

struct SendTypingRequest : public AsyncHttpRequest
{
	SendTypingRequest(const char *username, int iState) :
	  AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(username) + "/messages")
	{
		const char *state = (iState == PROTOTYPE_SELFTYPING_ON) ? "Control/Typing" : "Control/ClearTyping";

		JSONNode node;
		node << INT_PARAM("clientmessageid", (long)time(NULL)) << CHAR_PARAM("messagetype", state)
			<< CHAR_PARAM("contenttype", "text") << CHAR_PARAM("content", "");
		m_szParam = node.write().c_str();
	}
};

struct MarkMessageReadRequest : public AsyncHttpRequest
{
	MarkMessageReadRequest(const char *username, LONGLONG /*msgId*/, LONGLONG msgTimestamp) :
	  AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT)
	{
		m_szUrl.AppendFormat("/users/ME/conversations/%s/properties?name=consumptionhorizon", mir_urlEncode(username).c_str());

		JSONNode node(JSON_NODE);
		node << CHAR_PARAM("consumptionhorizon", CMStringA(::FORMAT, "%lld000;%lld000;%lld000", msgTimestamp, time(NULL), msgTimestamp));
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
