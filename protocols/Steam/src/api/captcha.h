#ifndef _STEAM_REQUEST_CAPTCHA_H_
#define _STEAM_REQUEST_CAPTCHA_H_

class GetCaptchaRequest : public HttpRequest
{
public:
	GetCaptchaRequest(const char *captchaId) :
		HttpRequest(HttpGet, STEAM_WEB_URL "/public/captcha.php")
	{
		flags = NLHRF_HTTP11 | NLHRF_NODUMP;

		Uri << CHAR_PARAM("gid", captchaId);
	}
};

#endif //_STEAM_REQUEST_CAPTCHA_H_
