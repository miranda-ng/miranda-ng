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
#include <tchar.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_system.h>

#include "mir_options.h"
#include "mir_memory.h"

static TCHAR* MyDBGetContactSettingTString(MCONTACT hContact, char* module, char* setting, TCHAR* out, size_t len, TCHAR *def)
{
	DBVARIANT dbv = { 0 };

	out[0] = _T('\0');

	if (!db_get_ts(hContact, module, setting, &dbv)) {
		mir_tstrncpy(out, dbv.ptszVal, (int)len);
		db_free(&dbv);
	}
	else {
		if (def != NULL)
			mir_tstrncpy(out, def, (int)len);
	}

	return out;
}


static TCHAR dbPath[MAX_PATH] = { 0 };		// database profile path (read at startup only)


static int PathIsAbsolute(const TCHAR *path)
{
	if (!path || !(mir_tstrlen(path) > 2))
		return 0;
	if ((path[1] == _T(':') && path[2] == _T('\\')) || (path[0] == _T('\\') && path[1] == _T('\\')))
		return 1;
	return 0;
}

static void PathToRelative(TCHAR *pOut, size_t outSize, const TCHAR *pSrc)
{
	if (!PathIsAbsolute(pSrc))
		mir_tstrncpy(pOut, pSrc, (int)outSize);
	else {
		if (dbPath[0] == _T('\0')) {
			char tmp[1024];
			CallService(MS_DB_GETPROFILEPATH, SIZEOF(tmp), (LPARAM)tmp);
			mir_sntprintf(dbPath, SIZEOF(dbPath), _T("%S\\"), tmp);
		}

		size_t len = mir_tstrlen(dbPath);
		if (_tcsnicmp(pSrc, dbPath, len))
			mir_sntprintf(pOut, outSize, _T("%s"), pSrc + len);
		else
			mir_tstrncpy(pOut, pSrc, (int)outSize);
	}
}

static void PathToAbsolute(TCHAR *pOut, size_t outSize, const TCHAR *pSrc)
{
	if (PathIsAbsolute(pSrc) || !isalnum(pSrc[0]))
		mir_tstrncpy(pOut, pSrc, (int)outSize);
	else {
		if (dbPath[0] == _T('\0')) {
			char tmp[1024];
			CallService(MS_DB_GETPROFILEPATH, SIZEOF(tmp), (LPARAM)tmp);
			mir_sntprintf(dbPath, SIZEOF(dbPath), _T("%S\\"), tmp);
		}

		mir_sntprintf(pOut, outSize, _T("%s%s"), dbPath, pSrc);
	}
}

static void LoadOpt(OptPageControl *ctrl, char *module)
{
	if (ctrl->var == NULL)
		return;

	TCHAR tmp[1024];
	switch (ctrl->type) {
	case CONTROL_CHECKBOX:
		*((BYTE *)ctrl->var) = db_get_b(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_SPIN:
		*((WORD *)ctrl->var) = db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_COLOR:
		*((COLORREF *)ctrl->var) = (COLORREF)db_get_dw(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_RADIO:
		*((WORD *)ctrl->var) = db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_COMBO:
		*((WORD *)ctrl->var) = db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_PROTOCOL_LIST:
		break;

	case CONTROL_TEXT:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
		break;

	case CONTROL_INT:
		*((int *)ctrl->var) = (int)db_get_dw(NULL, module, ctrl->setting, ctrl->dwDefValue);
		break;

	case CONTROL_FILE:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : ctrl->tszDefValue);
		PathToAbsolute(((TCHAR *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), tmp);
		break;

	case CONTROL_COMBO_TEXT:
	case CONTROL_COMBO_ITEMDATA:
		MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *)ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
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
	TCHAR tmp[1024];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		for (int i = 0; i < controlsSize; i++) {
			OptPageControl *ctrl = &controls[i];

			if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
				continue;

			switch (ctrl->type) {
			case CONTROL_CHECKBOX:
				CheckDlgButton(hwndDlg, ctrl->nID, db_get_b(NULL, module, ctrl->setting, ctrl->dwDefValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
				break;

			case CONTROL_SPIN:
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID), 0);
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
				SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue), 0));
				break;

			case CONTROL_COLOR:
				SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF)db_get_dw(NULL, module, ctrl->setting, ctrl->dwDefValue));
				break;

			case CONTROL_RADIO:
				CheckDlgButton(hwndDlg, ctrl->nID, db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
				break;

			case CONTROL_COMBO:
				SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, db_get_w(NULL, module, ctrl->setting, ctrl->dwDefValue), 0);
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

					int count;
					PROTOACCOUNT **protos;
					ProtoEnumAccounts(&count, &protos);

					for (int i = 0; i < count; i++) {
						PROTOACCOUNT *p = protos[i];
						if (p->szModuleName == NULL || p->szModuleName[0] == '\0')
							continue;

						if (ctrl->allowProtocol != NULL && !ctrl->allowProtocol(p->szModuleName))
							continue;

						char *setting = (char *)mir_alloc(128 * sizeof(char));
						mir_snprintf(setting, 128, ctrl->setting, p->szModuleName);

						BOOL show = (BOOL)db_get_b(NULL, module, setting, ctrl->dwDefValue);

						lvi.lParam = (LPARAM)setting;
						lvi.pszText = p->tszAccountName;
						lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
						ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK(show ? 2 : 1), LVIS_STATEIMAGEMASK);
					}

					ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
					ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);
				}
				break;

			case CONTROL_TEXT:
				SetDlgItemText(hwndDlg, ctrl->nID, MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue)));
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
				break;

			case CONTROL_INT:
				SetDlgItemInt(hwndDlg, ctrl->nID, db_get_dw(NULL, module, ctrl->setting, ctrl->dwDefValue), ctrl->min <= 0);
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, 9, 0);
				break;

			case CONTROL_FILE:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : ctrl->tszDefValue);
				{
					TCHAR abs[1024];
					PathToAbsolute(abs, 1024, tmp);
					SetDlgItemText(hwndDlg, ctrl->nID, abs);
				}
				SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
				break;

			case CONTROL_COMBO_TEXT:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
				SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SELECTSTRING, 0, (WPARAM)tmp);
				break;

			case CONTROL_COMBO_ITEMDATA:
				MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
				{
					int count = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCOUNT, 0, 0);
					int i;
					for (i = 0; i < count; i++) {
						TCHAR *id = (TCHAR *)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM)i, 0);
						if (mir_tstrcmp(id, tmp) == 0)
							break;
					}
					if (i < count)
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, i, 0);
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

					if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
						continue;

					switch (ctrl->type) {
					case CONTROL_CHECKBOX:
						db_set_b(NULL, module, ctrl->setting, (BYTE)IsDlgButtonChecked(hwndDlg, ctrl->nID));
						break;

					case CONTROL_SPIN:
						db_set_w(NULL, module, ctrl->setting, (WORD)SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
						break;

					case CONTROL_COLOR:
						db_set_dw(NULL, module, ctrl->setting, (DWORD)SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
						break;

					case CONTROL_RADIO:
						if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
							db_set_w(NULL, module, ctrl->setting, (BYTE)ctrl->value);
						break;

					case CONTROL_COMBO:
						db_set_w(NULL, module, ctrl->setting, (WORD)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0));
						break;

					case CONTROL_PROTOCOL_LIST:
					{
						LVITEM lvi = { 0 };
						HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
						int i;

						lvi.mask = (UINT)LVIF_PARAM;

						for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++) {
							lvi.iItem = i;
							ListView_GetItem(hwndProtocols, &lvi);

							char *setting = (char *)lvi.lParam;
							db_set_b(NULL, module, setting, (BYTE)ListView_GetCheckState(hwndProtocols, i));
						}
					}
						break;

					case CONTROL_TEXT:
					{
						TCHAR tmp[1024];
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, SIZEOF(tmp));
						db_set_ts(NULL, module, ctrl->setting, tmp);
					}
						break;

					case CONTROL_INT:
					{
						BOOL trans;
						int val = GetDlgItemInt(hwndDlg, ctrl->nID, &trans, ctrl->min <= 0);
						if (!trans)
							val = ctrl->dwDefValue;
						if (ctrl->max != 0)
							val = min(val, ctrl->max);
						if (ctrl->min != 0)
							val = max(val, ctrl->min);
						db_set_dw(NULL, module, ctrl->setting, val);
					}
						break;
					case CONTROL_FILE:
					{
						TCHAR tmp[1024];
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, SIZEOF(tmp));
						TCHAR rel[1024];
						PathToRelative(rel, 1024, tmp);
						db_set_ts(NULL, module, ctrl->setting, rel);
					}
						break;
					case CONTROL_COMBO_TEXT:
					{
						TCHAR tmp[1024];
						GetDlgItemText(hwndDlg, ctrl->nID, tmp, SIZEOF(tmp));
						db_set_ts(NULL, module, ctrl->setting, tmp);
					}
						break;
					case CONTROL_COMBO_ITEMDATA:
					{
						int sel = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0);
						db_set_ts(NULL, module, ctrl->setting, (TCHAR *)SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM)sel, 0));
					}
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

			if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
				continue;

			switch (ctrl->type) {
			case CONTROL_PROTOCOL_LIST:
				LVITEM lvi = { 0 };
				HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
				int i;

				lvi.mask = (UINT)LVIF_PARAM;

				for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++) {
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
