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

#define szConfigFile        _T("HTTPServer.xml")

const char szXmlHeader[] =	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n"
    "<?xml-stylesheet type=\"text/xsl\" href=\"HTTPServer.xsl\"?>\r\n"
    "<config>\r\n";

const TCHAR szXmlData[] =	_T("\t<share>\r\n")
    _T("\t\t<name>%s</name>\r\n")
	_T("\t\t<file>%s</file>\r\n")
	_T("\t\t<max_downloads>%d</max_downloads>\r\n")
	_T("\t\t<ip_address>%d.%d.%d.%d</ip_address>\r\n")
	_T("\t\t<ip_mask>%d.%d.%d.%d</ip_mask>\r\n")
	_T("\t</share>\r\n");

const char szXmlTail[] =	"</config>";

const TCHAR* pszDefaultShares[] = {
	_T("htdocs\\@settings\\favicon.ico"), _T("/favicon.ico"),
	_T("htdocs\\@settings\\index.xsl"), _T("/index.xsl"),
	_T("htdocs\\@settings\\theme\\"), _T("/theme/"),
	_T("htdocs\\"), _T("/"),
	0, 0
};

void ConnectionOpen(HANDLE hNewConnection, DWORD dwRemoteIP);
int PreShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/);

HANDLE hNetlibUser;
HANDLE hDirectBoundPort;

HINSTANCE hInstance = NULL;

tstring sLogFilePath;

// static so they can not be used from other modules ( sourcefiles )
static HANDLE hEventSystemInit = 0;
static HANDLE hPreShutdown = 0;

static HANDLE hHttpAcceptConnectionsService = 0;
static HANDLE hHttpAddChangeRemoveService = 0;
static HANDLE hHttpGetShareService = 0;
static HANDLE hHttpGetAllShares = 0;

static HGENMENU hAcceptConnectionsMenuItem = 0;

TCHAR szPluginPath[MAX_PATH] = {0};
int nPluginPathLen = 0;

DWORD dwLocalIpAddress = 0;
DWORD dwLocalPortUsed = 0;
DWORD dwExternalIpAddress = 0;

int nMaxUploadSpeed = -1;
int nMaxConnectionsTotal = -1;
int nMaxConnectionsPerUser = -1;
int nDefaultDownloadLimit = -1;

bool bIsOnline = true;
static HANDLE hEventProtoAck = 0;

bool bLimitOnlyWhenOnline = true;

bool bShutdownInProgress = false;

int hLangpack = 0;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {67848B07-83D2-49E9-8844-7E3DE268E304}
	{0x67848b07, 0x83d2, 0x49e9, {0x88, 0x44, 0x7e, 0x3d, 0xe2, 0x68, 0xe3, 0x4}}
};



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

bool bOpenLogFile() {
	SHELLEXECUTEINFO st = {0};
	st.cbSize = sizeof(st);
	st.fMask = SEE_MASK_INVOKEIDLIST;
	st.hwnd = NULL;
	st.lpFile = sLogFilePath.c_str();
	st.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx(&st);
	return true;
}


bool bWriteToFile(HANDLE hFile, const TCHAR *pszSrc, int nLen = -1)
{
	if (nLen < 0)
		nLen = (int)_tcslen(pszSrc);
	DWORD dwBytesWritten;
	return WriteFile(hFile, pszSrc, nLen, &dwBytesWritten, NULL) && (dwBytesWritten == (DWORD)nLen);
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

void LogEvent(const TCHAR *pszTitle, const TCHAR *pszLog)
{
	HANDLE hFile = CreateFile(sLogFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, TranslateT("Failed to open or create log file"), MSG_BOX_TITEL, MB_OK);
		return;
	}
	if (SetFilePointer(hFile, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER) {
		MessageBox(NULL, TranslateT("Failed to move to the end of the log file"), MSG_BOX_TITEL, MB_OK);
		CloseHandle(hFile);
		return;
	}

	TCHAR szTmp[128];
	time_t now;
	time(&now);
	int nLen = (int)_tcsftime(szTmp, SIZEOF(szTmp), _T("%d-%m-%Y %H:%M:%S -- "), localtime(&now));

	int nLogLen = (int)_tcslen(pszLog);
	while (nLogLen > 0 && (pszLog[nLogLen-1] == '\r' || pszLog[nLogLen-1] == '\n'))
		nLogLen--;

	if (!bWriteToFile(hFile, szTmp, nLen) ||
	    !bWriteToFile(hFile, pszTitle) ||
	    !bWriteToFile(hFile, _T(" : ")) ||
	    !bWriteToFile(hFile, pszLog, nLogLen) ||
	    !bWriteToFile(hFile, _T("\r\n"))) {
		MessageBox(NULL, TranslateT("Failed to write some part of the log file"), MSG_BOX_TITEL, MB_OK);
	}
	CloseHandle(hFile);
}



/////////////////////////////////////////////////////////////////////
// Member Function : dwReadIPAddress
// Type            : Global
// Parameters      : pszStr - ?
// Returns         : DWORD
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

DWORD dwReadIPAddress(TCHAR *pszStr, bool &bError)
{
	DWORD ip = 0;
	TCHAR *pszEnd;
	for (int n = 0 ; n < 4 ; n++) {
		int nVal = _tcstol(pszStr, &pszEnd, 10);
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
	CLFileShareListAccess clCritSection;

	CLFileShareNode * pclLastNode = NULL;

	TCHAR szBuf[1000];
	_tcscpy(szBuf, szPluginPath);
	_tcscat(szBuf, szConfigFile);
	HANDLE hFile = CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	
	TCHAR *pszCurPos = szBuf;

	bool bEof = false;
	while (!bEof) {
		DWORD dwBytesInBuffer = 0;
		
		// move rest of buffer to front
		if (pszCurPos && pszCurPos != szBuf) {
			dwBytesInBuffer = DWORD(SIZEOF(szBuf) - (pszCurPos - szBuf));
			memmove(szBuf, pszCurPos, dwBytesInBuffer);
		}
		
		// append data to buffer
		DWORD dwBytesRead = 0;
		bEof = !ReadFile(hFile, &szBuf[dwBytesInBuffer], SIZEOF(szBuf) - dwBytesInBuffer, 
			&dwBytesRead, NULL) || dwBytesRead <= 0;
		pszCurPos = szBuf;

		if (pszCurPos) {
			while (pszCurPos && (pszCurPos = _tcsstr(pszCurPos, _T("<share>"))) != NULL) {
				pszCurPos += 7;

				TCHAR *pszColData[5] = {NULL};
				for (int n = 0 ; n < 5 ; n++) {
					pszColData[n] = _tcsstr(pszCurPos, _T(">"));
					if (!pszColData[n])
						break;
					pszColData[n] += 1;

					pszCurPos = _tcsstr(pszColData[n], _T("</"));
					if (!pszCurPos)
						break;

					pszCurPos[0] = 0;// NULL terminate row data, we overwrite the '<'
					pszCurPos = _tcsstr(pszCurPos + 3, _T(">"));

					if (!pszCurPos)
						break;
					pszCurPos += 1;
				}
				if (!pszColData[4])
					continue;

				CLFileShareNode * pcl = new CLFileShareNode(pszColData[0], pszColData[1]);

				bool bError = false;
				TCHAR * pszEnd;

				pcl->st.nMaxDownloads = _tcstol(pszColData[2], &pszEnd, NULL);
				if (!pszEnd || *pszEnd != NULL)
					bError = true;

				pcl->st.dwAllowedIP = dwReadIPAddress(pszColData[3], bError);
				pcl->st.dwAllowedMask = dwReadIPAddress(pszColData[4], bError);
				if (bError) {
					delete pcl;
					continue;
				}

				if (! pclLastNode) {
					pclLastNode = pclFirstNode = pcl;
				} else {
					pclLastNode->pclNext = pcl;
					pclLastNode = pcl;
				}
				
				// refill buffer
				if (!bEof && pszCurPos - szBuf > SIZEOF(szBuf) / 2)
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
	CLFileShareListAccess clCritSection;
	TCHAR szBuf[1000];
	_tcscpy(szBuf, szPluginPath);
	_tcscat(szBuf, szConfigFile);
	HANDLE hFile = CreateFile(szBuf, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		TCHAR temp[200];
		mir_sntprintf(temp, SIZEOF(temp), _T("%s%s"), TranslateT("Failed to open or create file "), szConfigFile);
		MessageBox(NULL, temp, MSG_BOX_TITEL, MB_OK);
		return false;
	}

	DWORD dwBytesWriten = 0;
	if (! WriteFile(hFile, szXmlHeader, SIZEOF(szXmlHeader) - 1, &dwBytesWriten, NULL)) {
		TCHAR temp[200];
		mir_sntprintf(temp, SIZEOF(temp), _T("%s%s"), TranslateT("Failed to write xml header to file "), szConfigFile);
		MessageBox(NULL, temp, MSG_BOX_TITEL, MB_OK);
	} else {
		CLFileShareNode * pclCur = pclFirstNode;
		while (pclCur) {
			DWORD dwBytesToWrite = mir_sntprintf(szBuf, SIZEOF(szBuf), szXmlData,
			    pclCur->st.pszSrvPath,
			    pclCur->pszOrigRealPath,
			    pclCur->st.nMaxDownloads,
			    SplitIpAddress(pclCur->st.dwAllowedIP),
			    SplitIpAddress(pclCur->st.dwAllowedMask));

			if (! WriteFile(hFile, szBuf, dwBytesToWrite, &dwBytesWriten, NULL)) {
				TCHAR temp[200];
				mir_sntprintf(temp, SIZEOF(temp), _T("%s%s"), TranslateT("Failed to write xml data to file "), szConfigFile);
				MessageBox(NULL, temp, MSG_BOX_TITEL, MB_OK);
				break;
			}
			pclCur = pclCur->pclNext;
		}

		if (! WriteFile(hFile, szXmlTail, SIZEOF(szXmlTail) - 1, &dwBytesWriten, NULL)) {
				TCHAR temp[200];
				mir_sntprintf(temp, SIZEOF(temp), _T("%s%s"), TranslateT("Failed to write xml tail to file "), szConfigFile);
				MessageBox(NULL, temp, MSG_BOX_TITEL, MB_OK);
		}
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
// Developer       : KN, Houdini, changed By Sergio Vieira Rolanski
/////////////////////////////////////////////////////////////////////

static INT_PTR nAddChangeRemoveShare(WPARAM wParam, LPARAM lParam) {
	if (!lParam)
		return 1001;

	STFileShareInfo * pclNew = (STFileShareInfo*)lParam;

	// make the server path lowercase
	TCHAR *pszPos = pclNew->pszSrvPath;
	while (*pszPos) {
		*pszPos = tolower(*pszPos);
		pszPos++;
	}

	if (pclNew->lStructSize != sizeof(STFileShareInfo))
		return 1002;

	CLFileShareListAccess clCritSection;
	bool bIsDirectory = (pclNew->pszSrvPath[_tcslen(pclNew->pszSrvPath)-1] == '/');

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
		if (_tcsicmp(pclCur->st.pszSrvPath, pclNew->pszSrvPath) == 0) {
			if (pclCur->bAnyUsers()) {
				// Some downloads are in progress we will try an terminate them !!
				// we try for 5 sec.
				pclCur->CloseAllTransfers();
				int nTryCount = 0;
				do {
					nTryCount++;
					if (nTryCount >= 100)
						return 1004;
					clCritSection.Unlock();
					Sleep(50);
					clCritSection.Lock();
				} while (pclCur->bAnyUsers());
			}

			if (!pclNew->pszRealPath || pclNew->pszRealPath[0] == 0) {
				// remove this one
				*pclPrev = pclCur->pclNext;
				delete pclCur;
			} else {
				// update info !!
				if (! pclCur->bSetInfo(pclNew))
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

static INT_PTR nGetShare(WPARAM /*wParam*/, LPARAM lParam) {
	if (!lParam)
		return 1001;

	CLFileShareListAccess clCritSection;

	STFileShareInfo * pclShare = (STFileShareInfo*)lParam;
	CLFileShareNode * pclCur = pclFirstNode;
	while (pclCur) {
		if (_tcscmp(pclCur->st.pszSrvPath, pclShare->pszSrvPath) == 0) {
			if (pclShare->dwMaxRealPath <= _tcslen(pclCur->st.pszRealPath) + 1)
				return 1003;
			_tcscpy(pclShare->pszRealPath, pclCur->st.pszRealPath);
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

static INT_PTR nHttpGetAllShares(WPARAM /*wParam*/, LPARAM /*lParam*/) {/*
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

void HandleNewConnection(void *ch) {
	CLHttpUser * pclUser = (CLHttpUser *)ch;
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

void ConnectionOpen(HANDLE hNewConnection, DWORD dwRemoteIP) {
	in_addr stAddr;
	stAddr.S_un.S_addr = htonl(dwRemoteIP);

	CLHttpUser * pclUser = new CLHttpUser(hNewConnection, stAddr);
	forkthread(HandleNewConnection, 0, (void*)pclUser);
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

static int nProtoAck(WPARAM /*wParam*/, LPARAM lParam) {
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

INT_PTR nToggelAcceptConnections(WPARAM wparam, LPARAM /*lparam*/) {
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR | CMIM_NAME | CMIM_ICON;

	if (!hDirectBoundPort) {
		NETLIBUSERSETTINGS nus = { 0 };
		nus.cbSize = sizeof(nus);
		if (! CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM) hNetlibUser, (LPARAM) &nus)) {
			Netlib_LogfT(hNetlibUser, _T("Failed to get NETLIBUSERSETTINGS using MS_NETLIB_GETUSERSETTINGS"));
		}

		NETLIBBIND nlb = { 0 };
		nlb.cbSize = sizeof(NETLIBBIND);
		nlb.pfnNewConnection = ConnectionOpen;
		if (nus.specifyIncomingPorts && nus.szIncomingPorts && nus.szIncomingPorts[0])
			nlb.wPort = 0;
		else {
			//MessageBox( NULL, "Debug message using default port!", MSG_BOX_TITEL, MB_OK );
			nlb.wPort = 80;
		}
		hDirectBoundPort = (HANDLE) CallService(MS_NETLIB_BINDPORT, (WPARAM) hNetlibUser, (LPARAM) & nlb);
		if (!hDirectBoundPort) {
			TCHAR szTemp[200];
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Failed to bind to port %s\r\nThis is most likely because another program or service is using this port") ,
			    nlb.wPort == 80 ? _T("80") : _A2T(nus.szIncomingPorts));
			MessageBox(NULL, szTemp, MSG_BOX_TITEL, MB_OK);
			return 1001;
		}
		dwLocalPortUsed = nlb.wPort;
		dwLocalIpAddress = nlb.dwInternalIP;

		mi.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISABLE_SERVER));
		mi.ptszName = LPGENT("Disable HTTP server");
		Netlib_Logf(hNetlibUser, mi.pszName);
	} else if (hDirectBoundPort && wparam == 0) {
		Netlib_CloseHandle(hDirectBoundPort);
		hDirectBoundPort = 0;
		mi.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHARE_NEW_FILE));
		mi.ptszName = LPGENT("Enable HTTP server");
		Netlib_Logf(hNetlibUser, mi.pszName);
	} else {
		return 0; // no changes;
	}

	if (hAcceptConnectionsMenuItem)
		Menu_ModifyItem(hAcceptConnectionsMenuItem, &mi);

	if (! bShutdownInProgress)
		db_set_b(NULL, MODULE, "AcceptConnections", hDirectBoundPort != 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : DllMain
// Type            : Global
// Parameters      : hinst       - ?
//                   fdwReason   - ?
//                   lpvReserved - ?
// Returns         : BOOL WINAPI
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD /*fdwReason*/, LPVOID /*lpvReserved*/) {
	hInstance = hinst;
	return 1;
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

int MainInit(WPARAM /*wparam*/, LPARAM /*lparam*/) {
	/*
	STFileShareInfo * pShares = (STFileShareInfo *)5;
	CallService(MS_HTTP_GET_ALL_SHARES, 0, (LPARAM) &pShares);
	MirandaFree( pShares );*/

	if (! bReadConfigurationFile()) {
		//MessageBox( NULL, "Failed to read configuration file : " szConfigFile, MSG_BOX_TITEL, MB_OK );

		TCHAR szRealPath[MAX_PATH];
		TCHAR szSrvPath[MAX_PATH] = {0};
		STFileShareInfo share = { 0 };

		const TCHAR** p = pszDefaultShares;
		while (*p) {
			share.lStructSize = sizeof(share);
			share.dwAllowedIP = 0;
			share.dwAllowedMask = 0;
			share.nMaxDownloads = -1;

			share.pszRealPath = szRealPath;
			share.dwMaxRealPath = SIZEOF(szRealPath);
			_tcscpy(share.pszRealPath, p[0]);

			share.pszSrvPath = szSrvPath;
			share.dwMaxSrvPath = SIZEOF(szSrvPath);
			_tcscpy(share.pszSrvPath, p[1]);

			if (CallService(MS_HTTP_ADD_CHANGE_REMOVE, 0, (LPARAM)&share))
				break;

			p += 2;
		}

		bWriteConfigurationFile();
	}


	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = TranslateT("HTTP Server");
	hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);
	if (!hNetlibUser) {
		MessageBox(NULL, _T("Failed to register NetLib user"), MSG_BOX_TITEL, MB_OK);
		return 0;
	}

	if (db_get_b(NULL, MODULE, "AcceptConnections", 1))
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

int PreShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/) {
	{
		CLFileShareListAccess clCrit;
		bShutdownInProgress = true;

		for (CLFileShareNode * pclCur = pclFirstNode; pclCur ; pclCur = pclCur->pclNext) {
			pclCur->CloseAllTransfers();
		}
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

int nSystemShutdown(WPARAM /*wparam*/, LPARAM /*lparam*/) {
	while (pclFirstNode) {
		CLFileShareNode * pclCur = pclFirstNode;
		pclFirstNode = pclFirstNode->pclNext;
		delete pclCur;
	}
	pclFirstNode = NULL;

	UnInitGuiElements();

	db_set_b(NULL, MODULE, "IndexCreationMode", (BYTE)indexCreationMode);
	FreeIndexHTMLTemplate();

	return 0;
}

	/////////////////////////////////////////////////////////////////////
	// Member Function : MirandaPluginInfoEx
	// Type            : Global
	// Parameters      : mirandaVersion - ?
	// Returns         :
	// Description     :
	//
	// References      : -
	// Remarks         : -
	// Created         : 020422, 22 April 2002
	// Developer       : KN, Houdini
	/////////////////////////////////////////////////////////////////////

	extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD /*mirandaVersion*/) {
		return &pluginInfo;
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

	extern "C" __declspec(dllexport) int Load() {
		mir_getLP(&pluginInfo);
		InitializeCriticalSection(&csFileShareListAccess);

		hHttpAcceptConnectionsService = CreateServiceFunction(MS_HTTP_ACCEPT_CONNECTIONS, nToggelAcceptConnections);
		if (! hHttpAcceptConnectionsService) {
			MessageBox(NULL, _T("Failed to CreateServiceFunction MS_HTTP_ACCEPT_CONNECTIONS"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		hHttpAddChangeRemoveService = CreateServiceFunction(MS_HTTP_ADD_CHANGE_REMOVE, nAddChangeRemoveShare);
		if (! hHttpAddChangeRemoveService) {
			MessageBox(NULL, _T("Failed to CreateServiceFunction MS_HTTP_ADD_CHANGE_REMOVE"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		hHttpGetShareService = CreateServiceFunction(MS_HTTP_GET_SHARE, nGetShare);
		if (! hHttpGetShareService) {
			MessageBox(NULL, _T("Failed to CreateServiceFunction MS_HTTP_GET_SHARE"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		hHttpGetAllShares = CreateServiceFunction(MS_HTTP_GET_ALL_SHARES, nHttpGetAllShares);
		if (! hHttpGetAllShares) {
			MessageBox(NULL, _T("Failed to CreateServiceFunction MS_HTTP_GET_ALL_SHARES"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}


		hEventSystemInit = HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
		if (!hEventSystemInit) {
			MessageBox(NULL, _T("Failed to HookEvent ME_SYSTEM_MODULESLOADED"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
		if (!hPreShutdown) {
			MessageBox(NULL, _T("Failed to HookEvent ME_SYSTEM_PRESHUTDOWN"), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		if(CallService(MS_DB_GETPROFILEPATHT,MAX_PATH,(LPARAM)szPluginPath))
		{
			MessageBox(NULL, _T("Failed to retrieve plugin path."), MSG_BOX_TITEL, MB_OK);
			return 1;
		}
		_tcsncat(szPluginPath,_T("\\HTTPServer\\"), MAX_PATH);
		int err = CreateDirectoryTreeT(szPluginPath);
		if((err != 0) && (err != ERROR_ALREADY_EXISTS))
		{
			MessageBox(NULL, _T("Failed to create HTTPServer directory."), MSG_BOX_TITEL, MB_OK);
			return 1;
		}

		nPluginPathLen = (int)_tcslen(szPluginPath);

		sLogFilePath = szPluginPath;
		sLogFilePath += _T("HTTPServer.log");

		if (!bInitMimeHandling()) {
			MessageBox(NULL, _T("Failed to read configuration file : ") szMimeTypeConfigFile, MSG_BOX_TITEL, MB_OK);
		}

		nMaxUploadSpeed = db_get_dw(NULL, MODULE, "MaxUploadSpeed", nMaxUploadSpeed);
		nMaxConnectionsTotal = db_get_dw(NULL, MODULE, "MaxConnectionsTotal", nMaxConnectionsTotal);
		nMaxConnectionsPerUser = db_get_dw(NULL, MODULE, "MaxConnectionsPerUser", nMaxConnectionsPerUser);
		bLimitOnlyWhenOnline = db_get_b(NULL, MODULE, "LimitOnlyWhenOnline", bLimitOnlyWhenOnline) != 0;
		indexCreationMode = (eIndexCreationMode) db_get_b(NULL, MODULE, "IndexCreationMode", 2);

		if (db_get_b(NULL, MODULE, "AddAcceptConMenuItem", 1)) {
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIF_TCHAR;
			mi.pszContactOwner = NULL;  //all contacts
			mi.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHARE_NEW_FILE));
			mi.position = 1000085000;
			mi.ptszName = LPGENT("Enable HTTP server");
			mi.pszService = MS_HTTP_ACCEPT_CONNECTIONS;
			hAcceptConnectionsMenuItem = Menu_AddMainMenuItem(&mi);
		}

		if (indexCreationMode == INDEX_CREATION_HTML ||
		    indexCreationMode == INDEX_CREATION_DETECT)
			if (!LoadIndexHTMLTemplate()) {
				indexCreationMode = INDEX_CREATION_DISABLE;
				db_set_b(NULL, MODULE, "IndexCreationMode", (BYTE)indexCreationMode);
			}

		hEventProtoAck = HookEvent(ME_PROTO_ACK, nProtoAck);
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

	extern "C"  __declspec(dllexport) int Unload() {
		nSystemShutdown(0, 0);
		return 0;
	}