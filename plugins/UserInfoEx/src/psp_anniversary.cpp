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

/**
 * Dialog procedure for the anniversary add/edit dialog
 *
 * @param	 hDlg	- handle to the dialog window
 * @param	 uMsg	- the message to handle
 * @param	 wParam	- parameter
 * @param	 lParam	- parameter
 *
 * @return	different values
 **/
static INT_PTR CALLBACK DlgProc_AnniversaryEditor(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAnnivDate *pDlgEditAnniv = (MAnnivDate *)GetUserData(hDlg);

	switch (uMsg) {
	case WM_INITDIALOG:
		pDlgEditAnniv = (MAnnivDate *)lParam;
		if (!PtrIsValid(pDlgEditAnniv))
			break;

		SetUserData(hDlg, lParam);

		// set icons
		if (db_get_b(NULL, MODNAME, SET_ICONS_BUTTONS, 1)) {
			SendDlgItemMessage(hDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_OK));
			SendDlgItemMessage(hDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_BTN_CANCEL));
		}
		SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)Skin_GetIcon(ICO_DLG_ANNIVERSARY, TRUE));

		// translate controls
		SendDlgItemMessage(hDlg, IDOK, BUTTONTRANSLATE, NULL, NULL);
		SendDlgItemMessage(hDlg, IDCANCEL, BUTTONTRANSLATE, NULL, NULL);
		TranslateDialogDefault(hDlg);

		// init controls
		EnableWindow(GetDlgItem(hDlg, EDIT_CATEGORY), pDlgEditAnniv->Id() != ANID_BIRTHDAY);
		SetDlgItemText(hDlg, EDIT_CATEGORY, pDlgEditAnniv->Description());
		return TRUE;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case EDIT_CATEGORY:
			if (HIWORD(wParam) == EN_UPDATE)
				EnableWindow(GetDlgItem(hDlg, IDOK), GetWindowTextLength((HWND)lParam) > 0);
			break;

		case IDOK: {
			// read new description
			HWND hEdit = GetDlgItem(hDlg, EDIT_CATEGORY);
			int len = Edit_GetTextLength(hEdit);
			LPTSTR pszText;

			if (len == 0
				|| (pszText = (LPTSTR)_alloca((len + 1) * sizeof(TCHAR))) == NULL
				|| !Edit_GetText(hEdit, pszText, len + 1))
			{
				MsgErr(hDlg, LPGENT("Please enter a valid description first!"));
				break;
			}

			if (mir_tstrcmpi(pszText, pDlgEditAnniv->Description())) {
				pDlgEditAnniv->Description(pszText);
				pDlgEditAnniv->SetFlags(MAnnivDate::MADF_HASCUSTOM | MAnnivDate::MADF_CHANGED);
			}
		}
		// fall through
		case IDCANCEL:
			return EndDialog(hDlg, LOWORD(wParam));

		}
	}
	return FALSE;
}

/**
* Dialog procedure for the anniversary propertysheetpage
*
* @param	 hDlg	- handle to the dialog window
* @param	 uMsg	- the message to handle
* @param	 wParam	- parameter
* @param	 lParam	- parameter
*
* @return	different values
**/

INT_PTR CALLBACK PSPProcAnniversary(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CAnnivEditCtrl *pDateCtrl = CAnnivEditCtrl::GetObj(hDlg, EDIT_ANNIVERSARY_DATE);

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList) {
				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				TranslateDialogDefault(hDlg);

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_AGE, SET_CONTACT_AGE, DBVT_BYTE));
				pCtrlList->insert(CAnnivEditCtrl::CreateObj(hDlg, EDIT_ANNIVERSARY_DATE, NULL));

				// hContact == NULL or reminder disabled
				CAnnivEditCtrl::GetObj(hDlg, EDIT_ANNIVERSARY_DATE)->EnableReminderCtrl(lParam != NULL);

				SendDlgItemMessage(hDlg, EDIT_AGE, EM_LIMITTEXT, 3, 0);
				SendDlgItemMessage(hDlg, SPIN_AGE, UDM_SETRANGE32, 0, 200);
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_ICONCHANGED:
				{
					const ICONCTRL idIcon[] = {
						{ ICO_COMMON_BIRTHDAY, STM_SETIMAGE, ICO_BIRTHDAY },
						{ ICO_BTN_ADD, BM_SETIMAGE, BTN_ADD },
						{ ICO_BTN_DELETE, BM_SETIMAGE, BTN_DELETE }
					};
					IcoLib_SetCtrlIcons(hDlg, idIcon, SIZEOF(idIcon));
				}
				return FALSE;
			}
			break; /* case 0 */

		case EDIT_ANNIVERSARY_DATE:
			if (!PspIsLocked(hDlg) && PtrIsValid(pDateCtrl)) {
				LPNMHDR lpNmhdr = (LPNMHDR)lParam;
				switch (lpNmhdr->code) {
				case DTN_DATETIMECHANGE:
					pDateCtrl->OnDateChanged((LPNMDATETIMECHANGE)lParam);
					break;
				case DTN_DROPDOWN:
					HWND hMonthCal = DateTime_GetMonthCal(lpNmhdr->hwndFrom);
					SetWindowLongPtr(hMonthCal, GWL_STYLE, GetWindowLongPtr(hMonthCal, GWL_STYLE) | MCS_WEEKNUMBERS);
					InvalidateRect(hMonthCal, NULL, TRUE);
				}
			}
			return FALSE;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case EDIT_REMIND:
			if (!PspIsLocked(hDlg) && HIWORD(wParam) == EN_UPDATE)
			if (PtrIsValid(pDateCtrl))
				pDateCtrl->OnRemindEditChanged();
			return FALSE;

		/**
		* name:	BTN_MENU
		* desc:	the button to dropdown the list to show all items is pressed
		**/
		case BTN_MENU:
			if (HIWORD(wParam) == BN_CLICKED) {
				if (PtrIsValid(pDateCtrl))
					pDateCtrl->OnMenuPopup();
			}
			return FALSE;

		/**
		* name:	BTN_ADD
		* desc:	creates a new dialog to add a new anniversary
		**/
		case BTN_ADD:
			if (HIWORD(wParam) == BN_CLICKED && PtrIsValid(pDateCtrl)) {
				MAnnivDate Date;
				if (IDOK == DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD_ANNIVERSARY_EDITOR), hDlg, DlgProc_AnniversaryEditor, (LPARAM)&Date)) {
					SendMessage(GetParent(hDlg), PSM_CHANGED, NULL, NULL);
					if (!pDateCtrl->AddDate(Date))
						pDateCtrl->SetCurSel(pDateCtrl->NumDates() - 1);
				}
			}
			return FALSE;

		/**
		* name:	BTN_EDIT
		* desc:	edit the currently selected anniversary
		**/
		case BTN_EDIT:
			if (HIWORD(wParam) == BN_CLICKED && PtrIsValid(pDateCtrl)) {
				MAnnivDate *pDate = pDateCtrl->Current();
				if (!pDate)
					MsgErr(hDlg, LPGENT("No valid date selected for editing!"));
				else if (
					IDOK == DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD_ANNIVERSARY_EDITOR), hDlg, DlgProc_AnniversaryEditor, (LPARAM)pDate) &&
					(pDate->Flags() & (MAnnivDate::MADF_CHANGED | MAnnivDate::MADF_REMINDER_CHANGED)))
				{
					SendMessage(GetParent(hDlg), PSM_CHANGED, NULL, NULL);
					pDateCtrl->SetCurSel(pDateCtrl->CurrentIndex());
				}
			}
			return FALSE;

		/**
		* name:	BTN_DELETE
		* desc:	user wants to delete an anniversary
		**/
		case BTN_DELETE:
			if (HIWORD(wParam) == BN_CLICKED && PtrIsValid(pDateCtrl)) {
				MAnnivDate *pCurrent = pDateCtrl->Current();
				if (pCurrent) {
					int rc = MsgBox(hDlg, MB_YESNO | MB_ICON_QUESTION | MB_NOPOPUP, LPGENT("Delete"), NULL,
						LPGENT("Do you really want to delete the %s?"), pCurrent->Description());

					if (rc == IDYES)
						pDateCtrl->DeleteDate(pDateCtrl->CurrentIndex());
				}
			}
			return FALSE;

		/**
		* name:	CHECK_REMIND
		* desc:	state of reminder checkbox is changed
		**/
		case RADIO_REMIND1:
		case RADIO_REMIND2:
		case RADIO_REMIND3:
			CAnnivEditCtrl *pDateCtrl = CAnnivEditCtrl::GetObj(hDlg, EDIT_ANNIVERSARY_DATE);
			if (PtrIsValid(pDateCtrl) && HIWORD(wParam) == BN_CLICKED)
				pDateCtrl->OnReminderChecked();
			return FALSE;
		} /* switch (LOWORD(wParam)) */
		break;
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}
