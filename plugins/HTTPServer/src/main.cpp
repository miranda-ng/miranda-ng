//This file is part of HTTPServer a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "Glob.h"

#define szConfigFile        "HTTPServer.xml"

const char szXmlHeader[] = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n"
"<?xml-stylesheet type=\"text/xsl\" href=\"HTTPServer.xsl\"?>\r\n"
"<config>\r\n";

const char szXmlData[] = "\t<share>\r\n"
"\t\t<name>%s</name>\r\n"
"\t\t<file>%s</file>\r\n"
"\t\t<max_downloads>%d</max_downloads>\r\n"
"\t\t<ip_address>%d.%d.%d.%d</ip_address>\r\n"
"\t\t<ip_mask>%d.%d.%d.%d</ip_mask>\r\n"
"\t</share>\r\n";

const char szXmlTail[] = "</config>";

const char* pszDefaultShares[] = {
	"htdocs\\@settings\\favicon.ico",     "/favicon.ico",
	"htdocs\\@settings\\index.xsl",       "/index.xsl",
	"htdocs\\@settings\\theme\\",         "/theme/",
	"htdocs\\",                           "/",
	nullptr, nullptr
};

static IconItem iconList[] =
{
	{ "Disable server", "disable", IDI_DISABLE_SERVER },
	{ "Enable server", "enable", IDI_SHARE_NEW_FILE },
};

int OptionsInitialize(WPARAM, LPARAM);
int PreShutdown(WPARAM, LPARAM);

HNETLIBUSER hNetlibUser;
HANDLE hDirectBoundPort;

string sLogFilePath;

// static so they can not be used from other modules ( sourcefiles )
static HANDLE hEventSystemInit = nullptr;
static HANDLE hPreShutdown = nullptr;

static HANDLE hHttpAcceptConnectionsService = nullptr;
static HANDLE hHttpAddChangeRemoveService = nullptr;
static HANDLE hHttpGetShareService = nullptr;
static HANDLE hHttpGetAllShares = nullptr;

static HGENMENU hAcceptConnectionsMenuItem = nullptr;

char szPluginPath[MAX_PATH] = { 0 };
int nPluginPathLen = 0;

uint32_t dwLocalIpAddress = 0;
uint32_t dwLocalPortUsed = 0;
uint32_t dwExternalIpAddress = 0;

int nMaxUploadSpeed = -1;
int nMaxConnectionsTotal = -1;
int nMaxConnectionsPerUser = -1;
int nDefaultDownloadLimit = -1;

bool bIsOnline = true;

bool bLimitOnlyWhenOnline = true;

bool bShutdownInProgress = false;

CMPlugin g_plugin;

extern HWND hwndStatsticView;

/////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {67848B07-83D2-49E9-8844-7E3DE268E304}
	{0x67848b07, 0x83d2, 0x49e9, {0x88, 0x44, 0x7e, 0x3d, 0xe2, 0x68, 0xe3, 0x4}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////
// Member Function : bOpenLogFile
// Type            : Global
// Parameters      : None
// Returns         : Returns alway true
// Description     : Open the log file in default program assoaitated
//                   with .log files
// References      : -
// Remarks         : -
// Created         : 031020, 20 oktober 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bOpenLogFile()
{
	SHELLEXECUTEINFO st = { 0 };
	st.cbSize = sizeof(st);
	st.fMask = SEE_MASK_INVOKEIDLIST;
	st.hwnd = nullptr;
	st.lpFile = sLogFilePath.c_str();
	st.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx(&st);
	return true;
}


bool bWriteToFile(HANDLE hFile, const char * pszSrc, int nLen = -1)
{
	if (nLen < 0)
		nLen = (int)mir_strlen(pszSrc);
	DWORD dwBytesWritten;
	return WriteFile(hFile, pszSrc, nLen, &dwBytesWritten, nullptr) && (dwBytesWritten == (uint32_t)nLen);
}


/////////////////////////////////////////////////////////////////////
// Member Function : LogEvent
// Type            : Global
// Parameters      : title - ?
//                   logme - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030928, 28 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void LogEvent(const char * pszTitle, const char * pszLog)
{
	HANDLE hFile = CreateFile(sLogFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	if (SetFilePointer(hFile, 0, nullptr, FILE_END) == INVALID_SET_FILE_POINTER) {
		CloseHandle(hFile);
		return;
	}

	char szTmp[128];
	time_t now;
	time(&now);
	int nLen = (int)strftime(szTmp, sizeof(szTmp), "%d-%m-%Y %H:%M:%S -- ", localtime(&now));

	int nLogLen = (int)mir_strlen(pszLog);
	while (nLogLen > 0 && (pszLog[nLogLen - 1] == '\r' || pszLog[nLogLen - 1] == '\n'))
		nLogLen--;

	bWriteToFile(hFile, szTmp, nLen);
	bWriteToFile(hFile, pszTitle);
	bWriteToFile(hFile, " : ");
	bWriteToFile(hFile, pszLog, nLogLen);
	bWriteToFile(hFile, "\r\n");
	CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////
// Member Function : dwReadIPAddress
// Type            : Global
// Parameters      : pszStr - ?
// Returns         : uint32_t
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

uint32_t dwReadIPAddress(char * pszStr, bool &bError)
{
	uint32_t ip = 0;
	char *pszEnd;
	for (int n = 0; n < 4; n++) {
		int nVal = strtol(pszStr, &pszEnd, 10);
		if (pszEnd <= pszStr || (n != 3 && pszEnd[0] != '.') || (nVal < 0 || nVal > 255)) {
			bError = true;
			return 0;
		}
		pszStr = pszEnd + 1;
		ip |= nVal << ((3 - n) * 8);
	}
	return ip;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bReadConfigurationFile
// Type            : Global
// Parameters      : None
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030823, 23 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bReadConfigurationFile()
{
	mir_cslock lck(csFileShareListAccess);

	CLFileShareNode * pclLastNode = nullptr;

	char szBuf[1000];
	mir_strcpy(szBuf, szPluginPath);
	mir_strcat(szBuf, szConfigFile);
	HANDLE hFile = CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	char *pszCurPos = szBuf;

	bool bEof = false;
	while (!bEof) {
		uint32_t dwBytesInBuffer = 0;

		// move rest of buffer to front
		if (pszCurPos && pszCurPos != szBuf) {
			dwBytesInBuffer = uint32_t(sizeof(szBuf) - (pszCurPos - szBuf));
			memmove(szBuf, pszCurPos, dwBytesInBuffer * sizeof(wchar_t));
		}

		// append data to buffer
		DWORD dwBytesRead = 0;
		bEof = !ReadFile(hFile, &szBuf[dwBytesInBuffer], sizeof(szBuf) - dwBytesInBuffer,
			&dwBytesRead, nullptr) || dwBytesRead <= 0;
		pszCurPos = szBuf;

		if (pszCurPos) {
			while (pszCurPos && (pszCurPos = strstr(pszCurPos, "<share>")) != nullptr) {
				pszCurPos += 7;

				char * pszColData[5] = {};
				for (int n = 0; n < 5; n++) {
					pszColData[n] = strstr(pszCurPos, ">");
					if (!pszColData[n])
						break;
					pszColData[n] += 1;

					pszCurPos = strstr(pszColData[n], "</");
					if (!pszCurPos)
						break;

					pszCurPos[0] = 0;// NULL terminate row data, we overwrite the '<'
					pszCurPos = strstr(pszCurPos + 3, ">");

					if (!pszCurPos)
						break;
					pszCurPos += 1;
				}
				if (!pszColData[4])
					continue;

				CLFileShareNode * pcl = new CLFileShareNode(pszColData[0], pszColData[1]);

				bool bError = false;
				char * pszEnd;

				pcl->st.nMaxDownloads = strtol(pszColData[2], &pszEnd, NULL);
				if (!pszEnd || *pszEnd != NULL)
					bError = true;

				pcl->st.dwAllowedIP = dwReadIPAddress(pszColData[3], bError);
				pcl->st.dwAllowedMask = dwReadIPAddress(pszColData[4], bError);
				if (bError) {
					delete pcl;
					continue;
				}

				if (!pclLastNode) {
					pclLastNode = pclFirstNode = pcl;
				}
				else {
					pclLastNode->pclNext = pcl;
					pclLastNode = pcl;
				}

				// refill buffer
				if (!bEof && pszCurPos - szBuf > sizeof(szBuf) / 2)
					break;
			}
		}
	}
	CloseHandle(hFile);
	return true;
}


/////////////////////////////////////////////////////////////////////
// Member Function : bWriteConfigurationFile
// Type            : Global
// Parameters      : None
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bWriteConfigurationFile()
{
	mir_cslock lck(csFileShareListAccess);
	char szBuf[1000];
	mir_strcpy(szBuf, szPluginPath);
	mir_strcat(szBuf, szConfigFile);
	HANDLE hFile = CreateFile(szBuf, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwBytesWriten = 0;
	if (WriteFile(hFile, szXmlHeader, sizeof(szXmlHeader) - 1, &dwBytesWriten, nullptr)) {
		CLFileShareNode *pclCur = pclFirstNode;
		while (pclCur) {
			uint32_t dwBytesToWrite = mir_snprintf(szBuf, szXmlData,
				pclCur->st.pszSrvPath,
				pclCur->pszOrigRealPath,
				pclCur->st.nMaxDownloads,
				SplitIpAddress(pclCur->st.dwAllowedIP),
				SplitIpAddress(pclCur->st.dwAllowedMask));

			WriteFile(hFile, szBuf, dwBytesToWrite, &dwBytesWriten, nullptr);
			pclCur = pclCur->pclNext;
		}

		WriteFile(hFile, szXmlTail, sizeof(szXmlTail) - 1, &dwBytesWriten, nullptr);
	}
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return true;
}


/////////////////////////////////////////////////////////////////////
// Member Function : nAddChangeRemoveShare
// Type            : Global
// Parameters      : wParam - Contact handle
//                   lParam - ?
// Returns         : static int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN, Houdini, changed By Sérgio Vieira Rolanski
/////////////////////////////////////////////////////////////////////

static INT_PTR nAddChangeRemoveShare(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return 1001;

	STFileShareInfo * pclNew = (STFileShareInfo*)lParam;

	// make the server path lowercase
	char* pszPos = pclNew->pszSrvPath;
	while (*pszPos) {
		*pszPos = (char)tolower(*pszPos);
		pszPos++;
	}

	if (pclNew->lStructSize != sizeof(STFileShareInfo))
		return 1002;

	mir_cslockfull lck(csFileShareListAccess);
	bool bIsDirectory = (pclNew->pszSrvPath[mir_strlen(pclNew->pszSrvPath) - 1] == '/');

	CLFileShareNode **pclPrev = &pclFirstNode;
	CLFileShareNode * pclCur = pclFirstNode;

	// insert files after directories
	if (!bIsDirectory) {
		while (pclCur && pclCur->bIsDirectory()) {
			pclPrev = &pclCur->pclNext;
			pclCur = pclCur->pclNext;
		}
	}

	while (pclCur) {
		if (_stricmp(pclCur->st.pszSrvPath, pclNew->pszSrvPath) == 0) {
			if (pclCur->bAnyUsers()) {
				// Some downloads are in progress we will try an terminate them !!
				// we try for 5 sec.
				pclCur->CloseAllTransfers();
				int nTryCount = 0;
				do {
					nTryCount++;
					if (nTryCount >= 100)
						return 1004;
					lck.unlock();
					Sleep(50);
					lck.lock();
				} while (pclCur->bAnyUsers());
			}

			if (!pclNew->pszRealPath || pclNew->pszRealPath[0] == 0) {
				// remove this one
				*pclPrev = pclCur->pclNext;
				delete pclCur;
			}
			else {
				// update info !!
				if (!pclCur->bSetInfo(pclNew))
					return 1003;
			}
			return !bWriteConfigurationFile();
		}
		pclPrev = &pclCur->pclNext;
		pclCur = pclCur->pclNext;
	}

	// Node was not found we will add a new one.
	CLFileShareNode* pclNewNode = new CLFileShareNode(pclNew);
	pclNewNode->pclNext = *pclPrev;
	*pclPrev = pclNewNode;

	/* Add by Sérgio Vieira Rolanski */
	if (pclNew->dwOptions & OPT_SEND_LINK)
		SendLinkToUser(wParam, pclNew->pszSrvPath);

	return !bWriteConfigurationFile();
}

/////////////////////////////////////////////////////////////////////
// Member Function : nGetShare
// Type            : Global
// Parameters      : WPARAM - ?
//                   lParam - ?
// Returns         : static int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031011, 11 oktober 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

static INT_PTR nGetShare(WPARAM /*wParam*/, LPARAM lParam)
{
	if (!lParam)
		return 1001;

	mir_cslock lck(csFileShareListAccess);

	STFileShareInfo * pclShare = (STFileShareInfo*)lParam;
	CLFileShareNode * pclCur = pclFirstNode;
	while (pclCur) {
		if (mir_strcmp(pclCur->st.pszSrvPath, pclShare->pszSrvPath) == 0) {
			if (pclShare->dwMaxRealPath <= mir_strlen(pclCur->st.pszRealPath) + 1)
				return 1003;
			mir_strcpy(pclShare->pszRealPath, pclCur->st.pszRealPath);
			pclShare->dwAllowedIP = pclCur->st.dwAllowedIP;
			pclShare->dwAllowedMask = pclCur->st.dwAllowedMask;
			pclShare->nMaxDownloads = pclCur->st.nMaxDownloads;
			pclShare->dwOptions = pclCur->st.dwOptions;
			return 0;
		}
		pclCur = pclCur->pclNext;
	}
	return 1002;
}


/////////////////////////////////////////////////////////////////////
// Member Function : nAddChangeRemoveShare
// Type            : Global
// Parameters      : WPARAM - ?
//                   lParam - ?
// Returns         : static int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030903, 03 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

static INT_PTR nHttpGetAllShares(WPARAM /*wParam*/, LPARAM /*lParam*/)
{/*
 STFileShareInfo ** pTarget = (STFileShareInfo**)lParam;
 CLFileShareNode * pclCur;

 CLFileShareListAccess clCritSection;

 int nShareCount = 0;
 for( pclCur = pclFirstNode; pclCur ; pclCur = pclCur->pclNext)
 nShareCount++;

 int nCount = 0;
 *pTarget = (STFileShareInfo*)MirandaMalloc( sizeof( STFileShareInfo) * nShareCount );
 for( pclCur = pclFirstNode; pclCur ; pclCur = pclCur->pclNext)
 {
 // this code is not good !!!
 // we need to alloc and copy psz strings also !!
 memcpy( &((*pTarget)[nCount]), pclCur, sizeof( STFileShareInfo));
 nCount++;
 }
 */
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : HandleNewConnection
// Type            : Global
// Parameters      : ch - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030903, 03 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void __cdecl HandleNewConnection(CLHttpUser *pclUser)
{
	pclUser->HandleNewConnection();
	delete pclUser;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ConnectionOpen
// Type            : Global
// Parameters      : hNewConnection - ?
//                   dwRemoteIP     - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030813, 13 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void ConnectionOpen(HNETLIBCONN hNewConnection, uint32_t dwRemoteIP, void*)
{
	in_addr stAddr;
	stAddr.S_un.S_addr = htonl(dwRemoteIP);

	CLHttpUser *pclUser = new CLHttpUser(hNewConnection, stAddr);
	mir_forkThread<CLHttpUser>(HandleNewConnection, pclUser);
}

/////////////////////////////////////////////////////////////////////
// Member Function : nProtoAck
// Type            : Global
// Parameters      : wParam - ?
//                   lParam - ?
// Returns         : static int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031213, 13 december 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

static int nProtoAck(WPARAM /*wParam*/, LPARAM lParam)
{
	//todo: ignore weather protos
	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->type != ACKTYPE_STATUS ||                       //only send for statuses
		ack->result != ACKRESULT_SUCCESS)                  //only successful ones
		return 0;

	bIsOnline = ((int)ack->lParam != ID_STATUS_AWAY && (int)ack->lParam != ID_STATUS_NA);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nToggelAcceptConnections
// Type            : Global
// Parameters      : WPARAM - ?
//                   LPARAM - ?
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031011, 11 oktober 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

INT_PTR nToggelAcceptConnections(WPARAM wparam, LPARAM /*lparam*/)
{
	if (!hDirectBoundPort) {
		NETLIBUSERSETTINGS nus = {};
		nus.cbSize = sizeof(nus);
		Netlib_GetUserSettings(hNetlibUser, &nus);

		NETLIBBIND nlb = {};
		nlb.pfnNewConnection = ConnectionOpen;
		if (nus.specifyIncomingPorts && nus.szIncomingPorts && nus.szIncomingPorts[0])
			nlb.wPort = 0;
		else
			nlb.wPort = 80;

		hDirectBoundPort = Netlib_BindPort(hNetlibUser, &nlb);
		if (!hDirectBoundPort) {
			char szTemp[200];
			mir_snprintf(szTemp, Translate("Failed to bind to port %s\r\nThis is most likely because another program or service is using this port"),
				nlb.wPort == 80 ? "80" : nus.szIncomingPorts);
			MessageBox(nullptr, szTemp, MSG_BOX_TITLE, MB_OK);
			return 1001;
		}
		dwLocalPortUsed = nlb.wPort;
		dwLocalIpAddress = nlb.dwInternalIP;

		Menu_ModifyItem(hAcceptConnectionsMenuItem, LPGENW("Disable HTTP server"), iconList[0].hIcolib);
	}
	else if (hDirectBoundPort && wparam == 0) {
		Netlib_CloseHandle(hDirectBoundPort);
		hDirectBoundPort = nullptr;
		Menu_ModifyItem(hAcceptConnectionsMenuItem, LPGENW("Enable HTTP server"), iconList[1].hIcolib);
	}
	else return 0; // no changes;

	if (!bShutdownInProgress)
		g_plugin.setByte("AcceptConnections", hDirectBoundPort != nullptr);

	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : MainInit
// Type            : Global
// Parameters      : wparam - ?
//                   lparam - ?
// Returns         : int
// Description     : Called when system modules has been loaded
//
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int MainInit(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	if (!bReadConfigurationFile()) {
		char szRealPath[MAX_PATH];
		char szSrvPath[MAX_PATH] = { 0 };
		STFileShareInfo share;

		const char** p = pszDefaultShares;
		while (*p) {
			memset(&share, 0, sizeof(share));
			share.lStructSize = sizeof(share);
			share.dwAllowedIP = 0;
			share.dwAllowedMask = 0;
			share.nMaxDownloads = -1;

			share.pszRealPath = szRealPath;
			share.dwMaxRealPath = sizeof(szRealPath);
			mir_strcpy(share.pszRealPath, p[0]);

			share.pszSrvPath = szSrvPath;
			share.dwMaxSrvPath = sizeof(szSrvPath);
			mir_strcpy(share.pszSrvPath, p[1]);

			if (CallService(MS_HTTP_ADD_CHANGE_REMOVE, 0, (LPARAM)&share))
				break;

			p += 2;
		}

		bWriteConfigurationFile();
	}

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.a = Translate("HTTP Server");
	hNetlibUser = Netlib_RegisterUser(&nlu);

	if (g_plugin.getByte("AcceptConnections", 1))
		nToggelAcceptConnections(0, 0);

	InitGuiElements();

	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : PreShutdown
// Type            : Global
// Parameters      : WPARAM - ?
//                   LPARAM - ?
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030811, 11 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int PreShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	bShutdownInProgress = true;
	{
		mir_cslock lck(csFileShareListAccess);

		for (CLFileShareNode * pclCur = pclFirstNode; pclCur; pclCur = pclCur->pclNext)
			pclCur->CloseAllTransfers();
	}

	if (hDirectBoundPort)
		nToggelAcceptConnections(0, 0);

	Netlib_CloseHandle(hNetlibUser);

	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nSystemShutdown
// Type            : Global
// Parameters      : wparam - 0
//                   lparam - 0
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020428, 28 April 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int nSystemShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	while (pclFirstNode) {
		CLFileShareNode * pclCur = pclFirstNode;
		pclFirstNode = pclFirstNode->pclNext;
		delete pclCur;
	}
	pclFirstNode = nullptr;

	g_plugin.setByte("IndexCreationMode", (uint8_t)indexCreationMode);
	FreeIndexHTMLTemplate();
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Load
// Type            : Global
// Parameters      : link - ?
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	g_plugin.registerIcon(MSG_BOX_TITLE, iconList, MODULENAME);

	hHttpAcceptConnectionsService = CreateServiceFunction(MS_HTTP_ACCEPT_CONNECTIONS, nToggelAcceptConnections);
	hHttpAddChangeRemoveService = CreateServiceFunction(MS_HTTP_ADD_CHANGE_REMOVE, nAddChangeRemoveShare);
	hHttpGetShareService = CreateServiceFunction(MS_HTTP_GET_SHARE, nGetShare);
	hHttpGetAllShares = CreateServiceFunction(MS_HTTP_GET_ALL_SHARES, nHttpGetAllShares);

	hEventSystemInit = HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	Profile_GetPathA(MAX_PATH, szPluginPath);
	mir_strncat(szPluginPath, "\\HTTPServer\\", _countof(szPluginPath) - mir_strlen(szPluginPath));
	int err = CreateDirectoryTree(szPluginPath);
	if ((err != 0) && (err != ERROR_ALREADY_EXISTS)) {
		MessageBoxW(nullptr, L"Failed to create HTTPServer directory.", TranslateW(_A2W(MSG_BOX_TITLE)), MB_OK);
		return 1;
	}

	nPluginPathLen = (int)mir_strlen(szPluginPath);

	sLogFilePath = szPluginPath;
	sLogFilePath += "HTTPServer.log";

	if (!bInitMimeHandling())
		MessageBoxW(nullptr, L"Failed to read configuration file : " szMimeTypeConfigFile, TranslateW(_A2W(MSG_BOX_TITLE)), MB_OK);

	nMaxUploadSpeed = g_plugin.getDword("MaxUploadSpeed", nMaxUploadSpeed);
	nMaxConnectionsTotal = g_plugin.getDword("MaxConnectionsTotal", nMaxConnectionsTotal);
	nMaxConnectionsPerUser = g_plugin.getDword("MaxConnectionsPerUser", nMaxConnectionsPerUser);
	bLimitOnlyWhenOnline = g_plugin.getByte("LimitOnlyWhenOnline", bLimitOnlyWhenOnline) != 0;
	indexCreationMode = (eIndexCreationMode)g_plugin.getByte("IndexCreationMode", 2);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xf0a68784, 0xc30e, 0x4245, 0xb6, 0x2b, 0xb8, 0x71, 0x7e, 0xe6, 0xe1, 0x73);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SHARE_NEW_FILE);
	mi.position = 1000085000;
	mi.name.a = LPGEN("Enable HTTP server");
	mi.pszService = MS_HTTP_ACCEPT_CONNECTIONS;
	hAcceptConnectionsMenuItem = Menu_AddMainMenuItem(&mi);

	if (indexCreationMode == INDEX_CREATION_HTML || indexCreationMode == INDEX_CREATION_DETECT)
		if (!LoadIndexHTMLTemplate()) {
			indexCreationMode = INDEX_CREATION_DISABLE;
			g_plugin.setByte("IndexCreationMode", (uint8_t)indexCreationMode);
		}

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_PROTO_ACK, nProtoAck);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Unload
// Type            : Global
// Parameters      : none
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	nSystemShutdown(0, 0);
	if (hwndStatsticView)
		DestroyWindow(hwndStatsticView);
	return 0;
}
