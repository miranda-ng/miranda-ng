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

#define MIRANDA_VER 0x0A00
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_system.h>

#include "mir_options.h"
#include "mir_memory.h"


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


static TCHAR* MyDBGetContactSettingTString(HANDLE hContact, char* module, char* setting, TCHAR* out, size_t len, TCHAR *def)
{
	DBVARIANT dbv = {0};

	out[0] = _T('\0');

	if (!DBGetContactSettingTString(hContact, module, setting, &dbv))
	{
#ifdef UNICODE
		if (dbv.type == DBVT_ASCIIZ)
		{
			MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, out, (int)len);
		}
		else if (dbv.type == DBVT_UTF8)
		{
			MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, out, (int)len);
		}
		else if (dbv.type == DBVT_WCHAR)
		{
			lstrcpyn(out, dbv.pwszVal, (int)len);
		}
#else
		if (dbv.type == DBVT_ASCIIZ)
		{
			lstrcpyn(out, dbv.pszVal, len);
		}
#endif
		else
		{
			if (def != NULL)
				lstrcpyn(out, def, (int)len);
		}
		
		DBFreeVariant(&dbv);
	}
	else
	{
		if (def != NULL)
			lstrcpyn(out, def, (int)len);
	}

	return out;
}


static TCHAR dbPath[MAX_PATH] = {0};		// database profile path (read at startup only)


static int PathIsAbsolute(const TCHAR *path)
{
    if (!path || !(lstrlen(path) > 2))
        return 0;
    if ((path[1]==_T(':') && path[2]==_T('\\')) || (path[0]==_T('\\')&&path[1]==_T('\\'))) 
		return 1;
    return 0;
}

static void PathToRelative(TCHAR *pOut, size_t outSize, const TCHAR *pSrc)
{
    if (!PathIsAbsolute(pSrc)) 
	{
		lstrcpyn(pOut, pSrc, (int)outSize);
    }
    else 
	{
		if (dbPath[0] == _T('\0'))
		{
			char tmp[1024];
		    CallService(MS_DB_GETPROFILEPATH, MAX_REGS(tmp), (LPARAM) tmp);
			mir_sntprintf(dbPath, MAX_REGS(dbPath), _T(TCHAR_STR_PARAM) _T("\\"), tmp);
		}

		size_t len = lstrlen(dbPath);
        if (_tcsnicmp(pSrc, dbPath, len)) 
		{
            mir_sntprintf(pOut, outSize, _T("%s"), pSrc + len);
        }
        else 
		{
            lstrcpyn(pOut, pSrc, (int)outSize);
        }
    }
}

static void PathToAbsolute(TCHAR *pOut, size_t outSize, const TCHAR *pSrc)
{
    if (PathIsAbsolute(pSrc) || !isalnum(pSrc[0])) 
	{
        lstrcpyn(pOut, pSrc, (int)outSize);
    }
    else 
	{
		if (dbPath[0] == _T('\0'))
		{
			char tmp[1024];
		    CallService(MS_DB_GETPROFILEPATH, MAX_REGS(tmp), (LPARAM) tmp);
			mir_sntprintf(dbPath, MAX_REGS(dbPath), _T(TCHAR_STR_PARAM) _T("\\"), tmp);
		}

        mir_sntprintf(pOut, outSize, _T("%s%s"), dbPath, pSrc);
    }
}


static void LoadOpt(OptPageControl *ctrl, char *module)
{
	if (ctrl->var == NULL)
		return;
	
	switch(ctrl->type)
	{
		case CONTROL_CHECKBOX:
		{
			*((BYTE *) ctrl->var) = DBGetContactSettingByte(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_SPIN:
		{
			*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_COLOR:
		{
			*((COLORREF *) ctrl->var) = (COLORREF) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_RADIO:
		{
			*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_COMBO:
		{
			*((WORD *) ctrl->var) = DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_PROTOCOL_LIST:
		{
			break;
		}
		case CONTROL_TEXT:
		{
			MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *) ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
			break;
		}
		case CONTROL_PASSWORD:
		{
			char tmp[1024];
			tmp[0]=0;

			DBVARIANT dbv = {0};
			if (!DBGetContactSettingString(NULL, module, ctrl->setting, &dbv))
			{
				lstrcpynA(tmp, dbv.pszVal, MAX_REGS(tmp));
				DBFreeVariant(&dbv);
			}

			if (tmp[0] != 0)
				CallService(MS_DB_CRYPT_DECODESTRING, MAX_REGS(tmp), (LPARAM) tmp);
			else if (ctrl->szDefValue != NULL)
				lstrcpynA(tmp, ctrl->szDefValue, MAX_REGS(tmp));

			char *var = (char *) ctrl->var;
			int size = min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024);
			lstrcpynA(var, tmp, size);
			break;
		}
		case CONTROL_INT:
		{
			*((int *) ctrl->var) = (int) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue);
			break;
		}
		case CONTROL_FILE:
		{
			TCHAR tmp[1024];
			MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : ctrl->tszDefValue);
			PathToAbsolute(((TCHAR *) ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), tmp);
			break;
		}
		case CONTROL_COMBO_TEXT:
		case CONTROL_COMBO_ITEMDATA:
		{
			MyDBGetContactSettingTString(NULL, module, ctrl->setting, ((TCHAR *) ctrl->var), min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
			break;
		}
	}
}

void LoadOpts(OptPageControl *controls, int controlsSize, char *module)
{
	for (int i = 0 ; i < controlsSize ; i++)
	{
		LoadOpt(&controls[i], module);
	}
}



INT_PTR CALLBACK SaveOptsDlgProc(OptPageControl *controls, int controlsSize, char *module, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
					continue;

				switch(ctrl->type)
				{
					case CONTROL_CHECKBOX:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingByte(NULL, module, ctrl->setting, ctrl->dwDefValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_SPIN:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID),0);
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS,0, MAKELONG(DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue), 0));
						break;
					}
					case CONTROL_COLOR:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF) DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue));
						break;
					}
					case CONTROL_RADIO:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_COMBO:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, DBGetContactSettingWord(NULL, module, ctrl->setting, ctrl->dwDefValue), 0);
						break;
					}
					case CONTROL_PROTOCOL_LIST:
					{
						// Fill list view
						HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
						LVCOLUMN lvc;
						LVITEM lvi;
						
						ListView_SetExtendedListViewStyle(hwndProtocols, LVS_EX_CHECKBOXES);
						
						ZeroMemory(&lvc, sizeof(lvc));
						lvc.mask = LVCF_FMT;
						lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
						ListView_InsertColumn(hwndProtocols, 0, &lvc);
						
						ZeroMemory(&lvi, sizeof(lvi));
						lvi.mask = LVIF_TEXT | LVIF_PARAM;
						lvi.iSubItem = 0;
						lvi.iItem = 1000;
						
						PROTOACCOUNT **protos;
						int count;

						BOOL hasAccounts = ServiceExists(MS_PROTO_ENUMACCOUNTS);

						if (hasAccounts)
							CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&protos);
						else
							CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
						
						for (int i = 0; i < count; i++)
						{
							if (protos[i]->type != PROTOTYPE_PROTOCOL)
								continue;

							if (protos[i]->szModuleName == NULL || protos[i]->szModuleName[0] == '\0')
								continue;
							
							if (ctrl->allowProtocol != NULL && !ctrl->allowProtocol(protos[i]->szModuleName))
								continue;

							TCHAR *name;
							if (hasAccounts)
							{
								name = mir_tstrdup(protos[i]->tszAccountName);
							}
							else
							{
								char szName[128];
								CallProtoService(protos[i]->szModuleName, PS_GETNAME, sizeof(szName), (LPARAM)szName);
								name = mir_a2t(szName);
							}
							
							char *setting = (char *) mir_alloc(128 * sizeof(char));
							mir_snprintf(setting, 128, ctrl->setting, protos[i]->szModuleName);

							BOOL show = (BOOL)DBGetContactSettingByte(NULL, module, setting, ctrl->dwDefValue);
							
							lvi.lParam = (LPARAM)setting;
							lvi.pszText = TranslateTS(name);
							lvi.iItem = ListView_InsertItem(hwndProtocols, &lvi);
							ListView_SetItemState(hwndProtocols, lvi.iItem, INDEXTOSTATEIMAGEMASK(show?2:1), LVIS_STATEIMAGEMASK);

							mir_free(name);
						}
						
						ListView_SetColumnWidth(hwndProtocols, 0, LVSCW_AUTOSIZE);
						ListView_Arrange(hwndProtocols, LVA_ALIGNLEFT | LVA_ALIGNTOP);
						break;
					}
					case CONTROL_TEXT:
					{
						TCHAR tmp[1024];
						SetDlgItemText(hwndDlg, ctrl->nID, MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue)));
						SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
						break;
					}
					case CONTROL_PASSWORD:
					{
						char tmp[1024];
						tmp[0]=0;

						DBVARIANT dbv = {0};
						if (!DBGetContactSettingString(NULL, module, ctrl->setting, &dbv))
						{
							lstrcpynA(tmp, dbv.pszVal, MAX_REGS(tmp));
							DBFreeVariant(&dbv);
						}

						if (tmp[0] != 0)
							CallService(MS_DB_CRYPT_DECODESTRING, MAX_REGS(tmp), (LPARAM) tmp);
						else if (ctrl->szDefValue != NULL)
							lstrcpynA(tmp, ctrl->szDefValue, MAX_REGS(tmp));

						SetDlgItemTextA(hwndDlg, ctrl->nID, tmp);
						SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
						break;
					}
					case CONTROL_INT:
					{
						DWORD var = DBGetContactSettingDword(NULL, module, ctrl->setting, ctrl->dwDefValue);
						SetDlgItemInt(hwndDlg, ctrl->nID, var, ctrl->min <= 0);
						SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, 9, 0);
						break;
					}
					case CONTROL_FILE:
					{
						TCHAR tmp[1024];
						MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : ctrl->tszDefValue);
						TCHAR abs[1024];
						PathToAbsolute(abs, 1024, tmp);

						SetDlgItemText(hwndDlg, ctrl->nID, abs);
						SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024), 0);
						break;
					}
					case CONTROL_COMBO_TEXT:
					{
						TCHAR tmp[1024];
						MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
						SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SELECTSTRING, 0, (WPARAM) tmp);
						break;
					}
					case CONTROL_COMBO_ITEMDATA:
					{
						TCHAR tmp[1024];
						MyDBGetContactSettingTString(NULL, module, ctrl->setting, tmp, 1024, ctrl->tszDefValue == NULL ? NULL : TranslateTS(ctrl->tszDefValue));
						int count = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCOUNT, 0, 0);
						int i;
						for(i = 0; i < count; i++)
						{
							TCHAR *id = (TCHAR *) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM) i, 0);
							if (lstrcmp(id, tmp) == 0)
								break;
						}
						if (i < count)
							SendDlgItemMessage(hwndDlg, ctrl->nID, CB_SETCURSEL, i, 0);
						break;
					}
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				if (LOWORD(wParam) == ctrl->nID)
				{
					switch(ctrl->type)
					{
						case CONTROL_TEXT:
						case CONTROL_SPIN:
						case CONTROL_INT:
						case CONTROL_PASSWORD:
						{
							// Don't make apply enabled during buddy set
							if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
								return 0;

							break;
						}
						case CONTROL_COMBO_ITEMDATA:
						case CONTROL_COMBO_TEXT:
						case CONTROL_COMBO:
						{
							if (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus())
								return 0;

							break;
						}
					}

					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;

			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY)
			{
				for (int i = 0 ; i < controlsSize ; i++)
				{
					OptPageControl *ctrl = &controls[i];

					if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
						continue;

					switch(ctrl->type)
					{
						case CONTROL_CHECKBOX:
						{
							DBWriteContactSettingByte(NULL, module, ctrl->setting, (BYTE) IsDlgButtonChecked(hwndDlg, ctrl->nID));
							break;
						}
						case CONTROL_SPIN:
						{
							DBWriteContactSettingWord(NULL, module, ctrl->setting, (WORD) SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
							break;
						}
						case CONTROL_COLOR:
						{
							DBWriteContactSettingDword(NULL, module, ctrl->setting, (DWORD) SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
							break;
						}
						case CONTROL_RADIO:
						{
							if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
								DBWriteContactSettingWord(NULL, module, ctrl->setting, (BYTE) ctrl->value);
							break;
						}
						case CONTROL_COMBO:
						{
							DBWriteContactSettingWord(NULL, module, ctrl->setting, (WORD) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0));
							break;
						}
						case CONTROL_PROTOCOL_LIST:
						{
							LVITEM lvi = {0};
							HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
							int i;
							
							lvi.mask = (UINT) LVIF_PARAM;
							
							for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
							{
								lvi.iItem = i;
								ListView_GetItem(hwndProtocols, &lvi);
								
								char *setting = (char *)lvi.lParam;
								DBWriteContactSettingByte(NULL, module, setting, (BYTE)ListView_GetCheckState(hwndProtocols, i));
							}
							break;
						}
						case CONTROL_TEXT:
						{
							TCHAR tmp[1024];
							GetDlgItemText(hwndDlg, ctrl->nID, tmp, MAX_REGS(tmp));
							DBWriteContactSettingTString(NULL, module, ctrl->setting, tmp);
							break;
						}
						case CONTROL_PASSWORD:
						{
							char tmp[1024];
							GetDlgItemTextA(hwndDlg, ctrl->nID, tmp, MAX_REGS(tmp));

							if (ctrl->var != NULL)
							{
								char *var = (char *) ctrl->var;
								int size = min(ctrl->max <= 0 ? 1024 : ctrl->max, 1024);
								lstrcpynA(var, tmp, size);
							}

							if (ctrl->checkboxID != 0 && !IsDlgButtonChecked(hwndDlg, ctrl->checkboxID))
							{
								DBDeleteContactSetting(NULL, module, ctrl->setting);
								continue;
							}

							CallService(MS_DB_CRYPT_ENCODESTRING, MAX_REGS(tmp), (LPARAM) tmp);
							DBWriteContactSettingString(NULL, module, ctrl->setting, tmp);

							// Don't load from DB
							continue;
						}
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
							DBWriteContactSettingDword(NULL, module, ctrl->setting, val);
							break;
						}
						case CONTROL_FILE:
						{
							TCHAR tmp[1024];
							GetDlgItemText(hwndDlg, ctrl->nID, tmp, 1024);
							TCHAR rel[1024];
							PathToRelative(rel, 1024, tmp);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, rel);
							break;
						}
						case CONTROL_COMBO_TEXT:
						{
							TCHAR tmp[1024];
							GetDlgItemText(hwndDlg, ctrl->nID, tmp, 1024);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, tmp);
							break;
						}
						case CONTROL_COMBO_ITEMDATA:
						{
							int sel = SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETCURSEL, 0, 0);
							DBWriteContactSettingTString(NULL, module, ctrl->setting, (TCHAR *) SendDlgItemMessage(hwndDlg, ctrl->nID, CB_GETITEMDATA, (WPARAM) sel, 0));
							break;
						}
					}

					LoadOpt(ctrl, module);
				}
				
				return TRUE;
			}
			else if (lpnmhdr->idFrom != 0 && lpnmhdr->code == LVN_ITEMCHANGED)
			{
				// Changed for protocols
				for (int i = 0 ; i < controlsSize ; i++)
				{
					OptPageControl *ctrl = &controls[i];

					if (ctrl->type == CONTROL_PROTOCOL_LIST && ctrl->nID == lpnmhdr->idFrom)
					{
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
						
						if(IsWindowVisible(GetDlgItem(hwndDlg, ctrl->nID)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

						break;
					}
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				if (GetDlgItem(hwndDlg, ctrl->nID) == NULL)
					continue;

				switch(ctrl->type)
				{
					case CONTROL_PROTOCOL_LIST:
					{
						LVITEM lvi = {0};
						HWND hwndProtocols = GetDlgItem(hwndDlg, ctrl->nID);
						int i;
						
						lvi.mask = (UINT) LVIF_PARAM;
						
						for (i = 0; i < ListView_GetItemCount(hwndProtocols); i++)
						{
							lvi.iItem = i;
							ListView_GetItem(hwndProtocols, &lvi);
							mir_free((char *) lvi.lParam);
						}
						break;
					}
				}
			}
			break;
		}
	}

	return 0;
}
