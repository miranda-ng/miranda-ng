#ifndef _SKYPE_REQUEST_REGINFO_H_
#define _SKYPE_REQUEST_REGINFO_H_

class CreateEndpointRequest : public HttpRequest
{
public:
	CreateEndpointRequest(const char *token, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints", server)
	{

		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("Authentication", "skypetoken=%s", token);
		
		Body << VALUE("{}");
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
