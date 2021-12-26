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

/**
 * This is the dialog procedure for the advanced contact information propertysheetpage.
 *
 * @param		hDlg		- handle to the dialog window
 * @param		uMsg		- the message to handle
 * @param		wParam	- parameter
 * @param		lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcOrigin(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList) {
				LPIDSTRLIST pList;
				UINT nList;

				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				TranslateDialogDefault(hDlg);
				SetTimer(hDlg, 1, 5000, nullptr);

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STREET, SET_CONTACT_ORIGIN_STREET, DBVT_WCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_ZIP, SET_CONTACT_ORIGIN_ZIP, DBVT_WCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_CITY, SET_CONTACT_ORIGIN_CITY, DBVT_WCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STATE, SET_CONTACT_ORIGIN_STATE, DBVT_WCHAR));

				GetCountryList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_COUNTRY, SET_CONTACT_ORIGIN_COUNTRY, DBVT_WORD, pList, nList));

				pCtrlList->insert(CTzCombo::CreateObj(hDlg, EDIT_TIMEZONE, nullptr));
			}
		}
		break;

	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->idFrom) {
			case 0:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					LPCSTR pszProto;
					
					switch (((LPNMHDR) lParam)->code) {
					case PSN_INFOCHANGED:
						{
							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							if (hContact) {
								MTime mt;
								
								if (mt.DBGetStamp(hContact, USERINFO, SET_CONTACT_ADDEDTIME) && strstr(pszProto, "ICQ")) {
									uint32_t dwStamp;
									
									dwStamp = DB::Contact::WhenAdded(db_get_dw(hContact, pszProto, "UIN", 0), pszProto);
									if (dwStamp > 0)
										mt.FromStampAsUTC(dwStamp);
								}
								if (mt.IsValid()) {
									wchar_t szTime[MAX_PATH];
									LPTSTR ptr;
									
									mt.UTCToLocal();
									mt.DateFormatLong(szTime, _countof(szTime));
									
									mir_wstrcat(szTime, L" - ");
									ptr = szTime + mir_wstrlen(szTime);
									mt.TimeFormat(ptr, _countof(szTime) - (ptr - szTime));
									SetDlgItemText(hDlg, TXT_DATEADDED, szTime);
								}
							}
						 
							SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
						}
						break;
				
					case PSN_ICONCHANGED:
						{
							const ICONCTRL idIcon[] = {
								{ IDI_TREE_ADDRESS, STM_SETIMAGE, ICO_ADDRESS },
								{ IDI_CLOCK,        STM_SETIMAGE, ICO_CLOCK },
							};

							IcoLib_SetCtrlIcons(hDlg, idIcon, _countof(idIcon));
						}
					}
				}
			} /* switch (((LPNMHDR)lParam)->idFrom) */
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case EDIT_COUNTRY:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					LPIDSTRLIST pd = (LPIDSTRLIST)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					UpDate_CountryIcon(GetDlgItem(hDlg, ICO_COUNTRY), pd->nID);
				}
				break;
			}
		}
		break;

	case WM_TIMER:
		{
			wchar_t szTime[32];
			CTzCombo::GetObj(hDlg, EDIT_TIMEZONE)->GetTime(szTime, _countof(szTime));
			SetDlgItemText(hDlg, TXT_TIME, szTime);
			break;
		}

	case WM_DESTROY:
		KillTimer(hDlg, 1);
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}
