#ifndef _STEAM_SEARCH_H_
#define _STEAM_SEARCH_H_

namespace SteamWebApi
{
	class SearchApi : public BaseApi
	{
	public:

		class SearchItem : public Result
		{
		};

		class SearchResult : public Result
		{
			friend SearchApi;

		private:
			int count;
			std::vector<SearchItem*> items;

		public:
			SearchResult() : count(0) { }

			int GetCount() { return count; }
		};
		
		static void Search(HANDLE hConnection, const char *token, const char *text, SearchResult *searchResult)
		{
			searchResult->success = false;
			searchResult->count = 0;
			searchResult->items.clear();

			HttpRequest request(hConnection, REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/Search/v0001");
			request.AddParameter("access_token=%s", token);
			request.AddParameter("&keywords=%s", text);
			request.AddParameter("&offset=0&count=50&targets=users&fields=all");
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;
			
			JSONNODE *root = json_parse(response->pData), *node, *child;;

			node = json_get(root, "success");
			searchResult->success = json_as_bool(node) > 0;
			if (!searchResult->success)
				return;

			node = json_get(root, "count");
			searchResult->count = json_as_int(node);

			//node = json_get(root, "total");
			//searchResult->total = json_as_int(node);

			if (searchResult->count == 0)
				return;

			node = json_get(root, "results");
			root = json_as_array(node);
			if (root != NULL)
			{
				for (int i = 0;; i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					SearchItem *item = NULL;
				}
			}

			searchResult->success = true;
		}
	};
}

#endif //_STEAM_SEARCH_H_