#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class SetStatusRequest : public HttpsRequest
{
public:
	SetStatusRequest(const char *regToken, int status) :
		HttpsRequest(REQUEST_PUT, "client-s.gateway.messenger.live.com/v1/users/ME/presenceDocs/messagingService")
	{
		CMStringA auth = "registrationToken=";
		auth += regToken;
		Headers
			<< CHAR_VALUE("Accept", "application / json, text / javascript")
			<< CHAR_VALUE("Expires", "0")
			<< CHAR_VALUE("RegistrationToken", auth)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

			const char *data;
			if (status == ID_STATUS_ONLINE)
				data = "{\"status\":\"Online\"}";
			else if (status == ID_STATUS_INVISIBLE)
				data = "{\"status\":\"Hidden\"}";
			else if (status == ID_STATUS_AWAY)
				data = "{\"status\":\"Away\"}";
			else 
				data = "{\"status\":\"Online\"}";
			Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
