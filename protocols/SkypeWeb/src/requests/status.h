#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class SetStatusRequest : public HttpRequest
{
public:
	SetStatusRequest(const char *regToken, const char *status, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/presenceDocs/messagingService", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		CMStringA data;
		data.AppendFormat("{\"status\":\"%s\"}", status);
		Body << VALUE(data);
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
