#ifndef _STEAM_LOGIN_H_
#define _STEAM_LOGIN_H_

namespace SteamWebApi
{
	class LogInApi : public BaseApi
	{
	public:
		class LogIn : public Result
		{
			friend LogInApi;

		private:
			std::string steamid;
			std::string umqid;
			std::string message;

		public:

			const char *GetSteamId() { return steamid.c_str(); }
			const char *GetSessionId() { return umqid.c_str(); }
			const char *GetMessageId() { return message.c_str(); }
		};
		
		static void LogOn(HANDLE hConnection, const char *token, LogIn *login)
		{
			login->success = false;

			CMStringA data;
			data.AppendFormat("access_token=%s", token);

			HttpRequest request(hConnection, REQUEST_POST, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;
			
			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "error");
			ptrW error(json_as_string(node));
			if (lstrcmpi(error, L"OK"))
				return;

			node = json_get(root, "steamid");
			login->steamid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "umqid");
			login->umqid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "message");
			login->message = ptrA(mir_u2a(json_as_string(node)));

			login->success = true;
		}
	};
}


#endif //_STEAM_LOGIN_H_