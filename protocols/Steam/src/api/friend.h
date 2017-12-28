#ifndef _STEAM_REQUEST_FRIEND_H_
#define _STEAM_REQUEST_FRIEND_H_

class GetUserSummariesRequest : public HttpRequest
{
public:
	GetUserSummariesRequest(const char *token, const char *steamIds) :
		HttpRequest(HttpGet, STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001")
	{
		Uri
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("steamids", steamIds);
	}
};

#endif //_STEAM_REQUEST_FRIEND_H_
