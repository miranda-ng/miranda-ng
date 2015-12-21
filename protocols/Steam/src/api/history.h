#ifndef _STEAM_REQUEST_HISTORY_H_
#define _STEAM_REQUEST_HISTORY_H_

class GetConversationsRequest : public HttpRequest
{
public:
	GetConversationsRequest(const char *token) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/IFriendMessagesService/GetActiveMessageSessions/v0001")
	{
		AddParameter("access_token", token);
	}
};

class GetHistoryMessagesRequest : public HttpRequest
{
public:
	GetHistoryMessagesRequest(const char *token, const char *steamId, const char *who, time_t since) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/IFriendMessagesService/GetRecentMessages/v0001")
	{
		AddParameter("access_token", token);
		AddParameter("steamid1", steamId);
		AddParameter("steamid2", who);
		AddParameter("rtime32_start_time=%d", since);
	}
};

#endif //_STEAM_REQUEST_HISTORY_H_
