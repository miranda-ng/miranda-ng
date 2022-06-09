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

static INT_PTR CALLBACK DlgProc_AnniversaryEditor(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAnnivDate *pDlgEditAnniv = (MAnnivDate *)GetUserData(m_hwnd);

	switch (uMsg) {
	case WM_INITDIALOG:
		pDlgEditAnniv = (MAnnivDate *)lParam;
		if (!PtrIsValid(pDlgEditAnniv))
			break;

		SetUserData(m_hwnd, lParam);

		// set icons
		if (g_plugin.getByte(SET_ICONS_BUTTONS, 1)) {
			SendDlgItemMessage(m_hwnd, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_BTN_OK));
			SendDlgItemMessage(m_hwnd, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_BTN_CLOSE));
		}
		SendDlgItemMessage(m_hwnd, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)g_plugin.getIcon(IDI_ANNIVERSARY, true));

		// translate controls
		SendDlgItemMessage(m_hwnd, IDOK, BUTTONTRANSLATE, NULL, NULL);
		SendDlgItemMessage(m_hwnd, IDCANCEL, BUTTONTRANSLATE, NULL, NULL);
		TranslateDialogDefault(m_hwnd);

		// init controls
		EnableWindow(GetDlgItem(m_hwnd, EDIT_CATEGORY), pDlgEditAnniv->Id() != ANID_BIRTHDAY);
		SetDlgItemText(m_hwnd, EDIT_CATEGORY, pDlgEditAnniv->Description());
		return TRUE;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)wParam, RGB(255, 255, 255));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case EDIT_CATEGORY:
			if (HIWORD(wParam) == EN_UPDATE)
				EnableWindow(GetDlgItem(m_hwnd, IDOK), GetWindowTextLength((HWND)lParam) > 0);
			break;

		case IDOK:
			{
				// read new description
				HWND hEdit = GetDlgItem(m_hwnd, EDIT_CATEGORY);
				int len = Edit_GetTextLength(hEdit);
				LPTSTR pszText;

				if (len == 0
					|| (pszText = (LPTSTR)_alloca((len + 1) * sizeof(wchar_t))) == nullptr
					|| !Edit_GetText(hEdit, pszText, len + 1))
				{
					MsgErr(m_hwnd, LPGENW("Please enter a valid description first!"));
					break;
				}

				if (mir_wstrcmpi(pszText, pDlgEditAnniv->Description())) {
					pDlgEditAnniv->Description(pszText);
					pDlgEditAnniv->SetFlags(MAnnivDate::MADF_HASCUSTOM | MAnnivDate::MADF_CHANGED);
				}
			}
		  __fallthrough;

		case IDCANCEL:
			return EndDialog(m_hwnd, LOWORD(wParam));

		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct PSPAnniversaryDlg : public PSPBaseDlg
{
	PSPAnniversaryDlg() :
		PSPBaseDlg(IDD_CONTACT_ANNIVERSARY)
	{}

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_AGE, SET_CONTACT_AGE, DBVT_BYTE));
		m_ctrlList->insert(CAnnivEditCtrl::CreateObj(m_hwnd, EDIT_ANNIVERSARY_DATE, nullptr));

		// hContact == NULL or reminder disabled
		CAnnivEditCtrl::GetObj(m_hwnd, EDIT_ANNIVERSARY_DATE)->EnableReminderCtrl(m_hContact != NULL);

		SendDlgItemMessage(m_hwnd, EDIT_AGE, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(m_hwnd, SPIN_AGE, UDM_SETRANGE32, 0, 200);
		return true;
	}

	void OnIconsChanged() override
	{
		const ICONCTRL idIcon[] = {
			{ IDI_BIRTHDAY, STM_SETIMAGE, ICO_BIRTHDAY },
			{ IDI_BTN_ADD, BM_SETIMAGE, BTN_ADD },
			{ IDI_BTN_DELETE, BM_SETIMAGE, BTN_DELETE }
		};
		IcoLib_SetCtrlIcons(m_hwnd, idIcon, _countof(idIcon));
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		CAnnivEditCtrl *pDateCtrl = CAnnivEditCtrl::GetObj(m_hwnd, EDIT_ANNIVERSARY_DATE);

		switch(LOWORD(wParam)) {
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code) {
			case EDIT_ANNIVERSARY_DATE:
				if (!PspIsLocked(m_hwnd) && PtrIsValid(pDateCtrl)) {
					LPNMHDR lpNmhdr = (LPNMHDR)lParam;
					switch (lpNmhdr->code) {
					case DTN_DATETIMECHANGE:
						pDateCtrl->OnDateChanged((LPNMDATETIMECHANGE)lParam);
						break;
					case DTN_DROPDOWN:
						HWND hMonthCal = DateTime_GetMonthCal(lpNmhdr->hwndFrom);
						SetWindowLongPtr(hMonthCal, GWL_STYLE, GetWindowLongPtr(hMonthCal, GWL_STYLE) | MCS_WEEKNUMBERS);
						InvalidateRect(hMonthCal, nullptr, TRUE);
					}
				}
				return FALSE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case EDIT_REMIND:
				if (!PspIsLocked(m_hwnd) && HIWORD(wParam) == EN_UPDATE)
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
					if (IDOK == DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ANNIVERSARY_EDITOR), m_hwnd, DlgProc_AnniversaryEditor, (LPARAM)&Date)) {
						SendMessage(GetParent(m_hwnd), PSM_CHANGED, NULL, NULL);
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
						MsgErr(m_hwnd, LPGENW("No valid date selected for editing!"));
					else if (
						IDOK == DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ANNIVERSARY_EDITOR), m_hwnd, DlgProc_AnniversaryEditor, (LPARAM)pDate) &&
						(pDate->Flags() & (MAnnivDate::MADF_CHANGED | MAnnivDate::MADF_REMINDER_CHANGED)))
					{
						SendMessage(GetParent(m_hwnd), PSM_CHANGED, NULL, NULL);
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
						int rc = MsgBox(m_hwnd, MB_YESNO | MB_ICON_QUESTION | MB_NOPOPUP, LPGENW("Delete"), nullptr,
							LPGENW("Do you really want to delete the %s?"), pCurrent->Description());

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
				if (PtrIsValid(pDateCtrl) && HIWORD(wParam) == BN_CLICKED)
					pDateCtrl->OnReminderChecked();
				return FALSE;
			}
		}

		return PSPBaseDlg::DlgProc(uMsg, wParam, lParam);
	}
};

void InitAnniversaryDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000003;
	uip.pDialog = new PSPAnniversaryDlg();
	uip.dwInitParam = ICONINDEX(IDI_BIRTHDAY);
	uip.szTitle.w = LPGENW("General") L"\\" LPGENW("Anniversaries");
	g_plugin.addUserInfo(wParam, &uip);
}
