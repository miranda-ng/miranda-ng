#ifndef _STEAM_REQUEST_FRIEND_LIST_H_
#define _STEAM_REQUEST_FRIEND_LIST_H_

struct AddFriendRequest : public HttpRequest
{
	AddFriendRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/AddFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%lld||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who);
	}
};

struct BlockFriendRequest : public HttpRequest
{
	BlockFriendRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%lld||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who) << CHAR_PARAM("action", "ignore");
	}
};

struct UnblockFriendRequest : public HttpRequest
{
	UnblockFriendRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/BlockUserAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%lld||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who) << CHAR_PARAM("action", "unignore") << INT_PARAM("block", 0);
	}
};

struct RemoveFriendRequest : public HttpRequest
{
	RemoveFriendRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/actions/RemoveFriendAjax")
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%lld||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("steamid", who);
	}
};

#endif //_STEAM_REQUEST_FRIEND_LIST_H_
