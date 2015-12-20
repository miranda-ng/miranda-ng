#ifndef _STEAM_REQUEST_SESSION_H_
#define _STEAM_REQUEST_SESSION_H_

class GetSessionRequest : public HttpRequest
{
public:
	GetSessionRequest(const char *token, const char *steamId, const char *cookie) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobileloginsucceeded")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		char data[512];
		mir_snprintf(data, _countof(data),
			"oauth_token=%s&steamid=%s&webcookie=%s",
			token,
			steamId,
			cookie);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	}
};

class GetSessionRequest2 : public HttpRequest
{
public:
	GetSessionRequest2() :
		HttpRequest(REQUEST_GET, STEAM_WEB_URL)
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;
	}
};

#endif //_STEAM_REQUEST_SESSION_H_
