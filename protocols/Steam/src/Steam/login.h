#ifndef _STEAM_LOGIN_H_
#define _STEAM_LOGIN_H_

namespace SteamWebApi
{
	class LogonRequest : public HttpsPostRequest
	{
	public:
		LogonRequest(const char *token) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001")
		{
			char data[256];
			mir_snprintf(data, SIZEOF(data), "access_token=%s&ui_mode=web", token);

			SetData(data, strlen(data));
		}
	};

	class LogoffRequest : public HttpsPostRequest
	{
	public:
		LogoffRequest(const char *token, const char *umqId) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logoff/v0001")
		{
			char data[256];
			mir_snprintf(data, SIZEOF(data), "access_token=%s&umqid=%s", token, umqId);

			SetData(data, strlen(data));
		}
	};
}

#endif //_STEAM_LOGIN_H_