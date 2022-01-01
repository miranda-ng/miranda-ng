#ifndef commonheaders_h__
#define commonheaders_h__

/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#pragma once
/*
#ifndef DB_USEHELPERFUNCTIONS		// to supress static inline db helpers
#define DB_NOHELPERFUNCTIONS
#endif
*/
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS		// to suppress secure warnings in VC2005
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE	// to suppress secure deprecate warnings in VC2005
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined (_DEBUG)
#define TRACE(str)  { log0(str); }
#else
#define TRACE(str)
#endif

#if defined (_DEBUG)
#define TRACEVAR(str,n) { log1(str,n); }
#else
#define TRACEVAR(str,n)
#endif

#if defined (_DEBUG)
#define TRACET(str) OutputDebugString(str)
#else
#define TRACET(str)
#endif

#define SERVICE(serviceproc)         static INT_PTR serviceproc(WPARAM wParam, LPARAM lParam)
#define EVENTHOOK(eventhookproc)     static int eventhookproc(WPARAM wParam, LPARAM lParam)
#define CLINTERFACE                  static

#include <windows.h>
#include <Shlwapi.h>
#include <vssym32.h>
#include <UxTheme.h>
#include <malloc.h>
#include <time.h>
#include <stddef.h>
#include <io.h>
#include <math.h>
#include <stdio.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_skin_eng.h>
#include <m_message.h>
#include <m_skinbutton.h>
#include <m_contacts.h>
#include <m_avatars.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_metacontacts.h>
#include <m_timezones.h>
#include <m_extraicons.h>
#include <m_xstatus.h>
#include <m_cluiframes.h>
#include <m_protoint.h>
#include <m_netlib.h>
#include <m_tipper.h>
#include <m_toptoolbar.h>
#include <m_metacontacts.h>
#include <m_variables.h>
#include <m_smileyadd.h>
#include <m_folders.h>
#include <m_seenplugin.h>

#define TEXT_TEXT_MAX_LENGTH 1024

#include "modern_global_structure.h"
#include "modern_defsettings.h"
#include "modern_clist.h"
#include "modern_clc.h"
#include "modern_row.h"
#include "modern_skinselector.h"
#include "modern_skinengine.h"
#include "modern_statusbar.h"
#include "cluiframes.h"
#include "modern_commonprototypes.h"
#include "modern_rowheight_funcs.h"
#include "modern_cache_funcs.h"
#include "modern_log.h"

#include "resource.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	ClcLineInfo secondLine, thirdLine;

	void ReadSettings();

	int Load() override;
	int Unload() override;
};

#define DEFAULT_SKIN_FOLDER		"Skins\\Modern contact list"
extern wchar_t SkinsFolder[MAX_PATH];

//macros to free data and set it pointer to nullptr
#define mir_free_and_nil(x) {mir_free((void*)x); x = 0;}
// shared vars

#define CLUI_FRAME_AUTOHIDENOTIFY  512
#define CLUI_FRAME_SHOWALWAYS      1024

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

#define MENU_MIRANDAMENU            0xFFFF1234
#define MENU_STATUSMENU             0xFFFF1235
#define MENU_MINIMIZE               0xFFFF1236

#define UM_CREATECLC                (WM_USER+1)
#define UM_SETALLEXTRAICONS         (WM_USER+2)
#define UM_UPDATE                   (WM_USER+50)

// Define constants for CLUI_SizingOnBorder SC_SIZE

#define SCF_NONE    0
#define SCF_LEFT    1
#define SCF_RIGHT   2
#define SCF_TOP     3
#define SCF_BOTTOM  6

char* __cdecl strstri(char *a, const char *b);

//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "ModernBkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "ModernBkgrCfg/Changed"

HBITMAP ske_CreateDIB32(int cx, int cy);

int CLUI_ShowWindowMod(HWND hwnd, int cmd);

void MakeButtonSkinned(HWND hWnd);

#ifndef LWA_COLORKEY
#define LWA_COLORKEY            0x00000001
#endif

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA            0x01
#endif

#define strsetA(a,b) {if (a) mir_free_and_nill(a); a=mir_strdup(b);}
#define strsetT(a,b) {if (a) mir_free_and_nill(a); a=mir_wstrdup(b);}

void TRACE_ERROR();

HICON LoadSmallIcon(HINSTANCE hInstance, int idx);
BOOL DestroyIcon_protect(HICON icon);

#ifndef ETDT_ENABLETAB
#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)
#endif

enum
{
	STATE_DLL_LOADING = 0,
	STATE_CLUI_LOADING,
	STATE_NORMAL,
	STATE_PREPARETOEXIT,
	STATE_EXITING
};

#define MirandaLoading() ((g_CluiData.bSTATE<STATE_NORMAL))
#define MirandaExiting() ((g_CluiData.bSTATE>STATE_NORMAL))

#define SORTBY_NAME	   0
#define SORTBY_STATUS  1
#define SORTBY_LASTMSG 2
#define SORTBY_PROTO   3
#define SORTBY_RATE    4
#define SORTBY_NAME_LOCALE 5
#define SORTBY_LAST_ONLINE 6
#define SORTBY_NOTHING	10

#define DT_FORCENATIVERENDER   0x10000000

#define _BOOL(a) (a != 0)

/* modern_animated_avatars.c */
int AniAva_InitModule();								   // HAVE TO BE AFTER GDI+ INITIALIZED
int AniAva_UnloadModule();
int AniAva_UpdateOptions();								   //reload options, //hot enable/disable engine

int AniAva_AddAvatar(MCONTACT hContact, wchar_t * szFilename, int width, int heigth);  // adds avatars to be displayed
int AniAva_SetAvatarPos(MCONTACT hContact, RECT *rc, int overlayIdx, uint8_t bAlpha);	   // update avatars pos
int AniAva_InvalidateAvatarPositions(MCONTACT hContact);	   // reset positions of avatars to be drawn (still be painted at same place)
int AniAva_RemoveInvalidatedAvatars();					   // all avatars without validated position will be stop painted and probably removed
int AniAva_RemoveAvatar(MCONTACT hContact);				   // remove avatar
int AniAva_RedrawAllAvatars(BOOL updateZOrder);			   // request to repaint all
void AniAva_UpdateParent();
int AniAva_RenderAvatar(MCONTACT hContact, HDC hdcMem, RECT *rc);

#define CCI_NAME            1
#define CCI_GROUP          (1<<1)
#define CCI_PROTO          (1<<2)
#define CCI_STATUS         (1<<3)
#define CCI_LINES          (1<<4)
#define CCI_HIDDEN         (1<<4)
#define CCI_NOHIDEOFFLINE  (1<<5)
#define CCI_NOPROTO        (1<<6)
#define CCI_HIDESUBCONTACT (1<<7)
#define CCI_I              (1<<8)
#define CCI_APPARENT       (1<<9)
#define CCI_NOTONLIST      (1<<10)
#define CCI_IDLETS         (1<<11)
#define CCI_CCONTACT       (1<<12)
#define CCI_EXPAND         (1<<13)
#define CCI_UNKNOWN        (1<<14)
#define CCI_TIME           (1<<15)
#define CCI_OTHER         ~( CCI_NAME|CCI_GROUP|CCI_PROTO|CCI_STATUS|CCI_LINES|CCI_TIME )
#define CCI_ALL            (0xFFFFFFFF)

/* move to list module */
typedef void(*ItemDestuctor)(void*);

const ROWCELL* rowAddCell(ROWCELL* &, int);
void rowDeleteTree(ROWCELL *cell);
BOOL rowParse(ROWCELL* &cell, ROWCELL* parent, char *tbuf, int &hbuf, int &sequence, ROWCELL** RowTabAccess);
void rowSizeWithReposition(ROWCELL* &root, int width);

#define UNPACK_POINT(X) { (short)LOWORD(X), (short)HIWORD(X) }

#endif // commonheaders_h__
