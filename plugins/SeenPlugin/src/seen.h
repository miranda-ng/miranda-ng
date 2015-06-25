/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_genmenu.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_popup.h>
#include <m_timezones.h>
#include <m_ignore.h>
#include <m_button.h>
#include <m_string.h>

#include <m_tipper.h>

#include "resource.h"
#include "version.h"

WCHAR *any_to_IdleNotidleUnknown(MCONTACT hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen);
WCHAR *any_to_Idle(MCONTACT hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen);

#define NUM100NANOSEC  116444736000000000

#define S_MOD "SeenModule"

//#define UM_CHECKHOOKS (WM_USER+1)

#define debug(a) MessageBox(NULL,a,_T("Debug"),MB_OK)

#define IDI_USERDETAILS                 160
#define IDI_DOWNARROW                   264
#define IDI_RECVMSG                     136

#define ICON_OFFLINE		13
#define ICON_ONLINE			14
#define ICON_AWAY			15
#define ICON_NA				16
#define ICON_OCC			17
#define ICON_DND			18
#define ICON_FREE			19
#define ICON_INVIS			20

#define WM_REFRESH_UI (WM_USER+10)

#define DEFAULT_MENUSTAMP          _T("%d.%m.%Y - %H:%M [%s]")
#define DEFAULT_POPUPSTAMP         TranslateT("%n is %s (%u)")
#define DEFAULT_POPUPSTAMPTEXT     TranslateT("%i(%r)%bWas %o")
#define DEFAULT_USERSTAMP          TranslateT("Name:%t%N%bStatus:%t%s%bDay:%t%d.%m.%Y%bTime:%t%H:%M:%S%bPrevious Status:%t%o%b%b%P ID:%t%u%bExternal IP:%t%i%bInternal IP:%t%r%bClient ID: %t%C%b%bStatus Message:%t%T")
#define DEFAULT_FILESTAMP          _T("%d.%m.%Y %H:%M:%S%t%n%t%s%t%u%t%r | %i%t%N")
#define DEFAULT_FILENAME           _T("%miranda_logpath%\\LastSeen\\%P.txt")
#define DEFAULT_HISTORYSTAMP       _T("%d.%m.%Y - %H:%M [%s]")
#define DEFAULT_WATCHEDPROTOCOLS   ""

typedef struct{
	int count;
	WPARAM wpcontact[1024];
	BYTE times[1024];
} MISSEDCONTACTS;

int IsWatchedProtocol(const char* szProto);
TCHAR *ParseString(TCHAR*, MCONTACT, BYTE);
void GetColorsFromDWord(LPCOLORREF First, LPCOLORREF Second, DWORD colDword);
DWORD GetDWordFromColors(COLORREF First, COLORREF Second);
int OptionsInit(WPARAM,LPARAM);
int UserinfoInit(WPARAM,LPARAM);
void InitMenuitem(void);
int UpdateValues(WPARAM, LPARAM);
int ModeChange(WPARAM,LPARAM);
void SetOffline(void);
int ModeChange_mo(WPARAM,LPARAM);
int CheckIfOnline(void);
void ShowHistory(MCONTACT hContact, BYTE isAlert);

void InitFileOutput(void);
void UninitFileOutput(void);

struct logthread_info
{
	MCONTACT hContact;
	char   sProtoName[MAXMODULELABELLENGTH];
	WORD   currStatus;
};

extern HINSTANCE hInstance;
extern DWORD StatusColors15bits[];
extern BOOL includeIdle;
extern HANDLE ehmissed, ehuserinfo, ehmissed_proto;
extern MWindowList g_pUserInfo;
extern HGENMENU hmenuitem;
extern DWORD dwmirver;

extern BOOL g_bFileActive;

void   LoadWatchedProtos();
void   UnloadWatchedProtos();
extern LIST<char> arWatchedProtos;

extern LIST<logthread_info> arContacts;
extern mir_cs csContacts;
