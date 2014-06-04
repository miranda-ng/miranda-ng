#ifndef _STEAM_FRIEND_H_
#define _STEAM_FRIEND_H_

namespace SteamWebApi
{
	class GetUserSummariesRequest : public HttpsGetRequest
	{
	public:
		GetUserSummariesRequest(const char *token, const char *steamIds) :
			HttpsGetRequest(STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001")
		{
			AddParameter("access_token", token);
			AddParameter("steamids", steamIds);
		}
	};
}

#endif //_STEAM_FRIEND_H_