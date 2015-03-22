#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class SetStatusRequest : public HttpsRequest
{
public:
	SetStatusRequest(const char *regToken, bool status) :
		HttpsRequest(REQUEST_PUT, "client-s.gateway.messenger.live.com/v1/users/ME/presenceDocs/messagingService")
	{
		Headers
			<< CHAR_VALUE("Accept", "application / json, text / javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");

			const char *data = status
				? "{\"status\":\"Online\"}"
				: "{\"status\":\"Hidden\"}";
			Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
