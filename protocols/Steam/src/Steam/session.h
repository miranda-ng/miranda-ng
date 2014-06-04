#ifndef _STEAM_SESSION_H_
#define _STEAM_SESSION_H_

namespace SteamWebApi
{
	//class SessionApi : public BaseApi
	//{
	//public:
	//	class SessionId : public Result
	//	{
	//		friend SessionApi;

	//	private:
	//		std::string sessionid;

	//	public:

	//		const char *GetSessionId() { return sessionid.c_str(); }
	//	};

	//	static void GetSessionId(HANDLE hConnection, const char *token, const char *steamId, SessionId *sessionId)
	//	{
	//		sessionId->success = false;

	//		char login[MAX_PATH];
	//		mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

	//		char cookie[MAX_PATH];
	//		mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s", ptrA(mir_urlEncode(login)));

	//		SecureHttpGetRequest request(hConnection, STEAM_COM_URL "/mobilesettings/GetManifest/v0001");
	//		request.AddHeader("Cookie", cookie);

	//		mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
	//		if (!response)
	//			return;

	//		for (int i = 0; i < response->headersCount; i++)
	//		{
	//			if (lstrcmpiA(response->headers[i].szName, "Set-Cookie"))
	//				continue;

	//			std::string cookies = response->headers[i].szValue;
	//			size_t start = cookies.find("sessionid=") + 10;
	//			size_t end = cookies.substr(start).find(';');
	//			sessionId->sessionid = cookies.substr(start, end - start + 10);
	//			break;
	//		}

	//		sessionId->success = true;
	//	}
	//};

	class GetSessionRequest : public HttpsPostRequest
	{
	public:
		GetSessionRequest(const char *token, const char *steamId, const char *cookie) :
			HttpsPostRequest(STEAM_COM_URL "/mobileloginsucceeded")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			char data[512];
			mir_snprintf(data, SIZEOF(data),
				"oauth_token=%s&steamid=%s&webcookie=%s",
				token,
				steamId,
				cookie);

			SetData(data, strlen(data));
		}
	};
}

#endif //_STEAM_SESSION_H_