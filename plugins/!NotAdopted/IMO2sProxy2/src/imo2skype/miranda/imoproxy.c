/* Module:  imoproxy.c
   Purpose: Proxy-DLL for Miranda IM to load imo2sproxy as plugin
   Author:  leecher
   Date:    26.10.2009
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <prsht.h>
#include <commdlg.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <process.h> 
#pragma comment (lib, "Ws2_32.lib")
#include <stdio.h>
#include "io_layer.h"
#include "imo2sproxy.h"
#include "imo_request.h"
#include "socksproxy.h"
#include "w32skypeemu.h"
#include "skypepluginlink.h"
#include "include/newpluginapi.h"
#include "include/m_langpack.h"
#include "include/m_options.h"
#include "include/m_database.h"
#include "include/m_system.h"
#include "sdk/m_updater.h"
#include "resource.h"

// Crash dumper
#ifdef _DEBUG
void CrashLog (const char *pszFormat, ...)
{
	static FILE *fpLog = NULL;
	char szLine[1024];
	va_list ap;

	if (!fpLog) fpLog=fopen("imoproxy.log", "a");
	va_start(ap, pszFormat);
	_vsnprintf(szLine, sizeof(szLine), pszFormat, ap); 
	va_end(ap); 

	if (fpLog)
	{
		fprintf (fpLog, "%s", szLine);
		fflush (fpLog);
	}
}
#define LOG(_args_) CrashLog _args_ 
#ifndef _WIN64
#include "crash.c"
#endif
#endif


PLUGINLINK *pluginLink;
HINSTANCE m_hInst;
static HANDLE m_hOptHook=NULL, m_hPreShutdownHook=NULL, m_hHookModulesLoaded=NULL;

#define PROXY_SOCKS			0
#define PROXY_W32SKYPEEMU	1
#define PROXY_SKYPEPLUGIN	2
#define PROXY_MAX			3

static IMO2SPROXY_CFG m_stCfg;
static SOCKSPROXY_CFG m_stSocksCfg;
static W32SKYPEEMU_CFG m_stSypeEmuCfg;
static SKYPEPLUGINLINK_CFG m_stSkypePluginCfg;

static IMO2SPROXY *m_apProxy[PROXY_MAX] = {0};
static HANDLE m_hThread[PROXY_MAX]={0}, m_hEvent = INVALID_HANDLE_VALUE;
static BOOL m_bConsole = FALSE;

#define ANY_SIZE 1

typedef struct _MIB_IPADDRROW {
  DWORD          dwAddr;
  DWORD          dwIndex;
  DWORD          dwMask;
  DWORD          dwBCastAddr;
  DWORD          dwReasmSize;
  unsigned short unused1;
  unsigned short wType;
}MIB_IPADDRROW, *PMIB_IPADDRROW;

typedef struct _MIB_IPADDRTABLE {
  DWORD         dwNumEntries;
  MIB_IPADDRROW table[ANY_SIZE];
}MIB_IPADDRTABLE, *PMIB_IPADDRTABLE;


// Plugin Info
#define PINFO \
	"imo2sproxy-Plugin", \
	PLUGIN_MAKE_VERSION(1,0,0,15), \
	"Tunnelling Skype traffic via imo.im Web service", \
	"leecher", \
	"leecher@dose.0wnz.at", \
	"© 2010-2012 leecher", \
	"http://dose.0wnz.at", \
	0,	\
	0		//doesn't replace anything built-in


PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	PINFO
};

// New plugininfo 
PLUGININFOEX pluginInfoEx = {
	sizeof (pluginInfoEx),
	PINFO,
	{ 0x3005c2b1, 0x4278, 0x470c, { 0x94, 0x98, 0x5, 0x95, 0x3d, 0xfa, 0x4d, 0x88 } } // // {3005C2B1-4278-470c-9498-05953DFA4D88}
};

// Whatever this is...
// {95061E8D-B18C-4c1c-8E14-686DE967D851}
#define MIID_IMOPROXY { 0x95061e8d, 0xb18c, 0x4c1c, { 0x8e, 0x14, 0x68, 0x6d, 0xe9, 0x67, 0xd8, 0x51 } }
static const MUUID interfaces[] = { MIID_IMOPROXY, MIID_LAST };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int ShowError( FILE *stream, const char *format, ...)
{
	char szBuf[1024];
	va_list ap;
	int iRet;

	va_start(ap, format);
	iRet = _vsnprintf (szBuf, sizeof(szBuf), format, ap);
	va_end(ap);

	MessageBox (NULL, szBuf, Translate("IMOPROXY Error"), MB_ICONSTOP | MB_OK);
	return iRet;
}

// -----------------------------------------------------------------------------

static void LoadSettings(void)
{
	DBVARIANT dbv; 

	// General config
	if (!m_bConsole)
	{
		if (DBGetContactSetting(NULL, "IMOPROXY", "Logfile", &dbv)==0)
		{
			if ((m_stCfg.bVerbose = DBGetContactSettingByte(NULL, "IMOPROXY", "Verbose", 0)) &&
				*dbv.pszVal)
			{
				if (m_stCfg.fpLog) fclose (m_stCfg.fpLog);
				if (!(m_stCfg.fpLog = fopen(dbv.pszVal, "a")))
				{
					char szMsg[MAX_PATH+64];

					sprintf (szMsg, Translate("Cannot open Logfile %s for writing."), dbv.pszVal);
					MessageBox(NULL,szMsg,"IMOPROXY", MB_OK | MB_ICONWARNING);
					m_stCfg.bVerbose = FALSE;
				}
			}
			DBFreeVariant(&dbv); 
		}
	}
	m_stCfg.iFlags = DBGetContactSettingDword(NULL, "IMOPROXY", "Flags", 0);
	if (DBGetContactSetting(NULL, "IMOPROXY", "User", &dbv)==0)
	{
		if (m_stCfg.pszUser) free(m_stCfg.pszUser);
		m_stCfg.pszUser = strdup(dbv.pszVal);
		DBFreeVariant(&dbv); 
	}
	if (DBGetContactSetting(NULL, "IMOPROXY", "Password", &dbv)==0)
	{
		if (m_stCfg.pszPass) free(m_stCfg.pszPass);
		m_stCfg.pszPass = strdup(dbv.pszVal);
		DBFreeVariant(&dbv); 
	}

	// Socks Proxy config
	m_stSocksCfg.sPort = DBGetContactSettingWord(NULL, "IMOPROXY", "Port", 1401);
	if (DBGetContactSetting(NULL, "IMOPROXY", "Host", &dbv)==0)
	{
		m_stSocksCfg.lAddr = inet_addr(dbv.pszVal);
		DBFreeVariant(&dbv); 
	}
}

// -----------------------------------------------------------------------------

static BOOL CheckSettings(int iMask)
{
	DBVARIANT dbv; 

	if (iMask & PROXY_SOCKS)
	{
		if (DBGetContactSetting(NULL, SKYPE_PROTONAME, "Host", &dbv)==0)
		{
			if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "UseSkype2Socket", 0) &&
				(lstrcmp (dbv.pszVal, "127.0.0.1")==0 || 
				lstrcmp (dbv.pszVal, "localhost")==0) &&
				DBGetContactSettingWord(NULL, SKYPE_PROTONAME, "Port", 0) ==
				DBGetContactSettingWord(NULL, "IMOPROXY", "Port", 1401))
			{
				DBFreeVariant(&dbv);
				return TRUE;
			}
			DBFreeVariant(&dbv);
		}
	}
	if (DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "FirstRun", 9) == 9)
	{
		MessageBox (NULL, Translate("SKYPE plugin may not be installed, this plugin only works together with "
			"the SKYPE-plugin. Please check if you installed and enabled it."), "IMOPROXY", 
			MB_OK | MB_ICONWARNING);
	}
	else
	{
		
		if ((iMask & PROXY_SOCKS) && !ServiceExists(SKYPE_PROTONAME PSS_SKYPEAPIMSG) )
		{
			if (MessageBox (NULL, Translate("Your Skype plugin currently doesn't seem to be setup to use imo2proxy, "
				"do you want me to change its settings so that it uses this plugins?"), "IMOPROXY", 
				MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				DBWriteContactSettingByte (NULL, SKYPE_PROTONAME, "UseSkype2Socket", 1);
				DBWriteContactSettingWord (NULL, SKYPE_PROTONAME, "Port", m_stSocksCfg.sPort);
				DBWriteContactSettingString (NULL, SKYPE_PROTONAME, "Host", "127.0.0.1");
				return TRUE;
			}
		}
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

static BOOL EnumNetInterfaces (HWND hwndControl)
{
	HINSTANCE hLib;
	BOOL bRet = FALSE;

	if (hLib = LoadLibrary("Iphlpapi.dll"))
	{
		PMIB_IPADDRTABLE pTable;
		ULONG uSize=1;
		DWORD i;
		FARPROC GetIpAddrTable;

		if (GetIpAddrTable = (FARPROC)GetProcAddress (hLib, "GetIpAddrTable"))
		{
			if ((GetIpAddrTable (&pTable, &uSize, TRUE) == ERROR_INSUFFICIENT_BUFFER) &&
				(pTable = HeapAlloc (GetProcessHeap(), 0, uSize)))
			{
				if (GetIpAddrTable (pTable, &uSize, TRUE) == NO_ERROR)
				{
					struct in_addr addr;

					for (i=0; i<pTable->dwNumEntries; i++)
					{
						addr.S_un.S_addr = pTable->table[i].dwAddr;
						SendMessage (hwndControl, CB_ADDSTRING, 0, (LPARAM)inet_ntoa(addr));
					}
					bRet = pTable->dwNumEntries > 0;
				}
				HeapFree (GetProcessHeap(), 0, pTable);
			}
		}
		FreeLibrary (hLib);
	}
	return bRet;
}

// -----------------------------------------------------------------------------

static DWORD WINAPI ProxyThread(IMO2SPROXY *pProxy)
{
	if (pProxy->Open(pProxy)<0) return -1;
	SetEvent (m_hEvent);
	pProxy->Loop(pProxy);
	return 0;
}

// -----------------------------------------------------------------------------

static BYTE GetProxies(void)
{
	return DBGetContactSettingByte(NULL, "IMOPROXY", "Proxies", 
		(ServiceExists(SKYPE_PROTONAME PSS_SKYPEAPIMSG)?(1<<PROXY_SKYPEPLUGIN):(1<<PROXY_SOCKS)));
}

// -----------------------------------------------------------------------------

static BOOL StartProxy (int i)
{
	DWORD dwThreadId;
	BOOL bCreateThread = FALSE, bRet = TRUE;
	BYTE cEnabled = GetProxies();
	HANDLE ahEvents[2];

	// Username and Password must me available
	if (!m_stCfg.pszUser || !*m_stCfg.pszUser || 
		!m_stCfg.pszPass || !*m_stCfg.pszPass || !(cEnabled&(1<<i)) ) return FALSE;

	// Start the proxy, if enabled
	switch (i)
	{
	case PROXY_SOCKS:
		if (!(m_apProxy[i]))
			m_apProxy[i] = SocksProxy_Init (&m_stCfg, &m_stSocksCfg);
		break;
	case PROXY_W32SKYPEEMU:
		if (!(m_apProxy[i]))
			m_apProxy[i] = W32SkypeEmu_Init (&m_stCfg, &m_stSypeEmuCfg);
		break;
	case PROXY_SKYPEPLUGIN:
		if (!(m_apProxy[i]))
			m_apProxy[i] = SkypePluginLink_Init (&m_stCfg, &m_stSkypePluginCfg);
		if (m_apProxy[i]->Open(m_apProxy[i])<0)
		{
			m_apProxy[i]->Exit(m_apProxy[i]);
			return FALSE;
		}
		return TRUE;
	default:
		return FALSE;
	}
	if (!m_apProxy[i]) return FALSE;

	// As for example W32SKYPEEMU runs its own messagepump and the Window is
	// Registered in the Open-Function (as it can fail), we have to start
	// our Mainloop-Thread and wait for the Open() part to finish.
	// If it worked ok, the loop will start to run, otherwise we return
	// an error. So we need a Mutex for synchronisation.
	m_hEvent = ahEvents[1] = CreateEvent (NULL, FALSE, FALSE, NULL);

	if (!(m_hThread[i] = ahEvents[0] = (HANDLE)_beginthreadex (NULL, 0, ProxyThread, m_apProxy[i], 0, &dwThreadId)))
	{
		MessageBox (NULL, Translate("IMOPROXY Cannot start dispatch thread"), "IMOPROXY", MB_OK | MB_ICONSTOP);
		CloseHandle (m_hEvent);
		m_apProxy[i]->Exit(m_apProxy[i]);
		return FALSE;
	}

	bRet = WaitForMultipleObjects (2, ahEvents, FALSE, INFINITE)==WAIT_OBJECT_0+1;
	CloseHandle (m_hEvent);
	return bRet;
}

// -----------------------------------------------------------------------------

static void StopProxy (int i)
{
	if (m_apProxy[i])
	{
		m_apProxy[i]->Exit(m_apProxy[i]);
		m_apProxy[i] = NULL;
	}
		
	if (m_hThread[i])
	{
		if (WaitForSingleObject (m_hThread[i], 3000) != WAIT_OBJECT_0)
			TerminateThread (m_hThread[i], -1);
		m_hThread[i] = NULL;
	}
}

// -----------------------------------------------------------------------------

static BOOL StartProxies(int iMask)
{
	int i;
	BOOL bRet=TRUE;

	for (i=0; i<PROXY_MAX; i++)
		if (iMask&(1<<i))
			bRet &= StartProxy(i);
	return bRet;
}

// -----------------------------------------------------------------------------

static void StopProxies(int iMask)
{
	int i;

	for (i=0; i<PROXY_MAX; i++)
		if (iMask&(1<<i))
			StopProxy(i);
}

// -----------------------------------------------------------------------------

static void UpdateProxyStatus (HWND hWnd, int iID)
{
	struct {
		UINT uStatus;
		UINT uStart;
		UINT uStop;
	} astStatus[] = {
		{IDC_STATUSSOCKS, IDC_STARTSOCKS, IDC_STOPSOCKS},
		{IDC_STATUSCOMM, IDC_STARTCOMM, IDC_STOPCOMM},
		{IDC_STATUSSKYPEPL, IDC_STARTSKYPEPL, IDC_STOPSKYPEPL}
	};

	if (m_apProxy[iID])
	{
		SetDlgItemText (hWnd, astStatus[iID].uStatus, Translate("Running"));
		EnableWindow ((HWND)GetDlgItem (hWnd, astStatus[iID].uStart), FALSE);
		EnableWindow ((HWND)GetDlgItem (hWnd, astStatus[iID].uStop), TRUE);
	}
	else
	{
		BYTE cEnabled = GetProxies();

		SetDlgItemText (hWnd, astStatus[iID].uStatus, Translate("Stopped"));
		EnableWindow ((HWND)GetDlgItem (hWnd, astStatus[iID].uStart), 
			((cEnabled&(1<<iID)) &&	m_stCfg.pszUser && *m_stCfg.pszUser && 
			m_stCfg.pszPass && *m_stCfg.pszPass)?TRUE:FALSE);
		EnableWindow ((HWND)GetDlgItem (hWnd, astStatus[iID].uStop), FALSE);
	}
}

// -----------------------------------------------------------------------------

static int CALLBACK OptionsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char szOldHost[64]={0}, szOldLog[MAX_PATH]={0}, szOldUser[64], szOldPass[64];
	static short sOldPort=0;
	static BOOL bOldVerbose=FALSE;
	static int iOldFlags = 0, iOldProxies;

	switch (uMsg)
	{
		case WM_INITDIALOG:	
		{
			DBVARIANT dbv; 

			TranslateDialogDefault(hWnd);
			EnumNetInterfaces (GetDlgItem (hWnd, IDC_BINDIP));
			if (DBGetContactSetting(NULL, "IMOPROXY", "Host", &dbv)==0)
			{
				lstrcpyn (szOldHost, dbv.pszVal, sizeof(szOldHost));
				DBFreeVariant(&dbv); 
			} else lstrcpy (szOldHost, "127.0.0.1");
			SetDlgItemText (hWnd, IDC_BINDIP,szOldHost);
			SetDlgItemInt (hWnd, IDC_BINDPORT, sOldPort = DBGetContactSettingWord(NULL, "IMOPROXY", "Port", 1401), FALSE);
			iOldFlags = DBGetContactSettingDword(NULL, "IMOPROXY", "Flags", 0);
			if (iOldFlags & IMO2S_FLAG_ALLOWINTERACT) CheckDlgButton (hWnd, IDC_INTERACT, BST_CHECKED);
			if (iOldFlags & IMO2S_FLAG_CURRTIMESTAMP) CheckDlgButton (hWnd, IDC_CURRTIMESTAMP, BST_CHECKED);
			if (DBGetContactSetting(NULL, "IMOPROXY", "Logfile", &dbv)==0)
			{
				lstrcpyn (szOldLog, dbv.pszVal, sizeof(szOldLog));
				DBFreeVariant(&dbv); 
			}
			if (CallService (MS_SYSTEM_GETVERSION, 0, 0) >= 0x080000)
			{
				EnableWindow (GetDlgItem (hWnd, IDC_USENETLIB), TRUE);
				if (DBGetContactSettingByte (NULL, "IMOPROXY", "UseNetlib", 0))
					CheckDlgButton (hWnd, IDC_USENETLIB, BST_CHECKED);
			}
			SetDlgItemText (hWnd, IDC_LOGFILE, szOldLog);
			if (bOldVerbose = DBGetContactSettingByte(NULL, "IMOPROXY", "Verbose", 0))
				CheckDlgButton (hWnd, IDC_LOG, BST_CHECKED);
			else
			{
				EnableWindow (GetDlgItem (hWnd, IDC_LOGFILE), FALSE);
				EnableWindow (GetDlgItem (hWnd, IDC_OPEN), FALSE);
			}
			if (DBGetContactSetting(NULL, "IMOPROXY", "User", &dbv)==0)
			{
				lstrcpyn (szOldUser, dbv.pszVal, sizeof(szOldUser));
				DBFreeVariant(&dbv); 
			}
			SetDlgItemText (hWnd, IDC_USERNAME, szOldUser);
			if (DBGetContactSetting(NULL, "IMOPROXY", "Password", &dbv)==0)
			{
				lstrcpyn (szOldPass, dbv.pszVal, sizeof(szOldPass));
				DBFreeVariant(&dbv); 
			}
			iOldProxies= GetProxies();
			CheckDlgButton (hWnd, IDC_USESOCKS, (iOldProxies&(1<<PROXY_SOCKS))?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton (hWnd, IDC_USECOMM, (iOldProxies&(1<<PROXY_W32SKYPEEMU))?BST_CHECKED:BST_UNCHECKED);
			if (ServiceExists(SKYPE_PROTONAME PSS_SKYPEAPIMSG))
			{
				CheckDlgButton (hWnd, IDC_USESKYPEPL, (iOldProxies&(1<<PROXY_SKYPEPLUGIN))?BST_CHECKED:BST_UNCHECKED);
				UpdateProxyStatus (hWnd, PROXY_SKYPEPLUGIN);
			}
			else 
			{
				EnableWindow (GetDlgItem (hWnd, IDC_USESKYPEPL), FALSE);
				EnableWindow (GetDlgItem (hWnd, IDC_STARTSKYPEPL), FALSE);
			}
			SetDlgItemText (hWnd, IDC_PASSWORD, szOldPass);
			UpdateProxyStatus (hWnd, PROXY_SOCKS);
			UpdateProxyStatus (hWnd, PROXY_W32SKYPEEMU);
#ifdef _DEBUG
			EnableWindow (GetDlgItem(hWnd, IDC_CONSOLE), TRUE);
#endif
			return TRUE;
		}
		case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code)
			{
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					int iFlags=0, iProxies=0;
					short sPort;
					char szHost[64], szLog[MAX_PATH], szUser[64], szPass[64];
					BOOL bVerbose;

					GetDlgItemText (hWnd, IDC_BINDIP, szHost, sizeof(szHost));
					DBWriteContactSettingString (NULL, "IMOPROXY", "Host", szHost);
					DBWriteContactSettingWord (NULL, "IMOPROXY", "Port", 
						(sPort = GetDlgItemInt (hWnd, IDC_BINDPORT, NULL, FALSE)));
					if (IsDlgButtonChecked (hWnd, IDC_INTERACT)==BST_CHECKED)
						iFlags|=IMO2S_FLAG_ALLOWINTERACT;
					if (IsDlgButtonChecked (hWnd, IDC_CURRTIMESTAMP)==BST_CHECKED)
						iFlags|=IMO2S_FLAG_CURRTIMESTAMP;
					GetDlgItemText (hWnd, IDC_USERNAME, szUser, sizeof(szUser));
					DBWriteContactSettingString (NULL, "IMOPROXY", "User", szUser);
					GetDlgItemText (hWnd, IDC_PASSWORD, szPass, sizeof(szPass));
					DBWriteContactSettingString (NULL, "IMOPROXY", "Password", szPass);
					DBWriteContactSettingDword (NULL, "IMOPROXY", "Flags", iFlags);
					DBWriteContactSettingByte(NULL, "IMOPROXY", "Verbose", 
						(char)(bVerbose = (IsDlgButtonChecked (hWnd, IDC_LOG)==BST_CHECKED)));
					GetDlgItemText (hWnd, IDC_LOGFILE, szLog, sizeof(szLog));
					DBWriteContactSettingString (NULL, "IMOPROXY", "Logfile", szLog);
					if (IsDlgButtonChecked (hWnd, IDC_USESOCKS)==BST_CHECKED)
						iProxies|=(1<<PROXY_SOCKS);
					if (IsDlgButtonChecked (hWnd, IDC_USECOMM)==BST_CHECKED)
						iProxies|=(1<<PROXY_W32SKYPEEMU);
					if (IsDlgButtonChecked (hWnd, IDC_USESKYPEPL)==BST_CHECKED)
						iProxies|=(1<<PROXY_SKYPEPLUGIN);
					DBWriteContactSettingByte(NULL, "IMOPROXY", "UseNetlib", 
						(char)(IsDlgButtonChecked (hWnd, IDC_USENETLIB)==BST_CHECKED));
					DBWriteContactSettingByte(NULL, "IMOPROXY", "Proxies", (char)iProxies);
					iProxies^=iOldProxies;
					if (sPort != sOldPort || lstrcmp (szOldHost, szHost))
						iProxies|=(1<<PROXY_SOCKS);

					if (lstrcmp (szOldLog, szLog) || bOldVerbose != bVerbose ||
						lstrcmp (szOldUser, szUser) || lstrcmp(szOldPass, szPass) ||
						iFlags != iOldFlags)
						iProxies=(1<<PROXY_MAX)-1;

					/*
					StopProxies(iProxies);
					LoadSettings();
					CheckSettings(iProxies);
					StartProxies(iProxies);
					*/
					UpdateProxyStatus (hWnd, PROXY_SOCKS);
					UpdateProxyStatus (hWnd, PROXY_W32SKYPEEMU);
					if (ServiceExists(SKYPE_PROTONAME PSS_SKYPEAPIMSG))
						UpdateProxyStatus (hWnd, PROXY_SKYPEPLUGIN);
					return TRUE;
				}
			}
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LOG:
				{
					BOOL bEnable = (SendMessage ((HWND)lParam, BM_GETCHECK, 0, 0)==BST_CHECKED);

					EnableWindow (GetDlgItem (hWnd, IDC_LOGFILE), bEnable);
					EnableWindow (GetDlgItem (hWnd, IDC_OPEN), bEnable);
					break;
				}
				case IDC_OPEN:
				{
					char szFilename[MAX_PATH];
					OPENFILENAME ofn={0};

					GetDlgItemText (hWnd, IDC_LOGFILE, szFilename, sizeof(szFilename));
					ofn.lStructSize=sizeof(OPENFILENAME);
					ofn.hwndOwner=hWnd;
					ofn.Flags=OFN_HIDEREADONLY;
					ofn.lpstrTitle=Translate("Select where log file will be created");
					ofn.lpstrFilter=Translate("All files (*.*)\0*.*\0");
					ofn.lpstrFile=szFilename;
					ofn.nMaxFile=sizeof(szFilename)-2;
					ofn.nMaxFileTitle=sizeof(szFilename);
					if(GetSaveFileName(&ofn))
						SetDlgItemText(hWnd, IDC_LOGFILE, szFilename);
					break;
				}
				case IDC_STARTSOCKS:
				case IDC_STARTCOMM:
				case IDC_STARTSKYPEPL:
				{
					int iID;

					switch (LOWORD(wParam))
					{
					case IDC_STARTSOCKS: 
						iID = PROXY_SOCKS;
						break;
					case IDC_STARTCOMM: 
						iID = PROXY_W32SKYPEEMU; 
						break;
					case IDC_STARTSKYPEPL:
						iID = PROXY_SKYPEPLUGIN; 
						break;
					}
					EnableWindow ((HWND)lParam, FALSE);
					LoadSettings();
					CheckSettings (1<<iID);
					StartProxy (iID);
					UpdateProxyStatus (hWnd, iID);
					break;
				}
				case IDC_STOPSOCKS:
				case IDC_STOPCOMM:
				case IDC_STOPSKYPEPL:
				{
					int iID;

					switch (LOWORD(wParam))
					{
					case IDC_STOPSOCKS: 
						iID = PROXY_SOCKS;
						break;
					case IDC_STOPCOMM: 
						iID = PROXY_W32SKYPEEMU; 
						break;
					case IDC_STOPSKYPEPL:
						iID = PROXY_SKYPEPLUGIN; 
						break;
					}
					EnableWindow ((HWND)lParam, FALSE);
					StopProxy (iID);
					UpdateProxyStatus (hWnd, iID);
					break;
				}
				case IDC_CONSOLE:
				{
					CONSOLE_SCREEN_BUFFER_INFO coninfo;
					HANDLE hStdHandle;
					int hConHandle;
					HMENU hMenu;
					FILE *fp;
					HMODULE hKernel32;
					HWND (WINAPI *_GetConsoleWindow)(void), hWndCon;

					/* Some dirty hack for a simple console. I know this isn't really
					   proper and that I should use my own console, but it's enough for
					   debugging purposes ;-)
					 */
					if (HIWORD(wParam)!=BN_CLICKED) break;
					switch (SendMessage ((HWND)lParam, BM_GETCHECK, 0, 0))
					{
					case BST_UNCHECKED:
						if (m_stCfg.fpLog) fclose(m_stCfg.fpLog);
						m_stCfg.fpLog = NULL;
						LoadSettings ();
						FreeConsole();
						m_stCfg.bVerbose = bOldVerbose;
						m_bConsole = FALSE;
						break;
					case BST_CHECKED:
						m_bConsole = AllocConsole();
						hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
						GetConsoleScreenBufferInfo(hStdHandle, &coninfo);
						coninfo.dwSize.Y = 500;
						SetConsoleScreenBufferSize(hStdHandle, coninfo.dwSize);
						hConHandle = _open_osfhandle((long)hStdHandle, _O_TEXT);
						fp = _fdopen(hConHandle, "w"); 
						if (!fp)
						{
							FreeConsole();
							break;
						}
						// Only available in Win2k or above
						// Protect Console form closing, otherwise closing the console
						// would Shutdown Miranda
						if ((hKernel32 = GetModuleHandle ("kernel32.dll")) &&
							(*(FARPROC *)&_GetConsoleWindow = 
								GetProcAddress(hKernel32, "GetConsoleWindow")) &&
							(hWndCon = _GetConsoleWindow()))
						{
							hMenu = GetSystemMenu (hWndCon, FALSE);
							DeleteMenu (hMenu, SC_CLOSE, MF_BYCOMMAND);
						}
						else
						{
							fprintf (fp, Translate("WARNING: Only close this console by pushing the Console button "
								"in the settings dialog, otherwise you sould shutdown Miranda by closing "
								"the Window!\n"));
						}
						setvbuf(fp, NULL, _IONBF, 0 ); 
						if (m_stCfg.fpLog && m_stCfg.fpLog != stdout && m_stCfg.fpLog != stderr) fclose(m_stCfg.fpLog);
						bOldVerbose = m_stCfg.bVerbose;
						m_stCfg.bVerbose = 1;
						m_stCfg.fpLog = fp;
						break;
					}				
					break;
				}
			}
			SendMessage (GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;
	}
	return FALSE;
}

// -----------------------------------------------------------------------------

static int RegisterOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp={0};
   
	odp.cbSize = sizeof(odp);
	odp.hInstance = m_hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup = Translate("Network");
	odp.pszTitle = "Skype Imoproxy";
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

// -----------------------------------------------------------------------------

void RegisterToUpdate(void)
{
	//Use for the Updater plugin
	if(ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *)&pluginInfo, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);

#ifdef _WIN64
		update.szUpdateURL = "http://dose.0wnz.at/miranda/Skype/imo2sproxy_x64_binonly.zip";	// FIXME!!
		update.szVersionURL = "http://dose.0wnz.at/miranda/Skype/"; // FIXME
		update.pbVersionPrefix = (BYTE *)"imo2sproxy version "; //FIXME
		update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/imo2sproxy_x64_binonly.zip";
		update.szBetaVersionURL = "http://dose.0wnz.at/miranda/Skype/";
		update.pbBetaVersionPrefix = (BYTE *)"imo2sproxy version ";
#else
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=4146";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=4146";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">Skype to imo.im Gateway ";
	    update.szBetaUpdateURL = "http://dose.0wnz.at/miranda/Skype/imo2sproxy_w32_binonly.zip";
		update.szBetaVersionURL = "http://dose.0wnz.at/miranda/Skype/";
		update.pbBetaVersionPrefix = (BYTE *)"imo2sproxy version ";
#endif

		update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	}
}

// -----------------------------------------------------------------------------

int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	OutputDebugString ("IMOPROXY: PreShutdown");
	StopProxies (-1);
	if (m_stCfg.fpLog && m_stCfg.fpLog != stdout && m_stCfg.fpLog != stderr)
	{
		fclose(m_stCfg.fpLog);
		m_stCfg.fpLog = NULL;
	}
	if (m_stCfg.pszUser)
	{
		free(m_stCfg.pszUser);
		m_stCfg.pszUser = NULL;
	}
	if (m_stCfg.pszPass)
	{
		free(m_stCfg.pszPass);
		m_stCfg.pszPass = NULL;
	}

	FreeConsole();
	return 0;
}

// -----------------------------------------------------------------------------

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	BYTE CheckSkype = DBGetContactSettingByte (NULL, "IMOPROXY", "CheckSkype", 2);
	if (CheckSkype) CheckSettings(-1);
	if (CheckSkype == 2) DBWriteContactSettingByte (NULL, "IMOPROXY", "CheckSkype", 0);
	RegisterToUpdate();

	// On Miranda 0.0.0.8+ NETLIB suppotrs HTTPS, therefore we can use 
	// Netlib there
	if (CallService (MS_SYSTEM_GETVERSION, 0, 0) >= 0x080000 &&
		DBGetContactSettingByte (NULL, "IMOPROXY", "UseNetlib", 0))
		ImoRq_SetIOLayer (IoLayerNETLIB_Init);
	StartProxies(-1);

	return 0;
}

// -----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	m_hInst=hinstDLL;
	return TRUE;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

// -----------------------------------------------------------------------------

// New plugin API
__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

// -----------------------------------------------------------------------------

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

// -----------------------------------------------------------------------------

int __declspec(dllexport) Load(PLUGINLINK *link)
{

#ifdef _DEBUG
	Crash_Init();
#endif
	pluginLink = link;

	// Init IMO2S config structures
	Imo2sproxy_Defaults (&m_stCfg);
	SocksProxy_Defaults (&m_stSocksCfg);
	W32SkypeEmu_Defaults(&m_stSypeEmuCfg);
	SkypePluginLink_Defaults(&m_stSkypePluginCfg);
	m_stCfg.logerror = ShowError;
	LoadSettings();

	m_hOptHook = HookEvent(ME_OPT_INITIALISE, RegisterOptions);
	m_hPreShutdownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	m_hHookModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	
	OutputDebugString ("IMOPROXY: Loaded");
	return 0;
}

// -----------------------------------------------------------------------------

int __declspec(dllexport) Unload(void)
{
	OutputDebugString ("IMOPROXY: Unload");
	UnhookEvent(m_hOptHook);
	UnhookEvent(m_hPreShutdownHook);
	UnhookEvent(m_hHookModulesLoaded);
	return 0;
}
