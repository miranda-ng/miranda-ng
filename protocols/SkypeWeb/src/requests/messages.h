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
	SendMessageRequest(const char *regToken, const char *username, time_t timestamp, const char *message, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("clientmessageid", (long)timestamp));
		node.push_back(JSONNode("messagetype", "RichText"));
		node.push_back(JSONNode("contenttype", "text"));
		node.push_back(JSONNode("content", message));

		Body << VALUE(node.write().c_str());
	}
};

class SendActionRequest : public HttpRequest
{
public:
	SendActionRequest(const char *regToken, const char *username, const char *selfusername, time_t timestamp, const char *message, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		CMStringA content;
		content.AppendFormat("%s %s", selfusername, message);

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("clientmessageid", (long)timestamp));
		node.push_back(JSONNode("messagetype", "RichText"));
		node.push_back(JSONNode("contenttype", "text"));
		node.push_back(JSONNode("content", content));
		node.push_back(JSONNode("skypeemoteoffset", (int)(mir_strlen(selfusername) + 1)));

		Body << VALUE(node.write().c_str());
	}
};

class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(const char *regToken, const char *username, int iState, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, ptrA(mir_urlEncode(username)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		char *state = (iState == PROTOTYPE_SELFTYPING_ON) ? "Control/Typing" : "Control/ClearTyping";

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("clientmessageid", (long)time(NULL)));
		node.push_back(JSONNode("messagetype", state));
		node.push_back(JSONNode("contenttype", "text"));
		node.push_back(JSONNode("content", ""));

		Body << VALUE(node.write().c_str());
	}
};

class MarkMessageReadRequest : public HttpRequest
{
public:
	MarkMessageReadRequest(const char *username, const char *regToken, LONGLONG /*msgId*/ = 0, LONGLONG msgTimestamp = 0, bool isChat = false, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/conversations/%s:%s/properties?name=consumptionhorizon", server, !isChat ? "8" : "19", username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		//"lastReadMessageTimestamp;modificationTime;lastReadMessageId"

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("consumptionhorizon", CMStringA(::FORMAT, "%lld000;%lld000;%lld000", msgTimestamp, time(NULL), msgTimestamp).GetBuffer()));

		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
