#ifndef _STEAM_REQUEST_MESSAGE_H_
#define _STEAM_REQUEST_MESSAGE_H_

class SendMessageRequest : public HttpRequest
{
public:
	SendMessageRequest(const char *token, const char *umqId, const char *steamId, const char *text) :
		HttpRequest(REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001")
	{
		CMStringA data;
		data.AppendFormat("access_token=%s&umqid=%s&steamid_dst=%s&type=saytext&text=%s",
			token,
			umqId,
			steamId,
			ptrA(mir_urlEncode(text)));

		SetData(data, data.GetLength());
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_STEAM_REQUEST_MESSAGE_H_
