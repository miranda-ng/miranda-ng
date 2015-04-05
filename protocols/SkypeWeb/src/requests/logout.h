#ifndef _SKYPE_REQUEST_LOGOUT_H_
#define _SKYPE_REQUEST_LOGOUT_H_

class LogoutRequest : public HttpRequest
{
public:
	LogoutRequest() : HttpRequest(REQUEST_GET, "login.skype.com/logout")
	{
		flags |= NLHRF_REDIRECT;
		Url
			<< INT_VALUE("client_id", 578134)
			<< CHAR_VALUE("redirect_uri", "https%3A%2F%2Fweb.skype.com&intsrc=client-_-webapp-_-production-_-go-signin");
		Headers
			<< CHAR_VALUE("Referer", "https://web.skype.com/");
	}
};

#endif //_SKYPE_REQUEST_LOGOUT_H_
