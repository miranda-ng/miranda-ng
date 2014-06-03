#ifndef _STEAM_FRIEND_H_
#define _STEAM_FRIEND_H_

namespace SteamWebApi
{
	class FriendApi : public BaseApi
	{
	public:
		struct Summary : public Result
		{
			friend FriendApi;
		
		private:
			std::string steamId;

			std::wstring nickname;
			std::wstring firstname;
			std::wstring lastname;
			std::wstring secondname;
			std::string countryCode;
			std::string homepage;
			std::string avatarUrl;
			std::wstring gameInfo;
			UINT32 gameId;

			int state;

			DWORD created;
			DWORD lastEvent;

		public:
			Summary() : gameId(0), created(0), lastEvent(0) { }

			const char *GetSteamId() const { return steamId.c_str(); }
			const wchar_t *GetNickName() const { return nickname.c_str(); }
			const char *GetCountryCode() const { return countryCode.c_str(); }
			const char *GetHomepage() const { return homepage.c_str(); }
			const char *GetAvatarUrl() const { return avatarUrl.c_str(); }
			const wchar_t *GetGameInfo() const { return gameInfo.c_str(); }
			const DWORD GetGameId() const { return gameId; }
			int GetState() const { return state; }
			const DWORD GetCreated() const { return created; }
			const DWORD GetLastEvent() const { return lastEvent; }

			const wchar_t *GetFirstName() const
			{
				return firstname.c_str();
			}

			const wchar_t *GetLastName() const
			{
				return lastname.c_str();
			}
		};

		struct Summaries : public Result
		{
			friend FriendApi;

		private:
			std::vector<Summary*> items;

		public:
			size_t GetItemCount() const { return items.size(); }
			const Summary *GetAt(size_t idx) const { return items.at(idx); }
		};

		static void LoadSummaries(HANDLE hConnection, const char *token, const char *steamIds, Summaries *summaries)
		{
			summaries->success = false;

			SecureHttpGetRequest request(hConnection, STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001");
			request.AddParameter("access_token", token);
			request.AddParameter("steamids", steamIds);

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((summaries->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node, *child;

			node = json_get(root, "players");
			root = json_as_array(node);
			if (root != NULL)
			{
				for (size_t i = 0; i < json_size(root); i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					Summary *item = new Summary();

					node = json_get(child, "steamid");
					item->steamId = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "personaname");
					item->nickname = json_as_string(node);

					node = json_get(child, "realname");
					if (node != NULL)
					{
						std::wstring realname = json_as_string(node);
						if (!realname.empty())
						{
							size_t pos = realname.find(' ', 1);
							if (pos > 0)
							{
								item->firstname = realname.substr(0, pos);
								item->lastname = realname.substr(pos + 1).c_str();
							}
						}
					}

					node = json_get(child, "loccountrycode");
					if (node != NULL)
						item->countryCode = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "personastate");
					item->state = json_as_int(node);

					node = json_get(child, "profileurl");
					item->homepage = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "timecreated");
					item->created = json_as_int(node);

					node = json_get(child, "lastlogoff");
					item->lastEvent = json_as_int(node);

					node = json_get(child, "avatarfull");
					item->avatarUrl = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "gameextrainfo");
					item->gameInfo = json_as_string(node);

					node = json_get(child, "gameid");
					item->gameId = atol(ptrA(mir_u2a(json_as_string(node))));

					summaries->items.push_back(item);
				}
			}
			else
				return;

			summaries->success = true;
		}
	};

	class GetUserSummariesRequest : public HttpsGetRequest
	{
	public:
		GetUserSummariesRequest(const char *token, const char *steamIds) :
			HttpsGetRequest(STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001")
		{
			AddParameter("access_token", token);
			AddParameter("steamids", steamIds);
		}
	};
}

#endif //_STEAM_FRIEND_H_