#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

class SendMessageRequest : public HttpRequest
{
public:
	SendMessageRequest(const char *regToken, const char *username, time_t timestamp, const char *message, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		CMStringA data;
		data.AppendFormat("{\"clientmessageid\":\"%lld\",\"content\":\"%s\",\"messagetype\":\"RichText\",\"contenttype\":\"text\"}", timestamp, message);

		Body << VALUE(data);
	}
};

class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(const char *regToken, const char *username, bool bstate, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, mir_urlEncode(username))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
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
	GetHistoryRequest(const char *regToken, int time, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/conversations?startTime=%d&pageSize=100&view=msnp24Equivalent&targetType=Passport|Skype|Lync|Thread", server, time)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
