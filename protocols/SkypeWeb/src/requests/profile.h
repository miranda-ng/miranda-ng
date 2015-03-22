#ifndef _SKYPE_REQUEST_PROFILE_H_
#define _SKYPE_REQUEST_PROFILE_H_

class GetProfileRequest : public HttpRequest
{
public:
	GetProfileRequest(const char *token, const char *skypename = "self") :
		HttpRequest(REQUEST_GET, "api.skype.com/users/%s/profile", skypename)
	{
		flags |= NLHRF_SSL;

		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Accept", "application/json");


		//AddHeader("Origin", "https://web.skype.com");
		//AddHeader("Referer", "https://web.skype.com/main");
	}
};

#endif //_SKYPE_REQUEST_PROFILE_H_
