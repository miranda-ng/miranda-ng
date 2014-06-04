#ifndef _STEAM_RSA_KEY_H_
#define _STEAM_RSA_KEY_H_

namespace SteamWebApi
{
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