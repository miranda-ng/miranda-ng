{$IFNDEF M_NETLIB}
{$DEFINE M_NETLIB}

{>>/

    NetLib :

    Instead of you writing all the code for working with sockets and supporting
    app level protocols such as SOCKS5, it's all done for you.

    NetLib takes care of all that and you can even register a special abstract
    nexus, e.g. ICQ direct, the user can configure all this from the options dialog
    and you don't have to bother with any of it.

    NetLib wraps up any Winsock calls but you can still get the socket handle
    from your netlib handle and do stuff.

    It gives all modules an abstract way of dealing with transport -- mainly sockets
    and proxies, Now the but..

    It's new (mmmm) thus unsupported by any older version of Miranda, and if you
    want to be lazy and not write any "wrapper" mini netlib then you'll have
    the kudos of "only works with nightly build version of Miranda" :)

/<<}

const
  NETLIB_USER_AGENT = 'Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)';

const
  // for TNETLIBUSER.flags
  NUF_INCOMING      = $01; // bind incoming ports
  NUF_OUTGOING      = $02; // makes outgoing plain connections
  NUF_HTTPGATEWAY   = $04; // can use HTTP gateway for plain sockets.
                           // ???HttpGateway* are valid, enables the HTTP
                           // proxy option, displayed in options
  NUF_NOOPTIONS     = $08; // don't show this as an entry for custom settings to
                           // be defined for, TNETLIB.szDescriptiveName is ignored
  NUF_HTTPCONNS     = $10; // some connections are made for HTTP communication,
                           // enables the HTTP proxy option, displayed in options
  NUF_NOHTTPSOPTION = $20; // Disables the HTTPS proxy option in options, Use
                           // this if all communication is HTTP
  NUF_UNICODE       = $40; // if set ptszDescriptiveName points to Unicode, otherwise ANSI

  // for TNETLIBUSERSETTINGS.proxyType

  { SOCKS4 -- No DNS or multi addressing mode (proxy side) -- optional username can
  be given, no password }
  PROXYTYPE_SOCKS4 = 1;
  { SOCKS5 -- DNS names can be given as addresses to connect to, optional
  plain text username/password scheme (which may cause failure due to denied access)
  IP address maybe returned for DNS addresses -- thus server side DNS }
  PROXYTYPE_SOCKS5 = 2;
  PROXYTYPE_HTTP   = 3;
  PROXYTYPE_HTTPS  = 4;
  PROXYTYPE_IE     = 5;

  // for TNETLIBOPENCONNECTION.flags

  { this connection will be useed for HTTP communications,
  if configured for an HTTP(S) proxy the connection is opened as if there
  was no proxy }
  NLOCF_HTTP          = $0001;
  { this connection should send the sticky headers associated with NetLib
   user apart of any HTTP request}
  NLOCF_STICKYHEADERS = $0002;
  { this connection understands the newer structure, newer cbSize isnt enough}
  NLOCF_V2            = $0004;
  NLOCF_UDP           = $0008; // this connection is UDP
  NLOCF_SSL           = $0010; // this connection is SSL
  NLOCF_HTTPGATEWAY   = $0020; // this connection is HTTP Gateway

  // for TNETLIBHTTPPROXYINFO.flags

  NLHPIF_USEGETSEQUENCE      = $0001; // append sequence numbers to GET requests
  NLHPIF_USEPOSTSEQUENCE     = $0002; // append sequence numbers to POST requests
  NLHPIF_GETPOSTSAMESEQUENCE = $0004; // GET and POST use the same sequence
  NLHPIF_HTTP11              = $0008;  // HTTP 1.1 proxy

  // for TNETLIBHTTPREQUEST.flags, .requestType

  { used by MS_NETLIB_RECVHTTPHEADERS returned structure }

  REQUEST_RESPONSE = 0;
  REQUEST_GET      = 1;
  REQUEST_POST     = 2;
  REQUEST_CONNECT  = 3;
  REQUEST_HEAD     = 4; // new in 0.5.1
  REQUEST_PUT      = 5;
  REQUEST_DELETE   = 6;

  NLHRF_GENERATEHOST    = $00000001; // auto generate a 'host' header from .szUrl
  NLHRF_REMOVEHOST      = $00000002; // remove any host and/or protocol portion
                                     // of szUrl before sending it
  NLHRF_SMARTREMOVEHOST = $00000004; // removes host and/or protocol from szUrl
                                     // unless the connection was opened through
                                     // an HTTP or HTTPS proxy.
  NLHRF_SMARTAUTHHEADER = $00000008; // if the connection was opened through an
                                     // HTTP or HTTPS proxy then send a
                                     // Proxy-Authorization header if required.
  NLHRF_HTTP11          = $00000010; // use HTTP 1.1
  NLHRF_PERSISTENT      = $00000020; // preserve connection on exit, open connection provided
                                     // in the nlc field of the reply it should be supplied in
                                     // nlc field of request for reuse or closed if not needed
  NLHRF_SSL             = $00000040; // use ssl connection
  NLHRF_NOPROXY         = $00000080; // do not use proxy server
  NLHRF_REDIRECT        = $00000100; // handle HTTP redirect requests (response 30x), the
                                     // resulting url provided in szUrl of the response
  NLHRF_NODUMP          = $00010000; // never dump this to the log
  NLHRF_NODUMPHEADERS   = $00020000; // don't dump http headers (only useful for
                                     // POSTs and MS_NETLIB_HTTPTRANSACTION
  NLHRF_DUMPPROXY       = $00040000; // this transaction is a proxy communication.
                                     // For dump filtering only.
  NLHRF_DUMPASTEXT      = $00080000; // dump posted and reply data as text.
                                     // Headers are always dumped as text.
  NLHRF_NODUMPSEND      = $00100000; // do not dump sent message.

  // for TNETLIBBUFFER.flags

  MSG_NOHTTPGATEWAYWRAP = $010000; // don't wrap outgoing packet using
                                   // TNETLIBUSER.pfnHttpGatewayWrapSend
  MSG_NODUMP            = $020000; // don't dump this packet to the log
  MSG_DUMPPROXY         = $040000; // this is proxy communication, for dump
                                   // filtering only
  MSG_DUMPASTEXT        = $080000; // don't dump as hex, it's text
  MSG_RAW               = $100000; // send as raw, bybpass HTTP proxy stuff
  MSG_DUMPSSL           = $200000; // this is SSL traffic. For dump filtering only.


// all record types structures are declared in their own block because the C
// header file used forward declaration (to get typed parameters for certain
// function pointers). This sort of define-type-pointer-before-type can only be
// done in the same type block in D2 (don't know about later versions)

  FD_SETSIZE = 65;

type
  twaitcallback = function(timeout:dword):integer; cdecl;

  PNETLIBOPENCONNECTION = ^TNETLIBOPENCONNECTION;
  TNETLIBOPENCONNECTION = record
    cbSize : int;
    szHost : PAnsiChar; // can be an IP in string form
    wPort  : word;
    flags  : dword; // see NLOCF_* flags
    timeout: uint;
    // optional, called in the context of the thread that issued the attempt,
    // if it returns 0 the connection attempt is stopped, the remaining
    // timeout value can also be adjusted
    waitcallback:twaitcallback;
  end;

const
  {$IFNDEF WIN64}
  NETLIBOPENCONNECTION_V1_SIZE = 16;
  {$ELSE}
  NETLIBOPENCONNECTION_V1_SIZE = SIZEOF(TNETLIBOPENCONNECTION);
  {$ENDIF}
  // old sizeof() is 14 bytes, but there is padding of 2 bytes

type
  PNETLIBHTTPHEADER = ^TNETLIBHTTPHEADER;
  TNETLIBHTTPHEADER = record
    szName : PAnsiChar;
    szValue: PAnsiChar;
  end;
  ANETLIBHTTPHEADER = array [0..1000] of TNETLIBHTTPHEADER;

  PNETLIBHTTPREQUEST    = ^TNETLIBHTTPREQUEST;

  TNETLIBHTTPREQUEST = record
    cbSize       :int;
    requestType  :int;                // REQUEST_* constant
    flags        :dword;
    szUrl        :PAnsiChar;
    { doesn't contain Content-Length, it'll be added automatically }
    headers      :^ANETLIBHTTPHEADER; // pointer to an array of em?
    headersCount :int;                // yes they do
    pData        :PAnsiChar;          // data to be sent on POST request
    dataLength   :int;                // must be 0 for REQUEST_GET/REQUEST_CONNECT
    resultCode   :int;
    szResultDescr:PAnsiChar;
    nlc          :THANDLE;
    timeout      :int;                // Mirver 9.0+
  end;

const
  {$IFNDEF WIN64}
  NETLIBHTTPREQUEST_V1_SIZE = 44;
  {$ELSE}
  NETLIBHTTPREQUEST_V1_SIZE = SIZEOF(TNETLIBHTTPREQUEST);
  {$ENDIF}

type
  { This function pointer is to the CRT realloc() used by Miranda -- it allows reallocation of memory passed
  to us (not that we could EVER share the same CRT) but to allow DLLs in general to reallocate memory }
  TNetlibRealloc = function(Mem: pointer; size_t: int): pointer; cdecl;
  TNetlibHTTPGatewayInitProc  = function(hConn: THANDLE; nloc: PNETLIBOPENCONNECTION; nlhr: PNETLIBHTTPREQUEST): int; cdecl;
  TNetlibHTTPGatewayBeginProc = function(hConn: THANDLE; nloc: PNETLIBOPENCONNECTION): int; cdecl;
  TNetlibHTTPGatewayWrapSendProc = function(hConn: THANDLE; buf: PByte; len: int; flags: int; pfnNetLibSend: TMIRANDASERVICE): int; cdecl;
  TNetlibHTTPGatewayUnwrapRecvProc = function(nlhr: PNETLIBHTTPREQUEST; buf: PByte; len: int; outBufLen: pInt; NetlibRealloc: TNetlibRealloc): PByte; cdecl;

  PNETLIBUSER = ^TNETLIBUSER;
  TNETLIBUSER = record
    cbSize: int;
    szSettingsModule: PAnsiChar;  // used for DB settings and log, 'NL' stuff
    szDescriptiveName: TChar; // shows a descriptive name for which different proxy settings can be defined
    flags: dword;             // see NUF_* constants above
    szHttpGatewayHello    :PAnsiChar;
    szHttpGatewayUserAgent:PAnsiChar; // can be NULL(0) to send no User-Agent: also used by HTTPS proxies
    pfnHttpGatewayInit      :TNetlibHTTPGatewayInitProc;
    pfnHttpGatewayBegin     :TNetlibHTTPGatewayBeginProc; // can be NULL(0) if no begin is required
    pfnHttpGatewayWrapSend  :TNetlibHTTPGatewayWrapSendProc; // can be NULL(0) if no wrapping is required
    pfnHttpGatewayUnwrapRecv:TNetlibHTTPGatewayUnwrapRecvProc; // can be NULL(0) "      "
    minIncomingPorts: int; // only if NUF_INCOMING, will be used for validation of user input
  end;

  PNETLIBUSERSETTINGS = ^TNETLIBUSERSETTINGS;
  TNETLIBUSERSETTINGS = record
    cbSize              :int;   // filled before calling
    useProxy            :int;   // 1 or 0
    proxyType           :int;   // PROXYTYPE_* constant, see above
    szProxyServer       :PAnsiChar; // can be NULL(0)
    wProxyPort          :int;   // in host byte order
    useProxyAuth        :int;   // 1 or 0, always 0 for SOCKS4 (doesn't have auth)
    szProxyAuthUser     :PAnsiChar; // can be NULL(0), always used by SOCKS4
    szProxyAuthPassword :PAnsiChar; // can be NULL(0)
    useProxyAuthNtlm    :int;   // 1 or 0, only used by HTTP, HTTPS
    dnsThroughProxy     :int;   // 1 or 0
    specifyIncomingPorts:int;   // 1 or 0
    szIncomingPorts     :PAnsiChar; // can be NULL(0), form '1024-1050,1060-1070,2000'
    specifyOutgoingPorts:int;   // 0.3.3a+
    szOutgoingPorts     :PAnsiChar; // 0.3.3a+
    enableUPnP          :int;   // 0.6.1+ only for NUF_INCOMING
    validateSSL         :int;
  end;

  TNetlibNewConnectionProc    = procedure(hNewConnection:THANDLE; dwRemoveIP:dword); cdecl;
  TNetlibNewConnectionProc_V2 = procedure(hNewConnection:THANDLE; dwRemoveIP:dword; pExtra:pointer); cdecl;

  PNETLIBBIND = ^TNETLIBBIND;
  TNETLIBBIND = record
    cbSize:int;
    // new code should use V2
    pfnNewConnection:TNETLIBNEWCONNECTIONPROC;
    // or pfnNewConnectionV2:NETLIBNEWCONNECTIONPROC_V2;
    //function to call when there's a new connection. Params are: the
    //new connection, IP of remote machine (host byte order)
    dwInternalIP:dword;    //set on return, host byte order
    wPort       :word;     //set on return, host byte order
    pExtra      :^pointer; //argument is sent to callback, added during 0.3.4+
    dwExternalIP:dword;    //set on return, host byte order
    wExPort     :word;     //set on return, host byte order
  end;

type
  PNETLIBHTTPPROXYINFO = ^TNETLIBHTTPPROXYINFO;
  TNETLIBHTTPPROXYINFO = record
    cbSize           : int;
    flags            : dword; // see NLHPIF_* above
    szHttpPostUrl    : PAnsiChar;
    szHttpGetUrl     : PAnsiChar;
    firstGetSequence : int;
    firstPostSequence: int;
    combinePackets   : int; // MIRANDA_VER >= 0x0900
  end;

type
  PNETLIBBUFFER = ^TNETLIBBUFFER;
  TNETLIBBUFFER = record
    buf  : PAnsiChar;
    len  : int;
    flags: int; // see MSG_* constants above
  end;

  PNETLIBSELECT = ^TNETLIBSELECT;
  TNETLIBSELECT = record
    cbSize      : int;
    dwTimeout   : dword; // in milliseconds, INFINITE is acceptable
    hReadConns  : array [0..FD_SETSIZE] of THANDLE;
    hWriteConns : array [0..FD_SETSIZE] of THANDLE;
    hExceptConns: array [0..FD_SETSIZE] of THANDLE;
  end;

  PNETLIBSELECTEX = ^TNETLIBSELECTEX;
  TNETLIBSELECTEX = record
    cbSize      : int;
    dwTimeout   : dword; // in milliseconds, INFINITE is acceptable
    hReadConns  : array [0..FD_SETSIZE] of THANDLE;
    hWriteConns : array [0..FD_SETSIZE] of THANDLE;
    hExceptConns: array [0..FD_SETSIZE] of THANDLE;
    // Added in v0.3.3+
    hReadStatus  : array [0..FD_SETSIZE] of bool; // out, [in, expected to be FALSE]
    hWriteStatus : array [0..FD_SETSIZE] of bool; // out, [in, expected to be FALSE]
    hExceptStatus: array [0..FD_SETSIZE] of bool; // out, [in, expected to be FALSE]
  end;

  PNETLIBPACKETRECVER = ^TNETLIBPACKETRECVER;
  TNETLIBPACKETRECVER = record
    cbSize        : int;
    dwTimeout     : dword; // infinite is allowed -- initialise before use
    bytesUsed     : int;   // this many bytes are removed from the start of the
                           // buffer, set to 0 on return -- initialise before use
    bytesAvailable: int;   // equal the returnd value by service, unless the
                           // return value is 0 (connection closed)
    bufferSize    : int;   // same as the parameter given to
                           // MS_NETLIB_CREATEPACKETRECVER: wParam
    buffer        : PByte; // contains the read data
  end;

const
  {
    wParam : 0
    lParam : Pointer to an initalised TNETLIBUSER structure
    Affects: Initialises the netlib for a set of connections, see notes
    Returns: Returns a handle for future netlib calls, NULL on failure.
    Notes  : Netlib is loaded AFTER all plugins, thus a call to this service
             in Load() will fail, hook ME_SYSTEM_MODULESLOADED and call it
             from there.
             -
             Netlib will save settings under .szSettings module, all settings
             (being?) begin with 'NL'.
             -
             Defacto settings are the same as <All connections> combobox entry option
             as seen in Miranda->Options->Network
    Version: v0.1.2.2+
    Errors : ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY, ERROR_DUP_NAME
  }
  MS_NETLIB_REGISTERUSER:PAnsiChar = 'Netlib/RegisterUser';

{
  Assign a Netlib user handle a set of dynamic HTTP headers to be used with all

  HTTP connections that enable the HTTP-use-sticky headers flag.
  The headers persist until cleared with lParam=NULL.

  All memory should be allocated by the caller using malloc() from MS_SYSTEM_GET_MMI
  Once it has passed to Netlib, Netlib is the owner of it, the caller should not refer to the memory
  In any way after this point.

  wParam=(WPARAM)hNetLibUser
  lParam=(LPARAM)(AnsiChar*)szHeaders

  NOTE: The szHeaders parameter should be a NULL terminated string following the HTTP header syntax.
  This string will be injected verbatim, thus the user should be aware of setting strings that are not
  headers. This service is NOT THREAD SAFE, only a single thread is expected to set the headers and a single
  thread reading the pointer internally, stopping race conditions and mutual exclusion don't happen.

  Version 0.3.2a+ (2003/10/27)
}
  MS_NETLIB_SETSTICKYHEADERS:PAnsiChar = 'Netlib/SetStickyHeaders';

  {
    wParam : HANDLE
    lParam : Pointer to a initalised TNETLIBUSERSETTINGS structure
    Affects: Gets the user configured settings for a Netlib user, see notes
    Returns: [non zero] on SUCCESS, NULL(0) on failure
    Notes  : .cbSize must be filled with sizeof() before calling --
             the returned null terminated strings (in the structure) are valid
             as long as HANDLE remains open or proxy options are changed
             again, do not rely on them being around forever.
    Version: v0.1.2.2+
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_GETUSERSETTINGS:PAnsiChar = 'Netlib/GetUserSettings';

  {
    wParam : HANDLE
    lParam : Pointer to a initalised NETLIBUSERSETTINGS structure
    Affect : Changes the configurable settings for a Netlib user -- see notes
    Returns: [non zero] on success, NULL(0) on failure
    Notes  : This service is only really useful for people that specify NUF_NOOPTIONS
             when registering and want to create their own options.
             Settings will be stored even if the option to enable it, is it not enabled,
             e.g. useProxyAuth is 0, szProxyAuthPassword will still be saved
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_SETUSERSETTINGS:PAnsiChar = 'Netlib/SetUserSettings';

  {
    wParam : HANDLE / SOCKET
    lParam : 0
    Affects: Closes a handle, see notes
    Returns: Returns [non zero] on success, NULL(0) on failure
    Notes  : All netlib handles should be closed once they're finished with,
             If a SOCKET type is passed instead of netlib handle type, it is closed
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_CLOSEHANDLE:PAnsiChar = 'Netlib/CloseHandle';

  {
    wParam : HANDLE
    lParam : Pointer to a initialised TNETLIBBIND
    Affects: Open a port and wait for connections on it -- see notes
    Returns: Returns a handle on success, NULL(0) on failure
    Notes  : this function does the equivalent of socket(), bind(), getsockname(),
             listen(), accept() -- internally this function creates a new thread
             which waits around in accept() for new connections.
             When one is received, TNETLIBBIND.pfnNewConnection is called,
             from the context of the NEW thread and then it
             returns to waiting for connections.
             -
             Close the returned handle to end the thread and close the port.
             -
    Errors : ERROR_INVALID_PARAMETER, any returned by socket(), bind(), listen()
             getsockname()
  }
  MS_NETLIB_BINDPORT:PAnsiChar = 'Netlib/BindPort';

  {
    wParam : HANDLE
    lParam : Pointer to an initalised TNETLIBOPENCONNECTION structure
    Affects: Opens a connection -- see notes
    Returns: Returns a Handle to a new connection on success, NULL(0) on failure
    Notes  : internally this service is the equivalent of socket(), gethostbyname(),
             connect()
             -
             If NLOCF_HTTP is set and HANDLE is configured for HTTP(S) proxy
             then this function will connect() to that proxy server ONLY,
             without performing any initialisation conversation.
             -
             If HANDLE is configured for an HTTP proxy and does not support
             HTTP gateways and you try to open a connection without NLOCF_HTTP
             then this service will first attempt to open an HTTPS connection,
             if that fails, it will try a direct connection, if *that* fails
             then it will return failure with the error
             from connect() during the connection attempt
    Errors : ERROR_INVALID_PARAMETER, any returned by socket(), gethostbyname(),
             connect(), MS_NETLIB_SEND, MS_NETLIB_RECV, select()
             -
             ERROR_TIMEOUT (during proxy communication)
             ERROR_BAD_FORMAT (very invalid proxy reply)
             ERROR_ACCESS_DENIED (by proxy)
             ERROR_CONNECTION_UNAVAIL (socks proxy can't connect to identd)
             ERROR_INVALID_ACCESS (proxy refused identd auth)
             ERROR_INVALID_DATA (proxy returned invalid code)
             ERROR_INVALID_ID_AUTHORITY (proxy requires use of auth method that's not supported)
             ERROR_GEN_FAILURE (socks5/https general failure)
             ERROR_CALL_NOT_IMPLEMENTED (socks5 command not supported)
             ERROR_INVALID_ADDRESS (socks5 address type not supported)
             -
             HTTP: anything from TNETLIBUSER.pfnHttpGatewayInit, TNETLIBUSER.pfnHttpGatewayBegin,
                   MS_NETLIB_SENDHTTPREQUEST or MS_NETLIB_RECVHTTPHEADERS
  }
  MS_NETLIB_OPENCONNECTION:PAnsiChar = 'Netlib/OpenConnection';

  {
    wParam : HANDLE
    lParam : Pointer to an initialised NETLIBHTTPPROXYINFO structure
    Affects: Sets the required information for an HTTP proxy connection -- see notes
    Returns: [non zero] on success, NULL(0) on failure
    Notes  : This service is designed to be called from
             within TNETLIBUSER.pfnHttpGatewayInit (see notes in C header under
             MS_NETLIB_REGISTERUSER)
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_SETHTTPPROXYINFO:PAnsiChar = 'Netlib/SetHttpProxyInfo';

  {
    wParam : HANDLE
    lParam : 0
    Affects: Get's the SOCKET associated with a handle -- see notes
    Returns: the SOCKET on success, INVALID_SOCKET on failure
    Notes  : The Netlib handle passed to this service should only be passed
             if they were returned with MS_NETLIB_OPENCONNECTION or MS_NETLIB_BINDPORT
             -
             Be careful how you use this socket because you might be connected via an
             HTTP proxy, in which case calling send/recv() will break things
             -
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_GETSOCKET:PAnsiChar = 'Netlib/GetSocket';

{
  Converts string representation of IP and port into numerical SOCKADDR_INET
  IPv4 could supplied in formats address:port or address
  IPv6 could supplied in formats [address]:port or [address]
  wParam=(WPARAM)(char*) string to convert
  lParam=(LPARAM)(SOCKADDR_INET*) numeric IP address structure
  Returns 0 on success
}
  MS_NETLIB_STRINGTOADDRESS:PAnsiChar = 'Netlib/StringToAddress';

{
  Converts numerical representation of IP in SOCKADDR_INET into string representation with IP and port
  IPv4 will be supplied in formats address:port or address
  IPv6 will be supplied in formats [address]:port or [address]
  wParam=(WPARAM)(int) 0 - lParam - (SOCKADDR_INET*); 1 - lParam - (unsigned) in host byte order
  lParam=(LPARAM)(SOCKADDR_INET*) or (unsigned) numeric IP address structure
  Returns pointer to the string or NULL if not successful
}
  MS_NETLIB_ADDRESSTOSTRING:PAnsiChar = 'Netlib/AddressToString';

type
  TNETLIBCONNINFO = record
	  cbSize  :int;
	  szIpPort:array [0..63] of AnsiChar;
	  dwIpv4  :uint;
	  wPort   :word;
  end;

const
{
  Get connection Information
  IPv4 will be supplied in formats address:port or address
  IPv6 will be supplied in formats [address]:port or [address]
  wParam=(WPARAM)(HANDLE)hConnection
  lParam=(LPARAM)(NETLIBCONNINFO*) pointer to the connection information structure to fill
  Returns 0 if successful
}
  MS_NETLIB_GETCONNECTIONINFO:PAnsiChar = 'Netlib/GetConnectionInfo';

type
  TNETLIBIPLIST = record
	  cbNum:uint;
	  szIp :array[0..0,0..63] of AnsiChar;
  end;

const
{
  Get connection Information
  wParam=(WPARAM)IP filter 1 - return global only IPv6 address, 0 all IPs
  Returns (INT_PTR)(NETLIBIPLIST*) numeric IP address address array
  the last element of the array is all 0s, 0 if not successful
}
  MS_NETLIB_GETMYIP:PAnsiChar = 'Netlib/GetMyIP';

  {
    wParam : HANDLE
    lParam : Pointer to a initialised TNETLIBHTTPREQUEST structure
    Affect : Send an HTTP request over a connection, see notes
    Returns: The number of bytes on success, SOCKET_ERROR on failure
    Notes  : HANDLE must of been returned by MS_NETLIB_OPENCONNECTION,,
             If you use NLHRF_SMARTAUTHHEADER and NTLM auth is in use then
             full NTLM auth transcation occurs, comprising sending the
             domain, getting the challenge, sending the response.
             NETLIBHTTPREQUEST.resultCode and NETLIBHTTPREQUEST.szResultDescr are
             ignored by this service.
    Errors : ERROR_INVALID_PARAMETER, MS_NETLIB_SEND (return codes)
  }
  MS_NETLIB_SENDHTTPREQUEST:PAnsiChar = 'Netlib/SendHttpRequest';

  {
    wParam : HANDLE
    lParam : 0
    Affect : Receive HTTP headers, see notes
    Returns: A pointer to a TNETLIBHTTPREQUEST structure on success, NULL(0) on failure
    Notes  : The returned pointer must be freed after it's done with
             use MS_NETLIB_FREEHTTPREQUESTSTRUCT.
             -
             HANDLE must be returned by MS_NETLIB_OPENCONNECTION
             -
             Return^.pData=NIL and Return^.dataLength=0 always
             -
             The returned data should be retrieved using MS_NETLIB_RECV once
             the headers have been parsed.
             If headers haven't finished within 60 seconds the function returns
             NULL(0) and ERROR_TIMEOUT
    Errors : ERROR_INVALID_PARAMETER, any MS_NETLIB_RECV or select()
             ERROR_HANDLE_EOF (connection closed bfore headers complete)
             ERROR_TIMEOUT (headers still not complete after 60 seconds)
             ERROR_BAD_FORMAT (invalid character or line ending in headers, or first line is blank)
             ERROR_BUFFER_OVERFLOW (each header line must be less than 4096 chars long)
             ERROR_INVALID_DATA (first header line is malformed ("http/[01].[0-9] [0-9]+ .*", or no colon in subsequent line)

  }
  MS_NETLIB_RECVHTTPHEADERS:PAnsiChar = 'Netlib/RecvHttpHeaders';

  {
    wParam : 0
    lParam : Pointer returned by MS_NETLIB_RECVHTTPHEADERS to free
    Affect : Free the memory used by a TNETLIBHTTPREQUEST structure, see notes
    Returns: [non zero] on success, NULL(0) on failure
    Notes  : This service should only be used with memory pointers returned
             by either MS_NETLIB_RECVHTTPHEADERS or MS_NETLIB_HTTPTRANSACTION!.
    Errors : ERROR_INVALID_PARAMETER

  }
  MS_NETLIB_FREEHTTPREQUESTSTRUCT:PAnsiChar = 'Netlib/FreeHttpRequestStruct';

  {
    wParam : HANDLE
    lParam : Pointer to a TNETLIBHTTPREQUEST structure
    Affect : Carry out an entire HTTP transaction, see notes
    Returns: another pointer to a TNETLIBHTTPREQUEST structure or NULL(0)
             on failure
    Notes  : The returned pointer must be freed at some point
             with MS_NETLIB_FREEHTTPREQUESTSTRUCT,
             -
             TNETLIBHTTPREQUEST.szUrl should have a full HTTP URL, if it
             does not start with http://, that will be assumed, but do not
             take this assumption to stay assumed (heh..) in the future
             -
             this service equivalent of open(), sendhttp(), getheaders()
             netlib_recv(), netlib_closehandle()
             -
             TNETLIBHTTPREQUEST.headers will be added to with the following
             headers if they're not already present :
             "Host" (even if it is requested in .flags)
             "User-Agent" (in form : 'Miranda/d.d.d.d <(status of release)>')
             "Content-Length" (for POSTs only, set to TNETLIBHTTPREQUEST.dataLength)

             If you don't want to send any of these headers --
             set TNETLIBHTTPREQUEST.headers to NULL(0)
             -
             In the returned pointer, pData[dataLen] is always 0 for 'safety'
             also : headers, headersCount, pData, dataLength, resultCode and
                    szResultDescr are all valid
             -
             Also take care not to assume that a returned pointer means that
             at the HTTP level it all worked out -- refer to the resultCode for
             2xx before doing anything else
             -
    Errors : ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY
             Errors returned by the aforementioned internally used functions
  }
  MS_NETLIB_HTTPTRANSACTION:PAnsiChar = 'Netlib/HttpTransaction';

  {
    wParam : HANDLE
    lParam : Pointer to an initialised TNETLIBBUFFER structure
    Affect : Send data over an open connection see notes
    Returns: The number of bytes sent on success, SOCKET_ERROR on failure
    Notes  : see Netlib_Send() helper function
    Errors : ERROR_INVALID_PARAMETER,
             anything from socket(), connect()
             send(), TNETLIBUSER.pfnHttpGatewayWrapSend(),
             (HTTP proxy): ERROR_GEN_FAILURE (http result code wasn't 2xx)
             MS_NETLIB_SENDHTTPREQUEST, MS_NETLIB_RECVHTTPHEADERS
  }
  MS_NETLIB_SEND:PAnsiChar = 'Netlib/Send';

  {
    wParam : HANDLE
    lParam : Pointer to an initialised TNETLIBBUFFER structure
    Affect : Receive data over a connection, see notes
    Returns: The number of bytes read on success, SOCKET_ERROR on failure
    Notes  :
             This service uses some of the same flags as MS_NETLIB_SEND :
             MSG_PEEK,
             MSG_NODUMP,
             MSG_DUMPPROXY,
             MSG_NOHTTPGATEWAYWRAP,
             MSG_DUMPASTEXT,
             MSG_RAW
             -
             On using MSG_NOHTTPGATEWAYWRAP: Because packets through an HTTP proxy are
             batched and cached and stuff, using this flag is not a guarantee that it
             will be obeyed, and if it is it may even be propogated to future calls
             even if you don't specify it then. Because of this, the flag should be
             considered an all-or-nothing thing: either use it for the entire duration
             of a connection, or not at all.
    Errors : ERROR_INVALID_PARAMETER, anything from recv()
             (HTTP proxy):
             ERROR_GEN_FAILURE (http result code wasn't 2xx)
             ERROR_INVALID_DATA (no Content-Length header in reply)
             ERROR_NOT_ENOUGH_MEMORY (Content-Length very large)
             ERROR_HANDLE_EOF (connection closed before Content-Length bytes recved)
                               anything from select(),
             MS_NETLIB_RECVHTTPHEADERS, nlu.pfnHttpGatewayUnwrapRecv, socket(),
             connect(), MS_NETLIB_SENDHTTPREQUEST

  }
  MS_NETLIB_RECV:PAnsiChar = 'Netlib/Recv';

  {
    wParam : 0
    lParam : Pointer to an initialised TNETLIBSELECT structure
    Affect : Determine the status of one or more connections, see notes
    Returns: The numbe of ready connections, SOCKET_ERROR on failure
    Notes  : All handles passed to this service must have been returned
             either by MS_NETLIB_OPENCONNECTION or MS_NETLIB_BINDPORT,
             the last handle in each list must be followed by either NULL
             or INVALID_HANDLE_VALUE.
    Errors : ERROR_INVALID_HANDLE, ERROR_INVALID_DATA, anything from select()
  }
  MS_NETLIB_SELECT  :PAnsiChar = 'Netlib/Select';
  MS_NETLIB_SELECTEX:PAnsiChar = 'Netlib/SelectEx'; // added in v0.3.3

{
  Shutdown connection
  wParam=(WPARAM)(HANDLE)hConnection
  lParam=(LPARAM)0
  Returns 0
}
  MS_NETLIB_SHUTDOWN:PAnsiChar = 'Netlib/Shutdown';

  {
    wParam : HANDLE
    lParam : maxPacketSize
    Affect : Create a packet receiver, see notes
    Returns: A handle on success, NULL(0) on failure
    Notes  : The packet receiver implements the common situation where
             you have a variable length of packets coming thru over a connection
             and you want them split up in order to handle them.
             -
             The major limiation is, that the buffer is created in memory,
             so you can't have arbitrarily large packets
    Errors : ERROR_INVALID_PARAMETER, ERROR_OUTOFMEMORY
  }
  MS_NETLIB_CREATEPACKETRECVER:PAnsiChar = 'Netlib/CreatePacketRecver';

  {
    wParam : Handle returned by MS_NETLIB_CREATEPACKETRECVER
    lParam : Pointer to an initialised TNETLIBPACKETRECVER
    Returns: The total number of bytes available in the buffer, NULL(0)
             if the connection was closed or SOCKET_ERROR.
             -
             If TNETLIBPACKETRECVER.bytesUsed is set to zero and the
             buffer is already full up to the maxPacketSize, it is assumed
             that a too large packet has been received, All data in
             the buffer is discarded and receiving has started anew.
             -
             This will probably cause alignment problem so if you think
             that tis iss likely to happen, then you should deal with it
             yourself.
             -
             Closing the packet receiver will not close the associated
             connection but will discard any bytes still in the buffer,
             so if you intend to carry on reading from that connection,
             make sure you have processed the buffer first.
             -
             This service is equivalent of memmove() to remove
             the first bytesUsed from the buffer, select(), if dwTimeOut
             is not INFINITE, then MS_NETLIB_RECV
    Errors : ERROR_INVALID_PARAMETER, ERROR_TIMEOUT, anything from select(),
             MS_NETLIB_RECV
  }
  MS_NETLIB_GETMOREPACKETS:PAnsiChar = 'Netlib/GetMorePackets';

  {
    wParam : HANDLE
    lParam : Pointer to null terminated string to uh, log.
    Affect : Add a message to the log (if it's running) see notes
    Returns: non zeror on success, NULL(0) on failure
    Notes  : Don't include \r\n or #13#10 it's not needed,
             -
             Doesn't support formatting like the given C code for
             Netlib_Logf, just use FmtStr() and then call this service
             if you want that.
    Errors : ERROR_INVALID_PARAMETER
  }
  MS_NETLIB_LOG:PAnsiChar = 'Netlib/Log';
  MS_NETLIB_LOGW:PAnsiChar = 'Netlib/LogW';

  {
    Sets a gateway polling timeout interval
    wParam=(WPARAM)(HANDLE)hConn
    lParam=(LPARAM)timeout
    Returns previous timeout value
    Errors: -1
  }
  MS_NETLIB_SETPOLLINGTIMEOUT:PAnsiChar = 'Netlib/SetPollingTimeout';

{
  Makes connection SSL
  wParam=(WPARAM)(HANDLE)hConn
  lParam=0
  Returns 0 on failure 1 on success
}
  MS_NETLIB_STARTSSL:PAnsiChar = 'Netlib/StartSsl';

type
  TNETLIBSSL = record
    cbSize:int;
    host  :PAnsiChar; // Expected host name
    flags :int;       // Reserved
  end;

  const
/////////////////////////////////////////////////////////////////////////////////////////
// Security providers (0.6+)

  NNR_UNICODE = 1;
// Inits a required security provider. Right now only NTLM is supported
// Returns HANDLE = NULL on error or non-null value on success
// Known providers: Basic, NTLM, Negotiate, Kerberos, GSSAPI - (Kerberos SASL)
  MS_NETLIB_INITSECURITYPROVIDER:PAnsiChar = 'Netlib/InitSecurityProvider';

type
  TNETLIBNTLMINIT2 = record
    cbSize        :size_t;
    szProviderName:TChar;
    szPrincipal   :TChar;
    flags         :uint;
  end;

const
  MS_NETLIB_INITSECURITYPROVIDER2:PAnsiChar = 'Netlib/InitSecurityProvider2';

// Destroys a security provider's handle, provided by Netlib_InitSecurityProvider.
// Right now only NTLM is supported
  MS_NETLIB_DESTROYSECURITYPROVIDER:PAnsiChar = 'Netlib/DestroySecurityProvider';

// Returns the NTLM response string. The result value should be freed using mir_free
  MS_NETLIB_NTLMCREATERESPONSE:PAnsiChar = 'Netlib/NtlmCreateResponse';

type
  TNETLIBNTLMREQUEST = record
    szChallenge:PAnsiChar;
    userName   :PAnsiChar;
    password   :PAnsiChar;
//    complete   :int; // not present in last versions
  end;

type
  TNETLIBNTLMREQUEST2 = record
    cbSize     :size_t;
    szChallenge:PAnsiChar;
    szUserName :TChar;
    szPassword :TChar;
    complete   :uint;
    flags      :uint;
  end;

const
  MS_NETLIB_NTLMCREATERESPONSE2 = 'Netlib/NtlmCreateResponse2';

/////////////////////////////////////////////////////////////////////////////////////////
// Netlib hooks (0.8+)

// WARNING: these hooks are being called in the context of the calling thread, without switching
// to the first thread, like all another events do. The hook procedure should be ready for the
// multithreaded mode
//
// Parameters:
//    wParam: NETLIBNOTIFY* - points to the data being sent/received
//    lParam: NETLIBUSER*   - points to the protocol definition

type
  TNETLIBNOTIFY = record
    nlb   :PNETLIBBUFFER; // pointer to the request buffer
    result:int;           // amount of bytes really sent/received
  end;

const
  ME_NETLIB_FASTRECV :PAnsiChar = 'Netlib/OnRecv';  // being called on every receive
  ME_NETLIB_FASTSEND :PAnsiChar = 'Netlib/OnSend';  // being called on every send
  ME_NETLIB_FASTDUMP :PAnsiChar = 'Netlib/OnDump';  // being called on every dump

{$ENDIF}
