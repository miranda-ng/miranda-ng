/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/Deprecated/opt_notify.cpp $
Revision       : $Revision: 1615 $
Last change on : $Date: 2010-06-23 01:47:57 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

INT_PTR CALLBACK DlgProcPopUps(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hNotify = (HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwnd, IDC_SALL, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SALL, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SALL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(ICO_POPUP_ON,0));

			SendDlgItemMessage(hwnd, IDC_SOFFLINE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SOFFLINE, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SOFFLINE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_OFFLINE));

			SendDlgItemMessage(hwnd, IDC_SONLINE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SONLINE, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SONLINE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_ONLINE));

			SendDlgItemMessage(hwnd, IDC_SAWAY, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SAWAY, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SAWAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_AWAY));

			SendDlgItemMessage(hwnd, IDC_SNA, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SNA, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SNA, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_NA));

			SendDlgItemMessage(hwnd, IDC_SOCCUPIED, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SOCCUPIED, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SOCCUPIED, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_OCCUPIED));

			SendDlgItemMessage(hwnd, IDC_SDND, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SDND, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SDND, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_DND));

			SendDlgItemMessage(hwnd, IDC_SFREE4CHAT, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SFREE4CHAT, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SFREE4CHAT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_FREE4CHAT));

			SendDlgItemMessage(hwnd, IDC_SINVISIBLE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SINVISIBLE, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SINVISIBLE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_INVISIBLE));

			SendDlgItemMessage(hwnd, IDC_SPHONE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SPHONE, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SPHONE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_ONTHEPHONE));

			SendDlgItemMessage(hwnd, IDC_SLUNCH, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SLUNCH, BUTTONSETASPUSHBTN, 0, 0);
			SendDlgItemMessage(hwnd, IDC_SLUNCH, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_OUTTOLUNCH));

			return TRUE;
		}

		case UM_MNOTIFY_CHECK:
		{
			if (wParam != 1)
			{
				SetWindowLong(hwnd, DWLP_MSGRESULT, FALSE);
				return 0;
			}

			hNotify = *(HANDLE *)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hNotify);

			EnableWindow(GetDlgItem(hwnd, IDC_TIMEOUT), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_USEBACKCOLOR), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_USETEXTCOLOR), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_USETIMEOUT), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_LACTION), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_RACTION), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SALL), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SONLINE), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SOFFLINE), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SAWAY), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SNA), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SOCCUPIED), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SDND), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SFREE4CHAT), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SINVISIBLE), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SPHONE), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_SLUNCH), lParam);
			EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), lParam);

			DWORD dwStatusMode = MNotifyGetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, 0x03ff);
			CheckDlgButton(hwnd, IDC_SALL, dwStatusMode==0x3ff ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SOFFLINE, dwStatusMode & 0x001 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SONLINE, dwStatusMode & 0x002 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SAWAY, dwStatusMode & 0x004 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SNA, dwStatusMode & 0x008 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SOCCUPIED, dwStatusMode & 0x010 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SDND, dwStatusMode & 0x020 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SFREE4CHAT, dwStatusMode & 0x040 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SINVISIBLE, dwStatusMode & 0x080 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SPHONE, dwStatusMode & 0x100 ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_SLUNCH, dwStatusMode & 0x200 ? TRUE : FALSE);

			COLORREF cl;
			if ((cl = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, MNotifyGetDWord(hNotify, NFOPT_BACKCOLOR, 0))) != (COLORREF)-1)
			{
				CheckDlgButton(hwnd, IDC_USEBACKCOLOR, BST_UNCHECKED);
				SendDlgItemMessage(hwnd, IDC_BACKCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
				EnableWindow(GetDlgItem(hwnd, IDC_BACKCOLOR), TRUE);
			} else
			{
				CheckDlgButton(hwnd, IDC_USEBACKCOLOR, BST_CHECKED);
				SendDlgItemMessage(hwnd, IDC_BACKCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
				EnableWindow(GetDlgItem(hwnd, IDC_BACKCOLOR), FALSE);
			}
			if ((cl = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, MNotifyGetDWord(hNotify, NFOPT_TEXTCOLOR, 0))) != (COLORREF)-1)
			{
				CheckDlgButton(hwnd, IDC_USETEXTCOLOR, BST_UNCHECKED);
				SendDlgItemMessage(hwnd, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
				EnableWindow(GetDlgItem(hwnd, IDC_TEXTCOLOR), TRUE);
			} else
			{
				CheckDlgButton(hwnd, IDC_USETEXTCOLOR, BST_CHECKED);
				SendDlgItemMessage(hwnd, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
				EnableWindow(GetDlgItem(hwnd, IDC_TEXTCOLOR), FALSE);
			}

			HWND hwndCombo = GetDlgItem(hwnd, IDC_TIMEOUT);
			ComboBox_ResetContent(hwndCombo);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "Default"),   0);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "1 second"),   1);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "2 seconds"),  2);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "3 seconds"),  3);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "4 seconds"),  4);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "5 seconds"),  5);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "7 seconds"),  7);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "10 seconds"), 10);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "15 seconds"), 15);
			ComboBox_SetItemData(hwndCombo, ComboBox_AddString(hwndCombo, "Infinite"),   -1);

			int timeout = (int)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TIMEOUT, MNotifyGetDWord(hNotify, NFOPT_TIMEOUT, 0));
//			if (!timeout)
//			{
//				CheckDlgButton(hwnd, IDC_USETIMEOUT, BST_CHECKED);
//				EnableWindow(GetDlgItem(hwnd, IDC_TIMEOUT), FALSE);
//			} else
			if (timeout < 0)
			{
				SetDlgItemText(hwnd, IDC_TIMEOUT, "Infinite");
			} else
			{
				switch (timeout)
				{
				case 0: SetDlgItemText(hwnd, IDC_TIMEOUT, "Default"); break;
				case 1: SetDlgItemText(hwnd, IDC_TIMEOUT, "1 second"); break;
				case 2: SetDlgItemText(hwnd, IDC_TIMEOUT, "2 seconds"); break;
				case 3: SetDlgItemText(hwnd, IDC_TIMEOUT, "3 seconds"); break;
				case 4: SetDlgItemText(hwnd, IDC_TIMEOUT, "4 seconds"); break;
				case 5: SetDlgItemText(hwnd, IDC_TIMEOUT, "5 seconds"); break;
				case 7: SetDlgItemText(hwnd, IDC_TIMEOUT, "7 seconds"); break;
				case 10: SetDlgItemText(hwnd, IDC_TIMEOUT, "10 seconds"); break;
				case 15: SetDlgItemText(hwnd, IDC_TIMEOUT, "15 seconds"); break;
				default: SetDlgItemInt(hwnd, IDC_TIMEOUT, timeout, TRUE);
				}
			}

			const char *svcLAction = MNotifyGetString(hNotify, NFOPT_POPUP2_LCLICKSVC, "Popup2/DefaultActions");
			DWORD cookieLAction = MNotifyGetDWord(hNotify, NFOPT_POPUP2_LCLICKCOOKIE, (DWORD)-1);
			const char *svcRAction = MNotifyGetString(hNotify, NFOPT_POPUP2_RCLICKSVC, "Popup2/DefaultActions");
			DWORD cookieRAction = MNotifyGetDWord(hNotify, NFOPT_POPUP2_RCLICKCOOKIE, (DWORD)-1);

			int idItem, idRAction=0, idLAction=0;
			int nActions = MNotifyGetActions(hNotify, 0);
			HWND hwndLCombo = GetDlgItem(hwnd, IDC_LACTION);
			HWND hwndRCombo = GetDlgItem(hwnd, IDC_RACTION);
			ComboBox_ResetContent(hwndLCombo);
			ComboBox_ResetContent(hwndRCombo);
			ComboBox_SetItemData(hwndLCombo, ComboBox_AddString(hwndLCombo, "Basic Actions:"), -4);
			ComboBox_SetItemData(hwndRCombo, ComboBox_AddString(hwndRCombo, "Basic Actions:"), -4);
			ComboBox_SetItemData(hwndLCombo, idItem = ComboBox_AddString(hwndLCombo, "    Send Message"), -1);
			if (!idLAction && !strcmp(svcLAction, "Popup2/DefaultActions") && cookieLAction==0) idLAction=idItem;
			ComboBox_SetItemData(hwndRCombo, idItem = ComboBox_AddString(hwndRCombo, "    Send Message"), -1);
			if (!idRAction && !strcmp(svcRAction, "Popup2/DefaultActions") && cookieRAction==0) idRAction=idItem;
			ComboBox_SetItemData(hwndLCombo, idItem = ComboBox_AddString(hwndLCombo, "    Dismiss Popup"), -2);
			if (!idLAction && !strcmp(svcLAction, "Popup2/DefaultActions") && cookieLAction==1) idLAction=idItem;
			ComboBox_SetItemData(hwndRCombo, idItem = ComboBox_AddString(hwndRCombo, "    Dismiss Popup"), -2);
			if (!idRAction && !strcmp(svcRAction, "Popup2/DefaultActions") && cookieRAction==1) idRAction=idItem;
			ComboBox_SetItemData(hwndLCombo, idItem = ComboBox_AddString(hwndLCombo, "    Do Nothing"), -3);
			if (!idLAction && !strcmp(svcLAction, "Popup2/DefaultActions") && cookieLAction==2) idLAction=idItem;
			ComboBox_SetItemData(hwndRCombo, idItem = ComboBox_AddString(hwndRCombo, "    Do Nothing"), -3);
			if (!idRAction && !strcmp(svcRAction, "Popup2/DefaultActions") && cookieRAction==2) idRAction=idItem;
			if (nActions)
			{
				MNOTIFYACTIONINFO *actions = new MNOTIFYACTIONINFO[nActions];
				MNotifyGetActions(hNotify, actions);
				ComboBox_SetItemData(hwndLCombo, ComboBox_AddString(hwndLCombo, "Advanced Actions:"), -3);
				ComboBox_SetItemData(hwndRCombo, ComboBox_AddString(hwndRCombo, "Advanced Actions:"), -3);
				for (int i = 0; i < nActions; i++)
				{
					char buf[64];
					wsprintf(buf, "    %64s", actions[i].name);
					ComboBox_SetItemData(hwndLCombo, idItem = ComboBox_AddString(hwndLCombo, buf), i);
					if (!idLAction && !strcmp(svcLAction, actions[i].service) && cookieLAction==actions[i].cookie) idLAction=idItem;
					ComboBox_SetItemData(hwndRCombo, idItem = ComboBox_AddString(hwndRCombo, buf), i);
					if (!idRAction && !strcmp(svcRAction, actions[i].service) && cookieRAction==actions[i].cookie) idRAction=idItem;
				}
				delete [] actions;
			}
			ComboBox_SetCurSel(hwndLCombo, idLAction?idLAction:1);
			ComboBox_SetCurSel(hwndRCombo, idRAction?idRAction:2);

			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_PREVIEW:
			{
				HANDLE hNotifyObj = MNotifyCreate(hNotify);
				MNotifySetString(hNotifyObj, NFOPT_TITLE, "Popup2");
				MNotifySetString(hNotifyObj, NFOPT_TEXT, "The new preview :)");
				MPopup2Show(hNotifyObj);
				break;
			}
			case IDC_USEBACKCOLOR:
			{
				if (IsDlgButtonChecked(hwnd, IDC_USEBACKCOLOR))
				{
					MNotifySetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR"/Save",
						MNotifyGetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, 0));
					MNotifySetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, (DWORD)-1);
					EnableWindow(GetDlgItem(hwnd, IDC_BACKCOLOR), FALSE);
				} else
				{
					COLORREF cl = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR"/Save", 0);
					MNotifySetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, (DWORD)cl);
					SendDlgItemMessage(hwnd, IDC_BACKCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
					EnableWindow(GetDlgItem(hwnd, IDC_BACKCOLOR), TRUE);
				}
				SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				break;
			}
			case IDC_USETEXTCOLOR:
			{
				if (IsDlgButtonChecked(hwnd, IDC_USETEXTCOLOR))
				{
					MNotifySetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR"/Save",
						MNotifyGetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, 0));
					MNotifySetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, (DWORD)-1);
					EnableWindow(GetDlgItem(hwnd, IDC_TEXTCOLOR), FALSE);
				} else
				{
					COLORREF cl = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR"/Save", 0);
					MNotifySetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, (DWORD)cl);
					SendDlgItemMessage(hwnd, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, (LPARAM)cl);
					EnableWindow(GetDlgItem(hwnd, IDC_TEXTCOLOR), TRUE);
				}
				SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				break;
			};
			case IDC_BACKCOLOR:
			case IDC_TEXTCOLOR:
			{
				if (HIWORD(wParam) == CPN_COLOURCHANGED)
				{
					MNotifySetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, (DWORD)SendDlgItemMessage(hwnd,IDC_BACKCOLOR, CPM_GETCOLOUR, 0, 0));
					MNotifySetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, (DWORD)SendDlgItemMessage(hwnd,IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0));
					SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				}
				break;
			}
			case IDC_TIMEOUT:
			{
				int timeout=-2;
				int idItem;
				if ((idItem = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_TIMEOUT))) != CB_ERR)
				{
					timeout = (int)ComboBox_GetItemData(GetDlgItem(hwnd, IDC_TIMEOUT), idItem);
				} else
				{
					char buf[64];
					ComboBox_GetText(GetDlgItem(hwnd, IDC_TIMEOUT), buf, 64);
					if (*buf >= '0' && *buf <= '9')
						timeout = atoi(buf);
				}
				if (timeout>-2)
				{
					MNotifySetDWord(hNotify, NFOPT_POPUP2_TIMEOUT, (DWORD)timeout);
					SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				} else
				{
					timeout = (int)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TIMEOUT, 0);
					if (timeout < 0) SetDlgItemText(hwnd, IDC_TIMEOUT, "Infinite");
					else
					{
						switch (timeout)
						{
						case 0: SetDlgItemText(hwnd, IDC_TIMEOUT, "Default"); break;
						case 1: SetDlgItemText(hwnd, IDC_TIMEOUT, "1 second"); break;
						case 2: SetDlgItemText(hwnd, IDC_TIMEOUT, "2 seconds"); break;
						case 3: SetDlgItemText(hwnd, IDC_TIMEOUT, "3 seconds"); break;
						case 4: SetDlgItemText(hwnd, IDC_TIMEOUT, "4 seconds"); break;
						case 5: SetDlgItemText(hwnd, IDC_TIMEOUT, "5 seconds"); break;
						case 7: SetDlgItemText(hwnd, IDC_TIMEOUT, "7 seconds"); break;
						case 10: SetDlgItemText(hwnd, IDC_TIMEOUT, "10 seconds"); break;
						case 15: SetDlgItemText(hwnd, IDC_TIMEOUT, "15 seconds"); break;
						default: SetDlgItemInt(hwnd, IDC_TIMEOUT, timeout, TRUE);
						}
					}
				}
				break;
			}
			case IDC_LACTION:
			case IDC_RACTION:
			{
				int idCtrl = LOWORD(wParam);
				HWND hwndCombo = GetDlgItem(hwnd, idCtrl);
				int idItem;
				if ((idItem = ComboBox_GetCurSel(hwndCombo)) == CB_ERR)
					break;

				char *svc=0; DWORD cookie=0;
				MNOTIFYACTIONINFO *actions=0;
				int idAction = (int)ComboBox_GetItemData(hwndCombo, idItem);
				if (idAction == -1)
				{ // message
					svc = "Popup2/DefaultActions";
					cookie = 0;
				} else
				if (idAction == -2)
				{ // dismiss
					svc = "Popup2/DefaultActions";
					cookie = 1;
				} else
				if (idAction == -3)
				{ // do nothing
					svc = "Popup2/DefaultActions";
					cookie = 2;
				} else
				if (idAction >= 0)
				{
					int nActions = MNotifyGetActions(hNotify, 0);
					if (nActions)
					{
						actions = new MNOTIFYACTIONINFO[nActions];
						MNotifyGetActions(hNotify, actions);
						svc = actions[idAction].service;
						cookie = actions[idAction].cookie;
					}
				} else
				{ // do nothing
					ComboBox_SetCurSel(hwndCombo, 3);
				}

				if (svc)
				{
					MNotifySetString(hNotify,
						idCtrl==IDC_LACTION ? NFOPT_POPUP2_LCLICKSVC : NFOPT_POPUP2_RCLICKSVC,
						svc);
					MNotifySetDWord(hNotify,
						idCtrl==IDC_LACTION ? NFOPT_POPUP2_LCLICKCOOKIE : NFOPT_POPUP2_RCLICKCOOKIE,
						cookie);
					SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				}
				if (actions) delete [] actions;
				break;
			}
			case IDC_SALL:
			{
				DWORD dwStatusMode = IsDlgButtonChecked(hwnd, IDC_SALL) ? 0x3ff : 0;
				CheckDlgButton(hwnd, IDC_SOFFLINE, dwStatusMode & 0x001 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SONLINE, dwStatusMode & 0x002 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SAWAY, dwStatusMode & 0x004 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SNA, dwStatusMode & 0x008 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SOCCUPIED, dwStatusMode & 0x010 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SDND, dwStatusMode & 0x020 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SFREE4CHAT, dwStatusMode & 0x040 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SINVISIBLE, dwStatusMode & 0x080 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SPHONE, dwStatusMode & 0x100 ? TRUE : FALSE);
				CheckDlgButton(hwnd, IDC_SLUNCH, dwStatusMode & 0x200 ? TRUE : FALSE);
				MNotifySetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, dwStatusMode);
				SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				break;
			}
			case IDC_SOFFLINE:
			case IDC_SONLINE:
			case IDC_SAWAY:
			case IDC_SNA:
			case IDC_SOCCUPIED:
			case IDC_SDND:
			case IDC_SFREE4CHAT:
			case IDC_SINVISIBLE:
			case IDC_SPHONE:
			case IDC_SLUNCH:
			{
				DWORD dwStatusMode=0; // = MNotifyGetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, 0x03ff);
				if (IsDlgButtonChecked(hwnd, IDC_SOFFLINE)) dwStatusMode |= 0x001;
				if (IsDlgButtonChecked(hwnd, IDC_SONLINE)) dwStatusMode |= 0x002;
				if (IsDlgButtonChecked(hwnd, IDC_SAWAY)) dwStatusMode |= 0x004;
				if (IsDlgButtonChecked(hwnd, IDC_SNA)) dwStatusMode |= 0x008;
				if (IsDlgButtonChecked(hwnd, IDC_SOCCUPIED)) dwStatusMode |= 0x010;
				if (IsDlgButtonChecked(hwnd, IDC_SDND)) dwStatusMode |= 0x020;
				if (IsDlgButtonChecked(hwnd, IDC_SFREE4CHAT)) dwStatusMode |= 0x040;
				if (IsDlgButtonChecked(hwnd, IDC_SINVISIBLE)) dwStatusMode |= 0x080;
				if (IsDlgButtonChecked(hwnd, IDC_SPHONE)) dwStatusMode |= 0x100;
				if (IsDlgButtonChecked(hwnd, IDC_SLUNCH)) dwStatusMode |= 0x200;
				CheckDlgButton(hwnd, IDC_SALL, dwStatusMode==0x3ff ? TRUE : FALSE);
				MNotifySetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, dwStatusMode);
				SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
				break;
			}
			}
			break;
	}
	return FALSE;
}
