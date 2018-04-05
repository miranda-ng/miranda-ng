/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
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

static const WORD aa_Status[] = { ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH };

int IdleGetStatusIndex(WORD status)
{
	for (int j = 0; j < _countof(aa_Status); j++)
		if (aa_Status[j] == status)
			return j;

	return 0;
}

class COptionsDlg : public CPluginDlgBase
{
	CCtrlEdit edt1sttime;
	CCtrlSpin spinIdle;
	CCtrlCombo cmbAAStatus;
	CCtrlCheck chkShort, chkOnWindows, chkOnMiranda, chkScreenSaver, chkFullScreen, chkLocked;
	CCtrlCheck chkPrivate, chkStatusLock, chkTerminal, chkSoundsOff, chkShortIdle;

	void ShowHide()
	{
		BOOL bChecked = chkShort.GetState();
		chkOnWindows.Enable(bChecked);
		chkOnMiranda.Enable(bChecked);
		edt1sttime.Enable(bChecked);

		bChecked = chkShortIdle.GetState();
		cmbAAStatus.Enable(bChecked);
		chkStatusLock.Enable(bChecked);
	}

public:
	COptionsDlg() :
		CPluginDlgBase(hInst, IDD_OPT_IDLE, IDLEMOD),
		edt1sttime(this, IDC_IDLE1STTIME),
		spinIdle(this, IDC_IDLESPIN),
		cmbAAStatus(this, IDC_AASTATUS),
		chkShort(this, IDC_IDLESHORT),
		chkLocked(this, IDC_LOCKED),
		chkPrivate(this, IDC_IDLEPRIVATE),
		chkTerminal(this, IDC_IDLETERMINAL),
		chkOnWindows(this, IDC_IDLEONWINDOWS),
		chkSoundsOff(this, IDC_IDLESOUNDSOFF),
		chkOnMiranda(this, IDC_IDLEONMIRANDA),
		chkShortIdle(this, IDC_AASHORTIDLE),
		chkStatusLock(this, IDC_IDLESTATUSLOCK),
		chkFullScreen(this, IDC_FULLSCREEN),
		chkScreenSaver(this, IDC_SCREENSAVER)
	{
		CreateLink(chkShort, S.bIdleCheck);
		CreateLink(chkLocked, S.bIdleOnLock);
		CreateLink(chkPrivate, S.bIdlePrivate);
		CreateLink(chkTerminal, S.bIdleOnTerminal);
		CreateLink(chkShortIdle, S.bAAEnable);
		CreateLink(chkOnMiranda, S.bIdleMethod);
		CreateLink(chkSoundsOff, S.bIdleSoundsOff);
		CreateLink(chkStatusLock, S.bIdleStatusLock);
		CreateLink(chkFullScreen, S.bIdleOnFullScr);
		CreateLink(chkScreenSaver, S.bIdleOnSaver);

		chkShortIdle.OnChange = chkShort.OnChange = Callback(this, &COptionsDlg::onChange);
	}

	virtual void OnInitDialog() override
	{
		chkOnWindows.SetState(!S.bIdleMethod);

		spinIdle.SetRange(60, 1);
		spinIdle.SetPosition(S.iIdleTime1st);

		for (auto &it : aa_Status)
			cmbAAStatus.AddString(Clist_GetStatusModeDescription(it, 0));
		cmbAAStatus.SetCurSel(IdleGetStatusIndex(S.bAAStatus));

		ShowHide();
	}

	virtual void OnApply() override
	{
		S.iIdleTime1st = spinIdle.GetPosition();

		int curSel = cmbAAStatus.GetCurSel();
		if (curSel != CB_ERR)
			S.bAAStatus = aa_Status[curSel];

		// destroy any current idle and reset settings.
		IdleObject_Destroy();
		IdleObject_Create();
	}

	void onChange(CCtrlCheck*)
	{
		ShowHide();
	}
};

int IdleOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_IDLE);
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Idle");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptionsDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}
