#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class SetStatusRequest : public HttpRequest
{
public:
	SetStatusRequest(const char *regToken, const char *status, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/presenceDocs/messagingService", server)
	{
		CMStringA auth = "registrationToken=";
		CMStringA statuss;
		statuss.AppendFormat("{\"status\":\"%s\"}", status);
		auth += regToken;
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< CHAR_VALUE("RegistrationToken", auth)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

		Body << VALUE(statuss);
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
