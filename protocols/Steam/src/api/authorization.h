#ifndef _STEAM_REQUEST_AUTHORIZATION_H_
#define _STEAM_REQUEST_AUTHORIZATION_H_

class AuthorizationRequest : public HttpRequest
{
public:
	AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *twofactorcode, const char *guardCode, const char *guardId = "", const char *captchaId = "-1", const char *captchaText = "") :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/dologin/")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		AddHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
		AddHeader("Referer", STEAM_WEB_URL "/mobilelogin/dologin?oauth_client_id=3638BFB1&oauth_scope=read_profile%20write_profile%20read_client%20write_client");

		CMStringA data(CMStringDataFormat::FORMAT,
			"password=%s&username=%s&twofactorcode=%s&emailauth=%s&loginfriendlyname=%s&oauth_client_id=3638BFB1&captchagid=%s&captcha_text=%s&emailsteamid=%s&rsatimestamp=%s&rememberlogin=false&donotcache=%lld",
			ptrA(mir_urlEncode(password)),
			ptrA(mir_urlEncode(username)),
			twofactorcode,
			guardCode,
			"Miranda%20NG",
			captchaId,
			ptrA(mir_urlEncode(captchaText)),
			guardId,
			timestamp,
			time(NULL));
		SetData(data.GetBuffer(), data.GetLength());
	}
};

#endif //_STEAM_REQUEST_AUTHORIZATION_H_
