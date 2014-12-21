/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include <m_popup.h>
#include "netlib.h"

#define SECURITY_WIN32
#include <security.h>
#include <schannel.h>

#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "crypt32.lib")

typedef BOOL (*SSL_EMPTY_CACHE_FN_M)(VOID);

static HMODULE g_hSchannel;
static PSecurityFunctionTable g_pSSPI;
static HANDLE g_hSslMutex;
static SSL_EMPTY_CACHE_FN_M MySslEmptyCache;
static CredHandle hCreds;
static bool bSslInitDone;

typedef enum
{
	sockOpen,
	sockClosed,
	sockError
} SocketState;


struct SslHandle
{
	SOCKET s;

	CtxtHandle hContext;

	BYTE *pbRecDataBuf;
	int cbRecDataBuf;
	int sbRecDataBuf;

	BYTE *pbIoBuffer;
	int cbIoBuffer;
	int sbIoBuffer;

	SocketState state;
};

static void ReportSslError(SECURITY_STATUS scRet, int line, bool showPopup = false)
{
	TCHAR szMsgBuf[256];
	switch (scRet) {
	case 0:
	case ERROR_NOT_READY:
		return;

	case SEC_E_INVALID_TOKEN:
		_tcscpy(szMsgBuf, TranslateT("Client cannot decode host message. Possible causes: host does not support SSL or requires not existing security package"));
		break;

	case CERT_E_CN_NO_MATCH:
	case SEC_E_WRONG_PRINCIPAL:
		_tcscpy(szMsgBuf, TranslateT("Host we are connecting to is not the one certificate was issued for"));
		break;

	default:
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, scRet, LANG_USER_DEFAULT, szMsgBuf, SIZEOF(szMsgBuf), NULL);
	}

	TCHAR szMsgBuf2[512];
	mir_sntprintf(szMsgBuf2, SIZEOF(szMsgBuf2), _T("SSL connection failure (%x %u): %s"), scRet, line, szMsgBuf);

	char* szMsg = Utf8EncodeT(szMsgBuf2);
	NetlibLogf(NULL, szMsg);
	mir_free(szMsg);

	SetLastError(scRet);
	PUShowMessageT(szMsgBuf2, SM_WARNING);
}

static bool AcquireCredentials(void)
{
	SCHANNEL_CRED   SchannelCred;
	TimeStamp       tsExpiry;
	SECURITY_STATUS scRet;

	memset(&SchannelCred, 0, sizeof(SchannelCred));

	SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
	SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3TLS1_CLIENTS;
	SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION;

	// Create an SSPI credential.
	scRet = g_pSSPI->AcquireCredentialsHandle(
		NULL,                   // Name of principal
		UNISP_NAME,             // Name of package
		SECPKG_CRED_OUTBOUND,   // Flags indicating use
		NULL,                   // Pointer to logon ID
		&SchannelCred,          // Package specific data
		NULL,                   // Pointer to GetKey() func
		NULL,                   // Value to pass to GetKey()
		&hCreds, 				// (out) Cred Handle
		&tsExpiry);             // (out) Lifetime (optional)

	ReportSslError(scRet, __LINE__);
	return scRet == SEC_E_OK;
}

static bool SSL_library_init(void)
{
	if (bSslInitDone)
		return true;

	WaitForSingleObject(g_hSslMutex, INFINITE);

	g_pSSPI = InitSecurityInterface();
	if (g_pSSPI) {
		g_hSchannel = LoadLibraryA("schannel.dll");
		if (g_hSchannel)
			MySslEmptyCache = (SSL_EMPTY_CACHE_FN_M)GetProcAddress(g_hSchannel, "SslEmptyCache");
		AcquireCredentials();
		bSslInitDone = true;
	}

	ReleaseMutex(g_hSslMutex);
	return bSslInitDone;
}

void NetlibSslFree(SslHandle *ssl)
{
	if (ssl == NULL) return;

	g_pSSPI->DeleteSecurityContext(&ssl->hContext);

	mir_free(ssl->pbRecDataBuf);
	mir_free(ssl->pbIoBuffer);
	memset(ssl, 0, sizeof(SslHandle));
	mir_free(ssl);
}

BOOL NetlibSslPending(SslHandle *ssl)
{
	return ssl != NULL && (ssl->cbRecDataBuf != 0 || ssl->cbIoBuffer != 0);
}

static bool VerifyCertificate(SslHandle *ssl, PCSTR pszServerName, DWORD dwCertFlags)
{
	static LPSTR rgszUsages[] =
	{
		szOID_PKIX_KP_SERVER_AUTH,
		szOID_SERVER_GATED_CRYPTO,
		szOID_SGC_NETSCAPE
	};

	CERT_CHAIN_PARA          ChainPara = { 0 };
	HTTPSPolicyCallbackData  polHttps = { 0 };
	CERT_CHAIN_POLICY_PARA   PolicyPara = { 0 };
	CERT_CHAIN_POLICY_STATUS PolicyStatus = { 0 };
	PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
	PCCERT_CONTEXT           pServerCert = NULL;
	DWORD scRet;

	PWSTR pwszServerName = mir_a2u(pszServerName);

	scRet = g_pSSPI->QueryContextAttributes(&ssl->hContext, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &pServerCert);
	if (scRet != SEC_E_OK)
		goto cleanup;

	if (pServerCert == NULL) {
		scRet = SEC_E_WRONG_PRINCIPAL;
		goto cleanup;
	}

	ChainPara.cbSize = sizeof(ChainPara);
	ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
	ChainPara.RequestedUsage.Usage.cUsageIdentifier = SIZEOF(rgszUsages);
	ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

	if (!CertGetCertificateChain(NULL, pServerCert, NULL, pServerCert->hCertStore, &ChainPara, 0, NULL, &pChainContext)) {
		scRet = GetLastError();
		goto cleanup;
	}

	polHttps.cbStruct = sizeof(HTTPSPolicyCallbackData);
	polHttps.dwAuthType = AUTHTYPE_SERVER;
	polHttps.fdwChecks = dwCertFlags;
	polHttps.pwszServerName = pwszServerName;

	PolicyPara.cbSize = sizeof(PolicyPara);
	PolicyPara.pvExtraPolicyPara = &polHttps;

	PolicyStatus.cbSize = sizeof(PolicyStatus);

	if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChainContext, &PolicyPara, &PolicyStatus)) {
		scRet = GetLastError();
		goto cleanup;
	}

	if (PolicyStatus.dwError) {
		scRet = PolicyStatus.dwError;
		goto cleanup;
	}

	scRet = SEC_E_OK;

cleanup:
	if (pChainContext)
		CertFreeCertificateChain(pChainContext);
	if (pServerCert)
		CertFreeCertificateContext(pServerCert);
	mir_free(pwszServerName);

	ReportSslError(scRet, __LINE__, true);
	return scRet == SEC_E_OK;
}

static SECURITY_STATUS ClientHandshakeLoop(SslHandle *ssl, BOOL fDoInitialRead)
{
	DWORD dwSSPIFlags =
		ISC_REQ_SEQUENCE_DETECT |
		ISC_REQ_REPLAY_DETECT |
		ISC_REQ_CONFIDENTIALITY |
		ISC_REQ_EXTENDED_ERROR |
		ISC_REQ_ALLOCATE_MEMORY |
		ISC_REQ_STREAM;

	ssl->cbIoBuffer = 0;

	BOOL fDoRead = fDoInitialRead;

	SECURITY_STATUS scRet = SEC_I_CONTINUE_NEEDED;

	// Loop until the handshake is finished or an error occurs.
	while (scRet == SEC_I_CONTINUE_NEEDED || scRet == SEC_E_INCOMPLETE_MESSAGE || scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
		// Read server data
		if (0 == ssl->cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
			if (fDoRead) {
				static const TIMEVAL tv = { 6, 0 };
				fd_set fd;

				// If buffer not large enough reallocate buffer
				if (ssl->sbIoBuffer <= ssl->cbIoBuffer) {
					ssl->sbIoBuffer += 4096;
					ssl->pbIoBuffer = (PUCHAR)mir_realloc(ssl->pbIoBuffer, ssl->sbIoBuffer);
				}

				FD_ZERO(&fd);
				FD_SET(ssl->s, &fd);
				if (select(1, &fd, NULL, NULL, &tv) != 1) {
					NetlibLogf(NULL, "SSL Negotiation failure recieving data (timeout) (bytes %u)", ssl->cbIoBuffer);
					scRet = ERROR_NOT_READY;
					break;
				}

				DWORD cbData = recv(ssl->s, (char*)ssl->pbIoBuffer + ssl->cbIoBuffer, ssl->sbIoBuffer - ssl->cbIoBuffer, 0);
				if (cbData == SOCKET_ERROR) {
					NetlibLogf(NULL, "SSL Negotiation failure recieving data (%d)", WSAGetLastError());
					scRet = ERROR_NOT_READY;
					break;
				}
				if (cbData == 0) {
					NetlibLogf(NULL, "SSL Negotiation connection gracefully closed");
					scRet = ERROR_NOT_READY;
					break;
				}

				NetlibDumpData(NULL, ssl->pbIoBuffer + ssl->cbIoBuffer, cbData, 0, MSG_DUMPSSL);
				ssl->cbIoBuffer += cbData;
			}
			else fDoRead = TRUE;
		}

		// Set up the input buffers. Buffer 0 is used to pass in data
		// received from the server. Schannel will consume some or all
		// of this. Leftover data (if any) will be placed in buffer 1 and
		// given a buffer type of SECBUFFER_EXTRA.

		SecBuffer InBuffers[2];
		InBuffers[0].pvBuffer = ssl->pbIoBuffer;
		InBuffers[0].cbBuffer = ssl->cbIoBuffer;
		InBuffers[0].BufferType = SECBUFFER_TOKEN;

		InBuffers[1].pvBuffer = NULL;
		InBuffers[1].cbBuffer = 0;
		InBuffers[1].BufferType = SECBUFFER_EMPTY;

		SecBufferDesc InBuffer;
		InBuffer.cBuffers = SIZEOF(InBuffers);
		InBuffer.pBuffers = InBuffers;
		InBuffer.ulVersion = SECBUFFER_VERSION;

		// Set up the output buffers. These are initialized to NULL
		// so as to make it less likely we'll attempt to free random
		// garbage later.

		SecBuffer OutBuffers[1];
		OutBuffers[0].pvBuffer = NULL;
		OutBuffers[0].BufferType = SECBUFFER_TOKEN;
		OutBuffers[0].cbBuffer = 0;

		SecBufferDesc OutBuffer;
		OutBuffer.cBuffers = SIZEOF(OutBuffers);
		OutBuffer.pBuffers = OutBuffers;
		OutBuffer.ulVersion = SECBUFFER_VERSION;

		TimeStamp tsExpiry;
		DWORD dwSSPIOutFlags;
		scRet = g_pSSPI->InitializeSecurityContext(&hCreds, &ssl->hContext, NULL, dwSSPIFlags, 0,
			SECURITY_NATIVE_DREP, &InBuffer, 0, NULL, &OutBuffer, &dwSSPIOutFlags, &tsExpiry);

		// If success (or if the error was one of the special extended ones),
		// send the contents of the output buffer to the server.
		if (scRet == SEC_E_OK || scRet == SEC_I_CONTINUE_NEEDED || (FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))) {
			if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
				NetlibDumpData(NULL, (unsigned char*)(OutBuffers[0].pvBuffer), OutBuffers[0].cbBuffer, 1, MSG_DUMPSSL);

				DWORD cbData = send(ssl->s, (char*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
				if (cbData == SOCKET_ERROR || cbData == 0) {
					NetlibLogf(NULL, "SSL Negotiation failure sending data (%d)", WSAGetLastError());
					g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
					return SEC_E_INTERNAL_ERROR;
				}

				// Free output buffer.
				g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
				OutBuffers[0].pvBuffer = NULL;
			}
		}

		// we need to read more data from the server and try again.
		if (scRet == SEC_E_INCOMPLETE_MESSAGE)
			continue;

		// handshake completed successfully.
		if (scRet == SEC_E_OK) {
			// Store remaining data for further use
			if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
				memmove(ssl->pbIoBuffer,
					ssl->pbIoBuffer + (ssl->cbIoBuffer - InBuffers[1].cbBuffer),
					InBuffers[1].cbBuffer);
				ssl->cbIoBuffer = InBuffers[1].cbBuffer;
			}
			else ssl->cbIoBuffer = 0;
			break;
		}

		// Check for fatal error.
		if (FAILED(scRet)) break;

		// server just requested client authentication.
		if (scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
			// Server has requested client authentication and
			// GetNewClientCredentials(ssl);

			// Go around again.
			fDoRead = FALSE;
			scRet = SEC_I_CONTINUE_NEEDED;
			continue;
		}

		// Copy any leftover data from the buffer, and go around again.
		if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
			memmove(ssl->pbIoBuffer, ssl->pbIoBuffer + (ssl->cbIoBuffer - InBuffers[1].cbBuffer), InBuffers[1].cbBuffer);
			ssl->cbIoBuffer = InBuffers[1].cbBuffer;
		}
		else ssl->cbIoBuffer = 0;
	}

	// Delete the security context in the case of a fatal error.
	ReportSslError(scRet, __LINE__);

	if (ssl->cbIoBuffer == 0) {
		mir_free(ssl->pbIoBuffer);
		ssl->pbIoBuffer = NULL;
		ssl->sbIoBuffer = 0;
	}

	return scRet;
}

static bool ClientConnect(SslHandle *ssl, const char *host)
{
	if (SecIsValidHandle(&ssl->hContext)) {
		g_pSSPI->DeleteSecurityContext(&ssl->hContext);
		SecInvalidateHandle(&ssl->hContext);
	}

	if (MySslEmptyCache) MySslEmptyCache();

	DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
		ISC_REQ_REPLAY_DETECT |
		ISC_REQ_CONFIDENTIALITY |
		ISC_REQ_EXTENDED_ERROR |
		ISC_REQ_ALLOCATE_MEMORY |
		ISC_REQ_STREAM;

	//  Initiate a ClientHello message and generate a token.
	SecBuffer OutBuffers[1];
	OutBuffers[0].pvBuffer = NULL;
	OutBuffers[0].BufferType = SECBUFFER_TOKEN;
	OutBuffers[0].cbBuffer = 0;

	SecBufferDesc OutBuffer;
	OutBuffer.cBuffers = SIZEOF(OutBuffers);
	OutBuffer.pBuffers = OutBuffers;
	OutBuffer.ulVersion = SECBUFFER_VERSION;

	TimeStamp tsExpiry;
	DWORD dwSSPIOutFlags;
	SECURITY_STATUS scRet = g_pSSPI->InitializeSecurityContext(&hCreds, NULL, _A2T(host), dwSSPIFlags, 0, 
		SECURITY_NATIVE_DREP, NULL, 0, &ssl->hContext, &OutBuffer, &dwSSPIOutFlags, &tsExpiry);
	if (scRet != SEC_I_CONTINUE_NEEDED) {
		ReportSslError(scRet, __LINE__);
		return 0;
	}

	// Send response to server if there is one.
	if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
		NetlibDumpData(NULL, (unsigned char*)(OutBuffers[0].pvBuffer), OutBuffers[0].cbBuffer, 1, MSG_DUMPSSL);
		
		DWORD cbData = send(ssl->s, (char*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
		if (cbData == SOCKET_ERROR || cbData == 0) {
			NetlibLogf(NULL, "SSL failure sending connection data (%d %d)", ssl->s, WSAGetLastError());
			g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
			return 0;
		}

		// Free output buffer.
		g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
		OutBuffers[0].pvBuffer = NULL;
	}

	return ClientHandshakeLoop(ssl, TRUE) == SEC_E_OK;
}

SslHandle* NetlibSslConnect(SOCKET s, const char* host, int verify)
{
	SslHandle *ssl = (SslHandle*)mir_calloc(sizeof(SslHandle));
	ssl->s = s;

	SecInvalidateHandle(&ssl->hContext);

	DWORD dwFlags = 0;

	if (!host || inet_addr(host) != INADDR_NONE)
		dwFlags |= 0x00001000;

	bool res = SSL_library_init();

	if (res) res = ClientConnect(ssl, host);
	if (res && verify) res = VerifyCertificate(ssl, host, dwFlags);

	if (!res) {
		NetlibSslFree(ssl);
		ssl = NULL;
	}
	return ssl;
}

void NetlibSslShutdown(SslHandle *ssl)
{
	if (ssl == NULL || !SecIsValidHandle(&ssl->hContext))
		return;

	DWORD dwType = SCHANNEL_SHUTDOWN;

	SecBuffer OutBuffers[1];
	OutBuffers[0].pvBuffer = &dwType;
	OutBuffers[0].BufferType = SECBUFFER_TOKEN;
	OutBuffers[0].cbBuffer = sizeof(dwType);

	SecBufferDesc OutBuffer;
	OutBuffer.cBuffers = SIZEOF(OutBuffers);
	OutBuffer.pBuffers = OutBuffers;
	OutBuffer.ulVersion = SECBUFFER_VERSION;

	SECURITY_STATUS scRet = g_pSSPI->ApplyControlToken(&ssl->hContext, &OutBuffer);
	if (FAILED(scRet))
		return;

	// Build an SSL close notify message.

	DWORD dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
		ISC_REQ_REPLAY_DETECT |
		ISC_REQ_CONFIDENTIALITY |
		ISC_RET_EXTENDED_ERROR |
		ISC_REQ_ALLOCATE_MEMORY |
		ISC_REQ_STREAM;

	OutBuffers[0].pvBuffer = NULL;
	OutBuffers[0].BufferType = SECBUFFER_TOKEN;
	OutBuffers[0].cbBuffer = 0;

	OutBuffer.cBuffers = 1;
	OutBuffer.pBuffers = OutBuffers;
	OutBuffer.ulVersion = SECBUFFER_VERSION;

	TimeStamp tsExpiry;
	DWORD dwSSPIOutFlags;
	scRet = g_pSSPI->InitializeSecurityContext(&hCreds, &ssl->hContext, NULL, dwSSPIFlags, 0, SECURITY_NATIVE_DREP, NULL, 0,
		&ssl->hContext, &OutBuffer, &dwSSPIOutFlags, &tsExpiry);
	if (FAILED(scRet))
		return;

	// Send the close notify message to the server.
	if (OutBuffers[0].pvBuffer != NULL && OutBuffers[0].cbBuffer != 0) {
		NetlibDumpData(NULL, (unsigned char*)(OutBuffers[0].pvBuffer), OutBuffers[0].cbBuffer, 1, MSG_DUMPSSL);
		send(ssl->s, (char*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
		g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
	}
}

static int NetlibSslReadSetResult(SslHandle *ssl, char *buf, int num, int peek)
{
	if (ssl->cbRecDataBuf == 0)
		return (ssl->state == sockClosed ? 0 : SOCKET_ERROR);

	int bytes = min(num, ssl->cbRecDataBuf);
	int rbytes = ssl->cbRecDataBuf - bytes;

	memcpy(buf, ssl->pbRecDataBuf, bytes);
	if (!peek) {
		memmove(ssl->pbRecDataBuf, ssl->pbRecDataBuf + bytes, rbytes);
		ssl->cbRecDataBuf = rbytes;
	}

	return bytes;
}

int NetlibSslRead(SslHandle *ssl, char *buf, int num, int peek)
{
	if (ssl == NULL) return SOCKET_ERROR;

	if (num <= 0) return 0;

	if (ssl->state != sockOpen || (ssl->cbRecDataBuf != 0 && (!peek || ssl->cbRecDataBuf >= num)))
		return NetlibSslReadSetResult(ssl, buf, num, peek);

	SECURITY_STATUS scRet = SEC_E_OK;

	while (true) {
		if (0 == ssl->cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
			if (ssl->sbIoBuffer <= ssl->cbIoBuffer) {
				ssl->sbIoBuffer += 2048;
				ssl->pbIoBuffer = (PUCHAR)mir_realloc(ssl->pbIoBuffer, ssl->sbIoBuffer);
			}

			if (peek) {
				static const TIMEVAL tv = { 0 };
				fd_set fd;
				FD_ZERO(&fd);
				FD_SET(ssl->s, &fd);

				DWORD cbData = select(1, &fd, NULL, NULL, &tv);
				if (cbData == SOCKET_ERROR) {
					ssl->state = sockError;
					return NetlibSslReadSetResult(ssl, buf, num, peek);
				}

				if (cbData == 0 && ssl->cbRecDataBuf)
					return NetlibSslReadSetResult(ssl, buf, num, peek);
			}

			DWORD cbData = recv(ssl->s, (char*)ssl->pbIoBuffer + ssl->cbIoBuffer, ssl->sbIoBuffer - ssl->cbIoBuffer, 0);
			if (cbData == SOCKET_ERROR) {
				NetlibLogf(NULL, "SSL failure recieving data (%d)", WSAGetLastError());
				ssl->state = sockError;
				return NetlibSslReadSetResult(ssl, buf, num, peek);
			}

			if (cbData == 0) {
				NetlibLogf(NULL, "SSL connection gracefully closed");
				if (peek && ssl->cbRecDataBuf) {
					ssl->state = sockClosed;
					return NetlibSslReadSetResult(ssl, buf, num, peek);
				}

				// Server disconnected.
				if (ssl->cbIoBuffer) {
					ssl->state = sockError;
					return NetlibSslReadSetResult(ssl, buf, num, peek);
				}

				return 0;
			}
			else {
				NetlibDumpData(NULL, ssl->pbIoBuffer + ssl->cbIoBuffer, cbData, 0, MSG_DUMPSSL);
				ssl->cbIoBuffer += cbData;
			}
		}

		// Attempt to decrypt the received data.
		SecBuffer Buffers[4];
		Buffers[0].pvBuffer = ssl->pbIoBuffer;
		Buffers[0].cbBuffer = ssl->cbIoBuffer;
		Buffers[0].BufferType = SECBUFFER_DATA;

		Buffers[1].BufferType = SECBUFFER_EMPTY;
		Buffers[2].BufferType = SECBUFFER_EMPTY;
		Buffers[3].BufferType = SECBUFFER_EMPTY;

		SecBufferDesc Message;
		Message.ulVersion = SECBUFFER_VERSION;
		Message.cBuffers = SIZEOF(Buffers);
		Message.pBuffers = Buffers;

		if (g_pSSPI->DecryptMessage != NULL && g_pSSPI->DecryptMessage != PVOID(0x80000000))
			scRet = g_pSSPI->DecryptMessage(&ssl->hContext, &Message, 0, NULL);
		else
			scRet = ((DECRYPT_MESSAGE_FN)g_pSSPI->Reserved4)(&ssl->hContext, &Message, 0, NULL);

		// The input buffer contains only a fragment of an
		// encrypted record. Loop around and read some more
		// data.
		if (scRet == SEC_E_INCOMPLETE_MESSAGE)
			continue;

		if (scRet != SEC_E_OK && scRet != SEC_I_RENEGOTIATE && scRet != SEC_I_CONTEXT_EXPIRED) {
			ReportSslError(scRet, __LINE__);
			ssl->state = sockError;
			return NetlibSslReadSetResult(ssl, buf, num, peek);
		}

		// Locate data and (optional) extra buffers.
		SecBuffer *pDataBuffer = NULL;
		SecBuffer *pExtraBuffer = NULL;
		for (int i = 1; i < SIZEOF(Buffers); i++) {
			if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA)
				pDataBuffer = &Buffers[i];

			if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA)
				pExtraBuffer = &Buffers[i];
		}

		// Return decrypted data.
		DWORD resNum = 0;
		if (pDataBuffer) {
			DWORD bytes = peek ? 0 : min((DWORD)num, pDataBuffer->cbBuffer);
			DWORD rbytes = pDataBuffer->cbBuffer - bytes;

			NetlibDumpData(NULL, (PBYTE)pDataBuffer->pvBuffer, pDataBuffer->cbBuffer, 0, MSG_DUMPSSL);

			if (rbytes > 0) {
				int nbytes = ssl->cbRecDataBuf + rbytes;
				if (ssl->sbRecDataBuf < nbytes) {
					ssl->sbRecDataBuf = nbytes;
					ssl->pbRecDataBuf = (PUCHAR)mir_realloc(ssl->pbRecDataBuf, nbytes);
				}
				memcpy(ssl->pbRecDataBuf + ssl->cbRecDataBuf, (char*)pDataBuffer->pvBuffer + bytes, rbytes);
				ssl->cbRecDataBuf = nbytes;
			}

			if (peek) {
				resNum = bytes = min(num, ssl->cbRecDataBuf);
				memcpy(buf, ssl->pbRecDataBuf, bytes);
			}
			else {
				resNum = bytes;
				memcpy(buf, pDataBuffer->pvBuffer, bytes);
			}
		}

		// Move any "extra" data to the input buffer.
		if (pExtraBuffer) {
			memmove(ssl->pbIoBuffer, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
			ssl->cbIoBuffer = pExtraBuffer->cbBuffer;
		}
		else ssl->cbIoBuffer = 0;

		if (pDataBuffer && resNum)
			return resNum;

		// Server signaled end of session
		if (scRet == SEC_I_CONTEXT_EXPIRED) {
			NetlibLogf(NULL, "SSL Server signaled SSL Shutdown");
			ssl->state = sockClosed;
			return NetlibSslReadSetResult(ssl, buf, num, peek);
		}

		if (scRet == SEC_I_RENEGOTIATE) {
			// The server wants to perform another handshake
			// sequence.

			scRet = ClientHandshakeLoop(ssl, FALSE);
			if (scRet != SEC_E_OK) {
				ssl->state = sockError;
				return NetlibSslReadSetResult(ssl, buf, num, peek);
			}
		}
	}
}

int NetlibSslWrite(SslHandle *ssl, const char *buf, int num)
{
	if (ssl == NULL) return SOCKET_ERROR;

	SecPkgContext_StreamSizes Sizes;
	SECURITY_STATUS scRet = g_pSSPI->QueryContextAttributes(&ssl->hContext, SECPKG_ATTR_STREAM_SIZES, &Sizes);
	if (scRet != SEC_E_OK)
		return scRet;

	PUCHAR pbDataBuffer = (PUCHAR)mir_calloc(Sizes.cbMaximumMessage + Sizes.cbHeader + Sizes.cbTrailer);

	PUCHAR pbMessage = pbDataBuffer + Sizes.cbHeader;

	DWORD sendOff = 0;
	while (sendOff < (DWORD)num) {
		DWORD cbMessage = min(Sizes.cbMaximumMessage, (DWORD)num - sendOff);
		memcpy(pbMessage, buf + sendOff, cbMessage);

		SecBuffer Buffers[4] = { 0 };
		Buffers[0].pvBuffer = pbDataBuffer;
		Buffers[0].cbBuffer = Sizes.cbHeader;
		Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

		Buffers[1].pvBuffer = pbMessage;
		Buffers[1].cbBuffer = cbMessage;
		Buffers[1].BufferType = SECBUFFER_DATA;

		Buffers[2].pvBuffer = pbMessage + cbMessage;
		Buffers[2].cbBuffer = Sizes.cbTrailer;
		Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

		Buffers[3].BufferType = SECBUFFER_EMPTY;

		SecBufferDesc Message;
		Message.ulVersion = SECBUFFER_VERSION;
		Message.cBuffers = SIZEOF(Buffers);
		Message.pBuffers = Buffers;

		if (g_pSSPI->EncryptMessage != NULL)
			scRet = g_pSSPI->EncryptMessage(&ssl->hContext, 0, &Message, 0);
		else
			scRet = ((ENCRYPT_MESSAGE_FN)g_pSSPI->Reserved3)(&ssl->hContext, 0, &Message, 0);

		if (FAILED(scRet)) break;

		// Calculate encrypted packet size
		DWORD cbData = Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer;

		// Send the encrypted data to the server.
		NetlibDumpData(NULL, pbDataBuffer, cbData, 1, MSG_DUMPSSL);
		cbData = send(ssl->s, (char*)pbDataBuffer, cbData, 0);
		if (cbData == SOCKET_ERROR || cbData == 0) {
			NetlibLogf(NULL, "SSL failure sending data (%d)", WSAGetLastError());
			scRet = SEC_E_INTERNAL_ERROR;
			break;
		}

		sendOff += cbMessage;
	}

	mir_free(pbDataBuffer);
	return scRet == SEC_E_OK ? num : SOCKET_ERROR;
}

static INT_PTR GetSslApi(WPARAM, LPARAM lParam)
{
	SSL_API *si = (SSL_API*)lParam;
	if (si == NULL)
		return FALSE;

	if (si->cbSize != sizeof(SSL_API))
		return FALSE;

	si->connect = (HSSL(__cdecl *)(SOCKET, const char *, int))NetlibSslConnect;
	si->pending = (BOOL(__cdecl *)(HSSL))NetlibSslPending;
	si->read = (int(__cdecl *)(HSSL, char *, int, int))NetlibSslRead;
	si->write = (int(__cdecl *)(HSSL, const char *, int))NetlibSslWrite;
	si->shutdown = (void(__cdecl *)(HSSL))NetlibSslShutdown;
	si->sfree = (void(__cdecl *)(HSSL))NetlibSslFree;
	return TRUE;
}

int LoadSslModule(void)
{
	CreateServiceFunction(MS_SYSTEM_GET_SI, GetSslApi);
	g_hSslMutex = CreateMutex(NULL, FALSE, NULL);
	SecInvalidateHandle(&hCreds);
	return 0;
}

void UnloadSslModule(void)
{
	if (g_pSSPI && SecIsValidHandle(&hCreds))
		g_pSSPI->FreeCredentialsHandle(&hCreds);
	CloseHandle(g_hSslMutex);
	if (g_hSchannel)
		FreeLibrary(g_hSchannel);
}
