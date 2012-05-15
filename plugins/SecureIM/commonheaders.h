// Windows API

#define WIN32_LEAN_AND_MEAN
#define NETLIB_LOG

#ifdef _MSC_VER
#pragma once
#define _CRT_SECURE_NO_WARNINGS
// _MSC_VER: 1200=6.0 1300=7.0(2003) 1400=8.0(2005) 1500=9.0(2008)
#if _MSC_VER >= 1300
// MSVC 7.0 and above
#define mir_itoa _itoa
#define mir_unlink _unlink
#else
// MSVC 6.0 and below
#ifndef _DEBUG
#pragma optimize("gsy", on)
#endif 
#endif 
#endif

#ifndef mir_itoa
#define mir_itoa itoa
#endif

#ifndef mir_unlink
#define mir_unlink unlink
#endif

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#ifndef M_SIM_COMMONHEADERS_H
#define M_SIM_COMMONHEADERS_H

#define MIRANDA_VER 0x0700
#define MIRANDA_CUSTOM_LP
#include <m_stdhdr.h>

// Windows API
#include <windows.h>
#include <wingdi.h>
#include <winsock2.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#include <time.h>

#define MODULENAME "SecureIM"

#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hwndLV, i, fCheck) \
  ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)
#endif

#ifndef SIZEOF
#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))
#endif

#ifndef M_API_H__
#define M_API_H__

// Miranda API
#include "newpluginapi.h"
#include "m_stdhdr.h"
#include "m_plugins.h"
#include "m_system.h"
#include "m_database.h"
#include "m_protomod.h"
#include "m_protosvc.h"
#include "m_langpack.h"
#include "m_options.h"
#include "m_clist.h"
#include "m_clc.h"
#include "m_clui.h"
#include "m_cluiframes.h"
#include "m_utils.h"
#include "m_skin.h"
#include "m_popup.h"
#include "m_genmenu.h"
#include "m_icolib.h"
#include "m_message.h"
#include "m_netlib.h"
#include "m_metacontacts.h"
#include "m_updater.h"
#include "m_extraicons.h"
#include "m_folders.h"
 
#endif

// my libs
#include "secureim.h"
#include "version.h"
#include "resource.h"
#include "language.h"
#include "loadlib.h"
#include "mmi.h"
#include "crypt.h"
#include "gettime.h"
#include "language.h"
#include "options.h"
#include "popupoptions.h"
#include "loadicons.h"
#include "rtfconv.h"
#include "cryptopp.h"
#include "images.h"
#include "dbevent.h"
#include "splitmsg.h"
#include "svcs_proto.h"
#include "svcs_clist.h"
#include "svcs_menu.h"
#include "svcs_srmm.h"
#include "svcs_rsa.h"

extern LPCSTR szModuleName;
extern LPCSTR szVersionStr;
extern char TEMP[MAX_PATH];
extern int  TEMP_SIZE;

// shared vars
extern HINSTANCE g_hInst, g_hIconInst;
extern PLUGININFOEX pluginInfoEx;
extern MUUID interfaces[];

#define MIID_SECUREIM	{0x1B2A39E5, 0xE2F6, 0x494D, { 0x95, 0x8D, 0x18, 0x08, 0xFD, 0x11, 0x0D, 0xD5 }} //1B2A39E5-E2F6-494D-958D-1808FD110DD5

#define PREF_METANODB	0x2000	//!< Flag to indicate message should not be added to db by filter when sending
#define PREF_SIMNOMETA	0x4000	//!< Flag to indicate message should not be inspected by filter on metacontact

#define DLLEXPORT __declspec(dllexport)

extern "C" {

 DLLEXPORT int Load(PLUGINLINK *);
 DLLEXPORT PLUGININFOEX *MirandaPluginInfoEx(DWORD);
 DLLEXPORT MUUID* MirandaPluginInterfaces(void);
 DLLEXPORT int Unload();

}

extern HANDLE g_hEvent[2], g_hMenu[15], g_hCLIcon, g_hFolders;
extern HANDLE *g_hService;
extern HANDLE *g_hHook;
extern int iService, iHook;
extern HICON g_hICO[ICO_CNT], g_hIEC[1+IEC_CNT*MODE_CNT], g_hPOP[POP_CNT];
extern IconExtraColumn g_IEC[1+IEC_CNT*MODE_CNT];
extern int iBmpDepth;
extern BOOL bCoreUnicode, bMetaContacts, bPopupExists, bPopupUnicode;
extern BOOL bPGPloaded, bPGPkeyrings, bUseKeyrings, bPGPprivkey;
extern BOOL bGPGloaded, bGPGkeyrings, bSavePass;
extern BOOL bSFT, bSOM, bASI, bMCD, bSCM, bDGP, bAIP, bNOL, bAAK, bMCM;
extern BYTE bADV, bPGP, bGPG;
extern DWORD iCoreVersion;
extern CRITICAL_SECTION localQueueMutex;

int onModulesLoaded(WPARAM,LPARAM);
int onSystemOKToExit(WPARAM,LPARAM);

void AddServiceFunction(LPCSTR,MIRANDASERVICE);
void AddProtoServiceFunction(LPCSTR,MIRANDASERVICE);
void AddHookFunction(LPCSTR,MIRANDAHOOK);

LPSTR myDBGetString(HANDLE,const char *,const char *);
LPSTR myDBGetStringDecode(HANDLE,const char *,const char *);
int myDBWriteStringEncode(HANDLE,const char *,const char *,const char *);

#if defined(_DEBUG) || defined(NETLIB_LOG)
extern HANDLE hNetlibUser;
void InitNetlib();
void DeinitNetlib();
int Sent_NetLog(const char *,...);
#endif
/*
int DBWriteString(HANDLE,const char *,const char *,const char *);
int DBGetByte(HANDLE,const char *,const char *,int);
int DBWriteByte(HANDLE,const char *,const char *,BYTE);
int DBGetWord(HANDLE,const char *,const char *,int);
int DBWriteWord(HANDLE,const char *,const char *,WORD);
*/
void GetFlags();
void SetFlags();
/*
LPSTR u2a( LPCWSTR src );
LPWSTR a2u( LPCSTR src );
*/
LPSTR TranslateU( LPCSTR lpText );
int msgbox( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType );
void CopyToClipboard(HWND hwnd,LPSTR msg);
#define msgbox0(a,b,c,d) msgbox(a,b,c,d)
#define msgbox1(a,b,c,d) msgbox(a,b,c,d)

#endif

// EOF
