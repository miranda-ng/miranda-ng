/* aww whatup?  this is all the oauth functions, at the moment
 * they're all part of the twitter class.. i think this is the
 * best way?  
 */

#include "stdafx.h"
#include "twitter.h"
#include "utility.h"

OAuthParameters mir_twitter::BuildSignedOAuthParameters( const OAuthParameters& requestParameters, 
								   const std::wstring& url, 
								   const std::wstring& httpMethod, 
								   const OAuthParameters *postData,
								   const std::wstring& consumerKey, 
								   const std::wstring& consumerSecret,
								   const std::wstring& requestToken = L"", 
								   const std::wstring& requestTokenSecret = L"", 
								   const std::wstring& pin = L"" )
{
	wstring timestamp = OAuthCreateTimestamp();
	wstring nonce = OAuthCreateNonce();

	// create oauth requestParameters
	OAuthParameters oauthParameters;

	oauthParameters[L"oauth_timestamp"] = timestamp;
	oauthParameters[L"oauth_nonce"] = nonce;
	oauthParameters[L"oauth_version"] = L"1.0";
	oauthParameters[L"oauth_signature_method"] = L"HMAC-SHA1";
	oauthParameters[L"oauth_consumer_key"] = consumerKey;

	// add the request token if found
	if (!requestToken.empty())
	{
		oauthParameters[L"oauth_token"] = requestToken; /*debugLogW("requestToken not empty: %s", requestToken);*/
	}

	// add the authorization pin if found
	if (!pin.empty())
	{
		oauthParameters[L"oauth_verifier"] = pin;
	}

	// create a parameter list containing both oauth and original parameters
	// this will be used to create the parameter signature
	OAuthParameters allParameters = requestParameters;

	if(Compare(httpMethod, L"POST", false) && postData) {
		//debugLogA("in post section of buildOAuthParams");
		allParameters.insert(postData->begin(), postData->end());
	}

	allParameters.insert(oauthParameters.begin(), oauthParameters.end());

	// prepare a signature base, a carefully formatted string containing 
	// all of the necessary information needed to generate a valid signature
	wstring normalUrl = OAuthNormalizeUrl(url);
	//debugLogW("normalURL is %s", normalUrl);
	wstring normalizedParameters = OAuthNormalizeRequestParameters(allParameters);
	//debugLogW("normalisedparams is %s", normalizedParameters);
	wstring signatureBase = OAuthConcatenateRequestElements(httpMethod, normalUrl, normalizedParameters);
	//debugLogW("sigBase is %s", signatureBase);

	// obtain a signature and add it to header requestParameters
	wstring signature = OAuthCreateSignature(signatureBase, consumerSecret, requestTokenSecret);
	//debugLogW("**BuildSignedOAuthParameters - sig is %s", signature);
	oauthParameters[L"oauth_signature"] = signature;

	return oauthParameters;
}

wstring mir_twitter::UrlGetQuery( const wstring& url ) 
{
	wstring query;
	/*
	URL_COMPONENTS components = {sizeof(URL_COMPONENTS)};

	wchar_t buf[1024*4] = {};

	components.lpszExtraInfo = buf;
	components.dwExtraInfoLength = SIZEOF(buf);

	BOOL crackUrlOk = InternetCrackUrl(url.c_str(), url.size(), 0, &components);
	_ASSERTE(crackUrlOk);
	if(crackUrlOk)
	{*/

		map<wstring, wstring> brokenURL = CrackURL(url);

		query = brokenURL[L"extraInfo"];
		//debugLogW("inside crack, url is %s", url);
		wstring::size_type q = query.find_first_of(L'?');
		if(q != wstring::npos)
		{
			query = query.substr(q + 1);
		}

		wstring::size_type h = query.find_first_of(L'#');
		if(h != wstring::npos)
		{
			query = query.substr(0, h);
		}
	//}
	return query;
}

// OAuthWebRequest used for all OAuth related queries
//
// consumerKey and consumerSecret - must be provided for every call, they identify the application
// oauthToken and oauthTokenSecret - need to be provided for every call, except for the first token request before authorizing
// pin - only used during authorization, when the user enters the PIN they received from the twitter website
wstring mir_twitter::OAuthWebRequestSubmit( 
    const wstring& url, 
    const wstring& httpMethod, 
	const OAuthParameters *postData,
    const wstring& consumerKey, 
    const wstring& consumerSecret, 
	const wstring& oauthToken, 
    const wstring& oauthTokenSecret, 
    const wstring& pin
    )
{
	//debugLogW("URL is %s", url);
    wstring query = UrlGetQuery(url);
	//debugLogW("query is %s", query);
    OAuthParameters originalParameters = ParseQueryString(query);

    OAuthParameters oauthSignedParameters = BuildSignedOAuthParameters(
        originalParameters, 
        url, 
        httpMethod, postData,
        consumerKey, consumerSecret, 
        oauthToken, oauthTokenSecret, 
        pin );
    return OAuthWebRequestSubmit(oauthSignedParameters, url);
}

wstring mir_twitter::OAuthWebRequestSubmit( 
	const OAuthParameters& parameters, 
	const wstring& url 
	) 
{
	//debugLogW("OAuthWebRequestSubmit(%s)", url);

	//wstring oauthHeader = L"Authorization: OAuth ";
	wstring oauthHeader = L"OAuth ";

	for(OAuthParameters::const_iterator it = parameters.begin(); 
		it != parameters.end(); 
		++it)
	{
		//debugLogW("%s = ", it->first);
		//debugLogW("%s", it->second);
		//debugLogA("---------");

		if(it != parameters.begin())
		{
			oauthHeader += L",";
		}

		wstring pair;
		pair += it->first + L"=\"" + it->second + L"\"";
		oauthHeader += pair;
	}

	//debugLogW("oauthheader is %s", oauthHeader);

	return oauthHeader;
}

// parameters must already be URL encoded before calling BuildQueryString
std::wstring mir_twitter::BuildQueryString( const OAuthParameters &parameters ) 
{
	wstring query;
	//debugLogA("do we ever get here?");
	for(OAuthParameters::const_iterator it = parameters.begin(); 
		it != parameters.end(); 
		++it)
	{
		//debugLogA("aww como ONNNNNN");
		//debugLogA("%s = %s", it->first.c_str(), it->second.c_str());
		//debugLogW("in buildqueryString bit, first is %s", it->first);

		if(it != parameters.begin())
		{
			query += L"&";
		}

		wstring pair;
		pair += it->first + L"=" + it->second + L"";
		query += pair;
	}	
	return query;
}

wstring mir_twitter::OAuthConcatenateRequestElements( const wstring& httpMethod, wstring url, const wstring& parameters ) 
{
	wstring escapedUrl = UrlEncode(url);
	//debugLogW("before OAUTHConcat, params are %s", parameters);
	wstring escapedParameters = UrlEncode(parameters);
	//debugLogA(")))))))))))))))))))))))))))))))))))))))))))))))");
	//debugLogW("after url encode, its %s", escapedParameters);
	wstring ret = httpMethod + L"&" + escapedUrl + L"&" + escapedParameters;
	return ret;
}

/* CrackURL.. just basically pulls apart a url into a map of wstrings:
 * scheme, domain, port, path, extraInfo, explicitPort
 * explicitPort will be 0 or 1, 0 if there was no actual port in the url,
 * and 1 if it was explicitely specified.
 * eg "http://twitter.com/blah.htm" will give:
 * http, twitter.com, 80, blah.htm, "", 0
 * "https://twitter.com:989/blah.htm?boom" will give:
 * https, twitter.com, 989, blah.htm?boom, ?boom, 1
 */
map<wstring, wstring> mir_twitter::CrackURL(wstring url) {

	wstring scheme1, domain1, port1, path1, extraInfo, explicitPort;
	vector<wstring> urlToks, urlToks2, extraInfoToks;

	Split(url, urlToks, L':', false);
	//debugLogW("**CRACK - URL to split is %s", url);

	scheme1 = urlToks[0];
	//debugLogW("**CRACK - scheme is %s", scheme1);

	if (urlToks.size() == 2) { // if there is only 1 ":" in the url
		if (Compare(scheme1, L"http", false)) {
			port1 = L"80";
		}
		else {
			port1 = L"443";
		}

		//debugLogW("**CRACK::2 - port is %s", port1);

		Split(urlToks[1], urlToks2, L'/', false);
		domain1 = urlToks2[0];
		//debugLogW("**CRACK::2 - domain is %s", domain1);
		explicitPort = L"0";
	}
	else if (urlToks.size() == 3) { // if there are 2 ":"s in the URL, ie a port is explicitly set
		domain1 = urlToks[1].substr(2, urlToks[1].size());
		//debugLogW("**CRACK::3 - domain is %s", domain1);
		Split(urlToks[2], urlToks2, L'/', false);
		port1 = urlToks2[0];
		//debugLogW("**CRACK::3 - port is %s", port1);
		explicitPort = L"1";
	}
	else ppro_->debugLogW(L"**CRACK - not a proper URL? doesn't have a colon. URL is %s", url);

	for (size_t i = 1; i < urlToks2.size(); ++i) {
		if (i > 1) {
			path1 += L"/";
		}
		path1 += urlToks2[i];
	}
	//debugLogW("**CRACK - path is %s", path1);
	
	wstring::size_type foundHash = path1.find(L"#");
	wstring::size_type foundQ = path1.find(L"?");

	if ((foundHash != wstring::npos) || (foundQ != wstring::npos)) { // if we've found a # or a ?...
		if (foundHash == wstring::npos) { // if we didn't find a #, we must have found a ?
			extraInfo = path1.substr(foundQ);
		}
		else if (foundQ == wstring::npos) { // if we didn't find a ?, we must have found a #
			extraInfo = path1.substr(foundHash);
		}
		else { // we found both a # and a ?, whichever came first we grab the sub string from there
			if (foundQ < foundHash) {
				extraInfo = path1.substr(foundQ);
			}
			else {
				extraInfo = path1.substr(foundHash);
			}
		}
	}
	else { // we have no # or ? in the path...
		extraInfo = L"";
	}

	//debugLogW("**CRACK - extraInfo is %s", extraInfo);

	map<wstring, wstring> result;
	result[L"scheme"] = scheme1;
	result[L"domain"] = domain1;
	result[L"port"] = port1;
	result[L"path"] = path1;
	result[L"extraInfo"] = extraInfo;
	result[L"explicitPort"] = explicitPort;

	return result;
}

wstring mir_twitter::OAuthNormalizeUrl( const wstring& url ) 
{
	/*wchar_t scheme[1024*4] = {};
	wchar_t host[1024*4] = {};
	wchar_t path[1024*4] = {};

	URL_COMPONENTS components = { sizeof(URL_COMPONENTS) };

	components.lpszScheme = scheme;
	components.dwSchemeLength = SIZEOF(scheme);

	components.lpszHostName = host;
	components.dwHostNameLength = SIZEOF(host);

	components.lpszUrlPath = path;
	components.dwUrlPathLength = SIZEOF(path);

	BOOL crackUrlOk = InternetCrackUrl(url.c_str(), url.size(), 0, &components);*/

	wstring normalUrl = url;
	map<wstring, wstring> brokenURL = CrackURL(url);

	/*_ASSERTE(crackUrlOk);
	if(crackUrlOk)
	{*/
		wchar_t port[10] = {};

		// The port number must only be included if it is non-standard
		if(Compare(brokenURL[L"scheme"], L"http", false) && !(Compare(brokenURL[L"port"], L"80", false)) || 
			(Compare(brokenURL[L"scheme"], L"https", false) && !(Compare(brokenURL[L"port"], L"443", false))))
		{
			mir_snwprintf(port, SIZEOF(port), L":%s", brokenURL[L"port"]);
		}

		// InternetCrackUrl includes ? and # elements in the path, 
		// which we need to strip off
		wstring pathOnly = brokenURL[L"path"];
		wstring::size_type q = pathOnly.find_first_of(L"#?");
		if(q != wstring::npos)
		{
			pathOnly = pathOnly.substr(0, q);
		}

		normalUrl = brokenURL[L"scheme"] + L"://" + brokenURL[L"domain"] + port + L"/" + pathOnly;
		//debugLogW("**OAuthNOrmailseURL - normalUrl is %s", normalUrl);
	//}
	return normalUrl;
}

wstring mir_twitter::OAuthNormalizeRequestParameters( const OAuthParameters& requestParameters ) 
{
	list<wstring> sorted;
	for(OAuthParameters::const_iterator it = requestParameters.begin(); 
		it != requestParameters.end(); 
		++it)
	{
		wstring param = it->first + L"=" + it->second;
		sorted.push_back(param);
	}
	sorted.sort();

	wstring params;
	for(list<wstring>::iterator it = sorted.begin(); it != sorted.end(); ++it)
	{
		if(params.size() > 0)
		{
			params += L"&";
		}
		params += *it;
	}

	return params;
}

OAuthParameters mir_twitter::ParseQueryString( const wstring& url ) 
{
	OAuthParameters ret;

	vector<wstring> queryParams;
	Split(url, queryParams, L'&', false);

	for(size_t i = 0; i < queryParams.size(); ++i)
	{
		vector<wstring> paramElements;
		Split(queryParams[i], paramElements, L'=', true);
		_ASSERTE(paramElements.size() == 2);
		if(paramElements.size() == 2)
		{
			ret[paramElements[0]] = paramElements[1];
		}
	}
	return ret;
}

wstring mir_twitter::OAuthCreateNonce() 
{
	wchar_t ALPHANUMERIC[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	wstring nonce;

	for(int i = 0; i <= 16; ++i)
	{
		nonce += ALPHANUMERIC[rand() % (SIZEOF(ALPHANUMERIC) - 1)]; // don't count null terminator in array
	}
	return nonce;
}

wstring mir_twitter::OAuthCreateTimestamp() 
{
	__time64_t utcNow;
	__time64_t ret = _time64(&utcNow);
	_ASSERTE(ret != -1);

	wchar_t buf[100] = {};
	mir_snwprintf(buf, SIZEOF(buf), L"%I64u", utcNow);

	return buf;
}

wstring mir_twitter::OAuthCreateSignature( const wstring& signatureBase, const wstring& consumerSecret, const wstring& requestTokenSecret ) 
{
	// URL encode key elements
	wstring escapedConsumerSecret = UrlEncode(consumerSecret);
	wstring escapedTokenSecret = UrlEncode(requestTokenSecret);

	wstring key = escapedConsumerSecret + L"&" + escapedTokenSecret;
	string keyBytes = WideToUTF8(key);

	BYTE digest[MIR_SHA1_HASH_SIZE];
	string data = WideToUTF8(signatureBase);
	mir_hmac_sha1(digest, (PBYTE)keyBytes.c_str(), keyBytes.size(), (PBYTE)data.c_str(), data.size());
	ptrA encoded( mir_base64_encode(digest, sizeof(digest)));
	return UrlEncode((TCHAR*)_A2T(encoded));
}
