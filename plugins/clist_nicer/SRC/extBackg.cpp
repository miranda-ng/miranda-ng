/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

#include <commonheaders.h>
#include "../coolsb/coolscroll.h"

extern int g_hottrack;

extern HWND g_hwndViewModeFrame;
extern HIMAGELIST himlExtraImages;
extern struct CluiTopButton top_buttons[];

StatusItems_t *StatusItems = NULL;
ImageItem *g_ImageItems = NULL, *g_glyphItem = NULL;
ButtonItem *g_ButtonItems = NULL;
ImageItem *g_CLUIImageItem = NULL;
HBRUSH g_CLUISkinnedBkColor = 0;
COLORREF g_CLUISkinnedBkColorRGB = 0;

int ID_EXTBK_LAST = ID_EXTBK_LAST_D;

void SetTBSKinned(int mode);
void ReloadThemedOptions();
void ReloadExtraIcons();
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
    }, {"NA", "EXBK_NA", ID_STATUS_NA,
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
    }, {"On the phone", "EXBK_OTP", ID_STATUS_ONTHEPHONE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Out to lunch", "EXBK_OTL", ID_STATUS_OUTTOLUNCH,
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
    status = (status >= ID_STATUS_OFFLINE && status <= ID_EXTBK_LAST) ? status : ID_STATUS_OFFLINE;     // better check the index...
    *retitem = StatusItems[status - ID_STATUS_OFFLINE];
     if(g_hottrack && status != ID_EXTBKHOTTRACK)        // allow hottracking for ignored items, unless hottrack item itself should be ignored
         retitem->IGNORED = FALSE;
      return TRUE;
}

StatusItems_t *GetProtocolStatusItem(const char *szProto)
{
    int i;

    if(szProto == NULL)
        return NULL;

    for(i = ID_EXTBK_LAST_D - ID_STATUS_OFFLINE + 1; i <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; i++) {
        if(!strcmp(StatusItems[i].szName[0] == '{' ? &StatusItems[i].szName[3] : StatusItems[i].szName, szProto))
            return &StatusItems[i];
    }
    return NULL;
}

// fills the struct with the settings in the database
void LoadExtBkSettingsFromDB()
{
	DWORD ret;
	int n;
	char buffer[255];
	int protoCount = 0, i;
	PROTOACCOUNT **accs = 0;
	DBVARIANT dbv = {0};

	ProtoEnumAccounts( &protoCount, &accs );

	StatusItems = (StatusItems_t *)malloc(sizeof(StatusItems_t) * ((ID_EXTBK_LAST - ID_STATUS_OFFLINE) + protoCount + 2));
	CopyMemory(StatusItems, _StatusItems, sizeof(_StatusItems));

	for(i = 0; i < protoCount; i++) {
		ID_EXTBK_LAST++;
		CopyMemory(&StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE], &StatusItems[0], sizeof(StatusItems_t));
		mir_snprintf(StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE].szDBname, 30, "EXBK_%s", accs[i]->szModuleName );
		if(i == 0) {
			lstrcpynA(StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE].szName, "{-}", 30);
			strncat(StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE].szName, accs[i]->szModuleName, 30);
		}
		else
			lstrcpynA(StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE].szName, accs[i]->szModuleName, 30);
		StatusItems[ID_EXTBK_LAST - ID_STATUS_OFFLINE].statusID = ID_EXTBK_LAST;
	}
	for (n = 0; n <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; n++) {
		if (StatusItems[n].statusID != ID_EXTBKSEPARATOR) {
			StatusItems[n].imageItem = 0;
			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_IGNORE");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].IGNORED);
			StatusItems[n]. IGNORED = (BYTE) ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_GRADIENT");
			ret = cfg::getDword("CLCExt", buffer, StatusItems[n].GRADIENT);
			StatusItems[n]. GRADIENT = (BYTE) ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_CORNER");
			ret = cfg::getDword("CLCExt", buffer, StatusItems[n].CORNER);
			StatusItems[n]. CORNER = (BYTE) ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR");
			ret = cfg::getDword("CLCExt", buffer, StatusItems[n].COLOR);
			StatusItems[n]. COLOR = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2");
			ret = cfg::getDword(NULL, "CLCExt", buffer, StatusItems[n].COLOR2);
			StatusItems[n]. COLOR2 = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2_TRANSPARENT");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].COLOR2_TRANSPARENT);
			StatusItems[n]. COLOR2_TRANSPARENT = (BYTE) ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_TEXTCOLOR");
			ret = cfg::getDword("CLCExt", buffer, StatusItems[n].TEXTCOLOR);
			StatusItems[n]. TEXTCOLOR = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_ALPHA");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].ALPHA);
			StatusItems[n]. ALPHA = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_LEFT");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].MARGIN_LEFT);
			StatusItems[n]. MARGIN_LEFT = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_TOP");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].MARGIN_TOP);
			StatusItems[n]. MARGIN_TOP = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_RIGHT");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].MARGIN_RIGHT);
			StatusItems[n]. MARGIN_RIGHT = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_BOTTOM");
			ret = cfg::getByte("CLCExt", buffer, StatusItems[n].MARGIN_BOTTOM);
			StatusItems[n]. MARGIN_BOTTOM = ret;

			lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_BDRSTYLE");
			ret = cfg::getDword("CLCExt", buffer, StatusItems[n].BORDERSTYLE);
			StatusItems[n]. BORDERSTYLE = ret;
		}
	}
	if(cfg::dat.bFirstRun) {
		StatusItems_t *item = &StatusItems[ID_EXTBKBUTTONBAR - ID_STATUS_OFFLINE];

		item->COLOR = GetSysColor(COLOR_3DFACE);
		item->COLOR2 = GetSysColor(COLOR_3DFACE);

		item = &StatusItems[ID_EXTBKEVTAREA - ID_STATUS_OFFLINE];
		item->COLOR = item->COLOR2 = GetSysColor(COLOR_WINDOW);
		item->BORDERSTYLE = EDGE_ETCHED;
		SaveCompleteStructToDB();
	}
}

// writes whole struct to the database
static void SaveCompleteStructToDB(void)
{
    int n;
    char buffer[255];

    for (n = 0; n <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; n++) {
        if (StatusItems[n].statusID != ID_EXTBKSEPARATOR) {
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_IGNORE");
            cfg::writeByte("CLCExt", buffer, StatusItems[n].IGNORED);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_GRADIENT");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].GRADIENT);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_CORNER");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].CORNER);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].COLOR);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].COLOR2);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2_TRANSPARENT");
            cfg::writeByte("CLCExt", buffer, StatusItems[n].COLOR2_TRANSPARENT);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_TEXTCOLOR");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].TEXTCOLOR);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_ALPHA");
            cfg::writeByte("CLCExt", buffer, (BYTE)StatusItems[n].ALPHA);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_LEFT");
            cfg::writeByte("CLCExt", buffer, (BYTE)StatusItems[n].MARGIN_LEFT);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_TOP");
            cfg::writeByte("CLCExt", buffer, (BYTE)StatusItems[n].MARGIN_TOP);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_RIGHT");
            cfg::writeByte("CLCExt", buffer, (BYTE)StatusItems[n].MARGIN_RIGHT);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MRGN_BOTTOM");
            cfg::writeByte("CLCExt", buffer, (BYTE)StatusItems[n].MARGIN_BOTTOM);

            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_BDRSTYLE");
            cfg::writeDword("CLCExt", buffer, StatusItems[n].BORDERSTYLE);
        }
    }
}

void SetButtonToSkinned()
{
    int bSkinned = cfg::dat.bSkinnedButtonMode = cfg::getByte("CLCExt", "bskinned", 0);

    SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BM_SETSKINNED, 0, bSkinned);
    SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BM_SETSKINNED, 0, bSkinned);
    if(bSkinned) {
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN + 10, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN + 10, 0, 0);
    }
    else {
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN + 10, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN + 10, 0, 1);
    }
    SendMessage(g_hwndViewModeFrame, WM_USER + 100, 0, 0);
}

void Reload3dBevelColors()
{
    if(cfg::dat.hPen3DBright)
        DeleteObject(cfg::dat.hPen3DBright);
    if(cfg::dat.hPen3DDark)
        DeleteObject(cfg::dat.hPen3DDark);

    cfg::dat.hPen3DBright = CreatePen(PS_SOLID, 1, cfg::getDword("CLCExt", "3dbright", GetSysColor(COLOR_3DLIGHT)));
    cfg::dat.hPen3DDark = CreatePen(PS_SOLID, 1, cfg::getDword("CLCExt", "3ddark", GetSysColor(COLOR_3DSHADOW)));

}

// Save Non-StatusItems Settings
void SaveNonStatusItemsSettings(HWND hwndDlg)
{
    BOOL translated;

    cfg::writeByte("CLCExt", "EXBK_EqualSelection", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_EQUALSELECTION));
    cfg::writeByte("CLCExt", "EXBK_SelBlend", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SELBLEND));

    cfg::dat.cornerRadius = GetDlgItemInt(hwndDlg, IDC_CORNERRAD, &translated, FALSE);
    cfg::dat.bApplyIndentToBg = IsDlgButtonChecked(hwndDlg, IDC_APPLYINDENTBG) ? 1 : 0;
    cfg::dat.bUsePerProto = IsDlgButtonChecked(hwndDlg, IDC_USEPERPROTO) ? 1 : 0;
	cfg::dat.bWantFastGradients = IsDlgButtonChecked(hwndDlg, IDC_FASTGRADIENT) ? 1 : 0;
    cfg::dat.bOverridePerStatusColors = IsDlgButtonChecked(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR) ? 1 : 0;
    cfg::dat.titleBarHeight = (BYTE)GetDlgItemInt(hwndDlg, IDC_LASTITEMPADDING, &translated, FALSE);
    cfg::dat.group_padding = GetDlgItemInt(hwndDlg, IDC_GRPTOPPADDING, &translated, FALSE);

    cfg::writeByte("CLCExt", "CornerRad", cfg::dat.cornerRadius);
    cfg::writeByte("CLCExt", "applyindentbg", (BYTE)cfg::dat.bApplyIndentToBg);
    cfg::writeByte("CLCExt", "useperproto", (BYTE)cfg::dat.bUsePerProto);
    cfg::writeByte("CLCExt", "override_status", (BYTE)cfg::dat.bOverridePerStatusColors);
    cfg::writeByte("CLCExt", "bskinned", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SETALLBUTTONSKINNED) ? 1 : 0));
    cfg::writeByte("CLCExt", "FastGradients", cfg::dat.bWantFastGradients);
    cfg::writeByte("CLC", "IgnoreSelforGroups", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IGNORESELFORGROUPS));

    cfg::writeDword("CLCExt", "grp_padding", cfg::dat.group_padding);
    cfg::writeByte("CLCExt", "frame_height", cfg::dat.titleBarHeight);

    Reload3dBevelColors();
    SetButtonToSkinned();
}

/*
 * skin/theme related settings which are exported to/imported from the .ini style .clist file
 */

struct {char *szModule; char *szSetting; unsigned int size; int defaultval;} _tagSettings[] = {
    "CLCExt", "3dbright", 4, RGB(224, 225, 225),
    "CLCExt", "3ddark", 4, RGB(224, 225, 225),
    "CLCExt", "bskinned", 1, 0,
    "CLCExt", "CornerRad", 1, 0,
    "CLCExt", "applyindentbg", 1, 0,
    "CLCExt", "override_status", 1, 0,
    "CLCExt", "useperproto", 1, 0,
    "CLUI", "tb_skinned", 1, 0,
    "CLUI", "sb_skinned", 1, 0,
    "CLC", "RowGap", 1, 0,
    "CLC", "ExIconScale", 1, 0,
    "CLUI", "UseBkSkin", 1, 0,
    "CLUI", "clipborder", 1, 0,
    "CLUIFrames", "GapBetweenFrames", 4, 0,
    "CLC", "BkColour", 4, RGB(224, 224, 224),
    "CLCExt", "EXBK_CenterGroupnames", 1, 0,
    "CLUI", "TBSize", 1, 19,
    "CLC", "BkBmpUse", 2, 0,
    "CLUI", "clmargins", 4, 0,
    // frame stuff

    "WorldTime", "BgColour", 4, 0,
    "WorldTime", "FontCol", 4, 0,
    NULL, NULL, 0, 0
};
void extbk_export(char *file)
{
    int n, i;
    char buffer[255];
    char szSection[255];
    char szKey[255];
    DBVARIANT dbv = {0};
    DWORD data;

    data = 3;

    WritePrivateProfileStructA("Global", "Version", &data, 4, file);
    for (n = 0; n <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; n++) {
        if (StatusItems[n].statusID != ID_EXTBKSEPARATOR) {
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_ALPHA");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].ALPHA), sizeof(StatusItems[n].ALPHA), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR), sizeof(StatusItems[n].COLOR), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR2), sizeof(StatusItems[n].COLOR2), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2_TRANSPARENT");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR2_TRANSPARENT), sizeof(StatusItems[n].COLOR2_TRANSPARENT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_TEXTCOLOR");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].TEXTCOLOR), sizeof(StatusItems[n].TEXTCOLOR), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_CORNER");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].CORNER), sizeof(StatusItems[n].CORNER), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_GRADIENT");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].GRADIENT), sizeof(StatusItems[n].GRADIENT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_IGNORED");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].IGNORED), sizeof(StatusItems[n].IGNORED), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_BOTTOM");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_BOTTOM), sizeof(StatusItems[n].MARGIN_BOTTOM), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_LEFT");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_LEFT), sizeof(StatusItems[n].MARGIN_LEFT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_RIGHT");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_RIGHT), sizeof(StatusItems[n].MARGIN_RIGHT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_TOP");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_TOP), sizeof(StatusItems[n].MARGIN_TOP), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_BORDERSTYLE");
            WritePrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].BORDERSTYLE), sizeof(StatusItems[n].BORDERSTYLE), file);
        }
    }
    for(n = 0; n <= FONTID_LAST; n++) {
        mir_snprintf(szSection, 255, "Font%d", n);

        mir_snprintf(szKey, 255, "Font%dName", n);
        if(!cfg::getString(NULL, "CLC", szKey, &dbv)) {
            WritePrivateProfileStringA(szSection, "Name", dbv.pszVal, file);
            mir_free(dbv.pszVal);
        }
        mir_snprintf(szKey, 255, "Font%dSize", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "Size", &data, 1, file);

        mir_snprintf(szKey, 255, "Font%dSty", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "Style", &data, 1, file);

        mir_snprintf(szKey, 255, "Font%dSet", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "Set", &data, 1, file);

        mir_snprintf(szKey, 255, "Font%dCol", n);
        data = cfg::getDword("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "Color", &data, 4, file);

        mir_snprintf(szKey, 255, "Font%dFlags", n);
        data = (DWORD)cfg::getDword("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "Flags", &data, 4, file);

        mir_snprintf(szKey, 255, "Font%dAs", n);
        data = (DWORD)cfg::getWord("CLC", szKey, 8);
        WritePrivateProfileStructA(szSection, "SameAs", &data, 2, file);
    }
    i = 0;
    while(_tagSettings[i].szModule != NULL) {
        data = 0;
        switch(_tagSettings[i].size) {
            case 1:
                data = (DWORD)cfg::getByte( _tagSettings[i].szModule, _tagSettings[i].szSetting, (BYTE)_tagSettings[i].defaultval);
                break;
            case 2:
                data = (DWORD)cfg::getWord( _tagSettings[i].szModule, _tagSettings[i].szSetting, (DWORD)_tagSettings[i].defaultval);
                break;
            case 4:
                data = (DWORD)cfg::getDword( _tagSettings[i].szModule, _tagSettings[i].szSetting, (DWORD)_tagSettings[i].defaultval);
                break;
        }
        WritePrivateProfileStructA("Global", _tagSettings[i].szSetting, &data, _tagSettings[i].size, file);
        i++;
    }
    if(!cfg::getString(NULL, "CLC", "BkBitmap", &dbv)) {
        WritePrivateProfileStringA("Global", "BkBitmap", dbv.pszVal, file);
        DBFreeVariant(&dbv);
    }
}

DWORD __fastcall HexStringToLong(const char *szSource)
{
    char *stopped;
    COLORREF clr = strtol(szSource, &stopped, 16);
    if(clr == -1)
        return clr;
    return(RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
}

static StatusItems_t default_item =  {
	"{--Contact--}", "", 0,
     CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
     CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, -1,
     CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT, CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT,
     CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
};


static void PreMultiply(HBITMAP hBitmap, int mode)
{
	BYTE *p = NULL;
	DWORD dwLen;
    int width, height, x, y;
    BITMAP bmp;
    BYTE alpha;

	GetObject(hBitmap, sizeof(bmp), &bmp);
    width = bmp.bmWidth;
	height = bmp.bmHeight;
	dwLen = width * height * 4;
	p = (BYTE *)malloc(dwLen);
    if(p) {
        GetBitmapBits(hBitmap, dwLen, p);
        for (y = 0; y < height; ++y) {
            BYTE *px = p + width * 4 * y;

            for (x = 0; x < width; ++x) {
                if(mode) {
                    alpha = px[3];
                    px[0] = px[0] * alpha/255;
                    px[1] = px[1] * alpha/255;
                    px[2] = px[2] * alpha/255;
                }
                else
                    px[3] = 255;
                px += 4;
            }
        }
        dwLen = SetBitmapBits(hBitmap, dwLen, p);
        free(p);
    }
}

static void CorrectBitmap32Alpha(HBITMAP hBitmap)
{
	BITMAP bmp;
	DWORD dwLen;
	BYTE *p;
	int x, y;
    BOOL fixIt = TRUE;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	for (y = 0; y < bmp.bmHeight; ++y) {
        BYTE *px = p + bmp.bmWidth * 4 * y;

        for (x = 0; x < bmp.bmWidth; ++x)
		{
			if (px[3] != 0)
			{
				fixIt = FALSE;
			}
			else
			{
				px[3] = 255;
			}

			px += 4;
		}
	}

	if (fixIt)
	{
		SetBitmapBits(hBitmap, bmp.bmWidth * bmp.bmHeight * 4, p);
	}

	free(p);
}

static HBITMAP LoadPNG(const char *szFilename, ImageItem *item)
{
    HBITMAP hBitmap = 0;

    hBitmap = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)szFilename);
    if(hBitmap != 0)
        CorrectBitmap32Alpha(hBitmap);

    return hBitmap;
}

static void IMG_CreateItem(ImageItem *item, const char *fileName, HDC hdc)
{
    HBITMAP hbm = LoadPNG(fileName, item);
    BITMAP bm;

    if(hbm) {
        item->hbm = hbm;
        item->bf.BlendFlags = 0;
        item->bf.BlendOp = AC_SRC_OVER;
        item->bf.AlphaFormat = 0;

        GetObject(hbm, sizeof(bm), &bm);
        if(bm.bmBitsPixel == 32) {
            PreMultiply(hbm, 1);
            item->dwFlags |= IMAGE_PERPIXEL_ALPHA;
            item->bf.AlphaFormat = AC_SRC_ALPHA;
        }
        item->width = bm.bmWidth;
        item->height = bm.bmHeight;
        item->inner_height = item->height - item->bTop - item->bBottom;
        item->inner_width = item->width - item->bLeft - item->bRight;
        if(item->bTop && item->bBottom && item->bLeft && item->bRight) {
            item->dwFlags |= IMAGE_FLAG_DIVIDED;
            if(item->inner_height <= 0 || item->inner_width <= 0) {
                DeleteObject(hbm);
                item->hbm = 0;
                return;
            }
        }
        item->hdc = CreateCompatibleDC(hdc);
        item->hbmOld = reinterpret_cast<HBITMAP>(SelectObject(item->hdc, item->hbm));
    }
}

static void IMG_DeleteItem(ImageItem *item)
{
    if(!(item->dwFlags & IMAGE_GLYPH)) {
        SelectObject(item->hdc, item->hbmOld);
        DeleteObject(item->hbm);
        DeleteDC(item->hdc);
    }
    if(item->fillBrush)
        DeleteObject(item->fillBrush);
}

static void ReadItem(StatusItems_t *this_item, char *szItem, char *file)
{
    char buffer[512], def_color[20];
    COLORREF clr;

    StatusItems_t *defaults = &default_item;
    GetPrivateProfileStringA(szItem, "BasedOn", "None", buffer, 400, file);


    if(strcmp(buffer, "None")) {
        int i;

        for(i = 0; i <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; i++) {
            if(!_stricmp(StatusItems[i].szName[0] == '{' ? &StatusItems[i].szName[3] : StatusItems[i].szName, buffer)) {
                defaults = &StatusItems[i];
                break;
            }
        }
    }
    this_item->ALPHA = (int)GetPrivateProfileIntA(szItem, "Alpha", defaults->ALPHA, file);
    this_item->ALPHA = min(this_item->ALPHA, 100);

    clr = RGB(GetBValue(defaults->COLOR), GetGValue(defaults->COLOR), GetRValue(defaults->COLOR));
    _snprintf(def_color, 15, "%6.6x", clr);
    GetPrivateProfileStringA(szItem, "Color1", def_color, buffer, 400, file);
    this_item->COLOR = HexStringToLong(buffer);

    clr = RGB(GetBValue(defaults->COLOR2), GetGValue(defaults->COLOR2), GetRValue(defaults->COLOR2));
    _snprintf(def_color, 15, "%6.6x", clr);
    GetPrivateProfileStringA(szItem, "Color2", def_color, buffer, 400, file);
    this_item->COLOR2 = HexStringToLong(buffer);

    this_item->COLOR2_TRANSPARENT = (BYTE)GetPrivateProfileIntA(szItem, "COLOR2_TRANSPARENT", defaults->COLOR2_TRANSPARENT, file);

    this_item->CORNER = defaults->CORNER & CORNER_ACTIVE ? defaults->CORNER : 0;
    GetPrivateProfileStringA(szItem, "Corner", "None", buffer, 400, file);
    if(strstr(buffer, "tl"))
        this_item->CORNER |= CORNER_TL;
    if(strstr(buffer, "tr"))
        this_item->CORNER |= CORNER_TR;
    if(strstr(buffer, "bl"))
        this_item->CORNER |= CORNER_BL;
    if(strstr(buffer, "br"))
        this_item->CORNER |= CORNER_BR;
    if(this_item->CORNER)
        this_item->CORNER |= CORNER_ACTIVE;

    this_item->GRADIENT = defaults->GRADIENT & GRADIENT_ACTIVE ?  defaults->GRADIENT : 0;
    GetPrivateProfileStringA(szItem, "Gradient", "None", buffer, 400, file);
    if(strstr(buffer, "left"))
        this_item->GRADIENT = GRADIENT_RL;
    else if(strstr(buffer, "right"))
        this_item->GRADIENT = GRADIENT_LR;
    else if(strstr(buffer, "up"))
        this_item->GRADIENT = GRADIENT_BT;
    else if(strstr(buffer, "down"))
        this_item->GRADIENT = GRADIENT_TB;
    if(this_item->GRADIENT)
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
    ImageItem tmpItem, *newItem = NULL;
    char buffer[512], szItemNr[30];
    char szFinalName[MAX_PATH];
    HDC hdc = GetDC(pcli->hwndContactList);
    int i, n;
    BOOL alloced = FALSE;
    char szDrive[MAX_PATH], szPath[MAX_PATH];

    ZeroMemory(&tmpItem, sizeof(ImageItem));
    GetPrivateProfileStringA(itemname, "Glyph", "None", buffer, 500, szFileName);
    if(strcmp(buffer, "None")) {
        sscanf(buffer, "%d,%d,%d,%d", &tmpItem.glyphMetrics[0], &tmpItem.glyphMetrics[1],
               &tmpItem.glyphMetrics[2], &tmpItem.glyphMetrics[3]);
        if(tmpItem.glyphMetrics[2] > tmpItem.glyphMetrics[0] && tmpItem.glyphMetrics[3] > tmpItem.glyphMetrics[1]) {
            tmpItem.dwFlags |= IMAGE_GLYPH;
            tmpItem.glyphMetrics[2] = (tmpItem.glyphMetrics[2] - tmpItem.glyphMetrics[0]) + 1;
            tmpItem.glyphMetrics[3] = (tmpItem.glyphMetrics[3] - tmpItem.glyphMetrics[1]) + 1;
            goto done_with_glyph;
        }
    }
    GetPrivateProfileStringA(itemname, "Image", "None", buffer, 500, szFileName);
    if(strcmp(buffer, "None")) {

done_with_glyph:

        strncpy(tmpItem.szName, &itemname[1], sizeof(tmpItem.szName));
        tmpItem.szName[sizeof(tmpItem.szName) - 1] = 0;
        _splitpath(szFileName, szDrive, szPath, NULL, NULL);
        mir_snprintf(szFinalName, MAX_PATH, "%s\\%s\\%s", szDrive, szPath, buffer);
        tmpItem.alpha = GetPrivateProfileIntA(itemname, "Alpha", 100, szFileName);
        tmpItem.alpha = min(tmpItem.alpha, 100);
        tmpItem.alpha = (BYTE)((FLOAT)(((FLOAT) tmpItem.alpha) / 100) * 255);
        tmpItem.bf.SourceConstantAlpha = tmpItem.alpha;
        tmpItem.bLeft = GetPrivateProfileIntA(itemname, "Left", 0, szFileName);
        tmpItem.bRight = GetPrivateProfileIntA(itemname, "Right", 0, szFileName);
        tmpItem.bTop = GetPrivateProfileIntA(itemname, "Top", 0, szFileName);
        tmpItem.bBottom = GetPrivateProfileIntA(itemname, "Bottom", 0, szFileName);
        if(tmpItem.dwFlags & IMAGE_GLYPH) {
            tmpItem.width = tmpItem.glyphMetrics[2];
            tmpItem.height = tmpItem.glyphMetrics[3];
            tmpItem.inner_height = tmpItem.glyphMetrics[3] - tmpItem.bTop - tmpItem.bBottom;
            tmpItem.inner_width = tmpItem.glyphMetrics[2] - tmpItem.bRight - tmpItem.bLeft;

            if(tmpItem.bTop && tmpItem.bBottom && tmpItem.bLeft && tmpItem.bRight)
                tmpItem.dwFlags |= IMAGE_FLAG_DIVIDED;
            tmpItem.bf.BlendFlags = 0;
            tmpItem.bf.BlendOp = AC_SRC_OVER;
            tmpItem.bf.AlphaFormat = 0;
            tmpItem.dwFlags |= IMAGE_PERPIXEL_ALPHA;
            tmpItem.bf.AlphaFormat = AC_SRC_ALPHA;
            if(tmpItem.inner_height <= 0 || tmpItem.inner_width <= 0) {
                ReleaseDC(pcli->hwndContactList, hdc);
                return;
            }
        }
        GetPrivateProfileStringA(itemname, "Fillcolor", "None", buffer, 500, szFileName);
        if(strcmp(buffer, "None")) {
            COLORREF fillColor = HexStringToLong(buffer);
            tmpItem.fillBrush = CreateSolidBrush(fillColor);
            tmpItem.dwFlags |= IMAGE_FILLSOLID;
        }
        else
            tmpItem.fillBrush = 0;

        GetPrivateProfileStringA(itemname, "Stretch", "None", buffer, 500, szFileName);
        if(buffer[0] == 'B' || buffer[0] == 'b')
            tmpItem.bStretch = IMAGE_STRETCH_B;
        else if(buffer[0] == 'h' || buffer[0] == 'H')
            tmpItem.bStretch = IMAGE_STRETCH_V;
        else if(buffer[0] == 'w' || buffer[0] == 'W')
            tmpItem.bStretch = IMAGE_STRETCH_H;
        tmpItem.hbm = 0;

        if(!_stricmp(itemname, "$glyphs")) {
            IMG_CreateItem(&tmpItem, szFinalName, hdc);
            if(tmpItem.hbm) {
                newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
                ZeroMemory(newItem, sizeof(ImageItem));
                *newItem = tmpItem;
                g_glyphItem = newItem;
            }
            goto imgread_done;
        }
        if(itemname[0] == '@') {
            if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                IMG_CreateItem(&tmpItem, szFinalName, hdc);
            if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
                ImageItem *pItem = g_ImageItems;

                newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
                ZeroMemory(newItem, sizeof(ImageItem));
                *newItem = tmpItem;

                if(g_ImageItems == NULL)
                    g_ImageItems = newItem;
                else {
                    ImageItem *pItem = g_ImageItems;

                    while(pItem->nextItem != 0)
                        pItem = pItem->nextItem;
                    pItem->nextItem = newItem;
                }
            }
            goto imgread_done;
        }
        for(n = 0;;n++) {
            mir_snprintf(szItemNr, 30, "Item%d", n);
            GetPrivateProfileStringA(itemname, szItemNr, "None", buffer, 500, szFileName);
            if(!strcmp(buffer, "None"))
                break;
            if(!strcmp(buffer, "CLUI")) {
                if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                    IMG_CreateItem(&tmpItem, szFinalName, hdc);
                if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
                    COLORREF clr;

                    newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
                    ZeroMemory(newItem, sizeof(ImageItem));
                    *newItem = tmpItem;
                    g_CLUIImageItem = newItem;
                    GetPrivateProfileStringA(itemname, "Colorkey", "e5e5e5", buffer, 500, szFileName);
                    clr = HexStringToLong(buffer);
                    cfg::dat.colorkey = clr;
                    cfg::writeDword("CLUI", "ColorKey", clr);
                    if(g_CLUISkinnedBkColor)
                        DeleteObject(g_CLUISkinnedBkColor);
                    g_CLUISkinnedBkColor = CreateSolidBrush(clr);
                    g_CLUISkinnedBkColorRGB = clr;
                }
                continue;
            }
            for(i = 0; i <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; i++) {
                if(!_stricmp(StatusItems[i].szName[0] == '{' ? &StatusItems[i].szName[3] : StatusItems[i].szName, buffer)) {
                    if(!alloced) {
                        if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                            IMG_CreateItem(&tmpItem, szFinalName, hdc);
                        if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
                            newItem = reinterpret_cast<ImageItem *>(malloc(sizeof(ImageItem)));
                            ZeroMemory(newItem, sizeof(ImageItem));
                            *newItem = tmpItem;
                            StatusItems[i].imageItem = newItem;
                            if(g_ImageItems == NULL)
                                g_ImageItems = newItem;
                            else {
                                ImageItem *pItem = g_ImageItems;

                                while(pItem->nextItem != 0)
                                    pItem = pItem->nextItem;
                                pItem->nextItem = newItem;
                            }
                            alloced = TRUE;
                        }
                    }
                    else if(newItem != NULL)
                        StatusItems[i].imageItem = newItem;
                }
            }
        }
    }
imgread_done:
    ReleaseDC(pcli->hwndContactList, hdc);
}

void IMG_DeleteItems()
{
    ImageItem *pItem = g_ImageItems, *pNextItem;
    ButtonItem *pbItem = g_ButtonItems, *pbNextItem;

	int i;

	while(pItem) {
        IMG_DeleteItem(pItem);
        pNextItem = pItem->nextItem;
        free(pItem);
        pItem = pNextItem;
    }
    g_ImageItems = NULL;
    while(pbItem) {
        DestroyWindow(pbItem->hWnd);
        pbNextItem = pbItem->nextItem;
        free(pbItem);
        pbItem = pbNextItem;
    }
    g_ButtonItems = NULL;

    if(g_CLUIImageItem) {
        IMG_DeleteItem(g_CLUIImageItem);
        free(g_CLUIImageItem);
    }
    g_CLUIImageItem = NULL;

    if(g_glyphItem) {
        IMG_DeleteItem(g_glyphItem);
        free(g_glyphItem);
    }
    g_glyphItem = NULL;

    for(i = 0; i <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; i++)
        StatusItems[i].imageItem = NULL;
}

static UINT nextButtonID = IDC_TBFIRSTUID;

static void BTN_ReadItem(char *itemName, char *file)
{
    ButtonItem tmpItem, *newItem;
    char szBuffer[1024];
    ImageItem *imgItem = g_ImageItems;

    ZeroMemory(&tmpItem, sizeof(tmpItem));
    mir_snprintf(tmpItem.szName, sizeof(tmpItem.szName), "%s", &itemName[1]);
    tmpItem.width = GetPrivateProfileIntA(itemName, "Width", 16, file);
    tmpItem.height = GetPrivateProfileIntA(itemName, "Height", 16, file);
    tmpItem.xOff = GetPrivateProfileIntA(itemName, "xoff", 0, file);
    tmpItem.yOff = GetPrivateProfileIntA(itemName, "yoff", 0, file);

    tmpItem.dwFlags |= GetPrivateProfileIntA(itemName, "toggle", 0, file) ? BUTTON_ISTOGGLE : 0;

    GetPrivateProfileStringA(itemName, "Pressed", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgPressed = StatusItems[ID_EXTBKTBBUTTONSPRESSED - ID_STATUS_OFFLINE].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(imgItem->szName, szBuffer)) {
                tmpItem.imgPressed = imgItem;
                break;
            }
            imgItem = imgItem->nextItem;
        }
    }

    imgItem = g_ImageItems;
    GetPrivateProfileStringA(itemName, "Normal", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgNormal = StatusItems[ID_EXTBKTBBUTTONSNPRESSED - ID_STATUS_OFFLINE].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(imgItem->szName, szBuffer)) {
                tmpItem.imgNormal = imgItem;
                break;
            }
            imgItem = imgItem->nextItem;
        }
    }

    imgItem = g_ImageItems;
    GetPrivateProfileStringA(itemName, "Hover", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgHover = StatusItems[ID_EXTBKTBBUTTONMOUSEOVER - ID_STATUS_OFFLINE].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(imgItem->szName, szBuffer)) {
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
    if(!_stricmp(szBuffer, "service")) {
        tmpItem.szService[0] = 0;
        GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None")) {
            mir_snprintf(tmpItem.szService, 256, "%s", szBuffer);
            tmpItem.dwFlags |= BUTTON_ISSERVICE;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(!_stricmp(szBuffer, "protoservice")) {
        tmpItem.szService[0] = 0;
        GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None")) {
            mir_snprintf(tmpItem.szService, 256, "%s", szBuffer);
            tmpItem.dwFlags |= BUTTON_ISPROTOSERVICE;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(!_stricmp(szBuffer, "database")) {
        int n;

        GetPrivateProfileStringA(itemName, "Module", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None"))
            mir_snprintf(tmpItem.szModule, 256, "%s", szBuffer);
        GetPrivateProfileStringA(itemName, "Setting", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None"))
            mir_snprintf(tmpItem.szSetting, 256, "%s", szBuffer);
        if(GetPrivateProfileIntA(itemName, "contact", 0, file) != 0)
           tmpItem.dwFlags |= BUTTON_DBACTIONONCONTACT;

        for(n = 0; n <= 1; n++) {
            char szKey[20];
            BYTE *pValue;

            strcpy(szKey, n == 0 ? "dbonpush" : "dbonrelease");
            pValue = (n == 0 ? tmpItem.bValuePush : tmpItem.bValueRelease);

            GetPrivateProfileStringA(itemName, szKey, "None", szBuffer, 1000, file);
            switch(szBuffer[0]) {
                case 'b':
                {
                    BYTE value = (BYTE)atol(&szBuffer[1]);
                    pValue[0] = value;
                    tmpItem.type = DBVT_BYTE;
                    break;
                }
                case 'w':
                {
                    WORD value = (WORD)atol(&szBuffer[1]);
                    *((WORD *)&pValue[0]) = value;
                    tmpItem.type = DBVT_WORD;
                    break;
                }
                case 'd':
                {
                    DWORD value = (DWORD)atol(&szBuffer[1]);
                    *((DWORD *)&pValue[0]) = value;
                    tmpItem.type = DBVT_DWORD;
                    break;
                }
                case 's':
                {
                    mir_snprintf((char *)pValue, 256, &szBuffer[1]);
                    tmpItem.type = DBVT_ASCIIZ;
                    break;
                }
            }
        }
        if(tmpItem.szModule[0] && tmpItem.szSetting[0]) {
            tmpItem.dwFlags |= BUTTON_ISDBACTION;
            if(tmpItem.szModule[0] == '$' && (tmpItem.szModule[1] == 'c' || tmpItem.szModule[1] == 'C'))
                tmpItem.dwFlags |= BUTTON_ISCONTACTDBACTION;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(_stricmp(szBuffer, "Custom")) {
        int i = 0;

        while(top_buttons[i].id) {
            if(!_stricmp(top_buttons[i].szIcoLibIcon, szBuffer)) {
                tmpItem.uId = top_buttons[i].id;
                tmpItem.dwFlags |= BUTTON_ISINTERNAL;
                break;
            }
            i++;
        }
    }
    GetPrivateProfileStringA(itemName, "PassContact", "None", szBuffer, 1000, file);
    if(_stricmp(szBuffer, "None")) {
        if(szBuffer[0] == 'w' || szBuffer[0] == 'W')
            tmpItem.dwFlags |= BUTTON_PASSHCONTACTW;
        else if(szBuffer[0] == 'l' || szBuffer[0] == 'L')
            tmpItem.dwFlags |= BUTTON_PASSHCONTACTL;
    }

    GetPrivateProfileStringA(itemName, "Tip", "None", szBuffer, 1000, file);
    if(strcmp(szBuffer, "None")) {
#if defined(_UNICODE)
        MultiByteToWideChar(cfg::dat.langPackCP, 0, szBuffer, -1, tmpItem.szTip, 256);
        tmpItem.szTip[255] = 0;
#else
        mir_snprintf(tmpItem.szTip, 256, "%s", szBuffer);
#endif
    }
    else
        tmpItem.szTip[0] = 0;

    // create it

    newItem = (ButtonItem *)malloc(sizeof(ButtonItem));
    ZeroMemory(newItem, sizeof(ButtonItem));
    if(g_ButtonItems == NULL) {
        g_ButtonItems = newItem;
        *newItem = tmpItem;
        newItem->nextItem = 0;
    }
    else {
        ButtonItem *curItem = g_ButtonItems;
        while(curItem->nextItem)
            curItem = curItem->nextItem;
        *newItem = tmpItem;
        newItem->nextItem = 0;
        curItem->nextItem = newItem;
    }
    newItem->hWnd = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 5, 5, pcli->hwndContactList, (HMENU)newItem->uId, g_hInst, NULL);
    SendMessage(newItem->hWnd, BM_SETBTNITEM, 0, (LPARAM)newItem);
    SendMessage(newItem->hWnd, BUTTONSETASFLATBTN, 0, 0);
    SendMessage(newItem->hWnd, BUTTONSETASFLATBTN + 10, 0, 0);
    if(newItem->dwFlags & BUTTON_ISTOGGLE)
        SendMessage(newItem->hWnd, BUTTONSETASPUSHBTN, 0, 0);

    if(newItem->szTip[0])
        SendMessage(newItem->hWnd, BUTTONADDTOOLTIP, (WPARAM)newItem->szTip, 0);
    return;
}

void IMG_LoadItems()
{
    char *szSections = NULL;
    char *p;
    DBVARIANT dbv;
    char szFileName[MAX_PATH];
    TCHAR tszFileName[MAX_PATH];
    int  i = 0;

    if(cfg::getTString(NULL, "CLC", "AdvancedSkin", &dbv))
        return;

    MY_pathToAbsolute(dbv.ptszVal, tszFileName);

    /* TODO
     * rewrite the skin loading in TCHAR manner
     */

#if defined(_UNICODE)
    WideCharToMultiByte(CP_ACP, 0, tszFileName, MAX_PATH, szFileName, MAX_PATH, 0, 0);
#endif

    DBFreeVariant(&dbv);

    if(!PathFileExists(tszFileName))
        return;

	IMG_DeleteItems();

    szSections = reinterpret_cast<char *>(malloc(3002));
    ZeroMemory(szSections, 3002);
    p = szSections;
    GetPrivateProfileSectionNamesA(szSections, 3000, szFileName);

    szSections[3001] = szSections[3000] = 0;
    p = szSections;
    while(lstrlenA(p) > 1) {
		if(p[0] == '$' || p[0] == '@')
            IMG_ReadItem(p, szFileName);
        p += (lstrlenA(p) + 1);
    }
    nextButtonID = IDC_TBFIRSTUID;
    p = szSections;
    while(lstrlenA(p) > 1) {
        if(p[0] == '!')
            BTN_ReadItem(p, szFileName);
        p += (lstrlenA(p) + 1);
    }
    if(pcli && pcli->hwndContactList)
        SetButtonStates(pcli->hwndContactList);
    free(szSections);

    if(g_ButtonItems) {
        while(top_buttons[i].id) {
            if(top_buttons[i].hwnd != 0 && top_buttons[i].id != IDC_TBGLOBALSTATUS && top_buttons[i].id != IDC_TBMENU) {
                DestroyWindow(top_buttons[i].hwnd);
                top_buttons[i].hwnd = 0;
            }
            i++;
        }
        cfg::dat.dwFlags &= ~CLUI_FRAME_SHOWTOPBUTTONS;
        ConfigureCLUIGeometry(0);
    }
    if(g_ImageItems) {
    	cfg::writeByte("CLCExt", "bskinned", 1);
        SetButtonToSkinned();
    }
    if(g_CLUIImageItem) {
        cfg::dat.bFullTransparent = TRUE;
        cfg::dat.dwFlags &= ~CLUI_FRAME_CLISTSUNKEN;
        cfg::writeByte("CLUI", "fulltransparent", (BYTE)cfg::dat.bFullTransparent);
        cfg::writeByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_NOBORDER);
        ApplyCLUIBorderStyle(pcli->hwndContactList);
        SetWindowLong(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
        API::SetLayeredWindowAttributes(pcli->hwndContactList, cfg::dat.colorkey, 0, LWA_COLORKEY);
    }
    CoolSB_SetupScrollBar();
}

void LoadPerContactSkins(TCHAR *tszFileName)
{
    char *p, *szProto, *uid, szItem[100];
    char *szSections = reinterpret_cast<char *>(malloc(3002));
    StatusItems_t *items = NULL, *this_item;
    HANDLE hContact;
    int i = 1;
#if defined(_UNICODE)
    char    file[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, tszFileName, MAX_PATH, file, MAX_PATH, 0, 0);
    file[MAX_PATH - 1] = 0;
#else
    char    *file = tszFileName;
#endif

    ReadItem(&default_item, "%Default", file);
    ZeroMemory(szSections, 3000);
    p = szSections;
    GetPrivateProfileSectionNamesA(szSections, 3000, file);
    szSections[3001] = szSections[3000] = 0;
    p = szSections;
    while(lstrlenA(p) > 1) {
        if(p[0] == '%') {
            p += (lstrlenA(p) + 1);
            continue;
        }
        items = reinterpret_cast<StatusItems_t *>(realloc(items, i * sizeof(StatusItems_t)));
        ZeroMemory(&items[i - 1], sizeof(StatusItems_t));
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
        p += (lstrlenA(p) + 1);
        i++;
    }

    if(items) {
        hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
        while(hContact) {
            char UIN[40];
            int j;

            szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
            if(szProto) {
                uid = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
                if ((INT_PTR) uid != CALLSERVICE_NOTFOUND && uid != NULL) {
                    DBVARIANT dbv = {0};

                    DBGetContactSetting(hContact, szProto, uid, &dbv);
                    switch(dbv.type) {
                        case DBVT_DWORD:
                            mir_snprintf(UIN, 40, "%d", dbv.dVal);
                            break;
                        case DBVT_ASCIIZ:
                            mir_snprintf(UIN, 40, "%s", dbv.pszVal);
                            DBFreeVariant(&dbv);
                            break;
                        default:
                            UIN[0] = 0;
                            break;
                    }
                    for(j = 0; j < i - 1; j++) {
                        if(!strcmp(szProto, items[j].szName) && !strcmp(UIN, items[j].szDBname)
                           && lstrlenA(szProto) == lstrlenA(items[j].szName) && lstrlenA(UIN) == lstrlenA(items[j].szDBname)) {

                            //_DebugPopup(hContact, "Found: %s, %s", szProto, UIN);
                        	cfg::writeDword(hContact, "EXTBK", "TEXT", items[j].TEXTCOLOR);
                        	cfg::writeDword(hContact, "EXTBK", "COLOR1", items[j].COLOR);
                        	cfg::writeDword(hContact, "EXTBK", "COLOR2", items[j].COLOR2);
                        	cfg::writeByte(hContact, "EXTBK", "ALPHA", (BYTE)items[j].ALPHA);

                        	cfg::writeByte(hContact, "EXTBK", "LEFT", (BYTE)items[j].MARGIN_LEFT);
                        	cfg::writeByte(hContact, "EXTBK", "RIGHT", (BYTE)items[j].MARGIN_RIGHT);
                        	cfg::writeByte(hContact, "EXTBK", "TOP", (BYTE)items[j].MARGIN_TOP);
                        	cfg::writeByte(hContact, "EXTBK", "BOTTOM", (BYTE)items[j].MARGIN_BOTTOM);

                        	cfg::writeByte(hContact, "EXTBK", "TRANS", items[j].COLOR2_TRANSPARENT);
                        	cfg::writeDword(hContact, "EXTBK", "BDR", items[j].BORDERSTYLE);

                        	cfg::writeByte(hContact, "EXTBK", "CORNER", items[j].CORNER);
                        	cfg::writeByte(hContact, "EXTBK", "GRAD", items[j].GRADIENT);
                        	cfg::writeByte(hContact, "EXTBK", "TRANS", items[j].COLOR2_TRANSPARENT);

                        	cfg::writeByte(hContact, "EXTBK", "VALID", 1);
                            break;
                        }
                    }
                    if(j == i - 1) {            // disable the db copy if it has been disabled in the skin .ini file
                        if(cfg::getByte(hContact, "EXTBK", "VALID", 0))
                        	cfg::writeByte(hContact, "EXTBK", "VALID", 0);
                    }
                }
            }
            hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
        }
    }
    free(szSections);
    free(items);
}

void extbk_import(char *file, HWND hwndDlg)
{
    int n, i;
    char buffer[255];
    char szKey[255], szSection[255];
    DWORD data, version = 0;
    int oldexIconScale = cfg::dat.exIconScale;

    for (n = 0; n <= ID_EXTBK_LAST - ID_STATUS_OFFLINE; n++) {
        if (StatusItems[n].statusID != ID_EXTBKSEPARATOR) {
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_ALPHA");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].ALPHA), sizeof(StatusItems[n].ALPHA), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR), sizeof(StatusItems[n].COLOR), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR2), sizeof(StatusItems[n].COLOR2), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_COLOR2_TRANSPARENT");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].COLOR2_TRANSPARENT), sizeof(StatusItems[n].COLOR2_TRANSPARENT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_TEXTCOLOR");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].TEXTCOLOR), sizeof(StatusItems[n].TEXTCOLOR), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_CORNER");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].CORNER), sizeof(StatusItems[n].CORNER), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_GRADIENT");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].GRADIENT), sizeof(StatusItems[n].GRADIENT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_IGNORED");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].IGNORED), sizeof(StatusItems[n].IGNORED), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_BOTTOM");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_BOTTOM), sizeof(StatusItems[n].MARGIN_BOTTOM), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_LEFT");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_LEFT), sizeof(StatusItems[n].MARGIN_LEFT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_RIGHT");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_RIGHT), sizeof(StatusItems[n].MARGIN_RIGHT), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_MARGIN_TOP");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].MARGIN_TOP), sizeof(StatusItems[n].MARGIN_TOP), file);
            lstrcpyA(buffer, StatusItems[n].szDBname); lstrcatA(buffer, "_BORDERSTYLE");
            GetPrivateProfileStructA("ExtBKSettings", buffer, &(StatusItems[n].BORDERSTYLE), sizeof(StatusItems[n].BORDERSTYLE), file);
        }
    }

    data = 0;
    GetPrivateProfileStructA("Global", "Version", &version, 4, file);
    if(version >= 2) {
        for(n = 0; n <= FONTID_LAST; n++) {
            mir_snprintf(szSection, 255, "Font%d", n);

            mir_snprintf(szKey, 255, "Font%dName", n);
            GetPrivateProfileStringA(szSection, "Name", "Arial", buffer, sizeof(buffer), file);
            cfg::writeString(NULL, "CLC", szKey, buffer);

            mir_snprintf(szKey, 255, "Font%dSize", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Size", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dSty", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Style", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dSet", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Set", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dCol", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Color", &data, 4, file);
            cfg::writeDword("CLC", szKey, data);

            mir_snprintf(szKey, 255, "Font%dFlags", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Flags", &data, 4, file);
            cfg::writeDword("CLC", szKey, (WORD)data);

            mir_snprintf(szKey, 255, "Font%dAs", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "SameAs", &data, 2, file);
            cfg::writeDword("CLC", szKey, (WORD)data);
        }
    }
    i = 0;
    if(version >= 3) {
        char szString[MAX_PATH];
        szString[0] = 0;

        while(_tagSettings[i].szModule != NULL) {
            data = 0;
            GetPrivateProfileStructA("Global", _tagSettings[i].szSetting, &data, _tagSettings[i].size, file);
            switch(_tagSettings[i].size) {
                case 1:
                	cfg::writeByte(NULL, _tagSettings[i].szModule, _tagSettings[i].szSetting, (BYTE)data);
                    break;
                case 4:
                	cfg::writeDword(NULL, _tagSettings[i].szModule, _tagSettings[i].szSetting, data);
                    break;
                case 2:
                    cfg::writeWord( _tagSettings[i].szModule, _tagSettings[i].szSetting, (WORD)data);
                    break;
            }
            i++;
        }
        GetPrivateProfileStringA("Global", "BkBitmap", "", szString, MAX_PATH, file);
        if(lstrlenA(szString) > 0)
        	cfg::writeString(NULL, "CLC", "BkBitmap", szString);
    }

    Reload3dBevelColors();
    ReloadThemedOptions();
    SetTBSKinned(cfg::dat.bSkinnedToolbar);
    // refresh
    if(hwndDlg && ServiceExists(MS_CLNSE_FILLBYCURRENTSEL))
        CallService(MS_CLNSE_FILLBYCURRENTSEL, (WPARAM)hwndDlg, 0);
    pcli->pfnClcOptionsChanged();
    ConfigureCLUIGeometry(1);
    SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
    RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);
    if(oldexIconScale != cfg::dat.exIconScale) {
        ImageList_SetIconSize(himlExtraImages, cfg::dat.exIconScale, cfg::dat.exIconScale);
        if(cfg::dat.IcoLib_Avail)
            IcoLibReloadIcons();
        else {
            CLN_LoadAllIcons(0);
            //FYR: may be better to call pfnReloadProtoMenus
            pcli->pfnReloadProtoMenus();
            //FYR: Not necessary. It is already notified in pfnReloadProtoMenus
            //NotifyEventHooks(pcli->hPreBuildStatusMenuEvent, 0, 0);
            ReloadExtraIcons();
        }
        pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
    }
}

static void ApplyCLUISkin()
{
    DBVARIANT   dbv = {0};
    TCHAR       tszFinalName[MAX_PATH];
    char        szFinalName[MAX_PATH];
    if(!cfg::getTString(NULL, "CLC", "AdvancedSkin", &dbv)) {
        MY_pathToAbsolute(dbv.ptszVal, tszFinalName);
#if defined(_UNICODE)
        WideCharToMultiByte(CP_ACP, 0, tszFinalName, MAX_PATH, szFinalName, MAX_PATH, 0, 0);
#else
        mir_sntprintf(szFinalName, MAX_PATH, _T("%s"), tszFinalName);
#endif
        if(cfg::getByte("CLUI", "skin_changed", 0)) {
            extbk_import(szFinalName, 0);
            SaveCompleteStructToDB();
            cfg::writeByte("CLUI", "skin_changed", 0);
        }
        IMG_LoadItems();
        ShowWindow(pcli->hwndContactList, SW_SHOWNORMAL);
        SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
        RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
        DBFreeVariant(&dbv);
    }
}

static INT_PTR CALLBACK DlgProcSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
        {
            DBVARIANT dbv;
            TranslateDialogDefault(hwndDlg);

            CheckDlgButton(hwndDlg, IDC_EQUALSELECTION, (cfg::getByte("CLCExt", "EXBK_EqualSelection", 1) == 1) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_SELBLEND, cfg::getByte("CLCExt", "EXBK_SelBlend", 1));
            CheckDlgButton(hwndDlg, IDC_SETALLBUTTONSKINNED, cfg::getByte("CLCExt", "bskinned", 0));

            SendDlgItemMessage(hwndDlg, IDC_CORNERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
            SendDlgItemMessage(hwndDlg, IDC_CORNERSPIN, UDM_SETPOS, 0, cfg::dat.cornerRadius);

            SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(20, 0));
            SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.group_padding);

            SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(40, 0));
            SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.titleBarHeight);

            CheckDlgButton(hwndDlg, IDC_APPLYINDENTBG, cfg::dat.bApplyIndentToBg);
            CheckDlgButton(hwndDlg, IDC_USEPERPROTO, cfg::dat.bUsePerProto);
            CheckDlgButton(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR, cfg::dat.bOverridePerStatusColors);
            CheckDlgButton(hwndDlg, IDC_FASTGRADIENT, cfg::dat.bWantFastGradients);
            CheckDlgButton(hwndDlg, IDC_IGNORESELFORGROUPS, cfg::getByte("CLC", "IgnoreSelforGroups", 0) ? BST_CHECKED : BST_UNCHECKED);


            if(!cfg::getString(NULL, "CLC", "ContactSkins", &dbv)) {
                SetDlgItemTextA(hwndDlg, IDC_SKINFILE, dbv.pszVal);
                DBFreeVariant(&dbv);
                Utils::enableDlgControl(hwndDlg, IDC_RELOAD, TRUE);
            }
            else
            	Utils::enableDlgControl(hwndDlg, IDC_RELOAD, FALSE);
            CheckDlgButton(hwndDlg, IDC_USESKIN, cfg::getByte("CLUI", "useskin", 0) ? BST_CHECKED : BST_UNCHECKED);
            if(!cfg::getTString(NULL, "CLC", "AdvancedSkin", &dbv)) {
                SetDlgItemText(hwndDlg, IDC_SKINFILENAME, dbv.ptszVal);
                DBFreeVariant(&dbv);
            }
            else
                SetDlgItemText(hwndDlg, IDC_SKINFILENAME, _T(""));
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_USESKIN:
                {
                    int useskin = IsDlgButtonChecked(hwndDlg, IDC_USESKIN);

                    cfg::writeByte("CLUI", "useskin", (BYTE)(useskin ? 1 : 0));
                    break;
                }
                case IDC_UNLOAD:
                    IMG_DeleteItems();
                    CreateButtonBar(pcli->hwndContactList);
                    if(cfg::dat.bSkinnedToolbar)
                        SetTBSKinned(1);
                    ConfigureFrame();
                    SetButtonStates(pcli->hwndContactList);
                    SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
                    PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
                    break;
                case IDC_SELECTSKINFILE:
                    {
                        OPENFILENAME    ofn = {0};
                        TCHAR           str[MAX_PATH] = _T("*.clist"), final_path[MAX_PATH];

                        ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
                        ofn.hwndOwner = hwndDlg;
                        ofn.hInstance = NULL;
                        ofn.lpstrFilter = _T("*.clist\0");
                        ofn.lpstrFile = str;
                        ofn.Flags = OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.nMaxFileTitle = MAX_PATH;
                        ofn.lpstrDefExt = _T("");
                        if (!GetOpenFileName(&ofn))
                            break;
                        MY_pathToRelative(str, final_path);
                        if(PathFileExists(str)) {
                            int skinChanged = 0;
                            DBVARIANT dbv = {0};

                            if(!cfg::getTString(NULL, "CLC", "AdvancedSkin", &dbv)) {
                                if(_tcscmp(dbv.ptszVal, final_path))
                                    skinChanged = TRUE;
                                DBFreeVariant(&dbv);
                            }
                            else
                                skinChanged = TRUE;
                            cfg::writeTString(NULL, "CLC", "AdvancedSkin", final_path);
                            cfg::writeByte("CLUI", "skin_changed", (BYTE)skinChanged);
                            SetDlgItemText(hwndDlg, IDC_SKINFILENAME, final_path);
                        }
                        break;
                    }
                case IDC_RELOADSKIN:
                	cfg::writeByte("CLUI", "skin_changed", 1);
                    ApplyCLUISkin();
                    break;
                case IDC_RELOAD:
                    {
                        TCHAR   tszFilename[MAX_PATH], tszFinalPath[MAX_PATH];

                        GetDlgItemText(hwndDlg, IDC_SKINFILE, tszFilename, MAX_PATH);
                        tszFilename[MAX_PATH - 1] = 0;
                        MY_pathToAbsolute(tszFilename, tszFinalPath);
                        if(PathFileExists(tszFinalPath)) {
                            LoadPerContactSkins(tszFinalPath);
                            ReloadSkinItemsToCache();
                            pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
                        }
                        break;
                    }
            }
            if ((LOWORD(wParam) == IDC_SKINFILE || LOWORD(wParam) == IDC_SKINFILENAME)
                && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
                return 0;
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY:
                            SaveNonStatusItemsSettings(hwndDlg);
                            pcli->pfnClcOptionsChanged();
                            PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
                            return TRUE;
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static int iInit = TRUE;
   static HWND hwndSkinEdit = 0;

   switch(msg)
   {
      case WM_INITDIALOG:
      {
         TCITEM tci;
         RECT rcClient;
         int oPage = cfg::getByte("CLUI", "opage", 0);
         SKINDESCRIPTION sd;

         TranslateDialogDefault(hwnd);
         GetClientRect(hwnd, &rcClient);
         iInit = TRUE;
         tci.mask = TCIF_PARAM|TCIF_TEXT;
         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SKIN), hwnd, DlgProcSkinOpts);
         tci.pszText = TranslateT("Load and apply");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,5,25,rcClient.right-9,rcClient.bottom-60,1);
         ShowWindow((HWND)tci.lParam, oPage == 0 ? SW_SHOW : SW_HIDE);
         if(IS_THEMED)
             API::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

         if(ServiceExists(MS_CLNSE_INVOKE)) {
             ZeroMemory(&sd, sizeof(sd));
             sd.cbSize = sizeof(sd);
             sd.StatusItems = StatusItems;
             sd.hWndParent = hwnd;
             sd.hWndTab = GetDlgItem(hwnd, IDC_OPTIONSTAB);
             sd.pfnSaveCompleteStruct = SaveCompleteStructToDB;
             sd.lastItem = ID_EXTBK_LAST;
             sd.firstItem = ID_STATUS_OFFLINE;
             sd.pfnClcOptionsChanged = pcli->pfnClcOptionsChanged;
             sd.hwndCLUI = pcli->hwndContactList;
             hwndSkinEdit = (HWND)CallService(MS_CLNSE_INVOKE, 0, (LPARAM)&sd);
         }

         if(hwndSkinEdit) {
             ShowWindow(hwndSkinEdit, oPage == 1 ? SW_SHOW : SW_HIDE);
             TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
             if(IS_THEMED)
                 API::pfnEnableThemeDialogTexture(hwndSkinEdit, ETDT_ENABLETAB);
         }

         TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
         Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
         Utils::enableDlgControl(hwnd, IDC_IMPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
         iInit = FALSE;
         return FALSE;
      }

      case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
         if(!iInit)
             SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
         break;
      case WM_COMMAND:
          switch(LOWORD(wParam)) {
              case IDC_EXPORT:
                  {
                      char str[MAX_PATH] = "*.clist";
                      OPENFILENAMEA ofn = {0};
                      ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
                      ofn.hwndOwner = hwnd;
                      ofn.hInstance = NULL;
                      ofn.lpstrFilter = "*.clist";
                      ofn.lpstrFile = str;
                      ofn.Flags = OFN_HIDEREADONLY;
                      ofn.nMaxFile = sizeof(str);
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
                      OPENFILENAMEA ofn = {0};

                      ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
                      ofn.hwndOwner = hwnd;
                      ofn.hInstance = NULL;
                      ofn.lpstrFilter = "*.clist";
                      ofn.lpstrFile = str;
                      ofn.Flags = OFN_FILEMUSTEXIST;
                      ofn.nMaxFile = sizeof(str);
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
         switch(((LPNMHDR)lParam)->idFrom) {
            case 0:
               switch (((LPNMHDR)lParam)->code)
               {
                  case PSN_APPLY:
                     {
                        TCITEM tci;
                        int i,count;
                        tci.mask = TCIF_PARAM;
                        count = TabCtrl_GetItemCount(GetDlgItem(hwnd,IDC_OPTIONSTAB));
                        for (i=0;i<count;i++)
                        {
                           TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),i,&tci);
                           SendMessage((HWND)tci.lParam,WM_NOTIFY,0,lParam);
                        }
                     }
                  break;
               }
            break;
            case IDC_OPTIONSTAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);
                        cfg::writeByte("CLUI", "opage", (BYTE)TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)));
                        Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
                        Utils::enableDlgControl(hwnd, IDC_IMPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
                     }
                  break;
               }
            break;

         }
      break;
      case WM_DESTROY:
          hwndSkinEdit = 0;
          break;
   }
   return FALSE;
}

int CoolSB_SetupScrollBar()
{
    /*
     * a skinned scrollbar is only valid when ALL items are skinned with image items
     * and no item is set to ignored
     */

    cfg::dat.bSkinnedScrollbar = !StatusItems[ID_EXTBKSCROLLBACK - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLBACKLOWER - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLTHUMB - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLTHUMBHOVER - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLTHUMBPRESSED - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLBUTTON - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLBUTTONHOVER - ID_STATUS_OFFLINE].IGNORED &&
        !StatusItems[ID_EXTBKSCROLLBUTTONPRESSED - ID_STATUS_OFFLINE].IGNORED;


    if(!StatusItems[ID_EXTBKSCROLLBACK - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLBACKLOWER - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLTHUMB - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLTHUMBHOVER - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLTHUMBPRESSED - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLBUTTON - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLBUTTONHOVER - ID_STATUS_OFFLINE].imageItem ||
        !StatusItems[ID_EXTBKSCROLLBUTTONPRESSED - ID_STATUS_OFFLINE].imageItem)

        cfg::dat.bSkinnedScrollbar = FALSE;

    if(cfg::getByte("CLC", "NoVScrollBar", 0)) {
        UninitializeCoolSB(pcli->hwndContactTree);
        return 0;
    }
    if(cfg::dat.bSkinnedScrollbar) {
        InitializeCoolSB(pcli->hwndContactTree);
        CoolSB_SetStyle(pcli->hwndContactTree, SB_VERT, CSBS_HOTTRACKED);
    }
    else
        UninitializeCoolSB(pcli->hwndContactTree);
    return 0;
}
