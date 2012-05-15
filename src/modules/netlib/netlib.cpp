/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2012 Miranda ICQ/IM project,
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
#include "netlib.h"

static BOOL bModuleInitialized = FALSE;

HANDLE hConnectionHeaderMutex, hConnectionOpenMutex; 
DWORD g_LastConnectionTick;
int connectionTimeout;
HANDLE hSendEvent=NULL, hRecvEvent=NULL;

typedef BOOL (WINAPI *tGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);

static int CompareNetlibUser(const NetlibUser* p1, const NetlibUser* p2)
{
	return strcmp(p1->user.szSettingsModule, p2->user.szSettingsModule);
}

LIST<NetlibUser> netlibUser(5, CompareNetlibUser);
CRITICAL_SECTION csNetlibUser;

SSL_API si;

void NetlibFreeUserSettingsStruct(NETLIBUSERSETTINGS *settings)
{
	mir_free(settings->szIncomingPorts);
	mir_free(settings->szOutgoingPorts);
	mir_free(settings->szProxyAuthPassword);
	mir_free(settings->szProxyAuthUser);
	mir_free(settings->szProxyServer);
}

void NetlibInitializeNestedCS(struct NetlibNestedCriticalSection *nlncs)
{
	nlncs->dwOwningThreadId= 0;
	nlncs->lockCount=0;
	nlncs->hMutex=CreateMutex(NULL,FALSE,NULL);
}

void NetlibDeleteNestedCS(struct NetlibNestedCriticalSection *nlncs)
{
	CloseHandle(nlncs->hMutex);
}

int NetlibEnterNestedCS(struct NetlibConnection *nlc,int which)
{
	struct NetlibNestedCriticalSection *nlncs;
	DWORD dwCurrentThreadId=GetCurrentThreadId();

	WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
	if(nlc==NULL || nlc->handleType!=NLH_CONNECTION) {
		ReleaseMutex(hConnectionHeaderMutex);
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	nlncs = (which == NLNCS_SEND) ? &nlc->ncsSend : &nlc->ncsRecv;
	if(nlncs->lockCount && nlncs->dwOwningThreadId==dwCurrentThreadId) {
		nlncs->lockCount++;
		ReleaseMutex(hConnectionHeaderMutex);
		return 1;
	}
	InterlockedIncrement(&nlc->dontCloseNow);
	ResetEvent(nlc->hOkToCloseEvent);
	ReleaseMutex(hConnectionHeaderMutex);
	WaitForSingleObject(nlncs->hMutex,INFINITE);
	nlncs->dwOwningThreadId=dwCurrentThreadId;
	nlncs->lockCount=1;
	if(InterlockedDecrement(&nlc->dontCloseNow)==0)
		SetEvent(nlc->hOkToCloseEvent);
	return 1;
}

void NetlibLeaveNestedCS(struct NetlibNestedCriticalSection *nlncs)
{
	if(--nlncs->lockCount==0) {
		nlncs->dwOwningThreadId=0;
		ReleaseMutex(nlncs->hMutex);
	}
}

static INT_PTR GetNetlibUserSettingInt(const char *szUserModule,const char *szSetting,int defValue)
{
	DBVARIANT dbv;
	if(DBGetContactSetting(NULL,szUserModule,szSetting,&dbv)
	   && DBGetContactSetting(NULL,"Netlib",szSetting,&dbv))
		return defValue;
	if(dbv.type==DBVT_BYTE) return dbv.bVal;
	if(dbv.type==DBVT_WORD) return dbv.wVal;
	return dbv.dVal;
}

static char *GetNetlibUserSettingString(const char *szUserModule,const char *szSetting,int decode)
{
	DBVARIANT dbv;
	if(DBGetContactSettingString(NULL,szUserModule,szSetting,&dbv)
	   && DBGetContactSettingString(NULL,"Netlib",szSetting,&dbv)) {
		return NULL;
	}
	else {
		char *szRet;
		if(decode) CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM)dbv.pszVal);
		szRet=mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
		if(szRet==NULL) SetLastError(ERROR_OUTOFMEMORY);
		return szRet;
	}
}

static INT_PTR NetlibRegisterUser(WPARAM,LPARAM lParam)
{
	NETLIBUSER *nlu=(NETLIBUSER*)lParam;
	struct NetlibUser *thisUser;

	if(nlu==NULL || nlu->cbSize!=sizeof(NETLIBUSER) || nlu->szSettingsModule==NULL
	   || (!(nlu->flags&NUF_NOOPTIONS) && nlu->szDescriptiveName==NULL)
	   || (nlu->flags&NUF_HTTPGATEWAY && (nlu->pfnHttpGatewayInit==NULL))) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	thisUser = (struct NetlibUser*)mir_calloc(sizeof(struct NetlibUser));
	thisUser->handleType = NLH_USER;
	thisUser->user = *nlu;

	EnterCriticalSection(&csNetlibUser);
	if (netlibUser.getIndex(thisUser) >= 0) 
	{
		LeaveCriticalSection(&csNetlibUser);
		mir_free(thisUser);
		SetLastError(ERROR_DUP_NAME);
		return 0;
	}
	LeaveCriticalSection(&csNetlibUser);

	if (nlu->szDescriptiveName) {
		thisUser->user.ptszDescriptiveName = (thisUser->user.flags&NUF_UNICODE ? mir_u2t((WCHAR*)nlu->ptszDescriptiveName) : mir_a2t(nlu->szDescriptiveName));
	}
	if((thisUser->user.szSettingsModule=mir_strdup(nlu->szSettingsModule))==NULL
	   || (nlu->szDescriptiveName && thisUser->user.ptszDescriptiveName ==NULL)
	   || (nlu->szHttpGatewayUserAgent && (thisUser->user.szHttpGatewayUserAgent=mir_strdup(nlu->szHttpGatewayUserAgent))==NULL)) 
	{
		mir_free(thisUser);
		SetLastError(ERROR_OUTOFMEMORY);
		return 0;
	}
	if (nlu->szHttpGatewayHello)
		thisUser->user.szHttpGatewayHello=mir_strdup(nlu->szHttpGatewayHello);
	else
		thisUser->user.szHttpGatewayHello=NULL;

	thisUser->settings.cbSize=sizeof(NETLIBUSERSETTINGS);
	thisUser->settings.useProxy=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLUseProxy",0);
	thisUser->settings.proxyType=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLProxyType",PROXYTYPE_SOCKS5);
	if(thisUser->user.flags&NUF_NOHTTPSOPTION && thisUser->settings.proxyType==PROXYTYPE_HTTPS)
		thisUser->settings.proxyType=PROXYTYPE_HTTP;
	if(!(thisUser->user.flags&(NUF_HTTPCONNS|NUF_HTTPGATEWAY)) && thisUser->settings.proxyType==PROXYTYPE_HTTP) {
		thisUser->settings.useProxy=0;
		thisUser->settings.proxyType=PROXYTYPE_SOCKS5;
	}
	thisUser->settings.szProxyServer=GetNetlibUserSettingString(thisUser->user.szSettingsModule,"NLProxyServer",0);
	thisUser->settings.wProxyPort=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLProxyPort",1080);
	thisUser->settings.useProxyAuth=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLUseProxyAuth",0);
	thisUser->settings.szProxyAuthUser=GetNetlibUserSettingString(thisUser->user.szSettingsModule,"NLProxyAuthUser",0);
	thisUser->settings.szProxyAuthPassword=GetNetlibUserSettingString(thisUser->user.szSettingsModule,"NLProxyAuthPassword",1);
	thisUser->settings.dnsThroughProxy=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLDnsThroughProxy",1);
	thisUser->settings.specifyIncomingPorts=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLSpecifyIncomingPorts",0);
	thisUser->settings.szIncomingPorts=GetNetlibUserSettingString(thisUser->user.szSettingsModule,"NLIncomingPorts",0);
	thisUser->settings.specifyOutgoingPorts=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLSpecifyOutgoingPorts",0);
	thisUser->settings.szOutgoingPorts=GetNetlibUserSettingString(thisUser->user.szSettingsModule,"NLOutgoingPorts",0);
	thisUser->settings.enableUPnP=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLEnableUPnP",1); //default to on
	thisUser->settings.validateSSL=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLValidateSSL",0);

	thisUser->toLog=GetNetlibUserSettingInt(thisUser->user.szSettingsModule,"NLlog",1);

	EnterCriticalSection(&csNetlibUser);
	netlibUser.insert(thisUser);
	LeaveCriticalSection(&csNetlibUser);
	return (INT_PTR)thisUser;
}

static INT_PTR NetlibGetUserSettings(WPARAM wParam,LPARAM lParam)
{
	NETLIBUSERSETTINGS *nlus=(NETLIBUSERSETTINGS*)lParam;
	struct NetlibUser *nlu=(struct NetlibUser*)wParam;

	if(GetNetlibHandleType(nlu)!=NLH_USER || nlus==NULL || nlus->cbSize!=sizeof(NETLIBUSERSETTINGS)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	*nlus=nlu->settings;
	return 1;
}

static INT_PTR NetlibSetUserSettings(WPARAM wParam,LPARAM lParam)
{
	NETLIBUSERSETTINGS *nlus=(NETLIBUSERSETTINGS*)lParam;
	struct NetlibUser *nlu=(struct NetlibUser*)wParam;

	if(GetNetlibHandleType(nlu)!=NLH_USER || nlus==NULL || nlus->cbSize!=sizeof(NETLIBUSERSETTINGS)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	NetlibSaveUserSettingsStruct(nlu->user.szSettingsModule,nlus);
	return 1;
}

void NetlibDoClose(NetlibConnection *nlc, bool noShutdown)
{
	if (nlc->s == INVALID_SOCKET) return;

	NetlibLogf(nlc->nlu, "(%p:%u) Connection closed internal", nlc, nlc->s);
	if (nlc->hSsl)
	{
		if (!noShutdown) si.shutdown(nlc->hSsl);
		si.sfree(nlc->hSsl);
		nlc->hSsl = NULL;
	}
	closesocket(nlc->s);
	nlc->s = INVALID_SOCKET;
}

INT_PTR NetlibCloseHandle(WPARAM wParam, LPARAM)
{
	switch(GetNetlibHandleType(wParam)) {
		case NLH_USER:
		{	struct NetlibUser *nlu=(struct NetlibUser*)wParam;
			int i;
			
			EnterCriticalSection(&csNetlibUser);
			i = netlibUser.getIndex(nlu);
			if (i >= 0) netlibUser.remove(i);
			LeaveCriticalSection(&csNetlibUser);

			NetlibFreeUserSettingsStruct(&nlu->settings);
			mir_free(nlu->user.szSettingsModule);
			mir_free(nlu->user.szDescriptiveName);
			mir_free(nlu->user.szHttpGatewayHello);
			mir_free(nlu->user.szHttpGatewayUserAgent);
			mir_free(nlu->szStickyHeaders);
			break;
		}
		case NLH_CONNECTION:
		{	struct NetlibConnection *nlc=(struct NetlibConnection*)wParam;
			HANDLE waitHandles[4];
			DWORD waitResult;

			WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
			if (nlc->usingHttpGateway)
			{
				HttpGatewayRemovePacket(nlc, -1);
			}
			else
			{
				if(nlc->s != INVALID_SOCKET) {
					NetlibDoClose(nlc);
				}
				if (nlc->s2 != INVALID_SOCKET) closesocket(nlc->s2);
				nlc->s2 = INVALID_SOCKET;
			}
			ReleaseMutex(hConnectionHeaderMutex);

			waitHandles[0]=hConnectionHeaderMutex;
			waitHandles[1]=nlc->hOkToCloseEvent;
			waitHandles[2]=nlc->ncsRecv.hMutex;
			waitHandles[3]=nlc->ncsSend.hMutex;
			waitResult=WaitForMultipleObjects( SIZEOF(waitHandles),waitHandles,TRUE,INFINITE);
			if(waitResult<WAIT_OBJECT_0 || waitResult >= WAIT_OBJECT_0 + SIZEOF(waitHandles)) {
				ReleaseMutex(hConnectionHeaderMutex);
				SetLastError(ERROR_INVALID_PARAMETER);	  //already been closed
				return 0;
			}
			nlc->handleType=0;
			mir_free(nlc->nlhpi.szHttpPostUrl);
			mir_free(nlc->nlhpi.szHttpGetUrl);
			mir_free(nlc->dataBuffer);
			mir_free((char*)nlc->nloc.szHost);
			mir_free(nlc->szNewUrl);
			mir_free(nlc->szProxyServer);
			NetlibDeleteNestedCS(&nlc->ncsRecv);
			NetlibDeleteNestedCS(&nlc->ncsSend);
			CloseHandle(nlc->hOkToCloseEvent);
			DeleteCriticalSection(&nlc->csHttpSequenceNums);
			ReleaseMutex(hConnectionHeaderMutex);
			NetlibLogf(nlc->nlu,"(%p:%u) Connection closed",nlc,nlc->s);
			break;
		}
		case NLH_BOUNDPORT:
			return NetlibFreeBoundPort((struct NetlibBoundPort*)wParam);
		case NLH_PACKETRECVER:
		{	struct NetlibPacketRecver *nlpr=(struct NetlibPacketRecver*)wParam;
			mir_free(nlpr->packetRecver.buffer);
			break;
		}
		default:
			SetLastError(ERROR_INVALID_PARAMETER);
			return 0;
	}
	mir_free((void*)wParam);
	return 1;
}

static INT_PTR NetlibGetSocket(WPARAM wParam, LPARAM)
{
	SOCKET s;
	if(wParam==0) {
		s=INVALID_SOCKET;
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else {
		WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
		switch(GetNetlibHandleType(wParam)) {
			case NLH_CONNECTION:
				s=((struct NetlibConnection*)wParam)->s;
				break;
			case NLH_BOUNDPORT:
				s=((struct NetlibBoundPort*)wParam)->s;
				break;
			default:
				s=INVALID_SOCKET;
				SetLastError(ERROR_INVALID_PARAMETER);
				break;
		}
		ReleaseMutex(hConnectionHeaderMutex);
	}
	return s;
}

INT_PTR NetlibShutdown(WPARAM wParam, LPARAM)
{
	if (wParam) 
	{
		WaitForSingleObject(hConnectionHeaderMutex,INFINITE);
		switch(GetNetlibHandleType(wParam)) {
			case NLH_CONNECTION:
				{
					struct NetlibConnection* nlc = (struct NetlibConnection*)wParam;
					if (nlc->hSsl) si.shutdown(nlc->hSsl);
					if (nlc->s != INVALID_SOCKET) shutdown(nlc->s, 2);
					if (nlc->s2 != INVALID_SOCKET) shutdown(nlc->s2, 2);
					nlc->termRequested = true;
				}
				break;
			case NLH_BOUNDPORT:
				{
					struct NetlibBoundPort* nlb = (struct NetlibBoundPort*)wParam;
					if (nlb->s != INVALID_SOCKET) shutdown(nlb->s, 2);
				}
				break;
		}
		ReleaseMutex(hConnectionHeaderMutex);

	}
	return 0;
}

static const char szHexDigits[]="0123456789ABCDEF";
INT_PTR NetlibHttpUrlEncode(WPARAM,LPARAM lParam)
{
	unsigned char *szOutput,*szInput=(unsigned char*)lParam;
	unsigned char *pszIn,*pszOut;
	int outputLen;

	if(szInput==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return (INT_PTR)(char*)NULL;
	}
	for(outputLen=0,pszIn=szInput;*pszIn;pszIn++) {
		if ( (48 <= *pszIn && *pszIn <= 57) ||//0-9
             (65 <= *pszIn && *pszIn <= 90) ||//ABC...XYZ
             (97 <= *pszIn && *pszIn <= 122) ||//abc...xyz
			 *pszIn == '-' || *pszIn == '_' || *pszIn == '.' || *pszIn == ' ') outputLen++;
		else outputLen+=3;
	}
	szOutput=(unsigned char*)HeapAlloc(GetProcessHeap(),0,outputLen+1);
	if(szOutput==NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return (INT_PTR)(unsigned char*)NULL;
	}
	for(pszOut=szOutput,pszIn=szInput;*pszIn;pszIn++) {
		if ( (48 <= *pszIn && *pszIn <= 57) ||
             (65 <= *pszIn && *pszIn <= 90) ||
             (97 <= *pszIn && *pszIn <= 122) ||
			 *pszIn == '-' || *pszIn == '_' || *pszIn == '.') *pszOut++=*pszIn;
		else if(*pszIn==' ') *pszOut++='+';
		else {
			*pszOut++='%';
			*pszOut++=szHexDigits[*pszIn>>4];
			*pszOut++=szHexDigits[*pszIn&0xF];
		}
	}
	*pszOut='\0';
	return (INT_PTR)szOutput;
}

static const char base64chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
INT_PTR NetlibBase64Encode(WPARAM, LPARAM lParam)
{
	NETLIBBASE64 *nlb64=(NETLIBBASE64*)lParam;
	int iIn;
	char *pszOut;
	PBYTE pbIn;

	if(nlb64==NULL || nlb64->pszEncoded==NULL || nlb64->pbDecoded==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if(nlb64->cchEncoded<Netlib_GetBase64EncodedBufferSize(nlb64->cbDecoded)) {
		SetLastError(ERROR_BUFFER_OVERFLOW);
		return 0;
	}
	nlb64->cchEncoded=Netlib_GetBase64EncodedBufferSize(nlb64->cbDecoded);
	for(iIn=0,pbIn=nlb64->pbDecoded,pszOut=nlb64->pszEncoded;iIn<nlb64->cbDecoded;iIn+=3,pbIn+=3,pszOut+=4) {
		pszOut[0]=base64chars[pbIn[0]>>2];
		if(nlb64->cbDecoded-iIn==1) {
			pszOut[1]=base64chars[(pbIn[0]&3)<<4];
			pszOut[2]='=';
			pszOut[3]='=';
			pszOut+=4;
			break;
		}
		pszOut[1]=base64chars[((pbIn[0]&3)<<4)|(pbIn[1]>>4)];
		if(nlb64->cbDecoded-iIn==2) {
			pszOut[2]=base64chars[(pbIn[1]&0xF)<<2];
			pszOut[3]='=';
			pszOut+=4;
			break;
		}
		pszOut[2]=base64chars[((pbIn[1]&0xF)<<2)|(pbIn[2]>>6)];
		pszOut[3]=base64chars[pbIn[2]&0x3F];
	}
	pszOut[0]='\0';
	return 1;
}

static BYTE Base64CharToInt(char c)
{
	if(c>='A' && c<='Z') return c-'A';
	if(c>='a' && c<='z') return c-'a'+26;
	if(c>='0' && c<='9') return c-'0'+52;
	if(c=='+') return 62;
	if(c=='/') return 63;
	if(c=='=') return 64;
	return 255;
}

INT_PTR NetlibBase64Decode(WPARAM, LPARAM lParam)
{
	NETLIBBASE64 *nlb64=(NETLIBBASE64*)lParam;
	char *pszIn;
	PBYTE pbOut;
	BYTE b1,b2,b3,b4;
	int iIn;

	if(nlb64==NULL || nlb64->pszEncoded==NULL || nlb64->pbDecoded==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if(nlb64->cchEncoded&3) {
		SetLastError(ERROR_INVALID_DATA);
		return 0;
	}
	if(nlb64->cbDecoded<Netlib_GetBase64DecodedBufferSize(nlb64->cchEncoded)) {
		SetLastError(ERROR_BUFFER_OVERFLOW);
		return 0;
	}
	nlb64->cbDecoded=Netlib_GetBase64DecodedBufferSize(nlb64->cchEncoded);
	for(iIn=0,pszIn=nlb64->pszEncoded,pbOut=nlb64->pbDecoded;iIn<nlb64->cchEncoded;iIn+=4,pszIn+=4,pbOut+=3) {
		b1=Base64CharToInt(pszIn[0]);
		b2=Base64CharToInt(pszIn[1]);
		b3=Base64CharToInt(pszIn[2]);
		b4=Base64CharToInt(pszIn[3]);
		if(b1==255 || b1==64 || b2==255 || b2==64 || b3==255 || b4==255) {
			SetLastError(ERROR_INVALID_DATA);
			return 0;
		}
		pbOut[0]=(b1<<2)|(b2>>4);
		if(b3==64) {nlb64->cbDecoded-=2; break;}
		pbOut[1]=(b2<<4)|(b3>>2);
		if(b4==64) {nlb64->cbDecoded--; break;}
		pbOut[2]=b4|(b3<<6);
	}
	return 1;
}

void UnloadNetlibModule(void)
{
	if (!bModuleInitialized) return;

	if (hConnectionHeaderMutex != NULL)
	{
		int i;

		NetlibUnloadIeProxy();
		NetlibSecurityDestroy();
		NetlibUPnPDestroy();
		NetlibLogShutdown();

		DestroyHookableEvent(hRecvEvent); hRecvEvent = NULL;
		DestroyHookableEvent(hSendEvent); hSendEvent = NULL;

		for (i = netlibUser.getCount(); i > 0; i--)
			NetlibCloseHandle((WPARAM)netlibUser[i-1], 0);

		netlibUser.destroy();

		CloseHandle(hConnectionHeaderMutex);
		if (hConnectionOpenMutex) CloseHandle(hConnectionOpenMutex);
		DeleteCriticalSection(&csNetlibUser);
		WSACleanup();
	}	
}

int LoadNetlibModule(void)
{
	WSADATA wsadata;

	bModuleInitialized = TRUE;
	
	WSAStartup(MAKEWORD(2,2), &wsadata);

	HookEvent(ME_OPT_INITIALISE,NetlibOptInitialise);

	InitializeCriticalSection(&csNetlibUser);
	hConnectionHeaderMutex=CreateMutex(NULL,FALSE,NULL);
	NetlibLogInit();

	connectionTimeout = 0;

	OSVERSIONINFOEX osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (GetVersionEx((LPOSVERSIONINFO)&osvi))
	{
		// Connection limiting was introduced in Windows XP SP2 and later and set to 10 / sec
		if (osvi.dwMajorVersion == 5 && ((osvi.dwMinorVersion == 1 && osvi.wServicePackMajor >= 2) || osvi.dwMinorVersion > 1)) 
			connectionTimeout = 150;
		// Connection limiting has limits based on addition Windows Vista pre SP2
		else if (osvi.dwMajorVersion == 6 && osvi.wServicePackMajor < 2)
		{
			DWORD dwType = 0;
			tGetProductInfo pGetProductInfo = (tGetProductInfo) GetProcAddress(GetModuleHandleA("kernel32"), "GetProductInfo");
			if (pGetProductInfo != NULL) pGetProductInfo(6, 0, 0, 0, &dwType);
			switch( dwType )
			{
			case 0x01:  // Vista Ultimate edition have connection limit of 25 / sec - plenty for Miranda
			case 0x1c:
			   break;

			case 0x02:  // Vista Home Basic edition have connection limit of 2 / sec 
			case 0x05:
			   connectionTimeout = 1000;
			   break;
			
			default:    // all other editions have connection limit of 10 / sec
				connectionTimeout = 150;
				break;
			}
		}
		// Connection limiting is disabled by default and is controlled by registry setting in Windows Vista SP2 and later
		else if (osvi.dwMajorVersion >= 6)
		{
			static const char keyn[] = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
			static const char valn[] = "EnableConnectionRateLimiting";

			HKEY hSettings;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyn, 0, KEY_QUERY_VALUE, &hSettings) == ERROR_SUCCESS)
			{
				DWORD tValueLen, enabled;
				tValueLen = sizeof(enabled);
				if (RegQueryValueExA(hSettings, valn, NULL, NULL, (BYTE*)&enabled, &tValueLen) == ERROR_SUCCESS && enabled)
					connectionTimeout = 150;  // if enabled limit is set to 10 / sec
				RegCloseKey(hSettings);
			}

		}
	}

	hConnectionOpenMutex = connectionTimeout ? CreateMutex(NULL,FALSE,NULL) : NULL;
	g_LastConnectionTick = GetTickCount();

	CreateServiceFunction(MS_NETLIB_REGISTERUSER,NetlibRegisterUser);
	CreateServiceFunction(MS_NETLIB_GETUSERSETTINGS,NetlibGetUserSettings);
	CreateServiceFunction(MS_NETLIB_SETUSERSETTINGS,NetlibSetUserSettings);
	CreateServiceFunction(MS_NETLIB_CLOSEHANDLE,NetlibCloseHandle);
	CreateServiceFunction(MS_NETLIB_BINDPORT,NetlibBindPort);
	CreateServiceFunction(MS_NETLIB_OPENCONNECTION,NetlibOpenConnection);
	CreateServiceFunction(MS_NETLIB_SETHTTPPROXYINFO,NetlibHttpGatewaySetInfo);
	CreateServiceFunction(MS_NETLIB_SETSTICKYHEADERS,NetlibHttpSetSticky);
	CreateServiceFunction(MS_NETLIB_GETSOCKET,NetlibGetSocket);
	CreateServiceFunction(MS_NETLIB_URLENCODE,NetlibHttpUrlEncode);
	CreateServiceFunction(MS_NETLIB_BASE64ENCODE,NetlibBase64Encode);
	CreateServiceFunction(MS_NETLIB_BASE64DECODE,NetlibBase64Decode);
	CreateServiceFunction(MS_NETLIB_SENDHTTPREQUEST,NetlibHttpSendRequest);
	CreateServiceFunction(MS_NETLIB_RECVHTTPHEADERS,NetlibHttpRecvHeaders);
	CreateServiceFunction(MS_NETLIB_FREEHTTPREQUESTSTRUCT,NetlibHttpFreeRequestStruct);
	CreateServiceFunction(MS_NETLIB_HTTPTRANSACTION,NetlibHttpTransaction);
	CreateServiceFunction(MS_NETLIB_SEND,NetlibSend);
	CreateServiceFunction(MS_NETLIB_RECV,NetlibRecv);
	CreateServiceFunction(MS_NETLIB_SELECT,NetlibSelect);
	CreateServiceFunction(MS_NETLIB_SELECTEX,NetlibSelectEx);
	CreateServiceFunction(MS_NETLIB_SHUTDOWN,NetlibShutdown);
	CreateServiceFunction(MS_NETLIB_CREATEPACKETRECVER,NetlibPacketRecverCreate);
	CreateServiceFunction(MS_NETLIB_GETMOREPACKETS,NetlibPacketRecverGetMore);
	CreateServiceFunction(MS_NETLIB_SETPOLLINGTIMEOUT,NetlibHttpSetPollingTimeout);
	CreateServiceFunction(MS_NETLIB_STARTSSL,NetlibStartSsl);

	hRecvEvent = CreateHookableEvent(ME_NETLIB_FASTRECV);
	hSendEvent = CreateHookableEvent(ME_NETLIB_FASTSEND);

	NetlibUPnPInit();
	NetlibSecurityInit();
	NetlibLoadIeProxy();

	return 0;
}

void NetlibInitSsl(void)
{
	mir_getSI(&si);
}
