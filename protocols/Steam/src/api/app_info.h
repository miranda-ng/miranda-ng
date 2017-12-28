#ifndef _STEAM_REQUEST_APP_INFO_H_
#define _STEAM_REQUEST_APP_INFO_H_

class GetAppInfoRequest : public HttpRequest
{
public:
	GetAppInfoRequest(const char *token, const char *appIds) :
		HttpRequest(HttpGet, STEAM_API_URL "/ISteamGameOAuth/GetAppInfo/v0001")
	{
		Uri
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("appIds", appIds);
	}
};

#endif //_STEAM_REQUEST_APP_INFO_H_
