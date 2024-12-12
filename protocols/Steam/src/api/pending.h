#ifndef _STEAM_REQUEST_PENDING_H_
#define _STEAM_REQUEST_PENDING_H_

struct ApprovePendingRequest : public HttpRequest
{
	ApprovePendingRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, "")
	{
		m_szUrl.Format(STEAM_WEB_URL "/profiles/%lld/home_process", steamId);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s||oauth:%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios",
			steamId, token, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("id", who) << CHAR_PARAM("perform", "accept") 
			<< CHAR_PARAM("action", "approvePending") << CHAR_PARAM("itype", "friend") << INT_PARAM("json", 1) << INT_PARAM("xml", 0);
	}
};

struct IgnorePendingRequest : public HttpRequest
{
	IgnorePendingRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, "")
	{
		m_szUrl.Format(STEAM_WEB_URL "/profiles/%lld/home_process", steamId);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s||oauth:%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios",
			steamId, token, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("id", who) << CHAR_PARAM("perform", "ignore") 
			<< CHAR_PARAM("action", "approvePending") << CHAR_PARAM("itype", "friend") << INT_PARAM("json", 1) << INT_PARAM("xml", 0);
	}
};

struct BlockPendingRequest : public HttpRequest
{
	BlockPendingRequest(const char *token, const char *sessionId, int64_t steamId, const char *who) :
		HttpRequest(REQUEST_POST, "")
	{
		m_szUrl.Format(STEAM_WEB_URL "/profiles/%lld/home_process", steamId);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s||oauth:%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios",
			steamId, token, sessionId);

		AddHeader("Cookie", cookie);

		this << CHAR_PARAM("sessionID", sessionId) << CHAR_PARAM("id", who) << CHAR_PARAM("perform", "block")
			<< CHAR_PARAM("action", "approvePending") << CHAR_PARAM("itype", "friend") << INT_PARAM("json", 1) << INT_PARAM("xml", 0);
	}
};

#endif //_STEAM_REQUEST_PENDING_H_
