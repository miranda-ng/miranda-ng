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

#define H_SPACE 2

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

int frame_id = -1;

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

static void ShowFrame(int id, HWND hwnd, int show)
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
	for (auto &it : dialCtrls)
		InvalidateRect(GetDlgItem(hwnd, it), NULL, FALSE);
	InvalidateRect(hwnd, NULL, FALSE);

	if (frame_id != -1)
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)frame_id, (LPARAM)(FU_FMREDRAW));
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

class CFrameDlg : public CDlgBase
{
	CCtrlEdit edtNumber;
	CCtrlListBox m_calls;
	CCtrlMButton btnDialpad, btnCall;

	void ResizeFrame(bool bResizeAll)
	{
		ShowHideDialpad(0);

		if (bResizeAll)
			SendMessage(m_hwnd, WM_SIZE, 0, 0);

		if (opts.resize_frame) {
			if (calls.getCount() == 0 && !CanCallNumber()) {
				ShowFrame(frame_id, m_hwnd, SW_HIDE);
			}
			else {
				::ResizeFrame(frame_id, m_hwnd);
				ShowFrame(frame_id, m_hwnd, SW_SHOW);
			}
		}
	}

	void ShowHideDialpad(CCtrlEdit *)
	{
		SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);

		if (!CanCallNumber()) {
			for (auto &it : dialCtrls)
				ShowWindow(GetDlgItem(m_hwnd, it), SW_HIDE);
		}
		else {
			for (int i = 0; i < 3; ++i)
				ShowWindow(GetDlgItem(m_hwnd, dialCtrls[i]), SW_SHOW);

			bool showDialpad = (SendMessage(GetDlgItem(m_hwnd, IDC_DIALPAD), BM_GETCHECK, 0, 0) == BST_CHECKED);

			for (int i = 3; i < _countof(dialCtrls); ++i)
				ShowWindow(GetDlgItem(m_hwnd, dialCtrls[i]), showDialpad ? SW_SHOW : SW_HIDE);

			VoiceCall *talking = NULL;
			bool ringing = false;
			bool calling = false;
			for (auto &call : calls) {
				if (call->state == VOICE_STATE_TALKING)
					talking = call;
				else if (call->state == VOICE_STATE_CALLING)
					calling = true;
				else if (call->state == VOICE_STATE_RINGING)
					ringing = true;
			}

			wchar_t number[1024];
			GetDlgItemText(m_hwnd, IDC_NUMBER, number, _countof(number));
			lstrtrim(number);

			if (ringing && number[0] != 0) {
				SetWindowText(GetDlgItem(m_hwnd, IDC_NUMBER), _T(""));
				number[0] = 0;
			}

			if (ringing || calling) {
				for (auto &it : dialCtrls)
					EnableWindow(GetDlgItem(m_hwnd, it), FALSE);
			}
			else if (talking) {
				if (!showDialpad || !talking->CanSendDTMF()) {
					for (auto &it : dialCtrls)
						EnableWindow(GetDlgItem(m_hwnd, it), FALSE);

					EnableWindow(GetDlgItem(m_hwnd, IDC_DIALPAD), TRUE);
				}
				else {
					for (auto &it : dialCtrls)
						EnableWindow(GetDlgItem(m_hwnd, it), TRUE);

					EnableWindow(GetDlgItem(m_hwnd, IDC_NUMBER), FALSE);
					EnableWindow(GetDlgItem(m_hwnd, IDC_CALL), FALSE);
				}
			}
			else {
				for (auto &it : dialCtrls)
					EnableWindow(GetDlgItem(m_hwnd, it), TRUE);

				EnableWindow(GetDlgItem(m_hwnd, IDC_CALL), CanCall(number));
			}
		}

		SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);

		InvalidateAll(m_hwnd);
	}

public:
	CFrameDlg() :
		CDlgBase(g_plugin, IDD_CALLS),
		m_calls(this, IDC_CALLS),
		edtNumber(this, IDC_NUMBER),
		btnCall(this, IDC_CALL, g_plugin.getIcon(IDI_ACTION_CALL), "Make call"),
		btnDialpad(this, IDC_DIALPAD, g_plugin.getIcon(IDI_DIALPAD), LPGEN("Show dialpad"))
	{
		SetParent(g_clistApi.hwndContactList);

		btnCall.OnClick = Callback(this, &CFrameDlg::onClick_Call);
		btnDialpad.OnClick = Callback(this, &CFrameDlg::onClick_DialPad);

		edtNumber.OnChange = Callback(this, &CFrameDlg::ShowHideDialpad);

		m_calls.OnSelChange = Callback(this, &CFrameDlg::onSelChange_Calls);
	}

	bool OnInitDialog() override
	{
		btnCall.MakeFlat();
		btnDialpad.MakePush();

		ResizeFrame(true);
		return true;
	}

	void OnResize() override
	{
		SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);

		RECT rc;
		GetClientRect(m_hwnd, &rc);

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		if (CanCallNumber()) {
			bool showDialpad = btnDialpad.IsPushed();

			GetWindowRect(m_hwnd, &rc);

			RECT first = { 0 }, last = { 0 };
			GetWindowRect(GetDlgItem(m_hwnd, IDC_1), &first);
			GetWindowRect(GetDlgItem(m_hwnd, IDC_SHARP), &last);

			int dialpad_height = last.bottom - first.top;
			int dialpad_width = last.right - first.left;


			int call_height = 23;
			int call_width = 25;
			int top = height - call_height - 1;

			if (showDialpad)
				top -= dialpad_height + 1;

			MoveWindow(btnDialpad.GetHwnd(), 1, top, call_width - 2, call_height, FALSE);
			MoveWindow(GetDlgItem(m_hwnd, IDC_NUMBER), call_width, top, width - 2 * call_width, call_height, FALSE);
			MoveWindow(btnCall.GetHwnd(), width - call_width, top, call_width, call_height + 1, FALSE);

			int dialpad_top = top + call_height + 1;
			int dialpad_left = ((rc.right - rc.left) - dialpad_width) / 2;
			int deltaX = dialpad_left - first.left;
			int deltaY = dialpad_top - first.top;
			for (int i = 3; i < _countof(dialCtrls); ++i) {
				GetWindowRect(GetDlgItem(m_hwnd, dialCtrls[i]), &rc);
				MoveWindow(GetDlgItem(m_hwnd, dialCtrls[i]), rc.left + deltaX, rc.top + deltaY, rc.right - rc.left, rc.bottom - rc.top, FALSE);
			}

			height -= call_height + 2;
			if (showDialpad)
				height -= dialpad_height + 1;
		}

		if (height <= 2) {
			m_calls.Hide();
		}
		else {
			MoveWindow(m_calls.GetHwnd(), 1, 1, width - 2, height - 2, FALSE);
			m_calls.Show();
		}

		SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
		InvalidateAll(m_hwnd);
	}

	void onClick_DialPad(CCtrlMButton *)
	{
		ShowHideDialpad(0);
		ResizeFrame(false);
	}

	void onClick_Call(CCtrlMButton *)
	{
		wchar_t number[1024];
		GetDlgItemText(m_hwnd, IDC_NUMBER, number, _countof(number));
		lstrtrim(number);

		LIST<VoiceProvider> candidates(10, &sttCompareProvidesByDescription);

		for (auto &it: modules) {
			if (!it->CanCall(number))
				continue;

			candidates.insert(it);
		}

		if (candidates.getCount() < 1)
			return;

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

			RECT rc;
			GetWindowRect(GetDlgItem(m_hwnd, IDC_CALL), &rc);

			POINT p;
			p.x = rc.right;
			p.y = rc.bottom + 1;

			selected = TrackPopupMenu(menu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_RIGHTALIGN, p.x, p.y, 0, m_hwnd, 0);

			DestroyMenu(menu);

			if (selected == 0)
				return;

			selected--;
		}

		candidates[selected]->Call(0, number);
	}

	void onChange_Dialpad(CCtrlCheck *pCheck)
	{
		wchar_t text[2];
		switch (pCheck->GetCtrlId()) {
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

		VoiceCall *call = GetTalkingCall();
		if (call == NULL) {
			SendMessage(GetDlgItem(m_hwnd, IDC_NUMBER), EM_REPLACESEL, TRUE, (LPARAM)text);
		}
		else {
			wchar_t tmp[1024];

			GetWindowText(GetDlgItem(m_hwnd, IDC_NUMBER), tmp, _countof(tmp));

			tmp[_countof(tmp) - 2] = 0;
			lstrcat(tmp, text);

			SetWindowText(GetDlgItem(m_hwnd, IDC_NUMBER), tmp);

			call->SendDTMF(text[0]);
		}
	}

	void onSelChange_Calls(CCtrlListBox*)
	{
		int pos = m_calls.GetCurSel();
		if (pos == LB_ERR)
			return;

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(m_calls.GetHwnd(), &p);

		int ret = m_calls.SendMsg(LB_ITEMFROMPOINT, 0, MAKELONG(p.x, p.y));
		if (HIWORD(ret))
			return;
		if (pos != LOWORD(ret))
			return;

		RECT rc;
		m_calls.GetItemRect(pos, &rc);
		int x = rc.right - p.x;

		int action;
		if (x >= H_SPACE && x <= ICON_SIZE + H_SPACE)
			action = 2;
		else if (x >= ICON_SIZE + 2 * H_SPACE && x <= 2 * (ICON_SIZE + H_SPACE))
			action = 1;
		else
			return;

		VoiceCall *call = (VoiceCall *)m_calls.GetItemData(pos);
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

	void onMenu_Calls(CContextMenuPos *pPos)
	{
		int pos = pPos->iCurr;
		if (pos >= calls.getCount())
			return;

		if (IsFinalState(calls[pos].state))
			return;

		// Just to get things strait
		m_calls.SetCurSel(pos);

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

		int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pPos->pt.x, pPos->pt.y, 0, m_hwnd, NULL);
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
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
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
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlID != IDC_CALLS || dis->itemID == -1)
				break;

			VoiceCall *call = (VoiceCall *)dis->itemData;
			if (call == NULL)
				break;

			RECT rc = dis->rcItem;

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

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void Refresh()
	{
		if (m_calls.GetCount() == calls.getCount())
			return;

		m_calls.SendMsg(WM_SETREDRAW, FALSE, 0);
		m_calls.ResetContent();

		for (auto &call : calls) {
			wchar_t text[512];
			mir_snwprintf(text, _countof(text), _T("%d %s"), call->state, call->displayName);
			m_calls.AddString(text, (LPARAM)call);
		}

		m_calls.SendMsg(WM_SETREDRAW, TRUE, 0);
		ResizeFrame(false);
	}
};

// Module entry point /////////////////////////////////////////////////////////////////////////////

static CFrameDlg *pDialog = nullptr;

void InitFrames()
{
	if (g_plugin.bFramesExist) {
		pDialog = new CFrameDlg();

		CLISTFrame Frame = {};
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.szName.w = TranslateT("Voice Calls");
		Frame.hWnd = pDialog->GetHwnd();
		Frame.height = ICON_SIZE;
		Frame.align = alBottom;
		Frame.Flags = F_NOBORDER | F_LOCKED | F_UNICODE;
		Frame.hIcon = g_plugin.getIcon(IDI_MAIN, true);
		frame_id = g_plugin.addFrame(&Frame);

		ShowFrame(frame_id, pDialog->GetHwnd(), SW_HIDE);
	}
}

void RefreshFrame()
{
	if (pDialog)
		pDialog->Refresh();
}

void DeInitFrames()
{
	if (g_plugin.bFramesExist && frame_id != -1)
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, frame_id, 0);

	if (pDialog)
		pDialog->Close();
}
