/* aww whatup?  this is all the oauth functions, at the moment
 * they're all part of the twitter class.. i think this is the
 * best way?  
 */

#include "twitter.h"
//#include "tc2.h"
#include "utility.h"
#include "stdafx.h"
#include "common.h"

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
		oauthParameters[L"oauth_token"] = requestToken; /*WLOG("requestToken not empty: %s", requestToken);*/
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
		//LOG("in post section of buildOAuthParams");
		allParameters.insert(postData->begin(), postData->end());
	}

	allParameters.insert(oauthParameters.begin(), oauthParameters.end());

	// prepare a signature base, a carefully formatted string containing 
	// all of the necessary information needed to generate a valid signature
	wstring normalUrl = OAuthNormalizeUrl(url);
	//WLOG("normalURL is %s", normalUrl);
	wstring normalizedParameters = OAuthNormalizeRequestParameters(allParameters);
	//WLOG("normalisedparams is %s", normalizedParameters);
	wstring signatureBase = OAuthConcatenateRequestElements(httpMethod, normalUrl, normalizedParameters);
	//WLOG("sigBase is %s", signatureBase);

	// obtain a signature and add it to header requestParameters
	wstring signature = OAuthCreateSignature(signatureBase, consumerSecret, requestTokenSecret);
	//WLOG("**BuildSignedOAuthParameters - sig is %s", signature);
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
		//WLOG("inside crack, url is %s", url);
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
	//WLOG("URL is %s", url);
    wstring query = UrlGetQuery(url);
	//WLOG("query is %s", query);
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
	//WLOG("OAuthWebRequestSubmit(%s)", url);

	//wstring oauthHeader = L"Authorization: OAuth ";
	wstring oauthHeader = L"OAuth ";

	for(OAuthParameters::const_iterator it = parameters.begin(); 
		it != parameters.end(); 
		++it)
	{
		//WLOG("%s = ", it->first);
		//WLOG("%s", it->second);
		//LOG("---------");

		if(it != parameters.begin())
		{
			oauthHeader += L",";
		}

		wstring pair;
		pair += it->first + L"=\"" + it->second + L"\"";
		oauthHeader += pair;
	}

	//WLOG("oauthheader is %s", oauthHeader);

	return oauthHeader;
}

// parameters must already be URL encoded before calling BuildQueryString
std::wstring mir_twitter::BuildQueryString( const OAuthParameters &parameters ) 
{
	wstring query;
	//LOG("do we ever get here?");
	for(OAuthParameters::const_iterator it = parameters.begin(); 
		it != parameters.end(); 
		++it)
	{
		//LOG("aww como ONNNNNN");
		//LOG("%s = %s", it->first.c_str(), it->second.c_str());
		//WLOG("in buildqueryString bit, first is %s", it->first);

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
	//WLOG("before OAUTHConcat, params are %s", parameters);
	wstring escapedParameters = UrlEncode(parameters);
	//LOG(")))))))))))))))))))))))))))))))))))))))))))))))");
	//WLOG("after url encode, its %s", escapedParameters);
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
	//WLOG("**CRACK - URL to split is %s", url);

	scheme1 = urlToks[0];
	//WLOG("**CRACK - scheme is %s", scheme1);

	if (urlToks.size() == 2) { // if there is only 1 ":" in the url
		if (Compare(scheme1, L"http", false)) {
			port1 = L"80";
		}
		else {
			port1 = L"443";
		}

		//WLOG("**CRACK::2 - port is %s", port1);

		Split(urlToks[1], urlToks2, L'/', false);
		domain1 = urlToks2[0];
		//WLOG("**CRACK::2 - domain is %s", domain1);
		explicitPort = L"0";
	}
	else if (urlToks.size() == 3) { // if there are 2 ":"s in the URL, ie a port is explicitly set
		domain1 = urlToks[1].substr(2, urlToks[1].size());
		//WLOG("**CRACK::3 - domain is %s", domain1);
		Split(urlToks[2], urlToks2, L'/', false);
		port1 = urlToks2[0];
		//WLOG("**CRACK::3 - port is %s", port1);
		explicitPort = L"1";
	}
	else {
		WLOG("**CRACK - not a proper URL? doesn't have a colon. URL is %s", url);

	}

	for (size_t i = 1; i < urlToks2.size(); ++i) {
		if (i > 1) {
			path1 += L"/";
		}
		path1 += urlToks2[i];
	}
	//WLOG("**CRACK - path is %s", path1);
	
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

	//WLOG("**CRACK - extraInfo is %s", extraInfo);

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
			swprintf_s(port, SIZEOF(port), L":%s", brokenURL[L"port"]);
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
		//WLOG("**OAuthNOrmailseURL - normalUrl is %s", normalUrl);
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
	swprintf_s(buf, SIZEOF(buf), L"%I64u", utcNow);

	return buf;
}

string mir_twitter::HMACSHA1( const string& keyBytes, const string& data ) 
{
	// based on http://msdn.microsoft.com/en-us/library/aa382379%28v=VS.85%29.aspx

	string hash;

	//--------------------------------------------------------------------
	// Declare variables.
	//
	// hProv:           Handle to a cryptographic service provider (CSP). 
	//                  This example retrieves the default provider for  
	//                  the PROV_RSA_FULL provider type.  
	// hHash:           Handle to the hash object needed to create a hash.
	// hKey:            Handle to a symmetric key. This example creates a 
	//                  key for the RC4 algorithm.
	// hHmacHash:       Handle to an HMAC hash.
	// pbHash:          Pointer to the hash.
	// dwDataLen:       Length, in bytes, of the hash.
	// Data1:           Password string used to create a symmetric key.
	// Data2:           Message string to be hashed.
	// HmacInfo:        Instance of an HMAC_INFO structure that contains 
	//                  information about the HMAC hash.
	// 
	HCRYPTPROV  hProv       = NULL;
	HCRYPTHASH  hHash       = NULL;
	HCRYPTKEY   hKey        = NULL;
	HCRYPTHASH  hHmacHash   = NULL;
	PBYTE       pbHash      = NULL;
	DWORD       dwDataLen   = 0;
	//BYTE        Data1[]     = {0x70,0x61,0x73,0x73,0x77,0x6F,0x72,0x64};
	//BYTE        Data2[]     = {0x6D,0x65,0x73,0x73,0x61,0x67,0x65};
	HMAC_INFO   HmacInfo;

	//--------------------------------------------------------------------
	// Zero the HMAC_INFO structure and use the SHA1 algorithm for
	// hashing.

	ZeroMemory(&HmacInfo, sizeof(HmacInfo));
	HmacInfo.HashAlgid = CALG_SHA1;

	//--------------------------------------------------------------------
	// Acquire a handle to the default RSA cryptographic service provider.

	if (!CryptAcquireContext(
		&hProv,                   // handle of the CSP
		NULL,                     // key container name
		NULL,                     // CSP name
		PROV_RSA_FULL,            // provider type
		CRYPT_VERIFYCONTEXT))     // no key access is requested
	{
		_TRACE(" Error in AcquireContext 0x%08x \n",
			GetLastError());
		goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Derive a symmetric key from a hash object by performing the
	// following steps:
	//    1. Call CryptCreateHash to retrieve a handle to a hash object.
	//    2. Call CryptHashData to add a text string (password) to the 
	//       hash object.
	//    3. Call CryptDeriveKey to create the symmetric key from the
	//       hashed password derived in step 2.
	// You will use the key later to create an HMAC hash object. 

	if (!CryptCreateHash(
		hProv,                    // handle of the CSP
		CALG_SHA1,                // hash algorithm to use
		0,                        // hash key
		0,                        // reserved
		&hHash))                  // address of hash object handle
	{
		_TRACE("Error in CryptCreateHash 0x%08x \n",
			GetLastError());
		goto ErrorExit;
	}

	if (!CryptHashData(
		hHash,                    // handle of the hash object
		(BYTE*)keyBytes.c_str(),                    // password to hash
		keyBytes.size(),            // number of bytes of data to add
		0))                       // flags
	{
		_TRACE("Error in CryptHashData 0x%08x \n", 
			GetLastError());
		goto ErrorExit;
	}

	// key creation based on 
	// http://mirror.leaseweb.com/NetBSD/NetBSD-release-5-0/src/dist/wpa/src/crypto/crypto_cryptoapi.c
	struct {
		BLOBHEADER hdr;
		DWORD len;
		BYTE key[1024]; // TODO might want to dynamically allocate this, Should Be Fine though
	} key_blob;

	key_blob.hdr.bType = PLAINTEXTKEYBLOB;
	key_blob.hdr.bVersion = CUR_BLOB_VERSION;
	key_blob.hdr.reserved = 0;
	/*
	* Note: RC2 is not really used, but that can be used to
	* import HMAC keys of up to 16 byte long.
	* CRYPT_IPSEC_HMAC_KEY flag for CryptImportKey() is needed to
	* be able to import longer keys (HMAC-SHA1 uses 20-byte key).
	*/
	key_blob.hdr.aiKeyAlg = CALG_RC2;
	key_blob.len = keyBytes.size();
	ZeroMemory(key_blob.key, sizeof(key_blob.key));

	_ASSERTE(keyBytes.size() <= SIZEOF(key_blob.key));
	CopyMemory(key_blob.key, keyBytes.c_str(), min(keyBytes.size(), SIZEOF(key_blob.key)));

	if (!CryptImportKey(
		hProv, 
		(BYTE *)&key_blob,
		sizeof(key_blob), 
		0, 
		CRYPT_IPSEC_HMAC_KEY,
		&hKey)) 
	{
		_TRACE("Error in CryptImportKey 0x%08x \n", GetLastError());
		goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Create an HMAC by performing the following steps:
	//    1. Call CryptCreateHash to create a hash object and retrieve 
	//       a handle to it.
	//    2. Call CryptSetHashParam to set the instance of the HMAC_INFO 
	//       structure into the hash object.
	//    3. Call CryptHashData to compute a hash of the message.
	//    4. Call CryptGetHashParam to retrieve the size, in bytes, of
	//       the hash.
	//    5. Call malloc to allocate memory for the hash.
	//    6. Call CryptGetHashParam again to retrieve the HMAC hash.

	if (!CryptCreateHash(
		hProv,                    // handle of the CSP.
		CALG_HMAC,                // HMAC hash algorithm ID
		hKey,                     // key for the hash (see above)
		0,                        // reserved
		&hHmacHash))              // address of the hash handle
	{
		_TRACE("Error in CryptCreateHash 0x%08x \n", 
			GetLastError());
		goto ErrorExit;
	}

	if (!CryptSetHashParam(
		hHmacHash,                // handle of the HMAC hash object
		HP_HMAC_INFO,             // setting an HMAC_INFO object
		(BYTE*)&HmacInfo,         // the HMAC_INFO object
		0))                       // reserved
	{
		_TRACE("Error in CryptSetHashParam 0x%08x \n", 
			GetLastError());
		goto ErrorExit;
	}

	if (!CryptHashData(
		hHmacHash,                // handle of the HMAC hash object
		(BYTE*)data.c_str(),                    // message to hash
		data.size(),            // number of bytes of data to add
		0))                       // flags
	{
		_TRACE("Error in CryptHashData 0x%08x \n", 
			GetLastError());
		goto ErrorExit;
	}

	//--------------------------------------------------------------------
	// Call CryptGetHashParam twice. Call it the first time to retrieve
	// the size, in bytes, of the hash. Allocate memory. Then call 
	// CryptGetHashParam again to retrieve the hash value.

	if (!CryptGetHashParam(
		hHmacHash,                // handle of the HMAC hash object
		HP_HASHVAL,               // query on the hash value
		NULL,                     // filled on second call
		&dwDataLen,               // length, in bytes, of the hash
		0))
	{
		_TRACE("Error in CryptGetHashParam 0x%08x \n", 
			GetLastError());
		goto ErrorExit;
	}

	pbHash = (BYTE*)malloc(dwDataLen);
	if(NULL == pbHash) 
	{
		_TRACE("unable to allocate memory\n");
		goto ErrorExit;
	}

	if (!CryptGetHashParam(
		hHmacHash,                 // handle of the HMAC hash object
		HP_HASHVAL,                // query on the hash value
		pbHash,                    // pointer to the HMAC hash value
		&dwDataLen,                // length, in bytes, of the hash
		0))
	{
		_TRACE("Error in CryptGetHashParam 0x%08x \n", GetLastError());
		goto ErrorExit;
	}

	for(DWORD i = 0 ; i < dwDataLen ; i++) 
	{
		hash.push_back((char)pbHash[i]);
	}

	// Free resources.
	// lol goto
ErrorExit:
	if(hHmacHash)
		CryptDestroyHash(hHmacHash);
	if(hKey)
		CryptDestroyKey(hKey);
	if(hHash)
		CryptDestroyHash(hHash);    
	if(hProv)
		CryptReleaseContext(hProv, 0);
	if(pbHash)
		free(pbHash);

	return hash;
}

wstring mir_twitter::Base64String( const string& hash ) 
{
	Base64Coder coder;
	coder.Encode((BYTE*)hash.c_str(), hash.size());
	wstring encoded = UTF8ToWide(coder.EncodedMessage());
	return encoded;
}

wstring mir_twitter::OAuthCreateSignature( const wstring& signatureBase, const wstring& consumerSecret, const wstring& requestTokenSecret ) 
{
	// URL encode key elements
	wstring escapedConsumerSecret = UrlEncode(consumerSecret);
	wstring escapedTokenSecret = UrlEncode(requestTokenSecret);

	wstring key = escapedConsumerSecret + L"&" + escapedTokenSecret;
	string keyBytes = WideToUTF8(key);

	string data = WideToUTF8(signatureBase);
	string hash = HMACSHA1(keyBytes, data);
	wstring signature = Base64String(hash);

	// URL encode the returned signature
	signature = UrlEncode(signature);
	return signature;
}
