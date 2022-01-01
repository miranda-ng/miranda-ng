/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "netlib.h"

#define SECURITY_WIN32
#include <security.h>
#include <rpcdce.h>

#pragma comment(lib, "secur32.lib")

struct NtlmHandleType
{
	CtxtHandle hClientContext;
	CredHandle hClientCredential;
	wchar_t* szProvider;
	wchar_t* szPrincipal;
	unsigned cbMaxToken;
	bool hasDomain;
};

struct NTLM_String
{
	uint16_t     len;
	uint16_t     allocedSpace;
	uint32_t    offset;
};

struct NtlmType2packet
{
	char        sign[8];
	uint32_t       type;   // == 2
	NTLM_String targetName;
	uint32_t       flags;
	uint8_t        challenge[8];
	uint8_t        context[8];
	NTLM_String targetInfo;
};

static unsigned ntlmCnt = 0;
static mir_cs csSec;

static void ReportSecError(SECURITY_STATUS scRet, int line)
{
	wchar_t szMsgBuf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, scRet, LANG_USER_DEFAULT, szMsgBuf, _countof(szMsgBuf), nullptr);

	wchar_t *p = wcschr(szMsgBuf, 13); if (p) *p = 0;

	Netlib_LogfW(nullptr, L"Security error 0x%x on line %u (%s)", scRet, line, szMsgBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HANDLE) Netlib_InitSecurityProvider(const wchar_t *szProvider, const wchar_t *szPrincipal)
{
	HANDLE hSecurity = nullptr;

	if (mir_wstrcmpi(szProvider, L"Basic") == 0) {
		NtlmHandleType* hNtlm = (NtlmHandleType*)mir_calloc(sizeof(NtlmHandleType));
		hNtlm->szProvider = mir_wstrdup(szProvider);
		SecInvalidateHandle(&hNtlm->hClientContext);
		SecInvalidateHandle(&hNtlm->hClientCredential);
		ntlmCnt++;

		return hNtlm;
	}

	mir_cslock lck(csSec);

	PSecPkgInfo ntlmSecurityPackageInfo;
	SECURITY_STATUS sc = QuerySecurityPackageInfo((LPTSTR)szProvider, &ntlmSecurityPackageInfo);
	if (sc == SEC_E_OK) {
		NtlmHandleType* hNtlm;

		hSecurity = hNtlm = (NtlmHandleType*)mir_calloc(sizeof(NtlmHandleType));
		hNtlm->cbMaxToken = ntlmSecurityPackageInfo->cbMaxToken;
		FreeContextBuffer(ntlmSecurityPackageInfo);

		hNtlm->szProvider = mir_wstrdup(szProvider);
		hNtlm->szPrincipal = mir_wstrdup(szPrincipal ? szPrincipal : L"");
		SecInvalidateHandle(&hNtlm->hClientContext);
		SecInvalidateHandle(&hNtlm->hClientCredential);
		ntlmCnt++;
	}
	return hSecurity;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Netlib_DestroySecurityProvider(HANDLE hSecurity)
{
	if (hSecurity == nullptr)
		return;

	mir_cslock lck(csSec);

	if (ntlmCnt != 0) {
		NtlmHandleType* hNtlm = (NtlmHandleType*)hSecurity;
		if (hNtlm != nullptr) {
			if (SecIsValidHandle(&hNtlm->hClientContext))
				DeleteSecurityContext(&hNtlm->hClientContext);
			if (SecIsValidHandle(&hNtlm->hClientCredential))
				FreeCredentialsHandle(&hNtlm->hClientCredential);
			mir_free(hNtlm->szProvider);
			mir_free(hNtlm->szPrincipal);
			mir_free(hNtlm);
		}

		--ntlmCnt;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

char* CompleteGssapi(HANDLE hSecurity, unsigned char *szChallenge, unsigned chlsz)
{
	if (!szChallenge || !szChallenge[0]) return nullptr;

	NtlmHandleType* hNtlm = (NtlmHandleType*)hSecurity;
	unsigned char inDataBuffer[1024];

	SecBuffer inBuffers[2] =
	{
		{ sizeof(inDataBuffer), SECBUFFER_DATA, inDataBuffer },
		{ chlsz, SECBUFFER_STREAM, szChallenge }
	};

	SecBufferDesc inBuffersDesc = { SECBUFFER_VERSION, 2, inBuffers };

	unsigned long qop = 0;
	SECURITY_STATUS sc = DecryptMessage(&hNtlm->hClientContext, &inBuffersDesc, 0, &qop);
	if (sc != SEC_E_OK) {
		ReportSecError(sc, __LINE__);
		return nullptr;
	}

	// unsigned char LayerMask = inDataBuffer[0];
	// unsigned int MaxMessageSize = htonl(*(unsigned*)&inDataBuffer[1]);

	SecPkgContext_Sizes sizes;
	sc = QueryContextAttributes(&hNtlm->hClientContext, SECPKG_ATTR_SIZES, &sizes);
	if (sc != SEC_E_OK) {
		ReportSecError(sc, __LINE__);
		return nullptr;
	}

	unsigned char *tokenBuffer = (unsigned char*)alloca(sizes.cbSecurityTrailer);
	unsigned char *paddingBuffer = (unsigned char*)alloca(sizes.cbBlockSize);

	unsigned char outDataBuffer[4] = { 1, 0, 16, 0 };

	SecBuffer outBuffers[3] =
	{
		{ sizes.cbSecurityTrailer, SECBUFFER_TOKEN, tokenBuffer },
		{ sizeof(outDataBuffer), SECBUFFER_DATA, outDataBuffer },
		{ sizes.cbBlockSize, SECBUFFER_PADDING, paddingBuffer }
	};
	SecBufferDesc outBuffersDesc = { SECBUFFER_VERSION, 3, outBuffers };

	sc = EncryptMessage(&hNtlm->hClientContext, SECQOP_WRAP_NO_ENCRYPT, &outBuffersDesc, 0);
	if (sc != SEC_E_OK) {
		ReportSecError(sc, __LINE__);
		return nullptr;
	}

	unsigned i, ressz = 0;
	for (i = 0; i < outBuffersDesc.cBuffers; i++)
		ressz += outBuffersDesc.pBuffers[i].cbBuffer;

	unsigned char *response = (unsigned char*)alloca(ressz), *p = response;
	for (i = 0; i < outBuffersDesc.cBuffers; i++) {
		memcpy(p, outBuffersDesc.pBuffers[i].pvBuffer, outBuffersDesc.pBuffers[i].cbBuffer);
		p += outBuffersDesc.pBuffers[i].cbBuffer;
	}

	return mir_base64_encode(response, ressz);
}

/////////////////////////////////////////////////////////////////////////////////////////

char* NtlmCreateResponseFromChallenge(HANDLE hSecurity, const char *szChallenge, const wchar_t *login, const wchar_t *psw, bool http, unsigned &complete)
{
	if (hSecurity == nullptr || ntlmCnt == 0)
		return nullptr;

	SecBufferDesc outputBufferDescriptor, inputBufferDescriptor;
	SecBuffer outputSecurityToken, inputSecurityToken;
	char *szOutputToken;

	NtlmHandleType *hNtlm = (NtlmHandleType*)hSecurity;

	Netlib_Logf(nullptr, "NtlmCreateResponseFromChallenge (%s): chl=%s {%S:%S} => %d", hNtlm->szProvider, szChallenge, login, psw, complete);

	if (mir_wstrcmpi(hNtlm->szProvider, L"Basic")) {
		bool isGSSAPI = mir_wstrcmpi(hNtlm->szProvider, L"Kerberos") == 0;
		bool hasChallenge = szChallenge != nullptr && szChallenge[0] != '\0';
		if (hasChallenge) {
			size_t tokenLen;
			uint8_t *token = (uint8_t*)mir_base64_decode(szChallenge, &tokenLen);
			if (token == nullptr)
				return nullptr;

			if (isGSSAPI && complete)
				return CompleteGssapi(hSecurity, token, (unsigned)tokenLen);

			inputBufferDescriptor.cBuffers = 1;
			inputBufferDescriptor.pBuffers = &inputSecurityToken;
			inputBufferDescriptor.ulVersion = SECBUFFER_VERSION;
			inputSecurityToken.BufferType = SECBUFFER_TOKEN;
			inputSecurityToken.cbBuffer = (unsigned)tokenLen;
			inputSecurityToken.pvBuffer = token;

			// try to decode the domain name from the NTLM challenge
			if (login != nullptr && login[0] != '\0' && !hNtlm->hasDomain) {
				NtlmType2packet* pkt = (NtlmType2packet*)token;
				if (!strncmp(pkt->sign, "NTLMSSP", 8) && pkt->type == 2) {

					wchar_t* domainName = (wchar_t*)&token[pkt->targetName.offset];
					int domainLen = pkt->targetName.len;

					// Negotiate ANSI? if yes, convert the ANSI name to unicode
					if ((pkt->flags & 1) == 0) {
						int bufsz = MultiByteToWideChar(CP_ACP, 0, (char*)domainName, domainLen, nullptr, 0);
						wchar_t* buf = (wchar_t*)alloca((bufsz+1) * sizeof(wchar_t));
						domainLen = MultiByteToWideChar(CP_ACP, 0, (char*)domainName, domainLen, buf, bufsz) - 1;
						buf[domainLen] = 0;
						domainName = buf;
					}
					else domainLen /= sizeof(wchar_t);

					if (domainLen) {
						CMStringW wszNewLogin(FORMAT, L"%s\\%s", domainName, login);
						char* szChl = NtlmCreateResponseFromChallenge(hSecurity, nullptr, wszNewLogin, psw, http, complete);
						mir_free(szChl);
					}
				}
			}
		}
		else {
			if (SecIsValidHandle(&hNtlm->hClientContext))
				DeleteSecurityContext(&hNtlm->hClientContext);
			if (SecIsValidHandle(&hNtlm->hClientCredential))
				FreeCredentialsHandle(&hNtlm->hClientCredential);

			SEC_WINNT_AUTH_IDENTITY auth;

			if (login != nullptr && login[0] != '\0') {
				memset(&auth, 0, sizeof(auth));

				Netlib_Logf(nullptr, "Security login requested, user: %S pssw: %s", login, psw ? "(exist)" : "(no psw)");

				const wchar_t* loginName = login;
				const wchar_t* domainName = wcschr(login, '\\');
				size_t domainLen = 0;
				size_t loginLen = mir_wstrlen(loginName);
				if (domainName != nullptr) {
					loginName = domainName + 1;
					loginLen = mir_wstrlen(loginName);
					domainLen = domainName - login;
					domainName = login;
				}
				else if ((domainName = wcschr(login, '@')) != nullptr) {
					loginName = login;
					loginLen = domainName - login;
					domainLen = mir_wstrlen(++domainName);
				}

				auth.User = (PWORD)loginName;
				auth.UserLength = (ULONG)loginLen;
				auth.Password = (PWORD)psw;
				auth.PasswordLength = (ULONG)mir_wstrlen(psw);
				auth.Domain = (PWORD)domainName;
				auth.DomainLength = (ULONG)domainLen;
				auth.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

				hNtlm->hasDomain = domainLen != 0;
			}

			TimeStamp tokenExpiration;
			SECURITY_STATUS sc = AcquireCredentialsHandle(nullptr, hNtlm->szProvider, SECPKG_CRED_OUTBOUND, nullptr, hNtlm->hasDomain ? &auth : nullptr, nullptr, nullptr, &hNtlm->hClientCredential, &tokenExpiration);
			if (sc != SEC_E_OK) {
				ReportSecError(sc, __LINE__);
				return nullptr;
			}
		}

		outputBufferDescriptor.cBuffers = 1;
		outputBufferDescriptor.pBuffers = &outputSecurityToken;
		outputBufferDescriptor.ulVersion = SECBUFFER_VERSION;
		outputSecurityToken.BufferType = SECBUFFER_TOKEN;
		outputSecurityToken.cbBuffer = hNtlm->cbMaxToken;
		outputSecurityToken.pvBuffer = alloca(outputSecurityToken.cbBuffer);

		ULONG contextAttributes;
		TimeStamp tokenExpiration;
		SECURITY_STATUS sc = InitializeSecurityContext(&hNtlm->hClientCredential,
			hasChallenge ? &hNtlm->hClientContext : nullptr,
			hNtlm->szPrincipal, isGSSAPI ? ISC_REQ_MUTUAL_AUTH | ISC_REQ_STREAM : 0, 0, SECURITY_NATIVE_DREP,
			hasChallenge ? &inputBufferDescriptor : nullptr, 0, &hNtlm->hClientContext,
			&outputBufferDescriptor, &contextAttributes, &tokenExpiration);
		Netlib_Logf(nullptr, "InitializeSecurityContext(%S): 0x%x", hNtlm->szProvider, sc);

		complete = (sc != SEC_I_COMPLETE_AND_CONTINUE && sc != SEC_I_CONTINUE_NEEDED);
		if (sc == SEC_I_COMPLETE_NEEDED || sc == SEC_I_COMPLETE_AND_CONTINUE) {
			sc = CompleteAuthToken(&hNtlm->hClientContext, &outputBufferDescriptor);
			Netlib_Logf(nullptr, "CompleteAuthToken: 0x%x", sc);
		}

		if (sc != SEC_E_OK && sc != SEC_I_CONTINUE_NEEDED) {
			ReportSecError(sc, __LINE__);
			return nullptr;
		}

		szOutputToken = mir_base64_encode(outputSecurityToken.pvBuffer, outputSecurityToken.cbBuffer);
	}
	else {
		if (!login || !psw)
			return nullptr;

		T2Utf szAuth(CMStringW(FORMAT, L"%s:%s", login, psw));
		szOutputToken = mir_base64_encode(szAuth.get(), mir_strlen(szAuth));
		complete = true;
	}

	if (szOutputToken == nullptr)
		return nullptr;

	if (!http)
		return szOutputToken;

	CMStringA szResult(FORMAT, "%S %s", hNtlm->szProvider, szOutputToken);
	mir_free(szOutputToken);
	return szResult.Detach();
}

MIR_APP_DLL(char*) Netlib_NtlmCreateResponse(HANDLE hProvider, const char *szChallenge, wchar_t *pwszLogin, wchar_t *pwszPassword, unsigned &complete)
{
	return NtlmCreateResponseFromChallenge(hProvider, szChallenge, pwszLogin, pwszPassword, false, complete);
}
