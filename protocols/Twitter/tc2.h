/* aww yeah creating some crazy .h file

*/

#ifndef TC2_H
#define TC2_H

//#include "proto.h"
#include "stdafx.h"
#include "twitter.h"

using namespace std;

//typedef std::map<wstring, wstring> OAuthParameters;

wstring getHostName();
wstring getAccessUrl();
wstring getAuthorizeUrl();
wstring getRequestUrl();
wstring getUserTimelineUrl();
wstring getConsumerKey();
wstring getConsumerSecret();

//wstring UrlGetQuery( const wstring& url );

OAuthParameters ParseQueryString( const wstring& url );

wstring OAuthCreateNonce();

wstring OAuthCreateTimestamp();

string HMACSHA1( const string& keyBytes, const string& data );

wstring Base64String( const string& hash );

string char2hex( char dec );

string urlencode(const string &c);

wstring UrlEncode( const wstring& url );

wstring OAuthCreateSignature( const wstring& signatureBase, const wstring& consumerSecret, const wstring& requestTokenSecret );

wstring OAuthConcatenateRequestElements( const wstring& httpMethod, wstring url, const wstring& parameters );

wstring OAuthNormalizeRequestParameters( const OAuthParameters& requestParameters );

//wstring OAuthNormalizeUrl( const wstring& url );

/*OAuthParameters BuildSignedOAuthParameters( const OAuthParameters& requestParameters, 
								   const wstring& url, 
								   const wstring& httpMethod, 
								   const wstring& consumerKey, 
								   const wstring& consumerSecret,
								   const wstring& requestToken, 
								   const wstring& requestTokenSecret, 
								   const wstring& pin );*/

/*wstring OAuthWebRequestSubmit( 
	const OAuthParameters& parameters, 
	const wstring& url 
	);*/

// OAuthWebRequest used for all OAuth related queries
//
// consumerKey and consumerSecret - must be provided for every call, they identify the application
// oauthToken and oauthTokenSecret - need to be provided for every call, except for the first token request before authorizing
// pin - only used during authorization, when the user enters the PIN they received from the twitter website
/*wstring OAuthWebRequestSubmit( 
    const wstring& url, 
    const wstring& httpMethod, 
    const wstring& consumerKey, 
    const wstring& consumerSecret, 
    const wstring& oauthToken = L"", 
    const wstring& oauthTokenSecret = L"", 
    const wstring& pin = L""
    );*/



int testfunc(int argc, _TCHAR* argv[]);

#endif
