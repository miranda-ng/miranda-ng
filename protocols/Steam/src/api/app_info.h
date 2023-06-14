#ifndef _STEAM_REQUEST_APP_INFO_H_
#define _STEAM_REQUEST_APP_INFO_H_

class GetAppInfoRequest : public HttpRequest
{
public:
	GetAppInfoRequest(const char *token, const char *appIds) :
		HttpRequest(REQUEST_GET, "/ISteamGameOAuth/GetAppInfo/v0001")
	{
		this << CHAR_PARAM("access_token", token) << CHAR_PARAM("appIds", appIds);
	}

	//{
	//	"apps": [
	//		{
	//			"appid": 271590,
	//			"name" : "Grand Theft Auto V",
	//			"iconurl" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/apps/271590/1e72f87eb927fa1485e68aefaff23c7fd7178251.jpg",
	//			"logourl" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/apps/271590/e447e82f8b0c67f9e001498503c62f2a187bc609.jpg",
	//			"logosmallurl" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/apps/271590/e447e82f8b0c67f9e001498503c62f2a187bc609_thumb.jpg"
	//		}
	//	]
	//}
};

#endif //_STEAM_REQUEST_APP_INFO_H_
