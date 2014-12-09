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
 * Dialog procedure for the home contact information propertysheetpage
 *
 * @param	 hDlg	- handle to the dialog window
 * @param	 uMsg	- the message to handle
 * @param	 wParam	- parameter
 * @param	 lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcContactHome(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList) {
				TCHAR szAddr[MAX_PATH];
				MCONTACT hContact = lParam;
				LPIDSTRLIST pList;
				UINT nList;

				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				mir_sntprintf(szAddr, SIZEOF(szAddr), _T("%s (%s)"), TranslateT("Address"), TranslateT("home"));
				SetDlgItemText(hDlg, IDC_PAGETITLE, szAddr);
				SendDlgItemMessage(hDlg, BTN_GOTO, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open in browser"), MBBF_TCHAR);
				TranslateDialogDefault(hDlg);

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STREET, SET_CONTACT_STREET, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_CITY, SET_CONTACT_CITY, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_ZIP, SET_CONTACT_ZIP, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STATE, SET_CONTACT_STATE, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_HOMEPAGE, SET_CONTACT_HOMEPAGE, DBVT_TCHAR));

				GetCountryList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_COUNTRY, SET_CONTACT_COUNTRY, DBVT_WORD, pList, nList));

				break;
			}
		}
		return FALSE;

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					LPCSTR pszProto;
					HWND hCtrl;
			
					switch (((LPNMHDR)lParam)->code) {
					case PSN_INFOCHANGED:
						{
							BYTE bChanged = 0;

							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							// phone numbers
							hCtrl = GetDlgItem(hDlg, EDIT_PHONE);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_PHONE, TranslateT(SET_CONTACT_PHONE), hContact, USERINFO, pszProto, SET_CONTACT_PHONE);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_FAX, TranslateT(SET_CONTACT_FAX), hContact, USERINFO, pszProto, SET_CONTACT_FAX);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_CELLULAR, TranslateT(SET_CONTACT_CELLULAR), hContact, USERINFO, pszProto, SET_CONTACT_CELLULAR);
							bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, ICO_BTN_CUSTOMPHONE, 0, hContact, USERINFO, pszProto, SET_CONTACT_MYPHONE_CAT, SET_CONTACT_MYPHONE_VAL);
							SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);
								
							// emails
							hCtrl = GetDlgItem(hDlg, EDIT_EMAIL);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Primary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_EMAIL);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Secondary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_EMAIL0);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Tertiary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_EMAIL1);
							bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, ICO_BTN_EMAIL, 0, hContact, USERINFO, pszProto, SET_CONTACT_MYEMAIL_CAT, SET_CONTACT_MYEMAIL_VAL);
							SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);
							SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bChanged ? PSP_CHANGED : 0);
						}
						break;

					case PSN_APPLY:
						{
							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							hCtrl = GetDlgItem(hDlg, EDIT_PHONE);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_PHONE);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_FAX);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_CELLULAR);
							CtrlContactWriteMyItemsToDB(hCtrl, 3, hContact, USERINFO, pszProto, SET_CONTACT_MYPHONE_CAT, SET_CONTACT_MYPHONE_VAL);
							SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);

							hCtrl = GetDlgItem(hDlg, EDIT_EMAIL);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_EMAIL);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_EMAIL0);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_EMAIL1);
							CtrlContactWriteMyItemsToDB(hCtrl, 3, hContact, USERINFO, pszProto, SET_CONTACT_MYEMAIL_CAT, SET_CONTACT_MYEMAIL_VAL);
							SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);
						}
						break;

					case PSN_ICONCHANGED:
						{
							HICON hIcon = Skin_GetIcon(ICO_BTN_GOTO);
							SendDlgItemMessage(hDlg, BTN_GOTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							SetDlgItemText(hDlg, BTN_GOTO, hIcon ? _T("") : _T("->"));

							hIcon = Skin_GetIcon(ICO_COMMON_ADDRESS);
							SendDlgItemMessage(hDlg, ICO_ADDRESS, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							ShowWindow(GetDlgItem(hDlg, ICO_ADDRESS), hIcon ? SW_SHOW : SW_HIDE);

							SendDlgItemMessage(hDlg, EDIT_PHONE, WM_SETICON, NULL, NULL);
							SendDlgItemMessage(hDlg, EDIT_EMAIL, WM_SETICON, NULL, NULL);
						}
						break;
					}
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case EDIT_HOMEPAGE:
				if (HIWORD(wParam) == EN_UPDATE)
					EnableWindow(GetDlgItem(hDlg, BTN_GOTO), GetWindowTextLength((HWND)lParam) > 0);
				break;

			case BTN_GOTO:
				CEditCtrl::GetObj(hDlg, EDIT_HOMEPAGE)->OpenUrl();
				break;

			case EDIT_COUNTRY:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					LPIDSTRLIST pd = (LPIDSTRLIST)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					UpDate_CountryIcon(GetDlgItem(hDlg, ICO_COUNTRY), pd->nID);
				}
				break;
			}
		}
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}

/**
 * Dialog procedure for the company's contact information propertysheetpage
 *
 * @param	 hDlg	- handle to the dialog window
 * @param	 uMsg	- the message to handle
 * @param	 wParam	- parameter
 * @param	 lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcContactWork(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList)
			{
				TCHAR szAddr[MAX_PATH];
				MCONTACT hContact = lParam;
				LPIDSTRLIST pList;
				UINT nList;

				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				mir_sntprintf(szAddr, SIZEOF(szAddr), _T("%s (%s)"), TranslateT("Address and contact"), TranslateT("company"));
				SetDlgItemText(hDlg, IDC_PAGETITLE, szAddr);
				SendDlgItemMessage(hDlg, BTN_GOTO, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open in browser"), MBBF_TCHAR);
				TranslateDialogDefault(hDlg);

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STREET, SET_CONTACT_COMPANY_STREET, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_CITY, SET_CONTACT_COMPANY_CITY, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_ZIP, SET_CONTACT_COMPANY_ZIP, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_STATE, SET_CONTACT_COMPANY_STATE, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_HOMEPAGE, SET_CONTACT_COMPANY_HOMEPAGE, DBVT_TCHAR));

				GetCountryList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_COUNTRY, SET_CONTACT_COMPANY_COUNTRY, DBVT_WORD, pList, nList));
			}
		}
		break;

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					LPCSTR pszProto;
					HWND hCtrl;
			
					switch (((LPNMHDR)lParam)->code) {
					case PSN_INFOCHANGED:
						{
							BYTE bChanged = 0;

							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							// phone numbers
							hCtrl = GetDlgItem(hDlg, EDIT_PHONE);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_PHONE, TranslateT(SET_CONTACT_PHONE), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_PHONE);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_FAX, TranslateT(SET_CONTACT_FAX), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_FAX);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_CELLULAR, TranslateT(SET_CONTACT_CELLULAR), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_CELLULAR);
							bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, ICO_BTN_CUSTOMPHONE, 0, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYPHONE_CAT, SET_CONTACT_COMPANY_MYPHONE_VAL);
							SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);
								
							// emails
							hCtrl = GetDlgItem(hDlg, EDIT_EMAIL);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Primary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Secondary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL0);
							bChanged |= CtrlContactAddItemFromDB(hCtrl, ICO_BTN_EMAIL, TranslateT("Tertiary e-mail"), hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL1);
							bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, ICO_BTN_EMAIL, 0, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYEMAIL_CAT, SET_CONTACT_COMPANY_MYEMAIL_VAL);
							SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);
							SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bChanged ? PSP_CHANGED : 0);
						}
						break;

					case PSN_APPLY:
						{
							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							hCtrl = GetDlgItem(hDlg, EDIT_PHONE);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_PHONE);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_FAX);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_CELLULAR);
							CtrlContactWriteMyItemsToDB(hCtrl, 3, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYPHONE_CAT, SET_CONTACT_COMPANY_MYPHONE_VAL);
							SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);

							hCtrl = GetDlgItem(hDlg, EDIT_EMAIL);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL0);
							CtrlContactWriteItemToDB(hCtrl, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL1);
							CtrlContactWriteMyItemsToDB(hCtrl, 3, hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYEMAIL_CAT, SET_CONTACT_COMPANY_MYEMAIL_VAL);
							SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);
						}
						break;

					case PSN_ICONCHANGED:
						{
							HICON hIcon = Skin_GetIcon(ICO_BTN_GOTO);
							SendDlgItemMessage(hDlg, BTN_GOTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							SetDlgItemText(hDlg, BTN_GOTO, hIcon ? _T("") : _T("->"));

							hIcon = Skin_GetIcon(ICO_COMMON_ADDRESS);
							SendDlgItemMessage(hDlg, ICO_ADDRESS, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							ShowWindow(GetDlgItem(hDlg, ICO_ADDRESS), hIcon ? SW_SHOW : SW_HIDE);

							SendDlgItemMessage(hDlg, EDIT_PHONE, WM_SETICON, NULL, NULL);
							SendDlgItemMessage(hDlg, EDIT_EMAIL, WM_SETICON, NULL, NULL);
						}
						break;
					}
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case EDIT_HOMEPAGE:
				if (HIWORD(wParam) == EN_UPDATE) 
					EnableWindow(GetDlgItem(hDlg, BTN_GOTO), GetWindowTextLength((HWND)lParam) > 0);
				break;

			case BTN_GOTO:
				CEditCtrl::GetObj(hDlg, EDIT_HOMEPAGE)->OpenUrl();
				break;

			case EDIT_COUNTRY:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					LPIDSTRLIST pd = (LPIDSTRLIST)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					UpDate_CountryIcon(GetDlgItem(hDlg, ICO_COUNTRY), pd->nID);
				}
				break;
			}
		}
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}
