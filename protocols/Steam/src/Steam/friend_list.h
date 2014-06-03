#ifndef _STEAM_FRIEND_LIST_H_
#define _STEAM_FRIEND_LIST_H_

namespace SteamWebApi
{
	class FriendListApi : public BaseApi
	{
	public:

		enum FRIEND_TYPE
		{
			FRIEND_TYPE_NONE,
			FRIEND_TYPE_FRIEND,
			FRIEND_TYPE_IGNORED,
		};
		class FriendListItem
		{
			friend FriendListApi;

		private:
			std::string steamId;
			FRIEND_TYPE type;

		public:
			FriendListItem() : type(FRIEND_TYPE_NONE) { }

			const char *GetSteamId() const { return steamId.c_str(); }
			FRIEND_TYPE GetType() const { return type; }
		};

		class FriendList : public Result
		{
			friend FriendListApi;

		private:
			std::vector<FriendListItem*> items;

		public:
			size_t GetItemCount() const { return items.size(); }
			const FriendListItem * GetAt(size_t idx) const { return items.at(idx); }
		};

		static void Load(HANDLE hConnection, const char *token, const char *steamId, FriendList *friendList)
		{
			friendList->success = false;

			SecureHttpGetRequest request(hConnection, STEAM_API_URL "/ISteamUserOAuth/GetFriendList/v0001");
			request.AddParameter("access_token", token);
			request.AddParameter("steamid", steamId);
			request.AddParameter("relationship=friend,ignoredfriend,requestrecipient");
			//relationship = friend, requestrecipient

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
				for (size_t i = 0; i < json_size(root); i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					FriendListItem *item = new FriendListItem();

					node = json_get(child, "steamid");
					item->steamId = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "relationship");
					ptrA relationship(mir_u2a(json_as_string(node)));
					if (!lstrcmpiA(relationship, "friend"))
						item->type = FRIEND_TYPE_FRIEND;
					else if (!lstrcmpiA(relationship, "ignoredfriend"))
						item->type = FRIEND_TYPE_IGNORED;
					else if (!lstrcmpiA(relationship, "requestrecipient"))
						item->type = FRIEND_TYPE_NONE;
					else
					{
						continue;
					}

					friendList->items
						.push_back(item);
				}
			}

			friendList->success = true;
		}

		static void AddFriend(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *who, Result *result)
		{
			result->success = false;

			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char data[128];
			mir_snprintf(data, SIZEOF(data),
				"sessionID=%s&steamid=%s",
				sessionId,
				who);

			SecureHttpPostRequest request(hConnection, STEAM_COM_URL "/actions/AddFriendAjax");
			request.AddHeader("Cookie", cookie);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			JSONNODE *root = json_parse(response->pData), *node;

			//{"failed_invites":["xxxxxxxxxxxxxxxxx"], "failed_invites_result" : [41], "success" : 1}
			/*bool fail = false;
			node = json_get(root, "failed_invites_result");
			if (node)
				fail = true;*/

			node = json_get(root, "success");

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK || json_as_int(node) == 0/* || fail*/)
				return;

			result->success = true;
		}

		static void RemoveFriend(HANDLE hConnection, const char *token, const char *sessionId, const char *steamId, const char *who, Result *result)
		{
			result->success = false;

			char login[MAX_PATH];
			mir_snprintf(login, SIZEOF(login), "%s||oauth:%s", steamId, token);

			char cookie[MAX_PATH];
			mir_snprintf(cookie, SIZEOF(cookie), "steamLogin=%s;sessionid=%s;forceMobile=1", login, sessionId);

			char data[128];
			mir_snprintf(data, SIZEOF(data),
				"sessionID=%s&steamid=%s",
				sessionId,
				who);

			SecureHttpPostRequest request(hConnection, STEAM_COM_URL "/actions/RemoveFriendAjax");
			request.AddHeader("Cookie", cookie);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((result->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK || lstrcmpiA(response->pData, "true"))
				return;

			result->success = true;
		}
	};
}

#endif //_STEAM_FRIEND_LIST_H_