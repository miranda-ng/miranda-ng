#ifndef _SKYPE_REQUEST_PROFILE_H_
#define _SKYPE_REQUEST_PROFILE_H_

class GetProfileRequest : public HttpRequest
{
public:
	GetProfileRequest(const char *token) :
		HttpRequest(REQUEST_GET, "api.skype.com/users/self/profile")
	{
		flags |= NLHRF_SSL;

		AddHeader("X-Skypetoken", token);
		AddHeader("Accept", "application/json");
	}
};

#endif //_SKYPE_REQUEST_PROFILE_H_
