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
		JSONNODE *node = json_new(5);
		json_push_back(node, json_new_i("clientmessageid", timestamp));
		json_push_back(node, json_new_a("messagetype", "RichText"));
		json_push_back(node, json_new_a("contenttype", "text"));
		json_push_back(node, json_new_a("content", message));

		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));
		Body << VALUE(data);

		json_delete(node);
	}
};

class SendActionRequest : public HttpRequest
{
public:
	SendActionRequest(const char *regToken, const char *username, time_t timestamp, const char *message, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		CMStringA content;
		content.AppendFormat("%s %s", username, message);

		JSONNODE *node = json_new(5);
		json_push_back(node, json_new_i("clientmessageid", timestamp));
		json_push_back(node, json_new_a("messagetype", "RichText"));
		json_push_back(node, json_new_a("contenttype", "text"));
		json_push_back(node, json_new_a("content", content));
		json_push_back(node, json_new_i("skypeemoteoffset", (int)(mir_strlen(username) + 1)));

		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));
		Body << VALUE(data);

		json_delete(node);
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

		JSONNODE *node = json_new(5);
		json_push_back(node, json_new_i("clientmessageid", time(NULL)));
		json_push_back(node, json_new_a("messagetype", state));
		json_push_back(node, json_new_a("contenttype", "text"));
		json_push_back(node, json_new_a("content", ""));

		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));
		Body << VALUE(data);

		json_delete(node);
	}
};

class MarkMessageReadRequest : public HttpRequest
{
public:
	MarkMessageReadRequest(const char *username, const char *regToken, LONGLONG msgId = 0, LONGLONG msgTimestamp = 0, bool isChat = false, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/conversations/%s:%s/properties?name=consumptionhorizon", server, !isChat ? "8" : "19", username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		//"lastReadMessageTimestamp;modificationTime;lastReadMessageId"
		CMStringA data(::FORMAT, "{\"consumptionhorizon\":\"%lld000;%lld000;%lld000\"}", msgTimestamp, time(NULL), msgTimestamp);
		Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
