#ifndef _STEAM_FRIEND_LIST_H_
#define _STEAM_FRIEND_LIST_H_

namespace SteamWebApi
{
	class FriendListApi : public BaseApi
	{
	public:
		class FriendList : public Result
		{
			friend FriendListApi;

		private:
			std::vector<std::string> items;

		public:
			size_t GetItemCount() const { return items.size(); }
			const char * GetAt(int idx) const { return items.at(idx).c_str(); }
		};

		static void Load(HANDLE hConnection, const char *token, const char *steamId, FriendList *friendList)
		{
			friendList->success = false;

			SecureHttpGetRequest request(hConnection, STEAM_API_URL "/ISteamUserOAuth/GetFriendList/v0001");
			request.AddParameter("access_token", token);
			request.AddParameter("steamid", steamId);

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((friendList->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node, *child;

			node = json_get(root, "friends");
			root = json_as_array(node);
			if (root != NULL)
			{
				for (int i = 0;; i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					node = json_get(child, "steamid");
					friendList->items
						.push_back((char*)ptrA(mir_u2a(json_as_string(node))));
				}
			}

			friendList->success = true;
		}

		static void AddFriend(HANDLE hConnection, const char *sessionId, const char *steamId, Result *result)
		{
			result->success = false;

			char data[128];
			mir_snprintf(data, SIZEOF(data),
				"steamid=%s&sessionID=%s",
				sessionId,
				steamId);

			SecureHttpPostRequest request(hConnection, STEAM_COM_URL "/actions/AddFriendAjax");

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			result->success = true;
		}

		static void RemoveFriend(HANDLE hConnection, const char *sessionId, const char *steamId, Result *result)
		{
			result->success = false;

			char data[128];
			mir_snprintf(data, SIZEOF(data),
				"steamid=%s&sessionID=%s",
				sessionId,
				steamId);

			SecureHttpPostRequest request(hConnection, STEAM_COM_URL "/actions/RemoveFriendAjax");

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			result->success = true;
		}
	};
}

#endif //_STEAM_FRIEND_LIST_H_