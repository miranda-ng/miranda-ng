#ifndef _SKYPE_POLL_H_
#define _SKYPE_POLL_H_

class PollRequest : public HttpRequest
{
public:
	PollRequest(const char *regToken, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions/0/poll", server)
	{
		timeout = INFINITE;
		flags |= NLHRF_PERSISTENT;
		Headers 
			<< CHAR_VALUE("Connection", "keep-alive")
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);
	}
};
#endif //_SKYPE_POLL_H_