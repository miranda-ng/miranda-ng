/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-04 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

#define ID_EXTBKSEPARATOR		 40200

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {21948C89-B549-4C9D-8B4F-3F3726EC6B4B}
	{0x21948c89, 0xb549, 0x4c9d, {0x8b, 0x4f, 0x3f, 0x37, 0x26, 0xec, 0x6b, 0x4b}}
};

HINSTANCE g_hInst = 0;
int hLangpack;

StatusItems_t **StatusItems;
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

static void ReActiveCombo(HWND hwndDlg)
{
	if (IsDlgButtonChecked(hwndDlg, IDC_IGNORE)) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

		EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));

		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		ChangeControlItems(hwndDlg, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
	} else {
		ChangeControlItems(hwndDlg, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_IGNORE), IDC_IGNORE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), IsDlgButtonChecked(hwndDlg, IDC_GRADIENT));

		EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT));

		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), IsDlgButtonChecked(hwndDlg, IDC_CORNER));
	}
}

// enabled or disabled the whole status controlitems group (with exceptional control)
static void ChangeControlItems(HWND hwndDlg, int status, int except)
{
	if (except != IDC_GRADIENT)
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT), status);
	if (except != IDC_GRADIENT_LR)
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), status);
	if (except != IDC_GRADIENT_RL)
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), status);
	if (except != IDC_GRADIENT_TB)
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), status);
	if (except != IDC_GRADIENT_BT)
		EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), status);
	if (except != IDC_CORNER)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER), status);
	if (except != IDC_CORNER_TL)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), status);
	if (except != IDC_CORNER_TR)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), status);
	if (except != IDC_CORNER_BR)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), status);
	if (except != IDC_CORNER_BL)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), status);
	if (except != IDC_CORNER_TL)
		EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), status);
	if (except != IDC_MARGINLABLE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MARGINLABLE), status);
	if (except != IDC_MRGN_TOP)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_TOP), status);
	if (except != IDC_MRGN_RIGHT)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT), status);
	if (except != IDC_MRGN_BOTTOM)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM), status);
	if (except != IDC_MRGN_LEFT)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_LEFT), status);
	if (except != IDC_MRGN_TOP_SPIN)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_TOP_SPIN), status);
	if (except != IDC_MRGN_RIGHT_SPIN)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT_SPIN), status);
	if (except != IDC_MRGN_BOTTOM_SPIN)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM_SPIN), status);
	if (except != IDC_MRGN_LEFT_SPIN)
		EnableWindow(GetDlgItem(hwndDlg, IDC_MRGN_LEFT_SPIN), status);
	if (except != IDC_BASECOLOUR)
		EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR), status);
	if (except != IDC_COLORLABLE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLORLABLE), status);
	if (except != IDC_BASECOLOUR2)
		EnableWindow(GetDlgItem(hwndDlg, IDC_BASECOLOUR2), status);
	if (except != IDC_COLOR2LABLE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2LABLE), status);
	if (except != IDC_COLOR2_TRANSPARENT)
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR2_TRANSPARENT), status);
	if (except != IDC_TEXTCOLOUR)
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOUR), status);
	if (except != IDC_TEXTCOLOURLABLE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOURLABLE), status);

	if (except != IDC_ALPHA)
		EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHA), status);
	if (except != IDC_ALPHASPIN)
		EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHASPIN), status);
	if (except != IDC_ALPHALABLE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_ALPHALABLE), status);
	if (except != IDC_IGNORE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORE), status);

	if (except != IDC_BORDERTYPE)
		EnableWindow(GetDlgItem(hwndDlg, IDC_BORDERTYPE), status);
}

static void FillOptionDialogByStatusItem(HWND hwndDlg, StatusItems_t *item)
{
	char itoabuf[15];
	DWORD ret;
	int index;

	CheckDlgButton(hwndDlg, IDC_IGNORE, (item->IGNORED) ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hwndDlg, IDC_GRADIENT, (item->GRADIENT & GRADIENT_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_LR), item->GRADIENT & GRADIENT_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_RL), item->GRADIENT & GRADIENT_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_TB), item->GRADIENT & GRADIENT_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRADIENT_BT), item->GRADIENT & GRADIENT_ACTIVE);
	CheckDlgButton(hwndDlg, IDC_GRADIENT_LR, (item->GRADIENT & GRADIENT_LR) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_GRADIENT_RL, (item->GRADIENT & GRADIENT_RL) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_GRADIENT_TB, (item->GRADIENT & GRADIENT_TB) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_GRADIENT_BT, (item->GRADIENT & GRADIENT_BT) ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hwndDlg, IDC_CORNER, (item->CORNER & CORNER_ACTIVE) ? BST_CHECKED : BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TL), item->CORNER & CORNER_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_TR), item->CORNER & CORNER_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BR), item->CORNER & CORNER_ACTIVE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CORNER_BL), item->CORNER & CORNER_ACTIVE);

	CheckDlgButton(hwndDlg, IDC_CORNER_TL, (item->CORNER & CORNER_TL) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CORNER_TR, (item->CORNER & CORNER_TR) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CORNER_BR, (item->CORNER & CORNER_BR) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CORNER_BL, (item->CORNER & CORNER_BL) ? BST_CHECKED : BST_UNCHECKED);

	ret = item->COLOR;
	SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR);
	SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_SETCOLOUR, 0, ret);

	ret = item->COLOR2;
	SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETDEFAULTCOLOUR, 0, CLCDEFAULT_COLOR2);
	SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_SETCOLOUR, 0, ret);

	CheckDlgButton(hwndDlg, IDC_COLOR2_TRANSPARENT, (item->COLOR2_TRANSPARENT) ? BST_CHECKED : BST_UNCHECKED);

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
	if (item->BORDERSTYLE == -1)
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_SETCURSEL, 0, 0);
	else {
		index = 0;
		switch(item->BORDERSTYLE) {
			case 0:
			case -1:
				index = 0;
				break;
			case BDR_RAISEDOUTER:
				index = 1;
				break;
			case BDR_SUNKENINNER:
				index = 2;
				break;
			case EDGE_BUMP:
				index = 3;
				break;
			case EDGE_ETCHED:
				index = 4;
				break;
		}
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_SETCURSEL, (WPARAM)index, 0);
	}
	ReActiveCombo(hwndDlg);
}
// update dlg with selected item
static void FillOptionDialogByCurrentSel(HWND hwndDlg)
{
	int index = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETCURSEL, 0, 0);
	int itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, index, 0);
	if (itemData != ID_EXTBKSEPARATOR) {
		LastModifiedItem = itemData - ID_EXTBK_FIRST;

		if (CheckItem(itemData - ID_EXTBK_FIRST, hwndDlg))
			FillOptionDialogByStatusItem(hwndDlg, StatusItems[itemData - ID_EXTBK_FIRST]);
	}
}


// enabled all status controls if the selected item is a separator
static BOOL CheckItem(int item, HWND hwndDlg)
{
	if (StatusItems[item]->statusID == ID_EXTBKSEPARATOR) {
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
			case IDC_COLOR2_TRANSPARENT:
				ChangedSItems.bCOLOR2_TRANSPARENT = TRUE; break;
			case IDC_TEXTCOLOUR:
				ChangedSItems.bTEXTCOLOR = TRUE; break;

			case IDC_ALPHA:
				ChangedSItems.bALPHA = TRUE; break;
			case IDC_ALPHASPIN:
				ChangedSItems.bALPHA = TRUE; break;

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

			case IDC_BORDERTYPE:
				ChangedSItems.bBORDERSTYLE = TRUE; break;
		}
	}
}

static BOOL isValidItem(void)
{
	if (StatusItems[LastModifiedItem]->statusID == ID_EXTBKSEPARATOR)
		return FALSE;

	return TRUE;
}

// updates the struct with the changed dlg item
static void UpdateStatusStructSettingsFromOptDlg(HWND hwndDlg, int index)
{
	char buf[15];
	ULONG bdrtype;
	StatusItems_t *p = StatusItems[index];

	if (ChangedSItems.bIGNORED)
		p->IGNORED = IsDlgButtonChecked(hwndDlg, IDC_IGNORE);

	if (ChangedSItems.bGRADIENT) {
		p->GRADIENT = GRADIENT_NONE;
		if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT))
			p->GRADIENT |= GRADIENT_ACTIVE;
		if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_LR))
			p->GRADIENT |= GRADIENT_LR;
		if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_RL))
			p->GRADIENT |= GRADIENT_RL;
		if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_TB))
			p->GRADIENT |= GRADIENT_TB;
		if (IsDlgButtonChecked(hwndDlg, IDC_GRADIENT_BT))
			p->GRADIENT |= GRADIENT_BT;
	}
	if (ChangedSItems.bCORNER) {
		p->CORNER = CORNER_NONE;
		if (IsDlgButtonChecked(hwndDlg, IDC_CORNER))
			p->CORNER |= CORNER_ACTIVE ;
		if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TL))
			p->CORNER |= CORNER_TL ;
		if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_TR))
			p->CORNER |= CORNER_TR;
		if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BR))
			p->CORNER |= CORNER_BR;
		if (IsDlgButtonChecked(hwndDlg, IDC_CORNER_BL))
			p->CORNER |= CORNER_BL;
	}

	if (ChangedSItems.bCOLOR)
		p->COLOR = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR, CPM_GETCOLOUR, 0, 0);

	if (ChangedSItems.bCOLOR2)
		p->COLOR2 = SendDlgItemMessage(hwndDlg, IDC_BASECOLOUR2, CPM_GETCOLOUR, 0, 0);

	if (ChangedSItems.bCOLOR2_TRANSPARENT)
		p->COLOR2_TRANSPARENT = IsDlgButtonChecked(hwndDlg, IDC_COLOR2_TRANSPARENT);

	if (ChangedSItems.bTEXTCOLOR)
		p->TEXTCOLOR = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0);

	if (ChangedSItems.bALPHA) {
		GetWindowTextA(GetDlgItem(hwndDlg, IDC_ALPHA), buf, 10);		// can be removed now
		if (buf[0] != 0)
			p->ALPHA = (BYTE) SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_GETPOS, 0, 0);
	}

	if (ChangedSItems.bMARGIN_LEFT) {
		GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_LEFT), buf, 10);
		if (buf[0] != 0)
			p->MARGIN_LEFT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_GETPOS, 0, 0);
	}

	if (ChangedSItems.bMARGIN_TOP) {
		GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_TOP), buf, 10);
		if (buf[0] != 0)
			p->MARGIN_TOP = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_GETPOS, 0, 0);
	}

	if (ChangedSItems.bMARGIN_RIGHT) {
		GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_RIGHT), buf, 10);
		if (buf[0] != 0)
			p->MARGIN_RIGHT = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_GETPOS, 0, 0);
	}

	if (ChangedSItems.bMARGIN_BOTTOM) {
		GetWindowTextA(GetDlgItem(hwndDlg, IDC_MRGN_BOTTOM), buf, 10);
		if (buf[0] != 0)
			p->MARGIN_BOTTOM = (BYTE) SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_GETPOS, 0, 0);
	}
	if (ChangedSItems.bBORDERSTYLE) {
		bdrtype = SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_GETCURSEL, 0, 0);
		if (bdrtype == CB_ERR)
			p->BORDERSTYLE = 0;
		else {
			switch(bdrtype) {
				case 0:
				p->BORDERSTYLE = 0;
				break;
				case 1:
				p->BORDERSTYLE = BDR_RAISEDOUTER;
				break;
				case 2:
				p->BORDERSTYLE = BDR_SUNKENINNER;
				break;
				case 3:
				p->BORDERSTYLE = EDGE_BUMP;
				break;
				case 4:
				p->BORDERSTYLE = EDGE_ETCHED;
				break;
				default:
				p->BORDERSTYLE = 0;
				break;
			}
		}
	}
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
	ChangedSItems.bCOLOR2_TRANSPARENT = FALSE;
	ChangedSItems.bTEXTCOLOR = FALSE;
	ChangedSItems.bALPHA = FALSE;
	ChangedSItems.bMARGIN_LEFT = FALSE;
	ChangedSItems.bMARGIN_TOP = FALSE;
	ChangedSItems.bMARGIN_RIGHT = FALSE;
	ChangedSItems.bMARGIN_BOTTOM = FALSE;
	ChangedSItems.bIGNORED = FALSE;
	ChangedSItems.bBORDERSTYLE = FALSE;
}

static UINT _controls_to_refresh[] = {
	IDC_BORDERTYPE,
	IDC_3DDARKCOLOR,
	IDC_3DLIGHTCOLOR,
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
	IDC_COLOR2LABLE,
	IDC_COLORLABLE,
	IDC_TEXTCOLOURLABLE,
	IDC_COLOR2_TRANSPARENT,
	0
};

static void RefreshControls(HWND hwnd)
{
	for(int i = 0; _controls_to_refresh[i]; i++)
		InvalidateRect(GetDlgItem(hwnd, _controls_to_refresh[i]), NULL, FALSE);
}

// wenn die listbox geändert wurde
static void OnListItemsChange(HWND hwndDlg)
{
	SendMessage(hwndDlg, WM_SETREDRAW, FALSE, 0);
	SaveLatestChanges(hwndDlg);

	// set new selection
	last_selcount = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETSELCOUNT, 0, 0);
	if (last_selcount > 0) {
		int n, itemData, first_item;

		// get selected indizes
		SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETSELITEMS, 64, (LPARAM) last_indizes);

		// initialize with first items value

		first_item = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[0], 0) - ID_EXTBK_FIRST;
		StatusItems_t *pFirst = StatusItems[first_item];
		StatusItems_t DialogSettingForMultiSel = *StatusItems[first_item];
		for (n = 0; n < last_selcount; n++) {
			itemData = SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_GETITEMDATA, last_indizes[n], 0);
			if (itemData == ID_EXTBKSEPARATOR)
				continue;

			StatusItems_t *p = StatusItems[itemData - ID_EXTBK_FIRST];
			if (p->ALPHA != pFirst->ALPHA)
				DialogSettingForMultiSel.ALPHA = -1;
			if (p->COLOR != pFirst->COLOR)
				DialogSettingForMultiSel.COLOR = CLCDEFAULT_COLOR;
			if (p->COLOR2 != pFirst->COLOR2)
				DialogSettingForMultiSel.COLOR2 = CLCDEFAULT_COLOR2;
			if (p->COLOR2_TRANSPARENT != pFirst->COLOR2_TRANSPARENT)
				DialogSettingForMultiSel.COLOR2_TRANSPARENT = CLCDEFAULT_COLOR2_TRANSPARENT;
			if (p->TEXTCOLOR != pFirst->TEXTCOLOR)
				DialogSettingForMultiSel.TEXTCOLOR = CLCDEFAULT_TEXTCOLOR;
			if (p->CORNER != pFirst->CORNER)
				DialogSettingForMultiSel.CORNER = CLCDEFAULT_CORNER;
			if (p->GRADIENT != pFirst->GRADIENT)
				DialogSettingForMultiSel.GRADIENT = CLCDEFAULT_GRADIENT;
			if (p->IGNORED != pFirst->IGNORED)
				DialogSettingForMultiSel.IGNORED = CLCDEFAULT_IGNORE;
			if (p->MARGIN_BOTTOM != pFirst->MARGIN_BOTTOM)
				DialogSettingForMultiSel.MARGIN_BOTTOM = -1;
			if (p->MARGIN_LEFT != pFirst->MARGIN_LEFT)
				DialogSettingForMultiSel.MARGIN_LEFT = -1;
			if (p->MARGIN_RIGHT != pFirst->MARGIN_RIGHT)
				DialogSettingForMultiSel.MARGIN_RIGHT = -1;
			if (p->MARGIN_TOP != pFirst->MARGIN_TOP)
				DialogSettingForMultiSel.MARGIN_TOP = -1;
			if (p->BORDERSTYLE != pFirst->BORDERSTYLE)
				DialogSettingForMultiSel.BORDERSTYLE = -1;
		}

		if (last_selcount == 1 && pFirst->statusID == ID_EXTBKSEPARATOR) {
			ChangeControlItems(hwndDlg, 0, 0);
			last_selcount = 0;
		} else
			ChangeControlItems(hwndDlg, 1, 0);
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

	for (n = 0; n < ID_EXTBK_LAST - ID_EXTBK_FIRST; n++) {
		iOff = 0;
		if (strstr(StatusItems[n]->szName, "{-}")) {
			item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)"------------------------");
			SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBKSEPARATOR);
			iOff = 3;
		}
		item = SendDlgItemMessageA(hwndDlg, IDC_ITEMS, LB_ADDSTRING, 0, (LPARAM)StatusItems[n]->szName[iOff]);
		SendDlgItemMessage(hwndDlg, IDC_ITEMS, LB_SETITEMDATA, item, ID_EXTBK_FIRST + n);
	}
}

static INT_PTR CALLBACK SkinEdit_ExtBkDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SKINDESCRIPTION *psd = (SKINDESCRIPTION *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (psd) {
		ID_EXTBK_FIRST = psd->firstItem;
		ID_EXTBK_LAST = psd->lastItem;
		StatusItems = psd->StatusItems;
	}

	switch (msg) {
	case WM_INITDIALOG:
		psd = (SKINDESCRIPTION *)malloc(sizeof(SKINDESCRIPTION));
		if (psd == NULL)
			return FALSE;
		memset(psd, 0, sizeof(SKINDESCRIPTION));
		memcpy(psd, (void *)lParam, sizeof(SKINDESCRIPTION));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)psd);

		ID_EXTBK_FIRST = psd->firstItem;
		ID_EXTBK_LAST = psd->lastItem;
		StatusItems = psd->StatusItems;

		TranslateDialogDefault(hwndDlg);
		FillItemList(hwndDlg);
		SendMessage(hwndDlg, WM_USER + 101, 0, 0);

		psd->hMenuItems = CreatePopupMenu();
		AppendMenu(psd->hMenuItems, MF_STRING | MF_DISABLED, (UINT_PTR)0, LPGENT("Copy from"));
		AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);

		{
			for (int i = ID_EXTBK_FIRST; i < ID_EXTBK_LAST; i++) {
				int iOff = StatusItems[i - ID_EXTBK_FIRST]->szName[0] == '{' ? 3 : 0;
				if (iOff)
					AppendMenuA(psd->hMenuItems, MF_SEPARATOR, (UINT_PTR)0, NULL);
				AppendMenuA(psd->hMenuItems, MF_STRING, (UINT_PTR)i, &StatusItems[i - ID_EXTBK_FIRST]->szName[iOff]);
			}
		}
		return TRUE;

	case WM_USER + 101:
		SendDlgItemMessage(hwndDlg, IDC_MRGN_LEFT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MRGN_TOP_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MRGN_RIGHT_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MRGN_BOTTOM_SPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_ALPHASPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));

		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("<None>"));
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Raised"));
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Sunken"));
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Bumped"));
		SendDlgItemMessage(hwndDlg, IDC_BORDERTYPE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Etched"));

		SendDlgItemMessage(hwndDlg, IDC_3DDARKCOLOR, CPM_SETCOLOUR, 0, db_get_dw(NULL, "CLCExt", "3ddark", RGB(224,224,224)));
		SendDlgItemMessage(hwndDlg, IDC_3DLIGHTCOLOR, CPM_SETCOLOUR, 0, db_get_dw(NULL, "CLCExt", "3dbright", RGB(224,224,224)));
		return 0;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			int iItem = dis->itemData;
			StatusItems_t *item = 0;

			SetBkMode(dis->hDC, TRANSPARENT);
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));

			if (iItem >= ID_EXTBK_FIRST && iItem < ID_EXTBK_LAST)
				item = StatusItems[iItem - ID_EXTBK_FIRST];

			if (dis->itemState & ODS_SELECTED && iItem != ID_EXTBKSEPARATOR) {
				FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			else {
				FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
				if (item && item->IGNORED)
					SetTextColor(dis->hDC, RGB(255, 0, 0));
				else
					SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			}
			if (iItem == ID_EXTBKSEPARATOR) {
				HPEN hPen, hPenOld;
				POINT pt;

				hPen = CreatePen(PS_SOLID, 2, GetSysColor(COLOR_WINDOWTEXT));
				hPenOld = (HPEN)SelectObject(dis->hDC, hPen);

				MoveToEx(dis->hDC, dis->rcItem.left, (dis->rcItem.top + dis->rcItem.bottom) / 2, &pt);
				LineTo(dis->hDC, dis->rcItem.right, (dis->rcItem.top + dis->rcItem.bottom) / 2);
				SelectObject(dis->hDC, hPenOld);
				DeleteObject((HGDIOBJ)hPen);
			}
			else if (item) {
				char *szName = item->szName[0] == '{' ? &item->szName[3] : item->szName;

				TextOutA(dis->hDC, dis->rcItem.left, dis->rcItem.top, szName, mir_strlen(szName));
			}
			return TRUE;
		}

	case WM_CONTEXTMENU:
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_ITEMS);

			POINT pt;
			GetCursorPos(&pt);

			RECT rc;
			GetWindowRect(hwndList, &rc);

			if (PtInRect(&rc, pt)) {
				int iSelection = (int)TrackPopupMenu(psd->hMenuItems, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

				if (iSelection >= ID_EXTBK_FIRST && iSelection < ID_EXTBK_LAST) {
					iSelection -= ID_EXTBK_FIRST;
					StatusItems_t *pSel = StatusItems[iSelection];

					for(int i = ID_EXTBK_FIRST; i < ID_EXTBK_LAST; i++) {
						if ( SendMessage(hwndList, LB_GETSEL, i - ID_EXTBK_FIRST, 0) <= 0)
							continue;

						int iIndex = SendMessage(hwndList, LB_GETITEMDATA, i - ID_EXTBK_FIRST, 0);
						iIndex -= ID_EXTBK_FIRST;
						if (iIndex < 0)
							continue;

						StatusItems_t *p = StatusItems[iIndex];
						p->ALPHA = pSel->ALPHA;
						p->BORDERSTYLE = pSel->BORDERSTYLE;
						p->COLOR = pSel->COLOR;
						p->COLOR2 = pSel->COLOR2;
						p->COLOR2_TRANSPARENT = pSel->COLOR2_TRANSPARENT;
						p->CORNER = pSel->CORNER;
						p->GRADIENT = pSel->GRADIENT;
						p->IGNORED = pSel->IGNORED;
						p->imageItem = pSel->imageItem;
						p->MARGIN_BOTTOM = pSel->MARGIN_BOTTOM;
						p->MARGIN_LEFT = pSel->MARGIN_LEFT;
						p->MARGIN_RIGHT = pSel->MARGIN_RIGHT;
						p->MARGIN_TOP = pSel->MARGIN_TOP;
						p->TEXTCOLOR = pSel->TEXTCOLOR;
					}
					OnListItemsChange(hwndDlg);
				}
			}
		}
		break;

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
				if (iItem == ID_EXTBKSEPARATOR)
					return FALSE;
			}
			OnListItemsChange(hwndDlg);
			if (psd->pfnClcOptionsChanged)
				psd->pfnClcOptionsChanged();
			break;
		case IDC_GRADIENT:
			ReActiveCombo(hwndDlg);
			break;
		case IDC_CORNER:
			ReActiveCombo(hwndDlg);
			break;
		case IDC_IGNORE:
			ReActiveCombo(hwndDlg);
			break;
		case IDC_COLOR2_TRANSPARENT:
			ReActiveCombo(hwndDlg);
			break;
		case IDC_BORDERTYPE:
			break;
		}
		if ((LOWORD(wParam) == IDC_ALPHA || LOWORD(wParam) == IDC_MRGN_LEFT || LOWORD(wParam) == IDC_MRGN_BOTTOM || LOWORD(wParam) == IDC_MRGN_TOP || LOWORD(wParam) == IDC_MRGN_RIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
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
				if (psd->pfnSaveCompleteStruct)
					psd->pfnSaveCompleteStruct();
				db_set_dw(NULL, "CLCExt", "3dbright", SendDlgItemMessage(hwndDlg, IDC_3DLIGHTCOLOR, CPM_GETCOLOUR, 0, 0));
				db_set_dw(NULL, "CLCExt", "3ddark", SendDlgItemMessage(hwndDlg, IDC_3DDARKCOLOR, CPM_GETCOLOUR, 0, 0));

				if (psd->pfnClcOptionsChanged)
					psd->pfnClcOptionsChanged();
				if (psd->hwndCLUI) {
					SendMessage(psd->hwndCLUI, WM_SIZE, 0, 0);
					PostMessage(psd->hwndCLUI, WM_USER+100, 0, 0);		// CLUIINTM_REDRAW
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
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
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
	if (wParam)
		FillOptionDialogByCurrentSel((HWND)wParam);
	return 0;
}

/*
 * service function
 * creates additional tab pages under the given parent window handle
 * expects a SKINDESCRIPTON * in lParam
*/

static INT_PTR SkinEdit_Invoke(WPARAM wParam, LPARAM lParam)
{
	SKINDESCRIPTION *psd = (SKINDESCRIPTION *)lParam;
	TCITEM tci = {0};
	RECT rcClient;
	int iTabs;

	if (psd->cbSize != sizeof(SKINDESCRIPTION))
		return 0;

	iTabs = TabCtrl_GetItemCount(psd->hWndTab);
	GetClientRect(psd->hWndParent, &rcClient);

	tci.mask = TCIF_PARAM|TCIF_TEXT;
	tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SKINITEMEDIT), psd->hWndParent, SkinEdit_ExtBkDlgProc, (LPARAM)psd);

	tci.pszText = TranslateT("Skin items");
	TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
	MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 60, 1);
	psd->hwndSkinEdit = (HWND)tci.lParam;

	/*
	tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_IMAGEITEMEDIT), psd->hWndParent, SkinEdit_ImageItemEditProc, (LPARAM)psd);
	tci.pszText = TranslateT("Image items");
	TabCtrl_InsertItem(psd->hWndTab, iTabs++, &tci);
	MoveWindow((HWND)tci.lParam, 5, 25, rcClient.right - 9, rcClient.bottom - 60, 1);
	psd->hwndImageEdit = (HWND)tci.lParam;
	*/

	return (INT_PTR)psd->hwndSkinEdit;
}

static HANDLE hSvc_invoke = 0, hSvc_fillby = 0;

static int LoadModule()
{
	hSvc_invoke = CreateServiceFunction(MS_CLNSE_INVOKE, SkinEdit_Invoke);
	hSvc_fillby = CreateServiceFunction(MS_CLNSE_FILLBYCURRENTSEL, SkinEdit_FillByCurrentSel);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

static int systemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	ModulesLoaded(wParam, lParam);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	return(LoadModule());
}

static int ShutdownProc(WPARAM wParam, LPARAM lParam)
{
	if (hSvc_invoke)
		DestroyServiceFunction(hSvc_invoke);
	if (hSvc_fillby)
		DestroyServiceFunction(hSvc_fillby);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return ShutdownProc(0, 0);
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}
