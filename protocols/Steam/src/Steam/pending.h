#ifndef _STEAM_PENDING_H_
#define _STEAM_PENDING_H_

namespace SteamWebApi
{
	class PendingApi : public BaseApi
	{
	public:
		static void Accept(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *who, Result *result)
		{
			result->success = false;

			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/profiles/%s/home_process", steamId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=accept&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SecureHttpPostRequest request(hConnection, url);
			request.AddHeader("Cookie", cookie);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			//if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_FOUND)
			//	return;

			result->success = true;
		}

		static void Ignore(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *who, Result *result)
		{
			result->success = false;

			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/profiles/%s/home_process", steamId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=ignore&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SecureHttpPostRequest request(hConnection, url);
			request.AddHeader("Cookie", cookie);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			result->success = true;
		}

		static void Block(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *who, Result *result)
		{
			result->success = false;

			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/profiles/%s/home_process", steamId);

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=block&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SecureHttpPostRequest request(hConnection, url);
			request.AddHeader("Cookie", cookie);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			result->success = true;
		}
	};

	class ApprovePendingRequest : public HttpsPostRequest
	{
	public:
		ApprovePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
			HttpsPostRequest(STEAM_COM_URL "/profiles/%s/home_process")
		{
			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/profiles/%s/home_process", steamId);
			this->url = url;

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
			HttpsPostRequest(STEAM_COM_URL "/profiles/%s/home_process")
		{
			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char url[MAX_PATH];
			mir_snprintf(url, SIZEOF(url), STEAM_COM_URL "/profiles/%s/home_process", steamId);
			this->url = url;

			char data[MAX_PATH];
			mir_snprintf(data, SIZEOF(data), "sessionID=%s&id=%s&perform=ignore&action=approvePending&itype=friend&json=1&xml=0", sessionId, who);

			SetData(data, strlen(data));
			AddHeader("Cookie", cookie);
		}
	};
}

#endif //_STEAM_PENDING_H_