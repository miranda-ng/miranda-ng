#include "stdafx.h"

#define WM_BACKUP_DONE (WM_USER+1)

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CDlgBase
{
	void SetDialogState()
	{
		auto *periodText = FindControl(IDC_ED_PERIOD);
		auto *numBackupsText = FindControl(IDC_ED_NUMBACKUPS);

		bool bEnabled = m_enabled.IsChecked();
		m_backupOnStart.Enable(bEnabled);
		m_backupOnExit.Enable(bEnabled);
		m_backupPeriodic.Enable(bEnabled);
		m_nextTime.Enable(bEnabled);
		numBackupsText->Enable(bEnabled);
		m_numBackups.Enable(bEnabled);
		m_backup.Enable(bEnabled);
		m_folder.Enable(bEnabled);
		m_browseFolder.Enable(bEnabled);
		m_filemask.Enable(bEnabled);
		m_foldersPageLink.Enable(bEnabled);
		m_enableProgress.Enable(bEnabled);
		m_useZip.Enable(bEnabled);
		periodText->Enable(bEnabled);
		m_period.Enable(bEnabled);
		m_periodType.Enable(bEnabled);
		m_backupProfile.Enable(bEnabled);
		m_useCloudFile.Enable(g_plugin.bCloudFilePresent && bEnabled);

		if (!bEnabled) {
			m_cloudFileService.Disable();

			m_backupOnStart.SetState(false);
			m_backupOnExit.SetState(false);
			m_backupPeriodic.SetState(false);
		}
		else {
			m_cloudFileService.Enable(m_useCloudFile.IsChecked());
			onChange_UseZip(0);

			uint8_t backupTypes = g_plugin.backup_types;
			if (backupTypes == BT_DISABLED)
				backupTypes = g_plugin.backup_types.Default();
			m_backupOnStart.SetState((backupTypes & BT_START) != 0);
			m_backupOnExit.SetState((backupTypes & BT_EXIT) != 0);
			m_backupPeriodic.SetState((backupTypes & BT_PERIODIC) != 0);
		}
	}

	void CreateToolTip(LPTSTR ptszText, LPTSTR ptszTitle)
	{
		HWND hwndFolder = m_folder.GetHwnd();

		m_hPathTip = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS, nullptr,
			(WS_POPUP | TTS_NOPREFIX),
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hwndFolder, nullptr, g_plugin.getInst(), nullptr);

		if (m_hPathTip == nullptr)
			return;

		SetWindowPos(m_hPathTip, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));

		TOOLINFO ti = {};
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
		ti.hwnd = hwndFolder;
		ti.hinst = g_plugin.getInst();
		ti.lpszText = ptszText;
		GetClientRect(hwndFolder, &ti.rect);
		ti.rect.left = -80;

		SendMessage(m_hPathTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		SendMessage(m_hPathTip, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
		SendMessage(m_hPathTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)650);

		SetTimer(m_hwnd, 0, 3000, nullptr);
	}

	static int CALLBACK BrowseProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM)
	{
		switch (uMsg) {
		case BFFM_INITIALIZED:
			wchar_t backupfolder[MAX_PATH];
			PathToAbsoluteW(VARSW(g_plugin.folder), backupfolder);
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)backupfolder);
			break;
		}
		return FALSE;
	}

	static int EnumCloudFileServices(const CFSERVICEINFO *serviceInfo, void *param)
	{
		CCtrlCombo &combo = *(CCtrlCombo*)param;
		int pos = combo.GetCount();
		combo.InsertString(serviceInfo->userName, pos, (LPARAM)serviceInfo->accountName);
		if (mir_strcmp(serviceInfo->accountName, g_plugin.cloudfile_service) == 0)
			combo.SetCurSel(pos);
		return 0;
	}

	CCtrlBase m_nextTime;
	CCtrlEdit m_folder, m_filemask;
	CCtrlSpin m_period, m_numBackups;
	CCtrlCheck m_enabled, m_backupOnStart, m_backupOnExit, m_backupPeriodic;
	CCtrlCheck m_enableProgress, m_useZip, m_backupProfile, m_useCloudFile;
	CCtrlCombo m_periodType, m_cloudFileService;
	CCtrlButton m_browseFolder, m_backup;
	CCtrlHyperlink m_foldersPageLink;

	HWND m_hPathTip;
	HANDLE m_hEvent;

	UI_MESSAGE_MAP(COptionsDlg, CDlgBase);
		UI_MESSAGE(WM_BACKUP_DONE, BackupDone);
	UI_MESSAGE_MAP_END();

	INT_PTR BackupDone(UINT, WPARAM, LPARAM)
	{
		onChange_Period(0);
		return 0;
	}

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_enabled(this, IDC_RAD_ENABLED),
		m_backupOnStart(this, IDC_RAD_START),
		m_backupOnExit(this, IDC_RAD_EXIT),
		m_backupPeriodic(this, IDC_RAD_PERIODIC),
		m_backup(this, IDC_BUT_NOW),
		m_backupProfile(this, IDC_BACKUPPROFILE),
		m_period(this, SPIN_PERIOD, 60, 1),
		m_periodType(this, IDC_PT),
		m_nextTime(this, IDC_NEXTTIME),
		m_folder(this, IDC_ED_FOLDER),
		m_browseFolder(this, IDC_BUT_BROWSE),
		m_filemask(this, IDC_FILEMASK),
		m_foldersPageLink(this, IDC_LNK_FOLDERS, nullptr),
		m_numBackups(this, SPIN_NUMBACKUPS, 9999, 1),
		m_enableProgress(this, IDC_CHK_PROGRESS),
		m_useZip(this, IDC_CHK_USEZIP),
		m_useCloudFile(this, IDC_CLOUDFILE),
		m_cloudFileService(this, IDC_CLOUDFILESEVICE)
	{
		CreateLink(m_period, g_plugin.period);
		CreateLink(m_numBackups, g_plugin.num_backups);
		CreateLink(m_useZip, g_plugin.use_zip);
		CreateLink(m_filemask, g_plugin.file_mask);
		CreateLink(m_backupProfile, g_plugin.backup_profile);
		CreateLink(m_useCloudFile, g_plugin.use_cloudfile);

		m_enableProgress.SetState(!g_plugin.disable_progress);

		m_enabled.OnChange = Callback(this, &COptionsDlg::onChange_Enabled);
		m_backupOnStart.OnChange = m_backupOnExit.OnChange = m_backupPeriodic.OnChange = Callback(this, &COptionsDlg::onChange_BackupType);
		m_useCloudFile.OnChange = Callback(this, &COptionsDlg::onChange_UseCloudFile);
		m_useZip.OnChange = Callback(this, &COptionsDlg::onChange_UseZip);
		m_periodType.OnSelChanged = m_period.OnChange = Callback(this, &COptionsDlg::onChange_Period);

		m_backup.OnClick = Callback(this, &COptionsDlg::onClick_Backup);
		m_browseFolder.OnClick = Callback(this, &COptionsDlg::onClick_BrowseFolder);
		m_foldersPageLink.OnClick = Callback(this, &COptionsDlg::onClick_FoldersPageLink);
	}

	bool OnInitDialog() override
	{
		m_hEvent = HookEventMessage(ME_AUTOBACKUP_DONE, m_hwnd, WM_BACKUP_DONE);

		m_enabled.SetState(g_plugin.backup_types != BT_DISABLED);
		m_backupOnStart.SetState(g_plugin.backup_types & BT_START ? TRUE : FALSE);
		m_backupOnExit.SetState(g_plugin.backup_types & BT_EXIT ? TRUE : FALSE);
		m_backupPeriodic.SetState(g_plugin.backup_types & BT_PERIODIC ? TRUE : FALSE);
		onChange_Period(0);

		m_numBackups.SetPosition(g_plugin.num_backups);

		m_periodType.AddString(TranslateT("days"));
		m_periodType.AddString(TranslateT("hours"));
		m_periodType.AddString(TranslateT("minutes"));
		m_periodType.SetCurSel(g_plugin.period_type);

		if (ServiceExists(MS_FOLDERS_GET_PATH)) {
			m_folder.Hide();
			m_browseFolder.Hide();
			m_foldersPageLink.Show();
		}
		else {
			m_folder.SetText(g_plugin.folder);

			wchar_t tszTooltipText[4096];
			mir_snwprintf(tszTooltipText, L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
				L"%miranda_path%", TranslateT("path to Miranda root folder"),
				L"%miranda_profilesdir%", TranslateT("path to folder containing Miranda profiles"),
				L"%miranda_profilename%", TranslateT("name of current Miranda profile (filename, without extension)"),
				L"%miranda_userdata%", TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
				L"%appdata%", TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
				L"%username%", TranslateT("username for currently logged-on Windows user"),
				L"%mydocuments%", TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
				L"%desktop%", TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
				L"%xxxxxxx%", TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"));
			CreateToolTip(tszTooltipText, TranslateT("Variables"));
		}

		m_useCloudFile.Enable(g_plugin.bCloudFilePresent);
		if (ServiceExists(MS_CLOUDFILE_ENUMSERVICES)) {
			m_cloudFileService.Enable();
			CallService(MS_CLOUDFILE_ENUMSERVICES, (WPARAM)&COptionsDlg::EnumCloudFileServices, (LPARAM)&m_cloudFileService);
		}

		SetDialogState();
		return true;
	}

	bool OnApply() override
	{
		uint8_t backupTypes = BT_DISABLED;

		if (m_backupOnStart.IsChecked())
			backupTypes |= BT_START;
		else
			backupTypes &= ~BT_START;

		if (m_backupOnExit.IsChecked())
			backupTypes |= BT_EXIT;
		else
			backupTypes &= ~BT_EXIT;

		if (m_backupPeriodic.IsChecked())
			backupTypes |= BT_PERIODIC;
		else
			backupTypes &= ~BT_PERIODIC;

		g_plugin.backup_types = backupTypes;

		SetBackupTimer();

		g_plugin.period_type = m_periodType.GetCurSel();
		g_plugin.disable_progress = !m_enableProgress.GetState();

		ptrW folder(m_folder.GetText());

		wchar_t backupfolder[MAX_PATH];
		PathToAbsoluteW(VARSW(folder), backupfolder);
		int err = CreateDirectoryTreeW(backupfolder);
		if (err != ERROR_ALREADY_EXISTS && err != 0) {
			wchar_t msg[512];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, msg, 512, nullptr);
			MessageBox(nullptr, msg, TranslateT("Error creating backup folder"), MB_OK | MB_ICONERROR);
			return false;
		}

		wcsncpy_s(g_plugin.folder, folder, _TRUNCATE);
		g_plugin.setWString("Folder", folder);

		int currentService = m_cloudFileService.GetCurSel();
		g_plugin.cloudfile_service = currentService >= 0
			? (char*)m_cloudFileService.GetItemData(currentService)
			: nullptr;
		return true;
	}

	void OnTimer(CTimer*) override
	{
		if (IsWindow(m_hPathTip))
			KillTimer(m_hwnd, 4); // It will prevent tooltip autoclosing
	}

	void OnDestroy() override
	{
		if (m_hPathTip) {
			KillTimer(m_hwnd, 0);
			DestroyWindow(m_hPathTip);
			m_hPathTip = nullptr;
		}
	}

	void onChange_Enabled(CCtrlBase*)
	{
		SetDialogState();
	}

	void onChange_Period(CCtrlSpin*)
	{
		int iMultiplicator;
		switch (m_periodType.GetCurSel()) {
		case PT_MINUTES: iMultiplicator = 60; break;
		case PT_HOURS: iMultiplicator = 3600; break;
		default: iMultiplicator = 86400; break; // days
		}

		time_t nextBackup = time_t(g_plugin.getDword("LastBackupTimestamp")) + m_period.GetPosition() * iMultiplicator;

		wchar_t buf[100];
		TimeZone_PrintTimeStamp(nullptr, nextBackup, L"D t", buf, _countof(buf), 0);
		m_nextTime.SetText(CMStringW(FORMAT, L"%s %s", TranslateT("Next backup is scheduled to"), buf));
	}

	void onChange_BackupType(CCtrlBase*)
	{
		bool bEnabled = m_backupPeriodic.IsChecked();
		m_nextTime.Enable(bEnabled);

		if (!m_backupOnStart.IsChecked() && !m_backupOnExit.IsChecked() && !bEnabled) {
			m_enabled.SetState(false);
			SetDialogState();
		}
	}

	void onChange_UseCloudFile(CCtrlBase*)
	{
		m_cloudFileService.Enable(m_useCloudFile.IsChecked());
	}

	void onChange_UseZip(CCtrlCheck*)
	{
		m_backupProfile.Enable(m_useZip.GetState());
	}

	void onClick_Backup(CCtrlButton*)
	{
		BackupStart(nullptr);
	}

	void onClick_BrowseFolder(CCtrlButton*)
	{
		wchar_t folder_buff[MAX_PATH] = { 0 };

		BROWSEINFO bi;
		bi.hwndOwner = m_hwnd;
		bi.pidlRoot = nullptr;
		bi.pszDisplayName = folder_buff;
		bi.lpszTitle = TranslateT("Select backup folder");
		bi.ulFlags = BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseProc;
		bi.lParam = 0;
		bi.iImage = 0;

		LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if (pidl != nullptr) {
			SHGetPathFromIDList(pidl, folder_buff);
			m_folder.SetText(folder_buff);
			CoTaskMemFree((void*)pidl);
		}
	}

	void onClick_FoldersPageLink(CCtrlHyperlink*)
	{
		g_plugin.openOptions(L"Customize", L"Folders");
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Automatic backups");
	odp.szGroup.a = LPGEN("Database");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
