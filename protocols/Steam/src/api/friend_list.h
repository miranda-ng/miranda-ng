#ifndef _STEAM_REQUEST_FRIEND_LIST_H_
#define _STEAM_REQUEST_FRIEND_LIST_H_

class GetFriendListRequest : public HttpRequest
{
public:
	GetFriendListRequest(const char *token, const char *steamId, const char *relationship = "friend,ignoredfriend,requestrecipient") :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/GetFriendList/v0001")
	{
		AddParameter("access_token", token);
		AddParameter("steamid", steamId);
		AddParameter("relationship", relationship);
	}
};

class AddFriendRequest : public HttpRequest
{
public:
	AddFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/AddFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[128];
		mir_snprintf(data, SIZEOF(data),
			"sessionID=%s&steamid=%s",
			sessionId,
			who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class BlockFriendRequest : public HttpRequest
{
public:
	BlockFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[128];
		mir_snprintf(data, SIZEOF(data),
			"sessionID=%s&action=ignore&steamid=%s",
			sessionId,
			who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class RemoveFriendRequest : public HttpRequest
{
public:
	RemoveFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/RemoveFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[128];
		mir_snprintf(data, SIZEOF(data),
			"sessionID=%s&steamid=%s",
			sessionId,
			who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_STEAM_REQUEST_FRIEND_LIST_H_
