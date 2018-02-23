/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#pragma once

enum BackupType
{
	BT_DISABLED = 0,
	BT_START = 1,
	BT_EXIT = 2,
	BT_PERIODIC = 4
};

enum PeriodType
{
	PT_DAYS,
	PT_HOURS,
	PT_MINUTES
};

struct Options
{
	CMOption<BYTE>	backup_types;
	CMOption<WORD>	period;
	CMOption<BYTE>	period_type;
	wchar_t			folder[MAX_PATH];
	CMOption<WORD>	num_backups;
	CMOption<BYTE>	disable_progress;
	CMOption<BYTE>	disable_popups;
	CMOption<BYTE>	use_zip;
	CMOption<BYTE>	backup_profile;
	CMOption<BYTE>	use_cloudfile;
	CMOption<char*>	cloudfile_service;

	Options();
};

class COptionsDlg : public CPluginDlgBase
{
	CCtrlCheck m_disable;
	CCtrlCheck m_backupOnStart;
	CCtrlCheck m_backupOnExit;
	CCtrlCheck m_backupPeriodic;
	CCtrlButton m_backup;
	CCtrlSpin m_period;
	CCtrlCombo m_periodType;
	CCtrlEdit m_folder;
	CCtrlButton m_browseFolder;
	CCtrlHyperlink m_foldersPageLink;
	CCtrlSpin m_numBackups;
	CCtrlCheck m_disableProgress;
	CCtrlCheck m_disablePopups;
	CCtrlCheck m_useZip;
	CCtrlCheck m_backupProfile;
	CCtrlCheck m_useCloudFile;
	CCtrlCombo m_cloudFileService;

	HWND m_hPathTip;

protected:
	void OnInitDialog() override;
	void OnApply() override;
	void OnTimer(CTimer*) override;
	void OnDestroy() override;

	void Disable_OnChange(CCtrlBase*);
	void UseCloudFile_OnChange(CCtrlBase*);

	void Backup_OnClick(CCtrlButton*);
	void BrowseFolder_OnClick(CCtrlButton*);
	void FoldersPageLink_OnClick(CCtrlHyperlink*);

	void SetDialogState();

	void CreateToolTip(LPTSTR ptszText, LPTSTR ptszTitle);

	static int CALLBACK BrowseProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM);

	static int EnumCloudFileServices(const CFSERVICEINFO *serviceInfo, void *param);

public:
	COptionsDlg();
};