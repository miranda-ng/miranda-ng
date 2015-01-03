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

#include "commonheaders.h"
#include <m_popup.h>

#define SECURITY_WIN32
#include <security.h>

#include <openssl\ssl.h>

static HMODULE g_hOpenSSL;
static HMODULE g_hOpenSSLCrypto;
static HANDLE g_hSslMutex;
static bool bSslInitDone;

/* OpenSSL dynamic imports */

typedef void ( *pfnRAND_screen)(void);
typedef int ( *pfnSSL_library_init ) (void );
typedef int ( *pfnCRYPTO_set_mem_functions )(void *(*m)(size_t),void *(*r)(void *,size_t), void (*f)(void *));
typedef void ( *pfnSSL_load_error_strings )(void );
typedef void ( *pfnSSL_free )(SSL *ssl);
typedef int ( *pfnSSL_connect )(SSL *ssl);
typedef int ( *pfnSSL_read )(SSL *ssl,void *buf,int num);
typedef int ( *pfnSSL_set_fd )(SSL *s, int fd);
typedef int ( *pfnSSL_peek )(SSL *ssl,void *buf,int num);
typedef int ( *pfnSSL_pending )(const SSL *s);
typedef int ( *pfnSSL_write )(SSL *ssl,const void *buf,int num);
typedef SSL * ( *pfnSSL_new )(SSL_CTX *ctx);
typedef int ( *pfnSSL_shutdown )(SSL *s);
typedef void ( *pfnSSL_CTX_free )(SSL_CTX *);
typedef SSL_CTX * ( *pfnSSL_CTX_new )(const SSL_METHOD *meth);
typedef long ( *pfnSSL_CTX_ctrl )(SSL_CTX *ctx,int cmd, long larg, void *parg);
typedef int ( *pfnSSL_get_error )(const SSL *s,int ret_code);

typedef const SSL_CIPHER * ( *pfnSSL_get_current_cipher )(const SSL *s);
typedef const char * ( *pfnSSL_CIPHER_get_name )(const SSL_CIPHER *c);

typedef STACK_OF(X509) * ( *pfnSSL_get_peer_cert_chain )(const SSL *s);
typedef X509 * ( *pfnSSL_get_peer_certificate )(const SSL *s);
typedef void ( *pfnCRYPTO_free )(void *ptr);
typedef void ( *pfnX509_free )(X509 *x509);
typedef int ( *pfni2d_X509 )(X509 *x, unsigned char **out);

typedef const SSL_METHOD * ( *pfnSSLv23_client_method )(void);

static struct LIBOPENSSL {
	pfnRAND_screen				RAND_screen;
	pfnSSL_connect				SSL_connect;
	pfnSSL_free					SSL_free;
	pfnSSL_get_error			SSL_get_error;
	pfnSSL_library_init			SSL_library_init;
	pfnSSL_load_error_strings	SSL_load_error_strings;
	pfnSSL_new					SSL_new;
	pfnSSL_peek					SSL_peek;
	pfnSSL_pending				SSL_pending;
	pfnSSL_read					SSL_read;
	pfnSSL_write				SSL_write;
	pfnSSL_set_fd				SSL_set_fd;
	pfnSSL_shutdown				SSL_shutdown;
	pfnSSL_CTX_free				SSL_CTX_free;
	pfnSSL_CTX_new				SSL_CTX_new;
	pfnSSL_CTX_ctrl				SSL_CTX_ctrl;
	pfnCRYPTO_set_mem_functions	CRYPTO_set_mem_functions;

	pfnSSLv23_client_method		SSLv23_client_method;

	pfnSSL_get_current_cipher	SSL_get_current_cipher;
	pfnSSL_CIPHER_get_name		SSL_CIPHER_get_name;

	pfnSSL_get_peer_certificate	SSL_get_peer_certificate;
	pfnSSL_get_peer_cert_chain	SSL_get_peer_cert_chain;
	pfnCRYPTO_free				CRYPTO_free;
	pfnX509_free				X509_free;
	pfni2d_X509					i2d_X509;
} g_OpenSSL;


/* Crypt32 dynamic imports */

typedef BOOL (WINAPI *pfnCertGetCertificateChain)(HCERTCHAINENGINE, PCCERT_CONTEXT, LPFILETIME, HCERTSTORE, PCERT_CHAIN_PARA, DWORD, LPVOID, PCCERT_CHAIN_CONTEXT*);
typedef VOID (WINAPI *pfnCertFreeCertificateChain)(PCCERT_CHAIN_CONTEXT);
typedef BOOL (WINAPI *pfnCertFreeCertificateContext)(PCCERT_CONTEXT);
typedef BOOL (WINAPI *pfnCertVerifyCertificateChainPolicy)(LPCSTR, PCCERT_CHAIN_CONTEXT, PCERT_CHAIN_POLICY_PARA, PCERT_CHAIN_POLICY_STATUS);
typedef HCERTSTORE (WINAPI *pfnCertOpenStore)(LPCSTR, DWORD, HCRYPTPROV_LEGACY, DWORD, const void *);

typedef BOOL (WINAPI *pfnCertCloseStore)(HCERTSTORE, DWORD);
typedef BOOL (WINAPI *pfnCertAddCertificateContextToStore)(HCERTSTORE, PCCERT_CONTEXT, DWORD, PCCERT_CONTEXT*);
typedef PCCERT_CONTEXT (WINAPI *pfnCertCreateCertificateContext)(DWORD, const BYTE *,DWORD);

static struct LIBCRYPT {
	pfnCertGetCertificateChain		CertGetCertificateChain;
	pfnCertFreeCertificateChain		CertFreeCertificateChain;
	pfnCertFreeCertificateContext	CertFreeCertificateContext;
	pfnCertVerifyCertificateChainPolicy	CertVerifyCertificateChainPolicy;
	pfnCertOpenStore				CertOpenStore;
	pfnCertCloseStore				CertCloseStore;
	pfnCertAddCertificateContextToStore	CertAddCertificateContextToStore;
	pfnCertCreateCertificateContext	CertCreateCertificateContext;
} g_Crypt;


typedef enum
{
	sockOpen,
	sockClosed,
	sockError
} SocketState;

struct SslHandle
{
	SOCKET s;

	SSL_CTX *ctx;
	SSL	*session;

	SocketState state;
};


void SslLog(const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	CallServiceSync(MS_NETLIB_LOG, (WPARAM)NULL, (LPARAM)szText);
}

static void SSL_library_unload(void) {
	/* Load Library Pointers */
	if (!bSslInitDone) return;

	WaitForSingleObject(g_hSslMutex, INFINITE);

	FreeLibrary(g_hOpenSSL);
	g_hOpenSSL = NULL;
	FreeLibrary(g_hOpenSSLCrypto);
	g_hOpenSSLCrypto = NULL;
	bSslInitDone = false;

	ReleaseMutex(g_hSslMutex);
}

static bool SSL_library_load(void)
{
	/* Load Library Pointers */
	if (bSslInitDone) return true;

	WaitForSingleObject(g_hSslMutex, INFINITE);

	if (!bSslInitDone)
	{
		g_hOpenSSLCrypto = LoadLibraryA("libeay32.dll");
		g_hOpenSSL = LoadLibraryA("ssleay32.dll");
		if (g_hOpenSSL && g_hOpenSSLCrypto)
		{
			// load function pointers
			#define LOAD_FN(struc,lib, name) struc.##name = (pfn##name)GetProcAddress(lib, #name);
			LOAD_FN(g_OpenSSL, g_hOpenSSLCrypto, RAND_screen);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_connect);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_free);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_get_error);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_library_init);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_load_error_strings);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_new);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_peek);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_pending);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_read);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_write);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_set_fd);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_shutdown);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_CTX_free);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_CTX_new);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_CTX_ctrl);
			LOAD_FN(g_OpenSSL, g_hOpenSSLCrypto, CRYPTO_set_mem_functions);

			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSLv23_client_method);

			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_get_current_cipher);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_CIPHER_get_name);

			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_get_peer_certificate);
			LOAD_FN(g_OpenSSL, g_hOpenSSL, SSL_get_peer_cert_chain);
			LOAD_FN(g_OpenSSL, g_hOpenSSLCrypto, CRYPTO_free);
			LOAD_FN(g_OpenSSL, g_hOpenSSLCrypto, X509_free);
			LOAD_FN(g_OpenSSL, g_hOpenSSLCrypto, i2d_X509);

			HINSTANCE hCrypt = LoadLibraryA("crypt32.dll");
			if (hCrypt)
			{
				LOAD_FN(g_Crypt, hCrypt, CertGetCertificateChain);
				LOAD_FN(g_Crypt, hCrypt, CertFreeCertificateChain);
				LOAD_FN(g_Crypt, hCrypt, CertFreeCertificateContext);
				LOAD_FN(g_Crypt, hCrypt, CertVerifyCertificateChainPolicy);
				LOAD_FN(g_Crypt, hCrypt, CertOpenStore);
				LOAD_FN(g_Crypt, hCrypt, CertCloseStore);
				LOAD_FN(g_Crypt, hCrypt, CertAddCertificateContextToStore);
				LOAD_FN(g_Crypt, hCrypt, CertCreateCertificateContext);
			}

			// init OpenSSL
			g_OpenSSL.SSL_library_init();
			g_OpenSSL.SSL_load_error_strings();
			g_OpenSSL.CRYPTO_set_mem_functions(mir_calloc, mir_realloc, mir_free);
			// FIXME check errors

			bSslInitDone = true;
		} else {
			SSL_library_unload();
		}
	}

	ReleaseMutex(g_hSslMutex);
	return bSslInitDone;
}

const char* SSL_GetCipherName(SslHandle *ssl)
{
	if (!ssl || !ssl->session)
		return NULL;

	return g_OpenSSL.SSL_CIPHER_get_name(g_OpenSSL.SSL_get_current_cipher(ssl->session));
}

static void ReportSslError(SECURITY_STATUS scRet, int line, bool showPopup = false)
{
	TCHAR szMsgBuf[256];
	switch (scRet)
	{
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
	SslLog(szMsg);
	mir_free(szMsg);

	SetLastError(scRet);
	PUShowMessageT(szMsgBuf2, SM_WARNING);
}

void NetlibSslFree(SslHandle *ssl)
{
	if (ssl == NULL) return;

	/* Delete Context */
	if (ssl->session) g_OpenSSL.SSL_free (ssl->session);
	if (ssl->ctx) g_OpenSSL.SSL_CTX_free (ssl->ctx);
	ssl->session = NULL;
	ssl->ctx = NULL;

	memset(ssl, 0, sizeof(SslHandle));
	mir_free(ssl);
}

BOOL NetlibSslPending(SslHandle *ssl)
{
	/* return true if there is either unsend or buffered received data (ie. after peek) */
	return ssl && ssl->session && (g_OpenSSL.SSL_pending(ssl->session) > 0);
}

static bool ClientConnect(SslHandle *ssl, const char *host)
{
	SSL_METHOD * meth;

	// contrary to what it's named, SSLv23 announces all supported ciphers/versions,
	// generally TLS1.2 in a TLS1.0 Client Hello
	meth = (SSL_METHOD*)g_OpenSSL.SSLv23_client_method();
	if (!meth) {
		SslLog("SSL setup failure: client method");
		return false;
	}
	ssl->ctx = g_OpenSSL.SSL_CTX_new(meth);
	if (!ssl->ctx) {
		SslLog("SSL setup failure: context");
		return false;
	}
	// disable dangerous cipher suites
	g_OpenSSL.SSL_CTX_ctrl(ssl->ctx, SSL_CTRL_OPTIONS, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3, NULL);
	// SSL_read/write should transparently handle renegotiations
	g_OpenSSL.SSL_CTX_ctrl(ssl->ctx, SSL_CTRL_MODE, SSL_MODE_AUTO_RETRY, NULL);

	g_OpenSSL.RAND_screen();
	ssl->session = g_OpenSSL.SSL_new(ssl->ctx);
	if (!ssl->session) {
		SslLog("SSL setup failure: session");
		return false;
	}
	g_OpenSSL.SSL_set_fd(ssl->session, ssl->s);

	int err = g_OpenSSL.SSL_connect(ssl->session);

	if (err != 1) {
		err = g_OpenSSL.SSL_get_error(ssl->session, err);
		SslLog("SSL negotiation failure (%d)", err);
		return false;
	}

	const char* suite = SSL_GetCipherName(ssl);
	if (suite != NULL)
		SslLog("SSL established with %s", suite);
	return true;
}

static PCCERT_CONTEXT SSL_X509ToCryptCert(X509 * x509) {
	int len;
	unsigned char * buf = NULL;
	PCCERT_CONTEXT pCertContext = NULL;

	len = g_OpenSSL.i2d_X509(x509, &buf);
	if ((len >= 0) && buf) {
		pCertContext = g_Crypt.CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, buf, len);

		g_OpenSSL.CRYPTO_free(buf);
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
	HCERTSTORE store = g_Crypt.CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG, NULL);

	if (store) {
		X509 *server_cert = g_OpenSSL.SSL_get_peer_certificate(session);
		if (server_cert) {
			// add the server's cert first, to make sure CryptAPI builds the correct chain
			PCCERT_CONTEXT primary_cert;
			BOOL ok = g_Crypt.CertAddCertificateContextToStore(store, SSL_X509ToCryptCert(server_cert), CERT_STORE_ADD_ALWAYS, &primary_cert);
			if (ok && primary_cert) {
				// add all remaining certs to store (note: stack needs not be freed, it is not a copy)
				STACK_OF(X509) *server_chain = g_OpenSSL.SSL_get_peer_cert_chain(session);
				if (server_chain) {
					X509 *next_cert;
					int i;
					for (i = 0; i < server_chain->stack.num; i++) {
						next_cert = (X509 *)server_chain->stack.data[i];
						g_Crypt.CertAddCertificateContextToStore(store, SSL_X509ToCryptCert(next_cert), CERT_STORE_ADD_USE_EXISTING, NULL);
					}
				}

				// return primary cert; MUST be freed by caller which will free the associated store
				anchor = primary_cert;
			} else {
				if (primary_cert) g_Crypt.CertFreeCertificateContext(primary_cert);
			}

			g_OpenSSL.X509_free(server_cert);
		}

		g_Crypt.CertCloseStore(store, 0);
	}

	return anchor;
}

static bool VerifyCertificate(SslHandle *ssl, PCSTR pszServerName, DWORD dwCertFlags)
{
	if (!g_Crypt.CertGetCertificateChain)
		return true;

	static LPSTR rgszUsages[] =
	{
		szOID_PKIX_KP_SERVER_AUTH,
		szOID_SERVER_GATED_CRYPTO,
		szOID_SGC_NETSCAPE
	};

	CERT_CHAIN_PARA          ChainPara = {0};
	HTTPSPolicyCallbackData  polHttps = {0};
	CERT_CHAIN_POLICY_PARA   PolicyPara = {0};
	CERT_CHAIN_POLICY_STATUS PolicyStatus = {0};
	PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
	PCCERT_CONTEXT           pServerCert = NULL;
	DWORD scRet;

	PWSTR pwszServerName = mir_a2u(pszServerName);

	pServerCert = SSL_CertChainToCryptAnchor(ssl->session);

	if (pServerCert == NULL)
	{
		scRet = SEC_E_WRONG_PRINCIPAL;
		goto cleanup;
	}

	ChainPara.cbSize = sizeof(ChainPara);
	ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
	ChainPara.RequestedUsage.Usage.cUsageIdentifier = SIZEOF(rgszUsages);
	ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;
	if (!g_Crypt.CertGetCertificateChain(NULL, pServerCert, NULL, pServerCert->hCertStore,
	     &ChainPara, 0, NULL, &pChainContext))
	{
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

	if (!g_Crypt.CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, pChainContext,
		&PolicyPara, &PolicyStatus))
	{
		scRet = GetLastError();
		goto cleanup;
	}

	if (PolicyStatus.dwError)
	{
		scRet = PolicyStatus.dwError;
		goto cleanup;
	}

	scRet = SEC_E_OK;

cleanup:
	if (pChainContext)
		g_Crypt.CertFreeCertificateChain(pChainContext);
	if (pServerCert)
		g_Crypt.CertFreeCertificateContext(pServerCert);
	mir_free(pwszServerName);

	ReportSslError(scRet, __LINE__, true);
	return scRet == SEC_E_OK;
}

SslHandle *NetlibSslConnect(SOCKET s, const char* host, int verify)
{
	SslHandle *ssl = (SslHandle*)mir_calloc(sizeof(SslHandle));
	ssl->s = s;
	/* negotiate SSL session, verify cert, return NULL if failed */

	DWORD dwFlags = 0;
	if (!host || inet_addr(host) != INADDR_NONE)
		dwFlags |= 0x00001000;

	bool res = SSL_library_load();
	if (!res) {
		return NULL;
	}

	if (res) res = ClientConnect(ssl, host);
	if (res && verify) res = VerifyCertificate(ssl, host, dwFlags);

	if (!res)
	{
		NetlibSslFree(ssl);
		ssl = NULL;
	}
	return ssl;
}

void NetlibSslShutdown(SslHandle *ssl)
{
	/* Close SSL session, but keep socket open */
	if (ssl==NULL || ssl->session == NULL)
		return;

	g_OpenSSL.SSL_shutdown(ssl->session);
}

int NetlibSslRead(SslHandle *ssl, char *buf, int num, int peek)
{
	/* read number of bytes, keep in buffer if peek!=0 */
	if (!ssl || !ssl->session) return SOCKET_ERROR;
	if (num <= 0) return 0;

	int err = 0;
	if (peek)
		err = g_OpenSSL.SSL_peek(ssl->session, buf, num);
	else
		err = g_OpenSSL.SSL_read(ssl->session, buf, num);

	if (err <= 0) {
		int err2 = g_OpenSSL.SSL_get_error(ssl->session, err);
		switch (err2) {
			case SSL_ERROR_ZERO_RETURN:
				SslLog("SSL connection gracefully closed");
				ssl->state = sockClosed;
				break;
			default:
				SslLog("SSL failure recieving data (%d, %d, %d)", err, err2, WSAGetLastError());
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

	int err = g_OpenSSL.SSL_write(ssl->session, buf, num);
	if (err <= 0) {
		int err2 = g_OpenSSL.SSL_get_error(ssl->session, err);
		switch (err2) {
			case SSL_ERROR_ZERO_RETURN:
				SslLog("SSL connection gracefully closed");
				ssl->state = sockClosed;
				break;
			default:
				SslLog("SSL failure sending data (%d, %d, %d)", err, err2, WSAGetLastError());
				ssl->state = sockError;
				return SOCKET_ERROR;
		}
		return 0;
	}
	return err;
}

static INT_PTR GetSslApi(WPARAM, LPARAM lParam)
{
	SSL_API* si = (SSL_API*)lParam;
	if (si == NULL) return FALSE;

	if (si->cbSize != sizeof(SSL_API))
		return FALSE;

	si->connect = (HSSL (__cdecl *)(SOCKET, const char *, int))NetlibSslConnect;
	si->pending = (BOOL (__cdecl *)(HSSL))NetlibSslPending;
	si->read = (int  (__cdecl *)(HSSL, char *, int, int))NetlibSslRead;
	si->write = (int  (__cdecl *)(HSSL, const char *, int))NetlibSslWrite;
	si->shutdown = (void (__cdecl *)(HSSL))NetlibSslShutdown;
	si->sfree = (void (__cdecl *)(HSSL))NetlibSslFree;

	return TRUE;
}

int LoadSslModule(void)
{
	CreateServiceFunction(MS_SYSTEM_GET_SI, GetSslApi);
	g_hSslMutex = CreateMutex(NULL, FALSE, NULL);

	return 0;
}

void UnloadSslModule(void)
{
	CloseHandle(g_hSslMutex);
	SSL_library_unload();
}
