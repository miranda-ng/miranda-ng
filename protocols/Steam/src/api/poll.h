#ifndef _STEAM_REQUEST_POLL_H_
#define _STEAM_REQUEST_POLL_H_

class PollRequest : public HttpRequest
{
public:
	PollRequest(const char *token, const char *umqId, UINT32 messageId, int idleSeconds) :
		HttpRequest(HttpPost, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Poll/v0001")
	{
		timeout = (STEAM_API_TIMEOUT + 5) * 1000;
		// flags |= NLHRF_PERSISTENT;

		Headers << CHAR_PARAM("Connection", "keep-alive");

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("umqid", umqId)
			<< INT64_PARAM("message", messageId)
			<< INT_PARAM("secidletime", idleSeconds)
			<< INT_PARAM("sectimeout", STEAM_API_TIMEOUT);
	}
};

#endif //_STEAM_REQUEST_POLL_H_
