#ifndef _STEAM_REQUEST_FRIEND_LIST_H_
#define _STEAM_REQUEST_FRIEND_LIST_H_

class GetFriendListRequest : public HttpRequest
{
public:
	GetFriendListRequest(const char *token, const char *steamId, const char *relationship = "friend,ignoredfriend,requestrecipient") :
		HttpRequest(HttpGet, STEAM_API_URL "/ISteamUserOAuth/GetFriendList/v0001")
	{
		Uri
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("steamid", steamId)
			<< CHAR_PARAM("relationship", relationship);
	}

	//{
	//	"friends": [
	//		{
	//			"steamid": "XXXXXXXXXXXXXXXXX",
	//			"relationship" : "friend",
	//			"friend_since" : 1514314629
	//		}
	//	]
	//}

};

class AddFriendRequest : public HttpRequest
{
public:
	AddFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, STEAM_WEB_URL "/actions/AddFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("steamid", who);
	}

	// "true"

	// {"invited":["XXXXXXXXXXXXXXXXX"], "success" : 1}

	// {"failed_invites":["XXXXXXXXXXXXXXXXX"], "failed_invites_result" : [24], "success" : 1}
};

class BlockFriendRequest : public HttpRequest
{
public:
	BlockFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("steamid", who)
			<< CHAR_PARAM("action", "ignore");
	}
};

class UnblockFriendRequest : public HttpRequest
{
public:
	UnblockFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("steamid", who)
			<< CHAR_PARAM("action", "unignore")
			<< INT_PARAM("block", 0);
	}
};

class RemoveFriendRequest : public HttpRequest
{
public:
	RemoveFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, STEAM_WEB_URL "/actions/RemoveFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("steamid", who);
	}
};

#endif //_STEAM_REQUEST_FRIEND_LIST_H_
