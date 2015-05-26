#ifndef _STEAM_REQUEST_AUTHORIZATION_H_
#define _STEAM_REQUEST_AUTHORIZATION_H_

class AuthorizationRequest : public HttpRequest
{
public:
	AuthorizationRequest(const char *username, const char *password, const char *timestamp) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/dologin")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		char data[1024];
		mir_snprintf(data, SIZEOF(data),
			"username=%s&password=%s&oauth_client_id=3638BFB1&oauth_scope=read_profile write_profile read_client write_client&captchagid=-1&rsatimestamp=%s",
			ptrA(mir_urlEncode(username)),
			ptrA(mir_urlEncode(password)),
			timestamp);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}

	AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *guardCode) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/dologin")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		char data[1024];
		mir_snprintf(data, SIZEOF(data),
			"username=%s&password=%s&emailauth=%s&loginfriendlyname=MirandaNG&oauth_client_id=3638BFB1&oauth_scope=read_profile write_profile read_client write_client&captchagid=-1&rsatimestamp=%s",
			ptrA(mir_urlEncode(username)),
			ptrA(mir_urlEncode(password)),
			guardCode,
			timestamp);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}

	AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *captchaId, const char *captchaText) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/dologin")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		char data[1024];
		mir_snprintf(data, SIZEOF(data),
			"username=%s&password=%s&emailauth=&captchagid=%s&captcha_text=%s&oauth_client_id=3638BFB1&oauth_scope=read_profile write_profile read_client write_client&rsatimestamp=%s",
			ptrA(mir_urlEncode(username)),
			ptrA(mir_urlEncode(password)),
			captchaId,
			ptrA(mir_urlEncode(captchaText)),
			timestamp);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_STEAM_REQUEST_AUTHORIZATION_H_
