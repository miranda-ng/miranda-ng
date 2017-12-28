#ifndef _STEAM_REQUEST_AUTHORIZATION_H_
#define _STEAM_REQUEST_AUTHORIZATION_H_

class AuthorizationRequest : public HttpRequest
{
public:
	AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *twoFactorCode, const char *guardCode, const char *guardId = "", const char *captchaId = "-1", const char *captchaText = "") :
		HttpRequest(HttpPost, STEAM_WEB_URL "/mobilelogin/dologin/")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		Headers
			<< CHAR_PARAM("Referer", STEAM_WEB_URL "/mobilelogin/dologin?oauth_client_id=3638BFB1&oauth_scope=read_profile%20write_profile%20read_client%20write_client")
			<< CHAR_PARAM("Cookie", "mobileClientVersion=1291812;forceMobile=1;mobileClient=ios");

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("oauth_client_id", "3638BFB1")
			<< CHAR_PARAM("loginfriendlyname", "Miranda NG")
			<< CHAR_PARAM("password", password)
			<< CHAR_PARAM("username", username)
			<< CHAR_PARAM("twofactorcode", twoFactorCode)
			<< CHAR_PARAM("emailsteamid", guardId)
			<< CHAR_PARAM("emailauth", guardCode)
			<< CHAR_PARAM("captchagid", captchaId)
			<< CHAR_PARAM("captcha_text", captchaText)
			<< CHAR_PARAM("rsatimestamp", timestamp)
			<< BOOL_PARAM("rememberlogin", false)
			<< INT64_PARAM("donotcache", time(NULL));
	}
};

#endif //_STEAM_REQUEST_AUTHORIZATION_H_
