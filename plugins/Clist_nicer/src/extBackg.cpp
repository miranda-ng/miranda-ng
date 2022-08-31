/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "stdafx.h"
#include "coolscroll.h"

extern int g_hottrack;

extern HWND g_hwndViewModeFrame;
extern struct CluiTopButton BTNS[];

LIST<StatusItems_t> arStatusItems(10);
ImageItem *g_ImageItems = nullptr, *g_glyphItem = nullptr;
ButtonItem *g_ButtonItems = nullptr;
ImageItem *g_CLUIImageItem = nullptr;
HBRUSH g_CLUISkinnedBkColor = nullptr;
COLORREF g_CLUISkinnedBkColorRGB = 0;

int ID_EXTBK_LAST = ID_EXTBK_LAST_D;

//void SetTBSKinned(int mode);
void ReloadThemedOptions();
static void SaveCompleteStructToDB(void);

static StatusItems_t _StatusItems[] = {
	{"Offline", "EXBK_Offline", ID_STATUS_OFFLINE,
	CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Online", "EXBK_Online", ID_STATUS_ONLINE,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Away", "EXBK_Away", ID_STATUS_AWAY,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"DND", "EXBK_Dnd", ID_STATUS_DND,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"N/A", "EXBK_NA", ID_STATUS_NA,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Occupied", "EXBK_Occupied", ID_STATUS_OCCUPIED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Free for chat", "EXBK_FFC", ID_STATUS_FREECHAT,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Invisible", "EXBK_Invisible", ID_STATUS_INVISIBLE,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Idle", "EXBK_IDLE", ID_EXTBKIDLE,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Expanded Group", "EXBK_EXPANDEDGROUPS", ID_EXTBKEXPANDEDGROUP,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Collapsed Group", "EXBK_COLLAPSEDGROUP", ID_EXTBKCOLLAPSEDDGROUP,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Empty Group", "EXBK_EMPTYGROUPS", ID_EXTBKEMPTYGROUPS,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"{-}First contact of a group", "EXBK_FIRSTITEM", ID_EXTBKFIRSTITEM,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Single item in group", "EXBK_SINGLEITEM", ID_EXTBKSINGLEITEM,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Last contact of a group", "EXBK_LASTITEM", ID_EXTBKLASTITEM,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"{-}First contact of NON-group", "EXBK_FIRSTITEM_NG", ID_EXTBKFIRSTITEM_NG,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Single item in NON-group", "EXBK_SINGLEITEM_NG", ID_EXTBKSINGLEITEM_NG,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Last contact of NON-group", "EXBK_LASTITEM_NG", ID_EXTBKLASTITEM_NG,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"{-}Even Contact Positions", "EXBK_EVEN_CNTC_POS", ID_EXTBKEVEN_CNTCTPOS,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Odd Contact Positions", "EXBK_ODD_CNTC_POS", ID_EXTBKODD_CNTCTPOS,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"{-}Selection", "EXBK_SELECTION", ID_EXTBKSELECTION,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Hottracked Item", "EXBK_HOTTRACK", ID_EXTBKHOTTRACK,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Frame titlebars", "EXBK_FRAMETITLE", ID_EXTBKFRAMETITLE,
	CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 0
	}, {"Event area", "EXBK_EVTAREA", ID_EXTBKEVTAREA,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 0
	}, {"Status Bar", "EXBK_STATUSBAR", ID_EXTBKSTATUSBAR,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Tool bar", "EXBK_TOOLBAR", ID_EXTBKBUTTONBAR,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 0
	}, {"{-}UI Button - pressed", "EXBK_BUTTONSPRESSED", ID_EXTBKBUTTONSPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"UI Button - not pressed", "EXBK_BUTTONSNPRESSED", ID_EXTBKBUTTONSNPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"UI Button - mouseover", "EXBK_BUTTONSMOUSEOVER", ID_EXTBKBUTTONSMOUSEOVER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Toolbar button - pressed", "EXBK_TBBUTTONSPRESSED", ID_EXTBKTBBUTTONSPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Toolbar button - not pressed", "EXBK_TBBUTTONSNPRESSED", ID_EXTBKTBBUTTONSNPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"Toolbar button - mouseover", "EXBK_TBBUTTONMOUSEOVER", ID_EXTBKTBBUTTONMOUSEOVER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Status floater", "EXBK_STATUSFLOATER", ID_EXTBKSTATUSFLOATER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Frame border - no titlebar", "EXBK_OWNEDFRAMEBORDER", ID_EXTBKOWNEDFRAMEBORDER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, 1,
	1, 1, 1, CLCDEFAULT_IGNORE
	}, {"Frame border - with titlebar", "EXBK_OWNEDFRAMEBORDERTB", ID_EXTBKOWNEDFRAMEBORDERTB,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, 1,
	CLCDEFAULT_MRGN_TOP, 1, 1, CLCDEFAULT_IGNORE
	}, {"{-}Avatarframe", "EXBK_AVATARFRAME", ID_EXTBKAVATARFRAME,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Avatarframe offline", "EXBK_AVATARFRAMEOFFLINE", ID_EXTBKAVATARFRAMEOFFLINE,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"{-}Scrollbar backgroundupper", "EXBK_SCROLLBACK", ID_EXTBKSCROLLBACK,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar backgroundlower", "EXBK_SCROLLBACKLOWER", ID_EXTBKSCROLLBACKLOWER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar thumb", "EXBK_SCROLLTHUMB", ID_EXTBKSCROLLTHUMB,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar thumb hovered", "EXBK_SCROLLTHUMBHOVER", ID_EXTBKSCROLLTHUMBHOVER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar thumb pressed", "EXBK_SCROLLTHUMBPRESSED", ID_EXTBKSCROLLTHUMBPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar button", "EXBK_SCROLLBUTTON", ID_EXTBKSCROLLBUTTON,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar button hovered", "EXBK_SCROLLBUTTONHOVER", ID_EXTBKSCROLLBUTTONHOVER,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar button pressed", "EXBK_SCROLLBUTTONPRESSED", ID_EXTBKSCROLLBUTTONPRESSED,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar arrow up", "EXBK_SCROLLARROWUP", ID_EXTBKSCROLLARROWUP,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}, {"Scrollbar arrow down", "EXBK_SCROLLARROWDOWN", ID_EXTBKSCROLLARROWDOWN,
	CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
	CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
	}
};

BOOL __forceinline GetItemByStatus(int status, StatusItems_t *retitem)
{
	int idx = status - ID_STATUS_OFFLINE; // better check the index...
	if (idx < 0 || idx >= arStatusItems.getCount())
		idx = 0;

	*retitem = *arStatusItems[idx];
	if (g_hottrack && status != ID_EXTBKHOTTRACK) // allow hottracking for ignored items, unless hottrack item itself should be ignored
		retitem->IGNORED = FALSE;
	return TRUE;
}

StatusItems_t *GetProtocolStatusItem(const char *szProto)
{
	if (szProto == nullptr)
		return nullptr;

	for (int i = _countof(_StatusItems); i < arStatusItems.getCount(); i++) {
		StatusItems_t *p = arStatusItems[i];
		if (!mir_strcmp(p->szName[0] == '{' ? p->szName+3 : p->szName, szProto))
			return p;
	}
	return nullptr;
}

int CoolSB_SetupScrollBar()
{
	/*
	* a skinned scrollbar is only valid when ALL items are skinned with image items
	* and no item is set to ignored
	*/

	cfg::dat.bSkinnedScrollbar = !arStatusItems[ID_EXTBKSCROLLBACK - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLBACKLOWER - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLTHUMB - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLTHUMBHOVER - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLTHUMBPRESSED - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLBUTTON - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLBUTTONHOVER - ID_STATUS_OFFLINE]->IGNORED &&
		!arStatusItems[ID_EXTBKSCROLLBUTTONPRESSED - ID_STATUS_OFFLINE]->IGNORED;

	if (!arStatusItems[ID_EXTBKSCROLLBACK - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLBACKLOWER - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLTHUMB - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLTHUMBHOVER - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLTHUMBPRESSED - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLBUTTON - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLBUTTONHOVER - ID_STATUS_OFFLINE]->imageItem ||
		!arStatusItems[ID_EXTBKSCROLLBUTTONPRESSED - ID_STATUS_OFFLINE]->imageItem)
		cfg::dat.bSkinnedScrollbar = FALSE;

	if (db_get_b(0, "CLC", "NoVScrollBar", 0)) {
		UninitializeCoolSB(g_clistApi.hwndContactTree);
		return 0;
	}

	if (cfg::dat.bSkinnedScrollbar) {
		InitializeCoolSB(g_clistApi.hwndContactTree);
		CoolSB_SetStyle(g_clistApi.hwndContactTree, SB_VERT, CSBS_HOTTRACKED);
	}
	else UninitializeCoolSB(g_clistApi.hwndContactTree);
	return 0;
}

// fills the struct with the settings in the database
void LoadExtBkSettingsFromDB()
{
	for (auto &it : _StatusItems) {
		StatusItems_t *p = (StatusItems_t*)mir_alloc(sizeof(StatusItems_t));
		*p = it;
		arStatusItems.insert(p);
	}

	auto &accs = Accounts();
	for (auto &pa : accs) {
		StatusItems_t *p = (StatusItems_t*)mir_alloc(sizeof(StatusItems_t));
		*p = _StatusItems[0];
		ID_EXTBK_LAST++;

		mir_snprintf(p->szDBname, "EXBK_%s", pa->szModuleName);
		if (accs.indexOf(&pa) == 0)
			mir_snprintf(p->szName, "{-}%s", pa->szModuleName);
		else
			strncpy_s(p->szName, pa->szModuleName, _TRUNCATE);
		p->statusID = ID_EXTBK_LAST;
		arStatusItems.insert(p);
	}

	for (auto &p : arStatusItems) {
		if (p->statusID == ID_EXTBKSEPARATOR)
			continue;

		p->imageItem = nullptr;
		CMStringA buffer;
		buffer.Format("%s_IGNORE", p->szDBname);
		p->IGNORED = (uint8_t)db_get_b(0, "CLCExt", buffer, p->IGNORED);
		buffer.Format("%s_GRADIENT", p->szDBname);
		p->GRADIENT = (uint8_t)db_get_dw(0, "CLCExt", buffer, p->GRADIENT);
		buffer.Format("%s_CORNER", p->szDBname);
		p->CORNER = (uint8_t)db_get_dw(0, "CLCExt", buffer, p->CORNER);
		buffer.Format("%s_COLOR", p->szDBname);
		p->COLOR = db_get_dw(0, "CLCExt", buffer, p->COLOR);
		buffer.Format("%s_COLOR2", p->szDBname);
		p->COLOR2 = db_get_dw(0, "CLCExt", buffer, p->COLOR2);
		buffer.Format("%s_COLOR2_TRANSPARENT", p->szDBname);
		p->COLOR2_TRANSPARENT = (uint8_t)db_get_b(0, "CLCExt", buffer, p->COLOR2_TRANSPARENT);
		buffer.Format("%s_TEXTCOLOR", p->szDBname);
		p->TEXTCOLOR = db_get_dw(0, "CLCExt", buffer, p->TEXTCOLOR);
		buffer.Format("%s_ALPHA", p->szDBname);
		p->ALPHA = db_get_b(0, "CLCExt", buffer, p->ALPHA);
		buffer.Format("%s_MRGN_LEFT", p->szDBname);
		p->MARGIN_LEFT = db_get_b(0, "CLCExt", buffer, p->MARGIN_LEFT);
		buffer.Format("%s_MRGN_TOP", p->szDBname);
		p->MARGIN_TOP = db_get_b(0, "CLCExt", buffer, p->MARGIN_TOP);
		buffer.Format("%s_MRGN_RIGHT", p->szDBname);
		p->MARGIN_RIGHT = db_get_b(0, "CLCExt", buffer, p->MARGIN_RIGHT);
		buffer.Format("%s_MRGN_BOTTOM", p->szDBname);
		p->MARGIN_BOTTOM = db_get_b(0, "CLCExt", buffer, p->MARGIN_BOTTOM);
		buffer.Format("%s_BDRSTYLE", p->szDBname);
		p->BORDERSTYLE = db_get_dw(0, "CLCExt", buffer, p->BORDERSTYLE);
	}

	if (cfg::dat.bFirstRun) {
		StatusItems_t *item = arStatusItems[ID_EXTBKBUTTONBAR - ID_STATUS_OFFLINE];

		item->COLOR = GetSysColor(COLOR_3DFACE);
		item->COLOR2 = GetSysColor(COLOR_3DFACE);

		item = arStatusItems[ID_EXTBKEVTAREA - ID_STATUS_OFFLINE];
		item->COLOR = item->COLOR2 = GetSysColor(COLOR_WINDOW);
		item->BORDERSTYLE = EDGE_ETCHED;
		SaveCompleteStructToDB();
	}
}

// writes whole struct to the database
static void SaveCompleteStructToDB()
{
	for (auto &p : arStatusItems) {
		if (p->statusID != ID_EXTBKSEPARATOR) {
			char buffer[255];
			mir_snprintf(buffer, "%s_IGNORE", p->szDBname);
			db_set_b(0, "CLCExt", buffer, p->IGNORED);
			mir_snprintf(buffer, "%s_GRADIENT", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->GRADIENT);
			mir_snprintf(buffer, "%s_CORNER", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->CORNER);
			mir_snprintf(buffer, "%s_COLOR", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->COLOR);
			mir_snprintf(buffer, "%s_COLOR2", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->COLOR2);
			mir_snprintf(buffer, "%s_COLOR2_TRANSPARENT", p->szDBname);
			db_set_b(0, "CLCExt", buffer, p->COLOR2_TRANSPARENT);
			mir_snprintf(buffer, "%s_TEXTCOLOR", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->TEXTCOLOR);
			mir_snprintf(buffer, "%s_ALPHA", p->szDBname);
			db_set_b(0, "CLCExt", buffer, (uint8_t)p->ALPHA);
			mir_snprintf(buffer, "%s_MRGN_LEFT", p->szDBname);
			db_set_b(0, "CLCExt", buffer, (uint8_t)p->MARGIN_LEFT);
			mir_snprintf(buffer, "%s_MRGN_TOP", p->szDBname);
			db_set_b(0, "CLCExt", buffer, (uint8_t)p->MARGIN_TOP);
			mir_snprintf(buffer, "%s_MRGN_RIGHT", p->szDBname);
			db_set_b(0, "CLCExt", buffer, (uint8_t)p->MARGIN_RIGHT);
			mir_snprintf(buffer, "%s_MRGN_BOTTOM", p->szDBname);
			db_set_b(0, "CLCExt", buffer, (uint8_t)p->MARGIN_BOTTOM);
			mir_snprintf(buffer, "%s_BDRSTYLE", p->szDBname);
			db_set_dw(0, "CLCExt", buffer, p->BORDERSTYLE);
		}
	}
}

void SetButtonToSkinned()
{
	bool bSkinned = (cfg::dat.bSkinnedButtonMode = db_get_b(0, "CLCExt", "bskinned", 0)) != 0;
	bool bFlat = bSkinned || (db_get_b(0, "TopToolBar", "UseFlatButton", 0) != 0);

	for (int i = 0;; i++) {
		if (BTNS[i].pszButtonID == nullptr)
			break;
		if (BTNS[i].hwndButton != nullptr && BTNS[i].ctrlid != IDC_TBGLOBALSTATUS && BTNS[i].ctrlid != IDC_TBMENU)
			CustomizeButton(BTNS[i].hwndButton, bSkinned, !bSkinned, bFlat, true);
	}

	CustomizeButton(GetDlgItem(g_clistApi.hwndContactList, IDC_TBMENU), bSkinned, !bSkinned, bSkinned);
	CustomizeButton(GetDlgItem(g_clistApi.hwndContactList, IDC_TBGLOBALSTATUS), bSkinned, !bSkinned, bSkinned);
	SendMessage(g_hwndViewModeFrame, WM_USER + 100, 0, 0);
}

void Reload3dBevelColors()
{
	if (cfg::dat.hPen3DBright)
		DeleteObject(cfg::dat.hPen3DBright);
	if (cfg::dat.hPen3DDark)
		DeleteObject(cfg::dat.hPen3DDark);

	cfg::dat.hPen3DBright = CreatePen(PS_SOLID, 1, db_get_dw(0, "CLCExt", "3dbright", GetSysColor(COLOR_3DLIGHT)));
	cfg::dat.hPen3DDark = CreatePen(PS_SOLID, 1, db_get_dw(0, "CLCExt", "3ddark", GetSysColor(COLOR_3DSHADOW)));
}

// Save Non-arStatusItems Settings
void SaveNonStatusItemsSettings(HWND hwndDlg)
{
	BOOL translated;

	db_set_b(0, "CLCExt", "EXBK_EqualSelection", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_EQUALSELECTION));
	db_set_b(0, "CLCExt", "EXBK_SelBlend", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SELBLEND));

	cfg::dat.cornerRadius = GetDlgItemInt(hwndDlg, IDC_CORNERRAD, &translated, FALSE);
	cfg::dat.bApplyIndentToBg = IsDlgButtonChecked(hwndDlg, IDC_APPLYINDENTBG) ? 1 : 0;
	cfg::dat.bUsePerProto = IsDlgButtonChecked(hwndDlg, IDC_USEPERPROTO) ? 1 : 0;
	cfg::dat.bWantFastGradients = IsDlgButtonChecked(hwndDlg, IDC_FASTGRADIENT) ? 1 : 0;
	cfg::dat.bOverridePerStatusColors = IsDlgButtonChecked(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR) ? 1 : 0;
	cfg::dat.titleBarHeight = (uint8_t)GetDlgItemInt(hwndDlg, IDC_LASTITEMPADDING, &translated, FALSE);
	cfg::dat.group_padding = GetDlgItemInt(hwndDlg, IDC_GRPTOPPADDING, &translated, FALSE);

	db_set_b(0, "CLCExt", "CornerRad", cfg::dat.cornerRadius);
	db_set_b(0, "CLCExt", "applyindentbg", (uint8_t)cfg::dat.bApplyIndentToBg);
	db_set_b(0, "CLCExt", "useperproto", (uint8_t)cfg::dat.bUsePerProto);
	db_set_b(0, "CLCExt", "override_status", (uint8_t)cfg::dat.bOverridePerStatusColors);
	db_set_b(0, "CLCExt", "bskinned", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_SETALLBUTTONSKINNED) ? 1 : 0));
	db_set_b(0, "CLCExt", "FastGradients", cfg::dat.bWantFastGradients);
	db_set_b(0, "CLC", "IgnoreSelforGroups", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_IGNORESELFORGROUPS));

	db_set_dw(0, "CLCExt", "grp_padding", cfg::dat.group_padding);
	db_set_b(0, "CLCExt", "frame_height", cfg::dat.titleBarHeight);

	Reload3dBevelColors();
	SetButtonToSkinned();
}

// skin/theme related settings which are exported to/imported from the .ini style .clist file
struct { char *szModule; char *szSetting; unsigned int size; int defaultval; } _tagSettings[] = {
	"CLCExt", "3dbright", 4, RGB(224, 225, 225),
	"CLCExt", "3ddark", 4, RGB(224, 225, 225),
	"CLCExt", "bskinned", 1, 0,
	"CLCExt", "CornerRad", 1, 0,
	"CLCExt", "applyindentbg", 1, 0,
	"CLCExt", "override_status", 1, 0,
	"CLCExt", "useperproto", 1, 0,
	"CLUI", "sb_skinned", 1, 0,
	"CLC", "RowGap", 1, 0,
	"CLC", "ExIconScale", 1, 0,
	"CLUI", "UseBkSkin", 1, 0,
	"CLUI", "clipborder", 1, 0,
	"CLUIFrames", "GapBetweenFrames", 4, 0,
	"CLC", "BkColour", 4, RGB(224, 224, 224),
	"CLCExt", "EXBK_CenterGroupnames", 1, 0,
	"CLC", "BkBmpUse", 2, 0,
	"CLUI", "clmargins", 4, 0,
	// frame stuff

	"WorldTime", "BgColour", 4, 0,
	"WorldTime", "FontCol", 4, 0,
	nullptr, nullptr, 0, 0
};

void extbk_export(char *file)
{
	int i, n;
	CMStringA buffer;
	char szSection[255];
	char szKey[255];
	DBVARIANT dbv = { 0 };
	uint32_t data;

	data = 3;

	WritePrivateProfileStructA("Global", "Version", &data, 4, file);
	for (auto &p : arStatusItems) {
		if (p->statusID == ID_EXTBKSEPARATOR)
			continue;

		buffer.Format("%s_ALPHA", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->ALPHA), sizeof(p->ALPHA), file);
		buffer.Format("%s_COLOR", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR), sizeof(p->COLOR), file);
		buffer.Format("%s_COLOR2", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR2), sizeof(p->COLOR2), file);
		buffer.Format("%s_COLOR2_TRANSPARENT", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR2_TRANSPARENT), sizeof(p->COLOR2_TRANSPARENT), file);
		buffer.Format("%s_TEXTCOLOR", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->TEXTCOLOR), sizeof(p->TEXTCOLOR), file);
		buffer.Format("%s_CORNER", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->CORNER), sizeof(p->CORNER), file);
		buffer.Format("%s_GRADIENT", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->GRADIENT), sizeof(p->GRADIENT), file);
		buffer.Format("%s_IGNORED", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->IGNORED), sizeof(p->IGNORED), file);
		buffer.Format("%s_MARGIN_BOTTOM", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_BOTTOM), sizeof(p->MARGIN_BOTTOM), file);
		buffer.Format("%s_MARGIN_LEFT", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_LEFT), sizeof(p->MARGIN_LEFT), file);
		buffer.Format("%s_MARGIN_RIGHT", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_RIGHT), sizeof(p->MARGIN_RIGHT), file);
		buffer.Format("%s_MARGIN_TOP", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_TOP), sizeof(p->MARGIN_TOP), file);
		buffer.Format("%s_BORDERSTYLE", p->szDBname);
		WritePrivateProfileStructA("ExtBKSettings", buffer, &(p->BORDERSTYLE), sizeof(p->BORDERSTYLE), file);
	}

	for (n = 0; n <= FONTID_LAST; n++) {
		mir_snprintf(szSection, "Font%d", n);

		mir_snprintf(szKey, "Font%dName", n);
		if (!db_get_s(0, "CLC", szKey, &dbv)) {
			WritePrivateProfileStringA(szSection, "Name", dbv.pszVal, file);
			mir_free(dbv.pszVal);
		}
		mir_snprintf(szKey, "Font%dSize", n);
		data = (uint32_t)db_get_b(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "Size", &data, 1, file);

		mir_snprintf(szKey, "Font%dSty", n);
		data = (uint32_t)db_get_b(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "Style", &data, 1, file);

		mir_snprintf(szKey, "Font%dSet", n);
		data = (uint32_t)db_get_b(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "Set", &data, 1, file);

		mir_snprintf(szKey, "Font%dCol", n);
		data = db_get_dw(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "Color", &data, 4, file);

		mir_snprintf(szKey, "Font%dFlags", n);
		data = (uint32_t)db_get_dw(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "Flags", &data, 4, file);

		mir_snprintf(szKey, "Font%dAs", n);
		data = (uint32_t)db_get_w(0, "CLC", szKey, 8);
		WritePrivateProfileStructA(szSection, "SameAs", &data, 2, file);
	}

	i = 0;
	while (_tagSettings[i].szModule != nullptr) {
		data = 0;
		switch (_tagSettings[i].size) {
		case 1:
			data = (uint32_t)db_get_b(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, (uint8_t)_tagSettings[i].defaultval);
			break;
		case 2:
			data = (uint32_t)db_get_w(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, (uint32_t)_tagSettings[i].defaultval);
			break;
		case 4:
			data = (uint32_t)db_get_dw(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, (uint32_t)_tagSettings[i].defaultval);
			break;
		}
		WritePrivateProfileStructA("Global", _tagSettings[i].szSetting, &data, _tagSettings[i].size, file);
		i++;
	}

	if (!db_get_s(0, "CLC", "BkBitmap", &dbv)) {
		WritePrivateProfileStringA("Global", "BkBitmap", dbv.pszVal, file);
		db_free(&dbv);
	}
}

uint32_t __fastcall HexStringToLong(const char *szSource)
{
	char *stopped;
	COLORREF clr = strtol(szSource, &stopped, 16);
	if (clr == -1)
		return clr;
	return (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
}

static StatusItems_t default_item = {
	"{--Contact--}", "", 0,
	CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
	CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, uint32_t(-1),
	CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT, CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT,
	CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
};

static void PreMultiply(HBITMAP hBitmap, int mode)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	int width = bmp.bmWidth;
	int height = bmp.bmHeight;
	uint32_t dwLen = width * height * 4;
	uint8_t *p = (uint8_t *)malloc(dwLen);
	if (p == nullptr)
		return;

	GetBitmapBits(hBitmap, dwLen, p);
	for (int y = 0; y < height; ++y) {
		uint8_t *px = p + width * 4 * y;

		for (int x = 0; x < width; ++x) {
			if (mode) {
				uint8_t alpha = px[3];
				px[0] = px[0] * alpha / 255;
				px[1] = px[1] * alpha / 255;
				px[2] = px[2] * alpha / 255;
			}
			else px[3] = 255;
			px += 4;
		}
	}
	SetBitmapBits(hBitmap, dwLen, p);
	free(p);
}

static void CorrectBitmap32Alpha(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel != 32)
		return;

	uint32_t dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	uint8_t *p = (uint8_t*)calloc(1, dwLen);
	if (p == nullptr)
		return;

	GetBitmapBits(hBitmap, dwLen, p);

	BOOL fixIt = TRUE;
	for (int y = 0; y < bmp.bmHeight; ++y) {
		uint8_t *px = p + bmp.bmWidth * 4 * y;

		for (int x = 0; x < bmp.bmWidth; ++x) {
			if (px[3] != 0)
				fixIt = FALSE;
			else
				px[3] = 255;
			px += 4;
		}
	}

	if (fixIt)
		SetBitmapBits(hBitmap, bmp.bmWidth * bmp.bmHeight * 4, p);

	free(p);
}

static HBITMAP LoadPNG(const char *szFilename)
{
	HBITMAP hBitmap = Bitmap_Load(_A2T(szFilename));
	if (hBitmap != nullptr)
		CorrectBitmap32Alpha(hBitmap);

	return hBitmap;
}

static void IMG_CreateItem(ImageItem *item, const char *fileName, HDC hdc)
{
	HBITMAP hbm = LoadPNG(fileName);
	if (hbm) {
		item->hbm = hbm;
		item->bf.BlendFlags = 0;
		item->bf.BlendOp = AC_SRC_OVER;
		item->bf.AlphaFormat = 0;

		BITMAP bm;
		GetObject(hbm, sizeof(bm), &bm);
		if (bm.bmBitsPixel == 32) {
			PreMultiply(hbm, 1);
			item->dwFlags |= IMAGE_PERPIXEL_ALPHA;
			item->bf.AlphaFormat = AC_SRC_ALPHA;
		}
		item->width = bm.bmWidth;
		item->height = bm.bmHeight;
		item->inner_height = item->height - item->bTop - item->bBottom;
		item->inner_width = item->width - item->bLeft - item->bRight;
		if (item->bTop && item->bBottom && item->bLeft && item->bRight) {
			item->dwFlags |= IMAGE_FLAG_DIVIDED;
			if (item->inner_height <= 0 || item->inner_width <= 0) {
				DeleteObject(hbm);
				item->hbm = nullptr;
				return;
			}
		}
		item->hdc = CreateCompatibleDC(hdc);
		item->hbmOld = reinterpret_cast<HBITMAP>(SelectObject(item->hdc, item->hbm));
	}
}

static void IMG_DeleteItem(ImageItem *item)
{
	if (!(item->dwFlags & IMAGE_GLYPH)) {
		SelectObject(item->hdc, item->hbmOld);
		DeleteObject(item->hbm);
		DeleteDC(item->hdc);
	}
	if (item->fillBrush)
		DeleteObject(item->fillBrush);
}

static void ReadItem(StatusItems_t *this_item, char *szItem, char *file)
{
	char buffer[512], def_color[20];
	COLORREF clr;

	StatusItems_t *defaults = &default_item;
	GetPrivateProfileStringA(szItem, "BasedOn", "None", buffer, 400, file);


	if (mir_strcmp(buffer, "None")) {
		for (auto &p : arStatusItems) {
			if (!_stricmp(p->szName[0] == '{' ? p->szName + 3 : p->szName, buffer)) {
				defaults = p;
				break;
			}
		}
	}

	this_item->ALPHA = (int)GetPrivateProfileIntA(szItem, "Alpha", defaults->ALPHA, file);
	this_item->ALPHA = min(this_item->ALPHA, 100);

	clr = RGB(GetBValue(defaults->COLOR), GetGValue(defaults->COLOR), GetRValue(defaults->COLOR));
	mir_snprintf(def_color, "%6.6x", clr);
	GetPrivateProfileStringA(szItem, "Color1", def_color, buffer, 400, file);
	this_item->COLOR = HexStringToLong(buffer);

	clr = RGB(GetBValue(defaults->COLOR2), GetGValue(defaults->COLOR2), GetRValue(defaults->COLOR2));
	mir_snprintf(def_color, "%6.6x", clr);
	GetPrivateProfileStringA(szItem, "Color2", def_color, buffer, 400, file);
	this_item->COLOR2 = HexStringToLong(buffer);

	this_item->COLOR2_TRANSPARENT = (uint8_t)GetPrivateProfileIntA(szItem, "COLOR2_TRANSPARENT", defaults->COLOR2_TRANSPARENT, file);

	this_item->CORNER = defaults->CORNER & CORNER_ACTIVE ? defaults->CORNER : 0;
	GetPrivateProfileStringA(szItem, "Corner", "None", buffer, 400, file);
	if (strstr(buffer, "tl"))
		this_item->CORNER |= CORNER_TL;
	if (strstr(buffer, "tr"))
		this_item->CORNER |= CORNER_TR;
	if (strstr(buffer, "bl"))
		this_item->CORNER |= CORNER_BL;
	if (strstr(buffer, "br"))
		this_item->CORNER |= CORNER_BR;
	if (this_item->CORNER)
		this_item->CORNER |= CORNER_ACTIVE;

	this_item->GRADIENT = defaults->GRADIENT & GRADIENT_ACTIVE ? defaults->GRADIENT : 0;
	GetPrivateProfileStringA(szItem, "Gradient", "None", buffer, 400, file);
	if (strstr(buffer, "left"))
		this_item->GRADIENT = GRADIENT_RL;
	else if (strstr(buffer, "right"))
		this_item->GRADIENT = GRADIENT_LR;
	else if (strstr(buffer, "up"))
		this_item->GRADIENT = GRADIENT_BT;
	else if (strstr(buffer, "down"))
		this_item->GRADIENT = GRADIENT_TB;
	if (this_item->GRADIENT)
		this_item->GRADIENT |= GRADIENT_ACTIVE;

	this_item->MARGIN_LEFT = GetPrivateProfileIntA(szItem, "Left", defaults->MARGIN_LEFT, file);
	this_item->MARGIN_RIGHT = GetPrivateProfileIntA(szItem, "Right", defaults->MARGIN_RIGHT, file);
	this_item->MARGIN_TOP = GetPrivateProfileIntA(szItem, "Top", defaults->MARGIN_TOP, file);
	this_item->MARGIN_BOTTOM = GetPrivateProfileIntA(szItem, "Bottom", defaults->MARGIN_BOTTOM, file);
	this_item->BORDERSTYLE = GetPrivateProfileIntA(szItem, "Borderstyle", defaults->BORDERSTYLE, file);

	GetPrivateProfileStringA(szItem, "Textcolor", "ffffffff", buffer, 400, file);
	this_item->TEXTCOLOR = HexStringToLong(buffer);
}

void IMG_ReadItem(const char *itemname, const char *szFileName)
{
	ImageItem tmpItem, *newItem = nullptr;
	char buffer[512], szItemNr[30];
	char szFinalName[MAX_PATH];
	HDC hdc = GetDC(g_clistApi.hwndContactList);
	int n;
	BOOL alloced = FALSE;
	char szDrive[MAX_PATH], szPath[MAX_PATH];

	memset(&tmpItem, 0, sizeof(ImageItem));
	GetPrivateProfileStringA(itemname, "Glyph", "None", buffer, 500, szFileName);
	if (mir_strcmp(buffer, "None")) {
		sscanf(buffer, "%d,%d,%d,%d", &tmpItem.glyphMetrics[0], &tmpItem.glyphMetrics[1],
			&tmpItem.glyphMetrics[2], &tmpItem.glyphMetrics[3]);
		if (tmpItem.glyphMetrics[2] > tmpItem.glyphMetrics[0] && tmpItem.glyphMetrics[3] > tmpItem.glyphMetrics[1]) {
			tmpItem.dwFlags |= IMAGE_GLYPH;
			tmpItem.glyphMetrics[2] = (tmpItem.glyphMetrics[2] - tmpItem.glyphMetrics[0]) + 1;
			tmpItem.glyphMetrics[3] = (tmpItem.glyphMetrics[3] - tmpItem.glyphMetrics[1]) + 1;
			goto done_with_glyph;
		}
	}
	GetPrivateProfileStringA(itemname, "Image", "None", buffer, 500, szFileName);
	if (mir_strcmp(buffer, "None")) {

done_with_glyph:

		strncpy(tmpItem.szName, &itemname[1], sizeof(tmpItem.szName));
		tmpItem.szName[sizeof(tmpItem.szName) - 1] = 0;
		_splitpath(szFileName, szDrive, szPath, nullptr, nullptr);
		mir_snprintf(szFinalName, "%s\\%s\\%s", szDrive, szPath, buffer);
		tmpItem.alpha = GetPrivateProfileIntA(itemname, "Alpha", 100, szFileName);
		tmpItem.alpha = min(tmpItem.alpha, 100);
		tmpItem.alpha = (uint8_t)((FLOAT)(((FLOAT)tmpItem.alpha) / 100) * 255);
		tmpItem.bf.SourceConstantAlpha = tmpItem.alpha;
		tmpItem.bLeft = GetPrivateProfileIntA(itemname, "Left", 0, szFileName);
		tmpItem.bRight = GetPrivateProfileIntA(itemname, "Right", 0, szFileName);
		tmpItem.bTop = GetPrivateProfileIntA(itemname, "Top", 0, szFileName);
		tmpItem.bBottom = GetPrivateProfileIntA(itemname, "Bottom", 0, szFileName);
		if (tmpItem.dwFlags & IMAGE_GLYPH) {
			tmpItem.width = tmpItem.glyphMetrics[2];
			tmpItem.height = tmpItem.glyphMetrics[3];
			tmpItem.inner_height = tmpItem.glyphMetrics[3] - tmpItem.bTop - tmpItem.bBottom;
			tmpItem.inner_width = tmpItem.glyphMetrics[2] - tmpItem.bRight - tmpItem.bLeft;

			if (tmpItem.bTop && tmpItem.bBottom && tmpItem.bLeft && tmpItem.bRight)
				tmpItem.dwFlags |= IMAGE_FLAG_DIVIDED;
			tmpItem.bf.BlendFlags = 0;
			tmpItem.bf.BlendOp = AC_SRC_OVER;
			tmpItem.bf.AlphaFormat = 0;
			tmpItem.dwFlags |= IMAGE_PERPIXEL_ALPHA;
			tmpItem.bf.AlphaFormat = AC_SRC_ALPHA;
			if (tmpItem.inner_height <= 0 || tmpItem.inner_width <= 0) {
				ReleaseDC(g_clistApi.hwndContactList, hdc);
				return;
			}
		}
		GetPrivateProfileStringA(itemname, "Fillcolor", "None", buffer, 500, szFileName);
		if (mir_strcmp(buffer, "None")) {
			COLORREF fillColor = HexStringToLong(buffer);
			tmpItem.fillBrush = CreateSolidBrush(fillColor);
			tmpItem.dwFlags |= IMAGE_FILLSOLID;
		}
		else
			tmpItem.fillBrush = nullptr;

		GetPrivateProfileStringA(itemname, "Stretch", "None", buffer, 500, szFileName);
		if (buffer[0] == 'B' || buffer[0] == 'b')
			tmpItem.bStretch = IMAGE_STRETCH_B;
		else if (buffer[0] == 'h' || buffer[0] == 'H')
			tmpItem.bStretch = IMAGE_STRETCH_V;
		else if (buffer[0] == 'w' || buffer[0] == 'W')
			tmpItem.bStretch = IMAGE_STRETCH_H;
		tmpItem.hbm = nullptr;

		if (!_stricmp(itemname, "$glyphs")) {
			IMG_CreateItem(&tmpItem, szFinalName, hdc);
			if (tmpItem.hbm) {
				newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
				memset(newItem, 0, sizeof(ImageItem));
				*newItem = tmpItem;
				g_glyphItem = newItem;
			}
			goto imgread_done;
		}
		if (itemname[0] == '@') {
			if (!(tmpItem.dwFlags & IMAGE_GLYPH))
				IMG_CreateItem(&tmpItem, szFinalName, hdc);
			if (tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
				newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
				memset(newItem, 0, sizeof(ImageItem));
				*newItem = tmpItem;

				if (g_ImageItems == nullptr)
					g_ImageItems = newItem;
				else {
					ImageItem *pItem = g_ImageItems;

					while (pItem->nextItem != nullptr)
						pItem = pItem->nextItem;
					pItem->nextItem = newItem;
				}
			}
			goto imgread_done;
		}
		for (n = 0;; n++) {
			mir_snprintf(szItemNr, "Item%d", n);
			GetPrivateProfileStringA(itemname, szItemNr, "None", buffer, 500, szFileName);
			if (!mir_strcmp(buffer, "None"))
				break;
			if (!mir_strcmp(buffer, "CLUI")) {
				if (!(tmpItem.dwFlags & IMAGE_GLYPH))
					IMG_CreateItem(&tmpItem, szFinalName, hdc);
				if (tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
					newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
					memset(newItem, 0, sizeof(ImageItem));
					*newItem = tmpItem;

					if (g_CLUIImageItem) {
						IMG_DeleteItem(g_CLUIImageItem);
						free(g_CLUIImageItem);
					}
					g_CLUIImageItem = newItem;

					GetPrivateProfileStringA(itemname, "Colorkey", "e5e5e5", buffer, 500, szFileName);
					COLORREF clr = HexStringToLong(buffer);
					cfg::dat.colorkey = clr;
					db_set_dw(0, "CLUI", "ColorKey", clr);
					if (g_CLUISkinnedBkColor)
						DeleteObject(g_CLUISkinnedBkColor);
					g_CLUISkinnedBkColor = CreateSolidBrush(clr);
					g_CLUISkinnedBkColorRGB = clr;
				}
				continue;
			}
			
			for (auto &p : arStatusItems) {
				if (!_stricmp(p->szName[0] == '{' ? p->szName + 3 : p->szName, buffer)) {
					if (!alloced) {
						if (!(tmpItem.dwFlags & IMAGE_GLYPH))
							IMG_CreateItem(&tmpItem, szFinalName, hdc);
						if (tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
							newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
							memset(newItem, 0, sizeof(ImageItem));
							*newItem = tmpItem;
							p->imageItem = newItem;
							if (g_ImageItems == nullptr)
								g_ImageItems = newItem;
							else {
								ImageItem *pItem = g_ImageItems;

								while (pItem->nextItem != nullptr)
									pItem = pItem->nextItem;
								pItem->nextItem = newItem;
							}
							alloced = TRUE;
						}
					}
					else if (newItem != nullptr)
						p->imageItem = newItem;
				}
			}
		}
	}
imgread_done:
	ReleaseDC(g_clistApi.hwndContactList, hdc);
}

void IMG_DeleteItems()
{
	ImageItem *pItem = g_ImageItems, *pNextItem;
	ButtonItem *pbItem = g_ButtonItems, *pbNextItem;

	while (pItem) {
		IMG_DeleteItem(pItem);
		pNextItem = pItem->nextItem;
		free(pItem);
		pItem = pNextItem;
	}
	g_ImageItems = nullptr;
	while (pbItem) {
		DestroyWindow(pbItem->hWnd);
		pbNextItem = pbItem->nextItem;
		free(pbItem);
		pbItem = pbNextItem;
	}
	g_ButtonItems = nullptr;

	if (g_CLUIImageItem) {
		IMG_DeleteItem(g_CLUIImageItem);
		free(g_CLUIImageItem);
	}
	g_CLUIImageItem = nullptr;

	if (g_glyphItem) {
		IMG_DeleteItem(g_glyphItem);
		free(g_glyphItem);
	}
	g_glyphItem = nullptr;

	for (auto &it : arStatusItems)
		it->imageItem = nullptr;
}

static UINT nextButtonID = IDC_TBFIRSTUID;

static void BTN_ReadItem(char *itemName, char *file)
{
	ButtonItem tmpItem, *newItem;
	char szBuffer[1024];
	ImageItem *imgItem = g_ImageItems;

	memset(&tmpItem, 0, sizeof(tmpItem));
	strncpy_s(tmpItem.szName, &itemName[1], _TRUNCATE);
	tmpItem.width = GetPrivateProfileIntA(itemName, "Width", 16, file);
	tmpItem.height = GetPrivateProfileIntA(itemName, "Height", 16, file);
	tmpItem.xOff = GetPrivateProfileIntA(itemName, "xoff", 0, file);
	tmpItem.yOff = GetPrivateProfileIntA(itemName, "yoff", 0, file);

	tmpItem.dwFlags |= GetPrivateProfileIntA(itemName, "toggle", 0, file) ? BUTTON_ISTOGGLE : 0;

	GetPrivateProfileStringA(itemName, "Pressed", "None", szBuffer, 1000, file);
	if (!_stricmp(szBuffer, "default"))
		tmpItem.imgPressed = arStatusItems[ID_EXTBKTBBUTTONSPRESSED - ID_STATUS_OFFLINE]->imageItem;
	else {
		while (imgItem) {
			if (!_stricmp(imgItem->szName, szBuffer)) {
				tmpItem.imgPressed = imgItem;
				break;
			}
			imgItem = imgItem->nextItem;
		}
	}

	imgItem = g_ImageItems;
	GetPrivateProfileStringA(itemName, "Normal", "None", szBuffer, 1000, file);
	if (!_stricmp(szBuffer, "default"))
		tmpItem.imgNormal = arStatusItems[ID_EXTBKTBBUTTONSNPRESSED - ID_STATUS_OFFLINE]->imageItem;
	else {
		while (imgItem) {
			if (!_stricmp(imgItem->szName, szBuffer)) {
				tmpItem.imgNormal = imgItem;
				break;
			}
			imgItem = imgItem->nextItem;
		}
	}

	imgItem = g_ImageItems;
	GetPrivateProfileStringA(itemName, "Hover", "None", szBuffer, 1000, file);
	if (!_stricmp(szBuffer, "default"))
		tmpItem.imgHover = arStatusItems[ID_EXTBKTBBUTTONMOUSEOVER - ID_STATUS_OFFLINE]->imageItem;
	else {
		while (imgItem) {
			if (!_stricmp(imgItem->szName, szBuffer)) {
				tmpItem.imgHover = imgItem;
				break;
			}
			imgItem = imgItem->nextItem;
		}
	}

	GetPrivateProfileStringA(itemName, "NormalGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
	sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.normalGlyphMetrics[0], &tmpItem.normalGlyphMetrics[1],
		&tmpItem.normalGlyphMetrics[2], &tmpItem.normalGlyphMetrics[3]);
	tmpItem.normalGlyphMetrics[2] = (tmpItem.normalGlyphMetrics[2] - tmpItem.normalGlyphMetrics[0]) + 1;
	tmpItem.normalGlyphMetrics[3] = (tmpItem.normalGlyphMetrics[3] - tmpItem.normalGlyphMetrics[1]) + 1;

	GetPrivateProfileStringA(itemName, "PressedGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
	sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.pressedGlyphMetrics[0], &tmpItem.pressedGlyphMetrics[1],
		&tmpItem.pressedGlyphMetrics[2], &tmpItem.pressedGlyphMetrics[3]);
	tmpItem.pressedGlyphMetrics[2] = (tmpItem.pressedGlyphMetrics[2] - tmpItem.pressedGlyphMetrics[0]) + 1;
	tmpItem.pressedGlyphMetrics[3] = (tmpItem.pressedGlyphMetrics[3] - tmpItem.pressedGlyphMetrics[1]) + 1;


	GetPrivateProfileStringA(itemName, "HoverGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
	sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.hoverGlyphMetrics[0], &tmpItem.hoverGlyphMetrics[1],
		&tmpItem.hoverGlyphMetrics[2], &tmpItem.hoverGlyphMetrics[3]);
	tmpItem.hoverGlyphMetrics[2] = (tmpItem.hoverGlyphMetrics[2] - tmpItem.hoverGlyphMetrics[0]) + 1;
	tmpItem.hoverGlyphMetrics[3] = (tmpItem.hoverGlyphMetrics[3] - tmpItem.hoverGlyphMetrics[1]) + 1;

	tmpItem.uId = IDC_TBFIRSTUID - 1;

	GetPrivateProfileStringA(itemName, "Action", "Custom", szBuffer, 1000, file);
	if (!_stricmp(szBuffer, "service")) {
		tmpItem.szService[0] = 0;
		GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
		if (_stricmp(szBuffer, "None")) {
			strncpy_s(tmpItem.szService, szBuffer, _TRUNCATE);
			tmpItem.dwFlags |= BUTTON_ISSERVICE;
			tmpItem.uId = nextButtonID++;
		}
	}
	else if (!_stricmp(szBuffer, "protoservice")) {
		tmpItem.szService[0] = 0;
		GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
		if (_stricmp(szBuffer, "None")) {
			strncpy_s(tmpItem.szService, szBuffer, _TRUNCATE);
			tmpItem.dwFlags |= BUTTON_ISPROTOSERVICE;
			tmpItem.uId = nextButtonID++;
		}
	}
	else if (!_stricmp(szBuffer, "database")) {
		int n;

		GetPrivateProfileStringA(itemName, "Module", "None", szBuffer, 1000, file);
		if (_stricmp(szBuffer, "None"))
			strncpy_s(tmpItem.szModule, szBuffer, _TRUNCATE);
		GetPrivateProfileStringA(itemName, "Setting", "None", szBuffer, 1000, file);
		if (_stricmp(szBuffer, "None"))
			strncpy_s(tmpItem.szSetting, szBuffer, _TRUNCATE);
		if (GetPrivateProfileIntA(itemName, "contact", 0, file) != 0)
			tmpItem.dwFlags |= BUTTON_DBACTIONONCONTACT;

		for (n = 0; n <= 1; n++) {
			CMStringA szKey;
			uint8_t *pValue;

			if (n == 0)
				szKey = "dbonpush";
			else
				szKey = "dbonrelease";
			pValue = (n == 0 ? tmpItem.bValuePush : tmpItem.bValueRelease);

			GetPrivateProfileStringA(itemName, szKey, "None", szBuffer, 1000, file);
			switch (szBuffer[0]) {
			case 'b':
				{
					uint8_t value = (uint8_t)atol(&szBuffer[1]);
					pValue[0] = value;
					tmpItem.type = DBVT_BYTE;
					break;
				}
			case 'w':
				{
					uint16_t value = (uint16_t)atol(&szBuffer[1]);
					*((uint16_t *)&pValue[0]) = value;
					tmpItem.type = DBVT_WORD;
					break;
				}
			case 'd':
				{
					uint32_t value = (uint32_t)atol(&szBuffer[1]);
					*((uint32_t *)&pValue[0]) = value;
					tmpItem.type = DBVT_DWORD;
					break;
				}
			case 's':
				mir_snprintf((char *)pValue, 256, &szBuffer[1]);
				tmpItem.type = DBVT_ASCIIZ;
				break;
			}
		}
		if (tmpItem.szModule[0] && tmpItem.szSetting[0]) {
			tmpItem.dwFlags |= BUTTON_ISDBACTION;
			if (tmpItem.szModule[0] == '$' && (tmpItem.szModule[1] == 'c' || tmpItem.szModule[1] == 'C'))
				tmpItem.dwFlags |= BUTTON_ISCONTACTDBACTION;
			tmpItem.uId = nextButtonID++;
		}
	}
	else if (_stricmp(szBuffer, "Custom")) {
		int i = 0;

		while (BTNS[i].ctrlid) {
			if (!_stricmp(BTNS[i].pszButtonID, szBuffer)) {
				tmpItem.uId = BTNS[i].ctrlid - 20;
				tmpItem.dwFlags |= BUTTON_ISINTERNAL;
				break;
			}
			i++;
		}
	}

	GetPrivateProfileStringA(itemName, "PassContact", "None", szBuffer, 1000, file);
	if (_stricmp(szBuffer, "None")) {
		if (szBuffer[0] == 'w' || szBuffer[0] == 'W')
			tmpItem.dwFlags |= BUTTON_PASSHCONTACTW;
		else if (szBuffer[0] == 'l' || szBuffer[0] == 'L')
			tmpItem.dwFlags |= BUTTON_PASSHCONTACTL;
	}

	GetPrivateProfileStringA(itemName, "Tip", "None", szBuffer, 1000, file);
	if (mir_strcmp(szBuffer, "None")) {
		MultiByteToWideChar(cfg::dat.langPackCP, 0, szBuffer, -1, tmpItem.szTip, 256);
		tmpItem.szTip[255] = 0;
	}
	else
		tmpItem.szTip[0] = 0;

	// create it

	newItem = (ButtonItem *)malloc(sizeof(ButtonItem));
	memset(newItem, 0, sizeof(ButtonItem));
	if (g_ButtonItems == nullptr) {
		g_ButtonItems = newItem;
		*newItem = tmpItem;
		newItem->nextItem = nullptr;
	}
	else {
		ButtonItem *curItem = g_ButtonItems;
		while (curItem->nextItem)
			curItem = curItem->nextItem;
		*newItem = tmpItem;
		newItem->nextItem = nullptr;
		curItem->nextItem = newItem;
	}
	newItem->hWnd = CreateWindowEx(0, MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 5, 5, g_clistApi.hwndContactList, (HMENU)newItem->uId, g_plugin.getInst(), nullptr);
	CustomizeButton(newItem->hWnd, false, false, true);
	SendMessage(newItem->hWnd, BUTTONSETBTNITEM, 0, (LPARAM)newItem);
	if (newItem->dwFlags & BUTTON_ISTOGGLE)
		SendMessage(newItem->hWnd, BUTTONSETASPUSHBTN, TRUE, 0);

	if (newItem->szTip[0])
		SendMessage(newItem->hWnd, BUTTONADDTOOLTIP, (WPARAM)newItem->szTip, BATF_UNICODE);
	return;
}

void IMG_LoadItems()
{
	DBVARIANT dbv;
	if (db_get_ws(0, "CLC", "AdvancedSkin", &dbv))
		return;

	wchar_t tszFileName[MAX_PATH];
	MY_pathToAbsolute(dbv.pwszVal, tszFileName);

	// TODO: rewrite the skin loading in wchar_t manner
	char szFileName[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, tszFileName, MAX_PATH, szFileName, MAX_PATH, nullptr, nullptr);

	db_free(&dbv);

	if (!PathFileExists(tszFileName))
		return;

	IMG_DeleteItems();

	char *szSections = reinterpret_cast<char *>(malloc(3002));
	memset(szSections, 0, 3002);
	GetPrivateProfileSectionNamesA(szSections, 3000, szFileName);

	szSections[3001] = szSections[3000] = 0;
	char *p = szSections;
	while (mir_strlen(p) > 1) {
		if (p[0] == '$' || p[0] == '@')
			IMG_ReadItem(p, szFileName);
		p += (mir_strlen(p) + 1);
	}
	nextButtonID = IDC_TBFIRSTUID;
	p = szSections;
	while (mir_strlen(p) > 1) {
		if (p[0] == '!')
			BTN_ReadItem(p, szFileName);
		p += (mir_strlen(p) + 1);
	}
	if (g_clistApi.hwndContactList)
		SetButtonStates();
	free(szSections);

	if (g_ImageItems) {
		db_set_b(0, "CLCExt", "bskinned", 1);
		SetButtonToSkinned();
	}
	if (g_CLUIImageItem) {
		cfg::dat.bFullTransparent = TRUE;
		cfg::dat.dwFlags &= ~CLUI_FRAME_CLISTSUNKEN;
		db_set_b(0, "CLUI", "fulltransparent", (uint8_t)cfg::dat.bFullTransparent);
		db_set_b(0, "CLUI", "WindowStyle", SETTING_WINDOWSTYLE_NOBORDER);
		ApplyCLUIBorderStyle();
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(g_clistApi.hwndContactList, cfg::dat.colorkey, 0, LWA_COLORKEY);
	}
	CoolSB_SetupScrollBar();
}

void LoadPerContactSkins(wchar_t *tszFileName)
{
	char szItem[100];
	ptrA szSections(LPSTR(calloc(3002, 1)));
	char *p = szSections;

	StatusItems_t *items = nullptr, *this_item;
	int i = 1;

	char file[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, tszFileName, MAX_PATH, file, MAX_PATH, nullptr, nullptr);
	file[MAX_PATH - 1] = 0;

	ReadItem(&default_item, "%Default", file);
	GetPrivateProfileSectionNamesA(szSections, 3000, file);
	szSections[3001] = szSections[3000] = 0;

	while (mir_strlen(p) > 1) {
		if (p[0] == '%') {
			p += (mir_strlen(p) + 1);
			continue;
		}
		items = reinterpret_cast<StatusItems_t *>(realloc(items, i * sizeof(StatusItems_t)));
		memset(&items[i - 1], 0, sizeof(StatusItems_t));
		this_item = &items[i - 1];
		GetPrivateProfileStringA(p, "Proto", "", this_item->szName, 40, file);
		this_item->szName[39] = 0;
		GetPrivateProfileStringA(p, "UIN", "", this_item->szDBname, 40, file);
		this_item->szDBname[39] = 0;
		this_item->IGNORED = 0;
		GetPrivateProfileStringA(p, "Item", "", szItem, 100, file);
		szItem[99] = 0;
		//_DebugPopup(0, "Section: %s -> %s", p, szItem);
		ReadItem(this_item, szItem, file);
		p += (mir_strlen(p) + 1);
		i++;
	}

	if (items) {
		for (auto &hContact : Contacts()) {
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto == nullptr)
				continue;

			const char *uid = Proto_GetUniqueId(szProto);
			if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid != nullptr) {
				DBVARIANT dbv = { 0 };
				if (db_get(hContact, szProto, uid, &dbv))
					break;

				char UIN[40];
				switch (dbv.type) {
				case DBVT_DWORD:
					mir_snprintf(UIN, "%d", dbv.dVal);
					break;
				case DBVT_ASCIIZ:
					strncpy_s(UIN, dbv.pszVal, _TRUNCATE);
					db_free(&dbv);
					break;
				default:
					UIN[0] = 0;
					break;
				}

				int j;
				for (j = 0; j < i - 1; j++) {
					if (!mir_strcmp(szProto, items[j].szName) && !mir_strcmp(UIN, items[j].szDBname) &&
						mir_strlen(szProto) == mir_strlen(items[j].szName) && mir_strlen(UIN) == mir_strlen(items[j].szDBname)) {
						db_set_dw(hContact, "EXTBK", "TEXT", items[j].TEXTCOLOR);
						db_set_dw(hContact, "EXTBK", "COLOR1", items[j].COLOR);
						db_set_dw(hContact, "EXTBK", "COLOR2", items[j].COLOR2);
						db_set_b(hContact, "EXTBK", "ALPHA", (uint8_t)items[j].ALPHA);

						db_set_b(hContact, "EXTBK", "LEFT", (uint8_t)items[j].MARGIN_LEFT);
						db_set_b(hContact, "EXTBK", "RIGHT", (uint8_t)items[j].MARGIN_RIGHT);
						db_set_b(hContact, "EXTBK", "TOP", (uint8_t)items[j].MARGIN_TOP);
						db_set_b(hContact, "EXTBK", "BOTTOM", (uint8_t)items[j].MARGIN_BOTTOM);

						db_set_b(hContact, "EXTBK", "TRANS", items[j].COLOR2_TRANSPARENT);
						db_set_dw(hContact, "EXTBK", "BDR", items[j].BORDERSTYLE);

						db_set_b(hContact, "EXTBK", "CORNER", items[j].CORNER);
						db_set_b(hContact, "EXTBK", "GRAD", items[j].GRADIENT);
						db_set_b(hContact, "EXTBK", "TRANS", items[j].COLOR2_TRANSPARENT);

						db_set_b(hContact, "EXTBK", "VALID", 1);
						break;
					}
				}
				if (j == i - 1) // disable the db copy if it has been disabled in the skin .ini file
					if (db_get_b(hContact, "EXTBK", "VALID", 0))
						db_set_b(hContact, "EXTBK", "VALID", 0);
			}
		}
		free(items);
	}
}

void extbk_import(char *file, HWND hwndDlg)
{
	int i;
	char buffer[255];
	char szKey[255], szSection[255];
	uint32_t data, version = 0;

	for (auto &p : arStatusItems) {
		if (p->statusID == ID_EXTBKSEPARATOR)
			continue;

		mir_snprintf(buffer, "%s_ALPHA", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->ALPHA), sizeof(p->ALPHA), file);
		mir_snprintf(buffer, "%s_COLOR", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR), sizeof(p->COLOR), file);
		mir_snprintf(buffer, "%s_COLOR2", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR2), sizeof(p->COLOR2), file);
		mir_snprintf(buffer, "%s_COLOR2_TRANSPARENT", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->COLOR2_TRANSPARENT), sizeof(p->COLOR2_TRANSPARENT), file);
		mir_snprintf(buffer, "%s_TEXTCOLOR", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->TEXTCOLOR), sizeof(p->TEXTCOLOR), file);
		mir_snprintf(buffer, "%s_CORNER", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->CORNER), sizeof(p->CORNER), file);
		mir_snprintf(buffer, "%s_GRADIENT", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->GRADIENT), sizeof(p->GRADIENT), file);
		mir_snprintf(buffer, "%s_IGNORED", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->IGNORED), sizeof(p->IGNORED), file);
		mir_snprintf(buffer, "%s_MARGIN_BOTTOM", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_BOTTOM), sizeof(p->MARGIN_BOTTOM), file);
		mir_snprintf(buffer, "%s_MARGIN_LEFT", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_LEFT), sizeof(p->MARGIN_LEFT), file);
		mir_snprintf(buffer, "%s_MARGIN_RIGHT", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_RIGHT), sizeof(p->MARGIN_RIGHT), file);
		mir_snprintf(buffer, "%s_MARGIN_TOP", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->MARGIN_TOP), sizeof(p->MARGIN_TOP), file);
		mir_snprintf(buffer, "%s_BORDERSTYLE", p->szDBname);
		GetPrivateProfileStructA("ExtBKSettings", buffer, &(p->BORDERSTYLE), sizeof(p->BORDERSTYLE), file);
	}

	data = 0;
	GetPrivateProfileStructA("Global", "Version", &version, 4, file);
	if (version >= 2) {
		for (int n = 0; n <= FONTID_LAST; n++) {
			mir_snprintf(szSection, "Font%d", n);

			mir_snprintf(szKey, "Font%dName", n);
			GetPrivateProfileStringA(szSection, "Name", "Arial", buffer, sizeof(buffer), file);
			db_set_s(0, "CLC", szKey, buffer);

			mir_snprintf(szKey, "Font%dSize", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "Size", &data, 1, file);
			db_set_b(0, "CLC", szKey, (uint8_t)data);

			mir_snprintf(szKey, "Font%dSty", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "Style", &data, 1, file);
			db_set_b(0, "CLC", szKey, (uint8_t)data);

			mir_snprintf(szKey, "Font%dSet", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "Set", &data, 1, file);
			db_set_b(0, "CLC", szKey, (uint8_t)data);

			mir_snprintf(szKey, "Font%dCol", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "Color", &data, 4, file);
			db_set_dw(0, "CLC", szKey, data);

			mir_snprintf(szKey, "Font%dFlags", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "Flags", &data, 4, file);
			db_set_dw(0, "CLC", szKey, (uint16_t)data);

			mir_snprintf(szKey, "Font%dAs", n);
			data = 0;
			GetPrivateProfileStructA(szSection, "SameAs", &data, 2, file);
			db_set_dw(0, "CLC", szKey, (uint16_t)data);
		}
	}
	i = 0;
	if (version >= 3) {
		char szString[MAX_PATH];
		szString[0] = 0;

		while (_tagSettings[i].szModule != nullptr) {
			data = 0;
			GetPrivateProfileStructA("Global", _tagSettings[i].szSetting, &data, _tagSettings[i].size, file);
			switch (_tagSettings[i].size) {
			case 1:
				db_set_b(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, (uint8_t)data);
				break;
			case 4:
				db_set_dw(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, data);
				break;
			case 2:
				db_set_w(0, _tagSettings[i].szModule, _tagSettings[i].szSetting, (uint16_t)data);
				break;
			}
			i++;
		}
		GetPrivateProfileStringA("Global", "BkBitmap", "", szString, MAX_PATH, file);
		if (szString[0] != 0)
			db_set_s(0, "CLC", "BkBitmap", szString);
	}

	Reload3dBevelColors();
	ReloadThemedOptions();
	SetButtonToSkinned();
	// refresh
	if (hwndDlg && ServiceExists(MS_CLNSE_FILLBYCURRENTSEL))
		CallService(MS_CLNSE_FILLBYCURRENTSEL, (WPARAM)hwndDlg, 0);
	Clist_ClcOptionsChanged();
	ConfigureCLUIGeometry(1);
	SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
	RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

static void ApplyCLUISkin()
{
	DBVARIANT dbv = { 0 };
	wchar_t tszFinalName[MAX_PATH];
	char szFinalName[MAX_PATH];
	if (!db_get_ws(0, "CLC", "AdvancedSkin", &dbv)) {
		MY_pathToAbsolute(dbv.pwszVal, tszFinalName);

		WideCharToMultiByte(CP_ACP, 0, tszFinalName, MAX_PATH, szFinalName, MAX_PATH, nullptr, nullptr);

		if (db_get_b(0, "CLUI", "skin_changed", 0)) {
			extbk_import(szFinalName, nullptr);
			SaveCompleteStructToDB();
			db_set_b(0, "CLUI", "skin_changed", 0);
		}
		IMG_LoadItems();
		ShowWindow(g_clistApi.hwndContactList, SW_SHOWNORMAL);
		SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
		db_free(&dbv);
	}
}

static INT_PTR CALLBACK DlgProcSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG: {
			DBVARIANT dbv;
			TranslateDialogDefault(hwndDlg);

			CheckDlgButton(hwndDlg, IDC_EQUALSELECTION, (db_get_b(0, "CLCExt", "EXBK_EqualSelection", 1) == 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SELBLEND, db_get_b(0, "CLCExt", "EXBK_SelBlend", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SETALLBUTTONSKINNED, db_get_b(0, "CLCExt", "bskinned", 0) ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_CORNERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_CORNERSPIN, UDM_SETPOS, 0, cfg::dat.cornerRadius);

			SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(20, 0));
			SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.group_padding);

			SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(40, 0));
			SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.titleBarHeight);

			CheckDlgButton(hwndDlg, IDC_APPLYINDENTBG, cfg::dat.bApplyIndentToBg ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USEPERPROTO, cfg::dat.bUsePerProto ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR, cfg::dat.bOverridePerStatusColors ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FASTGRADIENT, cfg::dat.bWantFastGradients ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_IGNORESELFORGROUPS, db_get_b(0, "CLC", "IgnoreSelforGroups", 0) ? BST_CHECKED : BST_UNCHECKED);

			if (!db_get_s(0, "CLC", "ContactSkins", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_SKINFILE, dbv.pszVal);
				db_free(&dbv);
				Utils::enableDlgControl(hwndDlg, IDC_RELOAD, TRUE);
			}
			else
				Utils::enableDlgControl(hwndDlg, IDC_RELOAD, FALSE);
			CheckDlgButton(hwndDlg, IDC_USESKIN, db_get_b(0, "CLUI", "useskin", 0) ? BST_CHECKED : BST_UNCHECKED);
			if (!db_get_ws(0, "CLC", "AdvancedSkin", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_SKINFILENAME, dbv.pwszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_SKINFILENAME, L"");
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USESKIN:
			{
				int useskin = IsDlgButtonChecked(hwndDlg, IDC_USESKIN);
				db_set_b(0, "CLUI", "useskin", (uint8_t)(useskin ? 1 : 0));
			}
			break;

		case IDC_UNLOAD:
			IMG_DeleteItems();
			ConfigureFrame();
			SetButtonStates();
			SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
			PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
			break;

		case IDC_GETSKINS:
			if (HIWORD(wParam) == BN_CLICKED)
				Utils_OpenUrl("https://miranda-ng.org/tags/nicer-contact-list/");
			break;

		case IDC_SELECTSKINFILE:
			{
				OPENFILENAME ofn = { 0 };
				wchar_t str[MAX_PATH] = L"*.clist", final_path[MAX_PATH];

				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = nullptr;
				ofn.lpstrFilter = L"*.clist\0";
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = L"";
				if (!GetOpenFileName(&ofn))
					break;
				MY_pathToRelative(str, final_path);
				if (PathFileExists(str)) {
					int skinChanged = 0;
					DBVARIANT dbv = { 0 };

					if (!db_get_ws(0, "CLC", "AdvancedSkin", &dbv)) {
						if (mir_wstrcmp(dbv.pwszVal, final_path))
							skinChanged = TRUE;
						db_free(&dbv);
					}
					else
						skinChanged = TRUE;
					db_set_ws(0, "CLC", "AdvancedSkin", final_path);
					db_set_b(0, "CLUI", "skin_changed", (uint8_t)skinChanged);
					SetDlgItemText(hwndDlg, IDC_SKINFILENAME, final_path);
				}
			}
			break;

		case IDC_RELOADSKIN:
			db_set_b(0, "CLUI", "skin_changed", 1);
			ApplyCLUISkin();
			break;

		case IDC_RELOAD:
			wchar_t tszFilename[MAX_PATH], tszFinalPath[MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_SKINFILE, tszFilename, _countof(tszFilename));
			tszFilename[MAX_PATH - 1] = 0;
			MY_pathToAbsolute(tszFilename, tszFinalPath);
			if (PathFileExists(tszFinalPath)) {
				LoadPerContactSkins(tszFinalPath);
				ReloadSkinItemsToCache();
				Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
			}
			break;
		}
		if ((LOWORD(wParam) == IDC_SKINFILE || LOWORD(wParam) == IDC_SKINFILENAME)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SaveNonStatusItemsSettings(hwndDlg);
				Clist_ClcOptionsChanged();
				PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iInit = TRUE;
	static HWND hwndSkinEdit = nullptr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			int oPage = db_get_b(0, "CLUI", "opage", 0);

			RECT rcClient;
			GetClientRect(hwnd, &rcClient);
			iInit = TRUE;

			TCITEM tci;
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OPT_SKIN), hwnd, DlgProcSkinOpts);
			tci.pszText = TranslateT("Load and apply");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
			MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 60, 1);
			ShowWindow((HWND)tci.lParam, oPage == 0 ? SW_SHOW : SW_HIDE);
			if (IS_THEMED)
				EnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			if (ServiceExists(MS_CLNSE_INVOKE)) {
				SKINDESCRIPTION sd = { 0 };
				sd.cbSize = sizeof(sd);
				sd.StatusItems = arStatusItems.getArray();
				sd.hWndParent = hwnd;
				sd.hWndTab = GetDlgItem(hwnd, IDC_OPTIONSTAB);
				sd.pfnSaveCompleteStruct = SaveCompleteStructToDB;
				sd.lastItem = ID_STATUS_OFFLINE + arStatusItems.getCount();
				sd.firstItem = ID_STATUS_OFFLINE;
				sd.pfnClcOptionsChanged = &Clist_ClcOptionsChanged;
				sd.hwndCLUI = g_clistApi.hwndContactList;
				hwndSkinEdit = (HWND)CallService(MS_CLNSE_INVOKE, 0, (LPARAM)&sd);
			}

			if (hwndSkinEdit) {
				ShowWindow(hwndSkinEdit, oPage == 1 ? SW_SHOW : SW_HIDE);
				TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
				if (IS_THEMED)
					EnableThemeDialogTexture(hwndSkinEdit, ETDT_ENABLETAB);
			}

			TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
			Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
			Utils::enableDlgControl(hwnd, IDC_IMPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
			iInit = FALSE;
			return FALSE;
		}

	case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
		if (!iInit)
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXPORT:
			{
				char str[MAX_PATH] = "*.clist";
				OPENFILENAMEA ofn = { 0 };
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwnd;
				ofn.hInstance = nullptr;
				ofn.lpstrFilter = "*.clist";
				ofn.lpstrFile = str;
				ofn.Flags = OFN_HIDEREADONLY;
				ofn.nMaxFile = _countof(str);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = "clist";
				if (!GetSaveFileNameA(&ofn))
					break;
				extbk_export(str);
				break;
			}
		case IDC_IMPORT:
			{
				char str[MAX_PATH] = "*.clist";
				OPENFILENAMEA ofn = { 0 };
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwnd;
				ofn.hInstance = nullptr;
				ofn.lpstrFilter = "*.clist";
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST;
				ofn.nMaxFile = _countof(str);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = "";
				if (!GetOpenFileNameA(&ofn))
					break;
				extbk_import(str, hwndSkinEdit);
				SendMessage(hwndSkinEdit, WM_USER + 101, 0, 0);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
			TCITEM tci;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				tci.mask = TCIF_PARAM;
				int count = TabCtrl_GetItemCount(GetDlgItem(hwnd, IDC_OPTIONSTAB));
				for (int i = 0; i < count; i++) {
					TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), i, &tci);
					SendMessage((HWND)tci.lParam, WM_NOTIFY, 0, lParam);
				}
				break;
			}
			break;

		case IDC_OPTIONSTAB:
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGING:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
				break;

			case TCN_SELCHANGE:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
				db_set_b(0, "CLUI", "opage", (uint8_t)TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)));
				Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
				Utils::enableDlgControl(hwnd, IDC_IMPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
				break;
			}
			break;

		}
		break;
	case WM_DESTROY:
		hwndSkinEdit = nullptr;
		break;
	}
	return FALSE;
}

void InitSkinOptions(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Skins");
	odp.szTitle.a = LPGEN("Contact list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.pfnDlgProc = OptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);
}
