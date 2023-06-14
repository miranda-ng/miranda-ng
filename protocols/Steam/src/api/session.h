#ifndef _STEAM_REQUEST_SESSION_H_
#define _STEAM_REQUEST_SESSION_H_

struct GetHostsRequest : public HttpRequest
{
	GetHostsRequest() :
		HttpRequest(REQUEST_GET, "/ISteamDirectory/GetCMList/v0001?cellid=0")
	{}
};

struct GetSessionRequest : public HttpRequest
{
	GetSessionRequest(const char *token, const char *steamId, const char *cookie) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobileloginsucceeded")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		this
			<< CHAR_PARAM("oauth_token", token)
			<< CHAR_PARAM("steamid", steamId)
			<< CHAR_PARAM("webcookie", cookie);

		char data[512];
		mir_snprintf(data, _countof(data),
			"oauth_token=%s&steamid=%s&webcookie=%s",
			token,
			steamId,
			cookie);
	}
};

struct GetSessionRequest2 : public HttpRequest
{
	GetSessionRequest2(const char *token, const char *steamId) :
		HttpRequest(REQUEST_GET, STEAM_WEB_URL "/mobilesettings/GetManifest/v0001")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPHEADERS;

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s||oauth:%s", steamId, token);

		AddHeader("Cookie", cookie);
	}
};

#endif //_STEAM_REQUEST_SESSION_H_
