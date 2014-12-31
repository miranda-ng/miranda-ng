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

#include "..\..\core\commonheaders.h"
#include "netlib.h"

#include <wininet.h>

/*
/////////////////////////////////////////////////////////////////////
//  ResolveHostName                               (a helper function)
/////////////////////////////////////////////////////////////////////
DWORD __stdcall ResolveHostName(LPSTR lpszHostName,
	LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize)
{
	if (*lpdwIPAddressSize < 17 || lpszIPAddress == NULL)
	{
		*lpdwIPAddressSize = 17;
		return ERROR_INSUFFICIENT_BUFFER;
	}

	IN_ADDR ip;
	ip.s_addr = inet_addr(lpszHostName);
	if (ip.s_addr == INADDR_NONE)
	{
		PHOSTENT myhost = gethostbyname(lpszHostName);
		if (myhost != NULL)
			ip = *(PIN_ADDR)myhost->h_addr;
		else
			return SOCKET_ERROR;
	}
	mir_snprintf(lpszIPAddress, *lpdwIPAddressSize, "%u.%u.%u.%u",
		ip.s_net, ip.s_host, ip.s_lh, ip.s_impno);

	return 0;
}

/////////////////////////////////////////////////////////////////////
//  IsResolvable                                  (a helper function)
/////////////////////////////////////////////////////////////////////
BOOL __stdcall IsResolvable(LPSTR lpszHost)
{
	char szDummy[255];
	DWORD dwDummySize = sizeof (szDummy) - 1;

	if (ResolveHostName(lpszHost, szDummy, &dwDummySize))
		return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//  GetIPAddress                                  (a helper function)
/////////////////////////////////////////////////////////////////////
DWORD __stdcall GetIPAddress(LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize)
{
	char szHostBuffer[255];

	if (gethostname(szHostBuffer, sizeof (szHostBuffer) - 1) != ERROR_SUCCESS)
		return (ERROR_INTERNET_INTERNAL_ERROR);
	return (ResolveHostName(szHostBuffer, lpszIPAddress, lpdwIPAddressSize));
}

/////////////////////////////////////////////////////////////////////
//  IsInNet                                       (a helper function)
/////////////////////////////////////////////////////////////////////
BOOL __stdcall IsInNet(LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask)
{
	DWORD dwDest;
	DWORD dwIpAddr;
	DWORD dwMask;

	dwIpAddr = inet_addr(lpszIPAddress);
	dwDest = inet_addr(lpszDest);
	dwMask = inet_addr(lpszMask);

	if ((dwDest == INADDR_NONE)  ||
		(dwIpAddr == INADDR_NONE) || ((dwIpAddr & dwMask) != dwDest))
		return (FALSE);

	return (TRUE);
}

static const AutoProxyHelperVtbl OurVtbl =
{
	IsResolvable,
	GetIPAddress,
	ResolveHostName,
	IsInNet,
	NULL,
	NULL,
	NULL,
	NULL
};

static AutoProxyHelperFunctions HelperFunctions = { &OurVtbl };
*/

static char *szProxyHost[3];
static LIST<char> proxyBypass(5);

static HMODULE hModJS;

static pfnInternetInitializeAutoProxyDll pInternetInitializeAutoProxyDll;
static pfnInternetDeInitializeAutoProxyDll pInternetDeInitializeAutoProxyDll;
static pfnInternetGetProxyInfo pInternetGetProxyInfo;

static bool bEnabled, bOneProxy;

static void GetFile(char* szUrl, AUTO_PROXY_SCRIPT_BUFFER &buf)
{
	NetlibUser nlu = {0};
	NETLIBHTTPREQUEST nlhr = {0};

	nlu.handleType = NLH_USER;
	nlu.user.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.user.szSettingsModule = "(NULL)";
	nlu.toLog = 1;

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_REDIRECT;
	nlhr.szUrl = szUrl;

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)NetlibHttpTransaction((WPARAM)&nlu, (LPARAM)&nlhr);

	if (nlhrReply)
	{
		if (nlhrReply->resultCode == 200)
		{
			buf.lpszScriptBuffer = nlhrReply->pData;
			buf.dwScriptBufferSize = nlhrReply->dataLength + 1;

			nlhrReply->dataLength = 0;
			nlhrReply->pData = NULL;
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
}

bool NetlibGetIeProxyConn(NetlibConnection *nlc, bool forceHttps)
{
	bool noHttp = false;
	bool usingSsl = false;
	char szUrl[256] = "";

	if ((nlc->nloc.flags & (NLOCF_HTTP | NLOCF_HTTPGATEWAY) && nlc->nloc.flags & NLOCF_SSL) ||
		nlc->nloc.wPort == 443 || forceHttps)
	{
		mir_snprintf(szUrl, SIZEOF(szUrl), "https://%s", nlc->nloc.szHost);
		usingSsl = true;
	}
	else if (nlc->nloc.flags & (NLOCF_HTTPGATEWAY | NLOCF_HTTP) || nlc->usingHttpGateway)
		mir_snprintf(szUrl, SIZEOF(szUrl), "http://%s", nlc->nloc.szHost);
	else
	{
		strncpy_s(szUrl, nlc->nloc.szHost, _TRUNCATE);
		noHttp = true;
	}

	mir_free(nlc->szProxyServer); nlc->szProxyServer = NULL;
	nlc->wProxyPort = 0;
	nlc->proxyType = 0;

	char *mt = NetlibGetIeProxy(szUrl);
	char *m = NEWSTR_ALLOCA(mt);
	mir_free(mt);

	if (m == NULL) return false;

	// if multiple servers, use the first one
	char *c = strchr(m, ';'); if (c) *c = 0;

	// if 'direct' no proxy
	if (_stricmp(lrtrim(m), "direct") == 0) return false;

	// find proxy address
	char *h = strchr(m, ' ');
	if (h) { *h = 0; ++h; } else return false;

	// find proxy port
	char *p = strchr(h, ':');
	if (p) { *p = 0; ++p; }

	lrtrim(h); ltrim(p);
	if (_stricmp(m, "proxy") == 0 && h[0])
	{
		nlc->proxyType = (usingSsl || noHttp) ? PROXYTYPE_HTTPS : PROXYTYPE_HTTP;
		nlc->wProxyPort = p ? atol(p) : 8080;
		nlc->szProxyServer = mir_strdup(h);
	}
	else if (_stricmp(m, "socks") == 0 && h[0])
	{
		nlc->proxyType = PROXYTYPE_SOCKS4;
		nlc->wProxyPort = p ? atol(p) : 1080;
		nlc->szProxyServer = mir_strdup(h);
	}
	else if (_stricmp(m, "socks5") == 0 && h[0])
	{
		nlc->proxyType = PROXYTYPE_SOCKS5;
		nlc->wProxyPort = p ? atol(p) : 1080;
		nlc->szProxyServer = mir_strdup(h);
	}
	else
		return false;

	return true;
}

static char szAutoUrlStr[MAX_PATH] = "";
static AUTO_PROXY_SCRIPT_BUFFER abuf = {0};
static HANDLE hIeProxyMutex;
static bool bAutoProxyInit;

static void NetlibInitAutoProxy(void)
{
	if (bAutoProxyInit) return;

	if (!hModJS)
	{
		if (!(hModJS = LoadLibraryA("jsproxy.dll")))
			return;

		pInternetInitializeAutoProxyDll = (pfnInternetInitializeAutoProxyDll)
			GetProcAddress(hModJS, "InternetInitializeAutoProxyDll");

		pInternetDeInitializeAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)
			GetProcAddress(hModJS, "InternetDeInitializeAutoProxyDll");

		pInternetGetProxyInfo = (pfnInternetGetProxyInfo)
			GetProcAddress(hModJS, "InternetGetProxyInfo");
	}

	if (strstr(szAutoUrlStr, "file://") == NULL && strstr(szAutoUrlStr, "://") != NULL)
	{
		abuf.dwStructSize = sizeof(abuf);
		GetFile(szAutoUrlStr, abuf);
	}
	bAutoProxyInit = true;
}

struct IeProxyParam
{
	char *szUrl;
	char *szHost;
	char *szProxy;
};

static void NetlibIeProxyThread(void *arg)
{
	IeProxyParam *param = (IeProxyParam*)arg;
	param->szProxy = NULL;

	if (!bAutoProxyInit) {
		WaitForSingleObject(hIeProxyMutex, INFINITE);
		NetlibInitAutoProxy();
		ReleaseMutex(hIeProxyMutex);
	}

	BOOL res;
	char *loc = strstr(szAutoUrlStr, "file://");
	if (loc || strstr(szAutoUrlStr, "://") == NULL) {
		NetlibLogf(NULL, "Autoproxy Init file: %s", loc);
		loc = loc ? loc + 7 : szAutoUrlStr;
		res = pInternetInitializeAutoProxyDll(0, loc, NULL, NULL /*&HelperFunctions*/, NULL);
	}
	else {
		NetlibLogf(NULL, "Autoproxy Init %d", abuf.dwScriptBufferSize);
		if (abuf.dwScriptBufferSize)
			res = pInternetInitializeAutoProxyDll(0, NULL, NULL, NULL /*&HelperFunctions*/, &abuf);
		else
			res = false;
	}

	if (res) {
		char proxyBuffer[1024];
		char *proxy = proxyBuffer;
		DWORD dwProxyLen = sizeof(proxyBuffer);

		if (pInternetGetProxyInfo(param->szUrl, (DWORD)strlen(param->szUrl),
			param->szHost, (DWORD)strlen(param->szHost), &proxy, &dwProxyLen))
			param->szProxy = mir_strdup(lrtrim(proxy));

		NetlibLogf(NULL, "Autoproxy got response %s, Param: %s %s", param->szProxy, param->szUrl, param->szHost);
		pInternetDeInitializeAutoProxyDll(NULL, 0);
	}
	else NetlibLogf(NULL, "Autoproxy init failed");
}

char* NetlibGetIeProxy(char *szUrl)
{
	char *res = NULL;
	char* p = strstr(szUrl, "://");
	if (p) p += 3; else p = szUrl;

	char *szHost = NEWSTR_ALLOCA(p);
	p = strchr(szHost, '/'); if (p) *p = 0;
	p = strchr(szHost, ':'); if (p) *p = 0;
	_strlwr(szHost);

	if (bEnabled)
	{
		for (int i=0; i < proxyBypass.getCount(); i++)
		{
			if (strcmp(proxyBypass[i], "<local>") == 0)
			{
				if (strchr(szHost, '.') == NULL) return NULL;
			}
			else if (wildcmp(szHost, proxyBypass[i])) return NULL;
		}

		int ind = -1;
		if (strstr(szUrl, "http://"))
			ind = szProxyHost[0] ? 0 : 2;
		else if (strstr(szUrl, "https://"))
			ind = bOneProxy ? 0 : (szProxyHost[1] ? 1 : 2);
		else
			ind = szProxyHost[2] ? 2 : (bOneProxy ? 0 : (szProxyHost[1] ? 1 : 2));

		if (ind < 0 || !szProxyHost[ind]) return NULL;

		size_t len = strlen(szHost) + 20;
		res = (char*)mir_alloc(len);
		mir_snprintf(res, len, "%s %s", ind == 2 ? "SOCKS" : "PROXY", szProxyHost[ind]);
		return res;
	}

	if (szAutoUrlStr[0]) {
		IeProxyParam param = { szUrl, szHost, NULL };
		HANDLE hThread = mir_forkthread(NetlibIeProxyThread, &param);
		WaitForSingleObject(hThread, INFINITE);
		res = param.szProxy;
	}
	return res;
}

void NetlibLoadIeProxy(void)
{
	HKEY hSettings;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
		0, KEY_QUERY_VALUE, &hSettings))
		return;

	DWORD tValueLen, enabled = 0;
	char szHostStr[256] = "", szProxyBypassStr[4096] = "";

	tValueLen = sizeof(enabled);
	int tResult = RegQueryValueExA(hSettings, "ProxyEnable", NULL, NULL, (BYTE*)&enabled, &tValueLen);
	bEnabled = enabled && tResult == ERROR_SUCCESS;

	tValueLen = SIZEOF(szHostStr);
	tResult = RegQueryValueExA(hSettings, "ProxyServer", NULL, NULL, (BYTE*)szHostStr, &tValueLen);
	bEnabled = bEnabled && tResult == ERROR_SUCCESS;

	tValueLen = SIZEOF(szAutoUrlStr);
	RegQueryValueExA(hSettings, "AutoConfigUrl", NULL, NULL, (BYTE*)szAutoUrlStr, &tValueLen);

	tValueLen = SIZEOF(szProxyBypassStr);
	RegQueryValueExA(hSettings, "ProxyOverride", NULL, NULL, (BYTE*)szProxyBypassStr, &tValueLen);

	RegCloseKey(hSettings);

	if (bEnabled)
	{
		char* szProxy = ltrim(szHostStr);
		if (szProxy[0] == 0) { enabled = false; return; }

		while(true)
		{
			char *szProxyEnd = strchr(szProxy, ';');
			if (szProxyEnd) *szProxyEnd = 0;

			int ind = -1;
			if (strncmp(szProxy, "http = ", 5) == 0) { ind = 0; szProxy += 5; }
			else if (strncmp(szProxy, "https = ", 6) == 0) { ind = 1; szProxy += 6; }
			else if (strncmp(szProxy, "socks = ", 6) == 0) { ind = 2; szProxy += 6; }
			else if (strchr(szProxy, '=')) ind = -2;

			if (ind != -2)
			{
				bOneProxy = ind < 0; if (ind < 0) ind = 0;

				lrtrim(szProxy);

				if (strchr(szProxy, ':'))
					szProxyHost[ind] = mir_strdup(szProxy);
				else
				{
					size_t len = strlen(szProxy) + 10;
					szProxyHost[ind] = (char*)mir_alloc(len);
					mir_snprintf(szProxyHost[ind], len, "%s:%u", szProxy, ind == 2 ? 1080 : 8080);
				}
				if (bOneProxy) break;
			}
			if (szProxyEnd == NULL) break;
			szProxy = szProxyEnd + 1;
		}

		char* szProxyBypass = szProxyBypassStr;
		while(true)
		{
			char *szProxyBypassEnd = strchr(szProxyBypass, ';');
			if (szProxyBypassEnd) *szProxyBypassEnd = 0;

			lrtrim(szProxyBypass);

			proxyBypass.insert(_strlwr(mir_strdup(szProxyBypass)));
			if (szProxyBypassEnd == NULL) break;

			szProxyBypass = szProxyBypassEnd + 1;
		}
	}

	if (bEnabled || szAutoUrlStr[0])
		hIeProxyMutex = CreateMutex(NULL, FALSE, NULL);
}

void NetlibUnloadIeProxy(void)
{
	int i;

	for (i=0; i < 3; i++)
		mir_free(szProxyHost[i]);

	for (i=0; i < proxyBypass.getCount(); i++)
		mir_free(proxyBypass[i]);

	mir_free(abuf.lpszScriptBuffer);

	CloseHandle(hIeProxyMutex);
}
