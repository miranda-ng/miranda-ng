#ifndef _STEAM_REQUEST_HISTORY_H_
#define _STEAM_REQUEST_HISTORY_H_

struct GetHistoryMessagesRequest : public HttpRequest
{
	GetHistoryMessagesRequest(const char *token, int64_t steamId, int64_t who, time_t since) :
		HttpRequest(REQUEST_GET, "/IFriendMessagesService/GetRecentMessages/v0001")
	{
		this
			<< CHAR_PARAM("access_token", token)
			<< INT64_PARAM("steamid1", steamId)
			<< INT64_PARAM("steamid2", who)
			// Steam somehow doesn't respect too precise start time parameter, so we better request older time and then do own filtering again
			<< INT64_PARAM("rtime32_start_time", since - 1500);
	}
};

#endif //_STEAM_REQUEST_HISTORY_H_
