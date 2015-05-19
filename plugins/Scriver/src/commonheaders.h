/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#ifndef SRMM_COMMONHEADERS_H
#define SRMM_COMMONHEADERS_H

#define _CRT_SECURE_NO_WARNINGS

#define COMPILE_MULTIMON_STUBS

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <richedit.h>
#include <richole.h>

#include <malloc.h>
#include <math.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_avatars.h>
#include <m_hotkeys.h>
#include <m_popup.h>
#include <m_timezones.h>
#include <m_string.h>
#include <m_xstatus.h>
#include <win2k.h>

#include <m_ieview.h>
#include <m_smileyadd.h>
#include <m_metacontacts.h>

#include "resource.h"
#include "Version.h"
#include "infobar.h"
#include "cmdlist.h"
#include "sendqueue.h"
#include "msgs.h"
#include "globals.h"
#include "msgwindow.h"
#include "utils.h"
#include "input.h"
#include "richutil.h"
#include "statusicon.h"
#include "chat/chat.h"

#ifndef IMF_AUTOFONTSIZEADJUST
#define IMF_AUTOFONTSIZEADJUST	0x0010
#endif
#ifndef IMF_AUTOKEYBOARD
#define IMF_AUTOKEYBOARD		0x0001
#endif
#ifndef SES_EXTENDBACKCOLOR
#define SES_EXTENDBACKCOLOR	4
#endif
#ifndef ST_NEWCHARS
#define ST_NEWCHARS		4
#endif
#ifndef CFM_WEIGHT
#define	CFM_WEIGHT			0x00400000
#endif

extern HWND SM_FindWindowByContact(MCONTACT hContact);
extern HINSTANCE g_hInst;
extern void ChangeStatusIcons();
extern void LoadInfobarFonts();
extern HCURSOR hDragCursor;
extern ITaskbarList3 *pTaskbarInterface;
extern GlobalMessageData g_dat;
extern HMENU  g_hMenu;
extern HANDLE hHookWinPopup, hHookWinWrite;
extern HCURSOR hCurSplitNS, hCurSplitWE;

extern CREOleCallback reOleCallback;
extern CREOleCallback2 reOleCallback2;

#define SPLITTER_HEIGHT   4
#define TOOLBAR_HEIGHT   24
#define INFO_BAR_HEIGHT  54
#define INFO_BAR_INNER_HEIGHT (INFO_BAR_HEIGHT - 3)
#define INFO_BAR_AVATAR_HEIGHT INFO_BAR_INNER_HEIGHT
#define BOTTOM_RIGHT_AVATAR_HEIGHT 64
#define INFO_BAR_COLOR COLOR_INACTIVEBORDER

int Chat_Load();
int Chat_Unload();
int Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam);
int OptInitialise(WPARAM wParam, LPARAM lParam);
int FontServiceFontsChanged(WPARAM wParam, LPARAM lParam);
int StatusIconPressed(WPARAM wParam, LPARAM lParam);

#endif
