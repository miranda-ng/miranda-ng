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
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");

		CMStringA data;
		data.AppendFormat("{\"clientmessageid\":\"%lld\",\"content\":\"%s\",\"messagetype\":\"RichText\",\"contenttype\":\"text\"}", timestamp, message);

		Body << VALUE(data);
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
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");

		CMStringA data;
		data.AppendFormat("{\"clientmessageid\":\"%lld\",\"content\":\"%s %s\",\"messagetype\":\"RichText\",\"contenttype\":\"text\",\"skypeemoteoffset\":\"%d\"}", timestamp, username, message, (int)(mir_strlen(username) + 1));

		Body << VALUE(data);
	}
};

class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(const char *regToken, const char *username, bool bstate, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, ptrA(mir_urlEncode(username)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");
		CMStringA state;
		if (bstate) state = "Control/Typing";
		else state = "Control/ClearTyping";
		CMStringA data;
		data.AppendFormat("{\"clienmessageid\":%lld, \"content\":\"\", \"messagetype\":\"%s\", \"contenttype\":\"text\"}", time(NULL), state);

		Body << VALUE(data);
	}
};

class GetHistoryRequest : public HttpRequest
{
public:
	GetHistoryRequest(const char *regToken, const char *username, LONGLONG timestamp = 0, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages?startTime=%d&pageSize=100&view=msnp24Equivalent&targetType=Passport|Skype|Lync|Thread", server, ptrA(mir_urlEncode(username)), timestamp)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");
	}
};

class SyncHistoryFirstRequest : public HttpRequest
{
public:
	SyncHistoryFirstRequest(const char *regToken, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/conversations?startTime=0&pageSize=100&view=msnp24Equivalent&targetType=Passport|Skype|Lync|Thread", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8");
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
