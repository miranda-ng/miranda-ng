#ifndef _STEAM_REQUEST_CAPTCHA_H_
#define _STEAM_REQUEST_CAPTCHA_H_

class GetCaptchaRequest : public HttpRequest
{
public:
	GetCaptchaRequest(const char *captchaId) :
		HttpRequest(REQUEST_GET, FORMAT, STEAM_WEB_URL "/public/captcha.php?gid=%s", captchaId)
	{
		flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	}
};

#endif //_STEAM_REQUEST_CAPTCHA_H_
