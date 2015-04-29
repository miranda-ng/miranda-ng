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

#pragma once

#include "http.h"
#include "twitter.h"

std::string b64encode(const std::string &s);

std::string int2str(int32_t);
std::string int2str(uint64_t);
uint64_t str2int(const std::string &str);

class mir_twitter : public twitter
{
public:
	mir_twitter() : twitter(), handle_(NULL), httpPOST_(NULL) {}
	void set_handle(PROTO_INTERFACE *ppro, HANDLE h)
	{
		ppro_ = ppro;
		handle_ = h;
	}

	// OAuthWebRequest used for all OAuth related queries
	//
	// consumerKey and consumerSecret - must be provided for every call, they identify the application
	// oauthToken and oauthTokenSecret - need to be provided for every call, except for the first token request before authorizing
	// pin - only used during authorization, when the user enters the PIN they received from the twitter website
	std::wstring OAuthWebRequestSubmit( 
		const std::wstring& url, 
		const std::wstring& httpMethod, 
		const OAuthParameters *postData,
		const std::wstring& consumerKey, 
		const std::wstring& consumerSecret, 
		const std::wstring& oauthToken = L"", 
		const std::wstring& oauthTokenSecret = L"", 
		const std::wstring& pin = L""
		);

	std::wstring OAuthWebRequestSubmit( 
		const OAuthParameters& parameters, 
		const std::wstring& url);

	std::wstring UrlGetQuery( const std::wstring& url );

	OAuthParameters BuildSignedOAuthParameters( const OAuthParameters& requestParameters, 
								   const std::wstring& url, 
								   const std::wstring& httpMethod, 
								   const OAuthParameters *postData,
								   const std::wstring& consumerKey, 
								   const std::wstring& consumerSecret,
								   const std::wstring& requestToken, 
								   const std::wstring& requestTokenSecret, 
								   const std::wstring& pin );

	std::wstring BuildQueryString( const OAuthParameters &parameters ) ;
	std::wstring OAuthConcatenateRequestElements( const std::wstring& httpMethod, std::wstring url, const std::wstring& parameters );
	std::map<std::wstring, std::wstring> CrackURL(std::wstring );
	std::wstring brook_httpsend(std::wstring, std::wstring, std::wstring, std::wstring);
	void Disconnect(void) { if (httpPOST_) Netlib_CloseHandle(httpPOST_); httpPOST_ = NULL; }
	std::wstring OAuthNormalizeUrl( const std::wstring& url );
	std::wstring OAuthNormalizeRequestParameters( const OAuthParameters& requestParameters );
	OAuthParameters ParseQueryString( const std::wstring& url );

	std::wstring OAuthCreateNonce();
	std::wstring OAuthCreateTimestamp();
	std::wstring OAuthCreateSignature( const std::wstring& signatureBase, const std::wstring& consumerSecret, const std::wstring& requestTokenSecret );

protected:
	http::response slurp(const std::string &,http::method, OAuthParameters );

	HANDLE httpPOST_;
	HANDLE handle_;
	PROTO_INTERFACE *ppro_;
};

inline void mbcs_to_tcs(UINT code_page, const char *mbstr, TCHAR *tstr, int tlen)
{
	MultiByteToWideChar(code_page, 0, mbstr, -1, tstr, tlen);
}

bool save_url(HANDLE hNetlib,const std::string &url,const std::tstring &filename);