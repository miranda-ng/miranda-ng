/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#define NLH_INVALID      0
#define NLH_USER         'USER'
#define NLH_CONNECTION   'CONN'
#define NLH_BOUNDPORT    'BIND'
#define NLH_PACKETRECVER 'PCKT'
int GetNetlibHandleType(void*);

struct NetlibUser
{
	int handleType;
	NETLIBUSER user;
	NETLIBUSERSETTINGS settings;
	char * szStickyHeaders;
	int toLog;
	int inportnum;
	int outportnum;
};

struct NetlibNestedCriticalSection
{
	HANDLE hMutex;
	DWORD dwOwningThreadId;
	int lockCount;
};

struct NetlibHTTPProxyPacketQueue
{
	NetlibHTTPProxyPacketQueue *next;
	PBYTE dataBuffer;
	int dataBufferLen;
};

typedef union _SOCKADDR_INET_M {
	SOCKADDR_IN Ipv4;
	SOCKADDR_IN6 Ipv6;
	USHORT si_family;
} SOCKADDR_INET_M, *PSOCKADDR_INET_M;

struct NetlibConnection
{
	int handleType;
	SOCKET s, s2;
	bool usingHttpGateway;
	bool usingDirectHttpGateway;
	bool proxyAuthNeeded;
	bool dnsThroughProxy;
	bool termRequested;
	NetlibUser *nlu;
	NETLIBHTTPPROXYINFO nlhpi;
	PBYTE dataBuffer;
	int dataBufferLen;
	CRITICAL_SECTION csHttpSequenceNums;
	HANDLE hOkToCloseEvent;
	LONG dontCloseNow;
	NetlibNestedCriticalSection ncsSend, ncsRecv;
	HSSL hSsl;
	NetlibHTTPProxyPacketQueue * pHttpProxyPacketQueue;
	char *szNewUrl;
	char *szProxyServer;
	WORD wProxyPort;
	int proxyType;
	int pollingTimeout;
	unsigned lastPost;
	NETLIBOPENCONNECTION nloc;
};

struct NetlibBoundPort {
	int handleType;
	SOCKET s;
	SOCKET s6;
	WORD wPort;
	WORD wExPort;
	NetlibUser *nlu;
	NETLIBNEWCONNECTIONPROC_V2 pfnNewConnectionV2;
	HANDLE hThread;
	void *pExtra;
};

struct NetlibPacketRecver {
	int handleType;
	NetlibConnection *nlc;
	NETLIBPACKETRECVER packetRecver;
};

//netlib.c
void NetlibFreeUserSettingsStruct(NETLIBUSERSETTINGS *settings);
void NetlibDoClose(NetlibConnection *nlc, bool noShutdown = false);
INT_PTR NetlibCloseHandle(WPARAM wParam, LPARAM lParam);
void NetlibInitializeNestedCS(NetlibNestedCriticalSection *nlncs);
void NetlibDeleteNestedCS(NetlibNestedCriticalSection *nlncs);
#define NLNCS_SEND  0
#define NLNCS_RECV  1
int NetlibEnterNestedCS(NetlibConnection *nlc, int which);
void NetlibLeaveNestedCS(NetlibNestedCriticalSection *nlncs);
INT_PTR NetlibBase64Encode(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibBase64Decode(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpUrlEncode(WPARAM wParam, LPARAM lParam);

extern mir_cs csNetlibUser;
extern LIST<NetlibUser> netlibUser;

//netlibautoproxy.c
void NetlibLoadIeProxy(void);
void NetlibUnloadIeProxy(void);
char* NetlibGetIeProxy(char *szUrl);
bool NetlibGetIeProxyConn(NetlibConnection *nlc, bool forceHttps);

//netlibbind.c
int NetlibFreeBoundPort(NetlibBoundPort *nlbp);
INT_PTR NetlibBindPort(WPARAM wParam, LPARAM lParam);
bool BindSocketToPort(const char *szPorts, SOCKET s, SOCKET s6, int* portn);

//netlibhttp.c
INT_PTR NetlibHttpSendRequest(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpRecvHeaders(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpFreeRequestStruct(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpTransaction(WPARAM wParam, LPARAM lParam);
void NetlibHttpSetLastErrorUsingHttpResult(int result);
NETLIBHTTPREQUEST* NetlibHttpRecv(NetlibConnection* nlc, DWORD hflags, DWORD dflags, bool isConnect = false);
void NetlibConnFromUrl(const char* szUrl, bool secur, NETLIBOPENCONNECTION &nloc);

//netlibhttpproxy.c
int NetlibInitHttpConnection(NetlibConnection *nlc, NetlibUser *nlu, NETLIBOPENCONNECTION *nloc);
int NetlibHttpGatewayRecv(NetlibConnection *nlc, char *buf, int len, int flags);
int NetlibHttpGatewayPost(NetlibConnection *nlc, const char *buf, int len, int flags);
void HttpGatewayRemovePacket(NetlibConnection *nlc, int pck);

INT_PTR NetlibHttpGatewaySetInfo(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpSetPollingTimeout(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibHttpSetSticky(WPARAM wParam, LPARAM lParam);

//netliblog.c
void NetlibLogShowOptions(void);
void NetlibDumpData(NetlibConnection *nlc, PBYTE buf, int len, int sent, int flags);
void NetlibLogf(NetlibUser* nlu, const char *fmt, ...);
void NetlibLogInit(void);
void NetlibLogShutdown(void);

//netlibopenconn.c
DWORD DnsLookup(NetlibUser *nlu, const char *szHost);
int WaitUntilReadable(SOCKET s, DWORD dwTimeout, bool check = false);
int WaitUntilWritable(SOCKET s, DWORD dwTimeout);
bool NetlibDoConnect(NetlibConnection *nlc);
bool NetlibReconnect(NetlibConnection *nlc);
INT_PTR NetlibOpenConnection(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibStartSsl(WPARAM wParam, LPARAM lParam);

//netlibopts.c
int NetlibOptInitialise(WPARAM wParam, LPARAM lParam);
void NetlibSaveUserSettingsStruct(const char *szSettingsModule, NETLIBUSERSETTINGS *settings);

//netlibpktrecver.c
INT_PTR NetlibPacketRecverCreate(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibPacketRecverGetMore(WPARAM wParam, LPARAM lParam);

//netlibsock.c
#define NL_SELECT_READ  0x0001
#define NL_SELECT_WRITE 0x0002
#define NL_SELECT_ALL   (NL_SELECT_READ+NL_SELECT_WRITE)

INT_PTR NetlibSend(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibRecv(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibSelect(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibSelectEx(WPARAM wParam, LPARAM lParam);
INT_PTR NetlibShutdown(WPARAM wParam, LPARAM lParam);

bool NetlibStringToAddress(const char* str, SOCKADDR_INET_M* addr);
char* NetlibAddressToString(SOCKADDR_INET_M* addr);
void NetlibGetConnectionInfo(NetlibConnection* nlc, NETLIBCONNINFO *connInfo);
NETLIBIPLIST* GetMyIp(unsigned flags);

//netlibupnp.c
bool NetlibUPnPAddPortMapping(WORD intport, char *proto,
							  WORD *extport, DWORD *extip, bool search);
void NetlibUPnPDeletePortMapping(WORD extport, char* proto);
void NetlibUPnPCleanup(void*);
void NetlibUPnPInit(void);
void NetlibUPnPDestroy(void);

//netlibsecurity.c
void   NetlibSecurityInit(void);
void   NetlibDestroySecurityProvider(HANDLE hSecurity);
HANDLE NetlibInitSecurityProvider(const TCHAR* szProvider, const TCHAR* szPrincipal);
HANDLE NetlibInitSecurityProvider(const char* szProvider, const char* szPrincipal);
char*  NtlmCreateResponseFromChallenge(HANDLE hSecurity, const char *szChallenge, const TCHAR* login, const TCHAR* psw,
									   bool http, unsigned& complete);

static __inline INT_PTR NLSend(NetlibConnection *nlc, const char *buf, int len, int flags) {
	NETLIBBUFFER nlb = {(char*)buf, len, flags};
	return NetlibSend((WPARAM)nlc, (LPARAM)&nlb);
}

static __inline INT_PTR NLRecv(NetlibConnection *nlc, char *buf, int len, int flags) {
	NETLIBBUFFER nlb = {buf, len, flags};
	return NetlibRecv((WPARAM)nlc, (LPARAM)&nlb);
}
