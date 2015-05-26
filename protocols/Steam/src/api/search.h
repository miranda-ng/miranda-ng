#ifndef _STEAM_REQUEST_SEARCH_H_
#define _STEAM_REQUEST_SEARCH_H_

class SearchRequest : public HttpRequest
{
public:
	SearchRequest(const char *token, const char *text) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/Search/v0001")
	{
		AddParameter("access_token", token);
		AddParameter("keywords", ptrA(mir_urlEncode(text)));
		// todo: may need to load all results (15 first at now)
		AddParameter("offset=0&count=15&targets=users&fields=all");
	}
};

#endif //_STEAM_REQUEST_SEARCH_H_
