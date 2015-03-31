#ifndef _SKYPE_REQUEST_MESSAGES_H_
#define _SKYPE_REQUEST_MESSAGES_H_

class SendMsgRequest : public HttpRequest
{
public:
	SendMsgRequest(const char *regToken, const char *username, const char *message) :
		HttpRequest(REQUEST_POST, FORMAT, "client-s.gateway.messenger.live.com/v1/users/ME/conversations/8:%s/messages", username)
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

#endif //_SKYPE_REQUEST_MESSAGES_H_
