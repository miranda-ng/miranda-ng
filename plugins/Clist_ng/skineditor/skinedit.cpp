/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 *
 * This code was originally written by pixel for the clist_nicer plugin
 * for Miranda many years ago. Adopted to clist_nicer+ and now
 * clist_ng.
 *
 * $Id: skinedit.cpp 128 2010-09-26 12:12:56Z silvercircle $
 *
 */
#include <commonheaders.h>

#define ID_EXTBKSEPARATOR           40200

TStatusItem *StatusItems;
ChangedSItems_t ChangedSItems = {0};

static int LastModifiedItem = -1;
static int last_selcount = 0;
static int last_indizes[64];
static int ID_EXTBK_LAST = 0, ID_EXTBK_FIRST = 0;

/*                                                              
 * prototypes                                                                
 */

static void ChangeControlItems(HWND hwndDlg, int status, int except);
static BOOL CheckItem(int item, HWND hwndDlg);

static UINT _controls_to_refresh[] = {
     IDC_MRGN_BOTTOM,
     IDC_MRGN_LEFT,
     IDC_ALPHASPIN,
     IDC_CORNER,
     IDC_MRGN_TOP_SPIN,
     IDC_MRGN_RIGHT_SPIN,
     IDC_MRGN_BOTTOM_SPIN,
     IDC_MRGN_LEFT_SPIN,
     IDC_GRADIENT,
     IDC_GRADIENT_LR,
     IDC_GRADIENT_RL,
     IDC_GRADIENT_TB,
     IDC_BASECOLOUR,
     IDC_ALPHA,
     IDC_MRGN_TOP,
     IDC_MRGN_RIGHT,
     IDC_GRADIENT_BT,
     IDC_BASECOLOUR2,
     IDC_TEXTCOLOUR,
     IDC_CORNER_TL,
     IDC_CORNER_TR,
     IDC_CORNER_BR,
     IDC_CORNER_BL,
     IDC_IGNORE,
     IDC_ALPHALABLE,
     IDC_ALPHALABLE2,
     IDC_COLOR2LABLE,
     IDC_COLORLABLE,
     IDC_TEXTCOLOURLABLE,
     IDC_ALPHA2,
     IDC_ALPHASPIN2,
     IDC_SKIP_UNDERLAY,
     IDC_SKIP_IMAGE,
     0
};

static UINT _controls_to_hide[] = {
     IDC_ALPHASPIN,
     IDC_CORNER,
     IDC_GRADIENT,
     IDC_GRADIENT_LR,
     IDC_GRADIENT_RL,
     IDC_GRADIENT_TB,
     IDC_BASECOLOUR,
     IDC_ALPHA,
     IDC_GRADIENT_BT,
     IDC_BASECOLOUR2,
     IDC_TEXTCOLOUR,
     IDC_CORNER_TL,
     IDC_CORNER_TR,
     IDC_CORNER_BR,
     IDC_CORNER_BL,
     IDC_ALPHALABLE,
     IDC_ALPHALABLE2,
     IDC_COLOR2LABLE,
     IDC_COLORLABLE,
     IDC_TEXTCOLOURLABLE,
     IDC_ALPHA2,
     IDC_ALPHASPIN2,
     IDC_SKIP_UNDERLAY,
     IDC_SKIP_IMAGE,
     0
};

static void RefreshControls(HWND hwnd)
{
    for(int i = 0; _controls_to_refresh[i]; i++)
        InvalidateRect(GetDlgItem(hwnd, _controls_to_refresh[i]), NULL, FALSE);
}

static void ReActiveCombo(HWND hwndDlg)
{
    if (IsDlgButtonChecked(hwndDlg, IDC_IGNORE)) {
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_LR, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_RL, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_TB, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_BT, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

        Utils::enableDlgControl(hwndDlg, IDC_CORNER_TL, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_TR, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_BR, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_BL, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        ChangeControlItems(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
    } else {
        ChangeControlItems(hwndDlg, !IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_LR, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_RL, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_TB, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
        Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_BT, IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

        Utils::enableDlgControl(hwndDlg, IDC_CORNER_TL, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_TR, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_BR, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
        Utils::enableDlgControl(hwndDlg, IDC_CORNER_BL, IsDlgButtonChecked(hwndDlg, IDC_CORNER));
    }
}

// enabled or disabled the whole status controlitems group (with exceptional control)
static void ChangeControlItems(HWND hwndDlg, int status, int except)
{
    if (except != IDC_GRADIENT)
    	Utils::enableDlgControl(hwndDlg, IDC_GRADIENT, status);
    if (except != IDC_GRADIENT_LR)
    	Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_LR, status);
    if (except != IDC_GRADIENT_RL)
    	Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_RL, status);
    if (except != IDC_GRADIENT_TB)
    	Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_TB, status);
    if (except != IDC_GRADIENT_BT)
    	Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_BT, status);
    if (except != IDC_CORNER)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER, status);
    if (except != IDC_CORNER_TL)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER_TL, status);
    if (except != IDC_CORNER_TR)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER_TR, status);
    if (except != IDC_CORNER_BR)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER_BR, status);
    if (except != IDC_CORNER_BL)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER_BL, status);
    if (except != IDC_CORNER_TL)
    	Utils::enableDlgControl(hwndDlg, IDC_CORNER_TL, status);
    if (except != IDC_MARGINLABLE)
    	Utils::enableDlgControl(hwndDlg, IDC_MARGINLABLE, status);
    if (except != IDC_MRGN_TOP)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_TOP, status);
    if (except != IDC_MRGN_RIGHT)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_RIGHT, status);
    if (except != IDC_MRGN_BOTTOM)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_BOTTOM, status);
    if (except != IDC_MRGN_LEFT)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_LEFT, status);
    if (except != IDC_MRGN_TOP_SPIN)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_TOP_SPIN, status);
    if (except != IDC_MRGN_RIGHT_SPIN)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_RIGHT_SPIN, status);
    if (except != IDC_MRGN_BOTTOM_SPIN)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_BOTTOM_SPIN, status);
    if (except != IDC_MRGN_LEFT_SPIN)
    	Utils::enableDlgControl(hwndDlg, IDC_MRGN_LEFT_SPIN, status);
    if (except != IDC_BASECOLOUR)
    	Utils::enableDlgControl(hwndDlg, IDC_BASECOLOUR, status);
    if (except != IDC_COLORLABLE)
    	Utils::enableDlgControl(hwndDlg, IDC_COLORLABLE, status);
    if (except != IDC_BASECOLOUR2)
    	Utils::enableDlgControl(hwndDlg, IDC_BASECOLOUR2, status);
    if (except != IDC_COLOR2LABLE)
    	Utils::enableDlgControl(hwndDlg, IDC_COLOR2LABLE, status);
    if (except != IDC_TEXTCOLOUR)
    	Utils::enableDlgControl(hwndDlg, IDC_TEXTCOLOUR, status);
    if (except != IDC_TEXTCOLOURLABLE)
    	Utils::enableDlgControl(hwndDlg, IDC_TEXTCOLOURLABLE, status);

    if (except != IDC_ALPHA)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHA, status);
    if (except != IDC_ALPHASPIN)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHASPIN, status);

    if (except != IDC_ALPHA2)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHA2, status);
    if (except != IDC_ALPHASPIN2)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHASPIN2, status);

    if (except != IDC_ALPHALABLE)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHALABLE, status);
    if (except != IDC_ALPHALABLE2)
    	Utils::enableDlgControl(hwndDlg, IDC_ALPHALABLE2, status);
    if (except != IDC_IGNORE)
    	Utils::enableDlgControl(hwndDlg, IDC_IGNORE, status);

    if (except != IDC_SKIP_UNDERLAY)
    	Utils::enableDlgControl(hwndDlg, IDC_SKIP_UNDERLAY, status);
    if (except != IDC_SKIP_IMAGE)
    	Utils::enableDlgControl(hwndDlg, IDC_SKIP_IMAGE, status);
    
}

static void FillOptionDialogByStatusItem(HWND hwndDlg, TStatusItem *item)
{
    char 	itoabuf[15];
    DWORD 	ret, cmdShow;
    int		i;

    if(item->dwFlags & S_ITEM_IMAGE_ONLY)
    	cmdShow = SW_HIDE;
    else
    	cmdShow = SW_SHOW;

    for(i = 0; _controls_to_hide[i]; i++)
    	Utils::showDlgControl(hwndDlg, _controls_to_hide[i], cmdShow);
    
    if(item->dwFlags & S_ITEM_IMAGE_ONLY)
    	return;

    CheckDlgButton(hwndDlg, IDC_IGNORE, (item->IGNORED) ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hwndDlg, IDC_GRADIENT, (item->GRADIENT & GRADIENT_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
    Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_LR, item->GRADIENT & GRADIENT_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_RL, item->GRADIENT & GRADIENT_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_TB, item->GRADIENT & GRADIENT_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_GRADIENT_BT, item->GRADIENT & GRADIENT_ACTIVE);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_LR, (item->GRADIENT & GRADIENT_LR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_RL, (item->GRADIENT & GRADIENT_RL) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_TB, (item->GRADIENT & GRADIENT_TB) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_GRADIENT_BT, (item->GRADIENT & GRADIENT_BT) ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hwndDlg, IDC_CORNER, (item->CORNER & CORNER_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
    Utils::enableDlgControl(hwndDlg, IDC_CORNER_TL, item->CORNER & CORNER_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_CORNER_TR, item->CORNER & CORNER_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_CORNER_BR, item->CORNER & CORNER_ACTIVE);
    Utils::enableDlgControl(hwndDlg, IDC_CORNER_BL, item->CORNER & CORNER_ACTIVE);

    CheckDlgButton(hwndDlg, IDC_CORNER_TL, (item->CORNER & CORNER_TL) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_TR, (item->CORNER & CORNER_TR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_BR, (item->CORNER & CORNER_BR) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_CORNER_BL, (item->CORNER & CORNER_BL) ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hwndDlg, IDC_SKIP_UNDERLAY, (item->dwFlags & S_ITEM_SKIP_UNDERLAY) ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwndDlg, IDC_SKIP_IMAGE, (item->dwFlags & S_ITEM_SKIP_IMAGE) ? BST_CHECKED : BST_UNCHECKED);

    ret = item->COLOR;
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR);
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETCOLOUR, 0, ret);

    ret = item->COLOR2;
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR2);
    SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETCOLOUR, 0, ret);

    ret = item->TEXTCOLOR;
    SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_TEXTCOLOR);
    SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_SETCOLOUR, 0, ret);

    if (item->ALPHA == -1) {
        SetDlgItemTextA(hwndDlg, IDC_ALPHA, "");
    } else {
        ret = item->ALPHA;
        _itoa(ret, itoabuf, 10);    
        SetDlgItemTextA(hwndDlg, IDC_ALPHA, itoabuf);
    }

    if (item->ALPHA2 == -1) {
        SetDlgItemTextA(hwndDlg, IDC_ALPHA2, "");
    } else {
        ret = item->ALPHA2;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_ALPHA2, itoabuf);
    }

    if (item->MARGIN_LEFT == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_LEFT, "");
    else {
        ret = item->MARGIN_LEFT;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_LEFT, itoabuf);
    }

    if (item->MARGIN_TOP == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_TOP, "");
    else {
        ret = item->MARGIN_TOP;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_TOP, itoabuf);
    }

    if (item->MARGIN_RIGHT == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_RIGHT, "");
    else {
        ret = item->MARGIN_RIGHT;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_RIGHT, itoabuf);
    }

    if (item->MARGIN_BOTTOM == -1)
        SetDlgItemTextA(hwndDlg, IDC_MRGN_BOTTOM, "");
    else {
        ret = item->MARGIN_BOTTOM;
        _itoa(ret, itoabuf, 10);
        SetDlgItemTextA(hwndDlg, IDC_MRGN_BOTTOM, itoabuf);
    }
    ReActiveCombo(hwndDlg);
}
// update dlg with selected item
static void FillOptionDialogByCurrentSel(HWND hwndDlg)
{
    int index = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETCURSEL, 0, 0);
    int itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, index, 0);
    if(itemData != ID_EXTBKSEPARATOR) {
        LastModifiedItem = itemData - ID_EXTBK_FIRST;

        if (CheckItem(itemData - ID_EXTBK_FIRST, hwndDlg)) {
            FillOptionDialogByStatusItem(hwndDlg, &StatusItems[itemData - ID_EXTBK_FIRST]);
        }
    }
}


// enabled all status controls if the selected item is a separator
static BOOL CheckItem(int item, HWND hwndDlg)
{
    if (StatusItems[item].statusID == ID_EXTBKSEPARATOR) {
        ChangeControlItems(hwndDlg, 0, 0);
        return FALSE;
    } else {
        ChangeControlItems(hwndDlg, 1, 0);
        return TRUE;
    }
}

static void SetChangedStatusItemFlag(WPARAM wParam, HWND hwndDlg)
{
    if (LOWORD(wParam) != IDC_ITEMS
       && (GetDlgItem(hwndDlg, LOWORD(wParam)) == GetFocus() || HIWORD(wParam) == CPN_COLOURCHANGED)
       && (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == CPN_COLOURCHANGED)) {
        switch (LOWORD(wParam)) {
            case IDC_IGNORE:
                ChangedSItems.bIGNORED = TRUE; break;
            case IDC_GRADIENT:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_LR:
                ChangedSItems.bGRADIENT = TRUE;break;
            case IDC_GRADIENT_RL:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_BT:
                ChangedSItems.bGRADIENT = TRUE; break;
            case IDC_GRADIENT_TB:
                ChangedSItems.bGRADIENT = TRUE; break;

            case IDC_CORNER:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_TL:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_TR:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_BR:
                ChangedSItems.bCORNER = TRUE; break;
            case IDC_CORNER_BL:
                ChangedSItems.bCORNER = TRUE; break;

            case IDC_BASECOLOUR:
                ChangedSItems.bCOLOR = TRUE; break;     
            case IDC_BASECOLOUR2:
                ChangedSItems.bCOLOR2 = TRUE; break;
            case IDC_TEXTCOLOUR:
                ChangedSItems.bTEXTCOLOR = TRUE; break;

            case IDC_ALPHA:
                ChangedSItems.bALPHA = TRUE; break;
            case IDC_ALPHASPIN:
                ChangedSItems.bALPHA = TRUE; break;
            case IDC_ALPHA2:
                ChangedSItems.bALPHA2 = TRUE; break;
            case IDC_ALPHASPIN2:
                ChangedSItems.bALPHA2 = TRUE; break;

            case IDC_MRGN_LEFT:
                ChangedSItems.bMARGIN_LEFT = TRUE; break;
            case IDC_MRGN_LEFT_SPIN:
                ChangedSItems.bMARGIN_LEFT = TRUE; break;

            case IDC_MRGN_TOP:
                ChangedSItems.bMARGIN_TOP = TRUE; break;
            case IDC_MRGN_TOP_SPIN:
                ChangedSItems.bMARGIN_TOP = TRUE; break;

            case IDC_MRGN_RIGHT:
                ChangedSItems.bMARGIN_RIGHT = TRUE; break;
            case IDC_MRGN_RIGHT_SPIN:
                ChangedSItems.bMARGIN_RIGHT = TRUE; break;

            case IDC_MRGN_BOTTOM:
                ChangedSItems.bMARGIN_BOTTOM = TRUE; break;
            case IDC_MRGN_BOTTOM_SPIN:
                ChangedSItems.bMARGIN_BOTTOM = TRUE; break;

            case IDC_SKIP_UNDERLAY:
            case IDC_SKIP_IMAGE:
            	ChangedSItems.bFlags = TRUE;
            	break;
        }
    }
}

static BOOL isValidItem(void)
{
    if (StatusItems[LastModifiedItem].statusID == ID_EXTBKSEPARATOR)
        return FALSE;

    return TRUE;
}

// updates the struct with the changed dlg item
static void UpdateStatusStructSettingsFromOptDlg(HWND hwndDlg, int index)
{
    char buf[15];
    
    if (ChangedSItems.bIGNORED)
        StatusItems[index]. IGNORED = IsDlgButtonChecked(hwndDlg, IDC_IGNORE);

    if (ChangedSItems.bGRADIENT) {
        StatusItems[index]. GRADIENT = GRADIENT_NONE;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT))
            StatusItems[index].GRADIENT |= GRADIENT_ACTIVE;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_LR))
            StatusItems[index].GRADIENT |= GRADIENT_LR;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_RL))
            StatusItems[index].GRADIENT |= GRADIENT_RL;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_TB))
            StatusItems[index].GRADIENT |= GRADIENT_TB;
        if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_BT))
            StatusItems[index].GRADIENT |= GRADIENT_BT;
    }
    if (ChangedSItems.bCORNER) {
        StatusItems[index]. CORNER = CORNER_NONE;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER))
            StatusItems[index].CORNER |= CORNER_ACTIVE ;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TL))
            StatusItems[index].CORNER |= CORNER_TL ;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TR))
            StatusItems[index].CORNER |= CORNER_TR;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BR))
            StatusItems[index].CORNER |= CORNER_BR;
        if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BL))
            StatusItems[index].CORNER |= CORNER_BL;
    }

    if (ChangedSItems.bCOLOR)
        StatusItems[index]. COLOR = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bCOLOR2)
        StatusItems[index]. COLOR2 = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bALPHA2) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_ALPHA2), buf, 10);        // can be removed now
        if (lstrlenA(buf) > 0)
            StatusItems[index].ALPHA2 = (BYTE) SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN2, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bTEXTCOLOR)
        StatusItems[index]. TEXTCOLOR = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0);

    if (ChangedSItems.bALPHA) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_ALPHA), buf, 10);        // can be removed now
        if (lstrlenA(buf) > 0)
            StatusItems[index]. ALPHA = (BYTE) SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_LEFT) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_LEFT), buf, 10);        
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_LEFT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_TOP) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_TOP), buf, 10);     
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_TOP = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_RIGHT) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT), buf, 10);       
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_RIGHT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bMARGIN_BOTTOM) {
        GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM), buf, 10);      
        if (lstrlenA(buf) > 0)
            StatusItems[index]. MARGIN_BOTTOM = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_GETPOS, 0, 0);
    }

    if (ChangedSItems.bFlags) {
    	StatusItems[index].dwFlags = (IsDlgButtonChecked(hwndDlg, IDC_SKIP_UNDERLAY) ? StatusItems[index].dwFlags | S_ITEM_SKIP_UNDERLAY :
    		StatusItems[index].dwFlags & ~S_ITEM_SKIP_UNDERLAY);
    	StatusItems[index].dwFlags = (IsDlgButtonChecked(hwndDlg, IDC_SKIP_IMAGE) ? StatusItems[index].dwFlags | S_ITEM_SKIP_IMAGE :
    		StatusItems[index].dwFlags & ~S_ITEM_SKIP_IMAGE);
    }

    Skin::setupAGGItemContext(&StatusItems[index]);
}

static void SaveLatestChanges(HWND hwndDlg)
{
    int n, itemData;
    // process old selection
    if (last_selcount > 0) {
        for (n = 0; n < last_selcount; n++) {
            itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[n], 0);
            if (itemData != ID_EXTBKSEPARATOR) {
                UpdateStatusStructSettingsFromOptDlg(hwndDlg, itemData - ID_EXTBK_FIRST);
            }
        }
    }

    // reset bChange
    ChangedSItems.bALPHA = FALSE;
    ChangedSItems.bGRADIENT = FALSE;
    ChangedSItems.bCORNER = FALSE;
    ChangedSItems.bCOLOR = FALSE;
    ChangedSItems.bCOLOR2 = FALSE;
    ChangedSItems.bALPHA2 = FALSE;
    ChangedSItems.bTEXTCOLOR = FALSE;
    ChangedSItems.bALPHA = FALSE;
    ChangedSItems.bMARGIN_LEFT = FALSE;
    ChangedSItems.bMARGIN_TOP = FALSE;
    ChangedSItems.bMARGIN_RIGHT = FALSE;
    ChangedSItems.bMARGIN_BOTTOM = FALSE;
    ChangedSItems.bIGNORED = FALSE;
    ChangedSItems.bFlags = FALSE;
}

static void OnListItemsChange(HWND hwndDlg)
{
    SendMessage(hwndDlg, WM_SETREDRAW, FALSE, 0);
    SaveLatestChanges(hwndDlg);

    // set new selection
    last_selcount = SendMessage(GetDlgItem(hwndDlg, IDC_ITEMS), LB_GETSELCOUNT, 0, 0);  
    if (last_selcount > 0) {
        int n, real_index, itemData, first_item;
        TStatusItem DialogSettingForMultiSel;

    // get selected indizes
        SendMessage(GetDlgItem(hwndDlg, IDC_ITEMS), LB_GETSELITEMS, 64, (LPARAM) last_indizes);

    // initialize with first items value

        first_item = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[0], 0) - ID_EXTBK_FIRST;
        DialogSettingForMultiSel = StatusItems[first_item];
        for (n = 0; n < last_selcount; n++) {
            itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[n], 0);
            if (itemData != ID_EXTBKSEPARATOR) {
                real_index = itemData - ID_EXTBK_FIRST;
                if (StatusItems[real_index].ALPHA != StatusItems[first_item].ALPHA)
                    DialogSettingForMultiSel.ALPHA = -1;
                if (StatusItems[real_index].COLOR != StatusItems[first_item].COLOR)
                    DialogSettingForMultiSel.COLOR = CLCDEFAULT_COLOR;
                if (StatusItems[real_index].COLOR2 != StatusItems[first_item].COLOR2)
                    DialogSettingForMultiSel.COLOR2 = CLCDEFAULT_COLOR2;
                if (StatusItems[real_index].ALPHA2 != StatusItems[first_item].ALPHA2)
                    DialogSettingForMultiSel.ALPHA2 = -1;
                if (StatusItems[real_index].TEXTCOLOR != StatusItems[first_item].TEXTCOLOR)
                    DialogSettingForMultiSel.TEXTCOLOR = CLCDEFAULT_TEXTCOLOR;
                if (StatusItems[real_index].CORNER != StatusItems[first_item].CORNER)
                    DialogSettingForMultiSel.CORNER = CLCDEFAULT_CORNER;
                if (StatusItems[real_index].GRADIENT != StatusItems[first_item].GRADIENT)
                    DialogSettingForMultiSel.GRADIENT = CLCDEFAULT_GRADIENT;
                if (StatusItems[real_index].IGNORED != StatusItems[first_item].IGNORED)
                    DialogSettingForMultiSel.IGNORED = CLCDEFAULT_IGNORE;
                if (StatusItems[real_index].MARGIN_BOTTOM != StatusItems[first_item].MARGIN_BOTTOM)
                    DialogSettingForMultiSel.MARGIN_BOTTOM = -1;
                if (StatusItems[real_index].MARGIN_LEFT != StatusItems[first_item].MARGIN_LEFT)
                    DialogSettingForMultiSel.MARGIN_LEFT = -1;
                if (StatusItems[real_index].MARGIN_RIGHT != StatusItems[first_item].MARGIN_RIGHT)
                    DialogSettingForMultiSel.MARGIN_RIGHT = -1;
                if (StatusItems[real_index].MARGIN_TOP != StatusItems[first_item].MARGIN_TOP)
                    DialogSettingForMultiSel.MARGIN_TOP = -1;
                if (StatusItems[real_index].dwFlags != StatusItems[first_item].dwFlags)
                    DialogSettingForMultiSel.dwFlags = -1;
            }
        }

        if (last_selcount == 1 && StatusItems[first_item].statusID == ID_EXTBKSEPARATOR) {
            ChangeControlItems(hwndDlg, 0, 0);
            last_selcount = 0;
        } else {

            ChangeControlItems(hwndDlg, 1, 0);
        }
        FillOptionDialogByStatusItem(hwndDlg, &DialogSettingForMultiSel);
        InvalidateRect(GetDlgItem(hwndDlg, IDC_ITEMS), NULL, FALSE);
    }
    SendMessage(hwndDlg, WM_SETREDRAW, TRUE, 0);
    RefreshControls(hwndDlg);
}

// fills the combobox of the options dlg for the first time
static void FillItemList(HWND hwndDlg)
{
	int n, iOff;
	UINT item;

	for (n = 0; n <= ID_EXTBK_LAST - ID_EXTBK_FIRST; n++) {
		iOff = 0;
		if(strstr(StatusItems[n].szName, "{-}")) {
			item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)"------------------------");
			SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBKSEPARATOR);
			iOff = 3;
		}
		item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)&StatusItems[n].szName[iOff]);
		SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBK_FIRST + n);
	}
}

static BOOL CALLBACK SkinEdit_ExtBkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SKINDESCRIPTION *psd = (SKINDESCRIPTION *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    if(psd) {
        ID_EXTBK_FIRST = psd->firstItem;
        ID_EXTBK_LAST = psd->lastItem;
        StatusItems = psd->StatusItems;
    }
    switch (msg) {
        case WM_INITDIALOG:
            psd = (SKINDESCRIPTION *)malloc(sizeof(SKINDESCRIPTION));
            ZeroMemory(psd, sizeof(SKINDESCRIPTION));
            CopyMemory(psd, (void *)lParam, sizeof(SKINDESCRIPTION));
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)psd);

            if(psd) {
                ID_EXTBK_FIRST = psd->firstItem;
                ID_EXTBK_LAST = psd->lastItem;
                StatusItems = psd->StatusItems;
            }

            TranslateDialogDefault(hwndDlg);
            FillItemList(hwndDlg);
            SendMessage(hwndDlg, WM_USER + 101, 0, 0);

            psd->hMenuItems = CreatePopupMenu();
            AppendMenu(psd->hMenuItems, MF_STRING | MF_DISABLED, (UINT_PTR)0, _T("Copy from"));
            AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);

            {
				int i;
				
				for(i = ID_EXTBK_FIRST; i <= ID_EXTBK_LAST; i++) {
					int iOff = StatusItems[i - ID_EXTBK_FIRST].szName[0] == '{' ? 3 : 0;
					if(iOff)
						AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);
					AppendMenuA(psd->hMenuItems, MF_STRING, (UINT_PTR)i, &StatusItems[i - ID_EXTBK_FIRST].szName[iOff]);
				}
			}
            return TRUE;
        case WM_USER + 101:
            {
                SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
                SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN2, UDM_SETRANGE, 0, MAKELONG(100, 0));

                return 0;
            }

        case WM_DRAWITEM:
            {
                DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
                int iItem = dis->itemData;
                TStatusItem *item = 0;

                SetBkMode(dis->hDC, TRANSPARENT);
                FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));

                if(iItem >= ID_EXTBK_FIRST && iItem <= ID_EXTBK_LAST)
                    item = &StatusItems[iItem - ID_EXTBK_FIRST];

                if (dis->itemState & ODS_SELECTED && iItem != ID_EXTBKSEPARATOR) {
                    FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
                    SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                }
                else {
                    FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
                    if(item && item->IGNORED)
                        SetTextColor(dis->hDC, RGB(255, 0, 0));
                    else if(item && item->dwFlags & S_ITEM_IMAGE_ONLY)
                    	SetTextColor(dis->hDC, RGB(0, 0, 255));
                    else
                        SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
                }
                if(iItem == ID_EXTBKSEPARATOR) {
                    HPEN    hPen, hPenOld;
                    POINT   pt;

                    hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_WINDOWTEXT));
                    hPenOld = (HPEN)SelectObject(dis->hDC, hPen);

                    MoveToEx(dis->hDC, dis->rcItem.left, (dis->rcItem.top + dis->rcItem.bottom) / 2, &pt);
                    LineTo(dis->hDC, dis->rcItem.right, (dis->rcItem.top + dis->rcItem.bottom) / 2);
                    SelectObject(dis->hDC, hPenOld);
                    DeleteObject((HGDIOBJ)hPen);
                }
                else if(dis->itemID >= 0 && item) {
                    char   *szName = item->szName[0] == '{' ? &item->szName[3] : item->szName;

                    TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, szName, lstrlenA(szName));
                }
                return TRUE;
            }

        case WM_CONTEXTMENU:
            {
                POINT pt;
                RECT  rc;
                HWND hwndList = GetDlgItem(hwndDlg, IDC_ITEMS);

                GetCursorPos(&pt);
                GetWindowRect(hwndList, &rc);
                if(PtInRect(&rc, pt)) {
                    int iSelection = (int)TrackPopupMenu(psd->hMenuItems, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

                    if(iSelection >= ID_EXTBK_FIRST && iSelection <= ID_EXTBK_LAST) {
                        iSelection -= ID_EXTBK_FIRST;

                        for(int i = ID_EXTBK_FIRST; i <= ID_EXTBK_LAST; i++) {
                            if(SendMessage(hwndList, LB_GETSEL, i - ID_EXTBK_FIRST, 0) > 0) {
                                int iIndex = SendMessage(hwndList, LB_GETITEMDATA, i - ID_EXTBK_FIRST, 0);
                                iIndex -= ID_EXTBK_FIRST;

                                if(iIndex >= 0) {
                                    StatusItems[iIndex].ALPHA = StatusItems[iSelection].ALPHA;
                                    StatusItems[iIndex].COLOR = StatusItems[iSelection].COLOR;
                                    StatusItems[iIndex].COLOR2 = StatusItems[iSelection].COLOR2;
                                    StatusItems[iIndex].ALPHA2 = StatusItems[iSelection].ALPHA2;
                                    StatusItems[iIndex].CORNER = StatusItems[iSelection].CORNER;
                                    StatusItems[iIndex].GRADIENT = StatusItems[iSelection].GRADIENT;
                                    StatusItems[iIndex].IGNORED = StatusItems[iSelection].IGNORED;
                                    StatusItems[iIndex].imageItem = StatusItems[iSelection].imageItem;
                                    StatusItems[iIndex].MARGIN_BOTTOM = StatusItems[iSelection].MARGIN_BOTTOM;
                                    StatusItems[iIndex].MARGIN_LEFT = StatusItems[iSelection].MARGIN_LEFT;
                                    StatusItems[iIndex].MARGIN_RIGHT = StatusItems[iSelection].MARGIN_RIGHT;
                                    StatusItems[iIndex].MARGIN_TOP = StatusItems[iSelection].MARGIN_TOP;
                                    StatusItems[iIndex].TEXTCOLOR = StatusItems[iSelection].TEXTCOLOR;
                                    StatusItems[iIndex].dwFlags = StatusItems[iSelection].dwFlags;
                                }
                            }
                        }
                        OnListItemsChange(hwndDlg);
                    }
                }
                break;
            }
        case WM_COMMAND:
    // this will check if the user changed some actual statusitems values
    // if yes the flag bChanged will be set to TRUE
            SetChangedStatusItemFlag(wParam, hwndDlg);
            switch(LOWORD(wParam)) {
                case IDC_ITEMS:
                    if (HIWORD(wParam) != LBN_SELCHANGE)
                        return FALSE;
                    {
                        int iItem = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETCURSEL, 0, 0), 0);
                        if(iItem == ID_EXTBKSEPARATOR)
                            return FALSE;
                    }
                    OnListItemsChange(hwndDlg);
					if(psd->pfnClcOptionsChanged)
						psd->pfnClcOptionsChanged();
                    break;
                case IDC_SKIP_UNDERLAY:
                case IDC_SKIP_IMAGE:
                case IDC_GRADIENT:
                case IDC_CORNER:
                case IDC_IGNORE:
                    ReActiveCombo(hwndDlg);
                    break;
            }
            if ((LOWORD(wParam) == IDC_ALPHA || LOWORD(wParam) == IDC_ALPHA2 || LOWORD(wParam) == IDC_MRGN_LEFT || LOWORD(wParam) == IDC_MRGN_BOTTOM || LOWORD(wParam) == IDC_MRGN_TOP || LOWORD(wParam) == IDC_MRGN_RIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
                return 0;
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY:
                // save user made changes
                            SaveLatestChanges(hwndDlg);
                // save struct to DB
							if(psd->pfnSaveCompleteStruct)
								psd->pfnSaveCompleteStruct();

                            if(psd->pfnClcOptionsChanged)
								psd->pfnClcOptionsChanged();
							if(psd->hwndCLUI) {
								SendMessage(psd->hwndCLUI, WM_SIZE, 0, 0);
								PostMessage(psd->hwndCLUI, WM_USER+100, 0, 0);          // CLUIINTM_REDRAW
							}
                            break;
                    }
            }
            break;
        case WM_DESTROY:
            DestroyMenu(psd->hMenuItems);
            break;
        case WM_NCDESTROY:
            free(psd);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);
            break;
    }
    return FALSE;
}

/*                                                              
 * unimplemented                                                                
*/

static BOOL CALLBACK SkinEdit_ImageItemEditProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

static INT_PTR SkinEdit_FillByCurrentSel(WPARAM wParam, LPARAM lParam)
{
    if(wParam)
        FillOptionDialogByCurrentSel((HWND)wParam);
    return 0;
}

/*                                                              
 * service function                                                                
 * creates additional tab pages under the given parent window handle
 * expects a SKINDESCRIPTON * in lParam
*/

HWND SkinEdit_Invoke(WPARAM wParam, LPARAM lParam)
{
    SKINDESCRIPTION *psd = (SKINDESCRIPTION *)lParam;
    TCITEM  tci = {0};
    RECT    rcClient;
    int     iTabs;

    if(psd->cbSize != sizeof(SKINDESCRIPTION))
        return 0;

    iTabs = TabCtrl_GetItemCount(psd->hWndTab);
    GetClientRect(psd->hWndParent, &rcClient);

    tci.mask = TCIF_PARAM|TCIF_TEXT;
    tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SKINITEMEDIT), psd->hWndParent, (DLGPROC)SkinEdit_ExtBkDlgProc, (LPARAM)psd);

    tci.pszText = TranslateT("Skin items");
    TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
    MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 65, 1);
    psd->hwndSkinEdit = (HWND)tci.lParam;

    tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_IMAGEITEMEDIT), psd->hWndParent, (DLGPROC)SkinEdit_ImageItemEditProc, (LPARAM)psd);
    tci.pszText = TranslateT("Image items");
    TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
    MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 65, 1);
    psd->hwndImageEdit = (HWND)tci.lParam;
    
    return(psd->hwndSkinEdit);
}
