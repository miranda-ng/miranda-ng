#ifndef _STEAM_AUTHORIZATION_H_
#define _STEAM_AUTHORIZATION_H_

namespace SteamWebApi
{
	class AuthorizationApi : public BaseApi
	{
	public:

		class AuthResult : public Result
		{
			friend AuthorizationApi;

		private:
			std::string steamid;
			std::string token;
			std::string cookie;

			std::string sessionid;

			std::string emailauth;
			std::string emaildomain;
			std::string emailsteamid;

			std::string captchagid;
			std::string captcha_text;

			std::wstring message;

			bool captcha_needed;
			bool emailauth_needed;

		public:
			AuthResult()
			{
				captcha_needed = false;
				emailauth_needed = false;
				captchagid = "-1";
			}

			bool IsCaptchaNeeded() const { return captcha_needed; }
			bool IsEmailAuthNeeded() const { return emailauth_needed; }
			const char *GetSteamid() const { return steamid.c_str(); }
			const char *GetToken() const { return token.c_str(); }
			const char *GetCookie() const { return cookie.c_str(); }
			const char *GetSessionId() const { return sessionid.c_str(); }
			const char *GetAuthId() const { return emailauth.c_str(); }
			const char *GetAuthCode() const { return emailsteamid.c_str(); }
			const char *GetEmailDomain() const { return emaildomain.c_str(); }
			const char *GetCaptchaId() const { return captchagid.c_str(); }
			const wchar_t *GetMessage() const { return message.c_str(); }

			void SetAuthCode(char *code)
			{
				emailauth = code;
			}

			void SetCaptchaText(char *text)
			{
				captcha_text = text;
			}
		};

		static void Authorize(HANDLE hConnection, const wchar_t *username, const char *password, const char *timestamp, AuthResult *authResult)
		{
			authResult->success = false;
			authResult->captcha_needed = false;
			authResult->emailauth_needed = false;

			ptrA base64Username(mir_urlEncode(ptrA(mir_utf8encodeW(username))));

			char data[1024];
			mir_snprintf(data, SIZEOF(data),
				"username=%s&password=%s&emailauth=%s&emailsteamid=%s&captchagid=%s&captcha_text=%s&rsatimestamp=%s&donotcache=%ld&remember_login=true&oauth_client_id=DE45CD61&oauth_scope=read_profile write_profile read_client write_client",
				base64Username,
				ptrA(mir_urlEncode(password)),
				ptrA(mir_urlEncode(authResult->emailauth.c_str())),
				authResult->emailsteamid.c_str(),
				authResult->captchagid.c_str(),
				ptrA(mir_urlEncode(authResult->captcha_text.c_str())),
				timestamp,
				time(NULL));

			SecureHttpPostRequest request(hConnection, STEAM_COM_URL "/mobilelogin/dologin");
			request.ResetFlags(NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP);
			request.SetData(data, strlen(data));

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((authResult->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node;

			node = json_get(root, "success");
			authResult->success = json_as_bool(node) > 0;
			if (!authResult->success)
			{
				node = json_get(root, "emailauth_needed");
				authResult->emailauth_needed = json_as_bool(node) > 0;
				if (authResult->emailauth_needed)
				{
					node = json_get(root, "emailsteamid");
					authResult->emailsteamid = ptrA(mir_u2a(json_as_string(node)));

					node = json_get(root, "emaildomain");
					authResult->emaildomain = ptrA(mir_utf8encodeW(json_as_string(node)));
				}

				node = json_get(root, "captcha_needed");
				authResult->captcha_needed = json_as_bool(node) > 0;
				if (authResult->captcha_needed)
				{
					node = json_get(root, "captcha_gid");
					authResult->captchagid = ptrA(mir_u2a(json_as_string(node)));
				}

				if (!authResult->emailauth_needed && !authResult->captcha_needed)
				{
					node = json_get(root, "message");
					authResult->message = json_as_string(node);
				}
			}
			else
			{
				node = json_get(root, "login_complete");
				if (!json_as_bool(node))
					return;

				node = json_get(root, "oauth");
				root = json_parse(ptrA(mir_u2a(json_as_string(node))));

				node = json_get(root, "steamid");
				authResult->steamid = ptrA(mir_u2a(json_as_string(node)));

				node = json_get(root, "oauth_token");
				authResult->token = ptrA(mir_u2a(json_as_string(node)));

				node = json_get(root, "webcookie");
				authResult->cookie = ptrA(mir_u2a(json_as_string(node)));

				mir_snprintf(data, SIZEOF(data),
					"oauth_token=%s&steamid=%s&webcookie=%s",
					authResult->token.c_str(),
					authResult->steamid.c_str(),
					authResult->cookie.c_str());

				SecureHttpPostRequest second_request(hConnection, STEAM_COM_URL "/mobileloginsucceeded");
				second_request.ResetFlags(NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP);
				second_request.SetData(data, strlen(data));

				response = second_request.Send();
				if (!response)
					return;

				for (int i = 0; i < response->headersCount; i++)
				{
					if (lstrcmpiA(response->headers[i].szName, "Set-Cookie"))
						continue;

					std::string cookies = response->headers[i].szValue;
					size_t start = cookies.find("sessionid=") + 10;
					size_t end = cookies.substr(start).find(';');
					authResult->sessionid = cookies.substr(start, end - start + 10);
					break;
				}

				authResult->success = true;
				authResult->captcha_needed = false;
				authResult->emailauth_needed = false;
			}
		}
	};

	class AuthorizationRequest : public HttpsPostRequest
	{
		void InitData(const char *username, const char *password, const char *timestamp, const char *guardId = "-1", const char *guardCode = "")
		{
			char data[1024];
			mir_snprintf(data, SIZEOF(data),
				"username=%s&password=%s&emailsteamid=%s&emailauth=%s&captchagid=%s&captcha_text=%s&rsatimestamp=%s&donotcache=%ld&remember_login=true&oauth_client_id=DE45CD61&oauth_scope=read_profile write_profile read_client write_client",
				username,
				ptrA(mir_urlEncode(password)),
				guardId,
				guardCode,
				"-1",
				"",
				timestamp,
				time(NULL));

			SetData(data, strlen(data));
		}

	public:
		AuthorizationRequest(const char *username, const char *password, const char *timestamp) :
			HttpsPostRequest(STEAM_COM_URL "/mobilelogin/dologin")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			InitData(username, password, timestamp);
		}

		AuthorizationRequest(const char *username, const char *password, const char *timestamp, const char *guardId, const char *guardCode) :
			HttpsPostRequest(STEAM_COM_URL "/mobilelogin/dologin")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			InitData(username, password, timestamp, guardId, guardCode);
		}

		/*const wchar_t *GetUsername() { return username; }
		const char *GetPassword() { return password; }
		const char *GetTimestamp() { return timestamp; }*/
	};
}


#endif //_STEAM_AUTHORIZATION_H_