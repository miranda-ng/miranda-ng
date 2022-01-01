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

#pragma once

#define NLH_INVALID      0
#define NLH_USER         'USER'
#define NLH_CONNECTION   'CONN'
#define NLH_BOUNDPORT    'BIND'
#define NLH_PACKETRECVER 'PCKT'
int GetNetlibHandleType(void*);

#define NLHRF_SMARTREMOVEHOST	0x00000004	 // for internal purposes only

struct NetlibUser
{
	int handleType;
	NETLIBUSER user;
	NETLIBUSERSETTINGS settings;
	char *szStickyHeaders;
	int toLog;
	int inportnum;
	int outportnum;
};

struct NetlibNestedCriticalSection
{
	HANDLE hMutex;
	uint32_t dwOwningThreadId;
	int lockCount;
};

struct NetlibHTTPProxyPacketQueue
{
	NetlibHTTPProxyPacketQueue *next;
	uint8_t *dataBuffer;
	int dataBufferLen;
};

union SOCKADDR_INET_M
{
	SOCKADDR_IN Ipv4;
	SOCKADDR_IN6 Ipv6;
	USHORT si_family;
};

struct NetlibConnection : public MZeroedObject
{
	NetlibConnection();
	~NetlibConnection();

	int handleType;
	SOCKET s, s2;
	bool proxyAuthNeeded;
	bool dnsThroughProxy;
	bool termRequested;
	
	NetlibUser *nlu;
	NETLIBOPENCONNECTION nloc;

	char *szNewUrl;
	
	mir_cs csHttpSequenceNums;
	HANDLE hOkToCloseEvent;
	LONG dontCloseNow;
	NetlibNestedCriticalSection ncsSend, ncsRecv;

	// SSL support
	HSSL hSsl;
	MBinBuffer foreBuf;

	// proxy support
	NETLIBHTTPPROXYINFO nlhpi;
	NetlibHTTPProxyPacketQueue *pHttpProxyPacketQueue;
	int proxyType;
	char *szProxyServer;
	uint16_t wProxyPort;
	CMStringA szProxyBuf;

	int pollingTimeout;
	unsigned lastPost;
};

struct NetlibBoundPort : public MZeroedObject
{
	NetlibBoundPort(HNETLIBUSER nlu, NETLIBBIND *nlb);
	~NetlibBoundPort() {
		close();
	}

	void close();

	int handleType;
	SOCKET s;
	SOCKET s6;
	uint16_t wPort;
	uint16_t wExPort;
	NetlibUser *nlu;
	NETLIBNEWCONNECTIONPROC pfnNewConnection;
	HANDLE hThread;
	void *pExtra;
};

struct NetlibPacketRecver
{
	int handleType;
	NetlibConnection *nlc;
	NETLIBPACKETRECVER packetRecver;
};

//netlib.c
void NetlibFreeUserSettingsStruct(NETLIBUSERSETTINGS *settings);
void NetlibDoCloseSocket(NetlibConnection *nlc, bool noShutdown = false);
void NetlibInitializeNestedCS(NetlibNestedCriticalSection *nlncs);
void NetlibDeleteNestedCS(NetlibNestedCriticalSection *nlncs);
#define NLNCS_SEND  0
#define NLNCS_RECV  1
int NetlibEnterNestedCS(NetlibConnection *nlc, int which);
void NetlibLeaveNestedCS(NetlibNestedCriticalSection *nlncs);

extern mir_cs csNetlibUser;
extern LIST<NetlibUser> netlibUser;

extern HANDLE hEventConnected;
extern HANDLE hEventDisconnected;

// netlibautoproxy.cpp
void NetlibLoadIeProxy(void);
void NetlibUnloadIeProxy(void);
char* NetlibGetIeProxy(char *szUrl);
bool NetlibGetIeProxyConn(NetlibConnection *nlc, bool forceHttps);

// netlibbind.cpp
int NetlibFreeBoundPort(NetlibBoundPort *nlbp);
bool BindSocketToPort(const char *szPorts, SOCKET s, SOCKET s6, int* portn);

// netlibhttp.cpp
void NetlibHttpSetLastErrorUsingHttpResult(int result);
NETLIBHTTPREQUEST* NetlibHttpRecv(NetlibConnection* nlc, uint32_t hflags, uint32_t dflags, bool isConnect = false);
void NetlibConnFromUrl(const char* szUrl, bool secur, NETLIBOPENCONNECTION &nloc);

// netliblog.cpp
void NetlibLogShowOptions(void);
void NetlibLogInit(void);
void NetlibLogShutdown(void);

// netlibopenconn.cpp
uint32_t DnsLookup(NetlibUser *nlu, const char *szHost);
int WaitUntilReadable(SOCKET s, uint32_t dwTimeout, bool check = false);
int WaitUntilWritable(SOCKET s, uint32_t dwTimeout);
bool NetlibDoConnect(NetlibConnection *nlc);
bool NetlibReconnect(NetlibConnection *nlc);

// netlibopts.cpp
int NetlibOptInitialise(WPARAM wParam, LPARAM lParam);
void NetlibSaveUserSettingsStruct(const char *szSettingsModule, const NETLIBUSERSETTINGS *settings);

// netlibsock.cpp
#define NL_SELECT_READ  0x0001
#define NL_SELECT_WRITE 0x0002
#define NL_SELECT_ALL   (NL_SELECT_READ+NL_SELECT_WRITE)

// netlibssl.cpp
bool OpenSsl_Init();
void OpenSsl_Unload();

// netlibupnp.cpp
bool NetlibUPnPAddPortMapping(uint16_t intport, char *proto, uint16_t *extport, uint32_t *extip, bool search);
void NetlibUPnPDeletePortMapping(uint16_t extport, char* proto);
void NetlibUPnPCleanup(void*);
void NetlibUPnPInit(void);
void NetlibUPnPDestroy(void);

// netlibsecurity.c
char* NtlmCreateResponseFromChallenge(HANDLE hSecurity, const char *szChallenge, const wchar_t* login, const wchar_t* psw, bool http, unsigned& complete);
