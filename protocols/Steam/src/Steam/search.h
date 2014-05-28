#ifndef _STEAM_SEARCH_H_
#define _STEAM_SEARCH_H_

namespace SteamWebApi
{
	class SearchApi : public BaseApi
	{
	public:

		class SearchItem// : public Result
		{
			friend SearchApi;

		private:
				std::string steamId;

		public:
			const char *GetSteamId() const { return steamId.c_str(); }
		};

		class SearchResult : public Result
		{
			friend SearchApi;

		private:
			int count;
			std::vector<SearchItem*> items;

		public:
			SearchResult() : count(0) { }

			int GetItemCount() { return count; }
			const SearchItem *GetAt(int idx) const { return items.at(idx); }
		};
		
		static void Search(HANDLE hConnection, const char *token, const char *text, SearchResult *searchResult)
		{
			searchResult->success = false;
			searchResult->count = 0;
			searchResult->items.clear();

			// todo: may need to load all results
			// 15 first at now
			SecureHttpGetRequest request(hConnection, STEAM_API_URL "/ISteamUserOAuth/Search/v0001");
			request.AddParameter("access_token", token);
			request.AddParameter("keywords", ptrA(mir_urlEncode(text)));
			request.AddParameter("offset=0&count=15&targets=users&fields=all");

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((searchResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node, *child;

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
				for (size_t i = 0; i < json_size(root); i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					SearchItem *item = new SearchItem();

					node = json_get(child, "steamid");
					item->steamId = ptrA(mir_u2a(json_as_string(node)));

					searchResult->items.push_back(item);
				}
			}

			searchResult->success = true;
		}
	};
}

#endif //_STEAM_SEARCH_H_