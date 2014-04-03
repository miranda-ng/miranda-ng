#ifndef _STEAM_AUTHORIZATION_H_
#define _STEAM_AUTHORIZATION_H_

namespace SteamWebApi
{
	class AuthorizationApi : public BaseApi
	{
	public:

		class Authorization : public Result
		{
			friend AuthorizationApi;

		private:
			std::string steamid;
			std::string token;

			std::string emailauth;
			std::string emailsteamid;

			bool emailauth_needed;

		public:
			bool IsEmailAuthNeeded() { return emailauth_needed; }
			const char *GetSteamid() { return steamid.c_str(); }
			const char *GetToken() { return token.c_str(); }
			const char *GetAuthId() { return emailauth.c_str(); }
			const char *GetAuthCode() { return emailsteamid.c_str(); }

			void SetAuthCode(char *code)
			{
				emailauth = code;
			}
		};

		static void Authorize(HANDLE hConnection, const wchar_t *username, const char *password, Authorization *auth)
		{
			auth->success = false;

			ptrA base64Username(mir_urlEncode(ptrA(mir_utf8encodeW(username))));

			CryptoApi::RsaKey rsaKey;
			CryptoApi::GetRsaKey(hConnection, base64Username, &rsaKey);
			if (!rsaKey.IsSuccess()) return;

			int size = rsaKey.GetEncryptedSize();
			BYTE *rsaEncryptedPassword = (BYTE*)mir_alloc(size);
			rsaKey.Encrypt((unsigned char*)password, strlen(password), rsaEncryptedPassword);
			ptrA base64RsaEncryptedPassword(mir_base64_encode(rsaEncryptedPassword, size));
			mir_free(rsaEncryptedPassword);

			CMStringA data;
			data.AppendFormat("username=%s", base64Username);
			data.AppendFormat("&password=%s", ptrA(mir_urlEncode(base64RsaEncryptedPassword)));
			data.AppendFormat("&emailauth=%s", ptrA(mir_urlEncode(auth->emailauth.c_str())));
			data.AppendFormat("&emailsteamid=%s", auth->emailsteamid.c_str());
			//data.AppendFormat("&captchagid=%s", result->captchagid);
			//data.AppendFormat("&captcha_text=%s", ptrA(mir_urlEncode(result->captcha_text)));
			data.Append("&captchagid=-1");
			data.AppendFormat("&rsatimestamp=%llu", rsaKey.GetTimestamp());
			data.AppendFormat("&oauth_scope=%s", "read_profile%20write_profile%20read_client%20write_client");
			data.Append("&oauth_client_id=3638BFB1");

			HttpRequest request(hConnection, REQUEST_POST, "https://steamcommunity.com/mobilelogin/dologin");
			request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
			request.SetData(data.GetBuffer(), data.GetLength());
			
			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response || response->resultCode != HTTP_STATUS_OK)
				return;
			
			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "success");
			auth->success = json_as_bool(node) > 0;
			if (!auth->success)
			{
				node = json_get(root, "emailauth_needed");
				auth->emailauth_needed = json_as_bool(node) > 0;
				if (auth->emailauth_needed)
				{
					node = json_get(root, "emailsteamid");
					auth->emailsteamid = ptrA(mir_u2a(json_as_string(node)));
					auth->emailauth_needed = false;

					/*node = json_get(root, "emaildomain");
					result->emaildomain = json_as_string(node);*/
				}

				/*node = json_get(root, "captcha_needed");
				result->captcha_needed = json_as_bool(node) > 0;
				if (result->captcha_needed)
				{
					node = json_get(root, "captcha_gid");
					result->captchagid = json_as_string(node);
				}*/
			}
			else
			{
				node = json_get(root, "login_complete");
				if (!json_as_bool(node))
					return;

				node = json_get(root, "oauth");
				CMStringA oauth = mir_u2a(json_as_string(node));
				oauth.Replace("\\\"", "\"");
				root = json_parse(oauth.GetBuffer());
				//root = json_as_node(node);

				node = json_get(root, "steamid");
				auth->steamid = ptrA(mir_u2a(json_as_string(node)));

				node = json_get(root, "oauth_token");
				auth->token = ptrA(mir_u2a(json_as_string(node)));

				auth->success = true;
			}
		}
	};
}


#endif //_STEAM_AUTHORIZATION_H_