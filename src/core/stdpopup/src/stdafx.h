// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <time.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_clistint.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_toptoolbar.h>
#include <m_utils.h>

#include <m_ieview.h> //need this for special renderers
#include <m_popup_int.h>

#include "version.h"
#include "message_pump.h"
#include "options.h"
#include "popwin.h"
#include "services.h"
#include "resource.h"
#include "yapp_history.h"

#define MODULENAME "YAPP"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	void InitIcons();

	int Load() override;
	int Unload() override;
};

extern bool bShutdown;

extern HFONT hFontFirstLine, hFontSecondLine, hFontTime;
extern COLORREF colFirstLine, colSecondLine, colBg, colTime, colBorder, colSidebar, colTitleUnderline;

// work around a bug in neweventnotify, possibly httpserver
// ignore the address passed to the 'get plugin data' service
extern bool ignore_gpd_passed_addy;
 
// win32 defines for mingw version of windows headers :(
#ifndef LVM_SORTITEMSEX
#define LVM_SORTITEMSEX          (LVM_FIRST + 81)

typedef int (CALLBACK *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);

#define ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
  (BOOL)SendMessage((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))
#endif

#define PDF_UNICODE  0x0001
#define PDF_ICOLIB   0x0002

void ShowPopup(PopupData &pd_in);
