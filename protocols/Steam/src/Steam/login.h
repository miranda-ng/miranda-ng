#ifndef _STEAM_LOGIN_H_
#define _STEAM_LOGIN_H_

namespace SteamWebApi
{
	class LoginApi : public BaseApi
	{
	public:
		class LoginResult : public Result
		{
			friend LoginApi;

		private:
			std::string steamid;
			std::string umqid;
			UINT32 messageId;

		public:

			const char *GetSteamId() { return steamid.c_str(); }
			const char *GetUmqId() { return umqid.c_str(); }
			UINT32 GetMessageId() { return messageId; }
		};
		
		static void Logon(HANDLE hConnection, const char *token, LoginResult *loginResult)
		{
			loginResult->success = false;

			char data[256];
			mir_snprintf(data, SIZEOF(data), "access_token=%s", token);

			SecureHttpPostRequest request(hConnection, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001");
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((loginResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "error");
			ptrW error(json_as_string(node));
			if (lstrcmpi(error, L"OK"))
				return;

			node = json_get(root, "steamid");
			loginResult->steamid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "umqid");
			loginResult->umqid = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "message");
			loginResult->messageId = json_as_int(node);

			loginResult->success = true;
		}

		static void Logoff(HANDLE hConnection, const char *token, const char *umqId)
		{
			CMStringA data;
			data.AppendFormat("access_token=%s", token);
			data.AppendFormat("&umqid=%s", umqId);

			SecureHttpPostRequest request(hConnection, STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logoff/v0001");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
		}
	};

	class LogonRequest : public HttpsPostRequest
	{
	public:
		LogonRequest(const char *token) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logon/v0001")
		{
			char data[256];
			mir_snprintf(data, SIZEOF(data), "access_token=%s", token);

			SetData(data, strlen(data));
		}
	};

	class LogoffRequest : public HttpsPostRequest
	{
	public:
		LogoffRequest(const char *token, const char *umqId) :
			HttpsPostRequest(STEAM_API_URL "/ISteamWebUserPresenceOAuth/Logoff/v0001")
		{
			char data[256];
			mir_snprintf(data, SIZEOF(data), "access_token=%s&umqid=%s", token, umqId);

			SetData(data, strlen(data));
		}
	};
}

#endif //_STEAM_LOGIN_H_