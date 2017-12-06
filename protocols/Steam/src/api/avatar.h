#ifndef _STEAM_REQUEST_AVATAR_H_
#define _STEAM_REQUEST_AVATAR_H_

class GetAvatarRequest : public HttpRequest
{
public:
	GetAvatarRequest(const char *url) :
		HttpRequest(REQUEST_GET, url)
	{
		flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	}
};

#endif //_STEAM_REQUEST_AVATAR_H_
