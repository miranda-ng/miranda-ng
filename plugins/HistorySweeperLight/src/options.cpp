/*
Miranda IM History Sweeper Light plugin
Copyright (C) 2002-2003  Sergey V. Gershovich
Copyright (C) 2006-2009  Boris Krasnovskiy
Copyright (C) 2010, 2011 tico-tico

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

#include "historysweeperlight.h"

// Time Stamps strings
const char* time_stamp_strings[] =
{
	"Delete older than 1 day",
	"Delete older than 3 days",
	"Delete older than 7 days",
	"Delete older than 2 weeks (14 days)",
	"Delete older than 1 month (30 days)",
	"Delete older than 3 months (90 days)",
	"Delete older than 6 months (180 days)",
	"Delete older than 1 year (365 days)"
};

const char* keep_strings[] =
{
	"Keep 1 last event",
	"Keep 2 last events",
	"Keep 5 last events",
	"Keep 10 last events",
	"Keep 20 last events",
	"Keep 50 last events"
};

struct
{
	char* szDescr;
	char* szName;
	int   defIconID;
}
static const iconList[] =
{
	{ "Default Action", "actG", IDI_ACTG },
	{ "Action 1", "act1", IDI_ACT1 },
	{ "Action 2", "act2", IDI_ACT2 },
	{ "Delete All", "actDel", IDI_ACTDEL }
};

static HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFile;
	sid.cx = sid.cy = 16;
	sid.pszSection = ModuleName;
	sid.flags = SIDF_PATH_TCHAR;

	for (int i=0; i < SIZEOF(iconList); i++) {
		char szSettingName[100];
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", ModuleName, iconList[i].szName);
		sid.pszName = szSettingName;

		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = Skin_AddIcon(&sid);
	}
}

HICON LoadIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", ModuleName, name);
	return Skin_GetIcon(szSettingName);
}

HANDLE GetIconHandle(const char* name)
{
	int i;

	for (i=0; i < SIZEOF(iconList); i++)
		if (lstrcmpA(iconList[i].szName, name) == 0)
			return hIconLibItem[i];

	return NULL;
}

void  ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", ModuleName, name);
	Skin_ReleaseIcon(szSettingName);
}

HANDLE hAllContacts, hSystemHistory;

void ResetListOptions(HWND hwndList)
{
	int i;

	SendMessage(hwndList, CLM_SETBKBITMAP, 0, 0);
	SendMessage(hwndList, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(hwndList, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(hwndList, CLM_SETLEFTMARGIN, 4, 0);
	SendMessage(hwndList, CLM_SETINDENT, 10, 0);

	for (i=0; i<=FONTID_MAX; i++)
		SendMessage(hwndList, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));
}

static void ShowAllContactIcons(HWND hwndList)
{
	HANDLE hContact, hItem;

	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hAllContacts,
											MAKELPARAM(0, DBGetContactSettingByte(NULL, ModuleName, "SweepHistory", 0)));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hSystemHistory,
											MAKELPARAM(0, DBGetContactSettingByte(NULL, ModuleName, "SweepSHistory", 0)));

	for (hContact=db_find_first(); hContact;
											hContact=db_find_next(hContact))
	{
		hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem,
											MAKELPARAM(0, DBGetContactSettingByte(hContact, ModuleName, "SweepHistory", 0)));
	}
}//ShowAllContactIcons

void LoadSettings(HWND hwndDlg)
{
	int i; CLCINFOITEM cii = {0}; HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	cii.cbSize = sizeof(cii);
	cii.flags = CLCIIF_GROUPFONT;

	cii.pszText = TranslateT("*** Defaut Action ***");
	hAllContacts = (HANDLE)SendMessage(hwndList, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

	cii.pszText = TranslateT("*** System History ***");
	hSystemHistory = (HANDLE)SendMessage(hwndList, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

	ShowAllContactIcons(hwndList);

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_RESETCONTENT, 0, 0);

	for (i = 0; i < SIZEOF(time_stamp_strings); i++)
	{
		TCHAR* ptszTimeStr = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)time_stamp_strings[i]);
		SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
		mir_free(ptszTimeStr);
	}

	for (i = 0; i < SIZEOF(keep_strings); i++)
	{
		TCHAR* ptszTimeStr = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)keep_strings[i]);
		SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_ADDSTRING, 0, (LPARAM)ptszTimeStr);
		mir_free(ptszTimeStr);
	}

	SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_SETCURSEL, DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownOlder", 0), 0);
	SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_SETCURSEL, DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownKeep", 0), 0);

	CheckDlgButton(hwndDlg, IDC_UNSAFEMODE, DBGetContactSettingByte(NULL, ModuleName, "UnsafeMode", 0));
	CheckDlgButton(hwndDlg, IDC_SWEEPONCLOSE, DBGetContactSettingByte(NULL, ModuleName, "SweepOnClose", 0));
	CheckDlgButton(hwndDlg, IDC_HISTMW, DBGetContactSettingByte(NULL, ModuleName, "ChangeInMW", 0));
}//LoadSettings


void SaveSettings(HWND hwndDlg)
{
	int st, i; StatusIconData sid = {0}; HANDLE hContact, hItem; HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

	DBWriteContactSettingByte(NULL, ModuleName, "StartupShutdownOlder", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSOLDER, CB_GETCURSEL, 0, 0));
	DBWriteContactSettingByte(NULL, ModuleName, "StartupShutdownKeep", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SSKEEP, CB_GETCURSEL, 0, 0));
	DBWriteContactSettingByte(NULL, ModuleName, "UnsafeMode", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_UNSAFEMODE));
	DBWriteContactSettingByte(NULL, ModuleName, "SweepOnClose", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SWEEPONCLOSE));
	DBWriteContactSettingByte(NULL, ModuleName, "ChangeInMW", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HISTMW));

	sid.cbSize = sizeof(sid);
	sid.szModule = ModuleName;

	DBWriteContactSettingByte(NULL, ModuleName, "SweepHistory",
											(BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hAllContacts, 0));
	DBWriteContactSettingByte(NULL, ModuleName, "SweepSHistory",
											(BYTE)SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hSystemHistory, 0));

	for (hContact=db_find_first(); hContact;
											hContact=db_find_next(hContact))
	{
		hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);

		st = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, 0);
		if ( st != 0 )	DBWriteContactSettingByte(hContact, ModuleName, "SweepHistory", (BYTE)st);
		else			DBDeleteContactSetting(hContact, ModuleName, "SweepHistory");

		// set per-contact icons in status bar
		for(i = 0; i < 4; i++)
		{
			sid.dwId = i;
			sid.flags = (st == i) ? 0 : MBF_HIDDEN;
			CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
		}
	}

	// set tooltips
	st = DBGetContactSettingByte(NULL, ModuleName, "SweepHistory", 0);

	sid.dwId = 0;
	if		(st == 0)	sid.szTooltip = Translate("Keep all events");
	else if (st == 1)	sid.szTooltip = Translate(time_stamp_strings[DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownOlder", 0)]);
	else if (st == 2)	sid.szTooltip = Translate(keep_strings[DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownKeep", 0)]);
	else if (st == 3)	sid.szTooltip = Translate("Delete all events");
	CallService(MS_MSG_MODIFYICON, 0, (LPARAM)&sid);

	sid.dwId = 1;
	sid.szTooltip = Translate(time_stamp_strings[DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownOlder", 0)]);
	CallService(MS_MSG_MODIFYICON, 0, (LPARAM)&sid);

	sid.dwId = 2;
	sid.szTooltip = Translate(keep_strings[DBGetContactSettingByte(NULL, ModuleName, "StartupShutdownKeep", 0)]);
	CallService(MS_MSG_MODIFYICON, 0, (LPARAM)&sid);
}//SaveSettings

INT_PTR CALLBACK DlgProcHSOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			HIMAGELIST hIml = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
												ILC_MASK | (IsWinVerXPPlus() ? ILC_COLOR32 : ILC_COLOR16 ), 2, 2 );

			HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT);
			ImageList_AddIcon(hIml, hIcon);
			Skin_ReleaseIcon(hIcon);

			hIcon = LoadIconEx("act1");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT1, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("act2");
			ImageList_AddIcon(hIml, hIcon);
			SendDlgItemMessage(hwndDlg, IDC_ACT2, STM_SETICON, (WPARAM)hIcon, 0);

			hIcon = LoadIconEx("actDel");
			ImageList_AddIcon(hIml, hIcon);

			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRACOLUMNS, 1, 0);

			TranslateDialogDefault(hwndDlg);
			ResetListOptions(GetDlgItem(hwndDlg, IDC_LIST));
			LoadSettings(hwndDlg);
		}
		return TRUE;

	case WM_DESTROY:
		{
			HIMAGELIST hIml = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0);
			ImageList_Destroy(hIml);
			ReleaseIconEx("act1");
			ReleaseIconEx("act2");
			ReleaseIconEx("actDel");
		}
		break;

	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); // Send message to activate "Apply" button
		return TRUE;

	case WM_NOTIFY:
		{
			NMCLISTCONTROL* nmc = (NMCLISTCONTROL*)lParam;
			if ( nmc->hdr.idFrom == 0 && nmc->hdr.code == (unsigned)PSN_APPLY )
			{
				SaveSettings(hwndDlg);
			}
			else if (nmc->hdr.idFrom == IDC_LIST)
			{
				switch (nmc->hdr.code)
				{
				case CLN_NEWCONTACT:
				case CLN_LISTREBUILT:
					ShowAllContactIcons(nmc->hdr.hwndFrom);
					break;

				case CLN_OPTIONSCHANGED:
					ResetListOptions(nmc->hdr.hwndFrom);
					break;

				case NM_CLICK:
					{
						HANDLE hItem; DWORD hitFlags; int iImage;

						if ( nmc->iColumn == -1 )
							break;

						// Find clicked item
						hItem = (HANDLE)SendMessage(nmc->hdr.hwndFrom, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nmc->pt.x,nmc->pt.y));
						// Nothing was clicked
						if (hItem == NULL || !(IsHContactContact(hItem) || IsHContactInfo(hItem)))
							break;

						// It was not our extended icon
						if (!(hitFlags & CLCHT_ONITEMEXTRA))
							break;

						iImage = SendMessage(nmc->hdr.hwndFrom, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, 0));
						SendMessage(nmc->hdr.hwndFrom, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nmc->iColumn, (iImage + 1) % 4));

						// Activate Apply button
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;
				}
			}
		}
		break;
	}
	return FALSE;
}

int HSOptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HISTORYSWEEPER);
	odp.pszTitle = ModuleName;
	odp.pszGroup = LPGEN("History");
	odp.pfnDlgProc = DlgProcHSOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	return 0;
}
