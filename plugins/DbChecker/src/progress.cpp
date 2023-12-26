/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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
*/

#include "stdafx.h"

#define WM_PROCESSINGDONE  (WM_USER+1)
#define WM_ADDMESSAGE      (WM_USER+2)

void __cdecl WorkerThread(DbToolOptions *opts);
static HWND hwndStatus, hdlgProgress, hwndBar;
HWND hwndRichEdit;
int errorCount;
extern HFONT hBoldFont;

void AddToStatus(int flags, const wchar_t* fmt, ...)
{
	va_list vararg;
	va_start(vararg, fmt);

	wchar_t str[256];
	mir_vsnwprintf(str, _countof(str), fmt, vararg);
	va_end(vararg);

	PostMessage(hdlgProgress, WM_ADDMESSAGE, WPARAM(mir_wstrdup(str)), flags);

#ifdef _DEBUG
	OutputDebugString(str);
	OutputDebugStringA("\n");
#endif

	switch (flags & STATUS_CLASSMASK) {
	case STATUS_ERROR:
	case STATUS_FATAL:
		errorCount++;
	}
}

void SetProgressBar(int perThou)
{
	SendMessage(hwndBar, PBM_SETPOS, perThou, 0);
}

void ProcessingDone(void)
{
	SendMessage(hdlgProgress, WM_PROCESSINGDONE, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CProgressDlg implementation

CProgressPageDlg::CProgressPageDlg() :
	CSuper(IDD_PROGRESS)
{
}

bool CProgressPageDlg::OnInitDialog()
{
	CSuper::OnInitDialog();

	EnableWindow(GetDlgItem(m_hwndParent, IDOK), FALSE);
	hdlgProgress = m_hwnd;
	hwndStatus = GetDlgItem(m_hwnd, IDC_STATUS);
	hwndRichEdit = GetDlgItem(m_hwnd, IDC_MEMO);
	errorCount = 0;
	hwndBar = GetDlgItem(m_hwnd, IDC_PROGRESS);
	SendMessage(hwndBar, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
	{
		HDC hdc = GetDC(nullptr);
		HFONT hFont = (HFONT)SendMessage(m_hwnd, WM_GETFONT, 0, 0);
		HFONT hoFont = (HFONT)SelectObject(hdc, hFont);

		SIZE s;
		GetTextExtentPoint32(hdc, L"x", 1, &s);
		SelectObject(hdc, hoFont);
		ReleaseDC(nullptr, hdc);
		fontHeight = s.cy;

		RECT rc;
		GetClientRect(GetDlgItem(m_hwnd, IDC_STATUS), &rc);
		listWidth = rc.right;

		LOGFONT lf;
		GetObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_STATUS, WM_GETFONT, 0, 0), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		hBoldFont = CreateFontIndirect(&lf);
	}
	manualAbort = 0;
	TranslateDialogDefault(m_hwnd);
	mir_forkThread<DbToolOptions>(WorkerThread, getOpts());
	return true;
}

void CProgressPageDlg::OnDestroy()
{
	if (hBoldFont) {
		DeleteObject(hBoldFont);
		hBoldFont = nullptr;
	}
}

int CProgressPageDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_STATUS:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}

	return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
}

void CProgressPageDlg::OnNext()
{
	EndDialog(m_hwndParent, 1);
}

INT_PTR CProgressPageDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			mis->itemWidth = listWidth;
			mis->itemHeight = fontHeight;
		}
		return TRUE;

	case WM_DRAWITEM:
		wchar_t str[256];
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			int bold = 0;
			HFONT hoFont = nullptr;
			if ((int)dis->itemID == -1) break;
			SendMessage(dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)str);
			switch (dis->itemData & STATUS_CLASSMASK) {
			case STATUS_MESSAGE:
				SetTextColor(dis->hDC, RGB(0, 0, 0));
				break;
			case STATUS_WARNING:
				SetTextColor(dis->hDC, RGB(192, 128, 0));
				break;
			case STATUS_ERROR:
				SetTextColor(dis->hDC, RGB(192, 0, 0));
				break;
			case STATUS_FATAL:
				bold = 1;
				SetTextColor(dis->hDC, RGB(192, 0, 0));
				break;
			case STATUS_SUCCESS:
				bold = 1;
				SetTextColor(dis->hDC, RGB(0, 192, 0));
				break;
			}
			if (bold)
				hoFont = (HFONT)SelectObject(dis->hDC, hBoldFont);
			ExtTextOut(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_CLIPPED | ETO_OPAQUE, &dis->rcItem, str, (UINT)mir_wstrlen(str), nullptr);
			if (bold)
				SelectObject(dis->hDC, hoFont);
		}
		return TRUE;

	case WM_PROCESSINGDONE:
		SetProgressBar(1000);
		if (getOpts()->bAutoExit)
			PostMessage(GetParent(m_hwnd), WM_COMMAND, IDCANCEL, 0);

		SetDlgItemText(GetParent(m_hwnd), IDCANCEL, TranslateT("&Finish"));
		AddToStatus(STATUS_SUCCESS, TranslateT("Click Finish to continue"));

		if (manualAbort == 1)
			EndDialog(GetParent(m_hwnd), 0);
		else if (manualAbort == 2) {
			changePage(new CProgressPageDlg());
			break;
		}
		break;

	case WM_ADDMESSAGE:
		{
			int i = SendMessage(hwndStatus, LB_ADDSTRING, 0, wParam);
			SendMessage(hwndStatus, LB_SETITEMDATA, i, lParam);
			InvalidateRect(hwndStatus, nullptr, FALSE);
			SendMessage(hwndStatus, LB_SETTOPINDEX, i, 0);
			mir_free((void *)wParam);
		}
		break;

	case WZN_CANCELCLICKED:
		{
			auto *opts = getOpts();
			ResetEvent(opts->hEventRun);
			if (getOpts()->bFinished)
				break;

			if (MessageBox(m_hwnd, TranslateT("Processing has not yet completed, if you cancel now then the changes that have currently been made will be rolled back and the original database will be restored. Do you still want to cancel?"), TranslateT("Miranda Database Tool"), MB_YESNO) == IDYES) {
				manualAbort = 1;
				SetEvent(opts->hEventAbort);
			}
			SetEvent(opts->hEventRun);
		}
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
		return TRUE;
	}
	
	return CSuper::DlgProc(msg, wParam, lParam);
}
