#ifndef _STEAM_REQUEST_CAPTCHA_H_
#define _STEAM_REQUEST_CAPTCHA_H_

struct GetCaptchaRequest : public HttpRequest
{
	GetCaptchaRequest(const char *captchaId) :
		HttpRequest(REQUEST_GET, STEAM_WEB_URL "/public/captcha.php")
	{
		flags = NLHRF_HTTP11 | NLHRF_NODUMP;

		this << CHAR_PARAM("gid", captchaId);
	}
};

#endif //_STEAM_REQUEST_CAPTCHA_H_
