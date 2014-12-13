/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "commonheaders.h"

static void RebuildStatusMenu(void)
{
	pcli->pfnReloadProtoMenus();
}

static LRESULT CALLBACK OptEditBoxSubProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CONTEXTMENU:
		{
			POINT pt = {(LONG)LOWORD(lParam), (LONG)HIWORD(lParam)};
			RECT rc;
			GetClientRect(hwndDlg, &rc);

			if (pt.x == -1 && pt.y == -1)
			{
				GetCursorPos(&pt);
				if (!PtInRect(&rc, pt))
				{
					pt.x = rc.left + (rc.right - rc.left) / 2;
					pt.y = rc.top + (rc.bottom - rc.top) / 2;
				}
			}
			else
				ScreenToClient(hwndDlg, &pt);

			if (PtInRect(&rc, pt)) 
				HandlePopupMenu(hwndDlg, pt, GetDlgItem(GetParent(hwndDlg), IDC_OPTEDIT1));

			return 0;
		}

	case WM_CHAR:
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000)	// Ctrl + A
		{
			SendMessage(hwndDlg, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000)	// Ctrl + Backspace
		{
			DWORD start, end;
			TCHAR *text;
			int textLen;
			SendMessage(hwndDlg, EM_GETSEL, (WPARAM)&end, (LPARAM)(PDWORD)NULL);
			SendMessage(hwndDlg, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwndDlg, EM_GETSEL, (WPARAM)&start, (LPARAM)(PDWORD)NULL);
			textLen = GetWindowTextLength(hwndDlg);
			text = (TCHAR *)mir_alloc(sizeof(TCHAR) * (textLen + 1));
			GetWindowText(hwndDlg, text, textLen + 1);
			memmove(text + start, text + end, sizeof(TCHAR) * (textLen + 1 - end));
			SetWindowText(hwndDlg, text);
			mir_free(text);
			SendMessage(hwndDlg, EM_SETSEL, start, start);
			SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwndDlg), EN_CHANGE), (LPARAM)hwndDlg);
			return 0;
		}
		break;
	}

	return mir_callNextSubclass(hwndDlg, OptEditBoxSubProc, uMsg, wParam, lParam);
}

struct SingleProtoMsg
{
	int flags;
	TCHAR *msg;
	int max_length;
};

struct SingleStatusMsg
{
	int flags[9];
	TCHAR msg[9][1024];
};

struct OptDlgData
{
	BOOL proto_ok;
	struct SingleProtoMsg *proto_msg;
	struct SingleStatusMsg *status_msg;
};

static INT_PTR CALLBACK DlgOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct OptDlgData *data = (struct OptDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			int	val, i, index;

			TranslateDialogDefault(hwndDlg);

			data = (struct OptDlgData *)mir_alloc(sizeof(struct OptDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);

			SendDlgItemMessage(hwndDlg, IDC_OPTEDIT1, EM_LIMITTEXT, 1024, 0);
			SendDlgItemMessage(hwndDlg, IDC_SMAXLENGTH, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXLENGTH), 0);
			SendDlgItemMessage(hwndDlg, IDC_SMAXLENGTH, UDM_SETRANGE32, 1, 1024);
			SendDlgItemMessage(hwndDlg, IDC_EMAXLENGTH, EM_LIMITTEXT, 4, 0);

			data->status_msg = (struct SingleStatusMsg *)mir_alloc(sizeof(struct SingleStatusMsg)*(accounts->count + 1));

			for (i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
			{
				if (accounts->statusMsgFlags & Proto_Status2Flag(i))
				{
					index = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_INSERTSTRING, -1, (LPARAM)pcli->pfnGetStatusModeDescription(i, 0));
					if (index != CB_ERR && index != CB_ERRSPACE)
					{
						int j;
						char setting[80];

						SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i - ID_STATUS_ONLINE);

						val = db_get_b(NULL, "SimpleStatusMsg", (char *)StatusModeToDbSetting(i, "Flags"), STATUS_DEFAULT);
						data->status_msg[0].flags[i - ID_STATUS_ONLINE] = val;
						ptrT text( db_get_tsa(NULL, "SRAway", StatusModeToDbSetting(i, "Default")));
						mir_tstrncpy(data->status_msg[0].msg[i - ID_STATUS_ONLINE], (text == NULL) ? GetDefaultMessage(i) : text, 1024);

						for (j = 0; j < accounts->count; j++)
						{
							if (!IsAccountEnabled(accounts->pa[j]) || !CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) || !(CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
								continue;

							mir_snprintf(setting, SIZEOF(setting), "%sFlags", accounts->pa[j]->szModuleName);
							val = db_get_b(NULL, "SimpleStatusMsg", (char *)StatusModeToDbSetting(i, setting), STATUS_DEFAULT);
							data->status_msg[j+1].flags[i-ID_STATUS_ONLINE] = val;
							mir_snprintf(setting, SIZEOF(setting), "%sDefault", accounts->pa[j]->szModuleName);
							text = db_get_tsa(NULL, "SRAway", StatusModeToDbSetting(i, setting));
							mir_tstrncpy(data->status_msg[j + 1].msg[i - ID_STATUS_ONLINE], (text == NULL) ? GetDefaultMessage(i) : text, 1024);
						}
					}
				}
			}
			SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_SETCURSEL, 0, 0);

			data->proto_msg = (struct SingleProtoMsg *)mir_alloc(sizeof(struct SingleProtoMsg)*(accounts->count + 1));
			if (!data->proto_msg)
			{
				// TODO not really needed?
				EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTPROTO), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO1), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO2), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO3), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO4), FALSE);
				data->proto_ok = FALSE;
			}
			else
			{
				char setting[64];

				data->proto_ok = TRUE;

				index = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_ADDSTRING, 0, (LPARAM)TranslateT("Global status change"));

				if (index != CB_ERR && index != CB_ERRSPACE)
				{
					data->proto_msg[0].msg = NULL;

					val = db_get_b(NULL, "SimpleStatusMsg", "ProtoFlags", PROTO_DEFAULT);
					data->proto_msg[0].flags = val;
					data->proto_msg[0].max_length = 0;
					SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_SETITEMDATA, (WPARAM)index, 0);
				}

				for (i = 0; i < accounts->count; ++i)
				{
					if (!IsAccountEnabled(accounts->pa[i])
						|| !CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0)
						|| !(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
					{
						data->proto_msg[i+1].msg = NULL;
						continue;
					}

					index = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_ADDSTRING, 0, (LPARAM)accounts->pa[i]->tszAccountName);
					// SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_SETITEMDATA, index, (LPARAM)i + 1);
					if (index != CB_ERR && index != CB_ERRSPACE)
					{
						mir_snprintf(setting, SIZEOF(setting), "Proto%sDefault", accounts->pa[i]->szModuleName);
						data->proto_msg[i+1].msg = db_get_tsa(NULL, "SimpleStatusMsg", setting);

						mir_snprintf(setting, SIZEOF(setting), "Proto%sFlags", accounts->pa[i]->szModuleName);
						val = db_get_b(NULL, "SimpleStatusMsg", setting, PROTO_DEFAULT);
						data->proto_msg[i+1].flags = val;

						mir_snprintf(setting, SIZEOF(setting), "Proto%sMaxLen", accounts->pa[i]->szModuleName);
						val = db_get_w(NULL, "SimpleStatusMsg", setting, 1024);
						data->proto_msg[i+1].max_length = val;
						SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i + 1);
					}
				}

				if (accounts->statusMsgCount == 1)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTPROTO), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTPROTO), FALSE);
					SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_SETCURSEL, 1, 0);
				}
				else SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_SETCURSEL, 0, 0);

				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CBOPTPROTO, CBN_SELCHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_CBOPTPROTO));
			}

			if (db_get_b(NULL, "SimpleStatusMsg", "PutDefInList", 0))
				CheckDlgButton(hwndDlg, IDC_COPTMSG2, BST_CHECKED);

			if (ServiceExists(MS_VARS_FORMATSTRING))
			{
				HICON hIcon=NULL;
				char *szTipInfo=NULL;

				if (ServiceExists(MS_VARS_GETSKINITEM))
				{
					hIcon = (HICON)CallService(MS_VARS_GETSKINITEM, 0, VSI_HELPICON);
					szTipInfo = (char *)CallService(MS_VARS_GETSKINITEM, 0, VSI_HELPTIPTEXT);
				}

				if (hIcon != NULL)
					SendDlgItemMessage(hwndDlg, IDC_VARSHELP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				else
					SetDlgItemText(hwndDlg, IDC_VARSHELP, _T("V"));

				if (szTipInfo == NULL)
					SendDlgItemMessage(hwndDlg, IDC_VARSHELP, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open string formatting help"), 0);
				else
					SendDlgItemMessage(hwndDlg, IDC_VARSHELP, BUTTONADDTOOLTIP, (WPARAM)szTipInfo, 0);

				SendDlgItemMessage(hwndDlg, IDC_VARSHELP, BUTTONSETASFLATBTN, TRUE, 0);
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), ServiceExists(MS_VARS_FORMATSTRING));

			mir_subclassWindow( GetDlgItem(hwndDlg, IDC_OPTEDIT1), OptEditBoxSubProc);
			return TRUE;
		}

	case WM_COMMAND:
		if ( ( (HIWORD(wParam) == BN_CLICKED) || /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
			|| ( (HIWORD(wParam) == CBN_SELCHANGE) && (LOWORD(wParam) != IDC_CBOPTPROTO) && (LOWORD(wParam) != IDC_CBOPTSTATUS))
			) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_EMAXLENGTH:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				{
					BOOL translated;
					int val, i;

					val = GetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, &translated, FALSE);
					if (translated && val > 1024)
						SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1024, FALSE);
					if (translated && val < 1)
						SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1, FALSE);
					val = GetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, &translated, FALSE);

					i = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);
					data->proto_msg[i].max_length = val;
					break;
				}
			}
			break;

		case IDC_CBOPTPROTO:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			case CBN_SELENDOK:
				{
					int i, j, l, k, status_modes = 0, newindex = 0;

					i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);

					if (i == 0)
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_MAXLENGTH), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO3), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO4), FALSE);
						SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, 1024, FALSE);

						if (data->proto_msg[i].flags & PROTO_POPUPDLG)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO1);
						else if (data->proto_msg[i].flags & PROTO_NOCHANGE)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO2);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_MAXLENGTH), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_EMAXLENGTH), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SMAXLENGTH), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO3), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTPROTO4), TRUE);
						SetDlgItemInt(hwndDlg, IDC_EMAXLENGTH, data->proto_msg[i].max_length, FALSE);

						if (data->proto_msg[i].flags & PROTO_POPUPDLG)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO1);
						else if (data->proto_msg[i].flags & PROTO_NOCHANGE)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO2);
						else if (data->proto_msg[i].flags & PROTO_THIS_MSG)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO3);
						else if (data->proto_msg[i].flags & PROTO_NO_MSG)
							CheckRadioButton(hwndDlg, IDC_ROPTPROTO1, IDC_ROPTPROTO4, IDC_ROPTPROTO4);
					}

					if (data->proto_msg[i].flags & PROTO_NO_MSG || data->proto_msg[i].flags & PROTO_THIS_MSG
						|| data->proto_msg[i].flags & PROTO_NOCHANGE)
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);

						if (data->proto_msg[i].flags & PROTO_NO_MSG || data->proto_msg[i].flags & PROTO_NOCHANGE)
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						}
						else
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
						}

						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), TRUE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), TRUE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), TRUE);
					}

					if (i)
					{
						k = i - 1;
						status_modes = CallProtoService(accounts->pa[k]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);
					}
					else
						status_modes = accounts->statusMsgFlags;

					j = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
					SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_RESETCONTENT, 0, 0);

					for (l=ID_STATUS_ONLINE; l<=ID_STATUS_OUTTOLUNCH; l++)
					{
						int	 index;
						if (status_modes & Proto_Status2Flag(l))
						{
							index = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_INSERTSTRING, -1, (LPARAM)pcli->pfnGetStatusModeDescription(l, 0));
							if (index != CB_ERR && index != CB_ERRSPACE)
							{
								SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_SETITEMDATA, (WPARAM)index, (LPARAM)l - ID_STATUS_ONLINE);
								if (j == l-ID_STATUS_ONLINE)
									newindex=index;
							}
						}
					}

					if (!newindex)
					{
						SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_SETCURSEL, 0, 0);
						j = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
					}
					else
						SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_SETCURSEL, (WPARAM)newindex, 0);

					if (data->status_msg[i].flags[j] & STATUS_SHOW_DLG)
						CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_CHECKED);
					else
						CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_UNCHECKED);

					if (data->proto_msg[i].flags & PROTO_THIS_MSG)
					{
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
						if (data->proto_msg[i].msg)
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->proto_msg[i].msg);
						else
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
					}
					else
					{
						if (data->status_msg[i].flags[j] & STATUS_EMPTY_MSG)
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
						}
						else if (data->status_msg[i].flags[j] & STATUS_DEFAULT_MSG)
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(j+ID_STATUS_ONLINE));
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
						}
						else if (data->status_msg[i].flags[j] & STATUS_LAST_MSG)
						{
							char setting[80];
							DBVARIANT dbv, dbv2;

							if (i)
								mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", accounts->pa[k]->szModuleName);
							else
								mir_snprintf(setting, SIZEOF(setting), "LastMsg");

							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
							if (!db_get(NULL, "SimpleStatusMsg", setting, &dbv))
							{
								if (dbv.pszVal)
								{
									if (!db_get_ts(NULL, "SimpleStatusMsg", dbv.pszVal, &dbv2) && strlen(dbv.pszVal))
									{
										if ((dbv2.ptszVal) && (mir_tstrlen(dbv2.ptszVal)))
											SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.ptszVal);

										db_free(&dbv2);
									}
								}
								db_free(&dbv);
							}
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
						}
						else if (data->status_msg[i].flags[j] & STATUS_THIS_MSG)
						{
							if (data->proto_msg[i].flags & PROTO_NO_MSG || data->proto_msg[i].flags & PROTO_NOCHANGE)
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							}
							else
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
							}
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[i].msg[j]);
						}
						else if (data->status_msg[i].flags[j] & STATUS_LAST_STATUS_MSG)
						{
							char setting[80];
							DBVARIANT dbv;

							if (i)
								mir_snprintf(setting, SIZEOF(setting), "%sMsg", accounts->pa[k]->szModuleName);
							else
								mir_snprintf(setting, SIZEOF(setting), "Msg");

							if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(j + ID_STATUS_ONLINE, setting), &dbv))
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.ptszVal);
								db_free(&dbv);
							}
							else
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));

							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
						}
					}
					break;
				}
			}
			break;

		case IDC_ROPTPROTO1:
		case IDC_ROPTPROTO2:
		case IDC_ROPTPROTO3:
		case IDC_ROPTPROTO4:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				{
					int	i, j;

					i = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);
					j = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);

					data->proto_msg[i].flags = 0;

					if ((LOWORD(wParam) == IDC_ROPTPROTO2) || (LOWORD(wParam) == IDC_ROPTPROTO4))
					{
						data->proto_msg[i].flags |= (LOWORD(wParam) == IDC_ROPTPROTO4) ? PROTO_NO_MSG : PROTO_NOCHANGE;
						EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
					}
					else if (LOWORD(wParam) == IDC_ROPTPROTO3)
					{
						data->proto_msg[i].flags |= PROTO_THIS_MSG;
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
						if (data->proto_msg[i].msg)
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->proto_msg[i].msg);
						else
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
						EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), FALSE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), FALSE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);

						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), FALSE);
					}
					else if (LOWORD(wParam) == IDC_ROPTPROTO1)
					{
						data->proto_msg[i].flags |= PROTO_POPUPDLG;
						EnableWindow(GetDlgItem(hwndDlg, IDC_CBOPTSTATUS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTSTATUS), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG1), TRUE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG1), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG2), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG3), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG4), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ROPTMSG5), TRUE);

						EnableWindow(GetDlgItem(hwndDlg, IDC_COPTMSG2), TRUE);
					}

					if (LOWORD(wParam) != IDC_ROPTPROTO3)
					{
						if (data->status_msg[i].flags[j] & STATUS_EMPTY_MSG)
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
						}
						else if (data->status_msg[i].flags[j] & STATUS_DEFAULT_MSG)
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(j+ID_STATUS_ONLINE));
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
						}
						else if (data->status_msg[i].flags[j] & STATUS_LAST_MSG)
						{
							char setting[80];
							DBVARIANT dbv, dbv2;

							if (i)
								mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", accounts->pa[i-1]->szModuleName);
							else
								mir_snprintf(setting, SIZEOF(setting), "LastMsg");

							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
							if (!db_get(NULL, "SimpleStatusMsg", setting, &dbv))
							{
								if (dbv.pszVal)
								{
									if (!db_get_ts(NULL, "SimpleStatusMsg", dbv.pszVal, &dbv2) && strlen(dbv.pszVal))
									{
										if (dbv2.ptszVal && mir_tstrlen(dbv2.ptszVal))
											SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.ptszVal);
										db_free(&dbv2);
									}
								}
								db_free(&dbv);
							}
							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
						}
						else if (data->status_msg[i].flags[j] & STATUS_THIS_MSG)
						{
							if ((LOWORD(wParam) == IDC_ROPTPROTO2) || (LOWORD(wParam) == IDC_ROPTPROTO4))
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							}
							else
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
								EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
							}
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[i].msg[j]);
						}
						else if (data->status_msg[i].flags[j] & STATUS_LAST_STATUS_MSG)
						{
							char setting[80];
							DBVARIANT dbv;

							if (i)
								mir_snprintf(setting, SIZEOF(setting), "%sMsg", accounts->pa[i-1]->szModuleName);
							else
								mir_snprintf(setting, SIZEOF(setting), "Msg");

							if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(j + ID_STATUS_ONLINE, setting), &dbv))
							{
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.ptszVal);
								db_free(&dbv);
							}
							else
								SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));

							EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
							CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
						}
					}
					break;
				}
			}
			break;

		case IDC_CBOPTSTATUS:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
			case CBN_SELENDOK:
				{
					int	i, j;

					i = SendMessage((HWND)lParam, CB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
					j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);

					if (data->status_msg[j].flags[i] & STATUS_SHOW_DLG)
						CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_CHECKED);
					else
						CheckDlgButton(hwndDlg, IDC_COPTMSG1, BST_UNCHECKED);

					if (data->status_msg[j].flags[i] & STATUS_EMPTY_MSG)
					{
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG1);
					}
					else if (data->status_msg[j].flags[i] & STATUS_DEFAULT_MSG)
					{
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(i+ID_STATUS_ONLINE));
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG2);
					}
					else if (data->status_msg[j].flags[i] & STATUS_LAST_MSG)
					{
						char setting[80];
						DBVARIANT dbv,dbv2;

						if (j)
							mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", accounts->pa[j-1]->szModuleName);
						else
							mir_snprintf(setting, SIZEOF(setting), "LastMsg");

						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
						if (!db_get(NULL, "SimpleStatusMsg", setting, &dbv))
						{
							if (dbv.pszVal)
							{
								if (!db_get_ts(NULL, "SimpleStatusMsg", dbv.pszVal, &dbv2) && strlen(dbv.pszVal))
								{
									if (dbv2.ptszVal && mir_tstrlen(dbv2.ptszVal))
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.ptszVal);
									db_free(&dbv2);
								}
							}
							db_free(&dbv);
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG3);
					}
					else if (data->status_msg[j].flags[i] & STATUS_THIS_MSG)
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG4);
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[j].msg[i]);
					}
					else if (data->status_msg[j].flags[i] & STATUS_LAST_STATUS_MSG)
					{
						char setting[80];
						DBVARIANT dbv;

						if (j)
							mir_snprintf(setting, SIZEOF(setting), "%sMsg", accounts->pa[j-1]->szModuleName);
						else
							mir_snprintf(setting, SIZEOF(setting), "Msg");

						if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(i + ID_STATUS_ONLINE, setting), &dbv))
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.ptszVal);
							db_free(&dbv);
						}
						else
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));

						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						CheckRadioButton(hwndDlg, IDC_ROPTMSG1, IDC_ROPTMSG5, IDC_ROPTMSG5);
					}
					break;
				}
			}
			break;

		case IDC_COPTMSG1:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				{
					int	i, j;

					i = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
					j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);
					if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
						data->status_msg[j].flags[i] |= STATUS_SHOW_DLG;
					else
						data->status_msg[j].flags[i] &= ~STATUS_SHOW_DLG;
					break;
				}
			}
			break;

		case IDC_ROPTMSG1:
		case IDC_ROPTMSG2:
		case IDC_ROPTMSG3:
		case IDC_ROPTMSG4:
		case IDC_ROPTMSG5:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				{
					int	i, j;

					i = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
					j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);

					if (LOWORD(wParam) == IDC_ROPTMSG4 && data->proto_msg[j].flags & PROTO_THIS_MSG)
						break;

					data->status_msg[j].flags[i] = 0;

					if (IsDlgButtonChecked(hwndDlg, IDC_COPTMSG1) == BST_CHECKED)
						data->status_msg[j].flags[i] |= STATUS_SHOW_DLG;

					if (LOWORD(wParam) == IDC_ROPTMSG1)
					{
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						data->status_msg[j].flags[i] |= STATUS_EMPTY_MSG;
					}
					else if (LOWORD(wParam) == IDC_ROPTMSG2)
					{
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, GetDefaultMessage(i+ID_STATUS_ONLINE));
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						data->status_msg[j].flags[i] |= STATUS_DEFAULT_MSG;
					}
					else if (LOWORD(wParam) == IDC_ROPTMSG3)
					{
						char setting[80];
						DBVARIANT dbv, dbv2;

						if (j)
							mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", accounts->pa[j-1]->szModuleName);
						else
							mir_snprintf(setting, SIZEOF(setting), "LastMsg");

						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));
						if (!db_get(NULL, "SimpleStatusMsg", setting, &dbv))
						{
							if (dbv.pszVal)
							{
								if (!db_get_ts(NULL, "SimpleStatusMsg", dbv.pszVal, &dbv2) && strlen(dbv.pszVal))
								{
									if (dbv2.ptszVal && mir_tstrlen(dbv2.ptszVal))
										SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv2.ptszVal);
									db_free(&dbv2);
								}
							}
							db_free(&dbv);
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						data->status_msg[j].flags[i] |= STATUS_LAST_MSG;
					}
					else if (LOWORD(wParam) == IDC_ROPTMSG4)
					{
						data->status_msg[j].flags[i] |= STATUS_THIS_MSG;
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), TRUE);
						SetDlgItemText(hwndDlg, IDC_OPTEDIT1, data->status_msg[j].msg[i]);
					}
					else if (LOWORD(wParam) == IDC_ROPTMSG5)
					{
						char setting[80];
						DBVARIANT dbv;

						if (j)
							mir_snprintf(setting, SIZEOF(setting), "%sMsg", accounts->pa[j-1]->szModuleName);
						else
							mir_snprintf(setting, SIZEOF(setting), "Msg");

						if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(i+ID_STATUS_ONLINE, setting), &dbv))
						{
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, dbv.ptszVal);
							db_free(&dbv);
						}
						else
							SetDlgItemText(hwndDlg, IDC_OPTEDIT1, _T(""));

						EnableWindow(GetDlgItem(hwndDlg, IDC_OPTEDIT1), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARSHELP), FALSE);
						data->status_msg[j].flags[i] |= STATUS_LAST_STATUS_MSG;
					}
					break;
				}
			}
			break;

		case IDC_OPTEDIT1:
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
				int j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);

				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					TCHAR msg[1024];

					if (data->proto_msg[j].flags & PROTO_THIS_MSG)
					{
						int len = GetDlgItemText(hwndDlg, IDC_OPTEDIT1, msg, SIZEOF(msg));
						if (len > 0)
						{	
							if (data->proto_msg[j].msg == NULL)
								data->proto_msg[j].msg = mir_tstrdup(msg);
							else
							{
								mir_free(data->proto_msg[j].msg);
								data->proto_msg[j].msg = mir_tstrdup(msg);
							}
						}
						else
						{
							if (data->proto_msg[j].msg != NULL)
							{
								mir_free(data->proto_msg[j].msg);
								data->proto_msg[j].msg = NULL;
							}
						}
					}
					else
					{
						GetDlgItemText(hwndDlg, IDC_OPTEDIT1, msg, SIZEOF(msg));
						mir_tstrcpy(data->status_msg[j].msg[i], msg);
					}
				}
				break;
			}

		case IDC_VARSHELP:
			variables_showhelp(hwndDlg, IDC_OPTEDIT1, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
			break;

		case IDC_BOPTPROTO:
			{
				int	i, j, k;
				j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);

				if (j)
				{
					for (i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
					{
						if (accounts->statusMsgFlags & Proto_Status2Flag(i))
						{
							data->status_msg[0].flags[i-ID_STATUS_ONLINE] = data->status_msg[j].flags[i-ID_STATUS_ONLINE];
							if (data->status_msg[j].flags[i-ID_STATUS_ONLINE] & STATUS_THIS_MSG)
								mir_tstrcpy(data->status_msg[0].msg[i-ID_STATUS_ONLINE], data->status_msg[j].msg[i-ID_STATUS_ONLINE]);
						}
					}
				}

				for (k = 0; k < accounts->count; k++)
				{
					if (!IsAccountEnabled(accounts->pa[k]) || !CallProtoService(accounts->pa[k]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) || !(CallProtoService(accounts->pa[k]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
						continue;

					if (k != j - 1)
					{
						data->proto_msg[k+1].flags = data->proto_msg[j].flags;
						if (j)
							data->proto_msg[k+1].max_length = data->proto_msg[j].max_length;

						if (data->proto_msg[j].flags & PROTO_THIS_MSG)
						{
							int len = mir_tstrlen(data->proto_msg[j].msg);
							if (len > 0)
							{	
								if (data->proto_msg[k+1].msg == NULL)
									data->proto_msg[k+1].msg = mir_tstrdup(data->proto_msg[j].msg);
								else
								{
									mir_free(data->proto_msg[k+1].msg);
									data->proto_msg[k+1].msg = mir_tstrdup(data->proto_msg[j].msg);
								}
							}
							else
							{
								if (data->proto_msg[k+1].msg != NULL)
								{
									mir_free(data->proto_msg[k+1].msg);
									data->proto_msg[k+1].msg = NULL;
								}
							}
						}
						else if (data->proto_msg[j].flags & PROTO_POPUPDLG)
						{
							for (i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
							{
								if (CallProtoService(accounts->pa[k]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(i))
								{
									data->status_msg[k + 1].flags[i - ID_STATUS_ONLINE] = data->status_msg[j].flags[i - ID_STATUS_ONLINE];
									if (data->status_msg[j].flags[i - ID_STATUS_ONLINE] & STATUS_THIS_MSG)
										mir_tstrcpy(data->status_msg[k + 1].msg[i - ID_STATUS_ONLINE], data->status_msg[j].msg[i - ID_STATUS_ONLINE]);
								}
							}
						}
					}
				}
				break;
			} // case IDC_BOPTPROTO

		case IDC_BOPTSTATUS:
			{
				int status_modes, i, j, k;

				i = SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTSTATUS, CB_GETCURSEL, 0, 0), 0);
				j = SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_CBOPTPROTO, CB_GETCURSEL, 0, 0), 0);

				if (j)
					status_modes = CallProtoService(accounts->pa[j-1]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);
				else
					status_modes = accounts->statusMsgFlags;

				for (k = ID_STATUS_ONLINE; k <= ID_STATUS_OUTTOLUNCH; k++)
				{
					if (k - ID_STATUS_ONLINE != i && status_modes & Proto_Status2Flag(k))
					{
						data->status_msg[j].flags[k - ID_STATUS_ONLINE] = data->status_msg[j].flags[i];
						if (data->status_msg[j].flags[i] & STATUS_THIS_MSG)
							mir_tstrcpy(data->status_msg[j].msg[k - ID_STATUS_ONLINE], data->status_msg[j].msg[i]);
					}
				}
				break;
			} //case IDC_BOPTSTATUS
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			char szSetting[80];
			int i;

			for (i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
			{
				if (accounts->statusMsgFlags & Proto_Status2Flag(i))
				{
					db_set_ts(NULL, "SRAway", StatusModeToDbSetting(i, "Default"), data->status_msg[0].msg[i - ID_STATUS_ONLINE]);
					db_set_b(NULL, "SimpleStatusMsg", StatusModeToDbSetting(i, "Flags"), (BYTE)data->status_msg[0].flags[i - ID_STATUS_ONLINE]);

					for (int j = 0; j < accounts->count; j++)
					{
						if (!IsAccountEnabled(accounts->pa[j]))
							continue;

						if (!(CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
							continue;

						if (CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(i))
						{
							mir_snprintf(szSetting, SIZEOF(szSetting), "%sDefault", accounts->pa[j]->szModuleName);
							db_set_ts(NULL, "SRAway", StatusModeToDbSetting(i, szSetting), data->status_msg[j + 1].msg[i - ID_STATUS_ONLINE]);

							mir_snprintf(szSetting, SIZEOF(szSetting), "%sFlags", accounts->pa[j]->szModuleName);
							db_set_b(NULL, "SimpleStatusMsg", StatusModeToDbSetting(i, szSetting), (BYTE)data->status_msg[j + 1].flags[i - ID_STATUS_ONLINE]);
						}
					}
				}
			}

			db_set_b(NULL, "SimpleStatusMsg", "PutDefInList", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_COPTMSG2) == BST_CHECKED));

			if (data->proto_ok)
			{
				db_set_b(NULL, "SimpleStatusMsg", "ProtoFlags", (BYTE)data->proto_msg[0].flags);

				for (i = 0; i < accounts->count; i++)
				{
					if (!IsAccountEnabled(accounts->pa[i]))
						continue;

					if (!CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
						continue;

					if (!(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
						continue;

					mir_snprintf(szSetting, SIZEOF(szSetting), "Proto%sDefault", accounts->pa[i]->szModuleName);
					if (data->proto_msg[i+1].msg && (data->proto_msg[i+1].flags & PROTO_THIS_MSG))
						db_set_ts(NULL, "SimpleStatusMsg", szSetting, data->proto_msg[i+1].msg);
					//						else
					//							db_unset(NULL, "SimpleStatusMsg", szSetting);

					mir_snprintf(szSetting, SIZEOF(szSetting), "Proto%sMaxLen", accounts->pa[i]->szModuleName);
					db_set_w(NULL, "SimpleStatusMsg", szSetting, (WORD)data->proto_msg[i+1].max_length);

					mir_snprintf(szSetting, SIZEOF(szSetting), "Proto%sFlags", accounts->pa[i]->szModuleName);
					db_set_b(NULL, "SimpleStatusMsg", szSetting, (BYTE)data->proto_msg[i+1].flags);
				}
			}
			RebuildStatusMenu();
			return TRUE;
		}
		break;

	case WM_DESTROY:
		if (data->proto_ok)
		{
			for (int i = 0; i < accounts->count + 1; ++i)
			{
				if (data->proto_msg[i].msg) // they want to be free, do they?
					mir_free(data->proto_msg[i].msg);
			}
			mir_free(data->proto_msg);
		}
		mir_free(data->status_msg);
		mir_free(data);
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgVariablesOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			SendDlgItemMessage(hwndDlg, IDC_SSECUPDTMSG, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ESECUPDTMSG), 0);
			SendDlgItemMessage(hwndDlg, IDC_SSECUPDTMSG, UDM_SETRANGE32, 1, 999);
			SendDlgItemMessage(hwndDlg, IDC_SSECUPDTMSG, UDM_SETPOS, 0, MAKELONG((short)db_get_w(NULL, "SimpleStatusMsg", "UpdateMsgInt", 10), 0));
			SendDlgItemMessage(hwndDlg, IDC_ESECUPDTMSG, EM_LIMITTEXT, 3, 0);

			CheckDlgButton(hwndDlg, IDC_CUPDATEMSG, db_get_b(NULL, "SimpleStatusMsg", "UpdateMsgOn", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CNOIDLE, db_get_b(NULL, "SimpleStatusMsg", "NoUpdateOnIdle", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CNOICQREQ, db_get_b(NULL, "SimpleStatusMsg", "NoUpdateOnICQReq", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLEAVEWINAMP, db_get_b(NULL, "SimpleStatusMsg", "AmpLeaveTitle", 1) ? BST_CHECKED : BST_UNCHECKED);
			if (ServiceExists(MS_VARS_FORMATSTRING))
			{
				CheckDlgButton(hwndDlg, IDC_CVARIABLES, db_get_b(NULL, "SimpleStatusMsg", "EnableVariables", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CDATEPARSING, db_get_b(NULL, "SimpleStatusMsg", "ExclDateToken", 0) ? BST_CHECKED : BST_UNCHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CVARIABLES), FALSE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_CDATEPARSING), IsDlgButtonChecked(hwndDlg, IDC_CVARIABLES) == BST_CHECKED);

			SendMessage(hwndDlg, WM_USER + 2, 0, 0);
			return TRUE;
		}

	case WM_USER + 2:
		{
			BOOL bChecked = IsDlgButtonChecked(hwndDlg, IDC_CUPDATEMSG) == BST_CHECKED;
			EnableWindow(GetDlgItem(hwndDlg, IDC_ESECUPDTMSG), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SSECUPDTMSG), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CNOIDLE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CNOICQREQ), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLEAVEWINAMP), bChecked);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ESECUPDTMSG:
			{
				if ((HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE) return 0;
				int val = GetDlgItemInt(hwndDlg, IDC_ESECUPDTMSG, NULL, FALSE);
				if (val == 0 && GetWindowTextLength(GetDlgItem(hwndDlg, IDC_ESECUPDTMSG)))
					SendDlgItemMessage(hwndDlg, IDC_SSECUPDTMSG, UDM_SETPOS, 0, MAKELONG((short)1, 0));
				break;
			}

		case IDC_CUPDATEMSG:
			SendMessage(hwndDlg, WM_USER + 2, 0, 0);
			break;

		case IDC_CVARIABLES:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CDATEPARSING), IsDlgButtonChecked(hwndDlg, IDC_CVARIABLES) == BST_CHECKED);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			if (g_uUpdateMsgTimer)
				KillTimer(NULL, g_uUpdateMsgTimer);

			int val = SendDlgItemMessage(hwndDlg, IDC_SSECUPDTMSG, UDM_GETPOS, 0, 0);
			db_set_w(NULL, "SimpleStatusMsg", "UpdateMsgInt", (WORD)val);

			if (IsDlgButtonChecked(hwndDlg, IDC_CUPDATEMSG) == BST_CHECKED && val)
			{
				db_set_b(NULL, "SimpleStatusMsg", "UpdateMsgOn", (BYTE)1);
				g_uUpdateMsgTimer = SetTimer(NULL, 0, val * 1000, (TIMERPROC)UpdateMsgTimerProc);
			}
			else
			{
				db_set_b(NULL, "SimpleStatusMsg", "UpdateMsgOn", (BYTE)0);
			}

			db_set_b(NULL, "SimpleStatusMsg", "NoUpdateOnIdle", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CNOIDLE) == BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "NoUpdateOnICQReq", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CNOICQREQ) == BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "AmpLeaveTitle", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CLEAVEWINAMP) == BST_CHECKED));
			if (ServiceExists(MS_VARS_FORMATSTRING))
			{
				db_set_b(NULL, "SimpleStatusMsg", "EnableVariables", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CVARIABLES) == BST_CHECKED));
				db_set_b(NULL, "SimpleStatusMsg", "ExclDateToken", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CDATEPARSING) == BST_CHECKED));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL IsHistoryMsgsFound(HWND hwndDlg, int histMax)
{
	char szSetting[16];
	DBVARIANT dbv;
	int j = db_get_w(NULL, "SimpleStatusMsg", "LMMsg", 1);
	for (int i = 1; i <= histMax; ++i, --j) {
		if (j < 1)
			j = histMax;
		mir_snprintf(szSetting, SIZEOF(szSetting), "SMsg%d", j);
		if (!db_get_ts(NULL, "SimpleStatusMsg", szSetting, &dbv)) {
			if (dbv.ptszVal != NULL && *dbv.ptszVal != '\0') {
				db_free(&dbv);
				return TRUE;
			}
			db_free(&dbv);
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgAdvancedOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			int settingValue;

			TranslateDialogDefault(hwndDlg);

			// Layout
			int i_btnhide = SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hide"));
			int i_btndown = SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show next to cancel button"));
			int i_btndownflat = SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Flat, next to cancel button"));
			int i_btnlist = SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show in message list"));

			SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETITEMDATA, (WPARAM)i_btnhide, 0);
			SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETITEMDATA, (WPARAM)i_btndown, DLG_SHOW_BUTTONS);
			SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETITEMDATA, (WPARAM)i_btndownflat, DLG_SHOW_BUTTONS_FLAT);
			SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETITEMDATA, (WPARAM)i_btnlist, DLG_SHOW_BUTTONS_INLIST);

			settingValue = db_get_b(NULL, "SimpleStatusMsg", "DlgFlags", DLG_SHOW_DEFAULT);
			CheckDlgButton(hwndDlg, IDC_CSTATUSLIST, settingValue & DLG_SHOW_STATUS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CPROFILES, settingValue & DLG_SHOW_STATUS_PROFILES ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CICONS1, settingValue & DLG_SHOW_STATUS_ICONS ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CICONS2, settingValue & DLG_SHOW_LIST_ICONS ? BST_CHECKED : BST_UNCHECKED);

			if (!(settingValue & DLG_SHOW_STATUS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), FALSE);
			}

			if (settingValue & DLG_SHOW_BUTTONS)
				SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETCURSEL, (WPARAM)i_btndown, 0);
			else if (settingValue & DLG_SHOW_BUTTONS_FLAT)
				SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETCURSEL, (WPARAM)i_btndownflat, 0);
			else if (settingValue & DLG_SHOW_BUTTONS_INLIST)
				SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETCURSEL, (WPARAM)i_btnlist, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_SETCURSEL, (WPARAM)i_btnhide, 0);

			if (!ServiceExists(MS_SS_GETPROFILECOUNT)) {
				TCHAR szText[100];
				mir_sntprintf(szText, SIZEOF(szText), _T("%s *"), TranslateT("Show status profiles in status list"));
				SetDlgItemText(hwndDlg, IDC_CPROFILES, szText);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTE1), SW_SHOW);
			}

			// Misc.
			settingValue = db_get_b(NULL, "SimpleStatusMsg", "MaxHist", 10);

			SendDlgItemMessage(hwndDlg, IDC_SMAXHIST, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_EMAXHIST), 0);
			SendDlgItemMessage(hwndDlg, IDC_SMAXHIST, UDM_SETRANGE32, 0, 25);
			SendDlgItemMessage(hwndDlg, IDC_SMAXHIST, UDM_SETPOS, 0, MAKELONG((short)settingValue, 0));
			SendDlgItemMessage(hwndDlg, IDC_EMAXHIST, EM_LIMITTEXT, 2, 0);

			if (settingValue == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS2), FALSE);

			EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), IsHistoryMsgsFound(hwndDlg, settingValue));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTDEF), db_get_w(NULL, "SimpleStatusMsg", "DefMsgCount", 0) != 0);

			SendDlgItemMessage(hwndDlg, IDC_STIMEOUT, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ETIMEOUT), 0);
			SendDlgItemMessage(hwndDlg, IDC_STIMEOUT, UDM_SETRANGE32, 1, 60);
			SendDlgItemMessage(hwndDlg, IDC_STIMEOUT, UDM_SETPOS, 0, MAKELONG((short)db_get_b(NULL, "SimpleStatusMsg", "DlgTime", 5), 0));
			SendDlgItemMessage(hwndDlg, IDC_ETIMEOUT, EM_LIMITTEXT, 2, 0);

			CheckDlgButton(hwndDlg, IDC_CCLOSEWND, db_get_b(NULL, "SimpleStatusMsg", "AutoClose", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CRPOSWND, !db_get_b(NULL, "SimpleStatusMsg", "WinCentered", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CREMOVECR, db_get_b(NULL, "SimpleStatusMsg", "RemoveCR", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CSHOWSMSG, db_get_b(NULL, "SimpleStatusMsg", "ShowStatusMenuItem", 1) ? BST_CHECKED : BST_UNCHECKED);

			SendMessage(hwndDlg, WM_USER + 2, 0, 0);
			return TRUE;
		}

	case WM_USER + 2:
		{
			BOOL bChecked = IsDlgButtonChecked(hwndDlg, IDC_CCLOSEWND) == BST_CHECKED;
			EnableWindow(GetDlgItem(hwndDlg, IDC_ETIMEOUT), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STIMEOUT), bChecked);
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CSTATUSLIST:
			{
				BOOL bChecked = IsDlgButtonChecked(hwndDlg, IDC_CSTATUSLIST) == BST_CHECKED;
				EnableWindow(GetDlgItem(hwndDlg, IDC_CPROFILES), bChecked && ServiceExists(MS_SS_GETPROFILECOUNT));
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS1), bChecked);
				break;
			}

		case IDC_CBOPTBUTTONS:
			if ((HWND)lParam != GetFocus() || HIWORD(wParam) != CBN_SELCHANGE)
				return 0;
			break;

		case IDC_EMAXHIST:
			{
				if ((HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE)
					return 0;
				int val = GetDlgItemInt(hwndDlg, IDC_EMAXHIST, NULL, FALSE);
				if (val > 25)
					SendDlgItemMessage(hwndDlg, IDC_SMAXHIST, UDM_SETPOS, 0, MAKELONG((short)25, 0));
				EnableWindow(GetDlgItem(hwndDlg, IDC_CICONS2), val != 0);
				break;
			}

		case IDC_CCLOSEWND:
			SendMessage(hwndDlg, WM_USER + 2, 0, 0);
			break;

		case IDC_ETIMEOUT:
			{
				if ((HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE)
					return 0;
				int val = GetDlgItemInt(hwndDlg, IDC_ETIMEOUT, NULL, FALSE);
				if (val == 0 && GetWindowTextLength(GetDlgItem(hwndDlg, IDC_ETIMEOUT)))
					SendDlgItemMessage(hwndDlg, IDC_STIMEOUT, UDM_SETPOS, 0, MAKELONG((short)1, 0));
				else if (val > 60)
					SendDlgItemMessage(hwndDlg, IDC_STIMEOUT, UDM_SETPOS, 0, MAKELONG((short)60, 0));
				break;
			}

		case IDC_BOPTHIST:
			if (MessageBox(NULL, TranslateT("Are you sure you want to clear status message history?"), TranslateT("Confirm clearing history"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				int i, max_hist_msgs;
				char text[8], setting[80];

				if (hwndSAMsgDialog)
					DestroyWindow(hwndSAMsgDialog);

				max_hist_msgs = db_get_b(NULL, "SimpleStatusMsg", "MaxHist", 10);
				for (i = 1; i <= max_hist_msgs; i++) {
					mir_snprintf(text, SIZEOF(text), "SMsg%d", i);
					db_set_ts(NULL, "SimpleStatusMsg", text, _T(""));
				}
				db_set_s(NULL, "SimpleStatusMsg", "LastMsg", "");
				for (i = 0; i < accounts->count; i++) {
					if (!IsAccountEnabled(accounts->pa[i]))
						continue;

					if (!CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
						continue;

					if (!(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
						continue;

					mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", accounts->pa[i]->szModuleName);
					db_set_s(NULL, "SimpleStatusMsg", setting, "");
				}
				db_set_w(NULL, "SimpleStatusMsg", "LMMsg", (WORD)max_hist_msgs);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTHIST), FALSE);
			}
			return 0;

		case IDC_BOPTDEF:
			if (MessageBox(NULL, TranslateT("Are you sure you want to clear predefined status messages?"), TranslateT("Confirm clearing predefined"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				char szSetting[16];
				int nDefMSgCount;

				if (hwndSAMsgDialog)
					DestroyWindow(hwndSAMsgDialog);

				nDefMSgCount = db_get_w(NULL, "SimpleStatusMsg", "DefMsgCount", 0);
				for (int i = 1; i <= nDefMSgCount; i++) {
					mir_snprintf(szSetting, SIZEOF(szSetting), "DefMsg%d", i);
					db_unset(NULL, "SimpleStatusMsg", szSetting);
				}
				db_set_w(NULL, "SimpleStatusMsg", "DefMsgCount", 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BOPTDEF), FALSE);
			}
			return 0;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			// Layout
			int flags = 0, curSel;

			if (IsDlgButtonChecked(hwndDlg, IDC_CSTATUSLIST) == BST_CHECKED)
				flags |= DLG_SHOW_STATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_CICONS1) == BST_CHECKED)
				flags |= DLG_SHOW_STATUS_ICONS;
			if (IsDlgButtonChecked(hwndDlg, IDC_CICONS2) == BST_CHECKED)
				flags |= DLG_SHOW_LIST_ICONS;
			if (IsDlgButtonChecked(hwndDlg, IDC_CPROFILES) == BST_CHECKED)
				flags |= DLG_SHOW_STATUS_PROFILES;

			curSel = SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_GETCURSEL, 0, 0);
			if (curSel != CB_ERR)
				flags |= SendDlgItemMessage(hwndDlg, IDC_CBOPTBUTTONS, CB_GETITEMDATA, (WPARAM)curSel, 0);

			db_set_b(NULL, "SimpleStatusMsg", "DlgFlags", (BYTE)flags);

			// Misc.
			db_set_b(NULL, "SimpleStatusMsg", "MaxHist", (BYTE)GetDlgItemInt(hwndDlg, IDC_EMAXHIST, NULL, FALSE));
			db_set_b(NULL, "SimpleStatusMsg", "AutoClose", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CCLOSEWND) == BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "DlgTime", (BYTE)GetDlgItemInt(hwndDlg, IDC_ETIMEOUT, NULL, FALSE));
			db_set_b(NULL, "SimpleStatusMsg", "WinCentered", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CRPOSWND) != BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "RemoveCR", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CREMOVECR) == BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "ShowStatusMenuItem", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CSHOWSMSG) == BST_CHECKED));
			RebuildStatusMenu();

			return TRUE;
		}
		break;
	}
	return FALSE;
}

struct StatusOptDlgData
{
	int *status;
	int *setdelay;
	int setglobaldelay;
};

static INT_PTR CALLBACK DlgStatusOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct StatusOptDlgData *data = (struct StatusOptDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			int index, i;

			TranslateDialogDefault(hwndDlg);

			data = (struct StatusOptDlgData *)mir_alloc(sizeof(struct StatusOptDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);

			data->status = (int *)mir_alloc(sizeof(int) * accounts->count);
			data->setdelay = (int *)mir_alloc(sizeof(int) * accounts->count);
			for (i = 0; i < accounts->count; ++i)
			{
				if (!IsAccountEnabled(accounts->pa[i]) || !(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) &~ CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_5, 0)))
					continue;

				index = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_ADDSTRING, 0, (LPARAM)accounts->pa[i]->tszAccountName);
				if (index != LB_ERR && index != LB_ERRSPACE)
				{
					char setting[80];
					mir_snprintf(setting, SIZEOF(setting), "Startup%sStatus", accounts->pa[i]->szModuleName);
					data->status[i] = db_get_w(NULL, "SimpleStatusMsg", setting, ID_STATUS_OFFLINE);
					mir_snprintf(setting, SIZEOF(setting), "Set%sStatusDelay", accounts->pa[i]->szModuleName);
					data->setdelay[i] = db_get_w(NULL, "SimpleStatusMsg", setting, 300);
					SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
				}
			}
			SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LISTPROTO, LBN_SELCHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_LISTPROTO));

			data->setglobaldelay = db_get_w(NULL, "SimpleStatusMsg", "SetStatusDelay", 300);

			SendDlgItemMessage(hwndDlg, IDC_SSETSTATUS, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_ESETSTATUS), 0);
			SendDlgItemMessage(hwndDlg, IDC_SSETSTATUS, UDM_SETRANGE32, 0, 9000);
			SendDlgItemMessage(hwndDlg, IDC_ESETSTATUS, EM_LIMITTEXT, 4, 0);

			if (!db_get_b(NULL, "SimpleStatusMsg", "GlobalStatusDelay", 1))
			{
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_CHECKED);
				i = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETCURSEL, 0, 0), 0);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
			}
			else
			{
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
			}

			if (db_get_b(NULL, "SimpleStatusMsg", "StartupPopupDlg", 1))
			{
				CheckDlgButton(hwndDlg, IDC_POPUPDLG, BST_CHECKED);
				if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED)
				{
					CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
					SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
			}
			else
				CheckDlgButton(hwndDlg, IDC_POPUPDLG, BST_UNCHECKED);

			if (accounts->statusCount == 1 && accounts->statusMsgCount == 1)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
				CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED); //should work like when checked, but should not be checked
				i = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETCURSEL, 0, 0), 0);
				SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
			}

			return TRUE;
		}

	case WM_COMMAND:
		if ( ( (HIWORD(wParam) == BN_CLICKED) || /*(HIWORD(wParam) == EN_KILLFOCUS) ||*/ (HIWORD(wParam) == EN_CHANGE)
			|| ( (HIWORD(wParam) == LBN_SELCHANGE)  && (LOWORD(wParam) != IDC_LISTPROTO))
			) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_ESETSTATUS:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				{
					BOOL translated;
					int val = GetDlgItemInt(hwndDlg, IDC_ESETSTATUS, &translated, FALSE);
					if (translated && val > 9000)
						SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, 9000, FALSE);
					if (translated && val < 0)
						SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, 0, FALSE);
					val = GetDlgItemInt(hwndDlg, IDC_ESETSTATUS, &translated, FALSE);

					if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED || (accounts->statusCount == 1 && accounts->statusMsgCount == 1))
					{
						int i = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETCURSEL, 0, 0), 0);
						data->setdelay[i] = val;
					}
					else
						data->setglobaldelay = val;
					break;
				}
			}
			break;

		case IDC_SPECSET:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED || (accounts->statusCount == 1 && accounts->statusMsgCount == 1))
				{
					int i = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETCURSEL, 0, 0), 0);
					SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
				}
				else
					SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
				break;
			}
			break;

		case IDC_POPUPDLG:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				if (accounts->statusCount == 1 && accounts->statusMsgCount == 1)
					break;

				if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED)
					{
						CheckDlgButton(hwndDlg, IDC_SPECSET, BST_UNCHECKED);
						SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setglobaldelay, FALSE);
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), FALSE);
				}
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_SPECSET), TRUE);
				break;
			}
			break;

		case IDC_LISTPROTO:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				{
					int status_modes, i, l, index, newindex = 0;

					i = SendMessage((HWND)lParam, LB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0), 0);
					status_modes = CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) & ~CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_5, 0);

					SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_RESETCONTENT, 0, 0);
					for (l = ID_STATUS_OFFLINE; l <= ID_STATUS_OUTTOLUNCH; l++)
					{
						if (status_modes & Proto_Status2Flag(l) || l == ID_STATUS_OFFLINE)
						{
							index = SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_INSERTSTRING, -1, (LPARAM)pcli->pfnGetStatusModeDescription(l, 0));
							if (index != LB_ERR && index != LB_ERRSPACE)
							{
								SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_SETITEMDATA, (WPARAM)index, (LPARAM)l - ID_STATUS_OFFLINE);
								if (data->status[i] == l)
									newindex = index;
							}
						}
					}

					index = SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_INSERTSTRING, -1, (LPARAM)TranslateT("<last status>"));
					if (index != LB_ERR && index != LB_ERRSPACE)
					{
						SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_SETITEMDATA, (WPARAM)index, (LPARAM)ID_STATUS_CURRENT - ID_STATUS_OFFLINE);
						if (data->status[i] == ID_STATUS_CURRENT)
							newindex = index;
					}

					SendDlgItemMessage(hwndDlg, IDC_LISTSTATUS, LB_SETCURSEL, (WPARAM)newindex, 0);

					if (IsDlgButtonChecked(hwndDlg, IDC_SPECSET) == BST_CHECKED || (accounts->statusCount == 1 && accounts->statusMsgCount == 1))
						SetDlgItemInt(hwndDlg, IDC_ESETSTATUS, data->setdelay[i], FALSE);
					break;
				}
			}
			break;

		case IDC_LISTSTATUS:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				{
					int i = SendMessage((HWND)lParam, LB_GETITEMDATA, (WPARAM)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0), 0);
					int j = SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_LISTPROTO, LB_GETCURSEL, 0, 0), 0);
					data->status[j] = i + ID_STATUS_OFFLINE;
					break;
				}
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			char szSetting[80];
			for (int i = 0; i < accounts->count; i++)
			{
				if (!IsAccountEnabled(accounts->pa[i]) || !(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_5, 0)))
					continue;

				mir_snprintf(szSetting, SIZEOF(szSetting), "Startup%sStatus", accounts->pa[i]->szModuleName);
				db_set_w(NULL, "SimpleStatusMsg", szSetting, (WORD)data->status[i]);

				mir_snprintf(szSetting, SIZEOF(szSetting), "Set%sStatusDelay", accounts->pa[i]->szModuleName);
				db_set_w(NULL, "SimpleStatusMsg", szSetting, (WORD)data->setdelay[i]);
			}
			db_set_w(NULL, "SimpleStatusMsg", "SetStatusDelay", (WORD)data->setglobaldelay);
			db_set_b(NULL, "SimpleStatusMsg", "GlobalStatusDelay", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SPECSET) != BST_CHECKED));
			db_set_b(NULL, "SimpleStatusMsg", "StartupPopupDlg", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_POPUPDLG) == BST_CHECKED));
			return TRUE;
		}
		break;

	case WM_DESTROY:
		mir_free(data->status);
		mir_free(data->setdelay);
		mir_free(data);
		break;
	}
	return FALSE;
}

int InitOptions(WPARAM wParam, LPARAM lParam)
{
	if (accounts->statusCount == 0)
		return 0;

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.flags = ODPF_BOLDGROUPS;

	if (!ServiceExists(MS_SS_GETPROFILECOUNT)) {
		odp.pszTitle = LPGEN("Status");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STATUS);
		odp.pfnDlgProc = DlgStatusOptionsProc;
		Options_AddPage(wParam, &odp);
	}

	if (accounts->statusMsgCount == 0)
		return 0;

	odp.position = 870000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENERAL);
	odp.pszTitle = LPGEN("Status messages");
	odp.pszGroup = LPGEN("Status");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = DlgOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Variables");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_VARIABLES);
	odp.pfnDlgProc = DlgVariablesOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADVANCED);
	odp.pfnDlgProc = DlgAdvancedOptionsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
