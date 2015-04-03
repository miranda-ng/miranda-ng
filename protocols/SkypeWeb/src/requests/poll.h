#ifndef _SKYPE_POLL_H_
#define _SKYPE_POLL_H_

class PollRequest : public HttpRequest
{
public:
	PollRequest(const char *regToken, const char *server = "client-s.gateway.messenger.live.com") :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions/0/poll", server)
	{
		//timeout = 30 * 1000;
		//flags |= NLHRF_PERSISTENT;
		Headers 
			<< CHAR_VALUE("Connection", "keep-alive")
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Expires", "0")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("BehaviorOverride", "redirectAs404")
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Origin", "https://web.skype.com")
			<< CHAR_VALUE("Connection", "keep-alive");
	}
};
#endif //_SKYPE_POLL_H_