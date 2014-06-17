#ifndef _STEAM_AUTHORIZATION_H_
#define _STEAM_AUTHORIZATION_H_

namespace SteamWebApi
{
	class AuthorizationRequest : public HttpsPostRequest
	{
		void InitData(const char *username, const char *password, const char *timestamp, const char *guardId = "-1", const char *guardCode = "", const char *captchaId = "-1", const char *captchaText = "")
		{
			char data[1024];
			mir_snprintf(data, SIZEOF(data),
				"username=%s&password=%s&emailsteamid=%s&emailauth=%s&captchagid=%s&captcha_text=%s&rsatimestamp=%s&donotcache=%ld&remember_login=true&oauth_client_id=DE45CD61&oauth_scope=read_profile write_profile read_client write_client",
				username,
				ptrA(mir_urlEncode(password)),
				guardId,
				guardCode,
				captchaId,
				captchaText,
				timestamp,
				time(NULL));

			SetData(data, strlen(data));
		}

	public:
		AuthorizationRequest(const char *username, const char *password, const char *timestamp) :
			HttpsPostRequest(STEAM_COM_URL "/mobilelogin/dologin")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			InitData(username, password, timestamp);
		}

		AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *guardId, const char *guardCode) :
			HttpsPostRequest(STEAM_COM_URL "/mobilelogin/dologin")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			InitData(username, password, timestamp, guardId, guardCode);
		}

		AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *guardId, const char *guardCode, const char *captchaId, const char *captchaText) :
			HttpsPostRequest(STEAM_COM_URL "/mobilelogin/dologin")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			InitData(username, password, timestamp, guardId, guardCode, captchaId, captchaText);
		}
	};
}


#endif //_STEAM_AUTHORIZATION_H_