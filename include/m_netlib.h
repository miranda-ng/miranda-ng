/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-12 Miranda ICQ/IM project,
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

#ifndef M_NETLIB_H__
#define M_NETLIB_H__ 1

#include "m_utils.h"

/////////////////////////////////////////////////////////////////////////////////////////
// this module was created in 0.1.2.2
// All error codes are returned via GetLastError() (or WSAGetLastError():
// they're the same).
// This module is thread-safe where it is sensible for it to be so. This
// basically means that you can call anything from any thread, but don't try
// to predict what will happen if you try to recv() on the same connection from
// two different threads at the same time.
// Note that because the vast majority of the routines in this module return
// a pointer, I have decided to diverge from the rest of Miranda and go with
// the convention that functions return false on failure and nonzero on success.

struct NETLIBHTTPREQUEST;
struct NETLIBOPENCONNECTION;

#define NETLIB_USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.112 Safari/537.36"

/////////////////////////////////////////////////////////////////////////////////////////
// Initialises the netlib for a set of connections
// Returns a HNETLIBUSER to be used for future netlib calls, NULL on failure
// NOTE: Netlib is loaded after any plugins, so you need to wait until
//      ME_SYSTEM_MODULESLOADED before calling this function
// Netlib settings are stored under the module szSettingsModule
// All netlib settings being with "NL".
// The default settings for registered users that don't have any settings stored
// in the database are the same as those displayed by the <All connections> page
// of the netlib options page.
// Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY, ERROR_DUP_NAME

struct NETLIBUSER
{
	char *szSettingsModule;          // used for db settings and log
	MAllStrings szDescriptiveName;   // used in options dialog, already translated
	uint32_t flags;
	int minIncomingPorts;            // only if NUF_INCOMING. Will be used for validation of user input.
};

#define NUF_INCOMING      0x01  // binds incoming ports
#define NUF_OUTGOING      0x02  // makes outgoing plain connections
#define NUF_NOOPTIONS     0x08  // don't create an options page for this. szDescriptiveName is never used.
#define NUF_HTTPCONNS     0x10  // at least some connections are made for HTTP communication. Enables the HTTP proxy option in options.
#define NUF_NOHTTPSOPTION 0x20  // disable the HTTPS proxy option in options. Use this if all communication is HTTP.
#define NUF_UNICODE       0x40  // if set ptszDescriptiveName points to Unicode, otherwise it points to ANSI string

EXTERN_C MIR_APP_DLL(HNETLIBUSER) Netlib_RegisterUser(const NETLIBUSER *pDescr);

/////////////////////////////////////////////////////////////////////////////////////////
// Assign a Netlib user handle a set of dynamic HTTP headers to be used with all
//
// HTTP connections that enable the HTTP-use-sticky headers flag.
// The headers persist until cleared with lParam = NULL.
//
// All memory should be allocated by the caller using malloc() from MS_SYSTEM_GET_MMI
// Once it has passed to Netlib, Netlib is the owner of it, the caller should not refer to the memory
// In any way after this point.
//
// NOTE: The szHeaders parameter should be a NULL terminated string following the HTTP header syntax.
// This string will be injected verbatim, thus the user should be aware of setting strings that are not
// headers. This service is NOT THREAD SAFE, only a single thread is expected to set the headers and a single
// thread reading the pointer internally, stopping race conditions and mutual exclusion don't happen.
//
// Version 0.3.2a+ (2003/10/27)
//

EXTERN_C MIR_APP_DLL(int) Netlib_SetStickyHeaders(HNETLIBUSER nlu, const char *szHeaders);

/* Notes on HTTP gateway usage
When a connection is initiated through an HTTP proxy using
MS_NETLIB_OPENCONNECTION, netlib will GET nlu.szHttpGatewayHello and read
the replied headers. Once this succeeds nlu.pfnHttpGatewayInit will be called
with a valid handle to the connection, the NETLIBOPENCONNECTION structure that
MS_NETLIB_OPENCONNECTION was called with, and the replied HTTP headers as its
parameters. This function is responsible for recving and parsing the data then
calling MS_NETLIB_SETHTTPPROXYINFO with the appropriate information.
nlu.pfnHttpGatewayInit should return nonzero on success. If it returns zero
then the entire connection attempt will return signalling failure. If your
function needs to return an error code it can do so via SetLastError().
If nlu.pfnHttpGatewayInit returns success without having called
MS_NETLIB_SETHTTPPROXYINFO then the connection attempt will fail anyway.
If you need more fine-tuned control over the GET/POST URLs than just appending
sequence numbers you can call MS_NETLIB_SETHTTPPROXYINFO from within your
wrap/unwrap functions (see below).

Just prior to MS_NETLIB_OPENCONNECTION returning nlu.pfnHttpGatewayBegin is
called with the handle to the connection and the NETLIBOPENCONNECTION structure
as its parameters. This is for gateways that need special non-protocol
initialisation. If you do send any packets in this function, you probably want
to remember to use the MSG_NOHTTPGATEWAYWRAP flag. This function pointer can be
NULL if this functionality isn't needed. This function must return nonzero on
success. If it fails the connect attempt will return failure without changing
LastError.

Whenever MS_NETLIB_SEND is called on a connection through an HTTP proxy and
the MSG_NOHTTPGATEWAYWRAP flags is not set and nlu.pfnHttpGatewayWrapSend is
not NULL, nlu.pfnHttpGatewayWrapSend will be called *instead* of sending the
data. It is this function's responsibility to wrap the sending data
appropriately for transmission and call pfnNetlibSend to send it again.
The flags parameter to nlu.pfnHttpGatewayWrapSend should be passed straight
through to the pfnNetlibSend call. It has already been ORed with
MSG_NOHTTPGATEWAYWRAP. nlu.pfnHttpGatewayWrapSend should return the a
number of the same type as MS_NETLIB_SEND, ie the number of bytes sent or
SOCKET_ERROR. The number of wrapping bytes should be subtracted so that the
return value appears as if the proxy wasn't there.
pfnNetlibSend() is identical to CallService(MS_NETLIB_SEND, ...) but it's
quicker to call using this pointer than to do the CallService() lookup again.

Whenever an HTTP reply is received inside MS_NETLIB_RECV the headers and data
are read into memory. If the headers indicate success then the data is passed
to nlu.pfnHttpGatewayUnwrapRecv (if it's non-NULL) for processing. This
function should remove (and do other processing if necessary) all HTTP proxy
specific headers and return a pointer to the buffer whose size is returned in
*outBufLen. If the buffer needs to be resized then NetlibRealloc() should be
used for that purpose, *not* your own CRT's realloc(). NetlibRealloc() behaves
identically to realloc() so it's possible to free the original buffer and
create a new one if that's the most sensible way to write your parser.
If errors are encountered you should SetLastError() and return NULL;
MS_NETLIB_RECV will return SOCKET_ERROR. If the passed buffer unwraps to
contain no actual data you should set *outBufLen to 0 but make sure you return
some non-NULL buffer that can be freed.

When you call MS_NETLIB_SEND or MS_NETLIB_RECV from any of these functions, you
should use the MSG_DUMPPROXY flag so that the logging is neat.
*/

#define PROXYTYPE_SOCKS4   1
#define PROXYTYPE_SOCKS5   2
#define PROXYTYPE_HTTP     3
#define PROXYTYPE_HTTPS    4
#define PROXYTYPE_IE       5

struct NETLIBUSERSETTINGS
{
	int cbSize;                 // to be filled in before calling
	int useProxy;	            // 1 or 0
	int proxyType;	            // a PROXYTYPE_
	char *szProxyServer;        // can be NULL
	int wProxyPort;             // host byte order
	int useProxyAuth;           // 1 or 0. Always 0 for SOCKS4
	char *szProxyAuthUser;      // can be NULL, always used by SOCKS4
	char *szProxyAuthPassword;  // can be NULL
	int useProxyAuthNtlm;       // 1 or 0, only used by HTTP, HTTPS
	int dnsThroughProxy;        // 1 or 0
	int specifyIncomingPorts;   // 1 or 0
	char *szIncomingPorts;      // can be NULL. Of form "1024-1050, 1060-1070, 2000"
	int specifyOutgoingPorts;   // 0.3.3a+
	char *szOutgoingPorts;      // 0.3.3a+
	int enableUPnP;             // 0.6.1+ only for NUF_INCOMING
	int validateSSL;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Gets the user-configured settings for a netlib user
//
// Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
// The pointers referred to in the returned struct will remain valid until
// the hUser handle is closed, or until the user changes the settings in the
// options page, so it's best not to rely on them for too long.
// Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(int) Netlib_GetUserSettings(HNETLIBUSER nlu, NETLIBUSERSETTINGS *result);

/////////////////////////////////////////////////////////////////////////////////////////
//Gets the user-configured settings for a netlib user idetified by name
//
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This function behaves like Netlib_GetUserSettings but the user is identified
//by the name provided by registration. When the name is not found NETLIBUSERSETTINGS is set to NULL.
//Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(int) Netlib_GetUserSettingsByName(char * UserSettingsName, NETLIBUSERSETTINGS *result);

/////////////////////////////////////////////////////////////////////////////////////////
// Changes the user-configurable settings for a netlib user
//
// Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
// This function is only really useful for people that specify NUF_NOOPTIONS
// and want to create their own options.
// Even if a setting is not active (eg szProxyAuthPassword when useProxyAuth is
// zero) that settings is still set for use in the options dialog.
// Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(int) Netlib_SetUserSettings(HNETLIBUSER nlu, const NETLIBUSERSETTINGS *result);

/////////////////////////////////////////////////////////////////////////////////////////
//Changes the user-configurable settings for a netlib user idetified by name
//
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This function behaves like Netlib_SetUserSettings but the user is identified
//by the name provided by registration. Nothing will be changed when the name is not found.
//Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(int) Netlib_SetUserSettingsByName(char * UserSettingsName, NETLIBUSERSETTINGS *result);

/////////////////////////////////////////////////////////////////////////////////////////
// Closes a netlib handle
//
// Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
// This function should be called on all handles returned by netlib functions
// once you are done with them. If it's called on a socket-type handle, the
// socket will be closed.
// Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(int) Netlib_CloseHandle(HANDLE h);

/////////////////////////////////////////////////////////////////////////////////////////
// Open a port and wait for connections on it
//
// Returns a HANDLE on success, NULL on failure
// hUser should have been returned by MS_NETLIB_REGISTERUSER
// This function does the equivalent of socket(), bind(), getsockname(),
// listen(), accept()
// Internally this function creates a new thread which waits around in accept()
// for new connections. When one is received it calls nlb.pfnNewConnection *from
// this new thread* and then loops back to wait again.
// Close the returned handle to end the thread and close the open port.
// Errors: ERROR_INVALID_PARAMETER, any returned by socket() or bind() or
//   listen() or getsockname()
//
// Notes:
//
// During development of 0.3.1a+ (2003/07/04) passing wPort != 0
// will result in an attempt to bind on the port given in wPort
// if this port is taken then you will get an error, so be sure to check
// for such conditions.
//
// passing wPort != 0 is for people who need to open a set port for
// daemon activities, usually passing wPort == 0 is what you want and
// will result in a free port given by the TCP/IP socket layer and/or
// seeded from the user selected port ranges.
//
// also note that wPort if != 0, will have be converted to network byte order
//
/* pExtra was added during 0.3.4+, prior its just two args, since we use the cdecl convention
it shouldnt matter */

typedef void (*NETLIBNEWCONNECTIONPROC)(HNETLIBCONN hNewConnection, uint32_t dwRemoteIP, void *pExtra);

struct NETLIBBIND
{
	NETLIBNEWCONNECTIONPROC pfnNewConnection;

	// function to call when there's a new connection. Params are: the
	// new connection, IP of remote machine (host byte order)
	uint32_t dwInternalIP;   // set on return, host byte order
	uint32_t dwExternalIP;   // set on return, host byte order
	uint16_t wPort, wExPort; // set on return, host byte order
	void *pExtra;         // argument is sent to callback
};

EXTERN_C MIR_APP_DLL(HNETLIBBIND) Netlib_BindPort(HNETLIBUSER nlu, NETLIBBIND *nlb);

/////////////////////////////////////////////////////////////////////////////////////////
// Open a connection
//
// Returns a HNETLIBCONN to the new connection on success, NULL on failure
// hUser must have been returned by MS_NETLIB_REGISTERUSER
// Internally this function is the equivalent of socket(), gethostbyname(),
// connect()
// If NLOCF_HTTP is set and hUser is configured for an HTTP or HTTPS proxy then
// this function will connect() to the proxy server only, without performing any
// initialisation conversation.
// If hUser is configured for an HTTP proxy and does not support HTTP gateways
// and you try to open a connection without specifying NLOCF_HTTP then this
// function will first attempt to open an HTTPS connection, if that fails it
// will try a direct connection, if that fails it will return failure with the
// error from the connect() during the direct connection attempt.
// Errors: ERROR_INVALID_PARAMETER, any returned by socket(), gethostbyname(),
//          connect(), MS_NETLIB_SEND, MS_NETLIB_RECV, select()
//    ERROR_TIMEOUT (during proxy communication)
//    ERROR_BAD_FORMAT (very invalid proxy reply)
//    ERROR_ACCESS_DENIED (by proxy)
//    ERROR_CONNECTION_UNAVAIL (socks proxy can't connect to identd)
//    ERROR_INVALID_ACCESS (proxy refused identd auth)
//    ERROR_INVALID_DATA (proxy returned invalid code)
//    ERROR_INVALID_ID_AUTHORITY (proxy requires use of auth method that's not supported)
//    ERROR_GEN_FAILURE (socks5/https general failure)
//    ERROR_CALL_NOT_IMPLEMENTED (socks5 command not supported)
//    ERROR_INVALID_ADDRESS (socks5 address type not supported)
//    HTTP: anything from nlu.pfnHttpGatewayInit, nlu.pfnHttpGatewayBegin,
//          MS_NETLIB_SENDHTTPREQUEST or MS_NETLIB_RECVHTTPHEADERS

#define NLOCF_HTTP          0x0001 // this connection will be used for HTTP communications. If configured for an HTTP/HTTPS proxy the connection is opened as if there was no proxy.
#define NLOCF_STICKYHEADERS 0x0002 // this connection should send the sticky headers associated with NetLib user apart of any HTTP request
#define NLOCF_UDP           0x0008 // this connection is UDP
#define NLOCF_SSL           0x0010 // this connection is SSL

EXTERN_C MIR_APP_DLL(HNETLIBCONN) Netlib_OpenConnection(HNETLIBUSER nlu, const char *szHost, int port, int timeout = 0, int flags = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// Sets the required information for an HTTP proxy connection
//
// Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
// This function is designed to be called from within pfnHttpGatewayInit
// See notes below MS_NETLIB_REGISTERUSER.
// Errors: ERROR_INVALID_PARAMETER

#define NLHPIF_USEGETSEQUENCE      0x0001   // append sequence numbers to GET requests
#define NLHPIF_USEPOSTSEQUENCE     0x0002   // append sequence numbers to POST requests
#define NLHPIF_GETPOSTSAMESEQUENCE 0x0004	// GET and POST use the same sequence
#define NLHPIF_HTTP11              0x0008	// HTTP 1.1 proxy

struct NETLIBHTTPPROXYINFO
{
	uint32_t flags;
	int firstGetSequence, firstPostSequence;
	int combinePackets;
	char *szHttpPostUrl;
	char *szHttpGetUrl;
};

EXTERN_C MIR_APP_DLL(int) Netlib_SetHttpProxyInfo(HNETLIBCONN hConnection, const NETLIBHTTPPROXYINFO *nlhpi);

/////////////////////////////////////////////////////////////////////////////////////////
// Gets the SOCKET associated with a netlib handle
//
// Returns the SOCKET on success, INVALID_SOCKET on failure
// hNetlibHandle should have been returned by MS_NETLIB_BINDPORT or
// MS_NETLIB_OPENCONNECTION only.
// Be careful how you use this socket because you might be connected via an
// HTTP proxy in which case calling send() or recv() will totally break things.
// Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(UINT_PTR) Netlib_GetSocket(HNETLIBCONN hConnection);

/////////////////////////////////////////////////////////////////////////////////////////

#define Netlib_GetBase64DecodedBufferSize(cchEncoded)  (((cchEncoded)>>2)*3)
#define Netlib_GetBase64EncodedBufferSize(cbDecoded)  (((cbDecoded)*4+11)/12*4+1)

/////////////////////////////////////////////////////////////////////////////////////////
// Gets HNETLIBUSER owner of a connection

EXTERN_C MIR_APP_DLL(HNETLIBUSER) Netlib_GetConnNlu(HNETLIBCONN hConn);

/////////////////////////////////////////////////////////////////////////////////////////
// Gets the fake User-Agent header field to make some sites happy

EXTERN_C MIR_APP_DLL(char*) Netlib_GetUserAgent();

/////////////////////////////////////////////////////////////////////////////////////////
// Converts numerical representation of IP in SOCKADDR_INET into string representation with IP and port
// IPv4 will be supplied in formats address:port or address
// IPv6 will be supplied in formats [address]:port or [address]
// Returns pointer to the string or NULL if not successful

struct sockaddr_in;
EXTERN_C MIR_APP_DLL(char*) Netlib_AddressToString(sockaddr_in *addr);
EXTERN_C MIR_APP_DLL(bool)  Netlib_StringToAddress(const char *str, sockaddr_in *addr);

/////////////////////////////////////////////////////////////////////////////////////////
// Gets connection Information
// IPv4 will be supplied in formats address:port or address
// IPv6 will be supplied in formats [address]:port or [address]
// Returns 0 if successful

struct NETLIBCONNINFO
{
	char szIpPort[64];
	unsigned dwIpv4;
	uint16_t wPort;
};

EXTERN_C MIR_APP_DLL(int) Netlib_GetConnectionInfo(HNETLIBCONN hConnection, NETLIBCONNINFO *connInfo);

/////////////////////////////////////////////////////////////////////////////////////////
// Gets connection Information
//
// Returns (INT_PTR)(NETLIBIPLIST*) numeric IP address address array
// the last element of the array is all 0s, 0 if not successful

struct NETLIBIPLIST
{
	unsigned cbNum;
	char szIp[1][64];
};

EXTERN_C MIR_APP_DLL(NETLIBIPLIST*) Netlib_GetMyIp(bool bGlobalOnly);

/////////////////////////////////////////////////////////////////////////////////////////
// Send an HTTP request over a connection
//
// Returns number of bytes sent on success, SOCKET_ERROR on failure
// hConnection must have been returned by MS_NETLIB_OPENCONNECTION
// Note that if you use NLHRF_SMARTAUTHHEADER and NTLM authentication is in use
// then the full NTLM authentication transaction occurs, comprising sending the
// domain, receiving the challenge, then sending the response.
// nlhr.resultCode and nlhr.szResultDescr are ignored by this function.
// Errors: ERROR_INVALID_PARAMETER, anything returned by MS_NETLIB_SEND

struct NETLIBHTTPHEADER
{
	char *szName;
	char *szValue;
};

EXTERN_C MIR_APP_DLL(char*) Netlib_GetHeader(const NETLIBHTTPREQUEST *pRec, const char *pszName);

/////////////////////////////////////////////////////////////////////////////////////////

#define REQUEST_RESPONSE 0	// used by structure returned by MS_NETLIB_RECVHTTPHEADERS
#define REQUEST_GET      1
#define REQUEST_POST     2
#define REQUEST_CONNECT  3
#define REQUEST_HEAD  	 4
#define REQUEST_PUT      5
#define REQUEST_DELETE   6
#define REQUEST_PATCH    7

#define NLHRF_MANUALHOST      0x00000001   // do not remove any host and/or protocol portion of szUrl before sending it
#define NLHRF_HTTP11          0x00000010   // use HTTP 1.1
#define NLHRF_PERSISTENT      0x00000020   // preserve connection on exit, open connection provided in the nlc field of the reply
                                           // it should be supplied in nlc field of request for reuse or closed if not needed
#define NLHRF_SSL             0x00000040   // use SSL connection
#define NLHRF_NOPROXY         0x00000080   // do not use proxy server
#define NLHRF_REDIRECT        0x00000100   // handle HTTP redirect requests (response 30x), the resulting url provided in szUrl of the response
#define NLHRF_NODUMP          0x00010000   // never dump this to the log
#define NLHRF_NODUMPHEADERS   0x00020000   // don't dump http headers (only useful for POSTs and MS_NETLIB_HTTPTRANSACTION)
#define NLHRF_DUMPPROXY       0x00040000   // this transaction is a proxy communication. For dump filtering only.
#define NLHRF_DUMPASTEXT      0x00080000   // dump posted and reply data as text. Headers are always dumped as text.
#define NLHRF_NODUMPSEND      0x00100000   // do not dump sent message.

struct NETLIBHTTPREQUEST
{
	int cbSize;
	int requestType;	// a REQUEST_
	uint32_t flags;
	char *szUrl;
	NETLIBHTTPHEADER *headers;	 // If this is a POST request and headers doesn't contain a Content-Length it'll be added automatically
	int headersCount;
	char *pData;   // data to be sent in POST request.
	int dataLength;		 // must be 0 for REQUEST_GET/REQUEST_CONNECT
	int resultCode;
	char *szResultDescr;
	HNETLIBCONN nlc;
	int timeout;

	__forceinline const char *operator[](const char *pszName) {
		return Netlib_GetHeader(this, pszName);
	}
};

EXTERN_C MIR_APP_DLL(int) Netlib_SendHttpRequest(HNETLIBCONN hConnection, NETLIBHTTPREQUEST *pRec);

/////////////////////////////////////////////////////////////////////////////////////////
// Receives HTTP headers
//
// Returns a pointer to a NETLIBHTTPREQUEST structure on success, NULL on failure.
// Call Netlib_FreeHttpRequest() to free this.
// hConnection must have been returned by MS_NETLIB_OPENCONNECTION
// nlhr->pData = NULL and nlhr->dataLength = 0 always. The requested data should
// be retrieved using MS_NETLIB_RECV once the header has been parsed.
// If the headers haven't finished within 60 seconds the function returns NULL
// and ERROR_TIMEOUT.
// Errors: ERROR_INVALID_PARAMETER, any from MS_NETLIB_RECV or select()
//    ERROR_HANDLE_EOF (connection closed before headers complete)
//    ERROR_TIMEOUT (headers still not complete after 60 seconds)
//    ERROR_BAD_FORMAT (invalid character or line ending in headers, or first line is blank)
//    ERROR_BUFFER_OVERFLOW (each header line must be less than 4096 chars long)
//    ERROR_INVALID_DATA (first header line is malformed ("http/[01].[0-9] [0-9]+ .*", or no colon in subsequent line)

EXTERN_C MIR_APP_DLL(NETLIBHTTPREQUEST*) Netlib_RecvHttpHeaders(HNETLIBCONN hConnection, int flags = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// Free the memory used by a NETLIBHTTPREQUEST structure
//
// Returns true on success, false on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
// This should only be called on structures returned by
// MS_NETLIB_RECVHTTPHEADERS or MS_NETLIB_HTTPTRANSACTION. Calling it on an
// arbitrary structure will have disastrous results.
// Errors: ERROR_INVALID_PARAMETER

EXTERN_C MIR_APP_DLL(bool) Netlib_FreeHttpRequest(NETLIBHTTPREQUEST*);

/////////////////////////////////////////////////////////////////////////////////////////
// smart pointer for NETLIBHTTPREQUEST via a call of Netlib_FreeHttpRequest()

#ifdef __cplusplus
class NLHR_PTR
{
protected:
	NETLIBHTTPREQUEST *_p;

public:
	__forceinline explicit NLHR_PTR(NETLIBHTTPREQUEST *p) : _p(p) {}

	__forceinline NETLIBHTTPREQUEST* operator=(INT_PTR i_p)
	{
		return operator=((NETLIBHTTPREQUEST*)i_p);
	}
	__forceinline NETLIBHTTPREQUEST* operator=(NETLIBHTTPREQUEST *p)
	{
		if (_p)
			Netlib_FreeHttpRequest(_p);
		_p = p;
		return _p;
	}
	__forceinline operator NETLIBHTTPREQUEST*() const { return _p; }
	__forceinline NETLIBHTTPREQUEST* operator->() const { return _p; }
	__forceinline ~NLHR_PTR()
	{
		Netlib_FreeHttpRequest(_p);
	}
};

struct MIR_APP_EXPORT MHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	MHttpRequest();
	~MHttpRequest();

	CMStringA m_szUrl;
	CMStringA m_szParam;
	void *pUserInfo = nullptr;

	void AddHeader(const char *szName, const char *szValue);
};

template <class T>
class MTHttpRequest : public MHttpRequest
{
public:
	__forceinline MTHttpRequest()
	{}

	typedef void (T::*MTHttpRequestHandler)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);
	MTHttpRequestHandler m_pFunc = nullptr;
};

MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest*, const INT_PARAM&);
MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest*, const INT64_PARAM&);
MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest*, const CHAR_PARAM&);
MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest*, const WCHAR_PARAM&);

#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Do an entire HTTP transaction
//
// Returns a pointer to another NETLIBHTTPREQUEST structure on success, NULL on failure.
// Call Netlib_FreeHttpRequest() to free this.
// hUser must have been returned by MS_NETLIB_REGISTERUSER
// nlhr.szUrl should be a full HTTP URL. If it does not start with http:// , that
// will be assumed (but it's best not to use this fact, for reasons of
// extensibility).
// This function is the equivalent of MS_NETLIB_OPENCONNECTION,
// MS_NETLIB_SENDHTTPREQ, MS_NETLIB_RECVHTTPHEADERS, MS_NETLIB_RECV,
// MS_NETLIB_CLOSEHANDLE
// nlhr.headers will be augmented with the following headers unless they have
// already been set by the caller:
//  "Host" (regardless of whether it is requested in nlhr.flags)
//  "User-Agent"  (of the form "Miranda/0.1.2.2 (alpha)" or "Miranda/0.1.2.2")
//  "Content-Length" (for POSTs only. Set to nlhr.dataLength)
// If you do not want to send one of these headers, create a nlhr.headers with
// szValue = NULL.
// In the return value headers, headerCount, pData, dataLength, resultCode and
// szResultDescr are all valid.
// In the return value pData[dataLength] == 0 always, as an extra safeguard
// against programming slips.
// Note that the function can succeed (ie not return NULL) yet result in an HTTP
// error code. You should check that resultCode == 2xx before proceeding.
// Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY, anything from the above
//    list of functions

EXTERN_C MIR_APP_DLL(NETLIBHTTPREQUEST*) Netlib_HttpTransaction(HNETLIBUSER hNlu, NETLIBHTTPREQUEST *pRequest);

/////////////////////////////////////////////////////////////////////////////////////////
// Send data over a connection
//
// Returns the number of bytes sent on success, SOCKET_ERROR on failure
// Errors: ERROR_INVALID_PARAMETER
//        anything from send(), nlu.pfnHttpGatewayWrapSend()
//        HTTP proxy: ERROR_GEN_FAILURE (http result code wasn't 2xx)
//                    anything from socket(), connect(),
//                    MS_NETLIB_SENDHTTPREQUEST, MS_NETLIB_RECVHTTPHEADERS
// flags:

#define MSG_NOHTTPGATEWAYWRAP  0x010000	 // don't wrap the outgoing packet using nlu.pfnHttpGatewayWrapSend
#define MSG_NODUMP             0x020000    // don't dump this packet to the log
#define MSG_DUMPPROXY          0x040000	 // this is proxy communiciation. For dump filtering only.
#define MSG_DUMPASTEXT         0x080000    // this is textual data, don't dump as hex
#define MSG_RAW                0x100000	 // send as raw data, bypass any HTTP proxy stuff
#define MSG_DUMPSSL            0x200000	 // this is SSL traffic. For dump filtering only.
#define MSG_NOTITLE            0x400000	 // skip date, time & protocol from dump

EXTERN_C MIR_APP_DLL(int) Netlib_Send(HNETLIBCONN hConn, const char *buf, int len, int flags = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// Receive data over a connection
//
// Returns the number of bytes read on success, SOCKET_ERROR on failure,
// 0 if the connection has been closed
// Flags supported: MSG_PEEK, MSG_NODUMP, MSG_DUMPPROXY, MSG_NOHTTPGATEWAYWRAP,
//                 MSG_DUMPASTEXT, MSG_RAW
// On using MSG_NOHTTPGATEWAYWRAP: Because packets through an HTTP proxy are
//  batched and cached and stuff, using this flag is not a guarantee that it
//  will be obeyed, and if it is it may even be propogated to future calls
//  even if you don't specify it then. Because of this, the flag should be
//  considered an all-or-nothing thing: either use it for the entire duration
//  of a connection, or not at all.
// Errors: ERROR_INVALID_PARAMETER, anything from recv()
//        HTTP proxy: ERROR_GEN_FAILURE (http result code wasn't 2xx)
// 					  ERROR_INVALID_DATA (no Content-Length header in reply)
//                    ERROR_NOT_ENOUGH_MEMORY (Content-Length very large)
//                    ERROR_HANDLE_EOF (connection closed before Content-Length bytes recved)
//                    anything from select(), MS_NETLIB_RECVHTTPHEADERS,
// 						  nlu.pfnHttpGatewayUnwrapRecv, socket(), connect(),
// 						  MS_NETLIB_SENDHTTPREQUEST

EXTERN_C MIR_APP_DLL(int) Netlib_Recv(HNETLIBCONN hConn, char *buf, int len, int flags = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// Determine the status of one or more connections
// Returns the number of ready connections, SOCKET_ERROR on failure, 0 if the timeout expired.
// All handles passed to this function must have been returned by either
// MS_NETLIB_OPENCONNECTION or MS_NETLIB_BINDPORT.
// The last handle in each list must be followed by either NULL or INVALID_HANDLE_VALUE.
// Errors: ERROR_INVALID_HANDLE, ERROR_INVALID_DATA, anything from select()

struct NETLIBSELECT
{
	uint32_t dwTimeout; // in milliseconds, INFINITE is acceptable
	HNETLIBCONN hReadConns[FD_SETSIZE + 1];
	HNETLIBCONN hWriteConns[FD_SETSIZE + 1];
	HNETLIBCONN hExceptConns[FD_SETSIZE + 1];
};

EXTERN_C MIR_APP_DLL(int) Netlib_Select(NETLIBSELECT *nls);

struct NETLIBSELECTEX
{
	uint32_t dwTimeout; // in milliseconds, INFINITE is acceptable
	HNETLIBCONN hReadConns[FD_SETSIZE + 1];
	HNETLIBCONN hWriteConns[FD_SETSIZE + 1];
	HNETLIBCONN hExceptConns[FD_SETSIZE + 1];

	BOOL hReadStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
	BOOL hWriteStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
	BOOL hExceptStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
};

EXTERN_C MIR_APP_DLL(int) Netlib_SelectEx(NETLIBSELECTEX *nls);

/////////////////////////////////////////////////////////////////////////////////////////
// Shutdown connection

EXTERN_C MIR_APP_DLL(void) Netlib_Shutdown(HNETLIBCONN h);

/////////////////////////////////////////////////////////////////////////////////////////
// Create a packet receiver
//
// Returns a HANDLE on success, NULL on failure
// The packet receiver implements the common situation where you have variable
// length packets coming in over a connection and you want to split them up
// in order to handle them.
// The major limitation is that the buffer is created in memory, so you can't
// have arbitrarily large packets.
// Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY

EXTERN_C MIR_APP_DLL(HANDLE) Netlib_CreatePacketReceiver(HNETLIBCONN hConnection, int iMaxSize);

/////////////////////////////////////////////////////////////////////////////////////////
// Get the next set of packets from a packet receiver
//
// Returns the total number of bytes available in the buffer, 0 if the
// connection was closed, SOCKET_ERROR on error.
// hPacketRecver must have been returned by MS_NETLIB_CREATEPACKETRECVER
// If nlpr.bytesUsed is set to zero and the buffer is already full up to
// maxPacketSize, it is assumed that too large a packet has been received. All
// data in the buffer is discarded and receiving is begun anew. This will
// probably cause alignment problems so if you think this is likely to happen
// then you should deal with it yourself.
// Closing the packet receiver will not close the associated connection, but
// will discard any bytes still in the buffer, so if you intend to carry on
// reading from that connection, make sure you have processed the buffer first.
// This function is the equivalent of a memmove() to remove the first bytesUsed
// from the buffer, select() if dwTimeout is not INFINITE, then MS_NETLIB_RECV.
// Errors: ERROR_INVALID_PARAMETER, ERROR_TIMEOUT,
//        anything from select(), MS_NETLIB_RECV

struct NETLIBPACKETRECVER
{
	uint32_t dwTimeout; // fill before calling. In milliseconds. INFINITE is valid
	int bytesUsed;      // fill before calling. This many bytes are removed from the start of the buffer. Set to 0 on return
	int bytesAvailable; // equal to the return value, unless the return value is 0
	int bufferSize;     // same as parameter to MS_NETLIB_CREATEPACKETRECVER
	uint8_t *buffer;    // contains the recved data
};

EXTERN_C MIR_APP_DLL(int) Netlib_GetMorePackets(HANDLE hReceiver, NETLIBPACKETRECVER *nlprParam);

/////////////////////////////////////////////////////////////////////////////////////////
// Sets a gateway polling timeout interval
//
// Returns previous timeout value
// Errors: -1

EXTERN_C MIR_APP_DLL(int) Netlib_SetPollingTimeout(HNETLIBCONN hConnection, int iTimeout);

/////////////////////////////////////////////////////////////////////////////////////////
// netlib log funcitons

EXTERN_C MIR_APP_DLL(int) Netlib_Log(HNETLIBUSER hUser, const char *pszStr);
EXTERN_C MIR_APP_DLL(int) Netlib_LogW(HNETLIBUSER hUser, const wchar_t *pwszStr);

EXTERN_C MIR_APP_DLL(int) Netlib_Logf(HNETLIBUSER hUser, _Printf_format_string_ const char *fmt, ...);
EXTERN_C MIR_APP_DLL(int) Netlib_LogfW(HNETLIBUSER hUser, _Printf_format_string_ const wchar_t *fmt, ...);

EXTERN_C MIR_APP_DLL(void) Netlib_Dump(HNETLIBCONN nlc, const void *buf, size_t len, bool bIsSent, int flags);

// Inits a required security provider. Right now only NTLM is supported
// Returns HANDLE = NULL on error or non-null value on success
// Known providers: Basic, NTLM, Negotiate, Kerberos, GSSAPI - (Kerberos SASL)
EXTERN_C MIR_APP_DLL(HANDLE) Netlib_InitSecurityProvider(const wchar_t *szProviderName, const wchar_t *szPrincipal = nullptr);

// Destroys a security provider's handle, provided by Netlib_InitSecurityProvider.
// Right now only NTLM is supported
EXTERN_C MIR_APP_DLL(void) Netlib_DestroySecurityProvider(HANDLE hProvider);

// Returns the NTLM response string. The result value should be freed using mir_free
EXTERN_C MIR_APP_DLL(char*) Netlib_NtlmCreateResponse(HANDLE hProvider, const char *szChallenge, wchar_t *szLogin, wchar_t *szPass, unsigned &complete);

/////////////////////////////////////////////////////////////////////////////////////////
// SSL/TLS support

#if !defined(HSSL_DEFINED)
DECLARE_HANDLE(HSSL);
#endif

// Makes connection SSL
// Returns 0 on failure 1 on success
EXTERN_C MIR_APP_DLL(int) Netlib_StartSsl(HNETLIBCONN hConnection, const char *host);

// negotiates SSL session, verifies cert, returns NULL if failed
EXTERN_C MIR_APP_DLL(HSSL) Netlib_SslConnect(SOCKET s, const char* host, int verify);

// return true if there is either unsend or buffered received data (ie. after peek)
EXTERN_C MIR_APP_DLL(BOOL) Netlib_SslPending(HSSL ssl);

// reads number of bytes, keeps in buffer if peek != 0
EXTERN_C MIR_APP_DLL(int)  Netlib_SslRead(HSSL ssl, char *buf, int num, int peek);

// writes data to the SSL socket
EXTERN_C MIR_APP_DLL(int)  Netlib_SslWrite(HSSL ssl, const char *buf, int num);

// closes SSL session, but keeps socket open
EXTERN_C MIR_APP_DLL(void) Netlib_SslShutdown(HSSL ssl);

// frees all data associated with the SSL socket
EXTERN_C MIR_APP_DLL(void) Netlib_SslFree(HSSL ssl);

// gets TLS channel binging data for a socket
EXTERN_C MIR_APP_DLL(void*) Netlib_GetTlsUnique(HNETLIBCONN nlc, int &cbLen, int &tlsVer);

/////////////////////////////////////////////////////////////////////////////////////////
// WebSocket support

struct WSHeader
{
	WSHeader()
	{
		memset(this, 0, sizeof(*this));
	}

	bool bIsFinal, bIsMasked;
	int opCode, firstByte;
	size_t payloadSize, headerSize;
};

// connects to a WebSocket server
EXTERN_C MIR_APP_DLL(NETLIBHTTPREQUEST*) WebSocket_Connect(HNETLIBUSER, const char *szHost, NETLIBHTTPHEADER *pHeaders = nullptr);

// validates that the provided buffer contains full WebSocket datagram
EXTERN_C MIR_APP_DLL(bool) WebSocket_InitHeader(WSHeader &hdr, const void *pData, size_t bufSize);

// sends a packet to WebSocket
EXTERN_C MIR_APP_DLL(void) WebSocket_SendText(HNETLIBCONN nlc, const char *pData);
EXTERN_C MIR_APP_DLL(void) WebSocket_SendBinary(HNETLIBCONN nlc, const void *pData, size_t strLen);

/////////////////////////////////////////////////////////////////////////////////////////
// Netlib hooks (0.8+)

// WARNING: these hooks are being called in the context of the calling thread, without switching
// to the first thread, like all another events do. The hook procedure should be ready for the
// multithreaded mode
//
// Parameters:
//    wParam: NETLIBNOTIFY* - points to the data being sent/received
//    lParam: NETLIBUSER*   - points to the protocol definition

struct NETLIBNOTIFY
{
	const char *buf;
	int len;
	int flags;
	int result;          // amount of bytes really sent/received
};

#define ME_NETLIB_FASTRECV "Netlib/OnRecv"  // being called on every receive
#define ME_NETLIB_FASTSEND "Netlib/OnSend"  // being called on every send
#define ME_NETLIB_FASTDUMP "Netlib/OnDump"  // being called on every dump

struct NETLIBCONNECTIONEVENTINFO
{
	BOOL connected;      // 1-opening socket   0-closing socket
	BOOL listening;      // 1-bind             0-connect
	SOCKADDR_IN local;   // local IP+port (always used)
	SOCKADDR_IN remote;  // remote IP+port (only connect (opening + closing only if no proxy))
	SOCKADDR_IN proxy;   // proxy IP+port (only connect when used)
	char *szSettingsModule; // name of the registered Netlib user that requested the action
};

//This event is sent as a new port is bound or a new connection opened.
//It is NOT sent for sigle HTTP(S) requests.
//wParam=(WPARAM)(NETLIBCONNECTIONEVENTINFO*)hInfo
//lParam=(LPARAM)0 (not used)
#define ME_NETLIB_EVENT_CONNECTED "Netlib/Event/Connected"

//This event is sent if coneection or listening socket is closed.
//It is NOT sent for sigle HTTP(S) requests.
//wParam=(WPARAM)(NETLIBCONNECTIONEVENTINFO*)hInfo
//lParam=(LPARAM)0 (not used)
#define ME_NETLIB_EVENT_DISCONNECTED "Netlib/Event/Disconnected"

#endif // M_NETLIB_H__
