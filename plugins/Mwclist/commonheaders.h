/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
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
#ifndef _COMMON_HEADERS_H_
#define _COMMON_HEADERS_H_ 1

#define MIRANDA_VER 0x0A00

#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

#include "m_stdhdr.h"

#include <windows.h>
#include <commctrl.h>
#include <vssym32.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include "resource.h"
#include <win2k.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_plugins.h>
#include "m_genmenu.h"
#include "m_clui.h"
#include "m_mwclc.h"
#include "clc.h"
#include "clist.h"
#include "m_icolib.h"
#include <m_userinfo.h>
#include "CLUIFrames/cluiframes.h"
#include "m_cluiframes.h"
#include "m_metacontacts.h"
#include "BkgrCfg.h"
#include <m_file.h>
#include <m_addcontact.h>
#include "m_fontservice.h"

// shared vars
extern HINSTANCE g_hInst;

/* most free()'s are invalid when the code is executed from a dll, so this changes
 all the bad free()'s to good ones, however it's still incorrect code. The reasons for not
 changing them include:

  * DBFreeVariant has a CallService() lookup
  * free() is executed in some large loops to do with clist creation of group data
  * easy search and replace

*/

extern struct LIST_INTERFACE li;

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

extern BOOL __cdecl strstri(const char *a, const char *b);
extern BOOL __cdecl boolstrcmpi(const char *a, const char *b);
extern int __cdecl MyStrCmp (const char *a, const char *b);
extern int __cdecl MyStrLen (const char *a);
extern int __cdecl MyStrCmpi(const char *a, const char *b);
extern int __cdecl MyStrCmpiT(const TCHAR *a, const TCHAR *b);

extern DWORD exceptFunction(LPEXCEPTION_POINTERS EP);

extern int     ( *saveIconFromStatusMode )( const char *szProto,int nStatus, HANDLE hContact);

//from bkg options

//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "BkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "BkgrCfg/Changed"



#define EXTRACOLUMNCOUNT 9


#endif
