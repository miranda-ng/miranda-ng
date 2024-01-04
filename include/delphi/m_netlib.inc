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

  // for TNETLIBHTTPREQUEST.flags, .requestType

  { used by MS_NETLIB_RECVHTTPHEADERS returned structure }

  REQUEST_RESPONSE = 0;
  REQUEST_GET      = 1;
  REQUEST_POST     = 2;
  REQUEST_CONNECT  = 3;
  REQUEST_HEAD     = 4; // new in 0.5.1
  REQUEST_PUT      = 5;
  REQUEST_DELETE   = 6;

  NLHRF_MANUALHOST      = $00000002; // don't remove any host and/or protocol portion of szUrl before sending it
  NLHRF_HTTP11          = $00000010; // use HTTP 1.1
  NLHRF_PERSISTENT      = $00000020; // preserve connection on exit, open connection provided in the nlc field of the reply it should be supplied in
                                     // nlc field of request for reuse or closed if not needed
  NLHRF_SSL             = $00000040; // use ssl connection
  NLHRF_NOPROXY         = $00000080; // do not use proxy server
  NLHRF_REDIRECT        = $00000100; // handle HTTP redirect requests (response 30x), the resulting url provided in szUrl of the response
  NLHRF_NODUMP          = $00010000; // never dump this to the log
  NLHRF_NODUMPHEADERS   = $00020000; // don't dump http headers (only useful for POSTs and MS_NETLIB_HTTPTRANSACTION
  NLHRF_DUMPPROXY       = $00040000; // this transaction is a proxy communication. For dump filtering only.
  NLHRF_DUMPASTEXT      = $00080000; // dump posted and reply data as text. Headers are always dumped as text.
  NLHRF_NODUMPSEND      = $00100000; // do not dump sent message.

  // for TNETLIBBUFFER.flags

  MSG_NOHTTPGATEWAYWRAP = $010000; // don't wrap outgoing packet using TNETLIBUSER.pfnHttpGatewayWrapSend
  MSG_NODUMP            = $020000; // don't dump this packet to the log
  MSG_DUMPPROXY         = $040000; // this is proxy communication, for dump filtering only
  MSG_DUMPASTEXT        = $080000; // don't dump as hex, it's text
  MSG_RAW               = $100000; // send as raw, bybpass HTTP proxy stuff
  MSG_DUMPSSL           = $200000; // this is SSL traffic. For dump filtering only.


// all record types structures are declared in their own block because the C
// header file used forward declaration (to get typed parameters for certain
// function pointers). This sort of define-type-pointer-before-type can only be
// done in the same type block in D2 (don't know about later versions)

  FD_SETSIZE = 65;

type
  PNETLIBHTTPHEADER = ^TNETLIBHTTPHEADER;
  TNETLIBHTTPHEADER = record
    szName : PAnsiChar;
    szValue: PAnsiChar;
  end;
  ANETLIBHTTPHEADER = array [0..1000] of TNETLIBHTTPHEADER;

  PNETLIBHTTPREQUEST = ^TNETLIBHTTPREQUEST;

  TNETLIBHTTPREQUEST = record
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

type
  PNETLIBUSER = ^TNETLIBUSER;
  TNETLIBUSER = record
    szSettingsModule: PAnsiChar; // used for DB settings and log, 'NL' stuff
    szDescriptiveName: TChar;    // shows a descriptive name for which different proxy settings can be defined
    flags: dword;                // see NUF_* constants above
    minIncomingPorts: int;       // only if NUF_INCOMING, will be used for validation of user input
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

  {
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

function Netlib_RegisterUser(pInfo:PNETLIBUSER) : THANDLE; stdcall; external AppDll;

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

function Netlib_GetUserSettings(nlu:THANDLE; nlus:PNETLIBUSERSETTINGS) : int; stdcall; external AppDll;

  {
    Affects: Closes a handle, see notes
    Returns: Returns [non zero] on success, NULL(0) on failure
    Notes  : All netlib handles should be closed once they're finished with,
             If a SOCKET type is passed instead of netlib handle type, it is closed
    Errors : ERROR_INVALID_PARAMETER
  }

function Netlib_CloseHandle(pHandle:THANDLE) : int; stdcall; external AppDll;

  {
    Affect : Free the memory used by a TNETLIBHTTPREQUEST structure, see notes
    Returns: [non zero] on success, NULL(0) on failure
    Notes  : This service should only be used with memory pointers returned
             by either MS_NETLIB_RECVHTTPHEADERS or MS_NETLIB_HTTPTRANSACTION!.
    Errors : ERROR_INVALID_PARAMETER

  }

function Netlib_FreeHttpRequest(param:PNETLIBHTTPREQUEST) : bytebool; stdcall; external AppDll;

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

function Netlib_HttpTransaction(nlu:THANDLE; param:PNETLIBHTTPREQUEST) : PNETLIBHTTPREQUEST; stdcall; external AppDll;

  {
    Affect : Send data over an open connection see notes
    Returns: The number of bytes sent on success, SOCKET_ERROR on failure
    Notes  : see Netlib_Send() helper function
    Errors : ERROR_INVALID_PARAMETER,
             anything from socket(), connect()
             send(), TNETLIBUSER.pfnHttpGatewayWrapSend(),
             (HTTP proxy): ERROR_GEN_FAILURE (http result code wasn't 2xx)
             MS_NETLIB_SENDHTTPREQUEST, MS_NETLIB_RECVHTTPHEADERS
  }

function Netlib_Send(hConn:THANDLE; pBuf:Pointer; len,flags:int) : int; stdcall; external AppDll;

  {
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

function Netlib_Recv(hConn:THANDLE; pBuf:Pointer; len,flags:int) : int; stdcall; external AppDll;

  {
    Affect : Add a message to the log (if it's running) see notes
    Returns: non zeror on success, NULL(0) on failure
    Notes  : Don't include \r\n or #13#10 it's not needed,
             -
             Doesn't support formatting like the given C code for
             Netlib_Logf, just use FmtStr() and then call this service
             if you want that.
    Errors : ERROR_INVALID_PARAMETER
  }

function Netlib_Log(nlu:THANDLE; str:PAnsiChar) : int; stdcall; external AppDll;
function Netlib_LogW(nlu:THANDLE; str:PWideChar) : int; stdcall; external AppDll;

{$ENDIF}
