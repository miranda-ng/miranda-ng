/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

#include "stdafx.h"

#define LVF_EDITLABEL	8

#define CELIF_CUSTOM	8	// entry is userdefined (e.g. MyPhoneXX)
#define CELIF_SMS		16	// phone with ability to receive sms
#define LIF_TIPVISIBLE	32	// set if infotip is visible

typedef struct TCECListItem : CTRL {
	LPIDSTRLIST idstrList;
	int idstrListCount;
	int iListItem;
	LPTSTR pszText[2];
} LCITEM, *LPLCITEM;

typedef struct TListCtrl : CTRL {
	HWND hList;
	HWND hTip;
	POINT ptTip;
	int iHotItem;
	int iHotSubItem;
	HFONT hFont;

	struct {
		HWND hEdit;		// handle to edit window
		HWND hBtn;		// button to open dropdown list
		RECT rcCombo;
		struct {
			HWND hDrop;	// dropdown list
			int iItem;	// currently selected item of the dropdown
		} dropDown;
		LPLCITEM pItem;	// the item beiing edited
		int iItem;		// zero based index to item in the listview
		int iSubItem;	// zero based index to subitem
		int iTopIndex;	// zero based index to first visible item on list
	} labelEdit;
} LISTCTRL, *LPLISTCTRL;

typedef int (*MISERVICE)(WPARAM wParam, LPARAM lParam);

typedef struct TProfileEntries {
	LPTSTR szGroup;
	LPCSTR szCatFmt;
	LPCSTR szValFmt;
	MIRANDASERVICE GetList;
} PROFILEENTRY, *LPPROFILEENTRY;

static const PROFILEENTRY pFmt[3] = {
	{ LPGENW("Past"),        "Past%d",        "Past%dText",        (MIRANDASERVICE)GetPastList },
	{ LPGENW("Affiliation"), "Affiliation%d", "Affiliation%dText", (MIRANDASERVICE)GetAffiliationsList },
	{ LPGENW("Interest"),    "Interest%dCat", "Interest%dText",    (MIRANDASERVICE)GetInterestsList }
};

static LRESULT CALLBACK ProfileList_LabelEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern COLORREF clrBoth;
extern COLORREF clrChanged;
extern COLORREF clrCustom;
extern COLORREF clrNormal;
extern COLORREF clrMeta;


/**
 * name:	ProfileList_AddGroup
 * desc:	add a group to the listview
 * param:	hList	- handle to listview control
 *			pszText	- text of new group
 * return:	index the where group was added
 **/
static int ProfileList_AddGroup(HWND hList, LPTSTR pszText, int iItem)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.pszText = pszText;
	lvi.lParam = NULL;
	return ListView_InsertItem(hList, &lvi);
}

/**
 * name:	ProfileList_GetItemText
 * desc:	returns the text of a listview item
 * param:	hList	 - handle to listview control
 *			iItem	 - item index
 *			iSubItem - subitem (column) index
 *			pszText	 - pointer of a buffer to retrieve the text
 *			ccText	 - number of maximal characters pszText can take
 * return:	number of read characters
 **/
static int ProfileList_GetItemText(HWND hList, int iItem, int iSubItem, LPTSTR pszText, int ccText)
{
	LVITEM lvi;
	wchar_t szNull[2];

	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = ccText ? ccText : 2;
	lvi.pszText = pszText ? pszText : szNull;
	return SNDMSG(hList, LVM_GETITEMTEXT, iItem, (LPARAM)&lvi);
}

/**
 * name:	ProfileList_GetItemData
 * desc:	return the infostructure associated with the desired item
 * param:	hList	- handle to listview control
 *			iItem	- item index
 * return:	LPLCITEM structure
 **/
static LPLCITEM ProfileList_GetItemData(HWND hList, int iItem)
{
	LVITEM lvi;

	if (iItem < 0)
		return nullptr;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.iItem = iItem;
	return (SendMessage(hList, LVM_GETITEM, NULL, (LPARAM)&lvi)) ? (LPLCITEM)lvi.lParam : nullptr;
}

/**
 * name:	ProfileList_DeleteItem
 * desc:	delete an item from the listview
 * param:	hList	- handle to listview control
 *			iItem	- item index
 *	
 * return:	nothing
 **/
static void ProfileList_DeleteItem(HWND hList, int iItem)
{
	LPLCITEM pItem = ProfileList_GetItemData(hList, iItem);
	
	if (PtrIsValid(pItem)) {
		if (pItem->pszText[0])
			mir_free(pItem->pszText[0]);
		if (pItem->pszText[1])
			mir_free(pItem->pszText[1]);
		mir_free(pItem);
	}
	ListView_DeleteItem(hList, iItem);
}

/**
 * name:	ProfileList_Clear
 * desc:	delete all list items and their data
 *
 **/
static void ProfileList_Clear(HWND hList)
{
	LVITEM lvi;

	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;

	while (ListView_GetItem(hList, &lvi)) {
		if (PtrIsValid(lvi.lParam)) {
			if (((LPLCITEM)lvi.lParam)->pszText[0])
				mir_free(((LPLCITEM)lvi.lParam)->pszText[0]);
			if (((LPLCITEM)lvi.lParam)->pszText[1])
				mir_free(((LPLCITEM)lvi.lParam)->pszText[1]);
			mir_free((LPVOID)lvi.lParam);
		}
		lvi.iItem++;
	}
	ListView_DeleteAllItems(hList);
}

/**
 * name:	ProfileList_EndLabelEdit
 * desc:	destroys the edit control and saves the text to the list control if desired
 * param:	hList	- handle to listview control
 *			bSave	- tells, whether to save changes (TRUE) or not (FALSE)
 * return:	returns 0 on success or nonzero
 **/
static int ProfileList_EndLabelEdit(LPLISTCTRL pList, uint8_t bSave)
{
	// check if labeledit is enabled
	if (!pList->hList || !pList->labelEdit.hEdit)
		return 1;

	// set hEdit NULL to indicate the endlabeledit call and prevent other calls
	HWND hEdit = pList->labelEdit.hEdit;
	pList->labelEdit.hEdit = nullptr;

	if (bSave != FALSE && pList->labelEdit.pItem) {
		uint16_t ccText;
		LPTSTR	szEdit = nullptr;
		uint8_t bChanged = FALSE;

		// an list element was selected
		if (pList->labelEdit.iSubItem && pList->labelEdit.dropDown.iItem != pList->labelEdit.pItem->iListItem && pList->labelEdit.dropDown.iItem >= 0 && pList->labelEdit.dropDown.iItem < pList->labelEdit.pItem->idstrListCount) {
			pList->labelEdit.pItem->iListItem = pList->labelEdit.dropDown.iItem;
			if (pList->labelEdit.pItem->pszText[0]) {
				mir_free(pList->labelEdit.pItem->pszText[0]);
				pList->labelEdit.pItem->pszText[0] = nullptr;
			}
			bChanged = TRUE;
		}
		// value was edited
		else {
			if ((ccText = GetWindowTextLength(hEdit)) > 0 && (szEdit = (LPTSTR)mir_alloc((ccText + 2) * sizeof(wchar_t)))) {
				GetWindowText(hEdit, szEdit, ccText + 1);
				szEdit[ccText + 1] = 0;

				if (!pList->labelEdit.pItem->pszText[pList->labelEdit.iSubItem]) {
					pList->labelEdit.pItem->pszText[pList->labelEdit.iSubItem] = szEdit;
					bChanged = TRUE;
				}
				else if (mir_wstrcmp(pList->labelEdit.pItem->pszText[pList->labelEdit.iSubItem], szEdit)) {
					mir_free(pList->labelEdit.pItem->pszText[pList->labelEdit.iSubItem]);
					pList->labelEdit.pItem->pszText[pList->labelEdit.iSubItem] = szEdit;
					bChanged = TRUE;
				}
				else
					mir_free(szEdit);
			}
		}
		if (bChanged) {
			pList->labelEdit.pItem->wFlags |= CTRLF_CHANGED;
			pList->wFlags |= CTRLF_CHANGED;
			SendMessage(GetParent(GetParent(pList->hList)), PSM_CHANGED, 0, 0);
		}
	}
	if (pList->labelEdit.hBtn)
		DestroyWindow(pList->labelEdit.hBtn);
	if (pList->labelEdit.dropDown.hDrop)
		DestroyWindow(pList->labelEdit.dropDown.hDrop);
	DestroyWindow(hEdit);
	ListView_RedrawItems(pList->hList, pList->labelEdit.iItem, pList->labelEdit.iItem);
	memset(&pList->labelEdit, 0, sizeof(pList->labelEdit));
	SetFocus(pList->hList);
	return 0;
}

/**
 * name:	ProfileList_BeginLabelEdit
 * desc:	create an edit control to edit the label of the selected item
 * param:	pList	 - handle to listview control's info structure
 *			iItem	 - item index
 *			iSubItem - subitem (column) index
 * return:	handle to the edit control
 **/
static HWND ProfileList_BeginLabelEdit(LPLISTCTRL pList, int iItem, int iSubItem)
{
	if (pList->labelEdit.hEdit)
		ProfileList_EndLabelEdit(pList, FALSE);

	LVITEM lvi;
	lvi.mask = LVIF_PARAM | LVIF_STATE;
	lvi.stateMask = 0xFFFFFFFF;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;

	if (!ListView_GetItem(pList->hList, &lvi))
		return nullptr;

	LPLCITEM pItem = (LPLCITEM)lvi.lParam;

	MCONTACT hContact;
	PSGetContact(GetParent(pList->hList), hContact);

	// do not edit deviders or protocol based contact information
	if (!(lvi.state & LVIS_SELECTED) || !PtrIsValid(pItem) || (hContact && (pItem->wFlags & CTRLF_HASPROTO)))
		return nullptr;

	ListView_EnsureVisible(pList->hList, iItem, FALSE);
	ListView_GetSubItemRect(pList->hList, iItem, iSubItem, LVIR_BOUNDS, &pList->labelEdit.rcCombo);
		
	if (lvi.iSubItem == 0) {
		RECT rc2;
		
		ListView_GetSubItemRect(pList->hList, iItem, 1, LVIR_BOUNDS, &rc2);
		pList->labelEdit.rcCombo.right = rc2.left;
	}

	RECT rcList;
	GetClientRect(pList->hList, &rcList);
	pList->labelEdit.rcCombo.right = min(pList->labelEdit.rcCombo.right, rcList.right);
	pList->labelEdit.rcCombo.left = max(pList->labelEdit.rcCombo.left, rcList.left);
	InflateRect(&pList->labelEdit.rcCombo, -1, -1);

	// create the button control for the combobox
	if (!iSubItem && pItem->idstrList) {
		pList->labelEdit.hBtn = CreateWindowEx(WS_EX_NOPARENTNOTIFY, UINFOBUTTONCLASS, nullptr, WS_VISIBLE | WS_CHILD | MBS_DOWNARROW,
			pList->labelEdit.rcCombo.right - (pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top), pList->labelEdit.rcCombo.top,
			pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top,
			pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top,
			pList->hList, nullptr, g_plugin.getInst(), nullptr);
		if (pList->labelEdit.hBtn) {
			SetWindowLongPtr(pList->labelEdit.hBtn, GWLP_ID, BTN_EDIT);
			pList->labelEdit.rcCombo.right -= pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top;
		}
	}
	else {
		pList->labelEdit.rcCombo.bottom = 3 * pList->labelEdit.rcCombo.bottom - 2 * pList->labelEdit.rcCombo.top;
		if (rcList.bottom < pList->labelEdit.rcCombo.bottom)
			OffsetRect(&pList->labelEdit.rcCombo, 0, rcList.bottom - pList->labelEdit.rcCombo.bottom - 2);
	}
	// create the edit control
	pList->labelEdit.hEdit = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE,
				L"EDIT",
				(!iSubItem && pItem->idstrList && pItem->iListItem > 0 && pItem->iListItem < pItem->idstrListCount) 
					? pItem->idstrList[pItem->iListItem].ptszTranslated
					: (iSubItem >= 0 && iSubItem < 2 && pItem->pszText[iSubItem] && *pItem->pszText[iSubItem])
						? pItem->pszText[iSubItem] 
						: L"", 
						WS_VISIBLE | WS_CHILD | (iSubItem ? (WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL) : ES_AUTOHSCROLL),
				pList->labelEdit.rcCombo.left, pList->labelEdit.rcCombo.top,
				pList->labelEdit.rcCombo.right - pList->labelEdit.rcCombo.left,
				pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top,
				pList->hList, nullptr, g_plugin.getInst(), nullptr);
	if (!pList->labelEdit.hEdit)
		return nullptr;
	SendMessage(pList->labelEdit.hEdit, WM_SETFONT, (WPARAM)(pList->hFont), 0);
	SendMessage(pList->labelEdit.hEdit, EM_SETSEL, 0, (LPARAM)-1);
	SetUserData(pList->labelEdit.hEdit, pList);
	pList->labelEdit.dropDown.iItem = pItem->iListItem;
	pList->labelEdit.iItem = iItem;
	pList->labelEdit.iSubItem = iSubItem;
	pList->labelEdit.iTopIndex = ListView_GetTopIndex(pList->hList);
	pList->labelEdit.pItem = pItem;
	SetFocus(pList->labelEdit.hEdit);
	mir_subclassWindow(pList->labelEdit.hEdit, ProfileList_LabelEditProc);
	return pList->labelEdit.hEdit;
}

/**
 * name:	ProfileList_GetInsertIndex
 * desc:	finds index to add the new item to and adds an devider if necessary
 * param:	hList	- handle to listcontrol to search for the index in
 *			pszList	- database settings string, that identifies this category
 * 
 * return:	zero based index for the new item or -1 on failure
 **/
static int ProfileList_GetInsertIndex(HWND hList, LPTSTR pszGroup)
{
	LVFINDINFO lfi;
	int iDevider, iItem;
	
	// search for the devider to add the new item under
	lfi.flags = LVFI_STRING;
	lfi.psz = pszGroup;
	iDevider = SendMessage(hList, LVM_FINDITEM, (WPARAM)-1, (LPARAM)&lfi);
	
	// devider does not exist yet, add it!
	if (iDevider == -1) {
		LVITEM lvi;

		lvi.mask = LVIF_PARAM | LVIF_TEXT;
		lvi.iSubItem = 0;
		lvi.iItem = 0xFFFF;
		lvi.lParam = NULL;
		lvi.pszText = (LPTSTR)lfi.psz;
		if ((iItem = ListView_InsertItem(hList, &lvi)) == -1)
			return -1;
		iItem++;
	}
	else {
		// search next devider to add new item just before
		lfi.flags = LVFI_PARAM;
		lfi.lParam = NULL;
		if ((iItem = ListView_FindItem(hList, iDevider, &lfi)) == -1)
			iItem = 0xFFFF;
	}
	return iItem;
}

/**
 * name:	ProfileList_DropdownProc
 * desc:	procedure to catch messages for a listbox control for my own combobox
 * param:	hwnd	- handle to the listview control's window
 *			msg		- message sent to the control
 *			wParam	- message specific parameter
 *			lParam	- message specific parameter
 * return:	message specific
**/
static LRESULT CALLBACK ProfileList_DropdownProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPLISTCTRL pList;

	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			SetFocus(GetParent(hwnd));
			return 0;

		case VK_RETURN:
		case VK_F4:
			{
				LPIDSTRLIST pItem;

				if (!PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
					return CB_ERR;
				pList->labelEdit.dropDown.iItem = ListBox_GetCurSel(hwnd);
				if (pList->labelEdit.dropDown.iItem >= 0 && PtrIsValid(pItem = (LPIDSTRLIST)ListBox_GetItemData(hwnd, pList->labelEdit.dropDown.iItem)))
					SetWindowText(pList->labelEdit.hEdit, pItem->ptszTranslated);
				else
					pList->labelEdit.dropDown.iItem = -1;
				SetFocus(pList->labelEdit.hEdit);
				return 0;
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			POINT pt;
			LPIDSTRLIST pItem;

			if (!PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
				return CB_ERR;
			mir_callNextSubclass(hwnd, ProfileList_DropdownProc, msg, wParam, lParam);

			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwnd, &pt);

			if (SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTVSCROLL)
				return CB_ERR;

			pList->labelEdit.dropDown.iItem = SendMessage(hwnd, LB_GETCURSEL, 0, 0);

			if (pList->labelEdit.dropDown.iItem >= 0 && PtrIsValid(pItem = (LPIDSTRLIST)ListBox_GetItemData(hwnd, pList->labelEdit.dropDown.iItem)))
				SetWindowText(pList->labelEdit.hEdit, pItem->ptszTranslated);
			else
				pList->labelEdit.dropDown.iItem = -1;

			ProfileList_EndLabelEdit(pList, TRUE);
			return 0;
		}
	case WM_KILLFOCUS:
		if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))) {
			if (GetFocus() == pList->labelEdit.hEdit) {
				ShowWindow(hwnd, SW_HIDE);
				return 0;
			}
			ProfileList_EndLabelEdit(pList, FALSE);
		}
		return 0;
	}
	return mir_callNextSubclass(hwnd, ProfileList_DropdownProc, msg, wParam, lParam);
 }

/**
 * name:	ProfileList_LabelEditProc
 * desc:	procedure to catch messages for an edit control for my own combobox
 * param:	hwnd	- handle to the listview control's window
 *			msg		- message sent to the control
 *			wParam	- message specific parameter
 *			lParam	- message specific parameter
 * return:	message specific
 **/
static LRESULT CALLBACK ProfileList_LabelEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPLISTCTRL pList;

	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
				ProfileList_EndLabelEdit(pList, FALSE);
			return 0;
		case VK_F3:
			if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)) && pList->labelEdit.hBtn)
				SendMessage(pList->hList, WM_COMMAND, MAKEWPARAM(BTN_EDIT, BN_CLICKED), (LPARAM)pList->labelEdit.hBtn);
			return 0;
		case VK_RETURN:
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_WANTRETURN && !(GetKeyState(VK_CONTROL) & 0x8000))
				break;

			if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))) {
				bool bEditNext = !pList->labelEdit.iSubItem && !ProfileList_GetItemText(pList->hList, pList->labelEdit.iItem, 1, nullptr, NULL);
				ProfileList_EndLabelEdit(pList, TRUE);
				if (bEditNext)
					ProfileList_BeginLabelEdit(pList, pList->labelEdit.iItem, 1);
			}
			return 0;

		case VK_TAB:
			if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))) {
				LVITEM lvi;
				lvi.mask = LVIF_STATE;
				lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
				lvi.iItem = pList->labelEdit.iItem;

				if (!pList->labelEdit.iSubItem) {
					lvi.iSubItem = 1;
					lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
					ProfileList_EndLabelEdit(pList, TRUE);
				}
				else {
					lvi.iSubItem = 0;
					lvi.state = 0;

					ProfileList_EndLabelEdit(pList, TRUE);

					// unselect current list item
					if (!ListView_SetItem(pList->hList, &lvi))
						return 0;

					// search for next valid list item (skip deviders)
					lvi.iSubItem = 0;
					lvi.mask = LVIF_PARAM;
					do {
						lvi.iItem++;
						if (lvi.iItem == -1 || !ListView_GetItem(pList->hList, &lvi))
							return 0;
					} 
					while (!lvi.lParam);

					// select new list item
					lvi.mask = LVIF_STATE;
					lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
					if (!ListView_SetItem(pList->hList, &lvi))
						return 0;
				}
				ProfileList_BeginLabelEdit(pList, lvi.iItem, lvi.iSubItem);
				return 0;
			}
			return 1;
		}
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS | mir_callNextSubclass(hwnd, ProfileList_LabelEditProc, msg, wParam, lParam);

	case WM_KILLFOCUS:
		HWND hwndFocus = GetFocus();

		if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))
			&& hwndFocus != pList->labelEdit.dropDown.hDrop
			&& hwndFocus != pList->labelEdit.hEdit
			&& hwndFocus != pList->labelEdit.hBtn)
			ProfileList_EndLabelEdit(pList, hwndFocus == pList->hList);
		return 0;
	}

	return mir_callNextSubclass(hwnd, ProfileList_LabelEditProc, msg, wParam, lParam);
}

/**
 * name:	ListSubclassProc
 * desc:	procedure to catch messages for a listview control, to handle tooltips
 * param:	hwnd	- handle to the listview control's window
 *			msg		- message sent to the control
 *			wParam	- message specific parameter
 *			lParam	- message specific parameter
 * return:	message specific
 **/
static LRESULT CALLBACK ProfileList_SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	LPLISTCTRL pList = (LPLISTCTRL)GetUserData(hwnd);
	LVHITTESTINFO hi;

	switch (msg) {
	case WM_KEYDOWN:
		{
			int nCurSel, newSel;
			LVITEM lvi;

			switch (wParam) {
			case VK_F2:
				nCurSel = ListView_GetSelectionMark(hwnd);
				if (nCurSel == -1)
					break;
				ProfileList_BeginLabelEdit(pList, nCurSel, 0);
				return 0;
			case VK_F3:
				nCurSel = ListView_GetSelectionMark(hwnd);
				if (nCurSel == -1)
					break;
				ProfileList_BeginLabelEdit(pList, nCurSel, 1);
				return 0;
			case VK_UP:
			case VK_DOWN:
				lvi.iItem = nCurSel = ListView_GetSelectionMark(hwnd);
				lvi.iSubItem = 0;

				// find next valid item to select
				lvi.mask = LVIF_PARAM;
				do {
					if (wParam == VK_UP)
						lvi.iItem--;
					else
						lvi.iItem++;
					if (lvi.iItem == -1 || !ListView_GetItem(hwnd, &lvi))
						return 0;
				}
				while (!lvi.lParam);

				ListView_EnsureVisible(hwnd, lvi.iItem, FALSE);
				newSel = lvi.iItem;
				lvi.iItem = nCurSel;
				lvi.mask = LVIF_STATE;
				lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
				lvi.state = 0;
				ListView_SetItem(hwnd, &lvi);
				lvi.iItem = newSel;
				lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
				ListView_SetItem(hwnd, &lvi);
				ListView_SetSelectionMark(hwnd, lvi.iItem);
				return 0;
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (PtrIsValid(pList)) {
			RECT rchWnd, rcItem;
			SIZE textSize;

			hi.pt.x = GET_X_LPARAM(lParam);
			hi.pt.y = GET_Y_LPARAM(lParam);
			ListView_SubItemHitTest(hwnd, &hi);

			// show tip only if pointer is over an item
			if (pList->iHotItem != hi.iItem || pList->iHotSubItem != hi.iSubItem) {
				bool bReposition = pList->iHotItem != -1 || pList->iHotSubItem != -1;
				pList->iHotItem = hi.iItem;
				pList->iHotSubItem = hi.iSubItem;

				TOOLINFO ti = { 0 };
				LPLCITEM pItem;
				if ((hi.flags & LVHT_ONITEMLABEL) && PtrIsValid(pItem = ProfileList_GetItemData(hwnd, hi.iItem))) {
					GetWindowRect(hwnd, &rchWnd);
					ListView_GetSubItemRect(hwnd, hi.iItem, hi.iSubItem, LVIR_BOUNDS, &rcItem);

					// calculate size of text on the screen
					HDC hDC = GetDC(GetParent(hwnd));
					if (hDC != nullptr) {
						SelectObject(hDC, (HFONT)SendMessage(GetParent(hwnd), WM_GETFONT, NULL, NULL));
						GetTextExtentPoint32(hDC, pItem->pszText[hi.iSubItem], (int)mir_wstrlen(pItem->pszText[hi.iSubItem]), &textSize);
						ReleaseDC(GetParent(hwnd), hDC);
					}
					else textSize.cx = textSize.cy = 0;

					// show tip only for text that is larger than te listview can display
					if (textSize.cx > rchWnd.right - rchWnd.left || textSize.cx > rcItem.right - rcItem.left) {
						ti.cbSize = sizeof(TOOLINFO);
						ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
						ti.hinst = g_plugin.getInst();
						ti.hwnd = hwnd;
						ti.uId = (UINT_PTR)hwnd;
						ti.lpszText = pItem->pszText[hi.iSubItem];
						ti.rect = rcItem;
						SendMessage(pList->hTip, TTM_SETMAXTIPWIDTH, 0, 300);
						SendMessage(pList->hTip, TTM_SETTOOLINFO, NULL, (LPARAM)&ti);
						if (pList->iHotSubItem > 0) {
							SendMessage(pList->hTip, TTM_SETTITLE, 1, (LPARAM)
								((pItem->idstrList && pItem->iListItem > 0 && pItem->iListItem < pItem->idstrListCount)
								? pItem->idstrList[pItem->iListItem].ptszTranslated
								: (pItem->pszText[0] && *pItem->pszText[0])
								? pItem->pszText[0]
								: TranslateT("<empty>"))
								);
							InvalidateRect(pList->hTip, nullptr, TRUE);
						}
						else
							SendMessage(pList->hTip, TTM_SETTITLE, 0, (LPARAM)"");
						SendMessage(pList->hTip, TTM_ACTIVATE, TRUE, (LPARAM)&ti);
						pList->ptTip.x = rchWnd.left + GET_X_LPARAM(lParam) - 16;
						pList->ptTip.y = rchWnd.top + rcItem.top;
						// no TTN_SHOW is called if bReposition is TRUE, so repose here!
						if (bReposition) {
							RECT rcTip;
							GetClientRect(pList->hTip, &rcTip);
							SetWindowPos(pList->hTip, hwnd, pList->ptTip.x, pList->ptTip.y - rcTip.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
						}
						pList->wFlags |= LIF_TIPVISIBLE;
						return 0;
					}
				}
				if (pList->wFlags & LIF_TIPVISIBLE) {
					SendMessage(pList->hTip, TTM_ACTIVATE, FALSE, (LPARAM)&ti);
					pList->wFlags &= ~LIF_TIPVISIBLE;
				}
			}
		}
		return 0;

	case WM_LBUTTONDBLCLK: // begin label edit
		{
			hi.pt.x = GET_X_LPARAM(lParam);
			hi.pt.y = GET_Y_LPARAM(lParam);
			if (ListView_SubItemHitTest(hwnd, &hi))
				ProfileList_BeginLabelEdit(pList, hi.iItem, hi.iSubItem);
			return TRUE;
		}

	case WM_NOTIFY:
		if (!PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
			break;

		// ensure position of tooltip is on the topline of the item
		if (((LPNMHDR)lParam)->hwndFrom == pList->hTip) {
			RECT rcTip;
			GetClientRect(pList->hTip, &rcTip);

			switch (((LPNMHDR)lParam)->code) {
			case TTN_SHOW:
				SetWindowPos(pList->hTip, hwnd, pList->ptTip.x, pList->ptTip.y - rcTip.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		// show dropdown menu for category list
		case BTN_EDIT:
			int i;
			wchar_t szEdit[MAX_PATH];

			if (!PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
				break;
			GetWindowText(pList->labelEdit.hEdit, szEdit, _countof(szEdit));

			// need to create the dropdown list?
			if (pList->labelEdit.dropDown.hDrop == nullptr) {
				const int listHeight = 120;
				RECT rc, rcList;
				int add;

				// dropdown rect
				GetClientRect(pList->hList, &rcList);
				rc.left = pList->labelEdit.rcCombo.left;
				rc.right = pList->labelEdit.rcCombo.right + pList->labelEdit.rcCombo.bottom - pList->labelEdit.rcCombo.top; 

				if (rcList.bottom < pList->labelEdit.rcCombo.bottom + listHeight) {
					rc.bottom = pList->labelEdit.rcCombo.bottom - 7; // don't ask me why!
					rc.top = rc.bottom - listHeight;
				}
				else {
					rc.top = pList->labelEdit.rcCombo.bottom;
					rc.bottom = rc.top + listHeight;
				}

				pList->labelEdit.dropDown.hDrop = CreateWindowEx(0, L"LISTBOX", nullptr,
					WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_COMBOBOX | LBS_HASSTRINGS,
					rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
					hwnd, nullptr, g_plugin.getInst(), nullptr);
				if (!pList->labelEdit.dropDown.hDrop)
					return FALSE;
				SetUserData(pList->labelEdit.dropDown.hDrop, pList);
				mir_subclassWindow(pList->labelEdit.dropDown.hDrop, ProfileList_DropdownProc);
				SetWindowLongPtr(pList->labelEdit.dropDown.hDrop, GWLP_ID, LIST_DROPDOWN);
				SendMessage(pList->labelEdit.dropDown.hDrop, WM_SETFONT, (WPARAM)SendMessage(GetParent(pList->hList), WM_GETFONT, 0, 0), 0);

				// add items
				for (i = 0; i < pList->labelEdit.pItem->idstrListCount; i++) {
					add = ListBox_AddString(pList->labelEdit.dropDown.hDrop, pList->labelEdit.pItem->idstrList[i].ptszTranslated);
					ListBox_SetItemData(pList->labelEdit.dropDown.hDrop, add, pList->labelEdit.pItem->idstrList + i);
					if (!mir_wstrcmp(szEdit, pList->labelEdit.pItem->idstrList[i].ptszTranslated))
						ListBox_SetCurSel(pList->labelEdit.dropDown.hDrop, add);
				}
			}
			else {
				LPIDSTRLIST lpidList;

				i = 0;
				while (PtrIsValid(lpidList = (LPIDSTRLIST)ListBox_GetItemData(pList->labelEdit.dropDown.hDrop, i))) {
					if (!mir_wstrcmp(szEdit, lpidList->ptszTranslated)) {
						ListBox_SetCurSel(pList->labelEdit.dropDown.hDrop, i);
						break;
					}
					i++;
				}
				if (i == pList->labelEdit.pItem->idstrListCount) 
					ListBox_SetCurSel(pList->labelEdit.dropDown.hDrop, -1);
			}
			if (IsWindowVisible(pList->labelEdit.dropDown.hDrop)) {
				SetFocus(pList->labelEdit.hEdit);
			}
			else {
				ShowWindow(pList->labelEdit.dropDown.hDrop, SW_SHOW);
				//SetFocus(pList->labelEdit.dropDown.hDrop);
			}
		}
		break;

	case WM_MOUSEWHEEL:
	case WM_VSCROLL:
	case WM_HSCROLL:
		if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd)))
			ProfileList_EndLabelEdit(pList, FALSE);
		break;

	case WM_KILLFOCUS:
		{
			HWND hwndFocus = GetFocus();

			if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))
				&& pList->labelEdit.hEdit != hwndFocus
				&& pList->labelEdit.dropDown.hDrop != hwndFocus
				&& pList->labelEdit.hBtn != hwndFocus)
				ProfileList_EndLabelEdit(pList, FALSE);
			break;
		}

	case WM_DESTROY:
		if (PtrIsValid(pList = (LPLISTCTRL)GetUserData(hwnd))) {
			HFONT hFont;

			ProfileList_EndLabelEdit(pList, FALSE);
			ProfileList_Clear(hwnd);
			if (PtrIsValid(hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0)) && hFont != pList->hFont)
				DeleteObject(hFont);
			DestroyWindow(pList->hTip);
			mir_free(pList);
		}
		break;
	}
	return mir_callNextSubclass(hwnd, ProfileList_SubclassProc, msg, wParam, lParam);
}

/**
 * name:	DlgProcPspAbout()
 * desc:	dialog procedure
 *
 * return:	0 or 1
 **/

class PSPContactProfileDlg : public PSPBaseDlg
{
	CCtrlListView m_list;

	/**
	* name:	ProfileList_AddItemlistFromDB
	* desc:	reads an zero based indexed szList from the database of hContacts pszModule and fills a hList
	* param:	iItem		- index of new listviewitem
	*			iImage		- image to draw from a imagelist associated with the listview
	*			hContact	- handle to the contact, whose information are read
	*			pszModule	- the module the information are stored in
	*			szCatFormat	- name of a database setting that holds the categorytext
	*			szValFormat - name of a database setting that holds the valuetext
	*			wFlags		- flags to set for a new allocated item
	*
	* return	number of added rows or -1 if listview's changed flag is set
	**/

	int ProfileList_AddItemlistFromDB(
		int &iItem,
		LPIDSTRLIST idList,
		int nList,
		MCONTACT hContact,
		LPCSTR pszModule,
		LPCSTR szCatFormat,
		LPCSTR szValFormat,
		uint16_t wFlags)
	{
		DBVARIANT dbvVal, dbvCat;
		CHAR pszSetting[MAXSETTING];

		LVITEM lvi;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = iItem;

		int i;
		for (i = 0; ; i++) {
			// read the setting from db
			mir_snprintf(pszSetting, szValFormat, i);
			if (DB::Setting::GetWString(hContact, pszModule, pszSetting, &dbvVal))
				break;
			if (dbvVal.type != DBVT_WCHAR)
				continue;
			mir_snprintf(pszSetting, szCatFormat, i);
			DB::Setting::GetAString(hContact, pszModule, pszSetting, &dbvCat);
			
			// create the item object
			LPLCITEM pItem = (LPLCITEM)mir_alloc(sizeof(LCITEM));
			pItem->nType = CTRL_LIST_ITEM;
			pItem->idstrList = idList;
			pItem->idstrListCount = nList;
			pItem->iListItem = 0;
			pItem->pszText[0] = nullptr;
			pItem->pszText[1] = dbvVal.pwszVal;
			pItem->wFlags = wFlags;
			lvi.lParam = (LPARAM)pItem;

			// get id-str-list-item for the category string
			int j = 0;
			if (idList != nullptr) {
				for (j = 0; j < nList; j++) {
					switch (dbvCat.type) {
					case DBVT_BYTE:
						if (dbvCat.bVal != (uint8_t)idList[j].nID)
							continue;
						break;
					case DBVT_WORD:
						if (dbvCat.wVal != (uint16_t)idList[j].nID)
							continue;
						break;
					case DBVT_DWORD:
						if (dbvCat.dVal != (uint32_t)idList[j].nID)
							continue;
						break;
					case DBVT_ASCIIZ:
						if (mir_strcmp(dbvCat.pszVal, idList[j].pszText)) 
							continue;
						break;
					}
					pItem->iListItem = j;
					break;
				}
			}
			// item not found in the predefined category list?
			if ((idList == nullptr || j == nList) && dbvCat.type == DBVT_ASCIIZ) {
				pItem->pszText[0] = mir_a2u(dbvCat.pszVal);
				db_free(&dbvCat);
			}
			if ((lvi.iItem = m_list.InsertItem(&lvi)) < 0) {
				mir_free(pItem);
				db_free(&dbvCat);
				db_free(&dbvVal);
				break;
			}
			lvi.iItem++;
			dbvCat.type = dbvVal.type = DBVT_DELETED;
		}
		iItem = lvi.iItem;
		return i;
	}

	bool ProfileList_AddNewItem(const PROFILEENTRY *pEntry)
	{
		LPLCITEM pItem = (LPLCITEM)mir_alloc(sizeof(LCITEM));
		pItem->nType = CTRL_LIST_ITEM;
		pItem->wFlags = m_hContact ? CTRLF_HASCUSTOM : 0;
		pItem->iListItem = 0;
		pItem->pszText[0] = nullptr;
		pItem->pszText[1] = nullptr;
		// get category list
		pEntry->GetList((WPARAM)&pItem->idstrListCount, (LPARAM)&pItem->idstrList);

		LVITEM lvi;
		lvi.mask = LVIF_PARAM | LVIF_STATE;
		lvi.stateMask = 0xFFFFFFFF;
		lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
		lvi.iItem = ProfileList_GetInsertIndex(m_list.GetHwnd(), pEntry->szGroup);
		lvi.iSubItem = 0;
		lvi.lParam = (LPARAM)pItem;
		if ((lvi.iItem = m_list.InsertItem(&lvi)) >= 0) {
			ProfileList_BeginLabelEdit(pList, lvi.iItem, 0);
			return true;
		}

		mir_free(pItem);
		MsgErr(m_hwnd, LPGENW("Sorry, but there is a problem with adding a new item of type \"%s\""), pEntry->szGroup);
		return false;
	}

public:
	PSPContactProfileDlg() :
		PSPBaseDlg(IDD_CONTACT_PROFILE),
		m_list(this, LIST_PROFILE)
	{
		pList = (LPLISTCTRL)mir_calloc(sizeof(LISTCTRL));
	}

	LPLISTCTRL pList;

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		Ctrl_InitTextColours();

		// init info structure
		pList->hList = m_list.GetHwnd();
		pList->nType = CTRL_LIST_PROFILE;
		memset(&pList->labelEdit, 0, sizeof(pList->labelEdit));
		SetUserData(m_list.GetHwnd(), pList);

		// set new window procedure
		mir_subclassWindow(m_list.GetHwnd(), ProfileList_SubclassProc);

		// remove static edge in aero mode
		if (IsAeroMode())
			SetWindowLongPtr(m_list.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_list.GetHwnd(), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		// insert columns into the listboxes
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		// set listfont
		pList->hFont = (HFONT)m_list.SendMsg(WM_GETFONT, 0, 0);
		pList->wFlags |= LVF_EDITLABEL;

		LOGFONT lf;
		GetObject(pList->hFont, sizeof(lf), &lf);
		lf.lfHeight -= 6;
		HFONT hFont = CreateFontIndirect(&lf);
		m_list.SendMsg(WM_SETFONT, (WPARAM)hFont, 0);

		RECT rc;
		GetClientRect(m_list.GetHwnd(), &rc);
		rc.right -= GetSystemMetrics(SM_CXVSCROLL);

		// initiate the tooltips
		pList->hTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_list.GetHwnd(), nullptr, g_plugin.getInst(), nullptr);
		if (pList->hTip) {
			SetWindowPos(pList->hTip, HWND_TOPMOST, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			TOOLINFO ti;
			memset(&ti, 0, sizeof(TOOLINFO));
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
			ti.hinst = g_plugin.getInst();
			ti.hwnd = m_list.GetHwnd();
			ti.uId = (UINT_PTR)m_list.GetHwnd();
			SendMessage(pList->hTip, TTM_ADDTOOL, NULL, (LPARAM)&ti);
			SendMessage(pList->hTip, TTM_ACTIVATE, FALSE, (LPARAM)&ti);
		}

		// insert columns into the listboxes
		LVCOLUMN lvc;
		lvc.mask = LVCF_WIDTH;
		lvc.cx = rc.right / 8 * 3;
		m_list.InsertColumn(0, &lvc);
		lvc.cx = rc.right / 8 * 5;
		m_list.InsertColumn(1, &lvc);
		return true;
	}

	bool OnApply() override
	{
		if (pList->wFlags & CTRLF_CHANGED) {
			uint8_t iFmt = -1;
			wchar_t szGroup[MAX_PATH];
			CHAR pszSetting[MAXSETTING];
			LPLCITEM pItem;
			int iItem;
			const char *pszModule = USERINFO;

			if (!m_hContact)
				pszModule = GetBaseProto();

			*szGroup = 0;

			LVITEM lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.pszText = szGroup;
			lvi.cchTextMax = _countof(szGroup);

			for (iItem = lvi.iItem = lvi.iSubItem = 0; m_list.GetItem(&lvi); lvi.iItem++) {
				if (!PtrIsValid(pItem = (LPLCITEM)lvi.lParam)) {
					// delete reluctant items
					if (iFmt < _countof(pFmt)) {
						DB::Setting::DeleteArray(m_hContact, pszModule, pFmt[iFmt].szCatFmt, iItem);
						DB::Setting::DeleteArray(m_hContact, pszModule, pFmt[iFmt].szValFmt, iItem);
					}
					// find information about the group
					for (iFmt = 0; iFmt < _countof(pFmt); iFmt++) {
						if (!mir_wstrcmp(szGroup, pFmt[iFmt].szGroup))
							break;
					}
					// indicate, no group was found. should not happen!!
					if (iFmt == _countof(pFmt)) {
						*szGroup = 0;
						iFmt = -1;
					}
					iItem = 0;
				}
				else if (iFmt >= 0 && iFmt < _countof(pFmt)) {
					// save value
					if (!pItem->pszText[1] || !*pItem->pszText[1])
						continue;
					if (!(pItem->wFlags & (CTRLF_HASPROTO|CTRLF_HASMETA))) {
						mir_snprintf(pszSetting, pFmt[iFmt].szValFmt, iItem);
						db_set_ws(m_hContact, pszModule, pszSetting, pItem->pszText[1]);
						// save category
						mir_snprintf(pszSetting, pFmt[iFmt].szCatFmt, iItem);
						if (pItem->idstrList && pItem->iListItem > 0 && pItem->iListItem < pItem->idstrListCount)
							db_set_s(m_hContact, pszModule, pszSetting, (LPSTR)pItem->idstrList[pItem->iListItem].pszText);
						else if (pItem->pszText[0] && *pItem->pszText[0])
							db_set_ws(m_hContact, pszModule, pszSetting, (LPTSTR)pItem->pszText[0]);
						else
							db_unset(m_hContact, pszModule, pszSetting);
						// redraw the item if required
						if (pItem->wFlags & CTRLF_CHANGED) {
							pItem->wFlags &= ~CTRLF_CHANGED;
							m_list.RedrawItems(lvi.iItem, lvi.iItem);
						}
						iItem++;
					}
				}
			}
			// delete reluctant items
			if (iFmt >= 0 && iFmt < _countof(pFmt)) {
				DB::Setting::DeleteArray(m_hContact, pszModule, pFmt[iFmt].szCatFmt, iItem);
				DB::Setting::DeleteArray(m_hContact, pszModule, pFmt[iFmt].szValFmt, iItem);
			}

			pList->wFlags &= ~CTRLF_CHANGED;
		}
		
		return PSPBaseDlg::OnApply();
	}

	bool OnRefresh() override
	{
		int iItem = 0, iGrp = 0, numProtoItems, numUserItems;

		auto *pszProto = GetBaseProto();
		if (!(pList->wFlags & CTRLF_CHANGED) && pszProto) {
			ProfileList_Clear(m_list.GetHwnd());

			// insert the past information
			for (int i = 0; i < 3; i++) {
				int nList;
				LPIDSTRLIST idList;
				pFmt[i].GetList((WPARAM)&nList, (LPARAM)&idList);
				if ((numProtoItems = ProfileList_AddItemlistFromDB(iItem, idList, nList, m_hContact, pszProto, pFmt[i].szCatFmt, pFmt[i].szValFmt, CTRLF_HASPROTO)) < 0)
					return false;

				// scan all basic protocols for the subcontacts
				if (DB::Module::IsMetaAndScan(pszProto)) {
					int iDefault = db_mc_getDefaultNum(m_hContact);
					MCONTACT hSubContact, hDefContact;
					LPCSTR pszSubBaseProto;

					if ((hDefContact = db_mc_getSub(m_hContact, iDefault)) && (pszSubBaseProto = Proto_GetBaseAccountName(hDefContact))) {
						if ((numProtoItems += ProfileList_AddItemlistFromDB(iItem, idList, nList, hDefContact, pszSubBaseProto, pFmt[i].szCatFmt, pFmt[i].szValFmt, CTRLF_HASMETA | CTRLF_HASPROTO)) < 0)
							return false;

						// copy the missing settings from the other subcontacts
						int numSubs = db_mc_getSubCount(m_hContact);
						for (int j = 0; j < numSubs; j++) {
							if (j == iDefault)
								continue;
							if (!(hSubContact = db_mc_getSub(m_hContact, j)))
								continue;
							if (!(pszSubBaseProto = Proto_GetBaseAccountName(hSubContact)))
								continue;
							if ((numProtoItems += ProfileList_AddItemlistFromDB(iItem, idList, nList, hSubContact, pszSubBaseProto, pFmt[i].szCatFmt, pFmt[i].szValFmt, CTRLF_HASMETA | CTRLF_HASPROTO)) < 0)
								return false;
						}
					}
				}
				if ((numUserItems = ProfileList_AddItemlistFromDB(iItem, idList, nList, m_hContact, USERINFO, pFmt[i].szCatFmt, pFmt[i].szValFmt, CTRLF_HASCUSTOM)) < 0)
					return false;

				if (numUserItems || numProtoItems) {
					ProfileList_AddGroup(m_list.GetHwnd(), pFmt[i].szGroup, iGrp);
					iGrp = ++iItem;
				}
			}
		}
		return PSPBaseDlg::OnRefresh();
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		LVHITTESTINFO hi;

		switch (uMsg) {
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORDLG:
			if (IsAeroMode())
				return (INT_PTR)GetStockBrush(WHITE_BRUSH);
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				// some account data may have changed so reread database
				switch (((LPNMHDR)lParam)->code) {
				case PSN_KILLACTIVE: // user swiches to another propertysheetpage
					ProfileList_EndLabelEdit(pList, TRUE);
					break;
				}
				break;

			// handle notification messages from the list control
			case LIST_PROFILE:
				switch (((LPNMHDR)lParam)->code) {
				case NM_RCLICK:
					{
						HMENU hMenu = CreatePopupMenu();
						if (!hMenu)
							return 1;

						POINT pt;
						GetCursorPos(&pt);
						hi.pt = pt;
						ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hi.pt);
						ListView_SubItemHitTest(((LPNMHDR)lParam)->hwndFrom, &hi);
						LPLCITEM pItem = ProfileList_GetItemData(((LPNMHDR)lParam)->hwndFrom, hi.iItem);

						// insert menuitems
						MENUITEMINFO mii = { 0 };					
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID|MIIM_STRING;
						// insert "Add" Menuitem
						mii.wID = BTN_ADD_intEREST;
						mii.dwTypeData = TranslateT("Add interest");
						InsertMenuItem(hMenu, 0, TRUE, &mii);
						mii.wID = BTN_ADD_AFFLIATION;
						mii.dwTypeData = TranslateT("Add affiliation");
						InsertMenuItem(hMenu, 1, TRUE, &mii);
						mii.wID = BTN_ADD_PAST;
						mii.dwTypeData = TranslateT("Add past");
						InsertMenuItem(hMenu, 2, TRUE, &mii);

						if (hi.iItem != -1 && PtrIsValid(pItem) && !(m_hContact && (pItem->wFlags & CTRLF_HASPROTO))) {
							// insert separator
							mii.fMask = MIIM_FTYPE;
							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(hMenu, 3, TRUE, &mii);
							// insert "Delete" Menuitem
							mii.fMask = MIIM_ID | MIIM_STRING;
							mii.wID = BTN_EDIT_CAT;
							mii.dwTypeData = TranslateT("Edit category");
							InsertMenuItem(hMenu, 4, TRUE, &mii);
							mii.wID = BTN_EDIT_VAL;
							mii.dwTypeData = TranslateT("Edit value");
							InsertMenuItem(hMenu, 5, TRUE, &mii);
							mii.fMask = MIIM_FTYPE;
							mii.fType = MFT_SEPARATOR;
							InsertMenuItem(hMenu, 6, TRUE, &mii);
							// insert "Delete" Menuitem
							mii.fMask = MIIM_ID | MIIM_STRING;
							mii.wID = BTN_DEL;
							mii.dwTypeData = TranslateT("Delete");
							InsertMenuItem(hMenu, 7, TRUE, &mii);
						}
						TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, m_hwnd, nullptr);
						DestroyMenu(hMenu);
					}
					return 0;

				case LVN_GETDISPINFO:
					if (pList->labelEdit.iTopIndex != m_list.GetTopIndex())
						ProfileList_EndLabelEdit(pList, FALSE);
					break;

				case NM_CUSTOMDRAW:
					{
						LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)lParam;
						LPLCITEM pItem = (LPLCITEM)cd->nmcd.lItemlParam;
						RECT rc;

						switch (cd->nmcd.dwDrawStage) {
						case CDDS_PREPAINT:
							SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
							return TRUE;

						case CDDS_ITEMPREPAINT:
							ListView_GetItemRect(cd->nmcd.hdr.hwndFrom, cd->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);
							if (!PtrIsValid(pItem)) {
								HFONT hFont = (HFONT)SelectObject(cd->nmcd.hdc, GetBoldFont());
								SetTextColor(cd->nmcd.hdc, GetSysColor(COLOR_3DSHADOW));

								wchar_t szText[MAX_PATH];
								ProfileList_GetItemText(cd->nmcd.hdr.hwndFrom, cd->nmcd.dwItemSpec, 0, szText, MAX_PATH);
								rc.left += 6;
								DrawText(cd->nmcd.hdc, TranslateW(szText), -1, &rc, DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

								rc.bottom -= 2;
								rc.top = rc.bottom - 1;
								rc.left -= 6;
								DrawEdge(cd->nmcd.hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

								SelectObject(cd->nmcd.hdc, hFont);
								SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
								return TRUE;
							}
							// draw selected item
							if ((cd->nmcd.uItemState & CDIS_SELECTED) || (pList->labelEdit.iItem == (int)cd->nmcd.dwItemSpec)) {
								SetTextColor(cd->nmcd.hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
								FillRect(cd->nmcd.hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
							}
							// draw background of unselected item
							else {
								SetTextColor(cd->nmcd.hdc,
									(pItem->wFlags & CTRLF_CHANGED)
									? clrChanged : (pItem->wFlags & CTRLF_HASMETA)
									? clrMeta : ((pItem->wFlags & (CTRLF_HASCUSTOM)) && (pItem->wFlags & CTRLF_HASPROTO))
									? clrBoth : (pItem->wFlags & CTRLF_HASCUSTOM)
									? clrCustom : clrNormal);
								FillRect(cd->nmcd.hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
							}
							SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW);
							return TRUE;

						case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
							ListView_GetSubItemRect(cd->nmcd.hdr.hwndFrom, cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, &rc);
							if (cd->iSubItem == 0) {
								RECT rc2;
								ListView_GetSubItemRect(cd->nmcd.hdr.hwndFrom, cd->nmcd.dwItemSpec, 1, LVIR_BOUNDS, &rc2);
								rc.right = rc2.left;
							}
							rc.left += 3;
							DrawText(cd->nmcd.hdc,
								pItem->pszText[cd->iSubItem]
								? pItem->pszText[cd->iSubItem]
								: (cd->iSubItem == 0 && pItem->idstrList && pItem->iListItem > 0 && pItem->iListItem < pItem->idstrListCount)
								? pItem->idstrList[pItem->iListItem].ptszTranslated
								: TranslateT("<empty>"),
								-1, &rc, DT_END_ELLIPSIS | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
							SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
							return TRUE;
						} /* switch (cd->nmcd.dwDrawStage) */
						break;
					} /* case NM_CUSTOMDRAW: */
				} /* (((LPNMHDR)lParam)->code) */
				break;
			}
			break; /* case WM_NOTIFY: */

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case BTN_ADD_intEREST:
				return ProfileList_AddNewItem(&pFmt[2]);
			case BTN_ADD_AFFLIATION:
				return ProfileList_AddNewItem(&pFmt[1]);
			case BTN_ADD_PAST:
				return ProfileList_AddNewItem(&pFmt[0]);
			case BTN_EDIT_CAT:
				ProfileList_BeginLabelEdit(pList, m_list.GetSelectionMark(), 0);
				break;
			case BTN_EDIT_VAL:
				ProfileList_BeginLabelEdit(pList, m_list.GetSelectionMark(), 1);
				break;
			case BTN_DEL:
				if (IDYES == MsgBox(m_hwnd, MB_YESNO | MB_ICON_QUESTION, LPGENW("Question"), LPGENW("Delete an entry"), LPGENW("Do you really want to delete this entry?"))) {
					int iItem = m_list.GetSelectionMark();
					ProfileList_DeleteItem(m_list.GetHwnd(), iItem);

					pList->wFlags |= CTRLF_CHANGED;
					SendMessage(GetParent(m_hwnd), PSM_CHANGED, NULL, NULL);
					// check if to delete any devider
					if (!ProfileList_GetItemData(m_list.GetHwnd(), iItem--) && !ProfileList_GetItemData(m_list.GetHwnd(), iItem))
						m_list.DeleteItem(iItem);
				}
				break;
			}
			break;
		}
		return FALSE;
	}
};

void InitProfileDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000007;
	uip.pDialog = new PSPContactProfileDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_PROFILE);
	uip.szTitle.w = LPGENW("About") L"\\" LPGENW("Profile");
	g_plugin.addUserInfo(wParam, &uip);
}
