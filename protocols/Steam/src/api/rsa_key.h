﻿#ifndef _STEAM_REQUEST_RSA_KEY_H_
#define _STEAM_REQUEST_RSA_KEY_H_

class GetRsaKeyRequest : public HttpRequest
{
public:
	GetRsaKeyRequest(const char *username) :
		HttpRequest(REQUEST_POST, STEAM_WEB_URL "/mobilelogin/getrsakey/")
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;

		AddHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");

		CMStringA data;
		data.AppendFormat("username=%s&donotcache=%lld", ptrA(mir_urlEncode(username)), time(NULL));
		SetData(data, data.GetLength());
	}
};

#endif //_STEAM_REQUEST_RSA_KEY_H_
