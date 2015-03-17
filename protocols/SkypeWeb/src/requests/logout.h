#ifndef _SKYPE_REQUEST_LOGOUT_H_
#define _SKYPE_REQUEST_LOGOUT_H_

class LogoutRequest : public HttpRequest
{
public:
	LogoutRequest() : HttpRequest(REQUEST_POST, "login.skype.com/logout")
	{
		flags |= NLHRF_SSL;
	}
};

#endif //_SKYPE_REQUEST_LOGOUT_H_
