#ifndef _STEAM_REQUEST_SEARCH_H_
#define _STEAM_REQUEST_SEARCH_H_

class SearchRequest : public HttpRequest
{
public:
	SearchRequest(const char *token, const char *text, int offset = 0, int count = 30) :
		HttpRequest(HttpGet, STEAM_API_URL "/ISteamUserOAuth/Search/v0001")
	{
		Uri
			<< CHAR_PARAM("access_token", token)
			<< CHAR_PARAM("keywords", text)
			<< INT_PARAM("offset=%d", offset)
			<< INT_PARAM("count=%d", count)
			<< CHAR_PARAM("targets", "users")
			<< CHAR_PARAM("fields", "all");
	}
};

#endif //_STEAM_REQUEST_SEARCH_H_
