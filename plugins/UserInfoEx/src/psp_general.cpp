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
 * Dialog procedure for the contact information propertysheetpage
 *
 * @param	 hDlg	- handle to the dialog window
 * @param	 uMsg	- the message to handle
 * @param	 wParam	- parameter
 * @param	 lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcGeneral(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_TITLE, SET_CONTACT_TITLE, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_FIRSTNAME, SET_CONTACT_FIRSTNAME, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_SECONDNAME, SET_CONTACT_SECONDNAME, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_LASTNAME, SET_CONTACT_LASTNAME, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_NICK, SET_CONTACT_NICK, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_DISPLAYNAME, "CList", SET_CONTACT_MYHANDLE, DBVT_TCHAR));
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_PARTNER, SET_CONTACT_PARTNER, DBVT_TCHAR));

				GetNamePrefixList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_PREFIX, SET_CONTACT_PREFIX, DBVT_BYTE, pList, nList));

				// marital groupbox
				GetMaritalList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_MARITAL, SET_CONTACT_MARITAL, DBVT_BYTE, pList, nList));

				GetLanguageList(&nList, &pList);
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_LANG1, SET_CONTACT_LANG1, DBVT_TCHAR, pList, nList));
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_LANG2, SET_CONTACT_LANG2, DBVT_TCHAR, pList, nList));
				pCtrlList->insert(CCombo::CreateObj(hDlg, EDIT_LANG3, SET_CONTACT_LANG3, DBVT_TCHAR, pList, nList));
			}
		}
		break;

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				{
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
					char* pszProto;

					switch (((LPNMHDR)lParam)->code) {
					case PSN_INFOCHANGED:
						{
							BYTE bEnable;
							DBVARIANT dbv;
							CCtrlFlags Flags;
		
							if (PSGetBaseProto(hDlg, pszProto) && *pszProto) {
								Flags.W = DB::Setting::GetTStringCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv);
								if (Flags.B.hasCustom || Flags.B.hasProto || Flags.B.hasMeta) {
									if (dbv.type == DBVT_BYTE) {
										CheckDlgButton(hDlg, RADIO_FEMALE, (dbv.bVal == 'F'));
										CheckDlgButton(hDlg, RADIO_MALE, (dbv.bVal == 'M'));

										bEnable = !hContact || Flags.B.hasCustom || !db_get_b(NULL, MODNAME, SET_PROPSHEET_PCBIREADONLY, 0);
										EnableWindow(GetDlgItem(hDlg, RADIO_FEMALE), bEnable);
										EnableWindow(GetDlgItem(hDlg, RADIO_MALE), bEnable);
									}
									else
										db_free(&dbv);
								}
							}
						}
						break;

					case PSN_APPLY:
						{
							if (!PSGetBaseProto(hDlg, pszProto) || *pszProto == 0)
								break;

							// gender
							{
								BYTE gender
									= IsDlgButtonChecked(hDlg, RADIO_FEMALE)
									? 'F'
									: IsDlgButtonChecked(hDlg, RADIO_MALE)
									? 'M'
									: 0;

								if (gender)
									db_set_b(hContact, hContact ? USERINFO : pszProto, SET_CONTACT_GENDER, gender);
								else
									db_unset(hContact, hContact ? USERINFO : pszProto, SET_CONTACT_GENDER);
							}
						}
						break;

					case PSN_ICONCHANGED:
						{
							const ICONCTRL idIcon[] = {
								{ ICO_COMMON_FEMALE,  STM_SETIMAGE, ICO_FEMALE },
								{ ICO_COMMON_MALE,    STM_SETIMAGE, ICO_MALE },
								{ ICO_COMMON_MARITAL, STM_SETIMAGE, ICO_MARITAL },
							};
							IcoLib_SetCtrlIcons(hDlg, idIcon, SIZEOF(idIcon));
						}
					}
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			MCONTACT hContact;
			LPCSTR pszProto;

			switch (LOWORD(wParam)) {
			case RADIO_FEMALE:
				{
					if (!PspIsLocked(hDlg) && HIWORD(wParam) == BN_CLICKED) {
						DBVARIANT dbv;

						PSGetContact(hDlg, hContact);
						PSGetBaseProto(hDlg, pszProto);

						if (!DB::Setting::GetAsIsCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv)
							|| dbv.type != DBVT_BYTE
							|| (dbv.bVal != 'F' && SendMessage((HWND)lParam, BM_GETCHECK, NULL, NULL)))
							SendMessage(GetParent(hDlg), PSM_CHANGED, NULL, NULL);
					}
				}
				break;

			case RADIO_MALE:
				{
					if (!PspIsLocked(hDlg) && HIWORD(wParam) == BN_CLICKED) {
						DBVARIANT dbv;

						PSGetContact(hDlg, hContact);
						PSGetBaseProto(hDlg, pszProto);

						if (!DB::Setting::GetAsIsCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv)
							|| dbv.type != DBVT_BYTE
							|| (dbv.bVal != 'M' && SendMessage((HWND)lParam, BM_GETCHECK, NULL, NULL)))
							SendMessage(GetParent(hDlg), PSM_CHANGED, NULL, NULL);
					}
				}
			}
		}
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}
