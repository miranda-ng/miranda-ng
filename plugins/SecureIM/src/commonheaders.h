#define WIN32_LEAN_AND_MEAN
#define NETLIB_LOG

#define _CRT_SECURE_NO_WARNINGS
#define mir_itoa _itoa
#define mir_unlink _unlink

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

#include <windows.h>
#include <winsock2.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <Uxtheme.h>

#include <malloc.h>
#include <process.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_genmenu.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_extraicons.h>

#include <m_metacontacts.h>
#include <m_folders.h>
 
#include "secureim.h"
#include "version.h"
#include "resource.h"
#include "..\SecureIM_icons\src\resource.h"
#include "language.h"
#include "mmi.h"
#include "crypt.h"
#include "gettime.h"
#include "language.h"
#include "options.h"
#include "popupoptions.h"
#include "loadicons.h"
#include "rtfconv.h"
#include "cryptopp.h"
#include "loadlib.h"
#include "images.h"
#include "dbevent.h"
#include "splitmsg.h"
#include "svcs_proto.h"
#include "svcs_clist.h"
#include "svcs_menu.h"
#include "svcs_srmm.h"
#include "svcs_rsa.h"

#define MODULENAME "SecureIM"

extern char TEMP[MAX_PATH];
extern int  TEMP_SIZE;

// shared vars
extern HINSTANCE g_hInst, g_hIconInst;

#define PREF_METANODB	0x2000	//!< Flag to indicate message should not be added to db by filter when sending
#define PREF_SIMNOMETA	0x4000	//!< Flag to indicate message should not be inspected by filter on metacontact

extern HANDLE g_hEvent[2], g_hCLIcon, g_hFolders;
extern HGENMENU g_hMenu[15];
extern int iService, iHook;
extern HICON g_hICO[ICO_CNT], g_hIEC[1+IEC_CNT*MODE_CNT], g_hPOP[POP_CNT];
extern HANDLE g_IEC[1+IEC_CNT*MODE_CNT];
extern int iBmpDepth;
extern BOOL bPopupExists;
extern BOOL bPGPloaded, bPGPkeyrings, bUseKeyrings, bPGPprivkey;
extern BOOL bGPGloaded, bGPGkeyrings, bSavePass;
extern BOOL bSFT, bSOM, bASI, bMCD, bSCM, bDGP, bAIP, bNOL, bAAK, bMCM;
extern BYTE bPGP, bGPG;
extern DWORD iCoreVersion;
extern CRITICAL_SECTION localQueueMutex;

__forceinline int SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return ProtoBroadcastAck( GetContactProto(hContact), hContact, type, result, hProcess, lParam);
}

extern HANDLE hNetlibUser;
void InitNetlib();
void DeinitNetlib();
int Sent_NetLog(const char *,...);

void GetFlags();
void SetFlags();

int msgbox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
void CopyToClipboard(HWND hwnd,LPSTR msg);
#define msgbox0(a,b,c,d) msgbox(a,b,c,d)
#define msgbox1(a,b,c,d) msgbox(a,b,c,d)

#endif

// EOF
