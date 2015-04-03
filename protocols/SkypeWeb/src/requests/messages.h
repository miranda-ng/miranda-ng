#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

class SendMsgRequest : public HttpRequest
{
public:
	SendMsgRequest(const char *regToken, const char *username, const char *message, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, username)
	{
		Headers
			<< CHAR_VALUE("Accept", "application / json, text / javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		Body << FORMAT_VALUE("{\"clientmessageid\":\"\",\"content\":\"%s\",\"messagetype\":\"RichText\",\"contenttype\":\"text\"}", message);
	}
};

class SendTypingRequest : public HttpRequest
{
public:
	SendTypingRequest(const char *regToken, const char *username,bool bstate, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/conversations/8:%s/messages", server, mir_urlEncode(username))
	{
		Headers
			<< CHAR_VALUE("Accept", "application / json, text / javascript")
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
		data.AppendFormat("{\"clienmessageid\":%d, \"content\":\"\", \"messagetype\":\"%s\", \"contenttype\":\"text\"}", time(NULL), state);

		Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_MESSAGES_H_
