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

class CViewVersionInfo : public CDlgBase
{
	uint32_t m_flags;

	CCtrlButton m_btnCopyClip, m_btnCopyFile;
	CCtrlRichEdit m_redtViewVersionInfo;

public:
	CViewVersionInfo(uint32_t flags) :
		CDlgBase(g_plugin, IDD_VIEWVERSION),
		m_btnCopyClip(this, IDC_CLIPVER),
		m_btnCopyFile(this, IDC_FILEVER),
		m_redtViewVersionInfo(this, IDC_VIEWVERSIONINFO)
	{
		SetMinSize(400, 300);

		m_flags = flags;
		m_forceResizable = true;

		m_btnCopyClip.OnClick = Callback(this, &CViewVersionInfo::OnCopyClipClick);
		m_btnCopyFile.OnClick = Callback(this, &CViewVersionInfo::OnCopyFileClick);
		m_redtViewVersionInfo.OnBuildMenu = Callback(this, &CViewVersionInfo::OnViewVersionInfoBuildMenu);
	}

	bool OnInitDialog() override
	{
		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_VI));
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

	void OnDestroy() override
	{
		Window_FreeIcon_IcoLib(m_hwnd);
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "ViewInfo_");
		if (pViewDialog == this)
			pViewDialog = nullptr;
		if (g_plugin.bServiceMode)
			PostQuitMessage(0);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_VIEWVERSIONINFO:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_FILEVER:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

		case IDC_CLIPVER:
			return RD_ANCHORX_CENTRE | RD_ANCHORY_BOTTOM;

		case IDCANCEL:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void OnCopyClipClick(CCtrlBase*)
	{
		CallService(MS_CRASHDUMPER_STORETOCLIP, 0, m_flags);
	}

	void OnCopyFileClick(CCtrlBase*)
	{
		CallService(MS_CRASHDUMPER_STORETOFILE, 0, m_flags);
	}

	void OnViewVersionInfoBuildMenu(CCtrlBase*)
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
};

INT_PTR ViewVersionInfo(WPARAM wParam, LPARAM)
{
	if (pViewDialog == nullptr) {
		uint32_t dwFlags = wParam ? (VI_FLAG_PRNVAR | VI_FLAG_PRNDLL) : VI_FLAG_PRNVAR;
		pViewDialog = new CViewVersionInfo(dwFlags);
		pViewDialog->Show();
	}
	else {
		SetForegroundWindow(pViewDialog->GetHwnd());
		SetFocus(pViewDialog->GetHwnd());
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options 

class COptDialog : public CDlgBase
{
	CCtrlEdit m_edtUserName, m_edtPass;
	CCtrlCheck m_chkAutoUpload, m_chkClassicDates, m_chkRepSubfolder, m_chkCatchCrashes, m_chkSuccessPopups;
	CCtrlLabel m_lblRestart;

	void COptDialog::OnCatchCrashesChange(CCtrlCheck*)
	{
		m_chkRepSubfolder.Enable(m_chkCatchCrashes.GetState());

		if (m_bInitialized) {
			m_lblRestart.Show();
			g_plugin.bNeedRestart = true;
		}
	}

public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_edtUserName(this, IDC_USERNAME),
		m_edtPass(this, IDC_PASSWORD),
		m_chkAutoUpload(this, IDC_UPLOADCHN),
		m_chkCatchCrashes(this, IDC_CATCHCRASHES),
		m_chkClassicDates(this, IDC_CLASSICDATES),
		m_chkRepSubfolder(this, IDC_DATESUBFOLDER),
		m_chkSuccessPopups(this, IDC_SUCCESSPOPUPS),
		m_lblRestart(this, IDC_RESTARTNOTE)
	{
		CreateLink(m_chkAutoUpload, g_plugin.bUploadChanged);
		CreateLink(m_chkCatchCrashes, g_plugin.bCatchCrashes);
		CreateLink(m_chkClassicDates, g_plugin.bClassicDates);
		CreateLink(m_chkRepSubfolder, g_plugin.bUseSubFolder);
		CreateLink(m_chkSuccessPopups, g_plugin.bSuccessPopups);

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

		if (!g_plugin.bCatchCrashes)
			m_chkRepSubfolder.Disable();
		if (g_plugin.bNeedRestart)
			m_lblRestart.Show();
		return true;
	}

	bool COptDialog::OnApply()
	{
		char szSetting[100];
		m_edtUserName.GetTextA(szSetting, _countof(szSetting));
		g_plugin.setString("Username", szSetting);

		m_edtPass.GetTextA(szSetting, _countof(szSetting));
		g_plugin.setString("Password", szSetting);
		return true;
	}
};

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = MODULENAME;
	odp.szGroup.a = LPGEN("Services");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptDialog;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// popups

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

void ShowMessage(int type, const wchar_t *format, ...)
{
	switch (type) {
	case 1:
	case 3:
		if (!g_plugin.bSuccessPopups)
			return;
	}

	POPUPDATAW ppd;

	va_list va;
	va_start(va, format);
	int len = mir_vsnwprintf(ppd.lpwzText, _countof(ppd.lpwzText) - 1, format, va);
	ppd.lpwzText[len] = 0;
	va_end(va);

	mir_wstrcpy(ppd.lpwzContactName, _A2W(MODULENAME));
	ppd.lchIcon = g_plugin.getIcon(IDI_VI);
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.PluginData = (void*)type;
	PUAddPopupW(&ppd);
}
