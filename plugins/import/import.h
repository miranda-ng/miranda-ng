/*

Import plugin for Miranda IM

Copyright (C) 2001,2002,2003,2004 Martin Öberg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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


//#define _LOGGING   1

#define MIRANDA_VER 0x1000

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

#include <m_stdhdr.h>

#include <windows.h>
#include <commctrl.h> // datetimepicker

#include <stddef.h>
#include <time.h>
#include <io.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_utils.h>
#include <m_findadd.h>
#include <m_clist.h>

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
#define SetProgress(n)  SendMessage(hdlgProgress,PROGM_SETPROGRESS,n,0)

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


struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;

void AddMessage( const char* fmt, ... );

int CreateGroup(BYTE type, const char* name, HANDLE hContact);

extern HWND hdlgProgress;

extern DWORD nDupes, nContactsCount, nMessagesCount, nGroupsCount, nSkippedEvents, nSkippedContacts;
