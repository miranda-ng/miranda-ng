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

#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

class SendMessageRequest : public HttpRequest
{
public:
	SendMessageRequest(const char *username, ULONGLONG timestamp, const char *message, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", li.endpoint.szServer, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node;
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", "Text")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", message);

		Body << VALUE(node.write().c_str());
	}
};

class SendActionRequest : public HttpRequest
{
public:
	SendActionRequest(const char *username, ULONGLONG timestamp, const char *message, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", li.endpoint.szServer, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		CMStringA content;
		content.AppendFormat("%s %s", li.szSkypename, message);

		JSONNode node;
		node 
			<< JSONNode("clientmessageid", CMStringA(::FORMAT, "%llu", (ULONGLONG)timestamp))
			<< JSONNode("messagetype", "RichText")
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", content)
			<< JSONNode("skypeemoteoffset", (int)(mir_strlen(li.szSkypename) + 1));

		Body << VALUE(node.write().c_str());
	}
};

class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(const char *username, int iState, LoginInfo &li) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", li.endpoint.szServer, ptrA(mir_urlEncode(username)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		const char *state = (iState == PROTOTYPE_SELFTYPING_ON) ? "Control/Typing" : "Control/ClearTyping";

		JSONNode node;
		node
			<< JSONNode("clientmessageid", (long)time(NULL))
			<< JSONNode("messagetype", state)
			<< JSONNode("contenttype", "text")
			<< JSONNode("content", "");

		Body << VALUE(node.write().c_str());
	}
};

class MarkMessageReadRequest : public HttpRequest
{
public:
	MarkMessageReadRequest(const char *username, LONGLONG /*msgId*/, LONGLONG msgTimestamp, bool isChat, LoginInfo &li) :
	  HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/conversations/%d:%s/properties?name=consumptionhorizon", li.endpoint.szServer, !isChat ? 8 : 19, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		//"lastReadMessageTimestamp;modificationTime;lastReadMessageId"

		JSONNode node(JSON_NODE);
		node << JSONNode("consumptionhorizon", CMStringA(::FORMAT, "%lld000;%lld000;%lld000", msgTimestamp, time(NULL), msgTimestamp));

		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
