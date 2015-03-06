#ifndef _STEAM_POLL_H_
#define _STEAM_POLL_H_

namespace SteamWebApi
{
	class PollRequest : public HttpsPostRequest
	{
	public:
		PollRequest(const char *token, const char *umqId, UINT32 messageId, int idleSeconds) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Poll/v0001")
		{
			timeout = 30000;
			flags |= NLHRF_PERSISTENT;

			CMStringA data;
			data.AppendFormat("access_token=%s&umqid=%s&message=%u&secidletime=%d",
				token,
				umqId,
				messageId,
				idleSeconds);

			SetData(data, data.GetLength());
			
			AddHeader("Connection", "keep-alive");
		}
	};
}

#endif //_STEAM_POLL_H_