/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

struct DeleteMessageRequest : public AsyncHttpRequest
{
	DeleteMessageRequest(CSkypeProto *ppro, const char *username, const char *msgId) :
		AsyncHttpRequest(REQUEST_DELETE, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(username) + "/messages/" + msgId)
	{
		AddAuthentication(ppro);

		AddHeader("Origin", "https://web.skype.com");
		AddHeader("Referer", "https://web.skype.com/");
	}
};

struct MarkMessageReadRequest : public AsyncHttpRequest
{
	MarkMessageReadRequest(const char *username, int64_t msgTimestamp) :
	  AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(username) + "/properties?name=consumptionhorizon")
	{
		JSONNode node(JSON_NODE);
		node << CHAR_PARAM("consumptionhorizon", CMStringA(::FORMAT, "%lld;%lld;%lld", msgTimestamp, msgTimestamp, msgTimestamp));
		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
