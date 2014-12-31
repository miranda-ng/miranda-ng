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

#include "dbchecker.h"

#define WM_PROCESSINGDONE  (WM_USER+1)

void __cdecl WorkerThread(void *unused);
static HWND hwndStatus, hdlgProgress, hwndBar;
static bool bShortModeDone;
HANDLE hEventRun = NULL, hEventAbort = NULL;
int errorCount;
LRESULT wizardResult;

void AddToStatus(int flags, const TCHAR* fmt, ...)
{
	va_list vararg;
	va_start(vararg, fmt);

	TCHAR str[256];
	mir_vsntprintf(str, SIZEOF(str), fmt, vararg);
	va_end(vararg);

	int i = SendMessage(hwndStatus, LB_ADDSTRING, 0, (LPARAM)str);
	SendMessage(hwndStatus, LB_SETITEMDATA, i, flags);
	InvalidateRect(hwndStatus, NULL, FALSE);
	SendMessage(hwndStatus, LB_SETTOPINDEX, i, 0);

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

INT_PTR CALLBACK ProgressDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int fontHeight, listWidth;
	static int manualAbort;
	static HFONT hBoldFont = NULL;

	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
		EnableWindow(GetDlgItem(GetParent(hdlg), IDOK), FALSE);
		hdlgProgress = hdlg;
		hwndStatus = GetDlgItem(hdlg, IDC_STATUS);
		errorCount = 0;
		bShortModeDone = false;
		hwndBar = GetDlgItem(hdlg, IDC_PROGRESS);
		SendMessage(hwndBar, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
		{
			HDC hdc;
			HFONT hFont, hoFont;
			SIZE s;
			hdc = GetDC(NULL);
			hFont = (HFONT)SendMessage(hdlg, WM_GETFONT, 0, 0);
			hoFont = (HFONT)SelectObject(hdc, hFont);
			GetTextExtentPoint32(hdc, _T("x"), 1, &s);
			SelectObject(hdc, hoFont);
			ReleaseDC(NULL, hdc);
			fontHeight = s.cy;

			RECT rc;
			GetClientRect(GetDlgItem(hdlg, IDC_STATUS), &rc);
			listWidth = rc.right;

			LOGFONT lf;
			GetObject((HFONT)SendDlgItemMessage(hdlg, IDC_STATUS, WM_GETFONT, 0, 0), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			hBoldFont = CreateFontIndirect(&lf);
		}
		manualAbort = 0;
		hEventRun = CreateEvent(NULL, TRUE, TRUE, NULL);
		hEventAbort = CreateEvent(NULL, TRUE, FALSE, NULL);
		TranslateDialogDefault(hdlg);
		_beginthread(WorkerThread, 0, NULL);
		return TRUE;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			mis->itemWidth = listWidth;
			mis->itemHeight = fontHeight;
		}
		return TRUE;

	case WM_DRAWITEM:
		TCHAR str[256];
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			int bold = 0;
			HFONT hoFont = NULL;
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
			if (bold) hoFont = (HFONT)SelectObject(dis->hDC, hBoldFont);
			ExtTextOut(dis->hDC, dis->rcItem.left, dis->rcItem.top, ETO_CLIPPED | ETO_OPAQUE, &dis->rcItem, str, (UINT)_tcslen(str), NULL);
			if (bold) SelectObject(dis->hDC, hoFont);
		}
		return TRUE;

	case WM_PROCESSINGDONE:
		SetProgressBar(1000);
		if (bShortMode) {
			EnableWindow(GetDlgItem(GetParent(hdlg), IDC_BACK), FALSE);
			EnableWindow(GetDlgItem(GetParent(hdlg), IDOK), FALSE);
			SetDlgItemText(GetParent(hdlg), IDCANCEL, TranslateT("&Finish"));
			bShortModeDone = true;
			if (bAutoExit)
				PostMessage(GetParent(hdlg), WM_COMMAND, IDCANCEL, 0);
		}
		else {
			AddToStatus(STATUS_SUCCESS, TranslateT("Click Next to continue"));
			EnableWindow(GetDlgItem(GetParent(hdlg), IDOK), TRUE);
		}

		if (manualAbort == 1)
			EndDialog(GetParent(hdlg), 0);
		else if (manualAbort == 2) {
			if (opts.bCheckOnly)
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_FILEACCESS, (LPARAM)FileAccessDlgProc);
			else {
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_CLEANING, (LPARAM)CleaningDlgProc);
				CloseHandle(opts.hOutFile);
				opts.hOutFile = NULL;
			}
			break;
		}
		break;

	case WZN_CANCELCLICKED:
		if (bShortModeDone) {
			if (!errorCount) {
				if (bLaunchMiranda)
					CallService(MS_DB_SETDEFAULTPROFILE, (WPARAM)opts.filename, 0);
				wizardResult = 1;
			}
			return TRUE;
		}

		ResetEvent(hEventRun);
		if (IsWindowEnabled(GetDlgItem(GetParent(hdlg), IDOK)))
			break;

		if (MessageBox(hdlg, TranslateT("Processing has not yet completed, if you cancel now then the changes that have currently been made will be rolled back and the original database will be restored. Do you still want to cancel?"), TranslateT("Miranda Database Tool"), MB_YESNO) == IDYES) {
			manualAbort = 1;
			SetEvent(hEventAbort);
		}
		SetEvent(hEventRun);
		SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			ResetEvent(hEventRun);
			if (!IsWindowEnabled(GetDlgItem(GetParent(hdlg), IDOK))) {
				if (MessageBox(hdlg, TranslateT("Processing has not yet completed, if you go back now then the changes that have currently been made will be rolled back and the original database will be restored. Do you still want to go back?"), TranslateT("Miranda Database Tool"), MB_YESNO) == IDYES) {
					manualAbort = 2;
					SetEvent(hEventAbort);
				}
				SetEvent(hEventRun);
				break;
			}
			SetEvent(hEventRun);
			if (opts.bCheckOnly)
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_FILEACCESS, (LPARAM)FileAccessDlgProc);
			else
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_CLEANING, (LPARAM)CleaningDlgProc);
			break;

		case IDOK:
			PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_FINISHED, (LPARAM)FinishedDlgProc);
			break;
		}
		break;

	case WM_DESTROY:
		if (hEventAbort) {
			CloseHandle(hEventAbort);
			hEventAbort = NULL;
		}
		if (hEventRun) {
			CloseHandle(hEventRun);
			hEventRun = NULL;
		}
		if (hBoldFont) {
			DeleteObject(hBoldFont);
			hBoldFont = NULL;
		}
		break;
	}
	return FALSE;
}
