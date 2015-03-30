/* ============================================================================
Hardware HotKeys plugin for Miranda NG.
Copyright © Eugene f2065, http://f2065.narod.ru, f2065 mail.ru, ICQ 35078112

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111 - 1307, USA.
============================================================================ */

#include "HwHotKeys.h"

INT_PTR CALLBACK OptDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:

		hDialogWnd = hDlg;

		TranslateDialogDefault(hDlg);
	
		code_Close_tmp = code_Close;
		HwHotKeys_PrintFullKeyname(code_Close_tmp);
		SetDlgItemTextA(hDlg, dlg_close_edit, key_name_buffer);
		code_HideShow_tmp = code_HideShow;
		HwHotKeys_PrintFullKeyname(code_HideShow_tmp);
		SetDlgItemTextA(hDlg, dlg_hide_edit, key_name_buffer);
		code_ReadMsg_tmp = code_ReadMsg;
		HwHotKeys_PrintFullKeyname(code_ReadMsg_tmp);
		SetDlgItemTextA(hDlg, dlg_read_edit, key_name_buffer);

		break; //case WM_INITDIALOG

	case WM_COMMAND:
		switch (wParam)
		{
		case dlg_close_set:
			code_Close_tmp = key_code_assign;
			HwHotKeys_PrintFullKeyname(key_code_assign);
			SetDlgItemTextA(hDialogWnd, dlg_close_edit, key_name_buffer);
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0); // сообщает системе о том что были изменения
			break;
		case dlg_close_clear:
			code_Close_tmp = 0;
			SetDlgItemTextA(hDialogWnd, dlg_close_edit, "");
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		case dlg_hide_set:
			code_HideShow_tmp = key_code_assign;
			HwHotKeys_PrintFullKeyname(key_code_assign);
			SetDlgItemTextA(hDialogWnd, dlg_hide_edit, key_name_buffer);
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		case dlg_hide_clear:
			code_HideShow_tmp = 0;
			SetDlgItemTextA(hDialogWnd, dlg_hide_edit, "");
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		case dlg_read_set:
			code_ReadMsg_tmp = key_code_assign;
			HwHotKeys_PrintFullKeyname(key_code_assign);
			SetDlgItemTextA(hDialogWnd, dlg_read_edit, key_name_buffer);
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		case dlg_read_clear:
			code_ReadMsg_tmp = 0;
			SetDlgItemTextA(hDialogWnd, dlg_read_edit, "");
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break; //case WM_COMMAND

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY: // система просит сохранить настройки плагина
				code_Close = code_Close_tmp;
				db_set_dw(0, __DbModName, "Close", code_Close_tmp);
				code_HideShow = code_HideShow_tmp;
				db_set_dw(0, __DbModName, "HideShow", code_HideShow_tmp);
				code_ReadMsg = code_ReadMsg_tmp;
				db_set_dw(0, __DbModName, "ReadMsg", code_ReadMsg_tmp);
				break; //case PSN_APPLY
			}
			break; //case 0
		}
		break; //case WM_NOTIFY

	case WM_DESTROY:
		hDialogWnd = 0; // используется для вывода туда в реалтайме сканкодов клавы (пока открыты настройки), 0 (при закрытии диалога) блокирует это
		break;
	}

	return 0;
}

