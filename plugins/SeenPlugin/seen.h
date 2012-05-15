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

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/seen.h $
Revision       : $Rev: 1570 $
Last change on : $Date: 2007-12-30 01:30:07 +0300 (Вс, 30 дек 2007) $
Last change by : $Author: y_b $
*/
#ifndef _WIN32_IE
#define _WIN32_IE 0x0300
#endif
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)
#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

#include <windows.h>
#include <win2k.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>

#include <m_system.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_clist.h>
#include <m_userinfo.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_popup.h>
#include <m_system.h>

#include "m_tipper.h"
WCHAR *any_to_IdleNotidleUnknown(HANDLE hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen);
WCHAR *any_to_Idle(HANDLE hContact, const char *module_name, const char *setting_name, WCHAR *buff, int bufflen);

#ifdef __GNUC__
#define NUM100NANOSEC  116444736000000000ULL
#else
#define NUM100NANOSEC  116444736000000000
#endif

#define S_MOD "SeenModule"

//#define UM_CHECKHOOKS (WM_USER+1)

#define debug(a) MessageBox(NULL,a,"Debug",MB_OK)

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
#define	ICON_INVIS			20

#define DEFAULT_MENUSTAMP          "%d.%m.%Y - %H:%M [%s]"
#define DEFAULT_POPUPSTAMP         Translate("%n is %s (%u)")
#define DEFAULT_POPUPSTAMPTEXT     Translate("%i(%r)%bWas %o")
#define DEFAULT_USERSTAMP          Translate("Name:%t%N%bStatus:%t%s%bDay:%t%d.%m.%Y%bTime:%t%H:%M:%S%bPrevious Status:%t%o%b%b%P ID:%t%u%bExternal IP:%t%i%bInternal IP:%t%r%bClientID: %t%C%b%bStatus Message:%t%T")
#define DEFAULT_FILESTAMP          "%d.%m.%Y %H:%M:%S%t%n%t%s%t%u%t%r | %i%t%N"
#define DEFAULT_FILENAME           "logs\\%P.txt"
#define DEFAULT_HISTORYSTAMP       "%d.%m.%Y - %H:%M [%s]"
#define DEFAULT_WATCHEDPROTOCOLS   ""

#define VARIABLE_LIST "%s \n%%Y: \t %s \n%%y: \t %s \n%%m: \t %s \n%%E: \t %s \n%%e: \t %s \n%%d: \t %s \n%%W: \t %s \n%%w: \t %s \n\n%s \n%%H: \t %s \n%%h: \t %s \n%%p: \t %s \n%%M: \t %s \n%%S: \t %s \n\n%s \n%%n: \t %s \n%%N: \t %s \n%%u: \t %s \n%%G: \t %s \n%%s: \t %s \n%%T: \t %s \n%%o: \t %s \n%%i: \t %s \n%%r: \t %s \n%%C: \t %s \n%%P: \t %s \n\n%s \n%%t: \t %s \n%%b: \t %s\n\n%s\t%s \"#\" %s\n\t%s %s", Translate("-- Date --"), Translate("year (4 digits)"), Translate("year (2 digits)"), Translate("month"), Translate("name of month"), Translate("short name of month"), Translate("day"), Translate("weekday (full)"), Translate("weekday (abbreviated)"), Translate("-- Time --"), Translate("hours (24)"), Translate("hours (12)"), Translate("AM/PM"), Translate("minutes"), Translate("seconds"), Translate("-- User --"), Translate("username"), Translate("nick"), Translate("UIN/handle"), Translate("Group"), Translate("Status"), Translate("Status message"), Translate("Old status"), Translate("external IP"), Translate("internal IP"),Translate("Client info"),Translate("Protocol"), Translate("-- Format --"), Translate("tabulator"), Translate("line break"), Translate("Note:"),Translate("Use"),Translate("for empty string"),Translate("instead of"),Translate("<unknown>")

#ifndef LPCOLORREF
typedef DWORD   *LPCOLORREF;
#endif

typedef struct{
	int count;
	WPARAM wpcontact[1024];
	BYTE times[1024];
} MISSEDCONTACTS;

/* utils.c */
int IsWatchedProtocol(const char* szProto);
char *ParseString(char *,HANDLE,BYTE);
extern DWORD StatusColors15bits[];
void GetColorsFromDWord(LPCOLORREF First, LPCOLORREF Second, DWORD colDword);
DWORD GetDWordFromColors(COLORREF First, COLORREF Second);

void UninitMenuitem();

BOOL includeIdle;
typedef struct logthread_info {
  char sProtoName[MAXMODULELABELLENGTH];
  HANDLE hContact;
  WORD courStatus;
  int queueIndex;
} logthread_info;

extern logthread_info **contactQueue;
extern int contactQueueSize;

