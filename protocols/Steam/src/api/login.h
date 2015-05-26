#ifndef _STEAM_REQUEST_LOGIN_H_
#define _STEAM_REQUEST_LOGIN_H_

class LogonRequest : public HttpRequest
{
public:
	LogonRequest(const char *token) :
		HttpRequest(REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001")
	{
		char data[256];
		mir_snprintf(data, SIZEOF(data), "access_token=%s&ui_mode=web", token);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class LogoffRequest : public HttpRequest
{
public:
	LogoffRequest(const char *token, const char *umqId) :
		HttpRequest(REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logoff/v0001")
	{
		char data[256];
		mir_snprintf(data, SIZEOF(data), "access_token=%s&umqid=%s", token, umqId);

		SetData(data, strlen(data));
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_STEAM_REQUEST_LOGIN_H_
