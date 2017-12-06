#ifndef _STEAM_REQUEST_FRIEND_H_
#define _STEAM_REQUEST_FRIEND_H_

class GetUserSummariesRequest : public HttpRequest
{
public:
	GetUserSummariesRequest(const char *token, const char *steamIds) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001")
	{
		AddParameter("access_token", token);
		AddParameter("steamids", steamIds);
	}
};

#endif //_STEAM_REQUEST_FRIEND_H_
