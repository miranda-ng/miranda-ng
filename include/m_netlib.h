/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

//this module was created in 0.1.2.2
//All error codes are returned via GetLastError() (or WSAGetLastError():
//they're the same).
//This module is thread-safe where it is sensible for it to be so. This
//basically means that you can call anything from any thread, but don't try
//to predict what will happen if you try to recv() on the same connection from
//two different threads at the same time.
//Note that because the vast majority of the routines in this module return
//a pointer, I have decided to diverge from the rest of Miranda and go with
//the convention that functions return false on failure and nonzero on success.

struct NETLIBHTTPREQUEST_tag;
typedef struct NETLIBHTTPREQUEST_tag NETLIBHTTPREQUEST;
struct NETLIBOPENCONNECTION_tag;
typedef struct NETLIBOPENCONNECTION_tag NETLIBOPENCONNECTION;

//Initialises the netlib for a set of connections
//wParam = 0
//lParam = (LPARAM)(NETLIBUSER*)&nu
//Returns a HANDLE to be used for future netlib calls, NULL on failure
//NOTE: Netlib is loaded after any plugins, so you need to wait until
//      ME_SYSTEM_MODULESLOADED before calling this function
//Netlib settings are stored under the module szSettingsModule
//All netlib settings being with "NL".
//The default settings for registered users that don't have any settings stored
//in the database are the same as those displayed by the <All connections> page
//of the netlib options page.
//See notes below this function for the behaviour of HTTP gateways
//Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY, ERROR_DUP_NAME
typedef int (*NETLIBHTTPGATEWAYINITPROC)(HANDLE hConn, NETLIBOPENCONNECTION *nloc, NETLIBHTTPREQUEST *nlhr);
typedef int (*NETLIBHTTPGATEWAYBEGINPROC)(HANDLE hConn, NETLIBOPENCONNECTION *nloc);
typedef int (*NETLIBHTTPGATEWAYWRAPSENDPROC)(HANDLE hConn, PBYTE buf, int len, int flags, MIRANDASERVICE pfnNetlibSend);
typedef PBYTE (*NETLIBHTTPGATEWAYUNWRAPRECVPROC)(NETLIBHTTPREQUEST *nlhr, PBYTE buf, int len, int *outBufLen, void *(*NetlibRealloc)(void*, size_t));
typedef struct {
	int cbSize;
	char *szSettingsModule;         //used for db settings and log
	union {
		char *szDescriptiveName;          //used in options dialog, already translated
		TCHAR *ptszDescriptiveName;
	};
	DWORD flags;
	char *szHttpGatewayHello;
	char *szHttpGatewayUserAgent;		 //can be NULL to send no user-agent, also used by HTTPS proxies
	NETLIBHTTPGATEWAYINITPROC pfnHttpGatewayInit;
	NETLIBHTTPGATEWAYBEGINPROC pfnHttpGatewayBegin;		 //can be NULL if no beginning required
	NETLIBHTTPGATEWAYWRAPSENDPROC pfnHttpGatewayWrapSend;  //can be NULL if no wrapping required
	NETLIBHTTPGATEWAYUNWRAPRECVPROC pfnHttpGatewayUnwrapRecv;  //can be NULL if no wrapping required
	int minIncomingPorts;     //only if NUF_INCOMING. Will be used for validation of user input.
} NETLIBUSER;
#define NUF_INCOMING      0x01  //binds incoming ports
#define NUF_OUTGOING      0x02  //makes outgoing plain connections
#define NUF_HTTPGATEWAY   0x04  //can use HTTP gateway for plain sockets. ???HttpGateway* are valid.  Enables the HTTP proxy option in options.
#define NUF_NOOPTIONS     0x08  //don't create an options page for this. szDescriptiveName is never used.
#define NUF_HTTPCONNS     0x10  //at least some connections are made for HTTP communication. Enables the HTTP proxy option in options.
#define NUF_NOHTTPSOPTION 0x20  //disable the HTTPS proxy option in options. Use this if all communication is HTTP.
#define NUF_UNICODE 0x40  //if set ptszDescriptiveName points to Unicode, otherwise it points to ANSI string
#define MS_NETLIB_REGISTERUSER   "Netlib/RegisterUser"

#if defined(_UNICODE)
	#define NUF_TCHAR NUF_UNICODE
#else
	#define NUF_TCHAR 0
#endif



//Assign a Netlib user handle a set of dynamic HTTP headers to be used with all
//
//HTTP connections that enable the HTTP-use-sticky headers flag.
//The headers persist until cleared with lParam = NULL.
//
//All memory should be allocated by the caller using malloc() from MS_SYSTEM_GET_MMI
//Once it has passed to Netlib, Netlib is the owner of it, the caller should not refer to the memory
//In any way after this point.
//
//wParam = (WPARAM)hNetLibUser
//lParam = (LPARAM)(char*)szHeaders
//
//NOTE: The szHeaders parameter should be a NULL terminated string following the HTTP header syntax.
//This string will be injected verbatim, thus the user should be aware of setting strings that are not
//headers. This service is NOT THREAD SAFE, only a single thread is expected to set the headers and a single
//thread reading the pointer internally, stopping race conditions and mutual exclusion don't happen.
//
//Version 0.3.2a+ (2003/10/27)
//
#define MS_NETLIB_SETSTICKYHEADERS "Netlib/SetStickyHeaders"

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

//Gets the user-configured settings for a netlib user
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(NETLIBUSERSETTINGS*)&nlus
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//The pointers referred to in the returned struct will remain valid until
//the hUser handle is closed, or until the user changes the settings in the
//options page, so it's best not to rely on them for too long.
//Errors: ERROR_INVALID_PARAMETER
#define PROXYTYPE_SOCKS4   1
#define PROXYTYPE_SOCKS5   2
#define PROXYTYPE_HTTP     3
#define PROXYTYPE_HTTPS    4
#define PROXYTYPE_IE       5
typedef struct {
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
} NETLIBUSERSETTINGS;
#define MS_NETLIB_GETUSERSETTINGS  "Netlib/GetUserSettings"

//Changes the user-configurable settings for a netlib user
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(NETLIBUSERSETTINGS*)&nlus
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This function is only really useful for people that specify NUF_NOOPTIONS
//and want to create their own options.
//Even if a setting is not active (eg szProxyAuthPassword when useProxyAuth is
//zero) that settings is still set for use in the options dialog.
//Errors: ERROR_INVALID_PARAMETER
#define MS_NETLIB_SETUSERSETTINGS  "Netlib/SetUserSettings"

//Closes a netlib handle
//wParam = (WPARAM)(HANDLE)hNetlibHandle
//lParam = 0
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This function should be called on all handles returned by netlib functions
//once you are done with them. If it's called on a socket-type handle, the
//socket will be closed.
//Errors: ERROR_INVALID_PARAMETER
#define MS_NETLIB_CLOSEHANDLE   "Netlib/CloseHandle"
__forceinline INT_PTR Netlib_CloseHandle(HANDLE h) {return CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)h, 0);}

//Open a port and wait for connections on it
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(NETLIBBIND*)&nlb
//Returns a HANDLE on success, NULL on failure
//hUser should have been returned by MS_NETLIB_REGISTERUSER
//This function does the equivalent of socket(), bind(), getsockname(),
//listen(), accept()
//Internally this function creates a new thread which waits around in accept()
//for new connections. When one is received it calls nlb.pfnNewConnection *from
//this new thread* and then loops back to wait again.
//Close the returned handle to end the thread and close the open port.
//Errors: ERROR_INVALID_PARAMETER, any returned by socket() or bind() or
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

typedef void (*NETLIBNEWCONNECTIONPROC_V2)(HANDLE hNewConnection, DWORD dwRemoteIP, void * pExtra);
typedef void (*NETLIBNEWCONNECTIONPROC)(HANDLE hNewConnection, DWORD dwRemoteIP);

typedef struct {
	int cbSize;
	union { // new code should use V2
		NETLIBNEWCONNECTIONPROC pfnNewConnection;
		NETLIBNEWCONNECTIONPROC_V2 pfnNewConnectionV2;
	};
	     //function to call when there's a new connection. Params are: the
		 //new connection, IP of remote machine (host byte order)
	DWORD dwInternalIP;   //set on return, host byte order
	WORD wPort;			  //set on return, host byte order
	void * pExtra;		  //argument is sent to callback, added during 0.3.4+
	DWORD dwExternalIP;   //set on return, host byte order
	WORD wExPort;		  //set on return, host byte order
} NETLIBBIND;
#define MS_NETLIB_BINDPORT     "Netlib/BindPort"

//Open a connection
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(NETLIBOPENCONNECTION*)&nloc
//Returns a HANDLE to the new connection on success, NULL on failure
//hUser must have been returned by MS_NETLIB_REGISTERUSER
//Internally this function is the equivalent of socket(), gethostbyname(),
//connect()
//If NLOCF_HTTP is set and hUser is configured for an HTTP or HTTPS proxy then
//this function will connect() to the proxy server only, without performing any
//initialisation conversation.
//If hUser is configured for an HTTP proxy and does not support HTTP gateways
//and you try to open a connection without specifying NLOCF_HTTP then this
//function will first attempt to open an HTTPS connection, if that fails it
//will try a direct connection, if that fails it will return failure with the
//error from the connect() during the direct connection attempt.
//Errors: ERROR_INVALID_PARAMETER, any returned by socket(), gethostbyname(),
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
#define NLOCF_HTTP          0x0001 //this connection will be used for HTTP communications. If configured for an HTTP/HTTPS proxy the connection is opened as if there was no proxy.
#define NLOCF_STICKYHEADERS 0x0002 //this connection should send the sticky headers associated with NetLib user apart of any HTTP request
#define NLOCF_V2            0x0004 //this connection understands the newer structure, newer cbSize isnt enough
#define NLOCF_UDP           0x0008 // this connection is UDP
#define NLOCF_SSL           0x0010 // this connection is SSL
#define NLOCF_HTTPGATEWAY   0x0020 // this connection is HTTP Gateway

/* Added during 0.4.0+ development!! (2004/11/29) prior to this, connect() blocks til a connection is made or
a hard timeout is reached, this can be anywhere between 30-60 seconds, and it stops Miranda from unloading whilst
this is attempted, clearing sucking - so now you can set a timeout of any value, there is still a hard limit which is
always reached by Windows, If a timeout occurs, or Miranda is exiting then you will get ERROR_TIMEOUT as soon as possible.
*/

struct NETLIBOPENCONNECTION_tag {
	int cbSize;
	const char *szHost;	  //can contain the string representation of an IP
	WORD wPort;			  //host byte order
	DWORD flags;
	unsigned int timeout;
	/* optional, called in the context of the thread that issued the attempt, if it returns 0 the connection attempt is
	stopped, the remaining timeout value can also be adjusted */
	int (*waitcallback) (unsigned int * timeout);
};

#define NETLIBOPENCONNECTION_V1_SIZE offsetof(NETLIBOPENCONNECTION_tag, timeout) /* old sizeof() is 14 bytes, but there is padding of 2 bytes */

//typedef struct NETLIBOPENCONNECTION_tag NETLIBOPENCONNECTION;  //(above for reasons of forward referencing)
#define MS_NETLIB_OPENCONNECTION	"Netlib/OpenConnection"

//Sets the required information for an HTTP proxy connection
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = (LPARAM)(NETLIBHTTPPROXYINFO*)&nlhpi
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This function is designed to be called from within pfnHttpGatewayInit
//See notes below MS_NETLIB_REGISTERUSER.
//Errors: ERROR_INVALID_PARAMETER
#define NLHPIF_USEGETSEQUENCE      0x0001   //append sequence numbers to GET requests
#define NLHPIF_USEPOSTSEQUENCE     0x0002   //append sequence numbers to POST requests
#define NLHPIF_GETPOSTSAMESEQUENCE 0x0004	//GET and POST use the same sequence
#define NLHPIF_HTTP11              0x0008	//HTTP 1.1 proxy
typedef struct {
	int cbSize;
	DWORD flags;
	char *szHttpPostUrl;
	char *szHttpGetUrl;
	int firstGetSequence, firstPostSequence;
	int combinePackets;
} NETLIBHTTPPROXYINFO;
#define MS_NETLIB_SETHTTPPROXYINFO   "Netlib/SetHttpProxyInfo"

//Gets the SOCKET associated with a netlib handle
//wParam = (WPARAM)(HANDLE)hNetlibHandle
//lParam = 0
//Returns the SOCKET on success, INVALID_SOCKET on failure
//hNetlibHandle should have been returned by MS_NETLIB_BINDPORT or
//MS_NETLIB_OPENCONNECTION only.
//Be careful how you use this socket because you might be connected via an
//HTTP proxy in which case calling send() or recv() will totally break things.
//Errors: ERROR_INVALID_PARAMETER
#define MS_NETLIB_GETSOCKET    "Netlib/GetSocket"

#define Netlib_GetBase64DecodedBufferSize(cchEncoded)  (((cchEncoded)>>2)*3)
#define Netlib_GetBase64EncodedBufferSize(cbDecoded)  (((cbDecoded)*4+11)/12*4+1)

// Converts string representation of IP and port into numerical SOCKADDR_INET
// IPv4 could supplied in formats address:port or address
// IPv6 could supplied in formats [address]:port or [address]
// wParam = (WPARAM)(char*) string to convert
// lParam = (LPARAM)(SOCKADDR_INET*) numeric IP address structure
// Returns 0 on success
#define MS_NETLIB_STRINGTOADDRESS "Netlib/StringToAddress"

// Converts numerical representation of IP in SOCKADDR_INET into string representation with IP and port
// IPv4 will be supplied in formats address:port or address
// IPv6 will be supplied in formats [address]:port or [address]
// wParam = (WPARAM)(int)0 - lParam - (sockaddr_gen*); 1 - lParam - (unsigned) in host byte order
// lParam = (LPARAM)(sockaddr_gen*) or (unsigned) numeric IP address structure
// Returns pointer to the string or NULL if not successful
#define MS_NETLIB_ADDRESSTOSTRING  "Netlib/AddressToString"

typedef struct {
	int cbSize;
	char szIpPort[64];
	unsigned dwIpv4;
	WORD wPort;
} NETLIBCONNINFO;

// Get connection Information
// IPv4 will be supplied in formats address:port or address
// IPv6 will be supplied in formats [address]:port or [address]
// wParam = (WPARAM)(HANDLE)hConnection
// lParam = (LPARAM)(NETLIBCONNINFO*) pointer to the connection information structure to fill
// Returns 0 if successful
#define MS_NETLIB_GETCONNECTIONINFO  "Netlib/GetConnectionInfo"

typedef struct {
	unsigned cbNum;
	char szIp[1][64];
} NETLIBIPLIST;

// Get connection Information
// wParam = (WPARAM)IP filter 1 - return global only IPv6 address, 0 all IPs
// Returns (INT_PTR)(NETLIBIPLIST*) numeric IP address address array
// the last element of the array is all 0s, 0 if not successful
#define MS_NETLIB_GETMYIP  "Netlib/GetMyIP"

//Send an HTTP request over a connection
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = (LPARAM)(NETLIBHTTPREQUEST*)&nlhr
//Returns number of bytes sent on success, SOCKET_ERROR on failure
//hConnection must have been returned by MS_NETLIB_OPENCONNECTION
//Note that if you use NLHRF_SMARTAUTHHEADER and NTLM authentication is in use
//then the full NTLM authentication transaction occurs, comprising sending the
//domain, receiving the challenge, then sending the response.
//nlhr.resultCode and nlhr.szResultDescr are ignored by this function.
//Errors: ERROR_INVALID_PARAMETER, anything returned by MS_NETLIB_SEND
typedef struct {
	char *szName;
	char *szValue;
} NETLIBHTTPHEADER;

#define REQUEST_RESPONSE 0	//used by structure returned by MS_NETLIB_RECVHTTPHEADERS
#define REQUEST_GET      1
#define REQUEST_POST     2
#define REQUEST_CONNECT  3
#define REQUEST_HEAD  	 4	// new in 0.5.1
#define REQUEST_PUT      5
#define REQUEST_DELETE   6

#define NLHRF_GENERATEHOST    0x00000001   //auto-generate a "Host" header from szUrl
#define NLHRF_REMOVEHOST      0x00000002   //remove any host and/or protocol portion of szUrl before sending it
#define NLHRF_SMARTREMOVEHOST 0x00000004   //removes host and/or protocol from szUrl unless the connection was opened through an HTTP or HTTPS proxy.
#define NLHRF_SMARTAUTHHEADER 0x00000008   //if the connection was opened through an HTTP or HTTPS proxy then send a Proxy-Authorization header if required.
#define NLHRF_HTTP11          0x00000010   //use HTTP 1.1
#define NLHRF_PERSISTENT      0x00000020   //preserve connection on exit, open connection provided in the nlc field of the reply
                                           //it should be supplied in nlc field of request for reuse or closed if not needed
#define NLHRF_SSL             0x00000040   //use SSL connection
#define NLHRF_NOPROXY         0x00000080   //do not use proxy server
#define NLHRF_REDIRECT        0x00000100   //handle HTTP redirect requests (response 30x), the resulting url provided in szUrl of the response
#define NLHRF_NODUMP          0x00010000   //never dump this to the log
#define NLHRF_NODUMPHEADERS   0x00020000   //don't dump http headers (only useful for POSTs and MS_NETLIB_HTTPTRANSACTION)
#define NLHRF_DUMPPROXY       0x00040000   //this transaction is a proxy communication. For dump filtering only.
#define NLHRF_DUMPASTEXT      0x00080000   //dump posted and reply data as text. Headers are always dumped as text.
#define NLHRF_NODUMPSEND      0x00100000   //do not dump sent message.
struct NETLIBHTTPREQUEST_tag {
	int cbSize;
	int requestType;	//a REQUEST_
	DWORD flags;
	char *szUrl;
	NETLIBHTTPHEADER *headers;	 //If this is a POST request and headers
	     //doesn't contain a Content-Length it'll be added automatically
	int headersCount;
	char *pData;   //data to be sent in POST request.
	int dataLength;		 //must be 0 for REQUEST_GET/REQUEST_CONNECT
	int resultCode;
	char *szResultDescr;
	HANDLE nlc;
	int timeout;
};

#define NETLIBHTTPREQUEST_V1_SIZE (offsetof(NETLIBHTTPREQUEST_tag, timeout))
//typedef struct NETLIBHTTPREQUEST_tag NETLIBHTTPREQUEST;  //(above for reasons of forward referencing)
#define MS_NETLIB_SENDHTTPREQUEST   "Netlib/SendHttpRequest"

//Receive HTTP headers
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = 0
//Returns a pointer to a NETLIBHTTPREQUEST structure on success, NULL on
//failure.
//Call MS_NETLIB_FREEHTTPREQUESTSTRUCT to free this.
//hConnection must have been returned by MS_NETLIB_OPENCONNECTION
//nlhr->pData = NULL and nlhr->dataLength = 0 always. The requested data should
//be retrieved using MS_NETLIB_RECV once the header has been parsed.
//If the headers haven't finished within 60 seconds the function returns NULL
//and ERROR_TIMEOUT.
//Errors: ERROR_INVALID_PARAMETER, any from MS_NETLIB_RECV or select()
//    ERROR_HANDLE_EOF (connection closed before headers complete)
//    ERROR_TIMEOUT (headers still not complete after 60 seconds)
//    ERROR_BAD_FORMAT (invalid character or line ending in headers, or first line is blank)
//    ERROR_BUFFER_OVERFLOW (each header line must be less than 4096 chars long)
//    ERROR_INVALID_DATA (first header line is malformed ("http/[01].[0-9] [0-9]+ .*", or no colon in subsequent line)
#define MS_NETLIB_RECVHTTPHEADERS  "Netlib/RecvHttpHeaders"

//Free the memory used by a NETLIBHTTPREQUEST structure
//wParam = 0
//lParam = (LPARAM)(NETLIBHTTPREQUEST*)pnlhr
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//This should only be called on structures returned by
//MS_NETLIB_RECVHTTPHEADERS or MS_NETLIB_HTTPTRANSACTION. Calling it on an
//arbitrary structure will have disastrous results.
//Errors: ERROR_INVALID_PARAMETER
#define MS_NETLIB_FREEHTTPREQUESTSTRUCT  "Netlib/FreeHttpRequestStruct"

//Do an entire HTTP transaction
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(NETLIBHTTPREQUEST*)&nlhr
//Returns a pointer to another NETLIBHTTPREQUEST structure on success, NULL on
//failure.
//Call MS_NETLIB_FREEHTTPREQUESTSTRUCT to free this.
//hUser must have been returned by MS_NETLIB_REGISTERUSER
//nlhr.szUrl should be a full HTTP URL. If it does not start with http://, that
//will be assumed (but it's best not to use this fact, for reasons of
//extensibility).
//This function is the equivalent of MS_NETLIB_OPENCONNECTION,
//MS_NETLIB_SENDHTTPREQ, MS_NETLIB_RECVHTTPHEADERS, MS_NETLIB_RECV,
//MS_NETLIB_CLOSEHANDLE
//nlhr.headers will be augmented with the following headers unless they have
//already been set by the caller:
//  "Host" (regardless of whether it is requested in nlhr.flags)
//  "User-Agent"  (of the form "Miranda/0.1.2.2 (alpha)" or "Miranda/0.1.2.2")
//  "Content-Length" (for POSTs only. Set to nlhr.dataLength)
//If you do not want to send one of these headers, create a nlhr.headers with
//szValue = NULL.
//In the return value headers, headerCount, pData, dataLength, resultCode and
//szResultDescr are all valid.
//In the return value pData[dataLength] == 0 always, as an extra safeguard
//against programming slips.
//Note that the function can succeed (ie not return NULL) yet result in an HTTP
//error code. You should check that resultCode == 2xx before proceeding.
//Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY, anything from the above
//    list of functions
#define MS_NETLIB_HTTPTRANSACTION   "Netlib/HttpTransaction"

//Send data over a connection
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = (LPARAM)(NETLIBBUFFER*)&nlb
//Returns the number of bytes sent on success, SOCKET_ERROR on failure
//Errors: ERROR_INVALID_PARAMETER
//        anything from send(), nlu.pfnHttpGatewayWrapSend()
//        HTTP proxy: ERROR_GEN_FAILURE (http result code wasn't 2xx)
//                    anything from socket(), connect(),
//                    MS_NETLIB_SENDHTTPREQUEST, MS_NETLIB_RECVHTTPHEADERS
//flags:
#define MSG_NOHTTPGATEWAYWRAP  0x010000	 //don't wrap the outgoing packet using nlu.pfnHttpGatewayWrapSend
#define MSG_NODUMP             0x020000  //don't dump this packet to the log
#define MSG_DUMPPROXY          0x040000	 //this is proxy communiciation. For dump filtering only.
#define MSG_DUMPASTEXT         0x080000  //this is textual data, don't dump as hex
#define MSG_RAW                0x100000	 //send as raw data, bypass any HTTP proxy stuff
#define MSG_DUMPSSL            0x200000	 //this is SSL traffic. For dump filtering only.
typedef struct {
	char *buf;
	int len;
	int flags;
} NETLIBBUFFER;
#define MS_NETLIB_SEND	   "Netlib/Send"
static __inline INT_PTR Netlib_Send(HANDLE hConn, const char *buf, int len, int flags) {
	NETLIBBUFFER nlb = {(char*)buf, len, flags};
	return CallService(MS_NETLIB_SEND, (WPARAM)hConn, (LPARAM)&nlb);
}

//Receive data over a connection
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = (LPARAM)(NETLIBBUFFER*)&nlb
//Returns the number of bytes read on success, SOCKET_ERROR on failure,
//0 if the connection has been closed
//Flags supported: MSG_PEEK, MSG_NODUMP, MSG_DUMPPROXY, MSG_NOHTTPGATEWAYWRAP,
//                 MSG_DUMPASTEXT, MSG_RAW
//On using MSG_NOHTTPGATEWAYWRAP: Because packets through an HTTP proxy are
//  batched and cached and stuff, using this flag is not a guarantee that it
//  will be obeyed, and if it is it may even be propogated to future calls
//  even if you don't specify it then. Because of this, the flag should be
//  considered an all-or-nothing thing: either use it for the entire duration
//  of a connection, or not at all.
//Errors: ERROR_INVALID_PARAMETER, anything from recv()
//        HTTP proxy: ERROR_GEN_FAILURE (http result code wasn't 2xx)
//					  ERROR_INVALID_DATA (no Content-Length header in reply)
//                    ERROR_NOT_ENOUGH_MEMORY (Content-Length very large)
//                    ERROR_HANDLE_EOF (connection closed before Content-Length bytes recved)
//                    anything from select(), MS_NETLIB_RECVHTTPHEADERS,
//						  nlu.pfnHttpGatewayUnwrapRecv, socket(), connect(),
//						  MS_NETLIB_SENDHTTPREQUEST
#define MS_NETLIB_RECV	   "Netlib/Recv"
static __inline INT_PTR Netlib_Recv(HANDLE hConn, char *buf, int len, int flags) {
	NETLIBBUFFER nlb = {buf, len, flags};
	return CallService(MS_NETLIB_RECV, (WPARAM)hConn, (LPARAM)&nlb);
}

//Determine the status of one or more connections
//wParam = 0
//lParam = (LPARAM)(NETLIBSELECT*)&nls
//Returns the number of ready connections, SOCKET_ERROR on failure,
//0 if the timeout expired.
//All handles passed to this function must have been returned by either
//MS_NETLIB_OPENCONNECTION or MS_NETLIB_BINDPORT.
//The last handle in each list must be followed by either NULL or
//INVALID_HANDLE_VALUE.
//Errors: ERROR_INVALID_HANDLE, ERROR_INVALID_DATA, anything from select()
typedef struct {
	int cbSize;
	DWORD dwTimeout;      //in milliseconds, INFINITE is acceptable
	HANDLE hReadConns[FD_SETSIZE+1];
	HANDLE hWriteConns[FD_SETSIZE+1];
	HANDLE hExceptConns[FD_SETSIZE+1];
} NETLIBSELECT;

typedef struct {
	int cbSize;
	DWORD dwTimeout;      //in milliseconds, INFINITE is acceptable
	HANDLE hReadConns[FD_SETSIZE+1];
	HANDLE hWriteConns[FD_SETSIZE+1];
	HANDLE hExceptConns[FD_SETSIZE+1];
	/* Added in v0.3.3+ */
	BOOL hReadStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
	BOOL hWriteStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
	BOOL hExceptStatus[FD_SETSIZE+1]; /* out, [in, expected to be FALSE] */
} NETLIBSELECTEX;

#define MS_NETLIB_SELECT	   "Netlib/Select"
// added in v0.3.3
#define MS_NETLIB_SELECTEX	   "Netlib/SelectEx"

//Shutdown connection
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = 0
//Returns 0
#define MS_NETLIB_SHUTDOWN	   "Netlib/Shutdown"
__forceinline void Netlib_Shutdown(HANDLE h) {CallService(MS_NETLIB_SHUTDOWN, (WPARAM)h, 0);}

//Create a packet receiver
//wParam = (WPARAM)(HANDLE)hConnection
//lParam = (LPARAM)(int)maxPacketSize
//Returns a HANDLE on success, NULL on failure
//The packet receiver implements the common situation where you have variable
//length packets coming in over a connection and you want to split them up
//in order to handle them.
//The major limitation is that the buffer is created in memory, so you can't
//have arbitrarily large packets.
//Errors: ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY
#define MS_NETLIB_CREATEPACKETRECVER     "Netlib/CreatePacketRecver"

//Get the next set of packets from a packet receiver
//wParam = (WPARAM)(HANDLE)hPacketRecver
//lParam = (LPARAM)(NETLIBPACKETRECVER*)&nlpr
//Returns the total number of bytes available in the buffer, 0 if the
//connection was closed, SOCKET_ERROR on error.
//hPacketRecver must have been returned by MS_NETLIB_CREATEPACKETRECVER
//If nlpr.bytesUsed is set to zero and the buffer is already full up to
//maxPacketSize, it is assumed that too large a packet has been received. All
//data in the buffer is discarded and receiving is begun anew. This will
//probably cause alignment problems so if you think this is likely to happen
//then you should deal with it yourself.
//Closing the packet receiver will not close the associated connection, but
//will discard any bytes still in the buffer, so if you intend to carry on
//reading from that connection, make sure you have processed the buffer first.
//This function is the equivalent of a memmove() to remove the first bytesUsed
//from the buffer, select() if dwTimeout is not INFINITE, then MS_NETLIB_RECV.
//Errors: ERROR_INVALID_PARAMETER, ERROR_TIMEOUT,
//        anything from select(), MS_NETLIB_RECV
typedef struct {
	int cbSize;
	DWORD dwTimeout;	  //fill before calling. In milliseconds. INFINITE is valid
	int bytesUsed;		  //fill before calling. This many bytes are removed from the start of the buffer. Set to 0 on return
	int bytesAvailable;	  //equal to the return value, unless the return value is 0
	int bufferSize;		  //same as parameter to MS_NETLIB_CREATEPACKETRECVER
	BYTE *buffer;		  //contains the recved data
} NETLIBPACKETRECVER;
#define MS_NETLIB_GETMOREPACKETS    "Netlib/GetMorePackets"

//Add a message to the log (if it's running)
//wParam = (WPARAM)(HANDLE)hUser
//lParam = (LPARAM)(const char *)szMessage
//Returns nonzero on success, 0 on failure	(!! this is different to most of the rest of Miranda, but consistent with netlib)
//Do not include a final line ending in szMessage.
//Errors: ERROR_INVALID_PARAMETER
#define MS_NETLIB_LOG       "Netlib/Log"
#define MS_NETLIB_LOGW      "Netlib/LogW"

//Sets a gateway polling timeout interval
//wParam = (WPARAM)(HANDLE)hConn
//lParam = (LPARAM)timeout
//Returns previous timeout value
//Errors: -1
#define MS_NETLIB_SETPOLLINGTIMEOUT "Netlib/SetPollingTimeout"

//Makes connection SSL
//wParam = (WPARAM)(HANDLE)hConn
//lParam = (LPARAM)(NETLIBSSL*)&nlssl or null if no certficate validation required
//Returns 0 on failure 1 on success
#define MS_NETLIB_STARTSSL "Netlib/StartSsl"

typedef struct
{
	int cbSize;
	const char *host; //Expected host name
	int flags;        //Reserved
} NETLIBSSL;


//here's a handy piece of code to let you log using printf-style specifiers:
//#include <stdarg.h> and <stdio.h> before including this header in order to
//use it.
#if defined va_start && (defined _STDIO_DEFINED || defined _STDIO_H_) && (!defined NETLIB_NOLOGGING)
#pragma warning(disable:4505)

static INT_PTR Netlib_Logf(HANDLE hUser, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char szText[1024];
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	return CallService(MS_NETLIB_LOG, (WPARAM)hUser, (LPARAM)szText);
}

static INT_PTR Netlib_LogfW(HANDLE hUser, const wchar_t *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	wchar_t szText[1024];
	mir_vsnwprintf(szText, SIZEOF(szText), fmt, va);
	va_end(va);
	return CallService(MS_NETLIB_LOGW, (WPARAM)hUser, (LPARAM)szText);
}

#define Netlib_LogfT Netlib_LogfW
#endif //defined va_start

/////////////////////////////////////////////////////////////////////////////////////////
// Security providers (0.6+)

#define NNR_UNICODE 1

#ifdef UNICODE
	#define NNR_TCHAR NNR_UNICODE
#else
	#define NNR_TCHAR 0
#endif

// Inits a required security provider. Right now only NTLM is supported
// Returns HANDLE = NULL on error or non-null value on success
// Known providers: Basic, NTLM, Negotiate, Kerberos, GSSAPI - (Kerberos SASL)
#define MS_NETLIB_INITSECURITYPROVIDER "Netlib/InitSecurityProvider"

static __inline HANDLE Netlib_InitSecurityProvider(char* szProviderName)
{
	return (HANDLE)CallService(MS_NETLIB_INITSECURITYPROVIDER, 0, (LPARAM)szProviderName);
}

typedef struct {
	size_t cbSize;
	const TCHAR* szProviderName;
	const TCHAR* szPrincipal;
	unsigned flags;
}
	NETLIBNTLMINIT2;

#define MS_NETLIB_INITSECURITYPROVIDER2 "Netlib/InitSecurityProvider2"

static __inline HANDLE Netlib_InitSecurityProvider2(const TCHAR* szProviderName, const TCHAR* szPrincipal)
{
	NETLIBNTLMINIT2 temp = { sizeof(temp), szProviderName, szPrincipal, NNR_TCHAR };
	return (HANDLE)CallService(MS_NETLIB_INITSECURITYPROVIDER2, 0, (LPARAM)&temp);
}


// Destroys a security provider's handle, provided by Netlib_InitSecurityProvider.
// Right now only NTLM is supported
#define MS_NETLIB_DESTROYSECURITYPROVIDER "Netlib/DestroySecurityProvider"

static __inline void Netlib_DestroySecurityProvider(char* szProviderName, HANDLE hProvider)
{
	CallService(MS_NETLIB_DESTROYSECURITYPROVIDER, (WPARAM)szProviderName, (LPARAM)hProvider);
}

// Returns the NTLM response string. The result value should be freed using mir_free

typedef struct {
	char* szChallenge;
	char* userName;
	char* password;
}
	NETLIBNTLMREQUEST;

#define MS_NETLIB_NTLMCREATERESPONSE "Netlib/NtlmCreateResponse"

static __inline char* Netlib_NtlmCreateResponse(HANDLE hProvider, char* szChallenge, char* login, char* psw)
{
	NETLIBNTLMREQUEST temp = { szChallenge, login, psw };
	return (char*)CallService(MS_NETLIB_NTLMCREATERESPONSE, (WPARAM)hProvider, (LPARAM)&temp);
}

typedef struct {
	size_t cbSize;
	const char* szChallenge;
	const TCHAR* szUserName;
	const TCHAR* szPassword;
	unsigned complete;
	unsigned flags;
}
	NETLIBNTLMREQUEST2;

#define MS_NETLIB_NTLMCREATERESPONSE2 "Netlib/NtlmCreateResponse2"

static __inline char* Netlib_NtlmCreateResponse2(HANDLE hProvider, char* szChallenge, TCHAR* szLogin, TCHAR* szPass, unsigned *complete)
{
	NETLIBNTLMREQUEST2 temp = { sizeof(temp), szChallenge, szLogin, szPass, *complete, NNR_TCHAR };
	char* res = (char*)CallService(MS_NETLIB_NTLMCREATERESPONSE2, (WPARAM)hProvider, (LPARAM)&temp);
	*complete = temp.complete;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Netlib hooks (0.8+)

// WARNING: these hooks are being called in the context of the calling thread, without switching
// to the first thread, like all another events do. The hook procedure should be ready for the
// multithreaded mode
//
// Parameters:
//    wParam: NETLIBNOTIFY* - points to the data being sent/received
//    lParam: NETLIBUSER*   - points to the protocol definition

typedef struct {
	NETLIBBUFFER* nlb;      // pointer to the request buffer
	int           result;   // amount of bytes really sent/received
}
	NETLIBNOTIFY;

#define ME_NETLIB_FASTRECV "Netlib/OnRecv"  // being called on every receive
#define ME_NETLIB_FASTSEND "Netlib/OnSend"  // being called on every send
#define ME_NETLIB_FASTDUMP "Netlib/OnDump"  // being called on every dump

#endif // M_NETLIB_H__
