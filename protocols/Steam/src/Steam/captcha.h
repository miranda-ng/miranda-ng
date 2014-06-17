#ifndef _STEAM_CAPTCHA_H_
#define _STEAM_CAPTCHA_H_

namespace SteamWebApi
{
	class GetCaptchaRequest : public HttpGetRequest
	{
	public:
		GetCaptchaRequest(const char *url) :
			HttpGetRequest(url)
		{
			flags = NLHRF_HTTP11 | NLHRF_NODUMP;
		}
	};
}


#endif //_STEAM_CAPTCHA_H_