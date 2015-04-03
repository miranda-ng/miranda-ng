#ifndef _SKYPE_REQUEST_REGINFO_H_
#define _SKYPE_REQUEST_REGINFO_H_

class GetRegInfoRequest : public HttpRequest
{
public:
	GetRegInfoRequest(const char *token, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints", server)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("Authentication", "skypetoken=%s", token)
			<< CHAR_VALUE("Content-Type", "application/json; charset = UTF-8")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
		
		Body << VALUE("{}");
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
