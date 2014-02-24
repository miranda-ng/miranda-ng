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

#include "common.h"

UploadDialog *UploadDialog::instance = NULL;
UploadDialog *uDlg = NULL;

Mutex UploadDialog::mutexTabs;

extern Options &opt;

UploadDialog::UploadDialog()
{
	this->hwnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLG_UPLOAD), 0, UploadDlgProc);
	this->hwndTabs = GetDlgItem(this->hwnd, IDC_TAB);
	EnableThemeDialogTexture(this->hwnd, ETDT_ENABLETAB);

	MONITORINFO mi = {0};
	mi.cbSize = sizeof(mi);
	HMONITOR hMonitor = MonitorFromWindow(this->hwnd, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(hMonitor, &mi);

	RECT rc;
	GetWindowRect(this->hwnd, &rc);
	rc.left = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - (rc.right-rc.left)) / 2;
	rc.top = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - (rc.bottom-rc.top)) / 2;
	SetWindowPos(this->hwnd, 0, rc.left, rc.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

UploadDialog::~UploadDialog()
{
	if (this->hwnd) 
	{
		ShowWindow(this->hwnd, SW_HIDE);
		SendMessage(this->hwnd, WMU_DESTROY, 0, 0);
	}

	instance = NULL;
	uDlg = NULL;
}

void UploadDialog::selectTab(int index)
{
	if (tabs[index] != NULL)
		tabs[index]->select();
}

void UploadDialog::show()
{
	ShowWindow(this->hwnd, SW_SHOWNORMAL);
}

UploadDialog::Tab::Tab(GenericJob *Job)
:job(Job),bOptCloseDlg(opt.bCloseDlg),bOptCopyLink(opt.bCopyLink),bOptAutosend(opt.bAutosend),iOptAutoDelete(-1) 
{ 
	if (opt.bAutoDelete)
		this->iOptAutoDelete = Utils::getDeleteTimeMin();

	this->stzSpeed[0] = 0;
	this->stzComplet[0] = 0;
	this->stzRemain[0] = 0;

	TCHAR buff[256];
	TCITEM tab = {0};
	tab.mask = TCIF_TEXT;
	tab.pszText = Utils::getTextFragment(job->stzFileName, 20, buff);
	TabCtrl_InsertItem(uDlg->hwndTabs, uDlg->tabs.size(), &tab);

	Lock *lock = new Lock(mutexTabs);
	uDlg->tabs.push_back(this);
	delete lock;

	this->select();
}

UploadDialog::Tab::~Tab()
{ 
	Lock *lock = new Lock(mutexTabs);
	TabCtrl_DeleteItem(uDlg->hwndTabs, this->index());
	uDlg->tabs.erase(uDlg->tabs.begin() + this->index());
	delete lock;

	if (this->job->isCompleted())
		delete this->job;

	if (uDlg->tabs.size() == 0) 
		delete uDlg;
	else 
		uDlg->selectTab(0);
}

int UploadDialog::Tab::index()
{
	for (UINT i = 0; i < uDlg->tabs.size(); i++)
	{
		if (uDlg->tabs[i] == this)
			return i;
	}

	return -1;
}

void UploadDialog::Tab::select()
{ 
	TabCtrl_SetCurSel(uDlg->hwndTabs, this->index());

	Lock *lock = new Lock(mutexTabs);
	uDlg->activeTab = this->index();
	delete lock;

	this->job->refreshTab(true);
	InvalidateRect(uDlg->hwnd, NULL, TRUE);
}

void UploadDialog::Tab::labelCompleted()
{ 
	TCHAR buff[64], buff2[256];
	mir_sntprintf(buff2, SIZEOF(buff2), _T("* %s"), Utils::getTextFragment(this->job->stzFileName, 20, buff));

	TCITEM tab = {0};
	tab.mask = TCIF_TEXT;
	tab.pszText = buff2;
	TabCtrl_SetItem(uDlg->hwndTabs, this->index(), &tab);
}

LRESULT CALLBACK UploadDialog::TabControlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) {
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(uDlg->hwndTabs, &pt);

			TCHITTESTINFO tch;
			tch.pt = pt;
			tch.flags = 0;
			int index = TabCtrl_HitTest(uDlg->hwndTabs, &tch);

			if (index != -1)
				uDlg->tabs[index]->job->closeTab();
		}
		break;
	}

	return mir_callNextSubclass(hwnd, UploadDialog::TabControlProc, msg, wParam, lParam);
}

INT_PTR CALLBACK UploadDialog::UploadDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{	
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Utils::loadIconEx("main"));

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_TAB), TabControlProc);

			LOGFONT logFont = {0};
			HFONT hFont = (HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_ST_FILE), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(logFont), &logFont);
			logFont.lfWeight = FW_BOLD;
			hFont = CreateFontIndirect(&logFont);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_CONTACT), WM_SETFONT, (WPARAM)hFont, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_FILE), WM_SETFONT, (WPARAM)hFont, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_SERVER), WM_SETFONT, (WPARAM)hFont, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_SPEED), WM_SETFONT, (WPARAM)hFont, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_COMPLETED), WM_SETFONT, (WPARAM)hFont, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_ST_REMAIN), WM_SETFONT, (WPARAM)hFont, 0);

			logFont.lfHeight -= 4;
			hFont = CreateFontIndirect(&logFont);
			SendMessage(GetDlgItem(hwndDlg, IDC_UP_CONTACT), WM_SETFONT, (WPARAM)hFont, 0);

			SendDlgItemMessage(hwndDlg, IDC_BTN_PROTO, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_PROTO, BUTTONADDTOOLTIP, (WPARAM)TranslateT("User Info"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("pause"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_PAUSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Pause"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("clipboard"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLIPBOARD, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Copy Link to Clipboard"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_URL));
			SendDlgItemMessage(hwndDlg, IDC_BTN_DOWNLOAD, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open in Browser"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Utils::loadIconEx("main"));
			SendDlgItemMessage(hwndDlg, IDC_BTN_FILEMANAGER, BUTTONADDTOOLTIP, (WPARAM)TranslateT("FTP File Manager"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_OPTIONS));
			SendDlgItemMessage(hwndDlg, IDC_BTN_OPTIONS, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Options"), BATF_TCHAR);

			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_DELETE));
			SendDlgItemMessage(hwndDlg, IDC_BTN_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Close"), BATF_TCHAR);

			break;
		}
		case WM_MEASUREITEM:
		{
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		case WM_DRAWITEM:
		{
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
		case WM_COMMAND:
		{
			MCONTACT hContact = uDlg->tabs[uDlg->activeTab]->job->hContact;
			if (hContact != NULL)
			{
				if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), hContact))
					break;
			}

			if (HIWORD(wParam) == BN_CLICKED) 
			{
				switch (LOWORD(wParam)) 
				{
					case IDC_BTN_PROTO:
					{
						if (hContact != NULL)
						{
							RECT rc;
							HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
							GetWindowRect((HWND)lParam, &rc);
							TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
							DestroyMenu(hMenu);
						}

						break;
					}
					case IDC_BTN_PAUSE:
					{
						uDlg->tabs[uDlg->activeTab]->job->pauseHandler();
						return TRUE;
					}
					case IDC_BTN_CLIPBOARD:
					{
						UploadJob *job = (UploadJob *)uDlg->tabs[uDlg->activeTab]->job;
						Utils::copyToClipboard(job->szFileLink);
						return TRUE;
					}
					case IDC_BTN_DOWNLOAD:
					{
						UploadJob *job = (UploadJob *)uDlg->tabs[uDlg->activeTab]->job;
						ShellExecuteA(NULL, "open", job->szFileLink, NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
					}
					case IDC_BTN_FILEMANAGER:
					{
						CallService(MS_FTPFILE_SHOWMANAGER, 0, 0);
						return TRUE;
					}
					case IDC_BTN_OPTIONS:
					{
						Tab *tab = uDlg->tabs[uDlg->activeTab];

						POINT pt;
						GetCursorPos(&pt);
						SetForegroundWindow(uDlg->hwndTabs);

						HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_UPLOAD)); 
						if (hMenu) 
						{
							HMENU hPopupMenu = GetSubMenu(hMenu, 0); 
							TranslateMenu(hPopupMenu);
							CheckMenuItem(hPopupMenu, IDM_CLOSEDLG, MF_BYCOMMAND | tab->bOptCloseDlg ? MF_CHECKED : 0);
							CheckMenuItem(hPopupMenu, IDM_COPYLINK, MF_BYCOMMAND | tab->bOptCopyLink ? MF_CHECKED : 0);
							CheckMenuItem(hPopupMenu, IDM_AUTOSEND, MF_BYCOMMAND | tab->bOptAutosend ? MF_CHECKED : 0);

							HMENU hTimeMenu = GetSubMenu(hPopupMenu, 3); 
							CheckMenuItem(hTimeMenu, IDM_DISABLED, MF_BYCOMMAND | (tab->iOptAutoDelete == -1) ? MF_CHECKED : MF_UNCHECKED);

							int times[10] = {5, 15, 30, 1 * 60, 2 * 60, 5 * 60, 10 * 60,  1 * 24 * 60, 2 * 24 * 60, 7 * 24 * 60};
							bool bChecked = (tab->iOptAutoDelete == -1) ? true : false;
							TCHAR buff[256];

							for (int i = 0; i < SIZEOF(times); i++) 
							{
								if (i == 3 || i == 7)
									AppendMenu(hTimeMenu, MF_SEPARATOR, 0, 0);

								if (i < 3)
									mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d minutes"), times[i]);
								else if (i < 7)
									mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d hours"), times[i] / 60);
								else 
									mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d days"), times[i] / 60 / 24);

								UINT check = MF_UNCHECKED;
								if (!bChecked && tab->iOptAutoDelete == times[i])
								{ 
									check = MF_CHECKED;
									bChecked = true;
								}

								AppendMenu(hTimeMenu, MF_STRING | check, times[i], buff);
							}

							if (opt.bAutoDelete) 
							{
								switch (opt.timeRange) 
								{
									case Options::TR_MINUTES: mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d minutes"), opt.iDeleteTime); break;
									case Options::TR_HOURS: mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d hours"), opt.iDeleteTime); break;
									case Options::TR_DAYS: mir_sntprintf(buff, SIZEOF(buff), TranslateT("%d days"), opt.iDeleteTime); break;
								}

								AppendMenu(hTimeMenu, MF_SEPARATOR, 0, 0);
								AppendMenu(hTimeMenu, MF_STRING | bChecked ? MF_UNCHECKED : MF_CHECKED, IDM_CUSTOM, buff);
							}

							int command = TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, uDlg->hwndTabs, NULL);
							switch (command) 
							{
								case IDM_CLOSEDLG:
									tab->bOptCloseDlg = !tab->bOptCloseDlg; break;
								case IDM_COPYLINK:
									tab->bOptCopyLink = !tab->bOptCopyLink; break;
								case IDM_AUTOSEND:
									tab->bOptAutosend = !tab->bOptAutosend; break;
								case IDM_DISABLED:
									tab->iOptAutoDelete = -1; break;
								case IDM_CUSTOM:
									switch (opt.timeRange) 
									{
										case Options::TR_MINUTES: tab->iOptAutoDelete = opt.iDeleteTime; break;
										case Options::TR_HOURS: tab->iOptAutoDelete = opt.iDeleteTime * 60; break;
										case Options::TR_DAYS: tab->iOptAutoDelete = opt.iDeleteTime * 60 * 24; break;
									}
									break;
								default:
									tab->iOptAutoDelete = command;
									break;
							}

							if (command && ((UploadJob *)tab->job)->isCompleted()) 
								DBEntry::setDeleteTS(tab->job);

							DestroyMenu(hMenu);
						}
						return TRUE;
					}
					case IDC_BTN_CLOSE:
					{
						uDlg->tabs[uDlg->activeTab]->job->closeTab();
						return TRUE;
					}
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
            switch (((LPNMHDR)lParam)->code)
            {
				case TCN_SELCHANGE:
                { 
					uDlg->selectTab(TabCtrl_GetCurSel(uDlg->hwndTabs));
					break;
                }
				case TTN_GETDISPINFO:
				{
					LPNMTTDISPINFO pInfo = (LPNMTTDISPINFO)lParam;

					POINT pt;
					GetCursorPos(&pt);
					ScreenToClient(hwndDlg, &pt);

					UINT i; RECT rc;	
					for (i = 0; i < uDlg->tabs.size(); i++) 
					{
						if (uDlg->tabs[i] != NULL)
						{
							TabCtrl_GetItemRect(uDlg->hwndTabs, i, &rc);
							if (PtInRect(&rc, pt)) break;
						}
					}

					if (i < uDlg->tabs.size())
					{
						uDlg->tabs[i]->job->createToolTip();
						HWND hToolTip = TabCtrl_GetToolTips(uDlg->hwndTabs);
						SendMessage(hToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)500);
						pInfo->lpszText = uDlg->stzToolTipText;	
					}

					break;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			for (UINT i = 0; i < uDlg->tabs.size(); i++) 
				uDlg->tabs[i]->job->pause();

			int result = IDYES; 
			if (!Miranda_Terminated() && UploadJob::iRunningJobCount > 0)
				result = Utils::msgBox(TranslateT("Do you really want to cancel all running jobs?"), MB_YESNO | MB_ICONQUESTION);

			if (result == IDYES) 
			{
				SkinPlaySound(SOUND_CANCEL);
				size_t count = uDlg->tabs.size();
				for (UINT i = 0; i < count; i++) 
					uDlg->tabs[0]->job->closeAllTabs();
			}
			else
			{
				for (UINT i = 0; i < uDlg->tabs.size(); i++) 
					uDlg->tabs[i]->job->resume();		
			}	

			break;
		}
		case WMU_DESTROY:
		{
			DestroyWindow(hwndDlg);
			break;
		}
	}

	return FALSE;
}