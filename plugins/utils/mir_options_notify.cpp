/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "mir_options_notify.h"

#include <stdio.h>
#include <commctrl.h>
#include <tchar.h>

extern "C"
{
#include <newpluginapi.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_notify.h>
#include "templates.h"
}


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialog to save options
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK SaveOptsDlgProc(OptPageControl *controls, int controlsSize, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_USER+100:
		{
			HANDLE hNotify = (HANDLE)lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			TranslateDialogDefault(hwndDlg);

			for (int i = 0 ; i < controlsSize ; i++)
			{
				OptPageControl *ctrl = &controls[i];

				switch(ctrl->type)
				{
					case CONTROL_CHECKBOX:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, MNotifyGetByte(hNotify, ctrl->setting, (BYTE)ctrl->dwDefValue) == 1 ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_SPIN:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, ctrl->nID),0);
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETRANGE, 0, MAKELONG(ctrl->max, ctrl->min));
						SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_SETPOS,0, MAKELONG(MNotifyGetWord(hNotify, ctrl->setting, (WORD)ctrl->dwDefValue), 0));

						break;
					}
					case CONTROL_COLOR:
					{
						SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_SETCOLOUR, 0, (COLORREF) MNotifyGetDWord(hNotify, ctrl->setting, (DWORD)ctrl->dwDefValue));

						break;
					}
					case CONTROL_RADIO:
					{
						CheckDlgButton(hwndDlg, ctrl->nID, MNotifyGetWord(hNotify, ctrl->setting, (WORD)ctrl->dwDefValue) == ctrl->value ? BST_CHECKED : BST_UNCHECKED);
						break;
					}
					case CONTROL_TEXT:
					{
						SetDlgItemText(hwndDlg, ctrl->nID, MNotifyGetTString(hNotify, ctrl->setting, ctrl->szDefVale));

						if (ctrl->max > 0)
							SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max, 1024), 0);
						else
							SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, 1024, 0);
						
						break;
					}
					case CONTROL_TEMPLATE:
					{
						SetDlgItemText(hwndDlg, ctrl->nID, MNotifyGetTTemplate(hNotify, ctrl->setting, ctrl->szDefVale));

						if (ctrl->max > 0)
							SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, min(ctrl->max, 1024), 0);
						else
							SendDlgItemMessage(hwndDlg, ctrl->nID, EM_LIMITTEXT, 1024, 0);
						
						break;
					}
				}
			}

			return TRUE;
			break;
		}
		case WM_COMMAND:
		{
			// Don't make apply enabled during buddy set crap
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			{
				for (int i = 0 ; i < controlsSize ; i++)
				{
					if (controls[i].type == CONTROL_SPIN && LOWORD(wParam) == controls[i].nID)
					{
						return 0;
					}
				}
			}

			SendMessage(GetParent(GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) 
			{
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							HANDLE hNotify = (HANDLE)GetWindowLong(hwndDlg, GWL_USERDATA);
							TCHAR tmp[1024];

							for (int i = 0 ; i < controlsSize ; i++)
							{
								OptPageControl *ctrl = &controls[i];

								switch(ctrl->type)
								{
									case CONTROL_CHECKBOX:
									{
										MNotifySetByte(hNotify, ctrl->setting, (BYTE)IsDlgButtonChecked(hwndDlg, ctrl->nID));
										break;
									}
									case CONTROL_SPIN:
									{
										MNotifySetWord(hNotify, ctrl->setting, (WORD)SendDlgItemMessage(hwndDlg, ctrl->nIDSpin, UDM_GETPOS, 0, 0));
										break;
									}
									case CONTROL_COLOR:
									{
										MNotifySetDWord(hNotify, ctrl->setting, (DWORD)SendDlgItemMessage(hwndDlg, ctrl->nID, CPM_GETCOLOUR, 0, 0));
										break;
									}
									case CONTROL_RADIO:
									{
										if (IsDlgButtonChecked(hwndDlg, ctrl->nID))
											MNotifySetWord(hNotify, ctrl->setting, (BYTE)ctrl->value);
										break;
									}
									case CONTROL_TEXT:
									{
										GetDlgItemText(hwndDlg, ctrl->nID, tmp, MAX_REGS(tmp));
										MNotifySetTString(hNotify, ctrl->setting, tmp);
										break;
									}
									case CONTROL_TEMPLATE:
									{
										GetDlgItemText(hwndDlg, ctrl->nID, tmp, MAX_REGS(tmp));
										MNotifySetTTemplate(hNotify, ctrl->setting, tmp);
										break;
									}
								}
							}

							return TRUE;
						}
					}
					break;
				}
			}
			break;
		}
	}

	return 0;
}
