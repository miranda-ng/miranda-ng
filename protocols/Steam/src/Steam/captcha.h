#ifndef _STEAM_CAPTCHA_H_
#define _STEAM_CAPTCHA_H_

namespace SteamWebApi
{
	class GetCaptchaRequest : public HttpGetRequest
	{
	public:
		GetCaptchaRequest(const char *captchaId) :
			HttpGetRequest(STEAM_WEB_URL "/public/captcha.php?gid=%s", captchaId)
		{
			flags = NLHRF_HTTP11 | NLHRF_NODUMP;
		}
	};
}


#endif //_STEAM_CAPTCHA_H_