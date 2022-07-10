#ifndef _STEAM_REQUEST_POLL_H_
#define _STEAM_REQUEST_POLL_H_

class PollRequest : public HttpRequest
{
public:
	PollRequest(CSteamProto *ppro) :
		HttpRequest(HttpPost, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Poll/v0001")
	{
		timeout = (STEAM_API_TIMEOUT + 5) * 1000;
		// flags |= NLHRF_PERSISTENT;

		Headers << CHAR_PARAM("Connection", "keep-alive");

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("access_token", ppro->getMStringA("TokenSecret"))
			<< CHAR_PARAM("umqid", ppro->getMStringA("UMQID"))
			<< INT64_PARAM("message", ppro->getDword("MessageID"))
			<< INT_PARAM("secidletime", ppro->IdleSeconds());
	}

	//{
	//	"pollid": 0,
	//	"sectimeout" : 30,
	//	"error" : "Timeout"
	//}

	//{
	//	"pollid": 0,
	//	"messages": [
	//		{
	//			"type": "typing",
	//			"timestamp": 17276041,
	//			"utc_timestamp": 1514974857,
	//			"steamid_from": "XXXXXXXXXXXXXXXXX",
	//			"text": ""
	//		},
	//		{
	//			"type": "saytext",
	//			"timestamp" : 17380133,
	//			"utc_timestamp" : 1514974961,
	//			"steamid_from" : "XXXXXXXXXXXXXXXXX",
	//			"text" : "message"
	//		},
	//		{
	//			"type": "personarelationship",
	//			"timestamp" : 7732750,
	//			"utc_timestamp" : 1515187192,
	//			"steamid_from" : "XXXXXXXXXXXXXXXXX",
	//			"status_flags" : 1,
	//			"persona_state" : 2
	//		},
	//		{
	//			"type": "personastate",
	//			"timestamp" : 366860,
	//			"utc_timestamp" : 1515007523,
	//			"steamid_from" : "XXXXXXXXXXXXXXXXX",
	//			"status_flags" : 9055,
	//			"persona_state" : 1,
	//			"persona_name" : "nickname"
	//		},
	//		{
	//			"type": "notificationcountupdate",
	//			"timestamp" : 11605105,
	//			"utc_timestamp" : 1515191064
	//		}
	//	]
	//	,
	//	"messagelast": 4,
	//	"timestamp": 17276041,
	//	"utc_timestamp": 1514974857,
	//	"messagebase": 3,
	//	"sectimeout": 14,
	//	"error": "OK"
	//}

};

#endif //_STEAM_REQUEST_POLL_H_
