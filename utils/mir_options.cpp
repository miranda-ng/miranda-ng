/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_system.h>

#include "mir_options.h"

static wchar_t* MyDBGetContactSettingTString(MCONTACT hContact, char* module, char* setting, wchar_t* out, size_t len, wchar_t *def)
{
	DBVARIANT dbv = { 0 };

	out[0] = '\0';

	if (!db_get_ws(hContact, module, setting, &dbv)) {
		mir_wstrncpy(out, dbv.pwszVal, (int)len);
		db_free(&dbv);
	}
	else {
		if (def != nullptr)
			mir_wstrncpy(out, def, (int)len);
	}

	return out;
}


static wchar_t dbPath[MAX_PATH] = { 0 };		// database profile path (read at startup only)


static int PathIsAbsolute(const wchar_t *path)
{
	if (!path || !(mir_wstrlen(path) > 2))
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
		return 1;
	return 0;
}

static void PathToRelative(wchar_t *pOut, size_t outSize, const wchar_t *pSrc)
{
	if (!PathIsAbsolute(pSrc))
		mir_wstrncpy(pOut, pSrc, (int)outSize);
	else {
		if (dbPath[0] == '\0') {
			char tmp[1024];
			Profile_GetPathA(_countof(tmp), tmp);
			mir_snwprintf(dbPath, L"%S\\", tmp);
		}

		size_t len = mir_wstrlen(dbPath);
		if (!wcsnicmp(pSrc, dbPath, len))
			len = 0;
		mir_wstrncpy(pOut, pSrc + len, outSize);
	}
}

static void PathToAbsolute(wchar_t *pOut, size_t outSize, const wchar_t *pSrc)
{
	if (PathIsAbsolute(pSrc) || !isalnum(pSrc[0]))
		mir_wstrncpy(pOut, pSrc, (int)outSize);
	else {
		if (dbPath[0] == '\0') {
			char tmp[1024];
			Profile_GetPathA(_countof(tmp), tmp);
			mir_snwprintf(dbPath, L"%S\\", tmp);
		}

		mir_snwprintf(pOut, outSize, L"%s%s", dbPath, pSrc);
	}
}

static void LoadOpt(OptPageControl *ctrl, char *module)
{
	if (ctrl->var == nullptr)
		return;

	wchar_t tmp[1024];
	switch (ctrl->type) {
	case CONTROL_CHECKBOX:
		*((uint8_t *)ctrl->var) = db_get_b(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_SPIN:
		*((uint16_t *)ctrl->var) = db_get_w(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_COLOR:
		*((COLORREF *)ctrl->var) = (COLORREF)db_get_dw(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_RADIO:
		*((uint16_t *)ctrl->var) = db_get_w(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_COMBO:
		*((uint16_t *)ctrl->var) = db_get_w(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_PROTOCOL_LIST:
		break;

	case CONTROL_TEXT:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((wchar_t *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == nullptr ? nullptr : TranslateW_LP(ctrl->tszDefValue));
		break;

	case CONTROL_INT:
		*((int *)ctrl->var) = (int)db_get_dw(0, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_FILE:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == nullptr ? nullptr : ctrl->tszDefValue);
		PathToAbsolute(((wchar_t *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), tmp);
		break;

	case CONTROL_COMBO_TEXT:
	case CONTROL_COMBO_ITEMDATA:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((wchar_t *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == nullptr ? nullptr : TranslateW_LP(ctrl->tszDefValue));
		break;
	}
}

void LoadOpts(OptPageControl *controls, int controlsSize, char *module)
{
	for (int i = 0; i < controlsSize; i++)
		LoadOpt(&controls[i], module);
}

INT_PTR CALLBACK SaveOptsDlgProc(OptPageControl *controls, int controlsSize, char *module, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t tmp[1024];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialog_LP(hwndDlg, 0);

		for (int i = 0; i < controlsSize; i++) {
			OptPageControl *ctrl = &controls[i];

			if (GetDlgItem(hwndDlg, ctrl->nID) == nullptr)
				continue;

			switch (ctrl->type) {
			case CONTROL_CHECKBOX:
				CheckDlgButton(hwndDlg, ctrl->nID, db_get_b(0, module, ctrl->setting, ctrl->dwDefValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
				break;

			case CONTROL_SPIN:
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID), 0);
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS, 0, MAKELONG(db_get_w(0, module, ctrl->setting, ctrl->dwDefValue), 0));
				break;

			case CONTROL_COLOR:
				SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF)db_get_dw(0, module, ctrl->setting, ctrl->dwDefValue));
				break;

			case CONTROL_RADIO:
				CheckDlgButton(hwndDlg, ctrl->nID, db_get_w(0, module, ctrl->setting, ctrl->dwDefValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
				break;

			case CONTROL_COMBO:
				SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, db_get_w(0, module, ctrl->setting, ctrl->dwDefValue), 0);
				break;

			case CONTROL_PROTOCOL_LIST:
				{
					// Fill list view
					HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
					LVCOLUMN lvc;
					LVITEM lvi;

					ListView_SetExtendedListViewStyle(hwndProtocols, LVS_EX_CHECKBOXES);

					memset(&lvc, 0, sizeof(lvc));
					lvc.mask = LVCF_FMT;
					lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
					ListView_InsertColumn(hwndProtocols, 0, &lvc);

					memset(&lvi, 0, sizeof(lvi));
					lvi.mask = LVIF_TEXT | LVIF_PARAM;
					lvi.iSubItem = 0;
					lvi.iItem = 1000;

					for (auto &pa : Accounts()) {
						if (pa->szModuleName == nullptr || pa->szModuleName[0] == '\0')
							continue;

						if (ctrl->allowProtocol != nullptr && !ctrl->allowProtocol(pa->szModuleName))
							continue;

						char *setting = (char *)mir_alloc(128 * sizeof(char));
						mir_snprintf(setting, 128, ctrl->setting, pa->szModuleName);

						BOOL show = (BOOL)db_get_b(0, module, setting, ctrl->dwDefValue);

						lvi.lParam = (LPARAM)setting;
						lvi.pszText = pa->tszAccountName;
						lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
						ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK(show ? 2 : 1), LVIS_STATEIMAGEMASK);
					}

					ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
					ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);
				}
				break;

			case CONTROL_TEXT:
				SetDlgItemText(hwndDlg, ctrl->nID, MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == nullptr ? nullptr : TranslateW_LP(ctrl->tszDefValue)));
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
				break;

			case CONTROL_INT:
				SetDlgItemInt(hwndDlg, ctrl->nID, db_get_dw(0, module, ctrl->setting, ctrl->dwDefValue), ctrl->min <= 0);
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, 9, 0);
				break;

			case CONTROL_FILE:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == nullptr ? nullptr : ctrl->tszDefValue);
				{
					wchar_t abs[1024];
					PathToAbsolute(abs, 1024, tmp);
					SetDlgItemText(hwndDlg, ctrl->nID, abs);
				}
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
				break;

			case CONTROL_COMBO_TEXT:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == nullptr ? nullptr : TranslateW_LP(ctrl->tszDefValue));
				SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SELECTSTRING, 0, (WPARAM)tmp);
				break;

			case CONTROL_COMBO_ITEMDATA:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == nullptr ? nullptr : TranslateW_LP(ctrl->tszDefValue));
				{
					int count = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCOUNT, 0, 0);
					int k;
					for (k = 0; k < count; k++) {
						wchar_t *id = (wchar_t *)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM)k, 0);
						if (mir_wstrcmp(id, tmp) == 0)
							break;
					}
					if (k < count)
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, k, 0);
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		for (int i = 0; i < controlsSize; i++) {
			OptPageControl *ctrl = &controls[i];

			if (LOWORD(wParam) == ctrl->nID) {
				switch (ctrl->type) {
				case CONTROL_TEXT:
				case CONTROL_SPIN:
				case CONTROL_INT:
					// Don't make apply enabled during buddy set
					if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
						return 0;

					break;

				case CONTROL_COMBO_ITEMDATA:
				case CONTROL_COMBO_TEXT:
				case CONTROL_COMBO:
					if (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus())
						return 0;
					break;
				}

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY) {
				for (int i = 0; i < controlsSize; i++) {
					OptPageControl *ctrl = &controls[i];

					if (GetDlgItem(hwndDlg, ctrl->nID) == nullptr)
						continue;

					switch (ctrl->type) {
					case CONTROL_CHECKBOX:
						db_set_b(0, module, ctrl->setting, (uint8_t)IsDlgButtonChecked(hwndDlg, ctrl->nID));
						break;

					case CONTROL_SPIN:
						db_set_w(0, module, ctrl->setting, (uint16_t)SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
						break;

					case CONTROL_COLOR:
						db_set_dw(0, module, ctrl->setting, (uint32_t)SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
						break;

					case CONTROL_RADIO:
						if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
							db_set_w(0, module, ctrl->setting, (uint8_t)ctrl->value);
						break;

					case CONTROL_COMBO:
						db_set_w(0, module, ctrl->setting, (uint16_t)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0));
						break;

					case CONTROL_PROTOCOL_LIST:
						{
							LVITEM lvi = { 0 };
							lvi.mask = (UINT)LVIF_PARAM;

							HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
							int count = ListView_GetItemCount(hwndProtocols);
							for (int k = 0; k < count; k++) {
								lvi.iItem = k;
								ListView_GetItem(hwndProtocols, &lvi);

								char *setting = (char *)lvi.lParam;
								db_set_b(0, module, setting, (uint8_t)ListView_GetCheckState(hwndProtocols, k));
							}
						}
						break;

					case CONTROL_TEXT:
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, _countof(tmp));
						db_set_ws(0, module, ctrl->setting, tmp);
						break;

					case CONTROL_INT:
						BOOL trans;
						{
							int val = GetDlgItemInt(hwndDlg, ctrl->nID, &trans, ctrl->min <= 0);
							if (!trans)
								val = ctrl->dwDefValue;
							if (ctrl->max != 0)
								val = min(val, ctrl->max);
							if (ctrl->min != 0)
								val = max(val, ctrl->min);
							db_set_dw(0, module, ctrl->setting, val);
						}
						break;

					case CONTROL_FILE:
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, _countof(tmp));
						{
							wchar_t rel[1024];
							PathToRelative(rel, 1024, tmp);
							db_set_ws(0, module, ctrl->setting, rel);
						}
						break;

					case CONTROL_COMBO_TEXT:
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, _countof(tmp));
						db_set_ws(0, module, ctrl->setting, tmp);
						break;

					case CONTROL_COMBO_ITEMDATA:
						int sel = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0);
						db_set_ws(0, module, ctrl->setting, (wchar_t *)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM)sel, 0));
						break;
					}

					LoadOpt(ctrl, module);
				}

				return TRUE;
			}
			else if (lpnmhdr->idFrom != 0 && lpnmhdr->code == LVN_ITEMCHANGED) {
				// Changed for protocols
				for (int i = 0; i < controlsSize; i++) {
					OptPageControl *ctrl = &controls[i];

					if (ctrl->type == CONTROL_PROTOCOL_LIST && ctrl->nID == lpnmhdr->idFrom) {
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

						if (IsWindowVisible(GetDlgItem(hwndDlg, ctrl->nID)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

						break;
					}
				}
			}
		}
		break;

	case WM_DESTROY:
		for (int i = 0; i < controlsSize; i++) {
			OptPageControl *ctrl = &controls[i];

			if (GetDlgItem(hwndDlg, ctrl->nID) == nullptr)
				continue;

			switch (ctrl->type) {
			case CONTROL_PROTOCOL_LIST:
				LVITEM lvi = { 0 };
				lvi.mask = (UINT)LVIF_PARAM;

				HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
				int count = ListView_GetItemCount(hwndProtocols);
				for (i = 0; i < count; i++) {
					lvi.iItem = i;
					ListView_GetItem(hwndProtocols, &lvi);
					mir_free((char *)lvi.lParam);
				}
				break;
			}
		}
		break;
	}

	return 0;
}
