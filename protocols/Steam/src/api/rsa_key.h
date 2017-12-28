#ifndef _STEAM_REQUEST_RSA_KEY_H_
#define _STEAM_REQUEST_RSA_KEY_H_

class GetRsaKeyRequest : public HttpRequest
{
public:
	GetRsaKeyRequest(const char *username) :
		HttpRequest(HttpPost, STEAM_WEB_URL "/mobilelogin/getrsakey/")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		CMStringA data;
		data.AppendFormat("username=%s&donotcache=%lld", ptrA(mir_urlEncode(username)), time(NULL));

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("username", username)
			<< INT64_PARAM("donotcache", time(NULL));
	}
};

#endif //_STEAM_REQUEST_RSA_KEY_H_
