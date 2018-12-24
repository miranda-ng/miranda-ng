/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HWND hViewWnd;

HDWP MyResizeWindow(HDWP hDwp, HWND hwndDlg, HWND hwndCtrl, int nHorizontalOffset, int nVerticalOffset, int nWidthOffset, int nHeightOffset)
{
	if (nullptr == hwndDlg) /* Wine fix. */
		return hDwp;
	// get current bounding rectangle
	RECT rcinit;
	GetWindowRect(hwndCtrl, &rcinit);

	// get current top left point
	POINT pt;
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	return DeferWindowPos(hDwp, hwndCtrl, nullptr,
		pt.x + nHorizontalOffset,
		pt.y + nVerticalOffset,
		rcinit.right - rcinit.left + nWidthOffset,
		rcinit.bottom - rcinit.top + nHeightOffset,
		SWP_NOZORDER);
}

BOOL MyResizeGetOffset(HWND hwndCtrl, int nWidth, int nHeight, int* nDx, int* nDy)
{
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndCtrl, &rcinit);

	// calculate offsets
	*nDx = nWidth - (rcinit.right - rcinit.left);
	*nDy = nHeight - (rcinit.bottom - rcinit.top);

	return rcinit.bottom != rcinit.top && nHeight > 0;
}

CViewVersionInfo::CViewVersionInfo(DWORD flags) :
	CDlgBase(g_plugin, IDD_VIEWVERSION),
	m_btnCancel(this, IDCANCEL),
	m_btnCopyClip(this, IDC_CLIPVER),
	m_btnCopyFile(this, IDC_FILEVER),
	m_redtViewVersionInfo(this, IDC_VIEWVERSIONINFO)
{
	m_flags = flags;
	m_btnCancel.OnClick = Callback(this, &CViewVersionInfo::OnCancelClick);
	m_btnCopyClip.OnClick = Callback(this, &CViewVersionInfo::OnCopyClipClick);
	m_btnCopyFile.OnClick = Callback(this, &CViewVersionInfo::OnCopyFileClick);
	m_redtViewVersionInfo.OnBuildMenu = Callback(this, &CViewVersionInfo::OnViewVersionInfoBuildMenu);
}

bool CViewVersionInfo::OnInitDialog()
{
	Window_SetIcon_IcoLib(m_hwnd, GetIconHandle(IDI_VI));
	{
		CHARFORMAT2 chf;
		chf.cbSize = sizeof(chf);
		m_redtViewVersionInfo.SendMsg(EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM)&chf);
		mir_wstrcpy(chf.szFaceName, L"Courier New");
		m_redtViewVersionInfo.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&chf);

		CMStringW buffer;
		PrintVersionInfo(buffer, m_flags);
		m_redtViewVersionInfo.SetText(buffer.c_str());
	}

	if (m_flags & VI_FLAG_PRNDLL)
		SetWindowText(m_hwnd, TranslateT("View Version Information (with DLLs)"));

	Utils_RestoreWindowPositionNoMove(m_hwnd, NULL, MODULENAME, "ViewInfo_");
	Show();
	return true;
}

int CViewVersionInfo::Resizer(UTILRESIZECONTROL *)
{
	RECT rc;
	GetWindowRect(m_btnCopyFile.GetHwnd(), &rc);

	int dx, dy;
	if (MyResizeGetOffset(m_redtViewVersionInfo.GetHwnd(), LOWORD(m_flags) - 20, HIWORD(m_flags) - 30 - (rc.bottom - rc.top), &dx, &dy)) {
		HDWP hDwp = BeginDeferWindowPos(4);
		hDwp = MyResizeWindow(hDwp, m_hwnd, m_btnCopyFile.GetHwnd(), 0, dy, 0, 0);
		hDwp = MyResizeWindow(hDwp, m_hwnd, m_btnCopyClip.GetHwnd(), dx / 2, dy, 0, 0);
		hDwp = MyResizeWindow(hDwp, m_hwnd, m_btnCancel.GetHwnd(), dx, dy, 0, 0);
		hDwp = MyResizeWindow(hDwp, m_hwnd, m_redtViewVersionInfo.GetHwnd(), 0, 0, dx, dy);
		EndDeferWindowPos(hDwp);
	}
	return 0;
}

void CViewVersionInfo::OnCancelClick(CCtrlBase*)
{
	Close();
}

void CViewVersionInfo::OnCopyClipClick(CCtrlBase*)
{
	CallService(MS_CRASHDUMPER_STORETOCLIP, 0, m_flags);
}

void CViewVersionInfo::OnCopyFileClick(CCtrlBase*)
{
	CallService(MS_CRASHDUMPER_STORETOFILE, 0, m_flags);
}

bool  CViewVersionInfo::OnClose()
{
	hViewWnd = nullptr;
	Window_FreeIcon_IcoLib(m_hwnd);
	Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ViewInfo_");
	if (pViewDialog == this)
		pViewDialog = nullptr;
	if (servicemode)
		PostQuitMessage(0);
	return true;
}

void CViewVersionInfo::OnViewVersionInfoBuildMenu(CCtrlBase*)
{
	RECT rc;
	GetWindowRect(m_redtViewVersionInfo.GetHwnd(), &rc);

	POINT pt = { GET_X_LPARAM(m_flags), GET_Y_LPARAM(m_flags) };
	if (PtInRect(&rc, pt)) {
		static const CHARRANGE all = { 0, -1 };

		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		TranslateMenu(hSubMenu);

		CHARRANGE sel;
		SendMessage(m_redtViewVersionInfo.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin == sel.cpMax)
			EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr)) {
		case IDM_COPY:
			SendMessage(m_redtViewVersionInfo.GetHwnd(), WM_COPY, 0, 0);
			break;

		case IDM_COPYALL:
			SendMessage(m_redtViewVersionInfo.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&all);
			SendMessage(m_redtViewVersionInfo.GetHwnd(), WM_COPY, 0, 0);
			SendMessage(m_redtViewVersionInfo.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
			break;

		case IDM_SELECTALL:
			SendMessage(m_redtViewVersionInfo.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&all);
			break;
		}
		DestroyMenu(hMenu);
	}
}

INT_PTR CViewVersionInfo::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_GETMINMAXINFO) {
		LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
		mmi->ptMinTrackSize.x = 400; // The minimum width in points
		mmi->ptMinTrackSize.y = 300; // The minimum height in points
		return 0;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void DestroyAllWindows(void)
{
	if (hViewWnd != nullptr) {
		DestroyWindow(hViewWnd);
		hViewWnd = nullptr;
	}
}

COptDialog::COptDialog() :
	CDlgBase(g_plugin, IDD_OPTIONS),
	m_edtUserName(this, IDC_USERNAME),
	m_edtPass(this, IDC_PASSWORD),
	m_chkAutoUpload(this, IDC_UPLOADCHN),
	m_chkClassicDates(this, IDC_CLASSICDATES),
	m_chkRepSubfolder(this, IDC_DATESUBFOLDER),
	m_chkCatchCrashes(this, IDC_CATCHCRASHES),
	m_lblRestart(this, IDC_RESTARTNOTE)
{
	CreateLink(m_chkAutoUpload, "UploadChanged", DBVT_BYTE, 0);
	m_chkCatchCrashes.OnChange = Callback(this, &COptDialog::OnCatchCrashesChange);
}

bool COptDialog::OnInitDialog()
{
	CDlgBase::OnInitDialog();
	DBVARIANT dbv;
	if (g_plugin.getString("Username", &dbv) == 0) {
		m_edtUserName.SetTextA(dbv.pszVal);
		db_free(&dbv);
	}
	if (g_plugin.getString("Password", &dbv) == 0) {
		m_edtPass.SetTextA(dbv.pszVal);
		db_free(&dbv);
	}
	m_chkClassicDates.SetState(clsdates);
	m_chkRepSubfolder.SetState(dtsubfldr);
	m_chkCatchCrashes.SetState(catchcrashes);
	if (!catchcrashes) {
		m_chkClassicDates.Disable();
		m_chkRepSubfolder.Disable();
	}
	if (needrestart)
		m_lblRestart.Show();
	return true;
}

void COptDialog::OnCatchCrashesChange(CCtrlCheck*)
{
	m_chkClassicDates.Enable(m_chkCatchCrashes.GetState());
	m_chkRepSubfolder.Enable(m_chkCatchCrashes.GetState());
	m_lblRestart.Show();
	needrestart = 1;
}

bool COptDialog::OnApply()
{
	char szSetting[100];
	m_edtUserName.GetTextA(szSetting, _countof(szSetting));
	g_plugin.setString("Username", szSetting);

	m_edtPass.GetTextA(szSetting, _countof(szSetting));
	g_plugin.setString("Password", szSetting);

	clsdates = m_chkClassicDates.GetState();
	if (clsdates)
		g_plugin.setByte("ClassicDates", 1);
	else
		g_plugin.setByte("ClassicDates", 0);
	dtsubfldr = m_chkRepSubfolder.GetState();
	if (dtsubfldr)
		g_plugin.setByte("SubFolders", 1);
	else
		g_plugin.setByte("SubFolders", 0);
	catchcrashes = m_chkCatchCrashes.GetState();
	if (catchcrashes)
		g_plugin.setByte("CatchCrashes", 1);
	else
		g_plugin.setByte("CatchCrashes", 0);

	return true;
}

LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case WM_COMMAND:
		switch ((INT_PTR)PUGetPluginData(hWnd)) {
		case 0:
			OpenAuthUrl("https://vi.miranda-ng.org/detail/%s");
			break;

		case 1:
			OpenAuthUrl("https://vi.miranda-ng.org/global/%s");
			break;

		case 3:
			wchar_t path[MAX_PATH];
			mir_snwprintf(path, L"%s\\VersionInfo.txt", VersionInfoFolder);
			ShellExecute(nullptr, L"open", path, nullptr, nullptr, SW_SHOW);
			break;

		}
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		Window_FreeIcon_IcoLib(hWnd);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ShowMessage(int type, const wchar_t* format, ...)
{
	POPUPDATAT pi = {};

	va_list va;
	va_start(va, format);
	int len = mir_vsnwprintf(pi.lptzText, _countof(pi.lptzText) - 1, format, va);
	pi.lptzText[len] = 0;
	va_end(va);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		mir_wstrcpy(pi.lptzContactName, _A2W(MODULENAME));
		pi.lchIcon = LoadIconEx(IDI_VI);
		pi.PluginWindowProc = DlgProcPopup;
		pi.PluginData = (void*)type;

		PUAddPopupT(&pi);
	}
	else MessageBox(nullptr, pi.lptzText, _A2W(MODULENAME), MB_OK | MB_ICONINFORMATION);
}
