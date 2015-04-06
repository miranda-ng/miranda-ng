#ifndef _SKYPE_REQUEST_AVATAR_H_
#define _SKYPE_REQUEST_AVATAR_H_

class GetAvatarRequest : public HttpRequest
{
public:
	GetAvatarRequest(const char *url) : HttpRequest(REQUEST_GET, url)
	{
		flags |= NLHRF_REDIRECT;
	}
};

#endif //_SKYPE_REQUEST_AVATAR_H_
