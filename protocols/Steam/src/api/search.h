#ifndef _STEAM_REQUEST_SEARCH_H_
#define _STEAM_REQUEST_SEARCH_H_

class SearchRequest : public HttpRequest
{
public:
	SearchRequest(const char *token, const char *text, int offset = 0, int count = 30) :
		HttpRequest(REQUEST_GET, "/ISteamUserOAuth/Search/v0001")
	{
		this
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("keywords", text)
			<< INT_PARAM("offset", offset)
			<< INT_PARAM("count", count)
			<< CHAR_PARAM("targets", "users")
			<< CHAR_PARAM("fields", "all");
	}

	//{
	//	"count": 1,
	//	"total" : 336,
	//	"success" : true,
	//	"results" : [
	//		{
	//			"steamid": "XXXXXXXXXXXXXXXXX",
	//			"type" : "user"
	//		}
	//	]
	//}
};

#endif //_STEAM_REQUEST_SEARCH_H_
