/*

Import plugin for Miranda NG

Copyright (C) 2012-14 George Hazan

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

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

#include <windows.h>
#include <commctrl.h> // datetimepicker

#include <malloc.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_icolib.h>
#include <m_clist.h>
#include <m_db_int.h>
#include <m_metacontacts.h>

#include "version.h"
#include "resource.h"

// ** Global constants

#define IMPORT_MODULE  "MIMImport"        // Module name
#define IMPORT_SERVICE "MIMImport/Import" // Service for menu item

// Keys
#define IMP_KEY_FR      "FirstRun"   // First run


#define WIZM_GOTOPAGE    (WM_USER+10)	//wParam=resource id, lParam=dlgproc
#define WIZM_DISABLEBUTTON  (WM_USER+11)    //wParam=0:back, 1:next, 2:cancel
#define WIZM_SETCANCELTEXT  (WM_USER+12)    //lParam=(char*)newText
#define WIZM_ENABLEBUTTON   (WM_USER+13)    //wParam=0:back, 1:next, 2:cancel

#define PROGM_SETPROGRESS  (WM_USER+10)   //wParam=0..100
#define PROGM_ADDMESSAGE   (WM_USER+11)   //lParam=(char*)szText

#define ICQOSCPROTONAME  "ICQ"
#define MSNPROTONAME     "MSN"
#define YAHOOPROTONAME   "YAHOO"
#define NSPPROTONAME     "NET_SEND"
#define ICQCORPPROTONAME "ICQ Corp"
#define AIMPROTONAME     "AIM"

// Import type
#define IMPORT_CONTACTS 0
#define IMPORT_ALL      1
#define IMPORT_CUSTOM   2

// Custom import options
#define IOPT_ADDUNKNOWN 1
#define IOPT_MSGSENT    2
#define IOPT_MSGRECV    4
#define IOPT_URLSENT    8
#define IOPT_URLRECV    16
#define IOPT_AUTHREQ    32
#define IOPT_ADDED      64
#define IOPT_FILESENT   128
#define IOPT_FILERECV   256
#define IOPT_OTHERSENT  512
#define IOPT_OTHERRECV  1024
#define IOPT_SYSTEM     2048
#define IOPT_CONTACTS   4096
#define IOPT_GROUPS     8192

void AddMessage(const TCHAR* fmt, ...);

INT_PTR CALLBACK WizardIntroPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MirandaPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MirandaOptionsPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MirandaAdvOptionsPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FinishedPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);

bool IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO dbei);

int CreateGroup(const TCHAR* name, MCONTACT hContact);

extern HINSTANCE hInst;
extern HWND hdlgProgress, hwndAccMerge;
extern int nImportOption;
extern int nCustomOptions;
extern TCHAR importFile[];
extern time_t dwSinceDate;

HICON  GetIcon(int iIconId);
HANDLE GetIconHandle(int iIconId);
void   RegisterIcons(void);
