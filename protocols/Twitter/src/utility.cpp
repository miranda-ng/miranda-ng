/*
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

#include "utility.h"
//#include "tc2.h"

#include <io.h>

std::string b64encode(const std::string &s)
{
	return std::string( ptrA( mir_base64_encode((BYTE*)s.c_str(), (unsigned)s.length())));
}

http::response mir_twitter::slurp(const std::string &url,http::method meth,
	 OAuthParameters postParams)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	req.requestType = (meth == http::get) ? REQUEST_GET:REQUEST_POST;
	req.szUrl = const_cast<char*>(url.c_str());

	//std::wstring url_WSTR(url.length(),L' ');
	//std::copy(url.begin(), url.end(), url_WSTR.begin());
	std::wstring url_WSTR = UTF8ToWide(url);
	std::string pdata_STR;
	std::wstring pdata_WSTR;

	std::wstring auth;
	if (meth == http::get) {
		if (url_WSTR.size()>0) { WLOG("**SLURP::GET - we have a URL: %s", url_WSTR); }
		if (consumerKey_.size()>0) { LOG("**SLURP::GET - we have a consumerKey"); }
		if (consumerSecret_.size()>0) { LOG("**SLURP::GET - we have a consumerSecret"); }
		if (oauthAccessToken_.size()>0) { LOG("**SLURP::GET - we have a oauthAccessToken"); }
		if (oauthAccessTokenSecret_.size()>0) { LOG("**SLURP::GET - we have a oauthAccessTokenSecret"); }
		if (pin_.size()>0) { LOG("**SLURP::GET - we have a pin"); }
		//WLOG("consumerSEcret is %s", consumerSecret_);
		//WLOG("oauthAccessTok is %s", oauthAccessToken_);
		//WLOG("oautAccessTokSEc is %s", oauthAccessTokenSecret_);
		//WLOG("pin is %s", pin_);
		
		auth = OAuthWebRequestSubmit(url_WSTR, L"GET", NULL, consumerKey_, consumerSecret_, 
			oauthAccessToken_, oauthAccessTokenSecret_, pin_);
	}
	else if (meth == http::post) {

		//OAuthParameters postParams;
		if (url_WSTR.size()>0) { WLOG("**SLURP::POST - we have a URL: %s", url_WSTR); }
		if (consumerKey_.size()>0) { LOG("**SLURP::POST - we have a consumerKey"); }
		if (consumerSecret_.size()>0) { LOG("**SLURP::POST - we have a consumerSecret"); }
		if (oauthAccessToken_.size()>0) { LOG("**SLURP::POST - we have a oauthAccessToken"); }
		if (oauthAccessTokenSecret_.size()>0) { LOG("**SLURP::POST - we have a oauthAccessTokenSecret"); }
		if (pin_.size()>0) { LOG("**SLURP::POST - we have a pin"); }

		//WLOG("consumerKey is %s", consumerKey_);
		//WLOG("consumerSEcret is %s", consumerSecret_);
		//WLOG("oauthAccessTok is %s", oauthAccessToken_);
		//WLOG("oautAccessTokSEc is %s", oauthAccessTokenSecret_);

		//std::wstring pdata_WSTR(post_data.length(),L' ');
		//std::copy(post_data.begin(), post_data.end(), pdata_WSTR.begin());

		//postParams[L"status"] = UrlEncode(pdata_WSTR);
		//postParams[L"source"] = L"mirandang";

		pdata_WSTR = BuildQueryString(postParams);

		WLOG("**SLURP::POST - post data is: %s", pdata_WSTR); 

		auth = OAuthWebRequestSubmit(url_WSTR, L"POST", &postParams, consumerKey_, consumerSecret_, 
			oauthAccessToken_, oauthAccessTokenSecret_);
		//WLOG("**SLURP::POST auth is %s", auth);
	}
	else {
		LOG("**SLURP - There is something really wrong.. the http method was neither get or post.. WHY??");
	}

	//std::string auth_STR(auth.length(), ' ');
	//std::copy(auth.begin(), auth.end(), auth_STR.begin());

	std::string auth_STR = WideToUTF8(auth);

	NETLIBHTTPHEADER hdr[3];
	hdr[0].szName = "Authorization";
	hdr[0].szValue = const_cast<char*>(auth_STR.c_str());

	req.headers = hdr;
	req.headersCount = 1;

	if(meth == http::post)
	{
		hdr[1].szName = "Content-Type";
		hdr[1].szValue = "application/x-www-form-urlencoded";
		hdr[2].szName  = "Cache-Control";
		hdr[2].szValue = "no-cache";

		//char *pdata_STR = new char[pdata_WSTR.length() + 1];
		//sprintf(pdata_STR,"%ls",pdata_WSTR.c_str());

		pdata_STR = WideToUTF8(pdata_WSTR);

		req.headersCount = 3;
		req.dataLength = (int)pdata_STR.size();
		req.pData = const_cast<char*>(pdata_STR.c_str());
		LOG("**SLURP::POST - req.pdata is %s", req.pData);
	}

	req.flags = NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	req.nlc = httpPOST_;
	http::response resp_data;
	LOG("**SLURP - just before calling HTTPTRANSACTION");
	NETLIBHTTPREQUEST *resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(handle_), reinterpret_cast<LPARAM>(&req)));
	LOG("**SLURP - HTTPTRANSACTION complete.");
	if(resp)
	{
		LOG("**SLURP - the server has responded!");
		httpPOST_ = resp->nlc;
		resp_data.code = resp->resultCode;
		resp_data.data = resp->pData ? resp->pData:"";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
	}
	else { 
		httpPOST_ = NULL; 
		LOG("SLURP - there was no response!"); 
	}

	return resp_data;
}

INT_PTR mir_twitter::LOG(const char *fmt,...)
{
	va_list va;
	char text[1024];
	if (!handle_)
		return 0;

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return CallService(MS_NETLIB_LOG,(WPARAM)handle_,(LPARAM)text);
}

INT_PTR mir_twitter::WLOG(const char* first, const std::wstring last)
{
	char *str1 = new char[1024*96];
	mir_snprintf(str1, 1024*96, "%ls", last.c_str());

	return LOG(first, str1); 
}

bool save_url(HANDLE hNetlib,const std::string &url,const std::tstring &filename)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	req.requestType = REQUEST_GET;
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.szUrl = const_cast<char*>(url.c_str());

	NETLIBHTTPREQUEST *resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(hNetlib), reinterpret_cast<LPARAM>(&req)));

	if (resp)
	{
		bool success = (resp->resultCode == 200);
		if (success)
		{
			// Create folder if necessary
			std::tstring dir = filename.substr(0,filename.rfind('\\'));
			if( _taccess(dir.c_str(),0))
				CreateDirectoryTreeT(dir.c_str());

			// Write to file
			FILE *f = _tfopen(filename.c_str(), _T("wb"));
			fwrite(resp->pData,1,resp->dataLength,f);
			fclose(f);
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
		return success;
	}
	else
		return false;
}
