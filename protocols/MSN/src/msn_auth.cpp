/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2007-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include <WinInet.h>
#include "msn_proto.h"
#include "msn_ieembed.h"
#include "des.h"

#define LOGIN_POST_PARAMS "client_id=00000000480BC46C&scope=service%3A%3Askype.com%3A%3AMBI_SSL&response_type=token&redirect_uri=https%3A%2F%2Flogin.live.com%2Foauth20_desktop.srf"
#define AUTH_URL "https://login.live.com/oauth20_authorize.srf?"LOGIN_POST_PARAMS
#define POST_URL "https://login.live.com/ppsecure/post.srf?"LOGIN_POST_PARAMS


/* WinINET delayloading */
typedef BOOL (*pfnInternetGetCookieExA)(LPCSTR, LPCSTR, LPSTR, LPDWORD, DWORD, LPVOID);
pfnInternetGetCookieExA fpInternetGetCookieExA = NULL;


/* SkyLogin Prototypes */
typedef void* SkyLogin;
typedef SkyLogin (*pfnSkyLogin_Init)();
typedef void (*pfnSkyLogin_Exit)(SkyLogin pInst);
typedef int (*pfnSkyLogin_LoadCredentials)(SkyLogin pInst, char *pszUser);
typedef int (*pfnSkyLogin_PerformLogin)(SkyLogin pInst, char *pszUser, char *pszPass);
typedef int (*pfnSkyLogin_CreateUICString)(SkyLogin pInst, const char *pszNonce, char *pszOutUIC);
typedef int (*pfnSkyLogin_PerformLoginOAuth)(SkyLogin pInst, const char *OAuth);
typedef int (*pfnSkyLogin_GetCredentialsUIC)(SkyLogin pInst, char *pszOutUIC);
typedef char *(*pfnSkyLogin_GetUser)(SkyLogin pInst);

#define LOAD_FN(name) (##name = (pfn##name)GetProcAddress(hLibSkylogin, #name))

static const char defaultPassportUrl[] = "https://login.live.com/RST2.srf";

static const char authPacket[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<s:Envelope"
		" xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
		" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\""
		" xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\""
		" xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\""
		" xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\""
		" xmlns:wsa=\"http://www.w3.org/2005/08/addressing\""
		" xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2005/02/sc\""
		" xmlns:wst=\"http://schemas.xmlsoap.org/ws/2005/02/trust\">"
	"<s:Header>"
		"<wsa:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/02/trust/RST/Issue</wsa:Action>"
		"<wsa:To s:mustUnderstand=\"1\">HTTPS://login.live.com:443//RST2.srf</wsa:To>"
		"<wsa:MessageID>%u</wsa:MessageID>"
		"<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"
			"<ps:HostingApp>{7108E71A-9926-4FCB-BCC9-9A9D3F32E423}</ps:HostingApp>"
			"<ps:BinaryVersion>5</ps:BinaryVersion>"
			"<ps:UIVersion>1</ps:UIVersion>"
			"<ps:Cookies />"
			"<ps:RequestParams>AQAAAAIAAABsYwQAAAAxMDMz</ps:RequestParams>"
		"</ps:AuthInfo>"
		"<wsse:Security>"
			"<wsse:UsernameToken wsu:Id=\"user\">"
				"<wsse:Username>%s</wsse:Username>"
				"<wsse:Password>%s</wsse:Password>"
			"</wsse:UsernameToken>"
			"<wsu:Timestamp Id=\"Timestamp\">"
				"<wsu:Created>%S</wsu:Created>"
				"<wsu:Expires>%S</wsu:Expires>"
			"</wsu:Timestamp>"
		"</wsse:Security>"
	"</s:Header>"
	"<s:Body>"
		"<ps:RequestMultipleSecurityTokens xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"RSTS\">"
			"<wst:RequestSecurityToken Id=\"RST0\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>http://Passport.NET/tb</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST1\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>chatservice.live.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI_SSL\" />"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST2\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>messenger.msn.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"?id=507\" />"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST3\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>messengersecure.live.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI_SSL\" />"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST4\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>contacts.msn.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI\" />"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST5\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>storage.msn.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI\" />"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST6\">"
				"<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
				"<wsp:AppliesTo>"
					"<wsa:EndpointReference>"
						"<wsa:Address>sup.live.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI\" />"
			"</wst:RequestSecurityToken>"
		"</ps:RequestMultipleSecurityTokens>"
	"</s:Body>"
"</s:Envelope>";


/////////////////////////////////////////////////////////////////////////////////////////
// Tokens, tokens, tokens.....
GenericToken::GenericToken(const char *pszTokenName):
	m_pszTokenName(pszTokenName),
	m_pszToken(NULL),
	m_tExpires(0),
	m_proto(NULL)
{
}

GenericToken::~GenericToken()
{
	mir_free(m_pszToken);
//	mir_free(m_pszRefreshToken);
}

bool GenericToken::Load()
{
	DBVARIANT dbv;
	char szTokenName[64];

	mir_snprintf(szTokenName, sizeof(szTokenName), "%sToken", m_pszTokenName);
	if (m_proto->getString(szTokenName, &dbv) == 0) {
		replaceStr(m_pszToken, dbv.pszVal);
		db_free(&dbv);

		mir_snprintf(szTokenName, sizeof(szTokenName), "%sExpiretime", m_pszTokenName);
		m_tExpires = m_proto->getDword(szTokenName, 0);

		return true;
	}
	return false;
}

void GenericToken::Save()
{
	char szTokenName[64];

	mir_snprintf(szTokenName, sizeof(szTokenName), "%sToken", m_pszTokenName);
	m_proto->setString(szTokenName, m_pszToken);
	mir_snprintf(szTokenName, sizeof(szTokenName), "%sExpiretime", m_pszTokenName);
	m_proto->setDword(szTokenName, m_tExpires);
}

bool GenericToken::Expired(time_t t)
{
	return t+3600 >= m_tExpires;
}

void GenericToken::SetToken(const char *pszToken, time_t tExpires)
{ 
	replaceStr(m_pszToken, pszToken);
	m_tExpires = tExpires;
	Save();
}

const char *GenericToken::Token()
{
	Refresh();
	return m_pszToken;
}

void GenericToken::Clear()
{
	char szTokenName[64];

	mir_free(m_pszToken);
	m_pszToken = NULL;
	m_tExpires = 0;
	mir_snprintf(szTokenName, sizeof(szTokenName), "%sToken", m_pszTokenName);
	m_proto->delSetting(szTokenName);
	mir_snprintf(szTokenName, sizeof(szTokenName), "%sExpiretime", m_pszTokenName);
	m_proto->delSetting(szTokenName);
}

OAuthToken::OAuthToken(const char *pszTokenName, const char *pszService, bool bPrependT):
	GenericToken(pszTokenName),
	m_pszService(pszService),
	m_bPreprendT(bPrependT)
{
}

bool OAuthToken::Refresh(bool bForce)
{
	char szToken[1024], szRefreshToken[1024];
	time_t tExpires;

	if ((!bForce && !Expired()) || !m_proto->authRefreshToken ||
		!m_proto->RefreshOAuth(m_proto->authRefreshToken, m_pszService, szToken+(m_bPreprendT?2:0), 
			szRefreshToken, &tExpires)) return false;
	if (m_bPreprendT) memcpy (szToken, "t=", 2);
	//replaceStr(m_pszRefreshToken, szRefreshToken);
	replaceStr(m_proto->authRefreshToken, szRefreshToken);
	m_proto->setString("authRefreshToken", szRefreshToken);
	SetToken(szToken, tExpires);
	return true;
}

SkypeToken::SkypeToken(const char *pszTokenName):
	GenericToken(pszTokenName)
{
}

bool SkypeToken::Refresh(bool bForce)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[1];
	char szPOST[2048], szToken[1024];
	bool bRet = false;

	if (!bForce && !Expired()) return false;

	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.nlc = m_proto->hHttpsConnection;
	nlhr.headersCount = _countof(headers);
	nlhr.headers = headers;
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = (char*)MSN_USER_AGENT;
	nlhr.pData = szPOST;

	if (m_proto->MyOptions.netId == NETID_SKYPE) {
		BYTE digest[16];
		char szPassword[100]={0};

		int cbPasswd=mir_snprintf(szPassword, sizeof(szPassword), "%s\nskyper\n", m_proto->MyOptions.szEmail);
		if (db_get_static(NULL, m_proto->m_szModuleName, "Password", szPassword + cbPasswd, sizeof(szPassword) - cbPasswd - 1))
			return false;
		mir_md5_hash((BYTE*)szPassword, mir_strlen(szPassword), digest);
		mir_base64_encodebuf(digest, sizeof(digest), szPassword, sizeof(szPassword));
		nlhr.szUrl = "https://api.skype.com/login/skypetoken";
		nlhr.dataLength = mir_snprintf(szPOST, sizeof(szPOST), "scopes=client&clientVersion=%s&username=%s&passwordHash=%s", 
			msnProductVer, m_proto->MyOptions.szEmail, szPassword);
	} else {
		// Get skype_token
		nlhr.szUrl = "https://api.skype.com/rps/skypetoken";
		nlhr.dataLength = mir_snprintf(szPOST, sizeof(szPOST), "scopes=client&clientVersion=%s&access_token=%s&partner=999", 
			msnProductVer, m_proto->authSkypeComToken.Token());
	}

	m_proto->mHttpsTS = clock();
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_proto->hNetlibUserHttps, (LPARAM)&nlhr);
	m_proto->mHttpsTS = clock();

	if (nlhrReply)  {
		m_proto->hHttpsConnection = nlhrReply->nlc;

		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			char *pSkypeToken, *pExpireTime, *pEnd;
			time_t tExpires;

			if ((pSkypeToken = strstr(nlhrReply->pData, "\"skypetoken\":\"")) && 
				(pEnd=strchr(pSkypeToken+15, '"')))
			{
				*pEnd = 0;
				pSkypeToken+=14;
				mir_snprintf (szToken, sizeof(szToken), "skype_token %s", pSkypeToken);

				if ((pExpireTime = strstr(pEnd+1, "\"expiresIn\":")) && 
					(pEnd=strchr(pExpireTime+12, '}'))) {
						*pEnd = 0;
						tExpires = atoi(pExpireTime+12);
				} else tExpires=86400;
				SetToken(szToken, time(NULL)+tExpires);
				bRet=true;
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	} else m_proto->hHttpsConnection = NULL;
	return bRet;
}

const char *SkypeToken::XSkypetoken()
{
	Refresh();
	if (m_pszToken) {
		char *pszRet = strchr(m_pszToken, ' ');
		if (pszRet) return pszRet+1;
	}
	return NULL;
}
// Performs the MSN Passport login via TLS
int CMsnProto::MSN_GetPassportAuth(void)
{
	int retVal = -1;

	if (!bPassportAuth && authRefreshToken) {
		// Slow authentication required by fetching multiple tokens, i.e. 2-factor auth :(
		MSN_RefreshOAuthTokens(false);
		return 0;
	}

	char szPassword[100];
	if (db_get_static(NULL, m_szModuleName, "Password", szPassword, sizeof(szPassword)))
		return 0;

	szPassword[16] = 0;

	time_t ts = time(NULL);

	TCHAR szTs1[64], szTs2[64];
	TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, ts, _T("I"), szTs1, _countof(szTs1), 0);
	TimeZone_PrintTimeStamp(UTC_TIME_HANDLE, ts + 20 * 60, _T("I"), szTs2, _countof(szTs2), 0);

	CMStringA szAuthInfo(FORMAT, authPacket, int(ts), MyOptions.szEmail, ptrA(HtmlEncode(szPassword)), szTs1, szTs2);

	char* szPassportHost = (char*)mir_alloc(256);
	if (db_get_static(NULL, m_szModuleName, "MsnPassportHost", szPassportHost, 256))
		mir_strcpy(szPassportHost, defaultPassportUrl);

	bool defaultUrlAllow = mir_strcmp(szPassportHost, defaultPassportUrl) != 0;
	char *tResult = NULL;

	while (retVal == -1) {
		unsigned status;

		tResult = getSslResult(&szPassportHost, szAuthInfo, NULL, status);
		if (tResult == NULL) {
			if (defaultUrlAllow) {
				mir_strcpy(szPassportHost, defaultPassportUrl);
				defaultUrlAllow = false;
				continue;
			}
			else {
				retVal = 4;
				break;
			}
		}

		switch (status) {
		case 200:
			const char *errurl;
			{
				errurl = NULL;
				ezxml_t xml = ezxml_parse_str(tResult, mir_strlen(tResult));

				ezxml_t tokr = ezxml_get(xml, "S:Body", 0,
					"wst:RequestSecurityTokenResponseCollection", 0,
					"wst:RequestSecurityTokenResponse", -1);

				while (tokr != NULL) {
					ezxml_t toks = ezxml_get(tokr, "wst:RequestedSecurityToken", 0,
						"wsse:BinarySecurityToken", -1);

					const char* addr = ezxml_txt(ezxml_get(tokr, "wsp:AppliesTo", 0,
						"wsa:EndpointReference", 0, "wsa:Address", -1));

					ezxml_t xml_expires = ezxml_get(tokr, "wst:Lifetime", 0, "wsu:Expires", -1);
					time_t expires;
					expires = xml_expires?IsoToUnixTime(ezxml_txt(xml_expires)):time(NULL)+86400;


					if (mir_strcmp(addr, "http://Passport.NET/tb") == 0) {
						ezxml_t node = ezxml_get(tokr, "wst:RequestedSecurityToken", 0, "EncryptedData", -1);
						free(hotAuthToken);
						hotAuthToken = ezxml_toxml(node, 0);
						setString("hotAuthToken", hotAuthToken);

						node = ezxml_get(tokr, "wst:RequestedProofToken", 0, "wst:BinarySecret", -1);
						replaceStr(hotSecretToken, ezxml_txt(node));
						setString("hotSecretToken", hotSecretToken);
					}
					else if (mir_strcmp(addr, "chatservice.live.com") == 0) {
						ezxml_t node = ezxml_get(tokr, "wst:RequestedProofToken", 0,
							"wst:BinarySecret", -1);
						if (toks) {
							authStrToken.SetToken(ezxml_txt(toks), expires);
							replaceStr(authSecretToken, ezxml_txt(node));
							retVal = 0;
						}
						else {
							errurl = ezxml_txt(ezxml_get(tokr, "S:Fault", 0, "psf:pp", 0, "psf:flowurl", -1));
						}
					}
					else if (mir_strcmp(addr, "messenger.msn.com") == 0 && toks) {
						const char* tok = ezxml_txt(toks);
						char* ch = (char*)strchr(tok, '&');
						*ch = 0;
						replaceStr(tAuthToken, tok + 2);
						replaceStr(pAuthToken, ch + 3);
						*ch = '&';
					}
					else if (mir_strcmp(addr, "contacts.msn.com") == 0 && toks)
						authContactToken.SetToken(ezxml_txt(toks), expires);
					else if (mir_strcmp(addr, "messengersecure.live.com") == 0 && toks) {
						replaceStr(oimSendToken, ezxml_txt(toks));
					}
					else if (mir_strcmp(addr, "storage.msn.com") == 0 && toks)
						authStorageToken.SetToken(ezxml_txt(toks), expires);

					tokr = ezxml_next(tokr);
				}

				if (retVal != 0) {
					if (errurl) {
						debugLogA("Starting URL: '%s'", errurl);
						Utils_OpenUrl(errurl);
					}

					ezxml_t tokf = ezxml_get(xml, "S:Body", 0, "S:Fault", 0, "S:Detail", -1);
					ezxml_t tokrdr = ezxml_child(tokf, "psf:redirectUrl");
					if (tokrdr != NULL) {
						mir_strcpy(szPassportHost, ezxml_txt(tokrdr));
						debugLogA("Redirected to '%s'", szPassportHost);
					}
					else {
						const char* szFault = ezxml_txt(ezxml_get(tokf, "psf:error", 0, "psf:value", -1));
						retVal = mir_strcmp(szFault, "0x80048821") == 0 ? 3 : (tokf ? 5 : 7);
						if (retVal != 3 && defaultUrlAllow) {
							mir_strcpy(szPassportHost, defaultPassportUrl);
							defaultUrlAllow = false;
							retVal = -1;
						}
						else if (retVal != 3 && retVal != 7) {
							char err[512];
							mir_snprintf(err, "Unknown Authentication error: %s", szFault);
							MSN_ShowError(err);
						}
					}
				}

				ezxml_free(xml);
			}
			break;

		default:
			if (defaultUrlAllow) {
				mir_strcpy(szPassportHost, defaultPassportUrl);
				defaultUrlAllow = false;
			}
			else
				retVal = 6;
		}
		mir_free(tResult);
	}

	if (retVal != 0) {
		if (!g_bTerminated) {
			switch (retVal) {
			case 3:
				MSN_ShowError("Your username or password is incorrect");
				ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
				break;

			case 5:
				break;

			default:
				MSN_ShowError("Unable to contact MS Passport servers check proxy/firewall settings");
				ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
				break;
			}
		}
	}
	else
		setString("MsnPassportHost", szPassportHost);

	mir_free(szPassportHost);
	debugLogA("MSN_CheckRedirector exited with errorCode = %d", retVal);
	return retVal;
}

static void derive_key(BYTE* der, unsigned char* key, size_t keylen, unsigned char* data, size_t datalen)
{
	BYTE hash1[MIR_SHA1_HASH_SIZE], hash2[MIR_SHA1_HASH_SIZE],
		hash3[MIR_SHA1_HASH_SIZE], hash4[MIR_SHA1_HASH_SIZE];

	const size_t buflen = MIR_SHA1_HASH_SIZE + datalen;
	BYTE* buf = (BYTE*)alloca(buflen);

	mir_hmac_sha1(hash1, key, keylen, data, datalen);
	mir_hmac_sha1(hash3, key, keylen, hash1, MIR_SHA1_HASH_SIZE);

	memcpy(buf, hash1, MIR_SHA1_HASH_SIZE);
	memcpy(buf + MIR_SHA1_HASH_SIZE, data, datalen);
	mir_hmac_sha1(hash2, key, keylen, buf, buflen);

	memcpy(buf, hash3, MIR_SHA1_HASH_SIZE);
	memcpy(buf + MIR_SHA1_HASH_SIZE, data, datalen);
	mir_hmac_sha1(hash4, key, keylen, buf, buflen);

	memcpy(der, hash2, MIR_SHA1_HASH_SIZE);
	memcpy(der + MIR_SHA1_HASH_SIZE, hash4, 4);
}

typedef struct tag_MsgrUsrKeyHdr
{
	unsigned size;
	unsigned cryptMode;
	unsigned cipherType;
	unsigned hashType;
	unsigned ivLen;
	unsigned hashLen;
	unsigned long cipherLen;
} MsgrUsrKeyHdr;

static const MsgrUsrKeyHdr userKeyHdr =
{
	sizeof(MsgrUsrKeyHdr),
	1,			// CRYPT_MODE_CBC
	0x6603,		// CALG_3DES
	0x8004,		// CALG_SHA1
	8,			// sizeof(ivBytes)
	MIR_SHA1_HASH_SIZE,
	72			// sizeof(cipherBytes);
};


static unsigned char* PKCS5_Padding(char* in, size_t &len)
{
	const size_t nlen = ((len >> 3) + 1) << 3;
	unsigned char* res = (unsigned char*)mir_alloc(nlen);
	memcpy(res, in, len);

	const unsigned char pad = 8 - (len & 7);
	memset(res + len, pad, pad);

	len = nlen;
	return res;
}


char* CMsnProto::GenerateLoginBlob(char* challenge)
{
	unsigned key1len;
	mir_ptr<BYTE> key1((BYTE*)mir_base64_decode(authSecretToken, &key1len));

	BYTE key2[MIR_SHA1_HASH_SIZE + 4], key3[MIR_SHA1_HASH_SIZE + 4];

	static const unsigned char encdata1[] = "WS-SecureConversationSESSION KEY HASH";
	static const unsigned char encdata2[] = "WS-SecureConversationSESSION KEY ENCRYPTION";

	derive_key(key2, key1, key1len, (unsigned char*)encdata1, sizeof(encdata1) - 1);
	derive_key(key3, key1, key1len, (unsigned char*)encdata2, sizeof(encdata2) - 1);

	size_t chllen = mir_strlen(challenge);

	BYTE hash[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(hash, key2, MIR_SHA1_HASH_SIZE + 4, (BYTE*)challenge, chllen);

	unsigned char* newchl = PKCS5_Padding(challenge, chllen);

	const size_t pktsz = sizeof(MsgrUsrKeyHdr) + MIR_SHA1_HASH_SIZE + 8 + chllen;
	unsigned char* userKey = (unsigned char*)alloca(pktsz);

	unsigned char* p = userKey;
	memcpy(p, &userKeyHdr, sizeof(MsgrUsrKeyHdr));
	((MsgrUsrKeyHdr*)p)->cipherLen = (int)chllen;
	p += sizeof(MsgrUsrKeyHdr);

	unsigned char iv[8];
	Utils_GetRandom(iv, sizeof(iv));

	memcpy(p, iv, sizeof(iv));
	p += sizeof(iv);

	memcpy(p, hash, sizeof(hash));
	p += MIR_SHA1_HASH_SIZE;

	des3_context ctxd;
	memset(&ctxd, 0, sizeof(ctxd));
	des3_set_3keys(&ctxd, key3);
	des3_cbc_encrypt(&ctxd, iv, newchl, p, (int)chllen);

	mir_free(newchl);

	return mir_base64_encode(userKey, (unsigned)pktsz);
}

CMStringA CMsnProto::HotmailLogin(const char* url)
{
	unsigned char nonce[24];
	Utils_GetRandom(nonce, sizeof(nonce));

	unsigned key1len;
	mir_ptr<BYTE> key1((BYTE*)mir_base64_decode(hotSecretToken, &key1len));

	static const unsigned char encdata[] = "WS-SecureConversation";
	const size_t data1len = sizeof(nonce) + sizeof(encdata) - 1;

	unsigned char* data1 = (unsigned char*)alloca(data1len);
	memcpy(data1, encdata, sizeof(encdata) - 1);
	memcpy(data1 + sizeof(encdata) - 1, nonce, sizeof(nonce));

	unsigned char key2[MIR_SHA1_HASH_SIZE + 4];
	derive_key(key2, key1, key1len, data1, data1len);

	CMStringA result;
	result.Format("%s&da=%s&nonce=", url, ptrA(mir_urlEncode(hotAuthToken)));

	ptrA noncenc(mir_base64_encode(nonce, sizeof(nonce)));
	result.Append(ptrA(mir_urlEncode(noncenc)));

	BYTE hash[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(hash, key2, sizeof(key2), (BYTE*)result.GetString(), result.GetLength());
	ptrA szHash(mir_base64_encode(hash, sizeof(hash)));
	result.AppendFormat("&hash=%s", ptrA(mir_urlEncode(szHash)));
	return result;
}

/* 1	-	Login successful
   0	-	Login failed
   -1	-	Loading Skylogin library failed
   -2	-	Functions cannot be loaded from Skylogin library
   -3	-	Initializing Skylogin library failed
 */
int CMsnProto::MSN_SkypeAuth(const char *pszNonce, char *pszUIC)
{
	int iRet = -1;
	pfnSkyLogin_Init SkyLogin_Init;
	pfnSkyLogin_Exit SkyLogin_Exit;
	pfnSkyLogin_LoadCredentials SkyLogin_LoadCredentials;
	pfnSkyLogin_PerformLogin SkyLogin_PerformLogin;
	pfnSkyLogin_CreateUICString SkyLogin_CreateUICString;

	HMODULE hLibSkylogin = LoadLibrary(_T("Plugins\\skylogin.dll"));
	if (hLibSkylogin) {

		// load function pointers
		if (!LOAD_FN(SkyLogin_Init) ||
			!LOAD_FN(SkyLogin_Exit) ||
			!LOAD_FN(SkyLogin_LoadCredentials) ||
			!LOAD_FN(SkyLogin_PerformLogin) ||
			!LOAD_FN(SkyLogin_CreateUICString))
		{
			FreeLibrary(hLibSkylogin);
			return -2;
		}

		// Perform login
		SkyLogin hLogin = SkyLogin_Init();
		if (hLogin) {
			char szPassword[100];
			if (!db_get_static(NULL, m_szModuleName, "Password", szPassword, sizeof(szPassword))) {
				if (SkyLogin_LoadCredentials(hLogin, MyOptions.szEmail) ||
					SkyLogin_PerformLogin(hLogin, MyOptions.szEmail, szPassword))
				{
					if (SkyLogin_CreateUICString(hLogin, pszNonce, pszUIC))
						iRet = 1;
				}
				else
					iRet = 0;
				SkyLogin_Exit(hLogin);
			}
			else
				iRet = 0;
		}
		else
			iRet = -3;
		FreeLibrary(hLibSkylogin);
	}
	return iRet;
}

/* 1	-	Login successful
   0	-	Login failed
   -1	-	Loading Skylogin library failed
   -2	-	Functions cannot be loaded from Skylogin library
   -3	-	Initializing Skylogin library failed
 */
int CMsnProto::LoginSkypeOAuth(const char *pRefreshToken)
{
	int iRet = -1;
	pfnSkyLogin_Init SkyLogin_Init;
	pfnSkyLogin_Exit SkyLogin_Exit;
	pfnSkyLogin_LoadCredentials SkyLogin_LoadCredentials;
	pfnSkyLogin_PerformLoginOAuth SkyLogin_PerformLoginOAuth;
	pfnSkyLogin_GetCredentialsUIC SkyLogin_GetCredentialsUIC;
	pfnSkyLogin_GetUser SkyLogin_GetUser;

	HMODULE hLibSkylogin = LoadLibrary(_T("Plugins\\skylogin.dll"));
	if (hLibSkylogin) {
		// load function pointers
		if (!LOAD_FN(SkyLogin_Init) ||
			!LOAD_FN(SkyLogin_Exit) ||
			!LOAD_FN(SkyLogin_LoadCredentials) ||
			!LOAD_FN(SkyLogin_PerformLoginOAuth) ||
			!LOAD_FN(SkyLogin_GetCredentialsUIC) ||
			!LOAD_FN(SkyLogin_GetUser))
		{
			FreeLibrary(hLibSkylogin);
			return -2;
		}

		// Perform login
		SkyLogin hLogin = SkyLogin_Init();
		if (hLogin) {
			char szLoginToken[1024];
			if (RefreshOAuth(pRefreshToken, "service::login.skype.com::MBI_SSL", szLoginToken) &&
				SkyLogin_PerformLoginOAuth(hLogin, szLoginToken))
			{
				char szUIC[1024];
				if (SkyLogin_GetCredentialsUIC(hLogin, szUIC)) {
					char *pszPartner;

					replaceStr(authUIC, szUIC);
					iRet = 1;
					if (pszPartner = SkyLogin_GetUser(hLogin)) 
						setString("SkypePartner", pszPartner);
				}
			}
			else
				iRet = 0;
			SkyLogin_Exit(hLogin);
		} 
		else
			iRet = -3;
		FreeLibrary(hLibSkylogin);
	}
	return iRet;
}

static int CopyCookies(NETLIBHTTPREQUEST *nlhrReply, NETLIBHTTPHEADER *hdr)
{
	int i, nSize = 1;
	char *p;

	if (hdr) {
		hdr->szName = "Cookie";
		*hdr->szValue = 0;
	}
	for (i = 0; i < nlhrReply->headersCount; i++) {
		if (mir_strcmpi(nlhrReply->headers[i].szName, "Set-Cookie"))
			continue;
		if (p=strchr(nlhrReply->headers[i].szValue, ';')) *p=0;
		if (hdr) {
			if (*hdr->szValue) mir_strcat (hdr->szValue, "; ");
			mir_strcat (hdr->szValue, nlhrReply->headers[i].szValue);
		}
		else nSize += (int)mir_strlen(nlhrReply->headers[i].szValue) + 2;
	}
	return nSize;
}

/*
   pszService: 
	service::login.skype.com::MBI_SSL   - For LoginSkypeOAuth
	service::ssl.live.com::MBI_SSL		- For ssl-compact-ticket
	service::contacts.msn.com::MBI_SSL	- Contact SOAP service -> authContactToken
	service::m.hotmail.com::MBI_SSL     - ActiveSync contactlist, not used by us
	service::storage.live.com::MBI_SSL  - Storage service (authStorageToken)
	service::skype.com::MBI_SSL			- Root of all OAuth tokens
	service::skype.net::MBI_SSL			- ?
*/
bool CMsnProto::RefreshOAuth(const char *pszRefreshToken, const char *pszService, char *pszAccessToken, char *pszOutRefreshToken, time_t *ptExpires)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[3];
	bool bRet = false;
	CMStringA post;

	if (!authCookies) return false;

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.nlc = hHttpsConnection;
	nlhr.headersCount = 3;
	nlhr.headers = headers;
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = (char*)MSN_USER_AGENT;
	nlhr.headers[1].szName = "Content-Type";
	nlhr.headers[1].szValue = "application/x-www-form-urlencoded";
	nlhr.headers[2].szName = "Cookie";
	nlhr.headers[2].szValue = authCookies;
	post.Format("client_id=00000000480BC46C&scope=%s&grant_type=refresh_token&refresh_token=%s",
		ptrA(mir_urlEncode(pszService)), pszRefreshToken);
	
	nlhr.pData = (char*)(const char*)post;
	nlhr.dataLength = (int)mir_strlen(nlhr.pData);
	nlhr.szUrl = "https://login.live.com/oauth20_token.srf";

	// Query
	mHttpsTS = clock();
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);
	mHttpsTS = clock();
	if (nlhrReply)  {
		hHttpsConnection = nlhrReply->nlc;
		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			char *p;
			bRet = true;

			if (pszAccessToken && (p=strstr(nlhrReply->pData, "\"access_token\":"))) 
				bRet &= sscanf(p+sizeof("\"access_token\""), "\"%[^\"]\"", pszAccessToken)==1;
			if (pszOutRefreshToken && (p=strstr(nlhrReply->pData, "\"refresh_token\":"))) 
				bRet &= sscanf(p+sizeof("\"refresh_token\""), "\"%[^\"]\"", pszOutRefreshToken)==1;
			if (ptExpires && (p=strstr(nlhrReply->pData, "\"expires_in\""))) {
				int expires;
				if (sscanf(p+sizeof("\"expires_in\""), "%d,", &expires) == 1) {
					time(ptExpires);
					*ptExpires+=expires;
					bRet&=true;
				}
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	} else hHttpsConnection = NULL;
	return bRet;
}

void CMsnProto::LoadAuthTokensDB(void)
{
	DBVARIANT dbv;

	authSkypeComToken.Load();
	authMethod = getDword("authMethod", 0);
	bPassportAuth = getBool("PassportAuth", true);
	if (getString("authUser", &dbv) == 0) {
		replaceStr(authUser, dbv.pszVal);
		db_free(&dbv);
	}
	authSSLToken.Load();
	authContactToken.Load();
	if (getString("authUIC", &dbv) == 0) {
		replaceStr(authUIC, dbv.pszVal);
		db_free(&dbv);
	}
	if (getString("authCookies", &dbv) == 0) {
		replaceStr(authCookies, dbv.pszVal);
		db_free(&dbv);
	}
	authStrToken.Load();
	if (getString("hotSecretToken", &dbv) == 0) {
		replaceStr(hotSecretToken, dbv.pszVal);
		db_free(&dbv);
	}
	if (getString("hotAuthToken", &dbv) == 0) {
		free(hotAuthToken);
		hotAuthToken = strdup(dbv.pszVal);
		db_free(&dbv);
	}
	authStorageToken.Load();
	if (getString("authRefreshToken", &dbv) == 0) {
		replaceStr(authRefreshToken, dbv.pszVal);
		db_free(&dbv);
	}
	authSkypeToken.Load();
}

void CMsnProto::SaveAuthTokensDB(void)
{
	setDword("authMethod", authMethod);
	setString("authUser", authUser);
	setString("authUIC", authUIC);
	setString("authCookies", authCookies);
	setString("authRefreshToken", authRefreshToken);
}

typedef struct {
	/* Internal */
	IEEmbed *pEmbed;
	/* Input */
	CMsnProto *pProto;
	NETLIBHTTPREQUEST *nlhr;
	NETLIBHTTPREQUEST *nlhrReply;
	/* Output */
	char *pszURL;
	char *pszCookies;
} IEAUTH_PARAM;

LRESULT CALLBACK AuthWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			IEAUTH_PARAM *pAuth = (IEAUTH_PARAM*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (pAuth && pAuth->pEmbed) pAuth->pEmbed->ResizeBrowser();
			return(0);
		}

		case WM_CREATE:
		{
			IEAUTH_PARAM *pAuth = (IEAUTH_PARAM*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pAuth);
			pAuth->pEmbed = new IEEmbed(hwnd);
			WCHAR *pwszCookies = mir_a2u(pAuth->nlhr->headers[1].szValue);
			pAuth->pEmbed->addCookie(pwszCookies);
			pAuth->pEmbed->navigate(AUTH_URL);
			mir_free(pwszCookies);
			return(0);
		}

		case UM_DOCCOMPLETE:
		{
			if (!lParam) return 1;
			IEAUTH_PARAM *pAuth = (IEAUTH_PARAM*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			if (wcsstr((WCHAR*)lParam, L"oauth20_authorize")) {
				char *pszDoc = pAuth->pEmbed->GetHTMLDoc();
				CMStringA post;

				if (pszDoc) {
					if (pAuth->pProto->parseLoginPage(pszDoc, pAuth->nlhr, &post)) {
						pAuth->pEmbed->navigate(pAuth->nlhr);
					}
					mir_free(pszDoc);
				}
			}
			else if (wcsstr((WCHAR*)lParam, L"access_token=")) {
				DWORD cbCookie = 0;

				pAuth->pszURL = mir_u2a((WCHAR*)lParam);

				/* get_cookie doesn't give us all the necessary cookies, therefore we need to use
				 * InternetGetCookieExA
				 */
				if (!fpInternetGetCookieExA) {
					HMODULE hMod = LoadLibrary(_T("wininet.dll"));
					if (hMod) fpInternetGetCookieExA = (pfnInternetGetCookieExA)GetProcAddress(hMod, "InternetGetCookieExA");
				}
				if (fpInternetGetCookieExA &&
					fpInternetGetCookieExA("https://login.live.com", NULL, NULL, &cbCookie, INTERNET_COOKIE_HTTPONLY, NULL) &&
					(pAuth->pszCookies = (char*)mir_alloc(cbCookie))) {
					fpInternetGetCookieExA("https://login.live.com", NULL, pAuth->pszCookies, &cbCookie, INTERNET_COOKIE_HTTPONLY, NULL);
				}
				else pAuth->pszCookies = mir_u2a(pAuth->pEmbed->getCookies());			
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			else if (wcsstr((WCHAR*)lParam, L"res=cancel") || wcsstr((WCHAR*)lParam, L"access_denied")) {
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			return(0);
		}

		case WM_CLOSE:
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
		{
			IEAUTH_PARAM *pAuth = (IEAUTH_PARAM*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (pAuth && pAuth->pEmbed) delete pAuth->pEmbed;
			return(0);
		}

	}

	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}


void __cdecl CMsnProto::msn_IEAuthThread(void *pParam)
{
	HWND hWnd;
	MSG msg;
    WNDCLASSEX wc={0};
    static const TCHAR  *ClassName = _T("SkypeLoginWindow");

    CoInitialize(NULL);

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = sizeof(void*);
    wc.hInstance = g_hInst;
    wc.lpfnWndProc = AuthWindowProc;
    wc.lpszClassName = ClassName;
    RegisterClassEx(&wc);

    if ((hWnd = CreateWindowEx(0, ClassName, _T("MSN Login"), WS_OVERLAPPEDWINDOW,
                                            CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
                                            HWND_DESKTOP, NULL, g_hInst, pParam))) {
		ShowWindow( hWnd, SW_SHOW );
		UpdateWindow( hWnd );

		while( GetMessage(&msg, NULL, 0, 0) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	UnregisterClass(ClassName, g_hInst);
	CoUninitialize();
}

bool CMsnProto::parseLoginPage(char *pszHTML, NETLIBHTTPREQUEST *nlhr, CMStringA *post)
{
	char *pPPFT, *pEnd;

	/* Get PPFT */
	if ((pPPFT = strstr(pszHTML, "name=\"PPFT\"")) && (pPPFT = strstr(pPPFT, "value=\"")) && (pEnd=strchr(pPPFT+7, '"'))) {
		*pEnd=0;
		pPPFT+=7;

		/* Get POST URL if available */
		if ((nlhr->szUrl = strstr(pszHTML, "urlPost:'")) && (pEnd=strchr(nlhr->szUrl+9, '\''))) {
			*pEnd=0;
			nlhr->szUrl += 9;
		} else nlhr->szUrl = POST_URL;

		/* Create POST data */
		char szPassword[100];
		if (db_get_static(NULL, m_szModuleName, "Password", szPassword, sizeof(szPassword)))
			return false;
		szPassword[16] = 0;
		post->Format("PPFT=%s&login=%s&passwd=%s", ptrA(mir_urlEncode(pPPFT)), 
			ptrA(mir_urlEncode(MyOptions.szEmail)), ptrA(mir_urlEncode(szPassword)));

		/* Do the login and get the required tokens */
		nlhr->dataLength = post->GetLength();
		nlhr->pData = (char*)post->GetString();
		return true;
	}
	return false;
}

// -1 - Refresh failed
//  0 - No need to refresh
//  1 - Refresh succeeded
int CMsnProto::MSN_RefreshOAuthTokens(bool bJustCheck)
{
	time_t t;
	int i=authMethod==2?0:1, iRet;
	OAuthToken *tokens[] = {&authStrToken, &authContactToken, &authStorageToken, &authSSLToken, &authSkypeComToken};

	time(&t);
	if (bJustCheck) {
		for (;i<sizeof(tokens)/sizeof(tokens[0]);i++) 
			if (tokens[i]->Expired(t)) return 1;
		return 0;
	}

	for (iRet=0;i<sizeof(tokens)/sizeof(tokens[0]);i++) {
		if (tokens[i]->Expired())
			iRet=tokens[i]->Refresh(true)?1:-1;
	}
	return iRet;
}

void CMsnProto::MSN_SendATH(ThreadData* info)
{
	if (MyOptions.netId!=NETID_SKYPE) {
		/* MSN account login */

		switch (authMethod)
		{
		case 1: 
			info->sendPacketPayload("ATH", "CON\\USER",
				"<user><ssl-compact-ticket>t=%s</ssl-compact-ticket>"
				"<uic>%s</uic>"
				"<id>%s</id><alias>%s</alias></user>\r\n", 
				(const char*)authSSLToken ? ptrA(HtmlEncode(authSSLToken)) : "", 
				authUIC, 
				GetMyUsername(NETID_MSN), GetMyUsername(NETID_SKYPE));
			break;

		case 2: 
			info->sendPacketPayload("ATH", "CON\\USER",
				"<user><ssl-compact-ticket>%s</ssl-compact-ticket>"
				"<uic>%s</uic>"
				"<ssl-site-name>chatservice.live.com</ssl-site-name>"
				"</user>\r\n", 
				(const char*)authStrToken ? ptrA(HtmlEncode(authStrToken)) : "",
				authUIC);
			break;
		}

	} else {
		info->sendPacketPayload("ATH", "CON\\USER",
			"<user><uic>%s</uic><id>%s</id></user>\r\n", 
			authUIC, MyOptions.szEmail);
	}
}

// -1 - Error on login sequence 
//  0 - Login failed (invalid username?)
//  1 - Login via Skype login server succeeded
//  2 - Login via Skypeweb succeeded
int CMsnProto::MSN_AuthOAuth(void)
{
	int retVal = -1;
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[3];

	if (bAskingForAuth) return 0;

	// Is there already a valid token and we can skip this?
	if (!authSkypeComToken.Expired() && !mir_strcmp(authUser, MyOptions.szEmail)) {
		MSN_RefreshOAuthTokens(false);
		return authMethod;
	}

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.nlc = hHttpsConnection;
	nlhr.headersCount = 1;
	nlhr.headers = headers;
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = (char*)MSN_USER_AGENT;

	// Get oauth20 login data
	nlhr.szUrl = AUTH_URL;
	mHttpsTS = clock();
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);
	mHttpsTS = clock();

	if (nlhrReply)  {
		hHttpsConnection = nlhrReply->nlc;

		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			CMStringA post;

			/* Get POST-Data and URL */
			if (parseLoginPage(nlhrReply->pData, &nlhr, &post)) {
				/* Get Cookies */
				nlhr.headers[1].szValue = (char*)alloca(CopyCookies(nlhrReply, NULL));
				CopyCookies(nlhrReply, &nlhr.headers[1]);
				if (*nlhr.headers[1].szValue) nlhr.headersCount++;

				/* Setup headers */
				nlhr.headers[nlhr.headersCount].szName = "Content-Type";
				nlhr.headers[nlhr.headersCount++].szValue = "application/x-www-form-urlencoded";

				/* Do the login and get the required tokens */
				nlhr.requestType = REQUEST_POST;
				nlhr.flags &= (~NLHRF_REDIRECT);
				mHttpsTS = clock();
				nlhr.nlc = hHttpsConnection;
				NETLIBHTTPREQUEST *nlhrReply2 = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);
				mHttpsTS = clock();
				if (nlhrReply2) {
					char *pszURL=NULL, *pAccessToken, *pEnd;
					hHttpsConnection = nlhrReply2->nlc;

					bPassportAuth = true;
					
					if (nlhrReply2->resultCode == 302) {
						/* Extract access_token from Location can be found */
						for (int i = 0; i < nlhrReply2->headersCount; i++) {
							if (!mir_strcmpi(nlhrReply2->headers[i].szName, "Location")) {
								pszURL = nlhrReply2->headers[i].szValue;
								break;
							}
						}
					}
					else {
						/* There may be a problem with login, i.e. M$ security measures. Open up browser
						 * window in order to let user login there. May also be used for 2-factor auth */
						if (nlhrReply2->resultCode == 200 && nlhrReply2->pData) {
							UINT uThreadId;
							IEAUTH_PARAM param = {NULL, this, &nlhr, nlhrReply2, NULL, NULL};

							bAskingForAuth = true;
							WaitForSingleObject(ForkThreadEx(&CMsnProto::msn_IEAuthThread, &param, &uThreadId), INFINITE);
							pszURL = param.pszURL;
							mir_free(authCookies);
							authCookies = nlhr.headers[1].szValue = param.pszCookies;
							CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply2);
							nlhrReply2 = NULL;
							bAskingForAuth = false;
							bPassportAuth = false;
						}
					}
					setByte("PassportAuth", bPassportAuth);
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
					nlhrReply = nlhrReply2;

					if (pszURL &&
						(pAccessToken = strstr(pszURL, "access_token=")) &&
						(pEnd = strchr(pAccessToken + 13, '&'))) {
						char *pRefreshToken, *pExpires;
						bool bLogin = false;

						pRefreshToken = strstr(pszURL, "refresh_token=");
						pExpires = strstr(pszURL, "expires_in=");

						*pEnd = 0;
						pAccessToken += 13;
						UrlDecode(pAccessToken);

						/* Extract refresh token */
						if (pRefreshToken && (pEnd = strchr(pRefreshToken + 14, '&'))) {
							*pEnd = 0;
							pRefreshToken += 14;
						}
						replaceStr(authRefreshToken, pRefreshToken);

						/* Extract expire time */
						time_t authTokenExpiretime;
						time(&authTokenExpiretime);
						if (pExpires && (pEnd = strchr(pExpires + 11, '&'))) {
							*pEnd = 0;
							pExpires += 11;
							authTokenExpiretime += atoi(pExpires);
						}
						else authTokenExpiretime += 86400;
						authSkypeComToken.SetToken(pAccessToken, authTokenExpiretime);

						/* Copy auth Cookies to class for other web requests like contact list fetching to avoid ActiveSync */
						if (nlhrReply) {
							mir_free(authCookies);
							authCookies = nlhr.headers[1].szValue = (char*)mir_alloc(CopyCookies(nlhrReply, NULL));
							CopyCookies(nlhrReply, &nlhr.headers[1]);
						}

						int loginRet;
						/* Do Login via Skype login server, if not possible switch to SkypeWebExperience login */
						if ((loginRet = LoginSkypeOAuth(pRefreshToken)) < 1) {
							if (loginRet < 0) bLogin = true; else retVal = 0;
						}
						else {
							/* SkyLogin succeeded, request required tokens */
							if (authSSLToken.Refresh(true)) {
								replaceStr(authUser, MyOptions.szEmail);
								authMethod = retVal = 1;
							}
						}
						authSkypeToken.Clear();


						/* If you need Skypewebexperience login, as i.e. skylogin.dll is not available, we do this here */
						if (bLogin) {
							/* Prepare headers*/
							nlhr.headers[2].szValue = "application/json";
							nlhr.pData = "{\"trouterurl\":\"https://\",\"connectionid\":\"a\"}";
							nlhr.dataLength = (int)mir_strlen(nlhr.pData);
							nlhr.szUrl = "https://skypewebexperience.live.com/v1/User/Initialization";
							nlhr.nlc = hHttpsConnection;

							/* Request MappingContainer */
							mHttpsTS = clock();
							if (nlhrReply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
							nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);
							mHttpsTS = clock();
							if (nlhrReply) {
								hHttpsConnection = nlhrReply->nlc;

								if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
									/* Parse JSON stuff for MappingContainer */
									char *pMappingContainer;

									if ((pMappingContainer = strstr(nlhrReply->pData, "\"MappingContainer\":\"")) &&
										(pEnd = strchr(pMappingContainer + 20, '"'))) {
										*pEnd = 0;
										pMappingContainer += 20;
										UrlDecode(pMappingContainer);
										replaceStr(authUIC, pMappingContainer);
										replaceStr(authUser, MyOptions.szEmail);
										authMethod = retVal = 2;
									}
									else retVal = 0;
								}
								else retVal = 0;
							}
							else hHttpsConnection = NULL;
						}
					}
				}
				else hHttpsConnection = NULL;
			}
		}
		if (nlhrReply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	} else hHttpsConnection = NULL;

	if (retVal<=0) authSkypeComToken.Clear(); else {
		if (bPassportAuth) {
			// Fast authentication with just 1 SOAP call supported :)
			MSN_GetPassportAuth();
		} else {
			// Slow authentication required by fetching multiple tokens, i.e. 2-factor auth :(
			if (authMethod == 2) authStrToken.Refresh();
			authContactToken.Refresh();
			authStorageToken.Refresh();
		}
		SaveAuthTokensDB();
	}
	return retVal;
}

int	 CMsnProto::GetMyNetID(void)
{
	return strchr(MyOptions.szEmail, '@')?NETID_MSN:NETID_SKYPE;
}

const char *CMsnProto::GetMyUsername(int netId)
{
	static char szPartner[128];

	if (netId == NETID_SKYPE)
	{
		if (MyOptions.netId==NETID_MSN)
		{
			if (db_get_static(NULL, m_szModuleName, "SkypePartner", szPartner, sizeof(szPartner)) == 0)
				return szPartner;
		}
	}
	return MyOptions.szEmail;
}

void CMsnProto::FreeAuthTokens(void)
{
	mir_free(pAuthToken);
	mir_free(tAuthToken);
	mir_free(oimSendToken);
	mir_free(authSecretToken);
	mir_free(hotSecretToken);
	mir_free(authUIC);
	mir_free(authCookies);
	mir_free(authUser);
	mir_free(authRefreshToken);
	free(hotAuthToken);
}
