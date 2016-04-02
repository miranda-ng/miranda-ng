/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-16 Miranda NG project (http://miranda-ng.org),
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
#include <m_popup.h>

static HANDLE g_hSslMutex;
static bool bSslInitDone;

enum SocketState
{
	sockOpen,
	sockClosed,
	sockError
};

struct SslHandle
{
	SOCKET s;
	SSL_CTX *ctx;
	SSL *session;
	SocketState state;
};

static void SSL_library_unload(void)
{
	/* Load Library Pointers */
	if (!bSslInitDone) return;

	WaitForSingleObject(g_hSslMutex, INFINITE);

	FreeLibrary(g_hOpenSSL);
	g_hOpenSSL = NULL;

	FreeLibrary(g_hOpenSSLCrypto);
	g_hOpenSSLCrypto = NULL;
	FreeLibrary(g_hWinCrypt);
	g_hWinCrypt = NULL;

	bSslInitDone = false;

	ReleaseMutex(g_hSslMutex);
}

static bool SSL_library_load(void)
{
	/* Load Library Pointers */
	if (bSslInitDone) return true;

	WaitForSingleObject(g_hSslMutex, INFINITE);

	if (!bSslInitDone) {
		g_hOpenSSLCrypto = LoadLibraryA("libeay32.dll");
		g_hOpenSSL = LoadLibraryA("ssleay32.dll");
		g_hWinCrypt = LoadLibraryA("crypt32.dll");
		if (g_hOpenSSL && g_hOpenSSLCrypto && g_hWinCrypt) 
		{
			// init OpenSSL
			SSL_library_init();
			SSL_load_error_strings();
			CRYPTO_set_mem_functions(mir_calloc, mir_realloc, mir_free);
			// FIXME check errors

			bSslInitDone = true;
		}
		else {
			SSL_library_unload();
		}
	}

	return bSslInitDone;
}

const char* SSL_GetCipherName(SslHandle *ssl)
{
	if (!ssl || !ssl->session)
		return NULL;

	return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl->session));
}

static void ReportSslError(SECURITY_STATUS scRet, int line, bool = false)
{
	CMStringW tszMsg(FORMAT, L"SSL connection failure(%x %u) :", scRet, line);

	switch (scRet) {
	case 0:
	case ERROR_NOT_READY:
		return;

	case SEC_E_INVALID_TOKEN:
		tszMsg += TranslateT("Client cannot decode host message. Possible causes: host does not support SSL or requires not existing security package");
		break;

	case CERT_E_CN_NO_MATCH:
	case SEC_E_WRONG_PRINCIPAL:
		tszMsg += TranslateT("Host we are connecting to is not the one certificate was issued for");
		break;

	default:
		TCHAR szMsgBuf[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, scRet, LANG_USER_DEFAULT, szMsgBuf, _countof(szMsgBuf), NULL);
		tszMsg += szMsgBuf;
	}

	Netlib_LogfW(0, tszMsg);

	SetLastError(scRet);
	PUShowMessageT(tszMsg.GetBuffer(), SM_WARNING);
}

void NetlibSslFree(SslHandle *ssl)
{
	if (ssl == NULL) return;

	/* Delete Context */
	if (ssl->session) 
		SSL_free(ssl->session);
	if (ssl->ctx) 
		SSL_CTX_free(ssl->ctx);

	mir_free(ssl);
}

BOOL NetlibSslPending(HSSL ssl)
{
	/* return true if there is either unsend or buffered received data (ie. after peek) */
	return ssl && ssl->session && (SSL_pending(ssl->session) > 0);
}

static bool ClientConnect(SslHandle *ssl, const char*)
{
	SSL_METHOD *meth = (SSL_METHOD*)SSLv23_client_method();

	// contrary to what it's named, SSLv23 announces all supported ciphers/versions,
	// generally TLS1.2 in a TLS1.0 Client Hello
	if (!meth) {
		Netlib_Logf(0, "SSL setup failure: client method");
		return false;
	}
	ssl->ctx = SSL_CTX_new(meth);
	if (!ssl->ctx) {
		Netlib_Logf(0, "SSL setup failure: context");
		return false;
	}
	// disable dangerous cipher suites
	SSL_CTX_ctrl(ssl->ctx, SSL_CTRL_OPTIONS, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3, NULL);
	// SSL_read/write should transparently handle renegotiations
	SSL_CTX_ctrl(ssl->ctx, SSL_CTRL_MODE, SSL_MODE_AUTO_RETRY, NULL);

	RAND_screen();
	ssl->session = SSL_new(ssl->ctx);
	if (!ssl->session) {
		Netlib_Logf(0, "SSL setup failure: session");
		return false;
	}
	SSL_set_fd(ssl->session, ssl->s);

	int err = SSL_connect(ssl->session);

	if (err != 1) {
		err = SSL_get_error(ssl->session, err);
		Netlib_Logf(0, "SSL negotiation failure (%d)", err);
		return false;
	}

	const char* suite = SSL_GetCipherName(ssl);
	if (suite != NULL)
		Netlib_Logf(0, "SSL established with %s", suite);
	return true;
}

static PCCERT_CONTEXT SSL_X509ToCryptCert(X509 * x509)
{
	int len;
	unsigned char * buf = NULL;
	PCCERT_CONTEXT pCertContext = NULL;

	len = i2d_X509(x509, &buf);
	if ((len >= 0) && buf) {
		pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, buf, len);

		CRYPTO_free(buf);
	}
	return pCertContext;
}

static PCCERT_CONTEXT SSL_CertChainToCryptAnchor(SSL* session)
{
	/* convert the active certificate chain provided in the handshake of 'session' into
		the format used by CryptAPI.
	*/
	PCCERT_CONTEXT anchor = NULL;
	// create cert store
	HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG, NULL);

	if (store) {
		X509 *server_cert = SSL_get_peer_certificate(session);
		if (server_cert) {
			// add the server's cert first, to make sure CryptAPI builds the correct chain
			PCCERT_CONTEXT primary_cert;
			BOOL ok = CertAddCertificateContextToStore(store, SSL_X509ToCryptCert(server_cert), CERT_STORE_ADD_ALWAYS, &primary_cert);
			if (ok && primary_cert) {
				// add all remaining certs to store (note: stack needs not be freed, it is not a copy)
				STACK_OF(X509) *server_chain = SSL_get_peer_cert_chain(session);
				if (server_chain) {
					X509 *next_cert;
					int i;
					for (i = 0; i < server_chain->stack.num; i++) {
						next_cert = (X509 *)server_chain->stack.data[i];
						CertAddCertificateContextToStore(store, SSL_X509ToCryptCert(next_cert), CERT_STORE_ADD_USE_EXISTING, NULL);
					}
				}

				// return primary cert; MUST be freed by caller which will free the associated store
				anchor = primary_cert;
			}
			else {
				if (primary_cert) CertFreeCertificateContext(primary_cert);
			}

			X509_free(server_cert);
		}

		CertCloseStore(store, 0);
	}

	return anchor;
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

	pServerCert = SSL_CertChainToCryptAnchor(ssl->session);

	if (pServerCert == NULL) {
		scRet = SEC_E_WRONG_PRINCIPAL;
		goto cleanup;
	}

	ChainPara.cbSize = sizeof(ChainPara);
	ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
	ChainPara.RequestedUsage.Usage.cUsageIdentifier = _countof(rgszUsages);
	ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;
	if (!CertGetCertificateChain(NULL, pServerCert, NULL, pServerCert->hCertStore,
		&ChainPara, 0, NULL, &pChainContext)) {
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

	if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChainContext,
		&PolicyPara, &PolicyStatus)) {
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

SslHandle* NetlibSslConnect(SOCKET s, const char* host, int verify)
{
	/* negotiate SSL session, verify cert, return NULL if failed */
	
	SslHandle *ssl = (SslHandle*)mir_calloc(sizeof(SslHandle));
	ssl->s = s;
	bool res = ClientConnect(ssl, host);

	if (res && verify) {
		DWORD dwFlags = 0;
		if (!host || inet_addr(host) != INADDR_NONE)
			dwFlags |= 0x00001000;
		res = VerifyCertificate(ssl, host, dwFlags);
	}

	if (res)
		return ssl;

	NetlibSslFree(ssl);
	return NULL;
}

void NetlibSslShutdown(SslHandle *ssl)
{
	/* Close SSL session, but keep socket open */
	if (ssl == NULL || ssl->session == NULL)
		return;

	SSL_shutdown(ssl->session);
}

int NetlibSslRead(SslHandle *ssl, char *buf, int num, int peek)
{
	/* read number of bytes, keep in buffer if peek!=0 */
	if (!ssl || !ssl->session) return SOCKET_ERROR;
	if (num <= 0) return 0;

	int err = 0;
	if (peek)
		err = SSL_peek(ssl->session, buf, num);
	else
		err = SSL_read(ssl->session, buf, num);

	if (err <= 0) {
		int err2 = SSL_get_error(ssl->session, err);
		switch (err2) {
		case SSL_ERROR_ZERO_RETURN:
			Netlib_Logf(0, "SSL connection gracefully closed");
			ssl->state = sockClosed;
			break;
		default:
			Netlib_Logf(0, "SSL failure recieving data (%d, %d, %d)", err, err2, WSAGetLastError());
			ssl->state = sockError;
			return SOCKET_ERROR;
		}
		return 0;
	}

	return err;
}

int NetlibSslWrite(SslHandle *ssl, const char *buf, int num)
{
	/* write number of bytes */
	if (!ssl || !ssl->session) return SOCKET_ERROR;
	if (num <= 0) return 0;

	int err = SSL_write(ssl->session, buf, num);
	if (err > 0)
		return err;
	
	int err2 = SSL_get_error(ssl->session, err);
	switch (err2) {
	case SSL_ERROR_ZERO_RETURN:
		Netlib_Logf(0, "SSL connection gracefully closed");
		ssl->state = sockClosed;
		break;
	default:
		Netlib_Logf(0, "SSL failure sending data (%d, %d, %d)", err, err2, WSAGetLastError());
		ssl->state = sockError;
		return SOCKET_ERROR;
	}
	return 0;
}

static INT_PTR GetSslApi(WPARAM, LPARAM lParam)
{
	SSL_API* pSsl = (SSL_API*)lParam;
	if (pSsl == NULL)
		return FALSE;

	if (pSsl->cbSize != sizeof(SSL_API))
		return FALSE;

	pSsl->connect = NetlibSslConnect;
	pSsl->pending = NetlibSslPending;
	pSsl->read = NetlibSslRead;
	pSsl->write = NetlibSslWrite;
	pSsl->shutdown = NetlibSslShutdown;
	pSsl->sfree = NetlibSslFree;
	return TRUE;
}

int LoadSslModule(void)
{
	if (!SSL_library_load())
	{
		MessageBoxW(NULL, TranslateT("OpenSSL library loading failed"), TranslateT("OpenSSL error"), MB_ICONERROR | MB_OK);
		return 1;
	}
	CreateServiceFunction(MS_SYSTEM_GET_SI, GetSslApi);
	g_hSslMutex = CreateMutex(NULL, FALSE, NULL);
	return 0;
}

void UnloadSslModule(void)
{
	SSL_library_unload();
	CloseHandle(g_hSslMutex);
}
