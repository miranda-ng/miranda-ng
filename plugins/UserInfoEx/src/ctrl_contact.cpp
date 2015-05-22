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

#include "commonheaders.h"

#define MAX_CAT			64

//#define BTN_MENU		(WM_USER+1)
//#define BTN_EDIT		(WM_USER+2)
#define EDIT_VALUE		(WM_USER+3)

#define CBEXM_MENIITEMFIRST (WM_USER+100)	// caution: after this message there must be room for all menuitem commnds
											// so it should be the last message with the highest value

typedef struct TCbExItem
{
	WORD	wMask;
	WORD	wFlags;
	DWORD	dwID;
	TCHAR	szCat[MAX_CAT];
	LPTSTR	pszVal;
	LPCSTR	pszIcon;
	HICON	hIcon;
} CBEXITEMINTERN, *LPCBEXITEMINTERN;

typedef struct TComboEx 
{
	LPCBEXITEMINTERN	pItems;
	int			numItems;
	int			numOther;
	int			iSelectedItem;

	BYTE		bIsChanged;
	BYTE		bLocked;
	BYTE		bIsEditChanged;

	HINSTANCE	hInstance;
	HFONT		hFont;
	HWND		hEdit;
	HWND		hBtnMenu;
	HWND		hBtnEdit;
	HWND		hBtnAdd;
	HWND		hBtnDel;
	RECT		rect;
} CBEX, *LPCBEX;


static int compareProc(LPCVOID cbi1, LPCVOID cbi2)
{
	return mir_tstrcmp(((LPCBEXITEMINTERN)cbi1)->szCat, ((LPCBEXITEMINTERN)cbi2)->szCat);
}

static int CheckPhoneSyntax(LPTSTR pszSrc, LPTSTR szNumber, WORD cchNumber, int& errorPos)
{
	int lenNum = 0;
	BYTE	hasLeftBreaket = FALSE,
			hasRightBreaket = FALSE;

	if (!szNumber || !pszSrc || !*pszSrc || !cchNumber) return 0;
	*szNumber = 0;
	errorPos = -1;
	
	if (*pszSrc != '+') {
		errorPos = 2; // set cursor after first digit
		*(szNumber + lenNum++) = '+';
	}
	else
		*(szNumber + lenNum++) = *(pszSrc++);

	for (; lenNum < cchNumber - 1 && *pszSrc != 0; pszSrc++) {
		switch (*pszSrc) {
			case '(':
				if (hasLeftBreaket) {
					if (errorPos == -1) errorPos = lenNum;
					break;
				}
				if (*(szNumber + lenNum - 1) != ' ') {
					*(szNumber + lenNum++) = ' ';
					if (errorPos == -1) errorPos = lenNum + 1;
				}
				*(szNumber + lenNum++) = *pszSrc;
				hasLeftBreaket = TRUE;
				break;

			case ')':
				if (hasRightBreaket) {
					if (errorPos == -1) errorPos = lenNum;
					break;
				}
				*(szNumber + lenNum++) = *pszSrc;
				if (*(pszSrc + 1) != ' ') {
					*(szNumber + lenNum++) = ' ';
					if (errorPos == -1) errorPos = lenNum;
				}
				hasRightBreaket = TRUE;
				break;

			case ' ':
				if (*(szNumber + lenNum - 1) != ' ') {
					*(szNumber + lenNum++) = *pszSrc;
				}
				else
					if (errorPos == -1) errorPos = lenNum;
				break;

			default:
				if (*pszSrc >= '0' && *pszSrc <= '9' || *pszSrc == '-') {
					*(szNumber + lenNum++) = *pszSrc;
				}
				// remember first error position
				else if (errorPos == -1) errorPos = lenNum;
				break;
		}
	}
	*(szNumber + lenNum) = 0;
	return lenNum;
}

/**
 * name:	 DlgProc_EditEMail()
 * desc:	 dialog procedure
 *
 * return:	 0 or 1
 **/
static INT_PTR CALLBACK DlgProc_EMail(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPCBEXITEM cbi = (LPCBEXITEM)GetUserData(hDlg);

	switch (msg) {
	case WM_INITDIALOG:
		cbi = (LPCBEXITEM)lParam;
		if (!cbi)
			return FALSE;
		SetUserData(hDlg, lParam);

		SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)Skin_GetIcon(ICO_DLG_EMAIL, TRUE));
		if (db_get_b(NULL, MODNAME, SET_ICONS_BUTTONS, 1)) {
			SendDlgItemMessage(hDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_OK));
			SendDlgItemMessage(hDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_CANCEL));
		}

		if (*cbi->pszVal)
			SetWindowText(hDlg, LPGENT("Edit e-mail"));
		TranslateDialogDefault(hDlg);
		SendDlgItemMessage(hDlg, EDIT_CATEGORY, EM_LIMITTEXT, cbi->ccCat - 1, 0);
		SendDlgItemMessage(hDlg, EDIT_EMAIL, EM_LIMITTEXT, cbi->ccVal - 1, 0);
		SetDlgItemText(hDlg, EDIT_CATEGORY, cbi->pszCat);
		SetDlgItemText(hDlg, EDIT_EMAIL, cbi->pszVal);
		EnableWindow(GetDlgItem(hDlg, EDIT_CATEGORY), !(cbi->wFlags & CBEXIF_CATREADONLY));
		EnableWindow(GetDlgItem(hDlg, IDOK), *cbi->pszVal);

		// translate Userinfo buttons
		{
			TCHAR szButton[MAX_PATH];
			HWND hBtn = GetDlgItem(hDlg, IDOK);
			GetWindowText(hBtn, szButton, SIZEOF(szButton));
			SetWindowText(hBtn, TranslateTS(szButton));

			hBtn = GetDlgItem(hDlg, IDCANCEL);
			GetWindowText(hBtn, szButton, SIZEOF(szButton));
			SetWindowText(hBtn, TranslateTS(szButton));
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			if (HIWORD(wParam) == BN_CLICKED) {
				case IDOK:
				{
					if (cbi->pszVal && cbi->ccVal > 0)
						GetDlgItemText(hDlg, EDIT_EMAIL, cbi->pszVal, cbi->ccVal);
					if (cbi->pszCat && cbi->ccCat > 0)
						GetDlgItemText(hDlg, EDIT_CATEGORY, cbi->pszCat, cbi->ccCat);
				}
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					break;
			}
			case EDIT_EMAIL:
				if (HIWORD(wParam) == EN_UPDATE) {
					TCHAR szText[MAXDATASIZE];
					LPTSTR pszAdd, pszDot;
					if (PtrIsValid(cbi)) {
						GetWindowText((HWND)lParam, szText, SIZEOF(szText));
						EnableWindow(GetDlgItem(hDlg, IDOK), 
							((pszAdd = _tcschr(szText, '@')) && 
							*(pszAdd + 1) != '.' &&
							(pszDot = _tcschr(pszAdd, '.')) &&
							*(pszDot + 1) &&
							mir_tstrcmp(szText, cbi->pszVal)));
					}
				}
				break;
		}
		break;
	}
	return FALSE;
}

/**
 * name:	 DlgProc_EditPhone()
 * desc:	 dialog procedure
 *
 * return:	 0 or 1
 **/
INT_PTR CALLBACK DlgProc_Phone(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPCBEXITEM cbi = (LPCBEXITEM)GetUserData(hDlg);
	static int noRecursion = 0;

	switch (msg) {
	case WM_INITDIALOG:
		{
			UINT i, item, countryCount;
			LPIDSTRLIST	pCountries;
			HWND hCombo = GetDlgItem(hDlg, EDIT_COUNTRY);

			cbi = (LPCBEXITEM)lParam;
			if (!cbi) return FALSE;
			SetUserData(hDlg, lParam);

			SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)Skin_GetIcon(ICO_DLG_PHONE, TRUE));
			if (db_get_b(NULL, MODNAME, SET_ICONS_BUTTONS, 1)) {
				SendDlgItemMessage(hDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_OK));
				SendDlgItemMessage(hDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_CANCEL));
			}

			// translate Userinfo buttons
			{
				TCHAR szButton[MAX_PATH];
				HWND hBtn;

				hBtn = GetDlgItem(hDlg, IDOK);
				GetWindowText(hBtn, szButton, SIZEOF(szButton));
				SetWindowText(hBtn, TranslateTS(szButton));
				hBtn = GetDlgItem(hDlg, IDCANCEL);
				GetWindowText(hBtn, szButton, SIZEOF(szButton));
				SetWindowText(hBtn, TranslateTS(szButton));
			}
			if (*cbi->pszVal) SetWindowText(hDlg, LPGENT("Edit phone number"));
			if (cbi->wFlags & CBEXIF_SMS) CheckDlgButton(hDlg, CHECK_SMS, BST_CHECKED);
			TranslateDialogDefault(hDlg);

			EnableWindow(GetDlgItem(hDlg, IDOK), *cbi->pszVal);
			SendDlgItemMessage(hDlg, EDIT_AREA, EM_LIMITTEXT, 31, 0);
			SendDlgItemMessage(hDlg, EDIT_NUMBER, EM_LIMITTEXT, 63, 0);
			SendDlgItemMessage(hDlg, EDIT_CATEGORY, EM_LIMITTEXT, cbi->ccCat - 1, 0);
			SendDlgItemMessage(hDlg, EDIT_PHONE, EM_LIMITTEXT, cbi->ccVal - 1, 0);

			GetCountryList(&countryCount, &pCountries);
			for (i = 0; i < countryCount; i++) {
				if (pCountries[i].nID == 0 || pCountries[i].nID == 0xFFFF) continue;
				item = SendMessage(hCombo, CB_ADDSTRING, NULL, (LPARAM)pCountries[i].ptszTranslated);
				SendMessage(hCombo, CB_SETITEMDATA, item, pCountries[i].nID);
			}

			SetDlgItemText(hDlg, EDIT_PHONE, cbi->pszVal);
			SetDlgItemText(hDlg, EDIT_CATEGORY, cbi->pszCat);
			EnableWindow(GetDlgItem(hDlg, EDIT_CATEGORY), !(cbi->wFlags & CBEXIF_CATREADONLY));
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			if (HIWORD(wParam) == BN_CLICKED) {
				case IDOK:
				{
					TCHAR szText[MAXDATASIZE];
					int errorPos;

					if (!GetDlgItemText(hDlg, EDIT_PHONE, szText, SIZEOF(szText)) || !CheckPhoneSyntax(szText, cbi->pszVal, cbi->ccVal, errorPos) || errorPos > -1) {
						MsgErr(hDlg, TranslateT("The phone number should start with a + and consist of\nnumbers, spaces, brackets and hyphens only."));
						break;
					}
					// save category string
					GetDlgItemText(hDlg, EDIT_CATEGORY, cbi->pszCat, cbi->ccCat);

					// save SMS flag
					if (IsDlgButtonChecked(hDlg, CHECK_SMS) != ((cbi->wFlags & CBEXIF_SMS) == CBEXIF_SMS))
						cbi->wFlags ^= CBEXIF_SMS;
				}
				//fall through
			case IDCANCEL:
				EndDialog(hDlg, wParam);
				break;
			}

		case EDIT_COUNTRY:
			if (HIWORD(wParam) != CBN_SELCHANGE)
				break;

		case EDIT_AREA:
		case EDIT_NUMBER:
			if (LOWORD(wParam) != EDIT_COUNTRY && HIWORD(wParam) != EN_CHANGE) break;
			if (noRecursion) break;
			EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			{
				TCHAR szPhone[MAXDATASIZE], szArea[32], szData[64];
				int	 nCurSel = SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_GETCURSEL, 0, 0);
				UINT	nCountry = (nCurSel != CB_ERR) ? SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_GETITEMDATA, nCurSel, 0) : 0;

				GetDlgItemText(hDlg, EDIT_AREA, szArea, SIZEOF(szArea));
				GetDlgItemText(hDlg, EDIT_NUMBER, szData, SIZEOF(szData));
				mir_sntprintf(szPhone, SIZEOF(szPhone), _T("+%u (%s) %s"), nCountry, szArea, szData);
				noRecursion = 1;
				SetDlgItemText(hDlg, EDIT_PHONE, szPhone);
				noRecursion = 0;
			}
			break;

		case EDIT_PHONE:
			if (HIWORD(wParam) != EN_UPDATE) break;
			if (noRecursion) break;
			noRecursion = 1;
			{
				TCHAR szText[MAXDATASIZE], *pText, *pArea, *pNumber;
				int isValid = 1;
				GetDlgItemText(hDlg, EDIT_PHONE, szText, SIZEOF(szText));
				if (szText[0] != '+') isValid = 0;
				if (isValid) {
					int i, country = _tcstol(szText + 1, &pText, 10);
					if (pText - szText > 4)
						isValid = 0;
					else {
						for (i = SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
							if (country == SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_GETITEMDATA, i, 0)) {
								SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_SETCURSEL, i, 0);
								break;
							}
						}
					}
					if (i < 0) isValid = 0;
				}
				if (isValid) {
					pArea = pText + _tcscspn(pText, _T("0123456789"));
					pText = pArea + _tcsspn(pArea, _T("0123456789"));
					if (*pText) {
						*pText = '\0';
						pNumber = pText + 1 + _tcscspn(pText + 1, _T("0123456789"));
						SetDlgItemText(hDlg, EDIT_NUMBER, pNumber);
					}
					SetDlgItemText(hDlg, EDIT_AREA, pArea);
				}
				if (!isValid) {
					SendDlgItemMessage(hDlg, EDIT_COUNTRY, CB_SETCURSEL, -1, 0);
					SetDlgItemText(hDlg, EDIT_AREA, _T(""));
					SetDlgItemText(hDlg, EDIT_NUMBER, _T(""));
				}
			}
			noRecursion = 0;
			EnableWindow(GetDlgItem(hDlg, IDOK), GetWindowTextLength(GetDlgItem(hDlg, EDIT_PHONE)));
			break;
		}
		break;
	}
	return FALSE;
}

/**
 * name:	CtrlContactWndProc
 * desc:	window procedure for the extended combobox class
 * param:	hwnd	- handle to a extended combobox window
 *			msg		- message to handle
 *			wParam	- message specific
 *			lParam	- message specific
 * return:	message specific
 **/
static LRESULT CALLBACK CtrlContactWndProc(HWND hwnd, UINT msg,	WPARAM wParam, LPARAM lParam) 
{
	LPCBEX	cbex = (LPCBEX)GetWindowLongPtr(hwnd, 0);

	switch (msg) {

	/**
	* name:	WM_NCCREATE
	* desc:	is called to initiate the window creation
	* param:	wParam - not used
	*			lParam - pointer to a CREATESTRUCT
	*
	* return:	FALSE on error, TRUE if initialisation was ok
	**/
	case WM_NCCREATE:
	{
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;

		if (!(cbex = (LPCBEX)mir_calloc(1*sizeof(CBEX))))
			return FALSE;
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)cbex);
		cbex->bLocked = 1;
		cbex->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		cbex->hInstance = cs->hInstance;
		cbex->iSelectedItem = -1;
		cbex->rect.left = cs->x;
		cbex->rect.top = cs->y;
		cbex->rect.right = cs->x + cs->cx;
		cbex->rect.bottom = cs->y + cs->cy;
		return TRUE;
	}

	/**
		* name:	WM_NCCREATE
		* desc:	is called to create all subitems
		* param:	wParam - not used
		*			lParam - not used
		*
		* return:	FALSE on error, TRUE if initialisation was ok
		**/
	case WM_CREATE:
	{
		WORD wHeight = (WORD)(cbex->rect.bottom - cbex->rect.top);
		WORD wWidth = 130;
		WORD x = 0;

		if (!(cbex->hBtnEdit = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				UINFOBUTTONCLASS, 
				_T("none"),
				WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0, 0,
				wWidth, wHeight,
				hwnd,
				NULL,
				cbex->hInstance, NULL))) {
			cbex->bLocked = 0;
			return FALSE;
		}
		x += wWidth + 2;
		wWidth = wHeight;
		if (!(cbex->hBtnMenu = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				UINFOBUTTONCLASS,
				NULL,
				WS_VISIBLE|WS_CHILD|WS_TABSTOP|MBS_PUSHBUTTON|MBS_DOWNARROW,
				x, 0,
				wWidth, wHeight,
				hwnd,
				NULL,
				cbex->hInstance, NULL))) {
			DestroyWindow(cbex->hBtnEdit);
			cbex->bLocked = 0;
			return FALSE;
		}
		x += wWidth + 2;
		wWidth = (WORD)(cbex->rect.right - cbex->rect.left - x - (2 * (wHeight + 2)));
		if (!(cbex->hEdit = CreateWindowEx(WS_EX_CLIENTEDGE,
				_T("Edit"), 
				NULL,
				WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL,
				x, 1,
				wWidth,	wHeight - 2,
				hwnd,
				NULL,
				cbex->hInstance, NULL))) {
			DestroyWindow(cbex->hBtnEdit);
			DestroyWindow(cbex->hBtnMenu);
			cbex->bLocked = 0;
			return FALSE;
		}
		x += wWidth + 2;
		wWidth = wHeight;
		if (!(cbex->hBtnAdd = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				UINFOBUTTONCLASS,
				NULL,
				WS_VISIBLE|WS_CHILD|WS_TABSTOP|MBS_FLAT,
				x, 0,
				wWidth, wHeight,
				hwnd,
				NULL,
				cbex->hInstance, NULL))) {
			DestroyWindow(cbex->hBtnEdit);
			DestroyWindow(cbex->hBtnMenu);
			DestroyWindow(cbex->hEdit);
			cbex->bLocked = 0;
			return FALSE;
		}
		x += wWidth + 2;
		if (!(cbex->hBtnDel = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
				UINFOBUTTONCLASS,
				NULL,
				WS_VISIBLE|WS_CHILD|WS_TABSTOP|MBS_FLAT,
				x, 0,
				wWidth, wHeight,
				hwnd,
				NULL,
				cbex->hInstance, NULL))) {
			DestroyWindow(cbex->hBtnEdit);
			DestroyWindow(cbex->hBtnMenu);
			DestroyWindow(cbex->hEdit);
			DestroyWindow(cbex->hBtnAdd);
			cbex->bLocked = 0;
			return FALSE;
		}

		// set ids
		SetWindowLongPtr(cbex->hBtnEdit, GWLP_ID, BTN_EDIT);
		SetWindowLongPtr(cbex->hBtnMenu, GWLP_ID, BTN_MENU);
		SetWindowLongPtr(cbex->hEdit, GWLP_ID, EDIT_VALUE);
		SetWindowLongPtr(cbex->hBtnAdd, GWLP_ID, BTN_ADD);
		SetWindowLongPtr(cbex->hBtnDel, GWLP_ID, BTN_DEL);
		// set fonts & maximum edit control charachters
		SendMessage(cbex->hEdit, WM_SETFONT, (WPARAM)cbex->hFont, NULL);
		SendMessage(cbex->hEdit, EM_LIMITTEXT, (WPARAM)MAXDATASIZE, NULL);
		// add tooltips
		SendMessage(cbex->hBtnMenu, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Choose the item to display"), MBBF_TCHAR);
		SendMessage(cbex->hBtnEdit, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Edit the currently displayed item"), MBBF_TCHAR);
		SendMessage(cbex->hBtnAdd, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Add a new custom item"), MBBF_TCHAR);
		SendMessage(cbex->hBtnDel, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Delete the selected item"), MBBF_TCHAR);
		// reload icons
		CtrlContactWndProc(hwnd, WM_SETICON, NULL, NULL);
		cbex->bLocked = 0;
		return TRUE;
	}

	/**
		* name:	WM_DESTROY
		* desc:	default destroy message, so clear up memory
		* param:	wParam - not used
		*			lParam - not used
		* return:	return value of DefWindowProc
		**/
	case WM_DESTROY:
		CtrlContactWndProc(hwnd, CBEXM_DELALLITEMS, NULL, NULL);
		DestroyWindow(cbex->hBtnEdit);
		DestroyWindow(cbex->hBtnMenu);
		DestroyWindow(cbex->hBtnAdd);
		DestroyWindow(cbex->hBtnDel);
		DestroyWindow(cbex->hEdit);
		MIR_FREE(cbex);
		break;

	/**
		* name:	WM_CTLCOLOREDIT
		* desc:	is called on a paint message for a dialog item to determine its colour scheme
		* param:	wParam - pointer to a HDC
		*			lParam - pointer to a HWND
		* return:	a brush
		**/
	case WM_CTLCOLOREDIT:
		if (!db_get_b(NULL, MODNAME, SET_PROPSHEET_SHOWCOLOURS, 1) || (HWND)lParam != cbex->hEdit || !cbex->pItems || cbex->iSelectedItem < 0) 
			break;
		return Ctrl_SetTextColour((HDC)wParam, cbex->pItems[cbex->iSelectedItem].wFlags);

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == cbex->hEdit)
				return (BOOL)GetSysColor(COLOR_WINDOW);
		return FALSE;
	/**
		* name:	WM_SETICON
		* desc:	updates the icons of this control
		* param:	wParam - not used
		*			lParam - not used
		* return:	always 0
		**/
	case WM_SETICON:
	{
		HICON hIcon = Skin_GetIcon(ICO_BTN_ADD);
		SendMessage(cbex->hBtnAdd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(cbex->hBtnAdd, (hIcon ? _T("") : _T("+")));

		hIcon = Skin_GetIcon(ICO_BTN_DELETE);
		SendMessage(cbex->hBtnDel, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(cbex->hBtnDel, (hIcon ? _T("") : _T("-")));

		if (cbex->pItems && cbex->numItems > 0) {
			for (int i = 0; i < cbex->numItems; i++)
				cbex->pItems[i].hIcon = Skin_GetIcon(cbex->pItems[i].pszIcon);

			if (cbex->iSelectedItem >= 0 && cbex->iSelectedItem < cbex->numItems)
				SendMessage(cbex->hBtnEdit, BM_SETIMAGE, IMAGE_ICON, (LPARAM)cbex->pItems[cbex->iSelectedItem].hIcon);
		}
		return 0;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		/**
		* name:	BTN_MENU
		* desc:	the button to dropdown the list to show all items is pressed
		**/
		case BTN_MENU:
			if (HIWORD(wParam) == BN_CLICKED) {
				POINT pt = { 0, 0 };
				RECT rc;
				MENUITEMINFO mii;
				int i, nItems;
				HMENU hMenu;

				if (!(hMenu = CreatePopupMenu())) return 0;
				SetFocus((HWND)lParam);

				memset(&mii, 0, sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_ID|MIIM_STRING|MIIM_FTYPE|MIIM_STATE;
				mii.fType = MFT_STRING;

				// insert the items
				for (i = nItems = 0; i < cbex->numItems; i++) {
					if ((cbex->pItems[i].wFlags & CBEXIF_DELETED) || *cbex->pItems[i].szCat == 0) continue;
					mii.fState = (cbex->pItems[i].pszVal && *cbex->pItems[i].pszVal) ? MFS_CHECKED : MFS_UNCHECKED;
					mii.wID = CBEXM_MENIITEMFIRST + i;
					mii.dwTypeData = cbex->pItems[i].szCat;
					if (!InsertMenuItem(hMenu, i, TRUE, &mii)) {
						DestroyMenu(hMenu);
						return 0;
					}
					nItems++;
				}
				// add separator between default and custom items
				if (nItems > 3) {
					mii.fMask = MIIM_FTYPE;
					mii.fType = MFT_SEPARATOR;
					mii.wID = 0;
					mii.dwItemData = 0;
					InsertMenuItem(hMenu, 3, TRUE, &mii);
				}
				ClientToScreen((HWND)lParam, &pt);
				GetClientRect((HWND)lParam, &rc);
				i = TrackPopupMenuEx(hMenu, TPM_RIGHTALIGN|TPM_RETURNCMD, pt.x + rc.right, pt.y + rc.bottom, hwnd, NULL);
				SendMessage(cbex->hBtnMenu, BM_SETCHECK, NULL, NULL);
				if (i >= CBEXM_MENIITEMFIRST && i < CBEXM_MENIITEMFIRST + cbex->numItems) {
					CtrlContactWndProc(hwnd, CBEXM_SETCURSEL, (WPARAM)i - CBEXM_MENIITEMFIRST, NULL);
				}
				DestroyMenu(hMenu);
				return 0;
			}
			break;

		/**
			* name:	BTN_ADD
			* desc:	the button to add a new entry is pressed
			**/
		case BTN_ADD:
			if (HIWORD(wParam) == BN_CLICKED) {
				DLGPROC dlgProc;
				WORD dlgID;
				TCHAR szCat[MAX_CAT] = { 0 };
				TCHAR szVal[MAXDATASIZE] = { 0 };
				CBEXITEM cbi;
				HWND hDlgDetails;

				SetFocus((HWND)lParam);
				if (!(hDlgDetails = GetParent(GetParent(hwnd)))) return 1;
				if (SendMessage(hDlgDetails, PSM_ISLOCKED, NULL, NULL)) return 0;
						
				switch (GetWindowLongPtr(hwnd, GWLP_ID)) {
					case EDIT_PHONE:
						dlgID = IDD_ADDPHONE;
						dlgProc = DlgProc_Phone;
						cbi.pszIcon = ICO_BTN_CUSTOMPHONE;
						break;
					case EDIT_EMAIL:
						dlgID = IDD_ADDEMAIL;
						dlgProc = DlgProc_EMail;
						cbi.pszIcon = ICO_BTN_EMAIL;
						break;
					default:
						return 1;
				}
							
				cbi.iItem = -1;
				cbi.wMask = CBEXIM_CAT|CBEXIM_VAL|CBEXIM_FLAGS|CBEXIM_ICONTEXT;
				cbi.pszCat = szCat;
				cbi.pszVal = szVal;
				cbi.ccCat = MAX_CAT;
				cbi.ccVal = MAXDATASIZE;
				cbi.wFlags = 0;
				cbi.dwID = 0;

				if (DialogBoxParam(ghInst, MAKEINTRESOURCE(dlgID), GetParent(hwnd), dlgProc, (LPARAM)&cbi) == IDOK) {
					MCONTACT hContact = NULL;
							
					SendMessage(hDlgDetails, PSM_GETCONTACT, NULL, (LPARAM)&hContact);
					if (hContact) cbi.wFlags |= CTRLF_HASCUSTOM;
					cbi.wFlags |= CTRLF_CHANGED;
					if (SendMessage(hwnd, CBEXM_ADDITEM, NULL, (LPARAM)&cbi) > CB_ERR) {
						SendMessage(hDlgDetails, PSM_CHANGED, NULL, NULL);
						cbex->bIsChanged = TRUE;
						SendMessage(hwnd, CBEXM_SETCURSEL, cbex->numItems - 1, NULL);
					}
				}
				return 0;
			}
			break;

		/**
			* name:	BTN_EDIT
			* desc:	the button to edit an existing entry is pressed
			**/
		case BTN_EDIT:
			if (HIWORD(wParam) == BN_CLICKED) {
				DLGPROC dlgProc;
				WORD dlgID;
				TCHAR szCat[MAX_CAT] = { 0 };
				TCHAR szVal[MAXDATASIZE] = { 0 };
				CBEXITEM cbi;
				HWND hDlgDetails;

				SetFocus((HWND)lParam);
				if (!(hDlgDetails = GetParent(GetParent(hwnd)))) return 1;
				if (SendMessage(hDlgDetails, PSM_ISLOCKED, NULL, NULL)) return 0;
				if (!cbex->pItems || cbex->iSelectedItem == -1) return 0;

				switch (GetWindowLongPtr(hwnd, GWLP_ID)) {
					case EDIT_PHONE:
						dlgID = IDD_ADDPHONE;
						dlgProc = DlgProc_Phone;
						break;
					case EDIT_EMAIL:
						dlgID = IDD_ADDEMAIL;
						dlgProc = DlgProc_EMail;
						break;
					default:
						return 1;
				}
				cbi.iItem = cbex->iSelectedItem;
				cbi.dwID = 0;
				cbi.wMask = CBEXIM_CAT|CBEXIM_VAL|CBEXIM_FLAGS;
				cbi.pszCat = szCat;
				cbi.pszVal = szVal;
				cbi.ccCat = MAX_CAT;
				cbi.ccVal = MAXDATASIZE;
				if (!CtrlContactWndProc(hwnd, CBEXM_GETITEM, NULL, (LPARAM)&cbi)) {
					MsgErr(hwnd, LPGENT("CRITICAL: Unable to edit current entry!\nThis should not happen!"));
					return 1;
				}

				if (DialogBoxParam(ghInst, MAKEINTRESOURCE(dlgID), GetParent(hwnd), dlgProc, (LPARAM)&cbi) == IDOK) {
					MCONTACT hContact;

					SendMessage(hDlgDetails, PSM_GETCONTACT, NULL, (LPARAM)&hContact);
					if (hContact) cbi.wFlags |= CTRLF_HASCUSTOM;
					cbi.wFlags |= CTRLF_CHANGED;
					SendMessage(hwnd, CBEXM_SETITEM, NULL, (LPARAM)&cbi);
					SendMessage(hDlgDetails, PSM_CHANGED, NULL, NULL);
					cbex->bIsChanged = TRUE;
				}
				return 0;
			}
			break;

		/**
			* name:	BTN_DEL
			* desc:	the button to delete an existing entry is pressed
			**/
		case BTN_DEL:
			if (HIWORD(wParam) == BN_CLICKED) {
				HWND hDlgDetails;
				MSGBOX mBox;
				TCHAR szMsg[MAXDATASIZE];
					
				SetFocus((HWND)lParam);
				if (!(hDlgDetails = GetParent(GetParent(hwnd))) ||
						SendMessage(hDlgDetails, PSM_ISLOCKED, NULL, NULL) ||
						!cbex->pItems ||
						cbex->iSelectedItem < 0 ||
						cbex->iSelectedItem >= cbex->numItems ||
						FAILED(mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("Do you really want to delete the current selected item?\n\t%s\n\t%s"),
						cbex->pItems[cbex->iSelectedItem].szCat, cbex->pItems[cbex->iSelectedItem].pszVal))
			)
				{
						return 1;
				}
				mBox.cbSize = sizeof(MSGBOX);
				mBox.hParent = hDlgDetails;
				mBox.hiLogo = Skin_GetIcon(ICO_DLG_PHONE);
				mBox.uType = MB_YESNO|MB_ICON_QUESTION|MB_NOPOPUP;
				mBox.ptszTitle = TranslateT("Delete");
				mBox.ptszMsg = szMsg;
				if (IDYES == MsgBoxService(NULL, (LPARAM)&mBox)) {
					// clear value for standard entry
					if (cbex->pItems[cbex->iSelectedItem].wFlags & CBEXIF_CATREADONLY) {
						MIR_FREE(cbex->pItems[cbex->iSelectedItem].pszVal);
						SetWindowText(cbex->hEdit, _T(""));
						cbex->pItems[cbex->iSelectedItem].wFlags &= ~CBEXIF_SMS;
						cbex->pItems[cbex->iSelectedItem].wFlags |= CTRLF_CHANGED;
					}
					// clear values for customized database entry
					else 
					if (cbex->pItems[cbex->iSelectedItem].dwID != 0) {
						MIR_FREE(cbex->pItems[cbex->iSelectedItem].pszVal);
						*cbex->pItems[cbex->iSelectedItem].szCat = 0;
						cbex->pItems[cbex->iSelectedItem].wFlags |= CTRLF_CHANGED|CBEXIF_DELETED;
						CtrlContactWndProc(hwnd, CBEXM_SETCURSEL, cbex->iSelectedItem - 1, FALSE);
					}
					// delete default entry
					else
						CtrlContactWndProc(hwnd, CBEXM_DELITEM, NULL, cbex->iSelectedItem);

					SendMessage(hDlgDetails, PSM_CHANGED, NULL, NULL);
					cbex->bIsChanged = TRUE;
				}
				return 0;
			}
			break;

		/**
			* name:	EDIT_VALUE
			* desc:	the edit control wants us to act
			**/
		case EDIT_VALUE:
			switch (HIWORD(wParam)) {
				case EN_UPDATE:
				{
					TCHAR szVal[MAXDATASIZE] = { 0 };
					int ccVal;
					MCONTACT hContact;
					HWND hDlgDetails = GetParent(GetParent(hwnd));
							
					EnableWindow(cbex->hBtnDel, GetWindowTextLength(cbex->hEdit) > 0);

					if (SendMessage(hDlgDetails, PSM_ISLOCKED, NULL, NULL) ||
						cbex->bLocked || 
						!cbex->pItems || 
						cbex->iSelectedItem < 0 || 
						cbex->iSelectedItem >= cbex->numItems) return 1;

					// get the edit control's text value and check it for syntax
					switch (GetWindowLongPtr(hwnd, GWLP_ID)) {
						case EDIT_PHONE:
						{
							int errorPos;
							TCHAR szEdit[MAXDATASIZE];

							if (ccVal = GetWindowText(cbex->hEdit, szEdit, SIZEOF(szEdit))) {
								if (!(ccVal = CheckPhoneSyntax(szEdit, szVal, MAXDATASIZE, errorPos)) || errorPos > -1) {
									SetWindowText(cbex->hEdit, szVal);
									SendMessage(cbex->hEdit, EM_SETSEL, errorPos, errorPos);
								}
							}
							break;
						}
						case EDIT_EMAIL:
							ccVal = GetWindowText(cbex->hEdit, szVal, SIZEOF(szVal));
							break;
						default:
							ccVal = GetWindowText(cbex->hEdit, szVal, SIZEOF(szVal));
							break;
					}
							
					SendMessage(hDlgDetails, PSM_GETCONTACT, NULL, (LPARAM)&hContact);
					if ((cbex->pItems[cbex->iSelectedItem].wFlags & CTRLF_CHANGED) && !(hContact && (cbex->pItems[cbex->iSelectedItem].wFlags & CTRLF_HASCUSTOM))) return 0;
							
					if (*szVal == 0 || !cbex->pItems[cbex->iSelectedItem].pszVal || mir_tstrcmp(szVal, cbex->pItems[cbex->iSelectedItem].pszVal)) {
						cbex->pItems[cbex->iSelectedItem].wFlags |= CTRLF_CHANGED;
						cbex->pItems[cbex->iSelectedItem].wFlags |= (hContact ? CTRLF_HASCUSTOM : CTRLF_HASPROTO);
						cbex->bIsChanged = TRUE;
						InvalidateRect((HWND)lParam, NULL, TRUE);
						SendMessage(hDlgDetails, PSM_CHANGED, NULL, NULL);
					}
					return 0;
				}
				case EN_KILLFOCUS:
				{
					int ccText;
							
					if (!cbex->pItems || cbex->iSelectedItem < 0 || cbex->iSelectedItem >= cbex->numItems) return 1;
					if (!(cbex->pItems[cbex->iSelectedItem].wFlags & CTRLF_CHANGED)) return 0;

					if ((ccText = GetWindowTextLength(cbex->hEdit)) <= 0) {
						if (cbex->pItems[cbex->iSelectedItem].wFlags & CBEXIF_CATREADONLY) {
							MIR_FREE(cbex->pItems[cbex->iSelectedItem].pszVal);
							SetWindowText(cbex->hEdit, _T(""));
							cbex->pItems[cbex->iSelectedItem].wFlags &= ~CBEXIF_SMS;
						}
						else
							CtrlContactWndProc(hwnd, CBEXM_DELITEM, NULL, cbex->iSelectedItem);
						SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, NULL, NULL);
						cbex->bIsChanged = TRUE;
					}
					else
					if (cbex->pItems[cbex->iSelectedItem].pszVal = (LPTSTR)mir_realloc(cbex->pItems[cbex->iSelectedItem].pszVal, (ccText + 2) * sizeof(TCHAR))) {
						cbex->pItems[cbex->iSelectedItem].pszVal[ccText + 1] = 0;
						GetWindowText(cbex->hEdit, cbex->pItems[cbex->iSelectedItem].pszVal, ccText + 1);
					}
					return 0;
				}
			}
			break;
		}
		break;

	/**
		* name:	CBEXM_ADDITEM
		* desc:	add a item to the control
		* param:	wParam - not used
		*			lParam - (LPCBEXITEM)&item
		* return:	CB_ERR on failure, new item index if successful
		**/
	case CBEXM_ADDITEM:
	{
		LPCBEXITEM	pItem = (LPCBEXITEM)lParam;

		if (!pItem) return FALSE;

		// if an item with the id of pItem exists, change it instead of adding a new one
		// but only if it has not been changed by the user yet.
		if ((pItem->wMask & CBEXIM_ID) && cbex->pItems && pItem->dwID != 0) {
			int iIndex;
				
			for (iIndex = 0; iIndex < cbex->numItems; iIndex++) {
				if (cbex->pItems[iIndex].dwID == pItem->dwID) {
					pItem->iItem = iIndex;
					if (cbex->pItems[iIndex].wFlags & CTRLF_CHANGED)
						pItem->wFlags |= CTRLF_CHANGED;
					else
						CtrlContactWndProc(hwnd, CBEXM_SETITEM, 0, lParam);
					return iIndex;
				}
			}
		}

		// add a new item to the combobox
		if (!(cbex->pItems = (LPCBEXITEMINTERN)mir_realloc(cbex->pItems, (cbex->numItems + 1) * sizeof(CBEXITEMINTERN)))) {
			cbex->numItems = 0;
			return CB_ERR;
		}
		
		// set the ID
		cbex->pItems[cbex->numItems].dwID = (pItem->wMask & CBEXIM_ID) ? pItem->dwID : 0;

		// set category string
		if (!pItem->pszCat || !pItem->pszCat[0] || !mir_tstrncpy(cbex->pItems[cbex->numItems].szCat, pItem->pszCat, MAX_CAT)) {
			mir_sntprintf(cbex->pItems[cbex->numItems].szCat, SIZEOF(cbex->pItems[cbex->numItems].szCat), _T("%s %d"), TranslateT("Other"), ++cbex->numOther);
		}

		// set value string
		if ((pItem->wMask & CBEXIM_VAL) && pItem->pszVal && pItem->pszVal[0])
			cbex->pItems[cbex->numItems].pszVal = mir_tstrdup(pItem->pszVal);
		else
			cbex->pItems[cbex->numItems].pszVal = NULL;
		// set icon
		if ((pItem->wMask & CBEXIM_ICONTEXT) && pItem->pszIcon) {
			cbex->pItems[cbex->numItems].pszIcon = pItem->pszIcon;
			cbex->pItems[cbex->numItems].hIcon = Skin_GetIcon(pItem->pszIcon);
		}
		// set flags
		cbex->pItems[cbex->numItems].wFlags = (pItem->wMask & CBEXIM_CAT) ? pItem->wFlags : 0;

		cbex->numItems++;
		return cbex->numItems;
	}

	/**
		* name:	CBEXM_SETITEM
		* desc:	Set an item's information of the control.
		*			If iItem member of CBEXITEM is -1, the currently selected item is changed.
		* param:	wParam - not used
		*			lParam - (LPCBEXITEM)&item
		* return:	CB_ERR on failure, new item index if successful
		**/
	case CBEXM_SETITEM:
	{
		LPCBEXITEM	pItem = (LPCBEXITEM)lParam;

		if (!PtrIsValid(pItem) || !pItem->wMask || !PtrIsValid(cbex->pItems)) return FALSE;
		if (pItem->iItem == -1) pItem->iItem = cbex->iSelectedItem;
		if (pItem->iItem < 0 || pItem->iItem >= cbex->numItems) return FALSE;

		// set new category string
		if (pItem->wMask & CBEXIM_CAT) {
			// set category string
			if (!pItem->pszCat || !pItem->pszCat[0] || !mir_tstrncpy(cbex->pItems[pItem->iItem].szCat, pItem->pszCat, SIZEOF(cbex->pItems[pItem->iItem].szCat))) 
				mir_sntprintf(cbex->pItems[pItem->iItem].szCat, SIZEOF(cbex->pItems[pItem->iItem].szCat), _T("%s %d"), TranslateT("Other"), ++cbex->numOther);
			if (pItem->iItem == cbex->iSelectedItem)
				SetWindowText(cbex->hBtnEdit, cbex->pItems[pItem->iItem].szCat);
		}
		// set new value
		if (pItem->wMask & CBEXIM_VAL) {
			MIR_FREE(cbex->pItems[pItem->iItem].pszVal);
			if (pItem->pszVal && pItem->pszVal[0])
				cbex->pItems[pItem->iItem].pszVal = mir_tstrdup(pItem->pszVal);
			if (pItem->iItem == cbex->iSelectedItem)
				SetWindowText(cbex->hEdit, cbex->pItems[pItem->iItem].pszVal ? cbex->pItems[pItem->iItem].pszVal : _T(""));
		}

		// set icon
		if ((pItem->wMask & CBEXIM_ICONTEXT) && pItem->pszIcon) {
			cbex->pItems[pItem->iItem].pszIcon = pItem->pszIcon;
			cbex->pItems[pItem->iItem].hIcon = Skin_GetIcon(pItem->pszIcon);
			if (pItem->iItem == cbex->iSelectedItem)
				SendMessage(cbex->hBtnEdit, BM_SETIMAGE, IMAGE_ICON, (LPARAM)cbex->pItems[pItem->iItem].hIcon);
		}
		if (pItem->wMask & CBEXIM_FLAGS) {
			cbex->pItems[pItem->iItem].wFlags = pItem->wFlags;
			CtrlContactWndProc(hwnd, CBEXM_ENABLEITEM, NULL, NULL);
		}
		return TRUE;
	}

	/**
		* name:	CBEXM_GETITEM
		* desc:	Get an item from the control.
		*			If iItem member of CBEXITEM is -1, the currently selected item is returned.
		* param:	wParam - not used
		*			lParam - (LPCBEXITEM)&item
		* return:	CB_ERR on failure, new item index if successful
		**/
	case CBEXM_GETITEM:
	{
		LPCBEXITEM	pItem = (LPCBEXITEM)lParam;

		if (!pItem || !cbex->pItems) return FALSE;
			
		// try to find item by id
		if ((pItem->wMask & CBEXIM_ID) && pItem->dwID != 0) {
			int i;

			for (i = 0; i < cbex->numItems; i++) {
				if (cbex->pItems[i].dwID == pItem->dwID)
					break;
			}
			pItem->iItem = i;
		}
		else
		if (pItem->iItem == -1) pItem->iItem = cbex->iSelectedItem;
		if (pItem->iItem < 0 || pItem->iItem >= cbex->numItems) return FALSE;
			
		// return only currently selected itemindex
		if (!pItem->wMask) return TRUE;
		// return the unique id
		if (pItem->wMask & CBEXIM_ID) 
			pItem->dwID = cbex->pItems[pItem->iItem].dwID;
		// return category string
		if ((pItem->wMask & CBEXIM_CAT) && pItem->pszCat) {
			if (*cbex->pItems[pItem->iItem].szCat != 0)
				mir_tstrncpy(pItem->pszCat, cbex->pItems[pItem->iItem].szCat, pItem->ccCat - 1);
			else
				*pItem->pszCat = 0;
		}
		// return value string
		if ((pItem->wMask & CBEXIM_VAL) && pItem->pszVal) {
			if (cbex->pItems[pItem->iItem].pszVal)
				mir_tstrncpy(pItem->pszVal, cbex->pItems[pItem->iItem].pszVal, pItem->ccVal - 1);
			else
				*pItem->pszVal = 0;
		}
		// return the icon
		if (pItem->wMask & CBEXIM_ICONTEXT)
			pItem->pszIcon = cbex->pItems[pItem->iItem].pszIcon;
		// return the flags
		if (pItem->wMask & CBEXIM_FLAGS)
			pItem->wFlags = cbex->pItems[pItem->iItem].wFlags;
		return TRUE;
	}

	/**
		* name:	CBEXM_DELITEM
		* desc:	delete an item from the control
		* param:	wParam - not used
		*			lParam - item index
		* return:	CB_ERR on failure, new item index if successful
		**/
	case CBEXM_DELITEM:
	{
		if (!cbex->pItems || (int)lParam < 0 || (int)lParam >= cbex->numItems || (cbex->pItems[lParam].wFlags & CBEXIF_CATREADONLY))
			return FALSE;	
		MIR_FREE(cbex->pItems[(int)lParam].pszVal);
		memmove(cbex->pItems + (int)lParam, 
			cbex->pItems + (int)lParam + 1,
			(cbex->numItems - (int)lParam - 1) * sizeof(CBEXITEMINTERN));
		cbex->numItems--;
		memset((cbex->pItems + cbex->numItems), 0, sizeof(CBEXITEMINTERN));
		CtrlContactWndProc(hwnd, CBEXM_SETCURSEL, lParam - 1, FALSE);
		return TRUE;
	}

	/**
		* name:	CBEXM_DELITEM
		* desc:	delete an item from the control
		* param:	wParam - not used
		*			lParam - item index
		* return:	CB_ERR on failure, new item index if successful
		**/
	case CBEXM_DELALLITEMS:
	{
		int i;

		if (PtrIsValid(cbex)) {
			if (PtrIsValid(cbex->pItems)) {
				for (i = 0; i < cbex->numItems; i++) {
					MIR_FREE(cbex->pItems[i].pszVal);
				}
				MIR_FREE(cbex->pItems);
				cbex->pItems = NULL;
			}
			cbex->numItems = 0;
			cbex->iSelectedItem = -1;
			SetWindowText(cbex->hEdit, _T(""));
			SetWindowText(cbex->hBtnEdit, _T(""));
			SendMessage(cbex->hBtnEdit, WM_SETICON, NULL, NULL);
		}
		return TRUE;
	}

	/**
		* name:	CBEXM_ENABLEITEM
		* desc:	enables or disables the current item
		* param:	wParam - not used
		*			lParam - not used
		* return:	always 0
		**/
	case CBEXM_ENABLEITEM:
		if (cbex->iSelectedItem >= 0 && cbex->iSelectedItem < cbex->numItems) {
			MCONTACT hContact;
			BYTE bEnabled;
				
			PSGetContact(GetParent(hwnd), hContact);

			bEnabled	= !hContact ||
						(cbex->pItems[cbex->iSelectedItem].wFlags & CTRLF_HASCUSTOM) || 
						!(cbex->pItems[cbex->iSelectedItem].wFlags & (CTRLF_HASPROTO|CTRLF_HASMETA)) ||
						!db_get_b(NULL, MODNAME, SET_PROPSHEET_PCBIREADONLY, 0);

			EnableWindow(cbex->hBtnEdit, bEnabled);
			EnableWindow(cbex->hBtnDel, bEnabled && GetWindowTextLength(cbex->hEdit) > 0);
			EnableWindow(cbex->hEdit, bEnabled);
		}
		break;

	/**
		* name:	CBEXM_ISCHANGED
		* desc:	returns whether the control contains changed values or not
		* param:	wParam - not used
		*			lParam - not used
		* return:	TRUE if control was changed, FALSE if nothing was edited
		**/
	case CBEXM_ISCHANGED:
		return cbex->bIsChanged;

	/**
		* name:	CBEXM_RESETCHANGED
		* desc:	resets changed flag to FALSE
		* param:	wParam - not used
		*			lParam - not used
		* return:	always FALSE
		**/
	case CBEXM_RESETCHANGED:
		cbex->bIsChanged = 0;
		return 0;

	/**
		* name:	CBEXM_SETCURSEL
		* desc:	selects a certain item
		* param:	wParam - index of the item to select
		*			lParam - (BYTE)bValid - if TRUE, the next item with a value is selected
		* return:	always FALSE
		**/
	case CBEXM_SETCURSEL:
	{
		int i;

		if (!cbex->pItems) return 1;
		if ((int)wParam < 0 || (int)wParam >= cbex->numItems) wParam = max(cbex->iSelectedItem, 0);
		cbex->bLocked = 1;
			
		if ((BYTE)lParam == TRUE) {
			int i = (int)wParam;

			cbex->iSelectedItem = (int)wParam;
			while (i < cbex->numItems) {
				if (cbex->pItems[i].pszVal && *cbex->pItems[i].pszVal) {
					cbex->iSelectedItem = i;
					break;
				}
				i++;
			}
		}
		else {
			// search for the next none deleted item
			for (i = (int)wParam; i < cbex->numItems && *cbex->pItems[i].szCat == 0; i++);
			if (i == cbex->numItems && (int)wParam > 0) {
				for (i = 0; i < (int)wParam && *cbex->pItems[i].szCat == 0; i++);
				cbex->iSelectedItem = i == (int)wParam ? 0 : i;
			}
			else
				cbex->iSelectedItem = i;

		}
		SetWindowText(cbex->hBtnEdit, cbex->pItems[cbex->iSelectedItem].szCat);
		SetWindowText(cbex->hEdit, cbex->pItems[cbex->iSelectedItem].pszVal ? cbex->pItems[cbex->iSelectedItem].pszVal : _T(""));
		SendMessage(cbex->hBtnEdit, BM_SETIMAGE, IMAGE_ICON, (LPARAM)cbex->pItems[cbex->iSelectedItem].hIcon);
		CtrlContactWndProc(hwnd, CBEXM_ENABLEITEM, NULL, NULL);
		cbex->bLocked = 0;
		return 0;
	}
	case CBEXM_SORT:
		if (cbex->numItems > 4) {
			qsort(cbex->pItems + 3, cbex->numItems - 3, sizeof(CBEXITEMINTERN), compareProc);
		}
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_SETFOCUS:
		SetFocus(cbex->hEdit);
		SendMessage(cbex->hEdit, EM_SETSEL, 0, (LPARAM)-1);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/**
 * name:	CtrlContactUnLoadModule
 * desc:	calls required operations to clean up used memory and objects
 * param:	wParam - not used
 *			lParam - not used
 * return:	always 0
 **/
int CtrlContactUnLoadModule()
{
	UnregisterClass(UINFOCOMBOEXCLASS, ghInst);
	return 0;
}

/**
 * name:	CtrlContactLoadModule
 * desc:	registers window class and does some other initializations
 * param:	none
 * return:	always 0
 **/
int CtrlContactLoadModule()
{
	WNDCLASSEX wc;
	
	memset(&wc, 0, sizeof(wc));
	wc.cbSize				 = sizeof(wc);
	wc.lpszClassName	= UINFOCOMBOEXCLASS;
	wc.lpfnWndProc		= CtrlContactWndProc;
	wc.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra		 = sizeof(LPCBEX);
	wc.hbrBackground	= (HBRUSH)GetStockObject(COLOR_WINDOW);
	wc.style					= CS_GLOBALCLASS;
	RegisterClassEx(&wc);
	return 0;
}

 
/**
 * name:	CtrlContactAddItemFromDB
 * desc:	add a item read from db to the combobox
 * param:	hCtrl			- windowhandle to extended combobox control
 *			hIcon			- icon to use for custom items
 *			szItem			- category text for the item
 *			wForcedFlags	- force flag for each new entry
 *			hContact		- handle to contact whose settings to add
 *			pszModule		- primary module to search the setting in
 *			pszProto		- contacts protocol module
 *			szSettingVal	- value holding setting
 * return:	TRUE	- if the item is not updated, because its changed flag is set
 *			FALSE	- if item is added or updated successfully
 **/
int CtrlContactAddItemFromDB(
				HWND hCtrl,
				LPCSTR szIcon,
				LPTSTR szItem,
				MCONTACT hContact,
				LPCSTR pszModule,
				LPCSTR pszProto,
				LPCSTR szSettingVal)
{
	DBVARIANT dbv;
	CBEXITEM cbi;

	cbi.pszVal = NULL;
	cbi.dwID = hashSetting(szSettingVal);
	cbi.wFlags = CBEXIF_CATREADONLY|DB::Setting::GetTStringCtrl(hContact, pszModule, pszModule, pszProto, szSettingVal, &dbv);
	if (dbv.type >= DBVT_WCHAR) {
		// no value read from database
		if (cbi.wFlags == CBEXIF_CATREADONLY)
			cbi.pszVal = NULL;
		else { // check the database value
			cbi.pszVal = dbv.ptszVal;
			if (LPTSTR sms = _tcsstr(cbi.pszVal, _T(" SMS"))) {
				cbi.wFlags |= CBEXIF_SMS;
				*sms = 0;
			}
		}
	}
	cbi.pszCat = szItem;
	cbi.iItem = -1;
	cbi.wMask = CBEXIM_ALL;
	cbi.pszIcon = szIcon;
	SendMessage(hCtrl, CBEXM_ADDITEM, NULL, (LPARAM)&cbi);
	db_free(&dbv);
	return (cbi.wFlags & CTRLF_CHANGED) == CTRLF_CHANGED;
}

/**
 * name:	CtrlContactAddMyItemsFromDB
 * desc:	add a item read from db to the combobox
 * param:	hCtrl			- windowhandle to extended combobox control
 *			hIcon			- icon to use for custom items
 *			wForcedFlags	- force flag for each new entry
 *			hContact		- handle to contact whose settings to add
 *			pszModule		- primary module to search the setting in
 *			pszProto		- contacts protocol module
 *			szFormatCat		- format for the category holding setting
 *			szFormatVal		- format for the value holding setting
 * return:	TRUE	- if one of the items was not updated, because its changed flag is set
 *			FALSE	- if all items were added or updated successfully
 **/
int CtrlContactAddMyItemsFromDB(
				HWND hCtrl,
				LPCSTR szIcon,
				WORD wForcedFlags,
				MCONTACT hContact,
				LPCSTR pszModule,
				LPCSTR pszProto,
				LPCSTR szFormatCat,
				LPCSTR szFormatVal)
{
	CBEXITEM cbi;
	DBVARIANT dbv;
	CHAR pszSetting[MAXSETTING];
	WORD i;
	LPTSTR sms;
	int bAnyItemIsChanged = 0;

	memset(&cbi, 0, sizeof(cbi));
	cbi.iItem = -1;
	cbi.wMask = CBEXIM_ALL;
	cbi.pszIcon = szIcon;

	for (i = 0;
		SUCCEEDED(mir_snprintf(pszSetting, SIZEOF(pszSetting), szFormatVal, i)) &&
		(cbi.wFlags = DB::Setting::GetTStringCtrl(hContact, pszModule, pszModule, pszProto, pszSetting, &dbv));
		i++)
	{
		// read value
		cbi.dwID = hashSetting(pszSetting);
		cbi.pszVal = dbv.ptszVal;
		dbv.type = DBVT_DELETED;
		dbv.ptszVal = NULL;

		// read category
		if (SUCCEEDED(mir_snprintf(pszSetting, SIZEOF(pszSetting), szFormatCat, i))) {
			if (cbi.wFlags & CTRLF_HASCUSTOM) {
				if (DB::Setting::GetTString(hContact, pszModule, pszSetting, &dbv))
					dbv.type = DBVT_DELETED;
			}
			else
			if (cbi.wFlags & CTRLF_HASPROTO) {
				if (DB::Setting::GetTString(hContact, pszProto, pszSetting, &dbv))
					dbv.type = DBVT_DELETED;
			}

			if (dbv.type > DBVT_DELETED && dbv.ptszVal && *dbv.ptszVal) {
				cbi.pszCat = dbv.ptszVal;
				dbv.type = DBVT_DELETED;
				dbv.ptszVal = NULL;
			}
		}
		if (sms = _tcsstr(cbi.pszVal, _T(" SMS"))) {
			cbi.wFlags |= CBEXIF_SMS;
			*sms = 0;
		}
		cbi.wFlags |= wForcedFlags;
		if (CB_ERR == SendMessage(hCtrl, CBEXM_ADDITEM, NULL, (LPARAM)&cbi)) 
			break;
		bAnyItemIsChanged |= (cbi.wFlags & CTRLF_CHANGED) == CTRLF_CHANGED;
		if (cbi.pszCat) { 
			mir_free(cbi.pszCat);
			cbi.pszCat = NULL; 
		}
		if (cbi.pszVal) {
			mir_free(cbi.pszVal);
			cbi.pszVal = NULL;
		}
	}
	SendMessage(hCtrl, CBEXM_SORT, NULL, NULL);
	return bAnyItemIsChanged;
}

/**
 * name:	CtrlContactWriteItemToDB
 * desc:	write a item from combobox to database
 * param:	none
 * return:	always 0
 **/
int CtrlContactWriteItemToDB(
				HWND hCtrl,
				MCONTACT hContact,
				LPCSTR pszModule,
				LPCSTR pszProto,
				LPCSTR pszSetting)
{
	TCHAR szVal[MAXDATASIZE];
	CBEXITEM cbi;

	if (!CtrlContactWndProc(hCtrl, CBEXM_ISCHANGED, NULL, NULL)) return 1;

	cbi.wMask = CBEXIM_ID|CBEXIM_VAL|CBEXIM_FLAGS;
	cbi.pszVal = szVal;
	cbi.ccVal = MAXDATASIZE - 4;
	cbi.iItem = 0;
	cbi.dwID = hashSetting(pszSetting);
	if (!CtrlContactWndProc(hCtrl, CBEXM_GETITEM, NULL, (LPARAM)&cbi)) return 1;
	if (!(cbi.wFlags & CTRLF_CHANGED)) return 0;
	if (!hContact && !(pszModule = pszProto)) return 1;
	if (!*szVal)
		db_unset(hContact, pszModule, pszSetting);
	else {
		if (cbi.wFlags & CBEXIF_SMS)
			mir_tstrncat(szVal, _T(" SMS"), SIZEOF(szVal) - mir_tstrlen(szVal));

		if (db_set_ts(hContact, pszModule, pszSetting, szVal)) return 1;
	}
	cbi.wFlags &= ~CTRLF_CHANGED;
	cbi.wMask = CBEXIM_FLAGS;
	CtrlContactWndProc(hCtrl, CBEXM_SETITEM, NULL, (LPARAM)&cbi);
	InvalidateRect(GetDlgItem(hCtrl, EDIT_VALUE), NULL, TRUE);
	return 0;
}

/**
 * name:	CtrlContactWriteMyItemsToDB
 * desc:	write a list of custom items from combobox to database
 * param:	none
 * return:	always 0
 **/
int CtrlContactWriteMyItemsToDB(
				HWND hCtrl,
				int iFirstItem,
				MCONTACT hContact,
				LPCSTR pszModule,
				LPCSTR pszProto,
				LPCSTR szFormatCat,
				LPCSTR szFormatVal)
{
	CHAR pszSetting[MAXSETTING];
	TCHAR szCat[MAX_CAT];
	TCHAR szVal[MAXDATASIZE];
	LPTSTR pszOther;
	CBEXITEM cbi;
	INT_PTR ccOther;
	int i = 0;

	if (!CtrlContactWndProc(hCtrl, CBEXM_ISCHANGED, NULL, NULL)) return 1;
	if (!hContact && !(pszModule = pszProto)) return 1;
	
	pszOther = TranslateT("Other");
	ccOther = mir_tstrlen(pszOther);
	cbi.wMask = CBEXIM_CAT|CBEXIM_VAL|CBEXIM_FLAGS;
	cbi.pszCat = szCat;
	cbi.ccCat = MAX_CAT;
	cbi.pszVal = szVal;
	cbi.ccVal = MAXDATASIZE - 4;
	cbi.iItem = iFirstItem;
	cbi.dwID = 0;
	
	while (CtrlContactWndProc(hCtrl, CBEXM_GETITEM, NULL, (LPARAM)&cbi) && cbi.iItem < 50) {
		if (!(cbi.wFlags & CBEXIF_DELETED) && *szVal) {
			if (cbi.wFlags & CBEXIF_SMS) {
				mir_tstrncat(szVal, _T(" SMS"), SIZEOF(szVal) - mir_tstrlen(szVal));
			}
			mir_snprintf(pszSetting, SIZEOF(pszSetting), szFormatCat, i);
			if (*szCat && _tcsncmp(szCat, pszOther, ccOther)) {
				if (db_set_ts(hContact, pszModule, pszSetting, szCat)) return 1;
			}
			else
				db_unset(hContact, pszModule, pszSetting);
			mir_snprintf(pszSetting, SIZEOF(pszSetting), szFormatVal, i);
			if (db_set_ts(hContact, pszModule, pszSetting, szVal)) return 1;
			cbi.wFlags &= ~CTRLF_CHANGED;
			cbi.wMask = CBEXIM_FLAGS;
			CtrlContactWndProc(hCtrl, CBEXM_SETITEM, NULL, (LPARAM)&cbi);
			cbi.wMask = CBEXIM_CAT|CBEXIM_VAL|CBEXIM_FLAGS;
			i++;
		}
		
		cbi.iItem++;
	}
	DB::Setting::DeleteArray(hContact, pszModule, szFormatCat, i);
	DB::Setting::DeleteArray(hContact, pszModule, szFormatVal, i);
	InvalidateRect(GetDlgItem(hCtrl, EDIT_VALUE), NULL, TRUE);
	return 0;
}
