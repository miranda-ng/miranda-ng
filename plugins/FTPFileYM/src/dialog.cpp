/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

UploadDialog *UploadDialog::instance = NULL;
UploadDialog *uDlg = NULL;

mir_cs UploadDialog::mutexTabs;

extern Options &opt;

UploadDialog::UploadDialog()
{
	m_hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLG_UPLOAD), 0, UploadDlgProc);
	m_hwndTabs = GetDlgItem(m_hwnd, IDC_TAB);
	EnableThemeDialogTexture(m_hwnd, ETDT_ENABLETAB);

	MONITORINFO mi = { 0 };
	mi.cbSize = sizeof(mi);
	HMONITOR hMonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(hMonitor, &mi);

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	rc.left = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - (rc.right - rc.left)) / 2;
	rc.top = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - (rc.bottom - rc.top)) / 2;
	SetWindowPos(m_hwnd, 0, rc.left, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

UploadDialog::~UploadDialog()
{
	if (m_hwnd) {
		ShowWindow(m_hwnd, SW_HIDE);
		SendMessage(m_hwnd, WMU_DESTROY, 0, 0);
	}

	instance = NULL;
	uDlg = NULL;
}

void UploadDialog::selectTab(int index)
{
	if (m_tabs[index] != NULL)
		m_tabs[index]->select();
}

void UploadDialog::show()
{
	ShowWindow(m_hwnd, SW_SHOWNORMAL);
}

UploadDialog::Tab::Tab(GenericJob *Job) :
	m_job(Job),
	m_bOptCloseDlg(opt.bCloseDlg),
	m_bOptCopyLink(opt.bCopyLink),
	m_bOptAutosend(opt.bAutosend),
	m_iOptAutoDelete(-1)
{
	if (opt.bAutoDelete)
		m_iOptAutoDelete = Utils::getDeleteTimeMin();

	m_stzSpeed[0] = 0;
	m_stzComplet[0] = 0;
	m_stzRemain[0] = 0;

	TCHAR buff[256];
	TCITEM tab = { 0 };
	tab.mask = TCIF_TEXT;
	tab.pszText = Utils::getTextFragment(m_job->m_tszFileName, 20, buff);
	TabCtrl_InsertItem(uDlg->m_hwndTabs, uDlg->m_tabs.size(), &tab);
	{
		mir_cslock lock(mutexTabs);
		uDlg->m_tabs.push_back(this);
	}
	select();
}

UploadDialog::Tab::~Tab()
{
	{
		mir_cslock lock(mutexTabs);
		TabCtrl_DeleteItem(uDlg->m_hwndTabs, index());
		uDlg->m_tabs.erase(uDlg->m_tabs.begin() + index());
	}

	if (m_job->isCompleted())
		delete m_job;

	if (uDlg->m_tabs.size() == 0)
		delete uDlg;
	else
		uDlg->selectTab(0);
}

int UploadDialog::Tab::index()
{
	for (UINT i = 0; i < uDlg->m_tabs.size(); i++)
		if (uDlg->m_tabs[i] == this)
			return i;

	return -1;
}

void UploadDialog::Tab::select()
{
	TabCtrl_SetCurSel(uDlg->m_hwndTabs, index());
	{
		mir_cslock lock(mutexTabs);
		uDlg->m_activeTab = index();
	}
	m_job->refreshTab(true);
	InvalidateRect(uDlg->m_hwnd, NULL, TRUE);
}

void UploadDialog::Tab::labelCompleted()
{
	TCHAR buff[64], buff2[256];
	mir_sntprintf(buff2, _countof(buff2), _T("* %s"), Utils::getTextFragment(m_job->m_tszFileName, 20, buff));

	TCITEM tab = { 0 };
	tab.mask = TCIF_TEXT;
	tab.pszText = buff2;
	TabCtrl_SetItem(uDlg->m_hwndTabs, index(), &tab);
}

LRESULT CALLBACK UploadDialog::TabControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(uDlg->m_hwndTabs, &pt);

			TCHITTESTINFO tch;
			tch.pt = pt;
			tch.flags = 0;
			int index = TabCtrl_HitTest(uDlg->m_hwndTabs, &tch);

			if (index != -1)
				uDlg->m_tabs[index]->m_job->closeTab();
		}
		break;
	}

	return mir_callNextSubclass(hwnd, UploadDialog::TabControlProc, msg, wParam, lParam);
}

INT_PTR CALLBACK UploadDialog::UploadDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Utils::loadIconEx("main"));

		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_TAB), TabControlProc);
		{
			LOGFONT logFont = { 0 };
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_ST_FILE, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(logFont), &logFont);
			logFont.lfWeight = FW_BOLD;
			hFont = CreateFontIndirect(&logFont);
			SendDlgItemMessage(hwndDlg, IDC_ST_CONTACT, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ST_FILE, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ST_SERVER, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ST_SPEED, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ST_COMPLETED, WM_SETFONT, (WPARAM)hFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ST_REMAIN, WM_SETFONT, (WPARAM)hFont, 0);

			logFont.lfHeight -= 4;
			hFont = CreateFontIndirect(&logFont);
			SendDlgItemMessage(hwndDlg, IDC_UP_CONTACT, WM_SETFONT, (WPARAM)hFont, 0);

			SendDlgItemMessage(hwndDlg, IDC_BTN_PROTO, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_PROTO, BUTTONADDTOOLTIP, (WPARAM)TranslateT("User Info"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Pause"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("clipboard"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Copy Link to Clipboard"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_URL));
			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open in Browser"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("main"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BUTTONADDTOOLTIP, (WPARAM)TranslateT("FTP File Manager"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_OPTIONS));
			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Options"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(SKINICON_OTHER_DELETE));
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Close"), BATF_TCHAR);
		}
		break;

	case WM_MEASUREITEM:
		return Menu_MeasureItem((LPMEASUREITEMSTRUCT)lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem((LPDRAWITEMSTRUCT)lParam);

	case WM_COMMAND:
		{
			MCONTACT hContact = uDlg->m_tabs[uDlg->m_activeTab]->m_job->m_hContact;
			if (hContact != NULL)
				if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), hContact))
					break;

			if (HIWORD(wParam) == BN_CLICKED) {
				switch (LOWORD(wParam)) {
				case IDC_BTN_PROTO:
					if (hContact != NULL) {
						RECT rc;
						GetWindowRect((HWND)lParam, &rc);
						HMENU hMenu = Menu_BuildContactMenu(hContact);
						TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
						DestroyMenu(hMenu);
					}
					break;

				case IDC_BTN_PAUSE:
					uDlg->m_tabs[uDlg->m_activeTab]->m_job->pauseHandler();
					return TRUE;

				case IDC_BTN_CLIPBOARD:
					{
						UploadJob *job = (UploadJob *)uDlg->m_tabs[uDlg->m_activeTab]->m_job;
						Utils::copyToClipboard(job->m_szFileLink);
					}
					return TRUE;

				case IDC_BTN_DOWNLOAD:
					{
						UploadJob *job = (UploadJob *)uDlg->m_tabs[uDlg->m_activeTab]->m_job;
						ShellExecuteA(NULL, "open", job->m_szFileLink, NULL, NULL, SW_SHOWNORMAL);
					}
					return TRUE;

				case IDC_BTN_FILEMANAGER:
					CallService(MS_FTPFILE_SHOWMANAGER, 0, 0);
					return TRUE;

				case IDC_BTN_CLOSE:
					uDlg->m_tabs[uDlg->m_activeTab]->m_job->closeTab();
					return TRUE;

				case IDC_BTN_OPTIONS:
					Tab *tab = uDlg->m_tabs[uDlg->m_activeTab];

					POINT pt;
					GetCursorPos(&pt);
					SetForegroundWindow(uDlg->m_hwndTabs);

					HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_UPLOAD));
					if (hMenu) {
						HMENU hPopupMenu = GetSubMenu(hMenu, 0);
						TranslateMenu(hPopupMenu);
						CheckMenuItem(hPopupMenu, IDM_CLOSEDLG, MF_BYCOMMAND | tab->m_bOptCloseDlg ? MF_CHECKED : 0);
						CheckMenuItem(hPopupMenu, IDM_COPYLINK, MF_BYCOMMAND | tab->m_bOptCopyLink ? MF_CHECKED : 0);
						CheckMenuItem(hPopupMenu, IDM_AUTOSEND, MF_BYCOMMAND | tab->m_bOptAutosend ? MF_CHECKED : 0);

						HMENU hTimeMenu = GetSubMenu(hPopupMenu, 3);
						CheckMenuItem(hTimeMenu, IDM_DISABLED, MF_BYCOMMAND | (tab->m_iOptAutoDelete == -1) ? MF_CHECKED : MF_UNCHECKED);

						int times[10] = { 5, 15, 30, 1 * 60, 2 * 60, 5 * 60, 10 * 60,  1 * 24 * 60, 2 * 24 * 60, 7 * 24 * 60 };
						bool bChecked = (tab->m_iOptAutoDelete == -1);
						TCHAR buff[256];

						for (int i = 0; i < _countof(times); i++) {
							if (i == 3 || i == 7)
								AppendMenu(hTimeMenu, MF_SEPARATOR, 0, 0);

							if (i < 3)
								mir_sntprintf(buff, TranslateT("%d minutes"), times[i]);
							else if (i < 7)
								mir_sntprintf(buff, TranslateT("%d hours"), times[i] / 60);
							else
								mir_sntprintf(buff, TranslateT("%d days"), times[i] / 60 / 24);

							UINT check = MF_UNCHECKED;
							if (!bChecked && tab->m_iOptAutoDelete == times[i]) {
								check = MF_CHECKED;
								bChecked = true;
							}

							AppendMenu(hTimeMenu, MF_STRING | check, times[i], buff);
						}

						if (opt.bAutoDelete) {
							switch (opt.timeRange) {
							case Options::TR_MINUTES: mir_sntprintf(buff, TranslateT("%d minutes"), opt.iDeleteTime); break;
							case Options::TR_HOURS: mir_sntprintf(buff, TranslateT("%d hours"), opt.iDeleteTime); break;
							case Options::TR_DAYS: mir_sntprintf(buff, TranslateT("%d days"), opt.iDeleteTime); break;
							}

							AppendMenu(hTimeMenu, MF_SEPARATOR, 0, 0);
							AppendMenu(hTimeMenu, MF_STRING | bChecked ? MF_UNCHECKED : MF_CHECKED, IDM_CUSTOM, buff);
						}

						int command = TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, uDlg->m_hwndTabs, NULL);
						switch (command) {
						case IDM_CLOSEDLG:
							tab->m_bOptCloseDlg = !tab->m_bOptCloseDlg; break;
						case IDM_COPYLINK:
							tab->m_bOptCopyLink = !tab->m_bOptCopyLink; break;
						case IDM_AUTOSEND:
							tab->m_bOptAutosend = !tab->m_bOptAutosend; break;
						case IDM_DISABLED:
							tab->m_iOptAutoDelete = -1; break;
						case IDM_CUSTOM:
							switch (opt.timeRange) {
							case Options::TR_MINUTES: tab->m_iOptAutoDelete = opt.iDeleteTime; break;
							case Options::TR_HOURS: tab->m_iOptAutoDelete = opt.iDeleteTime * 60; break;
							case Options::TR_DAYS: tab->m_iOptAutoDelete = opt.iDeleteTime * 60 * 24; break;
							}
							break;
						default:
							tab->m_iOptAutoDelete = command;
							break;
						}

						if (command && ((UploadJob *)tab->m_job)->isCompleted())
							DBEntry::setDeleteTS(tab->m_job);

						DestroyMenu(hMenu);
					}
					return TRUE;
				}
			}
			break;
		}

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case TCN_SELCHANGE:
			uDlg->selectTab(TabCtrl_GetCurSel(uDlg->m_hwndTabs));
			break;

		case TTN_GETDISPINFO:
			LPNMTTDISPINFO pInfo = (LPNMTTDISPINFO)lParam;

			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwndDlg, &pt);

			UINT i; RECT rc;
			for (i = 0; i < uDlg->m_tabs.size(); i++) {
				if (uDlg->m_tabs[i] != NULL) {
					TabCtrl_GetItemRect(uDlg->m_hwndTabs, i, &rc);
					if (PtInRect(&rc, pt))
						break;
				}
			}

			if (i < uDlg->m_tabs.size()) {
				uDlg->m_tabs[i]->m_job->createToolTip();
				HWND hToolTip = TabCtrl_GetToolTips(uDlg->m_hwndTabs);
				SendMessage(hToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)500);
				pInfo->lpszText = uDlg->m_tszToolTipText;
			}

			break;
		}
		break;

	case WM_CLOSE:
		{
			for (UINT i = 0; i < uDlg->m_tabs.size(); i++)
				uDlg->m_tabs[i]->m_job->pause();

			int result = IDYES;
			if (!Miranda_Terminated() && UploadJob::iRunningJobCount > 0)
				result = Utils::msgBox(TranslateT("Do you really want to cancel all running jobs?"), MB_YESNO | MB_ICONQUESTION);

			if (result == IDYES) {
				SkinPlaySound(SOUND_CANCEL);
				size_t count = uDlg->m_tabs.size();
				for (UINT i = 0; i < count; i++)
					uDlg->m_tabs[0]->m_job->closeAllTabs();
			}
			else {
				for (UINT i = 0; i < uDlg->m_tabs.size(); i++)
					uDlg->m_tabs[i]->m_job->resume();
			}
		}
		break;

	case WMU_DESTROY:
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}
