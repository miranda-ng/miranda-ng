#ifndef _STEAM_REQUEST_PENDING_H_
#define _STEAM_REQUEST_PENDING_H_

class ApprovePendingRequest : public HttpRequest
{
public:
	ApprovePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[MAX_PATH];
		mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=accept&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class IgnorePendingRequest : public HttpRequest
{
public:
	IgnorePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[MAX_PATH];
		mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=ignore&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class BlockPendingRequest : public HttpRequest
{
public:
	BlockPendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(REQUEST_POST, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

		char data[MAX_PATH];
		mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=block&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

		SetData(data, strlen(data));
		AddHeader("Cookie", cookie);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_STEAM_REQUEST_PENDING_H_
