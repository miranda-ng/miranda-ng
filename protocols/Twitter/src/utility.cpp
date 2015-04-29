/*
Copyright © 2012-15 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "utility.h"

#include <io.h>

std::string b64encode(const std::string &s)
{
	return std::string(ptrA(mir_base64_encode((BYTE*)s.c_str(), (unsigned)s.length())));
}

std::string int2str(int32_t iVal)
{
	char buf[100];
	_itoa_s(iVal, buf, 10);
	return std::string(buf);
}

std::string int2str(uint64_t iVal)
{
	char buf[100];
	_i64toa_s(iVal, buf, _countof(buf), 10);
	return std::string(buf);
}

uint64_t str2int(const std::string &str)
{
	return atoll(str.c_str());
}

http::response mir_twitter::slurp(const std::string &url, http::method meth, OAuthParameters postParams)
{
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = (meth == http::get) ? REQUEST_GET : REQUEST_POST;
	req.szUrl = const_cast<char*>(url.c_str());

	std::wstring url_WSTR = UTF8ToWide(url);
	std::string pdata_STR;
	std::wstring pdata_WSTR;
	std::wstring auth;

	if (meth == http::get) {
		if (url_WSTR.size() > 0) { ppro_->debugLogW(L"**SLURP::GET - we have a URL: %s", url_WSTR); }
		if (consumerKey_.size() > 0) { ppro_->debugLogA("**SLURP::GET - we have a consumerKey"); }
		if (consumerSecret_.size() > 0) { ppro_->debugLogA("**SLURP::GET - we have a consumerSecret"); }
		if (oauthAccessToken_.size() > 0) { ppro_->debugLogA("**SLURP::GET - we have a oauthAccessToken"); }
		if (oauthAccessTokenSecret_.size() > 0) { ppro_->debugLogA("**SLURP::GET - we have a oauthAccessTokenSecret"); }
		if (pin_.size() > 0) { ppro_->debugLogA("**SLURP::GET - we have a pin"); }

		auth = OAuthWebRequestSubmit(url_WSTR, L"GET", NULL, consumerKey_, consumerSecret_,
			oauthAccessToken_, oauthAccessTokenSecret_, pin_);
	}
	else if (meth == http::post) {
		// OAuthParameters postParams;
		if (url_WSTR.size() > 0) { ppro_->debugLogW(L"**SLURP::POST - we have a URL: %s", url_WSTR); }
		if (consumerKey_.size() > 0) { ppro_->debugLogA("**SLURP::POST - we have a consumerKey"); }
		if (consumerSecret_.size() > 0) { ppro_->debugLogA("**SLURP::POST - we have a consumerSecret"); }
		if (oauthAccessToken_.size() > 0) { ppro_->debugLogA("**SLURP::POST - we have a oauthAccessToken"); }
		if (oauthAccessTokenSecret_.size() > 0) { ppro_->debugLogA("**SLURP::POST - we have a oauthAccessTokenSecret"); }
		if (pin_.size() > 0) { ppro_->debugLogA("**SLURP::POST - we have a pin"); }

		pdata_WSTR = BuildQueryString(postParams);

		ppro_->debugLogW(L"**SLURP::POST - post data is: %s", pdata_WSTR);

		auth = OAuthWebRequestSubmit(url_WSTR, L"POST", &postParams, consumerKey_, consumerSecret_, oauthAccessToken_, oauthAccessTokenSecret_);
	}
	else ppro_->debugLogA("**SLURP - There is something really wrong.. the http method was neither get or post.. WHY??");

	std::string auth_STR = WideToUTF8(auth);

	NETLIBHTTPHEADER hdr[3];
	hdr[0].szName = "Authorization";
	hdr[0].szValue = const_cast<char*>(auth_STR.c_str());

	req.headers = hdr;
	req.headersCount = 1;

	if (meth == http::post) {
		hdr[1].szName = "Content-Type";
		hdr[1].szValue = "application/x-www-form-urlencoded";
		hdr[2].szName = "Cache-Control";
		hdr[2].szValue = "no-cache";

		pdata_STR = WideToUTF8(pdata_WSTR);

		req.headersCount = 3;
		req.dataLength = (int)pdata_STR.size();
		req.pData = const_cast<char*>(pdata_STR.c_str());
		ppro_->debugLogA("**SLURP::POST - req.pdata is %s", req.pData);
	}

	req.flags = NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	req.nlc = httpPOST_;
	http::response resp_data;
	ppro_->debugLogA("**SLURP - just before calling HTTPTRANSACTION");
	NETLIBHTTPREQUEST *resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(handle_), reinterpret_cast<LPARAM>(&req)));
	ppro_->debugLogA("**SLURP - HTTPTRANSACTION complete.");
	if (resp) {
		ppro_->debugLogA("**SLURP - the server has responded!");
		httpPOST_ = resp->nlc;
		resp_data.code = resp->resultCode;
		resp_data.data = resp->pData ? resp->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else {
		httpPOST_ = NULL;
		ppro_->debugLogA("SLURP - there was no response!");
	}

	return resp_data;
}

bool save_url(HANDLE hNetlib, const std::string &url, const std::tstring &filename)
{
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.szUrl = const_cast<char*>(url.c_str());

	NETLIBHTTPREQUEST *resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(hNetlib), reinterpret_cast<LPARAM>(&req)));

	if (resp) {
		bool success = (resp->resultCode == 200);
		if (success) {
			// Create folder if necessary
			std::tstring dir = filename.substr(0, filename.rfind('\\'));
			if (_taccess(dir.c_str(), 0))
				CreateDirectoryTreeT(dir.c_str());

			// Write to file
			FILE *f = _tfopen(filename.c_str(), _T("wb"));
			fwrite(resp->pData, 1, resp->dataLength, f);
			fclose(f);
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		return success;
	}

	return false;
}
