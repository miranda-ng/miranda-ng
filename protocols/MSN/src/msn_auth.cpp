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

#include "msn_global.h"
#include "msn_proto.h"
#include "des.h"

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
						"<wsa:Address>messengerclear.live.com</wsa:Address>"
					"</wsa:EndpointReference>"
				"</wsp:AppliesTo>"
				"<wsp:PolicyReference URI=\"MBI_KEY_OLD\" />"
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
// Performs the MSN Passport login via TLS

int CMsnProto::MSN_GetPassportAuth(void)
{
	int retVal = -1;

	char szPassword[100];
	db_get_static(NULL, m_szModuleName, "Password", szPassword, sizeof(szPassword));
	szPassword[16] = 0;

	time_t ts = time(NULL);

	TCHAR szTs1[64], szTs2[64];
	tmi.printTimeStamp(UTC_TIME_HANDLE, ts, _T("I"), szTs1, SIZEOF(szTs1), 0);
	tmi.printTimeStamp(UTC_TIME_HANDLE, ts + 20 * 60, _T("I"), szTs2, SIZEOF(szTs2), 0);

	CMStringA szAuthInfo(FORMAT, authPacket, int(ts), MyOptions.szEmail, ptrA(HtmlEncode(szPassword)), szTs1, szTs2);

	char* szPassportHost = (char*)mir_alloc(256);
	if (db_get_static(NULL, m_szModuleName, "MsnPassportHost", szPassportHost, 256))
		strcpy(szPassportHost, defaultPassportUrl);

	bool defaultUrlAllow = strcmp(szPassportHost, defaultPassportUrl) != 0;
	char *tResult = NULL;

	while (retVal == -1) {
		unsigned status;

		tResult = getSslResult(&szPassportHost, szAuthInfo, NULL, status);
		if (tResult == NULL) {
			if (defaultUrlAllow) {
				strcpy(szPassportHost, defaultPassportUrl);
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
				ezxml_t xml = ezxml_parse_str(tResult, strlen(tResult));

				ezxml_t tokr = ezxml_get(xml, "S:Body", 0,
					"wst:RequestSecurityTokenResponseCollection", 0,
					"wst:RequestSecurityTokenResponse", -1);

				while (tokr != NULL) {
					ezxml_t toks = ezxml_get(tokr, "wst:RequestedSecurityToken", 0,
						"wsse:BinarySecurityToken", -1);

					const char* addr = ezxml_txt(ezxml_get(tokr, "wsp:AppliesTo", 0,
						"wsa:EndpointReference", 0, "wsa:Address", -1));

					if (strcmp(addr, "http://Passport.NET/tb") == 0) {
						ezxml_t node = ezxml_get(tokr, "wst:RequestedSecurityToken", 0, "EncryptedData", -1);
						free(hotAuthToken);
						hotAuthToken = ezxml_toxml(node, 0);

						node = ezxml_get(tokr, "wst:RequestedProofToken", 0, "wst:BinarySecret", -1);
						replaceStr(hotSecretToken, ezxml_txt(node));
					}
					else if (strcmp(addr, "messengerclear.live.com") == 0) {
						ezxml_t node = ezxml_get(tokr, "wst:RequestedProofToken", 0,
							"wst:BinarySecret", -1);
						if (toks) {
							replaceStr(authStrToken, ezxml_txt(toks));
							replaceStr(authSecretToken, ezxml_txt(node));
							retVal = 0;
						}
						else {
							errurl = ezxml_txt(ezxml_get(tokr, "S:Fault", 0, "psf:pp", 0, "psf:flowurl", -1));
						}
					}
					else if (strcmp(addr, "messenger.msn.com") == 0 && toks) {
						const char* tok = ezxml_txt(toks);
						char* ch = (char*)strchr(tok, '&');
						*ch = 0;
						replaceStr(tAuthToken, tok + 2);
						replaceStr(pAuthToken, ch + 3);
						*ch = '&';
					}
					else if (strcmp(addr, "contacts.msn.com") == 0 && toks) {
						replaceStr(authContactToken, ezxml_txt(toks));
					}
					else if (strcmp(addr, "messengersecure.live.com") == 0 && toks) {
						replaceStr(oimSendToken, ezxml_txt(toks));
					}
					else if (strcmp(addr, "storage.msn.com") == 0 && toks) {
						replaceStr(authStorageToken, ezxml_txt(toks));
					}

					tokr = ezxml_next(tokr);
				}

				if (retVal != 0) {
					if (errurl) {
						debugLogA("Starting URL: '%s'", errurl);
						CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)errurl);
					}

					ezxml_t tokf = ezxml_get(xml, "S:Body", 0, "S:Fault", 0, "S:Detail", -1);
					ezxml_t tokrdr = ezxml_child(tokf, "psf:redirectUrl");
					if (tokrdr != NULL) {
						strcpy(szPassportHost, ezxml_txt(tokrdr));
						debugLogA("Redirected to '%s'", szPassportHost);
					}
					else {
						const char* szFault = ezxml_txt(ezxml_get(tokf, "psf:error", 0, "psf:value", -1));
						retVal = strcmp(szFault, "0x80048821") == 0 ? 3 : (tokf ? 5 : 7);
						if (retVal != 3 && defaultUrlAllow) {
							strcpy(szPassportHost, defaultPassportUrl);
							defaultUrlAllow = false;
							retVal = -1;
						}
						else if (retVal != 3 && retVal != 7) {
							char err[512];
							mir_snprintf(err, SIZEOF(err), "Unknown Authentication error: %s", szFault);
							MSN_ShowError(err);
						}
					}
				}

				ezxml_free(xml);
			}
			break;

		default:
			if (defaultUrlAllow) {
				strcpy(szPassportHost, defaultPassportUrl);
				defaultUrlAllow = false;
			}
			else
				retVal = 6;
		}
		mir_free(tResult);
	}

	if (retVal != 0) {
		if (!Miranda_Terminated()) {
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
	BYTE hash1[MIR_SHA1_HASH_SIZE];
	BYTE hash2[MIR_SHA1_HASH_SIZE];
	BYTE hash3[MIR_SHA1_HASH_SIZE];
	BYTE hash4[MIR_SHA1_HASH_SIZE];

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
	BYTE *key1 = (BYTE*)mir_base64_decode(authSecretToken, &key1len);

	BYTE key2[MIR_SHA1_HASH_SIZE + 4];
	BYTE key3[MIR_SHA1_HASH_SIZE + 4];

	static const unsigned char encdata1[] = "WS-SecureConversationSESSION KEY HASH";
	static const unsigned char encdata2[] = "WS-SecureConversationSESSION KEY ENCRYPTION";

	derive_key(key2, key1, key1len, (unsigned char*)encdata1, sizeof(encdata1) - 1);
	derive_key(key3, key1, key1len, (unsigned char*)encdata2, sizeof(encdata2) - 1);

	size_t chllen = strlen(challenge);

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
	CallService(MS_UTILS_GETRANDOM, sizeof(iv), (LPARAM)iv);

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
	CallService(MS_UTILS_GETRANDOM, sizeof(nonce), (LPARAM)nonce);

	unsigned key1len;
	BYTE *key1 = (BYTE*)mir_base64_decode(hotSecretToken, &key1len);

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

void CMsnProto::FreeAuthTokens(void)
{
	mir_free(pAuthToken);
	mir_free(tAuthToken);
	mir_free(oimSendToken);
	mir_free(authStrToken);
	mir_free(authSecretToken);
	mir_free(authContactToken);
	mir_free(authStorageToken);
	mir_free(hotSecretToken);
	free(hotAuthToken);
}
