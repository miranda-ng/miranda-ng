#ifndef _STEAM_PENDING_H_
#define _STEAM_PENDING_H_

namespace SteamWebApi
{
	class ApprovePendingRequest : public HttpsPostRequest
	{
	public:
		ApprovePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
			HttpsPostRequest(STEAM_WEB_URL "/profiles/%s/home_process", steamId)
		{
			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=accept&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SetData(data, strlen(data));
			AddHeader("Cookie", cookie);
		}
	};

	class IgnorePendingRequest : public HttpsPostRequest
	{
	public:
		IgnorePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
			HttpsPostRequest(STEAM_WEB_URL "/profiles/%s/home_process", steamId)
		{
			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=ignore&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SetData(data, strlen(data));
			AddHeader("Cookie", cookie);
		}
	};

	class BlockPendingRequest : public HttpsPostRequest
	{
	public:
		BlockPendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
			HttpsPostRequest(STEAM_WEB_URL "/profiles/%s/home_process", steamId)
		{
			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=block&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SetData(data, strlen(data));
			AddHeader("Cookie", cookie);
		}
	};
}

#endif //_STEAM_PENDING_H_