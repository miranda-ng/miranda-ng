#ifndef _SKYPE_REQUEST_LOGOUT_H_
#define _SKYPE_REQUEST_LOGOUT_H_

class LogoutRequest : public HttpsPostRequest
{
public:
	LogoutRequest() : HttpsPostRequest("login.skype.com/logout") { }
};

#endif //_SKYPE_REQUEST_LOGOUT_H_
