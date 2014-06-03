#ifndef _STEAM_RSA_KEY_H_
#define _STEAM_RSA_KEY_H_

namespace SteamWebApi
{
	class RsaKeyApi : public BaseApi
	{
	public:

		class RsaKey : public Result
		{
			friend RsaKeyApi;

		private:
			std::string modulus;
			std::string exponent;
			std::string timestamp;

		public:
			const char * GetModulus() const { return modulus.c_str(); }
			const char * GetExponent() const { return exponent.c_str(); }
			const char * GetTimestamp() const { return timestamp.c_str(); }
		};

		static void GetRsaKey(HANDLE hConnection, const wchar_t *username, RsaKey *rsaKey)
		{
			rsaKey->success = false;

			ptrA base64Username(mir_urlEncode(ptrA(mir_utf8encodeW(username))));

			SecureHttpGetRequest request(hConnection, STEAM_COM_URL "/mobilelogin/getrsakey");
			request.AddParameter("username", (char*)base64Username);

			mir_ptr<NETLIBHTTPREQUEST> response(request.Send());
			if (!response)
				return;

			if ((rsaKey->status = (HTTP_STATUS)response->resultCode) != HTTP_STATUS_OK)
				return;

			JSONNODE *root = json_parse(response->pData), *node;
			if (!root) return;

			node = json_get(root, "success");
			if (!json_as_bool(node)) return;

			node = json_get(root, "publickey_mod");
			rsaKey->modulus = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "publickey_exp");
			rsaKey->exponent = ptrA(mir_u2a(json_as_string(node)));

			node = json_get(root, "timestamp");
			rsaKey->timestamp = ptrA(mir_u2a(json_as_string(node)));

			rsaKey->success = true;
		}
	};

	class RsaKeyRequest : public HttpsGetRequest
	{
	public:
		RsaKeyRequest(const char *username) :
			HttpsGetRequest(STEAM_COM_URL "/mobilelogin/getrsakey")
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

			AddParameter("username", (char*)username);
		}
	};
}

#endif //_STEAM_RSA_KEY_H_