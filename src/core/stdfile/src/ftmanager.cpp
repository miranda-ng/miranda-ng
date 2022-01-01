/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "file.h"

static HWND hwndFtMgr = nullptr;

struct TFtMgrData
{
	HWND hwndIncoming;
	HWND hwndOutgoing;

	HANDLE hhkPreshutdown;
	TBPFLAG errorState;
};

#define M_CALCPROGRESS (WM_USER + 200)

struct TFtProgressData
{
	unsigned int init, run, scan;
	unsigned __int64 totalBytes, totalProgress;
};

struct TLayoutWindowInfo
{
	TLayoutWindowInfo(HWND _hwnd) :
		hwnd(_hwnd)
	{
		::GetWindowRect(_hwnd, &rc);
	}

	HWND hwnd;
	RECT rc;
};

struct TFtPageData
{
	TFtPageData() :
		arWindows(1)
	{}

	OBJLIST<TLayoutWindowInfo> arWindows;
	int runningCount = 0;
	int height = 0, dataHeight = 0, scrollPos = 0;
};

static void LayoutTransfers(HWND hwnd, TFtPageData *dat)
{
	int top = 0;
	RECT rc;
	GetClientRect(hwnd, &rc);

	dat->scrollPos = GetScrollPos(hwnd, SB_VERT);
	dat->height = rc.bottom - rc.top;

	if (dat->arWindows.getCount()) {
		HDWP hdwp = BeginDeferWindowPos(dat->arWindows.getCount());
		top -= dat->scrollPos;
		for (auto &it : dat->arWindows) {
			int height = it->rc.bottom - it->rc.top;
			if (nullptr != it->hwnd) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, it->hwnd, nullptr, 0, top, rc.right, height, SWP_NOZORDER);
			top += height;
		}
		top += dat->scrollPos;
		EndDeferWindowPos(hdwp);
	}

	dat->dataHeight = top;

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE;
	si.nPage = dat->height;
	si.nMin = 0;
	si.nMax = dat->dataHeight;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

static INT_PTR CALLBACK FtMgrPageDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TFtPageData *dat = (TFtPageData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			// Force scrollbar visibility
			SCROLLINFO si = {};
			si.cbSize = sizeof(si);
			si.fMask = SIF_DISABLENOSCROLL;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

			dat = new TFtPageData();
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		}
		break;

	case WM_FT_ADD:
		{
			TLayoutWindowInfo *wnd = new TLayoutWindowInfo((HWND)lParam);
			if (g_plugin.bReverseOrder)
				dat->arWindows.insert(wnd, 0);
			else
				dat->arWindows.insert(wnd);
			LayoutTransfers(hwnd, dat);
			dat->runningCount++;
			PostMessage(GetParent(hwnd), WM_TIMER, 1, NULL);
		}
		break;

	case WM_FT_RESIZE:
		for (auto &it : dat->arWindows)
			if (it->hwnd == (HWND)lParam) {
				GetWindowRect(it->hwnd, &it->rc);
				break;
			}
		LayoutTransfers(hwnd, dat);
		break;

	case WM_FT_REMOVE:
		for (auto &it : dat->arWindows)
			if (it->hwnd == (HWND)lParam) {
				dat->arWindows.remove(dat->arWindows.indexOf(&it));
				break;
			}
		LayoutTransfers(hwnd, dat);
		break;

	case WM_FT_COMPLETED:
		//wParam: { ACKRESULT_SUCCESS | ACKRESULT_FAILED | ACKRESULT_DENIED }
		dat->runningCount--;
		{
			bool bFound = false;
			for (auto &it : dat->arWindows)
				// no error when canceling (WM_FT_REMOVE is send first, check if hwnd is still registered)
				if (it->hwnd == (HWND)lParam) {
					bFound = true;
					SendMessage(GetParent(hwnd), WM_TIMER, 1, (LPARAM)wParam);
					break;
				}

			if (!bFound)
				PostMessage(GetParent(hwnd), WM_TIMER, 1, NULL);
		}

		if(dat->runningCount == 0 && wParam == ACKRESULT_SUCCESS && g_plugin.bAutoClose)
			ShowWindow(hwndFtMgr, SW_HIDE);
		break;

	case WM_FT_CLEANUP:
		for (auto &it : dat->arWindows)
			SendMessage(it->hwnd, WM_FT_CLEANUP, wParam, lParam);
		break;

	case WM_SIZE:
		LayoutTransfers(hwnd, dat);
		break;

	case WM_MOUSEWHEEL:
		if (int zDelta = GET_WHEEL_DELTA_WPARAM(wParam)) {
			int nScrollLines = 0;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
			for (int i=0; i < (nScrollLines + 1) / 2; i++)
				SendMessage(hwnd, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
		}

		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
		return TRUE;

	case WM_VSCROLL:
		{
			int pos = dat->scrollPos;
			switch (LOWORD(wParam)) {
			case SB_LINEDOWN:
				pos += 15;
				break;
			case SB_LINEUP:
				pos -= 15;
				break;
			case SB_PAGEDOWN:
				pos += dat->height - 10;
				break;
			case SB_PAGEUP:
				pos -= dat->height - 10;
				break;
			case SB_THUMBTRACK:
				pos = HIWORD(wParam);
				break;
			}

			if (pos > dat->dataHeight - dat->height)
				pos = dat->dataHeight - dat->height;
			if (pos < 0)
				pos = 0;

			if (dat->scrollPos != pos) {
				ScrollWindow(hwnd, 0, dat->scrollPos - pos, nullptr, nullptr);
				SetScrollPos(hwnd, SB_VERT, pos, TRUE);
				dat->scrollPos = pos;
			}
		}
		break;

	case M_PRESHUTDOWN:
		for (auto &it : dat->arWindows)
			PostMessage(it->hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
		break;

	case M_CALCPROGRESS:
		{
			TFtProgressData *prg = (TFtProgressData *)wParam;
			for (auto &it : dat->arWindows) {
				FileDlgData *trdat = (FileDlgData *)GetWindowLongPtr(it->hwnd, GWLP_USERDATA);
				if (trdat->transferStatus.totalBytes && trdat->fs && !trdat->send && (trdat->transferStatus.totalBytes == trdat->transferStatus.totalProgress))
					prg->scan++;
				else if (trdat->transferStatus.totalBytes && trdat->fs) { // in progress
					prg->run++;
					prg->totalBytes += trdat->transferStatus.totalBytes;
					prg->totalProgress += trdat->transferStatus.totalProgress;
				}
				else if (trdat->fs) // starting
					prg->init++;
			}
		}
		break;

	case WM_DESTROY:
		delete dat;
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK FtMgrDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct TFtMgrData *dat = (struct TFtMgrData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HWND hwndTab = GetDlgItem(hwnd, IDC_TABS);

	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);
			Window_SetSkinIcon_IcoLib(hwnd, SKINICON_EVENT_FILE);

			dat = (struct TFtMgrData *)mir_calloc(sizeof(struct TFtMgrData));

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);

			dat->hhkPreshutdown = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwnd, M_PRESHUTDOWN);

			dat->hwndIncoming = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FTPAGE), hwnd, FtMgrPageDlgProc);
			dat->hwndOutgoing = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FTPAGE), hwnd, FtMgrPageDlgProc);
			ShowWindow(dat->hwndIncoming, SW_SHOW);

			TCITEM tci = {};
			tci.mask = TCIF_PARAM|TCIF_TEXT;
			tci.pszText = TranslateT("Incoming");
			tci.lParam = (LPARAM)dat->hwndIncoming;
			TabCtrl_InsertItem(hwndTab, 0, &tci);
			tci.pszText = TranslateT("Outgoing");
			tci.lParam = (LPARAM)dat->hwndOutgoing;
			TabCtrl_InsertItem(hwndTab, 1, &tci);

			Utils_RestoreWindowPosition(hwnd, NULL, MODULENAME, "FtMgrDlg_", RWPF_NOACTIVATE);
			// Fall through to setup initial placement
			__fallthrough;
		}
	case WM_SIZE:
		{
			RECT rc, rcButton;
			GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rcButton);
			OffsetRect(&rcButton, -rcButton.left, -rcButton.top);

			GetClientRect(hwnd, &rc);
			InflateRect(&rc, -6, -6);

			HDWP hdwp = BeginDeferWindowPos(3);

			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDC_CLEAR), NULL, rc.left, rc.bottom-rcButton.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwnd, IDCANCEL), nullptr, rc.right-rcButton.right, rc.bottom-rcButton.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

			rc.bottom -= rcButton.bottom + 5;

			if (nullptr != hwndTab) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, hwndTab, nullptr, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER);

			EndDeferWindowPos(hdwp);

			GetWindowRect(hwndTab, &rc);
			MapWindowPoints(nullptr, hwnd, (LPPOINT)&rc, 2);
			TabCtrl_AdjustRect(hwndTab, FALSE, &rc);
			InflateRect(&rc, -5, -5);

			hdwp = BeginDeferWindowPos(2);

			if (nullptr != dat->hwndIncoming) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, dat->hwndIncoming, HWND_TOP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 0);
			if (nullptr != dat->hwndOutgoing) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, dat->hwndOutgoing, HWND_TOP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 0);

			EndDeferWindowPos(hdwp);
		}
		break;

	case WM_MOUSEWHEEL:
		if (IsWindowVisible(dat->hwndIncoming))
			SendMessage(dat->hwndIncoming, msg, wParam, lParam);
		if (IsWindowVisible(dat->hwndOutgoing))
			SendMessage(dat->hwndOutgoing, msg, wParam, lParam);
		break;

	case WM_FT_SELECTPAGE:
		if (TabCtrl_GetCurSel(hwndTab) != (int)wParam) {
			TCITEM tci = {};
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, TabCtrl_GetCurSel(hwndTab), &tci);

			ShowWindow((HWND)tci.lParam, SW_HIDE);
			TabCtrl_SetCurSel(hwndTab, wParam);
			TabCtrl_GetItem(hwndTab, TabCtrl_GetCurSel(hwndTab), &tci);
			ShowWindow((HWND)tci.lParam, SW_SHOW);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
			lpmmi->ptMinTrackSize.x = 300;
			lpmmi->ptMinTrackSize.y = 400;
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		case IDC_CLEAR:
			PostMessage(dat->hwndIncoming, WM_FT_CLEANUP, 0, 0);
			PostMessage(dat->hwndOutgoing, WM_FT_CLEANUP, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_TABS:
			TCITEM tci = {};
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGING:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, TabCtrl_GetCurSel(hwndTab), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
				break;

			case TCN_SELCHANGE:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, TabCtrl_GetCurSel(hwndTab), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
				break;
			}
			break;
		}
		break;

	case M_PRESHUTDOWN:
		SendMessage(dat->hwndIncoming, M_PRESHUTDOWN, 0, 0);
		SendMessage(dat->hwndOutgoing, M_PRESHUTDOWN, 0, 0);
		DestroyWindow(hwnd);
		break;

	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		if (g_plugin.bAutoClear) {
			PostMessage(dat->hwndIncoming, WM_FT_CLEANUP, 0, 0);
			PostMessage(dat->hwndOutgoing, WM_FT_CLEANUP, 0, 0);
		}
		return TRUE; /* Disable default IDCANCEL notification */

	case WM_DESTROY:
		UnhookEvent(dat->hhkPreshutdown);
		Window_FreeIcon_IcoLib(hwnd);
		DestroyWindow(dat->hwndIncoming);
		DestroyWindow(dat->hwndOutgoing);
		mir_free(dat);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		Utils_SaveWindowPosition(hwnd, NULL, MODULENAME, "FtMgrDlg_");
		break;

	case WM_ACTIVATE:
		dat->errorState = TBPF_NOPROGRESS;
		wParam = 1;
		break;

	case WM_SHOWWINDOW:
		if (!wParam) { // hiding
			KillTimer(hwnd, 1);
			break;
		}
		lParam = 0;

	case WM_TIMER:
		if (pTaskbarInterface) {
			SetTimer(hwnd, 1, 400, nullptr);
			if ((lParam == ACKRESULT_FAILED) || (lParam == ACKRESULT_DENIED))
				dat->errorState = TBPF_ERROR;

			TFtProgressData prg = { 0 };
			SendMessage(dat->hwndIncoming, M_CALCPROGRESS, (WPARAM)&prg, 0);
			SendMessage(dat->hwndOutgoing, M_CALCPROGRESS, (WPARAM)&prg, 0);
			if (dat->errorState) {
				pTaskbarInterface->SetProgressState(hwnd, dat->errorState);
				if (!prg.run)
					pTaskbarInterface->SetProgressValue(hwnd, 1, 1);
			}
			else if (prg.run)
				pTaskbarInterface->SetProgressState(hwnd, TBPF_NORMAL);
			else if (prg.init || prg.scan)
				pTaskbarInterface->SetProgressState(hwnd, TBPF_INDETERMINATE);
			else {
				pTaskbarInterface->SetProgressState(hwnd, TBPF_NOPROGRESS);
				KillTimer(hwnd, 1);
			}

			if (prg.run)
				pTaskbarInterface->SetProgressValue(hwnd, prg.totalProgress, prg.totalBytes);
		}
		break;
	}

	return FALSE;
}

HWND FtMgr_Show(bool bForceActivate, bool bFromMenu)
{
	bool bJustCreated = (hwndFtMgr == nullptr);
	if (bJustCreated)
		hwndFtMgr = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FTMGR), NULL, FtMgrDlgProc);

	if (bFromMenu) { /* lqbe */
		ShowWindow(hwndFtMgr, SW_RESTORE);
		ShowWindow(hwndFtMgr, SW_SHOW);
		SetForegroundWindow(hwndFtMgr);
		return hwndFtMgr;
	}
	if (g_plugin.bAutoMin && bJustCreated) { /* lqbe */
		ShowWindow(hwndFtMgr, SW_HIDE);
		ShowWindow(hwndFtMgr, SW_MINIMIZE);
		return hwndFtMgr;
	}
	if (bForceActivate) { /* lqbe */
		ShowWindow(hwndFtMgr, SW_RESTORE);
		ShowWindow(hwndFtMgr, SW_SHOWNOACTIVATE);
		SetForegroundWindow(hwndFtMgr);
		return hwndFtMgr;
	}
	if (!bJustCreated && IsWindowVisible(hwndFtMgr))
		return hwndFtMgr;

	ShowWindow(hwndFtMgr, g_plugin.bAutoMin ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
	return hwndFtMgr;
}

void FtMgr_Destroy()
{
	if (hwndFtMgr)
		DestroyWindow(hwndFtMgr);
}

void FtMgr_ShowPage(int page)
{
	if (hwndFtMgr)
		SendMessage(hwndFtMgr, WM_FT_SELECTPAGE, page, 0);
}

HWND FtMgr_AddTransfer(FileDlgData *fdd)
{
	bool bForceActivate = fdd->send || !g_plugin.bAutoAccept;
	TFtMgrData *dat = (TFtMgrData *)GetWindowLongPtr(FtMgr_Show(bForceActivate, false), GWLP_USERDATA);
	if (dat == nullptr)
		return nullptr;

	HWND hwndBox = fdd->send ? dat->hwndOutgoing : dat->hwndIncoming;
	HWND hwndFt = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILETRANSFERINFO), hwndBox, DlgProcFileTransfer, (LPARAM)fdd);
	ShowWindow(hwndFt, SW_SHOWNA);
	SendMessage(hwndBox, WM_FT_ADD, 0, (LPARAM)hwndFt);
	FtMgr_ShowPage(fdd->send ? 1 : 0);
	return hwndFt;
}
