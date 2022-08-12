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

#include "stdafx.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HWND hwnd_frame = NULL;
HWND hwnd_container = NULL;

int frame_id = -1;

#define H_SPACE 2

// Functions //////////////////////////////////////////////////////////////////////////////////////

static int GetMaxLineHeight()
{
	return max(ICON_SIZE, font_max_height) + 1;
}

BOOL FrameIsFloating(int id)
{
	if (id == -1)
		return TRUE; // no frames, always floating

	return CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, id), 0);
}

void ResizeFrame(int id, HWND hwnd)
{
	int height = calls.getCount() * GetMaxLineHeight();
	if (height > 0)
		height += 2;

	if (CanCallNumber()) {
		height += 23 + 2;

		if (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED) {
			RECT first, last;
			GetWindowRect(GetDlgItem(hwnd, IDC_1), &first);
			GetWindowRect(GetDlgItem(hwnd, IDC_SHARP), &last);

			height += last.bottom - first.top + 1;
		}
	}

	if (FrameIsFloating(id)) {
		HWND parent = GetParent(hwnd);
		if (parent == NULL)
			return;

		RECT r_client;
		GetClientRect(hwnd, &r_client);

		if (r_client.bottom - r_client.top == height)
			return;

		RECT parent_client, parent_window, r_window;
		GetClientRect(parent, &parent_client);
		GetWindowRect(parent, &parent_window);
		GetWindowRect(hwnd, &r_window);

		int diff = (parent_window.bottom - parent_window.top) - (parent_client.bottom - parent_client.top);
		if (g_plugin.bFramesExist)
			diff += (r_window.top - parent_window.top);

		SetWindowPos(parent, 0, 0, 0, parent_window.right - parent_window.left, height + diff, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	else {
		int old_height = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, id), 0);
		if (old_height == height)
			return;

		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, id), (LPARAM)height);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)id, (LPARAM)(FU_TBREDRAW | FU_FMREDRAW | FU_FMPOS));
	}
}

void ShowFrame(int id, HWND hwnd, int show)
{
	if (!g_plugin.bFramesExist || id == -1) {
		ShowWindow(GetParent(hwnd), show);
		return;
	}

	BOOL bIsVisible = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id)) & F_VISIBLE;
	if ((bIsVisible && show == SW_HIDE) || (!bIsVisible && show == SW_SHOW))
		CallService(MS_CLIST_FRAMES_SHFRAME, id, 0);		
}

/////////////////////////////////////////////////////////////////////////////////////////

static int dialCtrls[] = {
	IDC_DIALPAD, IDC_NUMBER, IDC_CALL,
	IDC_1, IDC_2, IDC_3,
	IDC_4, IDC_5, IDC_6,
	IDC_7, IDC_8, IDC_9,
	IDC_AST, IDC_0, IDC_SHARP
};

static wchar_t *lstrtrim(wchar_t *str)
{
	int len = lstrlen(str);

	int i;
	for (i = len - 1; i >= 0 && (str[i] == ' ' || str[i] == '\t'); --i);
	if (i < len - 1) {
		++i;
		str[i] = _T('\0');
		len = i;
	}

	for (i = 0; i < len && (str[i] == ' ' || str[i] == '\t'); ++i);
	if (i > 0)
		memmove(str, &str[i], (len - i + 1) * sizeof(wchar_t));

	return str;
}

static void InvalidateAll(HWND hwnd)
{
	InvalidateRect(GetDlgItem(hwnd, IDC_CALLS), NULL, FALSE);
	for (int i = 0; i < _countof(dialCtrls); ++i)
		InvalidateRect(GetDlgItem(hwnd, dialCtrls[i]), NULL, FALSE);
	InvalidateRect(hwnd, NULL, FALSE);

	if (frame_id != -1)
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_FMREDRAW));
}

static void ShowHideDialpad(HWND hwnd)
{
	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

	if (!CanCallNumber()) {
		for (int i = 0; i < _countof(dialCtrls); ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), SW_HIDE);
	}
	else {
		int i;
		for (i = 0; i < 3; ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), SW_SHOW);

		bool showDialpad = (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED);

		for (i = 3; i < _countof(dialCtrls); ++i)
			ShowWindow(GetDlgItem(hwnd, dialCtrls[i]), showDialpad ? SW_SHOW : SW_HIDE);

		VoiceCall *talking = NULL;
		bool ringing = false;
		bool calling = false;
		for (i = 0; i < calls.getCount(); i++) {
			VoiceCall *call = &calls[i];
			if (call->state == VOICE_STATE_TALKING)
				talking = call;
			else if (call->state == VOICE_STATE_CALLING)
				calling = true;
			else if (call->state == VOICE_STATE_RINGING)
				ringing = true;
		}

		wchar_t number[1024];
		GetDlgItemText(hwnd, IDC_NUMBER, number, _countof(number));
		lstrtrim(number);

		if (ringing && number[0] != 0) {
			SetWindowText(GetDlgItem(hwnd, IDC_NUMBER), _T(""));
			number[0] = 0;
		}

		if (ringing || calling) {
			for (i = 0; i < _countof(dialCtrls); ++i)
				EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), FALSE);
		}
		else if (talking) {
			if (!showDialpad || !talking->CanSendDTMF()) {
				for (i = 0; i < _countof(dialCtrls); ++i)
					EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), FALSE);

				EnableWindow(GetDlgItem(hwnd, IDC_DIALPAD), TRUE);
			}
			else {
				for (i = 0; i < _countof(dialCtrls); ++i)
					EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), TRUE);

				EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CALL), FALSE);
			}
		}
		else {
			for (i = 0; i < _countof(dialCtrls); ++i)
				EnableWindow(GetDlgItem(hwnd, dialCtrls[i]), TRUE);

			EnableWindow(GetDlgItem(hwnd, IDC_CALL), CanCall(number));
		}
	}

	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

	InvalidateAll(hwnd);
}

static int sttCompareProvidesByDescription(const VoiceProvider *p1, const VoiceProvider *p2)
{
	return lstrcmp(p2->description, p1->description);
}

static void DrawIconLib(HDC hDC, const RECT &rc, int iconId)
{
	HICON hIcon = g_plugin.getIcon(iconId);
	if (hIcon == NULL)
		return;

	DrawIconEx(hDC, rc.left, (rc.top + rc.bottom - ICON_SIZE) / 2, hIcon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
	IcoLib_ReleaseIcon(hIcon);
}

static INT_PTR CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_CREATE:
	case WM_INITDIALOG:
		SendDlgItemMessage(hwnd, IDC_DIALPAD, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwnd, IDC_DIALPAD, BUTTONSETASPUSHBTN, TRUE, 0);
		SendDlgItemMessageA(hwnd, IDC_DIALPAD, BUTTONADDTOOLTIP, (LPARAM)Translate("Show dialpad"), 0);
		SendDlgItemMessage(hwnd, IDC_DIALPAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("vc_dialpad", TRUE));

		SendDlgItemMessage(hwnd, IDC_CALL, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessageA(hwnd, IDC_CALL, BUTTONADDTOOLTIP, (LPARAM)Translate("Make call"), 0);
		SendDlgItemMessage(hwnd, IDC_CALL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("vca_call", TRUE));

		PostMessage(hwnd, WMU_RESIZE_FRAME, 0, 1);
		break;

	case WM_SIZE:
		SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
		GetClientRect(hwnd, &rc);
		{
			int width = rc.right - rc.left;
			int height = rc.bottom - rc.top;

			if (CanCallNumber()) {
				bool showDialpad = (SendMessage(GetDlgItem(hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED);

				GetWindowRect(hwnd, &rc);

				RECT first = { 0 }, last = { 0 };
				GetWindowRect(GetDlgItem(hwnd, IDC_1), &first);
				GetWindowRect(GetDlgItem(hwnd, IDC_SHARP), &last);

				int dialpad_height = last.bottom - first.top;
				int dialpad_width = last.right - first.left;


				int call_height = 23;
				int call_width = 25;
				int top = height - call_height - 1;

				if (showDialpad)
					top -= dialpad_height + 1;

				MoveWindow(GetDlgItem(hwnd, IDC_DIALPAD), 1, top, call_width - 2, call_height, FALSE);
				MoveWindow(GetDlgItem(hwnd, IDC_NUMBER), call_width, top, width - 2 * call_width, call_height, FALSE);
				MoveWindow(GetDlgItem(hwnd, IDC_CALL), width - call_width, top, call_width, call_height + 1, FALSE);

				int dialpad_top = top + call_height + 1;
				int dialpad_left = ((rc.right - rc.left) - dialpad_width) / 2;
				int deltaX = dialpad_left - first.left;
				int deltaY = dialpad_top - first.top;
				for (int i = 3; i < _countof(dialCtrls); ++i) {
					GetWindowRect(GetDlgItem(hwnd, dialCtrls[i]), &rc);
					MoveWindow(GetDlgItem(hwnd, dialCtrls[i]), rc.left + deltaX, rc.top + deltaY, rc.right - rc.left, rc.bottom - rc.top, FALSE);
				}

				height -= call_height + 2;
				if (showDialpad)
					height -= dialpad_height + 1;
			}

			if (height <= 2) {
				ShowWindow(GetDlgItem(hwnd, IDC_CALLS), SW_HIDE);
			}
			else {
				MoveWindow(GetDlgItem(hwnd, IDC_CALLS), 1, 1, width - 2, height - 2, FALSE);
				ShowWindow(GetDlgItem(hwnd, IDC_CALLS), SW_SHOW);
			}
		}
		SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
		InvalidateAll(hwnd);
		break;

	case WMU_REFRESH:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);

			SendMessage(list, WM_SETREDRAW, FALSE, 0);
			SendMessage(list, LB_RESETCONTENT, 0, 0);
			for (int i = 0; i < calls.getCount(); i++) {
				VoiceCall *call = &calls[i];

				wchar_t text[512];
				mir_snwprintf(text, _countof(text), _T("%d %s"), call->state, call->displayName);

				int pos = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)text);
				if (pos == LB_ERR)
					// TODO Show error
					continue;

				SendMessage(list, LB_SETITEMDATA, pos, (LPARAM)call);
			}
			SendMessage(list, WM_SETREDRAW, TRUE, 0);
		}
		__fallthrough;

	case WMU_RESIZE_FRAME:
		ShowHideDialpad(hwnd);

		if (lParam)
			SendMessage(hwnd, WM_SIZE, 0, 0);

		if (opts.resize_frame) {
			if (calls.getCount() == 0 && !CanCallNumber()) {
				ShowFrame(frame_id, hwnd, SW_HIDE);
			}
			else {
				ResizeFrame(frame_id, hwnd);
				ShowFrame(frame_id, hwnd, SW_SHOW);
			}
		}

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CALL:
			{
				wchar_t number[1024];
				GetDlgItemText(hwnd, IDC_NUMBER, number, _countof(number));
				lstrtrim(number);

				LIST<VoiceProvider> candidates(10, &sttCompareProvidesByDescription);

				for (int i = 0; i < modules.getCount(); i++) {
					if (!modules[i].CanCall(number))
						continue;

					candidates.insert(&modules[i]);
				}

				if (candidates.getCount() < 1)
					break;

				int selected;
				if (candidates.getCount() == 1) {
					selected = 0;
				}
				else {
					HMENU menu = CreatePopupMenu();

					for (int i = 0; i < candidates.getCount(); ++i) {
						wchar_t text[1024];
						mir_snwprintf(text, _countof(text), TranslateT("Call with %s"), candidates[i]->description);

						MENUITEMINFO mii = { 0 };
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_ID | MIIM_TYPE;
						mii.fType = MFT_STRING;
						mii.dwTypeData = text;
						mii.cch = lstrlen(text);
						mii.wID = i + 1;

						// TODO: Add icon to menu

						InsertMenuItem(menu, 0, TRUE, &mii);
					}

					GetWindowRect(GetDlgItem(hwnd, IDC_CALL), &rc);

					POINT p;
					p.x = rc.right;
					p.y = rc.bottom + 1;

					selected = TrackPopupMenu(menu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_RIGHTALIGN,
						p.x, p.y, 0, hwnd, NULL);

					DestroyMenu(menu);

					if (selected == 0)
						break;

					selected--;
				}

				candidates[selected]->Call(NULL, number);
				break;
			}

		case IDC_NUMBER:
			if (HIWORD(wParam) == EN_CHANGE)
				ShowHideDialpad(hwnd);
			break;

		case IDC_DIALPAD:
			ShowHideDialpad(hwnd);
			SendMessage(hwnd, WMU_RESIZE_FRAME, 0, 0);
			break;

		case IDC_1:
		case IDC_2:
		case IDC_3:
		case IDC_4:
		case IDC_5:
		case IDC_6:
		case IDC_7:
		case IDC_8:
		case IDC_9:
		case IDC_AST:
		case IDC_0:
		case IDC_SHARP:
			wchar_t text[2];
			switch (LOWORD(wParam)) {
			case IDC_1: text[0] = _T('1'); break;
			case IDC_2: text[0] = _T('2'); break;
			case IDC_3: text[0] = _T('3'); break;
			case IDC_4: text[0] = _T('4'); break;
			case IDC_5: text[0] = _T('5'); break;
			case IDC_6: text[0] = _T('6'); break;
			case IDC_7: text[0] = _T('7'); break;
			case IDC_8: text[0] = _T('8'); break;
			case IDC_9: text[0] = _T('9'); break;
			case IDC_AST: text[0] = _T('*'); break;
			case IDC_0: text[0] = _T('0'); break;
			case IDC_SHARP: text[0] = _T('#'); break;
			}
			text[1] = 0;

			Skin_PlaySound("voice_dialpad");
			{
				VoiceCall *call = GetTalkingCall();
				if (call == NULL) {
					SendMessage(GetDlgItem(hwnd, IDC_NUMBER), EM_REPLACESEL, TRUE, (LPARAM)text);
				}
				else {
					wchar_t tmp[1024];

					GetWindowText(GetDlgItem(hwnd, IDC_NUMBER), tmp, _countof(tmp));

					tmp[_countof(tmp) - 2] = 0;
					lstrcat(tmp, text);

					SetWindowText(GetDlgItem(hwnd, IDC_NUMBER), tmp);

					call->SendDTMF(text[0]);
				}
			}
			break;

		case IDC_CALLS:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				HWND list = GetDlgItem(hwnd, IDC_CALLS);

				int pos = SendMessage(list, LB_GETCURSEL, 0, 0);
				if (pos == LB_ERR)
					break;

				POINT p;
				GetCursorPos(&p);
				ScreenToClient(list, &p);

				int ret = SendMessage(list, LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
				if (HIWORD(ret))
					break;
				if (pos != LOWORD(ret))
					break;

				SendMessage(list, LB_GETITEMRECT, pos, (LPARAM)&rc);
				int x = rc.right - p.x;

				int action;
				if (x >= H_SPACE && x <= ICON_SIZE + H_SPACE)
					action = 2;
				else if (x >= ICON_SIZE + 2 * H_SPACE && x <= 2 * (ICON_SIZE + H_SPACE))
					action = 1;
				else
					break;

				VoiceCall *call = (VoiceCall *)SendMessage(list, LB_GETITEMDATA, pos, 0);
				switch (call->state) {
				case VOICE_STATE_TALKING:
					if (action == 1)
						call->Hold();
					else
						call->Drop();
					break;

				case VOICE_STATE_RINGING:
				case VOICE_STATE_ON_HOLD:
					if (action == 1)
						Answer(call);
					else
						call->Drop();
					break;

				case VOICE_STATE_CALLING:
					if (action == 2)
						call->Drop();
					break;
				}
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND list = GetDlgItem(hwnd, IDC_CALLS);
			if ((HANDLE)wParam != list)
				break;

			POINT p;
			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);
			ScreenToClient(list, &p);

			int pos = SendMessage(list, LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
			if (HIWORD(pos))
				break;
			pos = LOWORD(pos);

			if (pos >= calls.getCount())
				break;

			if (IsFinalState(calls[pos].state))
				break;

			// Just to get things strait
			SendMessage(list, LB_SETCURSEL, pos, 0);

			HMENU menu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENUS));
			HMENU submenu = GetSubMenu(menu, 0);
			TranslateMenu(submenu);

			switch (calls[pos].state) {
			case VOICE_STATE_CALLING:
				DeleteMenu(menu, ID_FRAMEPOPUP_ANSWERCALL, MF_BYCOMMAND);
				DeleteMenu(menu, ID_FRAMEPOPUP_HOLDCALL, MF_BYCOMMAND);
				break;

			case VOICE_STATE_TALKING:
				DeleteMenu(menu, ID_FRAMEPOPUP_ANSWERCALL, MF_BYCOMMAND);
				if (!calls[pos].module->CanHold())
					DeleteMenu(menu, ID_FRAMEPOPUP_HOLDCALL, MF_BYCOMMAND);
				break;
			}

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);
			int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);
			DestroyMenu(menu);

			switch (ret) {
			case ID_FRAMEPOPUP_DROPCALL:
				calls[pos].Drop();
				break;

			case ID_FRAMEPOPUP_ANSWERCALL:
				Answer(&calls[pos]);
				break;

			case ID_FRAMEPOPUP_HOLDCALL:
				calls[pos].Hold();
				break;
			}
			break;
		}

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			if (mis->CtlID != IDC_CALLS)
				break;

			mis->itemHeight = GetMaxLineHeight();
		}
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		return (LRESULT)bk_brush;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlID != IDC_CALLS || dis->itemID == -1)
				break;

			VoiceCall *call = (VoiceCall *)dis->itemData;
			if (call == NULL)
				break;

			rc = dis->rcItem;

			FillRect(dis->hDC, &rc, bk_brush);

			rc.left += H_SPACE;
			rc.right -= H_SPACE;
			rc.bottom--;

			int old_bk_mode = SetBkMode(dis->hDC, TRANSPARENT);

			// Draw status
			DrawIconLib(dis->hDC, rc, IDI_TALKING + call->state);

			if (call->secure)
				DrawIconLib(dis->hDC, rc, IDI_SECURE);

			// Draw voice provider icon
			rc.left += ICON_SIZE + H_SPACE;

			HICON hIcon = call->module->GetIcon();
			if (hIcon != NULL) {
				DrawIconEx(dis->hDC, rc.left, (rc.top + rc.bottom - ICON_SIZE) / 2, hIcon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
				call->module->ReleaseIcon(hIcon);
			}

			// Draw contact
			rc.left += ICON_SIZE + H_SPACE;

			int numIcons = 0;
			switch (call->state) {
			case VOICE_STATE_CALLING:
				numIcons = 1;
				break;

			case VOICE_STATE_TALKING:
				if (call->module->CanHold())
					numIcons = 2;
				else
					numIcons = 1;
				break;

			case VOICE_STATE_RINGING:
			case VOICE_STATE_ON_HOLD:
				numIcons = 2;
				break;
			}

			rc.right -= numIcons * (ICON_SIZE + H_SPACE);

			HFONT old_font = (HFONT)SelectObject(dis->hDC, fonts[call->state]);
			COLORREF old_color = SetTextColor(dis->hDC, font_colors[call->state]);

			DrawText(dis->hDC, call->displayName, -1, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);

			SelectObject(dis->hDC, old_font);
			SetTextColor(dis->hDC, old_color);

			// Draw action icons
			rc = dis->rcItem;
			rc.right -= H_SPACE;
			rc.bottom--;

			switch (call->state) {
			case VOICE_STATE_CALLING:
				rc.left = rc.right - ICON_SIZE;
				DrawIconLib(dis->hDC, rc, IDI_ACTION_DROP);
				break;

			case VOICE_STATE_TALKING:
				rc.left = rc.right - ICON_SIZE;
				DrawIconLib(dis->hDC, rc, IDI_ACTION_DROP);

				if (call->module->CanHold()) {
					rc.right -= ICON_SIZE + H_SPACE;
					rc.left = rc.right - ICON_SIZE;
					DrawIconLib(dis->hDC, rc, IDI_ACTION_HOLD);
				}
				break;

			case VOICE_STATE_RINGING:
			case VOICE_STATE_ON_HOLD:
				rc.left = rc.right - ICON_SIZE;
				DrawIconLib(dis->hDC, rc, IDI_ACTION_DROP);

				rc.right -= ICON_SIZE + H_SPACE;
				rc.left = rc.right - ICON_SIZE;
				DrawIconLib(dis->hDC, rc, IDI_ACTION_ANSWER);
				break;
			}

			SetBkMode(dis->hDC, old_bk_mode);
			return TRUE;
		}

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Module entry point /////////////////////////////////////////////////////////////////////////////

void InitFrames()
{
	if (g_plugin.bFramesExist) {
		hwnd_frame = CreateDialogW(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CALLS), g_clistApi.hwndContactList, FrameWndProc);

		CLISTFrame Frame = {};
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.szName.w = TranslateT("Voice Calls");
		Frame.hWnd = hwnd_frame;
		Frame.height = ICON_SIZE;
		Frame.align = alBottom;
		Frame.Flags = F_NOBORDER | F_LOCKED | F_UNICODE;
		Frame.hIcon = g_plugin.getIcon(IDI_MAIN, true);
		frame_id = CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

		int flags = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), 0);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frame_id), flags & ~F_VISIBLE);
	}
}

void DeInitFrames()
{
	if (g_plugin.bFramesExist && frame_id != -1)
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, frame_id, 0);

	if (hwnd_frame != NULL)
		DestroyWindow(hwnd_frame);
}
