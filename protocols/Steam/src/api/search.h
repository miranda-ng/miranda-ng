#ifndef _STEAM_REQUEST_SEARCH_H_
#define _STEAM_REQUEST_SEARCH_H_

class SearchRequest : public HttpRequest
{
public:
	SearchRequest(const char *token, const char *text, int offset = 0, int count = 30) :
		HttpRequest(REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/Search/v0001")
	{
		AddParameter("access_token", token);
		AddParameter("keywords", ptrA(mir_urlEncode(text)));
		AddParameter("offset=%d", offset);
		AddParameter("count=%d", count);
		AddParameter("targets=users&fields=all");
	}
};

#endif //_STEAM_REQUEST_SEARCH_H_
