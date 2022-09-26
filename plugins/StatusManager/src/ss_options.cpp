/*
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

char* OptName(int i, const char* setting)
{
	static char buf[100];
	mir_snprintf(buf, "%d_%s", i, setting);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Command line processing

static char* GetStatusDesc(int status)
{
	switch (status) {
	case ID_STATUS_AWAY:       return "away";
	case ID_STATUS_NA:         return "na";
	case ID_STATUS_DND:        return "dnd";
	case ID_STATUS_OCCUPIED:   return "occupied";
	case ID_STATUS_FREECHAT:   return "freechat";
	case ID_STATUS_ONLINE:     return "online";
	case ID_STATUS_OFFLINE:    return "offline";
	case ID_STATUS_INVISIBLE:  return "invisible";
	case ID_STATUS_LAST:       return "last";
	}
	return "offline";
}

static char* GetCMDLArguments(TProtoSettings &protoSettings)
{
	if (protoSettings.getCount() == 0)
		return nullptr;

	char *cmdl, *pnt;
	pnt = cmdl = (char*)mir_alloc(mir_strlen(protoSettings[0].m_szName) + mir_strlen(GetStatusDesc(protoSettings[0].m_status)) + 4);

	for (int i = 0; i < protoSettings.getCount(); i++) {
		*pnt++ = '/';
		mir_strcpy(pnt, protoSettings[i].m_szName);
		pnt += mir_strlen(protoSettings[i].m_szName);
		*pnt++ = '=';
		mir_strcpy(pnt, GetStatusDesc(protoSettings[i].m_status));
		pnt += mir_strlen(GetStatusDesc(protoSettings[i].m_status));
		if (i != protoSettings.getCount() - 1) {
			*pnt++ = ' ';
			*pnt++ = '\0';
			cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + mir_strlen(protoSettings[i + 1].m_szName) + mir_strlen(GetStatusDesc(protoSettings[i + 1].m_status)) + 4);
			pnt = cmdl + mir_strlen(cmdl);
		}
	}

	if (SSPlugin.getByte(SETTING_SHOWDIALOG, FALSE) == TRUE) {
		*pnt++ = ' ';
		*pnt++ = '\0';
		cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + 12);
		pnt = cmdl + mir_strlen(cmdl);
		mir_strcpy(pnt, "/showdialog");
		pnt += 11;
		*pnt = '\0';
	}

	return cmdl;
}

static char* GetCMDL(TProtoSettings &protoSettings)
{
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);

	char* cmdl = (char*)mir_alloc(mir_strlen(path) + 4);
	mir_snprintf(cmdl, mir_strlen(path) + 4, "\"%s\" ", path);

	char* args = GetCMDLArguments(protoSettings);
	if (args) {
		cmdl = (char*)mir_realloc(cmdl, mir_strlen(cmdl) + mir_strlen(args) + 1);
		mir_strcat(cmdl, args);
		mir_free(args);
	}
	return cmdl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Link processing

static wchar_t* GetLinkDescription(TProtoSettings &protoSettings)
{
	if (protoSettings.getCount() == 0)
		return nullptr;

	CMStringW result(SHORTCUT_DESC);
	for (auto &p : protoSettings) {
		wchar_t *status;
		if (p->m_status == ID_STATUS_LAST)
			status = TranslateT("<last>");
		else if (p->m_status == ID_STATUS_CURRENT)
			status = TranslateT("<current>");
		else if (p->m_status >= MIN_STATUS && p->m_status <= MAX_STATUS)
			status = Clist_GetStatusModeDescription(p->m_status, 0);
		else
			status = nullptr;
		if (status == nullptr)
			status = TranslateT("<unknown>");

		result.AppendChar('\r');
		result.Append(p->m_tszAccName);
		result.AppendChar(':');
		result.AppendChar(' ');
		result.Append(status);
	}

	return mir_wstrndup(result, result.GetLength());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Command line options

class CCmdlDlg : public CDlgBase
{
	TProtoSettings ps;

	CCtrlButton btnLink, btnCopy;

public:
	CCmdlDlg(int iProfileNo) :
		CDlgBase(g_plugin, IDD_CMDLOPTIONS),
		btnCopy(this, IDC_COPY),
		btnLink(this, IDC_SHORTCUT),
		ps(protoList)
	{
		GetProfile(iProfileNo, ps);
	}

	bool OnInitDialog() override
	{
		char* cmdl = GetCMDL(ps);
		SetDlgItemTextA(m_hwnd, IDC_CMDL, cmdl);
		mir_free(cmdl);
		return true;
	}

	void onClick_Copy(CCtrlButton*)
	{
		wchar_t cmdl[2048];
		GetDlgItemText(m_hwnd, IDC_CMDL, cmdl, _countof(cmdl));
		Utils_ClipboardCopy(cmdl);
	}

	void onClick_Link(CCtrlButton*)
	{
		wchar_t savePath[MAX_PATH];
		if (SHGetSpecialFolderPath(nullptr, savePath, CSIDL_DESKTOPDIRECTORY, FALSE))
			wcsncat_s(savePath, SHORTCUT_FILENAME, _countof(savePath) - mir_wstrlen(savePath));
		else
			mir_snwprintf(savePath, L".\\%s", SHORTCUT_FILENAME);

		// Get a pointer to the IShellLink interface.
		IShellLink *psl;
		HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
		if (SUCCEEDED(hres)) {
			ptrA args(GetCMDLArguments(ps));
			ptrW desc(GetLinkDescription(ps));

			// Set the path to the shortcut target, and add the
			// description.
			wchar_t path[MAX_PATH];
			GetModuleFileName(nullptr, path, _countof(path));
			psl->SetPath(path);
			psl->SetDescription(desc);
			psl->SetArguments(_A2T(args));

			// Query IShellLink for the IPersistFile interface for saving the
			// shortcut in persistent storage.
			IPersistFile *ppf;
			hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

			if (SUCCEEDED(hres)) {
				// Save the link by calling IPersistFile::Save.
				hres = ppf->Save(savePath, TRUE);
				ppf->Release();
			}
			psl->Release();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// basic class for all SS options

class CSSOptionsBaseDlg : public CDlgBase
{
	void OnFinishWizard(void*)
	{
		if (hTTBModuleLoadedHook) {
			RemoveTopToolbarButtons();
			CreateTopToolbarButtons(0, 0);
		}

		UnregisterHotKeys();
		RegisterHotKeys();
	}

protected:
	CSSOptionsBaseDlg(int iDlg) :
		CDlgBase(g_plugin, iDlg)
	{
		m_OnFinishWizard = Callback(this, &CSSOptionsBaseDlg::OnFinishWizard);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Main SS options dialog

class CSSMainOptDlg : public CSSOptionsBaseDlg
{
	// creates profile combo box according to 'dat'
	void ReinitProfiles()
	{	
		profiles.ResetContent();

		int defProfile;
		int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
		if (profileCount > 0) {
			for (int i = 0; i < profileCount; i++) {
				wchar_t profileName[128];
				if (GetProfileName(i, (LPARAM)profileName))
					continue;

				profiles.AddString(profileName, i);
			}
		}
		else profiles.AddString(TranslateT("default"));
		profiles.SetCurSel(defProfile);

		chkSetDocked.Enable(db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 1));
		if (!chkSetDocked.Enabled())
			chkSetDocked.SetState(false);

		int val = SSPlugin.getByte(SETTING_WINSTATE, SETTING_STATE_NORMAL);
		SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_RESETCONTENT, 0, 0);

		int item = SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
		SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETITEMDATA, item, (LPARAM)SETTING_STATE_HIDDEN);
		if (val == SETTING_STATE_HIDDEN)
			SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETCURSEL, item, 0);

		if (!db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0)) {
			item = SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Minimized"));
			SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_MINIMIZED);
			if (val == SETTING_STATE_MINIMIZED)
				SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETCURSEL, item, 0);
		}
		
		item = SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Normal"));
		SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETITEMDATA, item, SETTING_STATE_NORMAL);
		if (val == SETTING_STATE_NORMAL || (val == SETTING_STATE_MINIMIZED) && db_get_b(0, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
			SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_SETCURSEL, item, 0);
	}

	CCtrlButton btnShowCmdl;
	CCtrlCheck chkSetProfile, chkShowDialog, chkSetWinSize, chkSetWinState, chkSetWinLocation, chkSetDocked;
	CCtrlCombo profiles;
	CTimer timer;

public:
	CSSMainOptDlg() :
		CSSOptionsBaseDlg(IDD_OPT_STARTUPSTATUS),
		timer(this, 10),
		profiles(this, IDC_PROFILE),
		btnShowCmdl(this, IDC_SHOWCMDL),
		chkSetDocked(this, IDC_SETDOCKED),
		chkSetProfile(this, IDC_SETPROFILE),
		chkShowDialog(this, IDC_SHOWDIALOG),
		chkSetWinSize(this, IDC_SETWINSIZE),
		chkSetWinState(this, IDC_SETWINSTATE),
		chkSetWinLocation(this, IDC_SETWINLOCATION)
	{
		btnShowCmdl.OnClick = Callback(this, &CSSMainOptDlg::onClick_Show);

		timer.OnEvent = Callback(this, &CSSMainOptDlg::onTimer);

		chkSetDocked.OnChange = Callback(this, &CSSMainOptDlg::onChange_Docked);
		chkSetProfile.OnChange = Callback(this, &CSSMainOptDlg::onChange_SetProfile);
		chkShowDialog.OnChange = Callback(this, &CSSMainOptDlg::onChange_ShowDialog);
		chkSetWinSize.OnChange = Callback(this, &CSSMainOptDlg::onChange_SetWinSize);
		chkSetWinState.OnChange = Callback(this, &CSSMainOptDlg::onChange_SetWinState);
		chkSetWinLocation.OnChange = Callback(this, &CSSMainOptDlg::onChange_SetWinLocation);
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_SETPROFILE, SSPlugin.getByte(SETTING_SETPROFILE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_OVERRIDE, SSPlugin.getByte(SETTING_OVERRIDE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWDIALOG, SSPlugin.getByte(SETTING_SHOWDIALOG, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SETWINSTATE, SSPlugin.getByte(SETTING_SETWINSTATE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SETWINLOCATION, SSPlugin.getByte(SETTING_SETWINLOCATION, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SETDOCKED, SSPlugin.getByte(SETTING_SETDOCKED, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SETWINSIZE, SSPlugin.getByte(SETTING_SETWINSIZE, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_OFFLINECLOSE, SSPlugin.getByte(SETTING_OFFLINECLOSE, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_AUTODIAL, SSPlugin.getByte(SETTING_AUTODIAL, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_AUTOHANGUP, SSPlugin.getByte(SETTING_AUTOHANGUP, 0) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(m_hwnd, IDC_SETPROFILEDELAY, SSPlugin.getWord(SETTING_SETPROFILEDELAY, 500), FALSE);
		SetDlgItemInt(m_hwnd, IDC_DLGTIMEOUT, SSPlugin.getWord(SETTING_DLGTIMEOUT, 5), FALSE);
		SetDlgItemInt(m_hwnd, IDC_XPOS, SSPlugin.getWord(SETTING_XPOS, 0), TRUE);
		SetDlgItemInt(m_hwnd, IDC_YPOS, SSPlugin.getWord(SETTING_YPOS, 0), TRUE);
		SetDlgItemInt(m_hwnd, IDC_WIDTH, SSPlugin.getWord(SETTING_WIDTH, 0), FALSE);
		SetDlgItemInt(m_hwnd, IDC_HEIGHT, SSPlugin.getWord(SETTING_HEIGHT, 0), FALSE);

		int val = SSPlugin.getByte(SETTING_DOCKED, DOCKED_NONE);
		int item = SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left"));
		SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_LEFT);
		if (val == DOCKED_LEFT)
			SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

		item = SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right"));
		SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_RIGHT);
		if (val == DOCKED_RIGHT)
			SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

		item = SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_ADDSTRING, 0, (LPARAM)TranslateT("None"));
		SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETITEMDATA, (WPARAM)item, (LPARAM)DOCKED_NONE);
		if (val == DOCKED_NONE)
			SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_SETCURSEL, (WPARAM)item, 0);

		ReinitProfiles();
		timer.Start(100);
		return true;
	}

	bool OnApply() override
	{
		bool bChecked = chkSetProfile.GetState();
		SSPlugin.setByte(SETTING_SETPROFILE, bChecked);
		if (bChecked)
			SSPlugin.setDword(SETTING_SETPROFILEDELAY, GetDlgItemInt(m_hwnd, IDC_SETPROFILEDELAY, nullptr, FALSE));

		if (bChecked || chkShowDialog.GetState())
			SSPlugin.setWord(SETTING_DEFAULTPROFILE, (uint16_t)profiles.GetCurData());

		SSPlugin.setByte(SETTING_OVERRIDE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_OVERRIDE));
		SSPlugin.setByte(SETTING_SHOWDIALOG, bChecked = chkShowDialog.GetState());
		if (bChecked)
			SSPlugin.setDword(SETTING_DLGTIMEOUT, GetDlgItemInt(m_hwnd, IDC_DLGTIMEOUT, nullptr, FALSE));

		SSPlugin.setByte(SETTING_SETWINSTATE, bChecked = chkSetWinState.GetState());
		if (bChecked) {
			int val = (int)SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_GETITEMDATA, SendDlgItemMessage(m_hwnd, IDC_WINSTATE, CB_GETCURSEL, 0, 0), 0);
			SSPlugin.setByte(SETTING_WINSTATE, (uint8_t)val);
		}
		
		SSPlugin.setByte(SETTING_SETDOCKED, bChecked = chkSetDocked.GetState());
		if (bChecked) {
			int val = (int)SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_GETITEMDATA, SendDlgItemMessage(m_hwnd, IDC_DOCKED, CB_GETCURSEL, 0, 0), 0);
			SSPlugin.setByte(SETTING_DOCKED, (uint8_t)val);
		}
		
		SSPlugin.setByte(SETTING_SETWINLOCATION, bChecked = chkSetWinLocation.GetState());
		if (bChecked) {
			SSPlugin.setDword(SETTING_XPOS, GetDlgItemInt(m_hwnd, IDC_XPOS, nullptr, TRUE));
			SSPlugin.setDword(SETTING_YPOS, GetDlgItemInt(m_hwnd, IDC_YPOS, nullptr, TRUE));
		}
		
		SSPlugin.setByte(SETTING_SETWINSIZE, bChecked = chkSetWinSize.GetState());
		if (bChecked) {
			SSPlugin.setDword(SETTING_WIDTH, GetDlgItemInt(m_hwnd, IDC_WIDTH, nullptr, FALSE));
			SSPlugin.setDword(SETTING_HEIGHT, GetDlgItemInt(m_hwnd, IDC_HEIGHT, nullptr, FALSE));
		}
		
		SSPlugin.setByte(SETTING_OFFLINECLOSE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_OFFLINECLOSE));
		SSPlugin.setByte(SETTING_AUTODIAL, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_AUTODIAL));
		SSPlugin.setByte(SETTING_AUTOHANGUP, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_AUTOHANGUP));
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_SHOWWINDOW && wParam)
			ReinitProfiles();

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onClick_Show(CCtrlButton*)
	{
		CCmdlDlg *pDlg = new CCmdlDlg(profiles.GetCurData());
		pDlg->Show();
	}

	void onChange_SetProfile(CCtrlCheck*)
	{
		bool bChecked = chkSetProfile.GetState();
		profiles.Enable(bChecked || chkShowDialog.GetState());
		EnableWindow(GetDlgItem(m_hwnd, IDC_SETPROFILEDELAY), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_OVERRIDE), bChecked);
	}

	void onChange_ShowDialog(CCtrlCheck*)
	{
		bool bChecked = chkShowDialog.GetState();
		profiles.Enable(chkSetProfile.GetState() || bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_DLGTIMEOUT), bChecked);
	}

	void onChange_SetWinState(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_WINSTATE), chkSetWinState.GetState());
	}

	void onChange_SetWinLocation(CCtrlCheck*)
	{
		bool bChecked = chkSetWinLocation.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_XPOS), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_YPOS), bChecked);
	}

	void onChange_SetWinSize(CCtrlCheck*)
	{
		bool bChecked = chkSetWinSize.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_WIDTH), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_HEIGHT), !db_get_b(0, MODULE_CLUI, SETTING_AUTOSIZE, 0) && bChecked);
	}

	void onChange_Docked(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_DOCKED), chkSetDocked.GetState());
	}

	void onTimer(CTimer*)
	{
		if (chkSetWinLocation.GetState() && chkSetWinSize.GetState()) {
			SetDlgItemTextA(m_hwnd, IDC_CURWINSIZE, "");
			SetDlgItemTextA(m_hwnd, IDC_CURWINLOC, "");
		}
		else {
			wchar_t text[128];
			mir_snwprintf(text, TranslateT("size: %d x %d"), db_get_dw(0, MODULE_CLIST, SETTING_WIDTH, 0), db_get_dw(0, MODULE_CLIST, SETTING_HEIGHT, 0));
			SetDlgItemText(m_hwnd, IDC_CURWINSIZE, text);

			mir_snwprintf(text, TranslateT("loc: %d x %d"), db_get_dw(0, MODULE_CLIST, SETTING_XPOS, 0), db_get_dw(0, MODULE_CLIST, SETTING_YPOS, 0));
			SetDlgItemText(m_hwnd, IDC_CURWINLOC, text);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// new profile dialog

class CAddProfileDlg : public CSSOptionsBaseDlg
{
	CCtrlEdit edtProfile;
	CCtrlButton btnOk;

public:
	CAddProfileDlg() :
		CSSOptionsBaseDlg(IDD_ADDPROFILE),
		btnOk(this, IDOK),
		edtProfile(this, IDC_PROFILENAME)
	{
		edtProfile.OnChange = Callback(this, &CAddProfileDlg::onChange_Edit);
	}

	bool OnInitDialog() override
	{
		btnOk.Disable();
		return true;
	}

	bool OnApply() override
	{
		ptrW profileName(edtProfile.GetText());
		SendMessage(m_hwndParent, UM_ADDPROFILE, 0, (LPARAM)profileName.get());
		return true;
	}

	void OnDestroy() override
	{
		EnableWindow(m_hwndParent, TRUE);
	}

	void onChange_Edit(CCtrlEdit*)
	{
		btnOk.Enable(edtProfile.SendMsg(EM_LINELENGTH, 0, 0) > 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// advanced options dialog

static int DeleteSetting(const char *szSetting, void *lParam)
{
	LIST<char> *p = (LIST<char> *)lParam;
	p->insert(mir_strdup(szSetting));
	return 0;
}

class CSSAdvancedOptDlg : public CDlgBase
{
	bool bNeedRebuildMenu;

	OBJLIST<PROFILEOPTIONS> arProfiles;

	void ReinitProfiles()
	{
		// creates profile combo box according to 'dat'
		cmbProfile.ResetContent();
		for (auto &it : arProfiles) 
			cmbProfile.AddString(it->tszName, arProfiles.indexOf(&it));

		cmbProfile.SetCurSel(0);
		SetProfile();
	}

	void SetProfile()
	{
		int sel = cmbProfile.GetCurData();
		chkCreateTTB.SetState(arProfiles[sel].createTtb);
		chkShowDialog.SetState(arProfiles[sel].showDialog);
		chkCreateMMI.SetState(arProfiles[sel].createMmi);
		chkInSubmenu.SetState(arProfiles[sel].inSubMenu);
		chkInSubmenu.Enable(arProfiles[sel].createMmi);
		chkRegHotkey.SetState(arProfiles[sel].regHotkey);
		edtHotkey.SendMsg(HKM_SETHOTKEY, arProfiles[sel].hotKey, 0);
		edtHotkey.Enable(arProfiles[sel].regHotkey);

		// fill proto list
		lstAccount.ResetContent();
		for (auto &it : arProfiles[sel].ps)
			lstAccount.AddString(it->m_tszAccName, (LPARAM)it);
		lstAccount.SetCurSel(0);

		SetProtocol();
	}

	void SetProtocol()
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			// fill status box
			SMProto* ps = (SMProto*)lstAccount.GetItemData(idx);

			int flags = (CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_2, 0))&~(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_5, 0));
			lstStatus.ResetContent();
			for (auto &it : statusModes) {
				if ((flags & it.iFlag) || (it.iStatus == ID_STATUS_OFFLINE)) {
					int item = lstStatus.AddString(Clist_GetStatusModeDescription(it.iStatus, 0), it.iStatus);
					if (ps->m_status == it.iStatus)
						lstStatus.SetCurSel(item);
				}
			}

			int item = lstStatus.AddString(TranslateT("<current>"), ID_STATUS_CURRENT);
			if (ps->m_status == ID_STATUS_CURRENT)
				lstStatus.SetCurSel(item);

			item = lstStatus.AddString(TranslateT("<last>"), ID_STATUS_LAST);
			if (ps->m_status == ID_STATUS_LAST)
				lstStatus.SetCurSel(item);
		}

		SetStatusMsg();
	}

	// set status message
	void SetStatusMsg()
	{
		bool bStatusMsg = false;
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			SMProto *ps = (SMProto*)lstAccount.GetItemData(idx);

			CheckRadioButton(m_hwnd, IDC_MIRANDAMSG, IDC_CUSTOMMSG, ps->m_szMsg != nullptr ? IDC_CUSTOMMSG : IDC_MIRANDAMSG);
			if (ps->m_szMsg != nullptr)
				edtStatusMsg.SetText(ps->m_szMsg);

			bStatusMsg = ((((CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG)) &&
				(CallProtoService(ps->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(ps->m_status))) || (ps->m_status == ID_STATUS_CURRENT) || (ps->m_status == ID_STATUS_LAST));
		}
		chkMiranda.Enable(bStatusMsg);
		chkCustom.Enable(bStatusMsg);
		btnHelp.Enable(bStatusMsg && chkCustom.GetState());
		edtStatusMsg.Enable(bStatusMsg && chkCustom.GetState());
	}

	CCtrlEdit edtStatusMsg, edtHotkey;
	CCtrlCombo cmbProfile;
	CCtrlCheck chkMiranda, chkCustom, chkCreateMMI, chkInSubmenu, chkRegHotkey, chkCreateTTB, chkShowDialog;
	CCtrlButton btnAdd, btnDelete, btnHelp;
	CCtrlListBox lstStatus, lstAccount;

public:
	CSSAdvancedOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_STATUSPROFILES),
		cmbProfile(this, IDC_PROFILE),
		btnAdd(this, IDC_ADDPROFILE),
		btnHelp(this, IDC_VARIABLESHELP),
		btnDelete(this, IDC_DELPROFILE),
		chkCustom(this, IDC_CUSTOMMSG),
		chkMiranda(this, IDC_MIRANDAMSG),
		chkCreateMMI(this, IDC_CREATEMMI),
		chkInSubmenu(this, IDC_INSUBMENU),
		chkRegHotkey(this, IDC_REGHOTKEY),
		chkCreateTTB(this, IDC_CREATETTB),
		chkShowDialog(this, IDC_SHOWDIALOG),
		edtHotkey(this, IDC_HOTKEY),
		edtStatusMsg(this, IDC_STATUSMSG),
		lstStatus(this, IDC_STATUS),
		lstAccount(this, IDC_PROTOCOL),
		arProfiles(5),
		bNeedRebuildMenu(false)
	{
		btnAdd.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Add);
		btnHelp.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Help);
		btnDelete.OnClick = Callback(this, &CSSAdvancedOptDlg::onClick_Delete);

		cmbProfile.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Profile);

		chkCreateMMI.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_CreateMMI);
		chkInSubmenu.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_InSubmenu);
		chkCustom.OnChange = chkMiranda.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_StatusMsg);
		chkRegHotkey.OnChange = chkCreateTTB.OnChange = chkShowDialog.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Option);
		
		edtHotkey.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_Hotkey);
		edtStatusMsg.OnChange = Callback(this, &CSSAdvancedOptDlg::onChange_EdtStatusMsg);

		lstStatus.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Status);
		lstAccount.OnSelChange = Callback(this, &CSSAdvancedOptDlg::onChange_Account);
	}

	bool OnInitDialog() override
	{
		int defProfile;
		int profileCount = GetProfileCount((WPARAM)&defProfile, 0);
		if (profileCount == 0) {
			profileCount = 1;
			defProfile = 0;
			NotifyChange();
		}

		for (int i = 0; i < profileCount; i++) {
			PROFILEOPTIONS *ppo = new PROFILEOPTIONS;
			TProtoSettings &ar = ppo->ps;

			if (GetProfile(i, ar) == -1) {
				/* create an empty profile */
				if (i == defProfile)
					ppo->tszName = mir_wstrdup(TranslateT("default"));
				else
					ppo->tszName = mir_wstrdup(TranslateT("unknown"));
			}
			else {
				ppo->tszName = SSPlugin.getWStringA(OptName(i, SETTING_PROFILENAME));
				if (ppo->tszName == nullptr) {
					if (i == defProfile)
						ppo->tszName = mir_wstrdup(TranslateT("default"));
					else
						ppo->tszName = mir_wstrdup(TranslateT("unknown"));
				}
				ppo->createTtb = SSPlugin.getByte(OptName(i, SETTING_CREATETTBBUTTON), 0);
				ppo->showDialog = SSPlugin.getByte(OptName(i, SETTING_SHOWCONFIRMDIALOG), 0);
				ppo->createMmi = SSPlugin.getByte(OptName(i, SETTING_CREATEMMITEM), 0);
				ppo->inSubMenu = SSPlugin.getByte(OptName(i, SETTING_INSUBMENU), 1);
				ppo->regHotkey = SSPlugin.getByte(OptName(i, SETTING_REGHOTKEY), 0);
				ppo->hotKey = SSPlugin.getWord(OptName(i, SETTING_HOTKEY), MAKEWORD((char)('0' + i), HOTKEYF_CONTROL | HOTKEYF_SHIFT));
			}
			arProfiles.insert(ppo);
		}
		if (hTTBModuleLoadedHook == nullptr)
			chkCreateTTB.Disable();

		ReinitProfiles();
		ShowWindow(GetDlgItem(m_hwnd, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELPEX) ? SW_SHOW : SW_HIDE);
		return true;
	}

	bool OnApply() override
	{
		int oldCount = SSPlugin.getWord(SETTING_PROFILECOUNT, 0);
		for (int i = 0; i < oldCount; i++) {
			LIST<char> arSettings(10);
			db_enum_settings(0, DeleteSetting, SSMODULENAME, &arSettings);

			char setting[128];
			int len = mir_snprintf(setting, "%d_", i);
			for (auto &it : arSettings) {
				if (!strncmp(setting, it, len))
					SSPlugin.delSetting(it);
				mir_free(it);
			}
		}

		for (auto &it : arProfiles) {
			int i = arProfiles.indexOf(&it);
			SSPlugin.setByte(OptName(i, SETTING_SHOWCONFIRMDIALOG), it->showDialog);
			SSPlugin.setByte(OptName(i, SETTING_CREATETTBBUTTON), it->createTtb);
			SSPlugin.setByte(OptName(i, SETTING_CREATEMMITEM), it->createMmi);
			SSPlugin.setByte(OptName(i, SETTING_INSUBMENU), it->inSubMenu);
			SSPlugin.setByte(OptName(i, SETTING_REGHOTKEY), it->regHotkey);
			SSPlugin.setWord(OptName(i, SETTING_HOTKEY), it->hotKey);
			SSPlugin.setWString(OptName(i, SETTING_PROFILENAME), it->tszName);

			for (auto jt : it->ps) {
				if (jt->m_szMsg != nullptr) {
					char setting[128];
					mir_snprintf(setting, "%s_%s", jt->m_szName, SETTING_PROFILE_STSMSG);
					SSPlugin.setWString(OptName(i, setting), jt->m_szMsg);
				}
				SSPlugin.setWord(OptName(i, jt->m_szName), jt->m_status);
			}
		}
		SSPlugin.setWord(SETTING_PROFILECOUNT, (uint16_t)arProfiles.getCount());

		// Rebuild status menu
		if (bNeedRebuildMenu)
			Menu_ReloadProtoMenus();
		return true;
	}

	// add a profile
	void onClick_Add(CCtrlButton*)
	{
		CAddProfileDlg *pDlg = new CAddProfileDlg();
		pDlg->SetParent(m_hwnd);
		pDlg->Show();
		EnableWindow(m_hwnd, FALSE);
	}

	void onClick_Delete(CCtrlButton*)
	{
		// wparam == profile no
		int sel = cmbProfile.GetCurData();
		if (arProfiles.getCount() == 1) {
			MessageBox(nullptr, TranslateT("At least one profile must exist"), TranslateT("Status manager"), MB_OK);
			return;
		}

		arProfiles.remove(sel);

		int defProfile;
		GetProfileCount((WPARAM)&defProfile, 0);
		if (sel == defProfile) {
			MessageBox(nullptr, TranslateT("Your default profile will be changed"), TranslateT("Status manager"), MB_OK);
			SSPlugin.setWord(SETTING_DEFAULTPROFILE, 0);
		}
		ReinitProfiles();;
	}

	void onClick_Help(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, IDC_STATUSMSG, VHF_INPUT | VHF_EXTRATEXT | VHF_HELP | VHF_FULLFILLSTRUCT | VHF_HIDESUBJECTTOKEN, nullptr, "Protocol ID");
	}

	void onChange_Status(CCtrlListBox*)
	{
		int idx = lstAccount.GetCurSel();
		if (idx != -1) {
			SMProto* ps = (SMProto*)lstAccount.GetItemData(idx);
			ps->m_status = lstStatus.GetItemData(lstStatus.GetCurSel());
			NotifyChange();
		}
		SetStatusMsg();
	}

	void onChange_Account(CCtrlListBox*)
	{
		SetProtocol();
	}

	void onChange_Profile(CCtrlCombo*)
	{
		SetProfile();
	}

	void onChange_StatusMsg(CCtrlCheck*)
	{
		int len;
		SMProto* ps = (SMProto*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr)
			mir_free(ps->m_szMsg);

		ps->m_szMsg = nullptr;
		if (chkCustom.GetState()) {
			len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
			ps->m_szMsg = (wchar_t*)mir_calloc(sizeof(wchar_t) * (len + 1));
			GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
		}
		SetStatusMsg();
	}

	void onChange_Hotkey(CCtrlEdit*)
	{
		int sel = cmbProfile.GetCurData();
		arProfiles[sel].hotKey = edtHotkey.SendMsg(HKM_GETHOTKEY, 0, 0);
	}

	void onChange_EdtStatusMsg(CCtrlEdit*)
	{
		// update the status message in memory, this is done on each character tick, not nice
		// but it works
		SMProto* ps = (SMProto*)lstAccount.GetItemData(lstAccount.GetCurSel());
		if (ps->m_szMsg != nullptr) {
			if (*ps->m_szMsg)
				mir_free(ps->m_szMsg);
			ps->m_szMsg = nullptr;
		}
		int len = edtStatusMsg.SendMsg(WM_GETTEXTLENGTH, 0, 0);
		ps->m_szMsg = (wchar_t*)mir_calloc(sizeof(wchar_t) * (len + 1));
		GetDlgItemText(m_hwnd, IDC_STATUSMSG, ps->m_szMsg, (len + 1));
	}

	void onChange_CreateMMI(CCtrlCheck*)
	{
		chkInSubmenu.Enable(chkCreateMMI.GetState());
		onChange_InSubmenu(nullptr);
	}

	void onChange_InSubmenu(CCtrlCheck*)
	{
		bNeedRebuildMenu = TRUE;
		onChange_Option(nullptr);
	}

	void onChange_Option(CCtrlCheck*)
	{
		int sel = cmbProfile.GetCurData();
		PROFILEOPTIONS &po = arProfiles[sel];
		po.createMmi = chkCreateMMI.GetState();
		po.inSubMenu = chkInSubmenu.GetState();
		po.createTtb = chkCreateTTB.GetState();
		po.regHotkey = chkRegHotkey.GetState();
		po.showDialog = chkShowDialog.GetState();
		edtHotkey.Enable(chkRegHotkey.GetState());
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case UM_ADDPROFILE:
			wchar_t *tszName = (wchar_t*)lParam;
			if (tszName == nullptr)
				break;

			PROFILEOPTIONS* ppo = new PROFILEOPTIONS;
			ppo->tszName = mir_wstrdup(tszName);
			arProfiles.insert(ppo);

			ReinitProfiles();
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

int StartupStatusOptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Startup status");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new CSSMainOptDlg();
	SSPlugin.addOptions(wparam, &odp);

	odp.szTab.a = LPGEN("Status profiles");
	odp.pDialog = new CSSAdvancedOptDlg();
	SSPlugin.addOptions(wparam, &odp);
	return 0;
}
