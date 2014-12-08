#ifndef _STEAM_STATUS_H_
#define _STEAM_STATUS_H_

namespace SteamWebApi
{
	class SetStatusRequest : public HttpsPostRequest
	{
	public:
		SetStatusRequest(const char *token, const char *umqId, int state) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Message/v0001")
		{
			CMStringA data;
			data.AppendFormat("access_token=%s&umqid=%s&type=personastate&persona_state=%d",
				token,
				umqId,
				state);

			SetData(data, data.GetLength());
		}
	};
}


#endif //_STEAM_STATUS_H_