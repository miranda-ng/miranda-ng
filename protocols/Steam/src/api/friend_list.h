#ifndef _STEAM_REQUEST_FRIEND_LIST_H_
#define _STEAM_REQUEST_FRIEND_LIST_H_

class GetFriendListRequest : public HttpRequest
{
public:
	GetFriendListRequest(const char *token, int64_t steamId, const char *relationship) :
		HttpRequest(REQUEST_GET, "/ISteamUserOAuth/GetFriendList/v0001")
	{
		this << CHAR_PARAM("access_token", token) << INT64_PARAM("steamid", steamId) << CHAR_PARAM("relationship", relationship);
	}
};

class AddFriendRequest : public HttpRequest
{
public:
	AddFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/AddFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who);
	}
};

class BlockFriendRequest : public HttpRequest
{
public:
	BlockFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who) << CHAR_PARAM("action", "ignore");
	}
};

class UnblockFriendRequest : public HttpRequest
{
public:
	UnblockFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who) << CHAR_PARAM("action", "unignore") << INT_PARAM("block", 0);
	}
};

class RemoveFriendRequest : public HttpRequest
{
public:
	RemoveFriendRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/RemoveFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who);
	}
};

#endif //_STEAM_REQUEST_FRIEND_LIST_H_
