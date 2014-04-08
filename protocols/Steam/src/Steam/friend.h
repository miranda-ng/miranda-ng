#ifndef _STEAM_FRIEND_H_
#define _STEAM_FRIEND_H_

namespace SteamWebApi
{
	class FriendApi : public BaseApi
	{
	public:
		struct Friend : public Result
		{
			friend FriendApi;
		
		private:
			std::string steamId;

			std::wstring nickname;
			std::wstring realname;
			std::string countryCode;
			std::string homepage;
			std::string avatarUrl;

			int state;

			DWORD created;
			DWORD lastEvent;

		public:
			const char *GetSteamId() const { return steamId.c_str(); }
			const wchar_t *GetNickname() const { return nickname.c_str(); }
			const wchar_t *GetRealname() const { return realname.c_str(); }
			const char *GetCountryCode() const { return countryCode.c_str(); }
			const char *GetHomepage() const { return homepage.c_str(); }
			const char *GetAvatarUrl() const { return avatarUrl.c_str(); }
			int GetState() const { return state; }
			const DWORD GetCreated() const { return created; }
			const DWORD GetLastEvent() const { return lastEvent; }
		};

		static void LoadSummaries(HANDLE hConnection, const char *token, const char *steamId, Friend *result)
		{
			result->success = false;

			HttpRequest *request = new HttpRequest(hConnection, REQUEST_GET, STEAM_API_URL "/ISteamUserOAuth/GetUserSummaries/v0001");
			request->AddParameter("access_token", token);
			request->AddParameter("steamids", steamId);

			mir_ptr<NETLIBHTTPREQUEST> response(request->Send());
			delete request;

			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node, *child;

			node = json_get(root, "players");
			root = json_as_array(node);
			if (root != NULL)
			{
				for (int i = 0;; i++)
				{
					child = json_at(root, i);
					if (child == NULL)
						break;

					node = json_get(child, "steamid");
					ptrA cSteamId(ptrA(mir_u2a(json_as_string(node))));
					if (lstrcmpA(steamId, cSteamId))
						return;
					result->steamId = steamId;

					node = json_get(child, "personaname");
					result->nickname = json_as_string(node);

					node = json_get(child, "realname");
					if (node != NULL)
						result->realname = json_as_string(node);

					node = json_get(child, "loccountrycode");
					if (node != NULL)
						result->countryCode = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "personastate");
					result->state = json_as_int(node);

					node = json_get(child, "profileurl");
					result->homepage = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(child, "timecreated");
					result->created = json_as_int(node);

					node = json_get(child, "lastlogoff");
					result->lastEvent = json_as_int(node);

					node = json_get(child, "avatarfull");
					result->avatarUrl = ptrA(mir_u2a(json_as_string(node)));
				}
			}
			else
				return;

			result->success = true;
		}
	};
}

#endif //_STEAM_FRIEND_H_