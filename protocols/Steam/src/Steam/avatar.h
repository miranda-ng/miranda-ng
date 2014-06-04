#ifndef _STEAM_AVATAR_H_
#define _STEAM_AVATAR_H_

namespace SteamWebApi
{
	class GetAvatarRequest : public HttpGetRequest
	{
	public:
		GetAvatarRequest(const char *url) :
			HttpGetRequest(url)
		{
			flags = NLHRF_HTTP11 | NLHRF_NODUMP;
		}
	};
}


#endif //_STEAM_AVATAR_H_