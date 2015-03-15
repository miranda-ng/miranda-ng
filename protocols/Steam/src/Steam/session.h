#ifndef _STEAM_SESSION_H_
#define _STEAM_SESSION_H_

namespace SteamWebApi
{
	class GetSessionRequest : public HttpsPostRequest
	{
	public:
		GetSessionRequest(const char *token, const char *steamId, const char *cookie) :
			HttpsPostRequest(STEAM_WEB_URL "/mobileloginsucceeded")
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