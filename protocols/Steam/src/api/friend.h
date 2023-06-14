#ifndef _STEAM_REQUEST_FRIEND_H_
#define _STEAM_REQUEST_FRIEND_H_

class GetUserSummariesRequest : public HttpRequest
{
public:
	GetUserSummariesRequest(CSteamProto *ppro, const char *steamIds) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0002")
	{
		this << CHAR_PARAM("access_token", ppro->getMStringA("TokenSecret")) << CHAR_PARAM("steamids", steamIds);
	}

	//{
	//	"players": [
	//		{
	//			"steamid": "76561197960435530",
	//			"communityvisibilitystate" : 3,
	//			"profilestate" : 1,
	//			"personaname" : "Robin",
	//			"lastlogoff" : 1514966746,
	//			"profileurl" : "http://steamcommunity.com/id/robinwalker/",
	//			"avatar" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/f1/f1dd60a188883caf82d0cbfccfe6aba0af1732d4.jpg",
	//			"avatarmedium" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/f1/f1dd60a188883caf82d0cbfccfe6aba0af1732d4_medium.jpg",
	//			"avatarfull" : "https://steamcdn-a.akamaihd.net/steamcommunity/public/images/avatars/f1/f1dd60a188883caf82d0cbfccfe6aba0af1732d4_full.jpg",
	//			"personastate" : 0,
	//			"realname" : "Robin Walker",
	//			"primaryclanid" : "103582791429521412",
	//			"timecreated" : 1063407589,
	//			"personastateflags" : 0,
	//			"loccountrycode" : "US",
	//			"locstatecode" : "WA",
	//			"loccityid" : 3961
	//		}
	//	]
	//}
};

#endif //_STEAM_REQUEST_FRIEND_H_
