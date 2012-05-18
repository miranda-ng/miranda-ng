/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* 
This file contain the source that is related to display contact
information, including the one shows in user detail and the brief
information
*/

#include "weather.h"

extern INT_PTR CALLBACK DlgProcINIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//============  CONTACT INFORMATION  ============

// initialize user info
// lParam = current contact
int UserInfoInit(WPARAM wParam, LPARAM lParam) 
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.position = 100000000;
	odp.pszTitle = LPGEN(WEATHERPROTONAME);

	if (lParam == 0) 
	{
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO);
		odp.pfnDlgProc = DlgProcINIPage;
		CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	}
	else
	{
		// check if it is a weather contact
		if(IsMyContact((HANDLE)lParam)) 
		{
			// register the contact info page
			odp.pszTemplate = MAKEINTRESOURCE(IDD_USERINFO);
			odp.pfnDlgProc = DlgProcUIPage;
			odp.flags = ODPF_BOLDGROUPS;
			CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
		}
	}

	return 0;
}

// dialog process for the weather tab under user info
// lParam = current contact
INT_PTR CALLBACK DlgProcUIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	WEATHERINFO w;
	char str[MAX_TEXT_SIZE];
	HANDLE hContact;

	hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) 
	{
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg,IDC_MOREDETAIL), BUTTONSETASFLATBTN, 0, 0);
		// save the contact handle for later use
		hContact = (HANDLE)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);
		// load weather info for the contact
		w = LoadWeatherInfo((HANDLE)lParam);
		SetDlgItemTextWth(hwndDlg, IDC_INFO1, GetDisplay(&w, Translate("Current condition for %n"), str));

		SendDlgItemMessage(hwndDlg, IDC_INFOICON, STM_SETICON, 
			(WPARAM)LoadSkinnedProtoIcon(WEATHERPROTONAME,
			DBGetContactSettingWord(hContact, WEATHERPROTONAME, "StatusIcon",0)), 0);

		{	// bold and enlarge the current condition
			LOGFONT lf;
			HFONT hNormalFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_INFO2,WM_GETFONT,0,0);
			GetObject(hNormalFont,sizeof(lf),&lf);
			lf.lfWeight=FW_BOLD;
			lf.lfWidth=7;
			lf.lfHeight=15;
			SendDlgItemMessage(hwndDlg, IDC_INFO2, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);
		}
		// set the text for displaying other current weather conditions data
		GetDisplay(&w, "%c     %t", str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO2, str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO3, w.feel);
		SetDlgItemTextWth(hwndDlg, IDC_INFO4, w.pressure);
		GetDisplay(&w, "%i  %w", str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO5, str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO6, w.dewpoint);
		SetDlgItemTextWth(hwndDlg, IDC_INFO7, w.sunrise);
		SetDlgItemTextWth(hwndDlg, IDC_INFO8, w.sunset);
		SetDlgItemTextWth(hwndDlg, IDC_INFO9, w.high);
		SetDlgItemTextWth(hwndDlg, IDC_INFO10, w.low);
		GetDisplay(&w, Translate("Last update on:   %u"), str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO11, str);
		SetDlgItemTextWth(hwndDlg, IDC_INFO12, w.humid);
		SetDlgItemTextWth(hwndDlg, IDC_INFO13, w.vis);
		break;

	case WM_DESTROY: 
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)SendDlgItemMessage(hwndDlg, IDC_INFOICON, STM_SETICON, 0, 0), 0);
		DeleteObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_INFO2, WM_GETFONT, 0, 0));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDC_MOREDETAIL: 
			{
				HWND hMoreDataDlg;

				hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
				if (hMoreDataDlg == NULL)
					hMoreDataDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BRIEF), NULL, 
					DlgProcMoreData, (LPARAM)hContact);
				else 
				{
					SetForegroundWindow(hMoreDataDlg);
					SetFocus(hMoreDataDlg);
				}
				ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 0);
				ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 1);
				break;
			}
		}
		break;
	}
	return 0;
}

//============  BRIEF INFORMATION  ============

static int BriefDlgResizer(HWND hwnd, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	switch(urc->wId) 
	{
	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;

	case IDC_MTEXT:
	case IDC_DATALIST:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_MUPDATE:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_MTOGGLE:
	case IDC_MWEBPAGE:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}



// dialog process for more data in the user info window
// lParam = contact handle
INT_PTR CALLBACK DlgProcMoreData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	static const unsigned tabstops = 48;
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) 
	{
	case WM_INITDIALOG:
		// save the contact handle for later use
		hContact = (HANDLE)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_AUTOURLDETECT, (WPARAM) TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETEVENTMASK, 0, ENM_LINK);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETMARGINS, EC_LEFTMARGIN, 5);
		SendDlgItemMessage(hwndDlg, IDC_MTEXT, EM_SETTABSTOPS, 1, (LPARAM)&tabstops);

		// get the list to display
		{
			LV_COLUMN lvc = { 0 };
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			RECT aRect = { 0 };
			GetClientRect(hList, &aRect);

			// managing styles
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			ListView_SetExtendedListViewStyleEx(hList,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP,
				LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

			// inserting columns
			lvc.cx = LIST_COLUMN;
			lvc.pszText = Translate("Variable");
			ListView_InsertColumnWth(hList, 0, &lvc);

			lvc.cx = aRect.right - LIST_COLUMN - GetSystemMetrics(SM_CXVSCROLL) - 3;
			lvc.pszText = Translate("Information");
			ListView_InsertColumnWth(hList, 1, &lvc);

			// inserting data
			SendMessage(hwndDlg, WM_UPDATEDATA, 0, 0);
		}
		TranslateDialogDefault(hwndDlg);

		// prevent dups of the window
		WindowList_Add(hDataWindowList, hwndDlg, hContact);

		// restore window position
		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, WEATHERPROTONAME, "BriefInfo_");
		return TRUE;

	case WM_UPDATEDATA:
		ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_DATALIST));
		LoadBriefInfoText(hwndDlg, hContact);
		DBDataManage(hContact, WDBM_DETAILDISPLAY, (WPARAM)hwndDlg, 0);

		// set icons
		{
			WORD statusIcon = DBGetContactSettingWord(hContact, WEATHERPROTONAME, "StatusIcon", 0);

			ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIconBig(WEATHERPROTONAME, statusIcon)));
			ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedProtoIcon(WEATHERPROTONAME, statusIcon)));
		}
		RedrawWindow(GetDlgItem(hwndDlg, IDC_HEADERBAR), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case WM_SIZE: 
		{
			RECT rc;
			HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);
			GetWindowRect(hList, &rc);
			ListView_SetColumnWidth(hList, 1, ListView_GetColumnWidth(hList, 1) + 
				(int)LOWORD(lParam) - (rc.right - rc.left));
		}
		{
			UTILRESIZEDIALOG urd = {0};
			urd.cbSize     = sizeof(urd);
			urd.hwndDlg    = hwndDlg;
			urd.hInstance  = hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_BRIEF);
			urd.pfnResizer = BriefDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}				
		break;

	case WM_GETMINMAXINFO: 
		{
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;

			// The minimum width in points
			mmi->ptMinTrackSize.x = 350;
			// The minimum height in points
			mmi->ptMinTrackSize.y = 300;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDCANCEL:
			// close the info window
			DestroyWindow(hwndDlg);
			break;

		case IDC_MUPDATE: 
			{
				LV_ITEM lvi = {0};
				HWND hList = GetDlgItem(hwndDlg, IDC_DATALIST);

				// update current data
				// set the text to "updating"
				SetDlgItemTextWth(hwndDlg, IDC_MTEXT, Translate("Retrieving new data, please wait..."));
				ListView_DeleteAllItems(hList);
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = 1;
				lvi.pszText = (LPSTR)"";
				lvi.iItem = ListView_InsertItemWth(hList, &lvi);
				lvi.pszText = Translate("Retrieving new data, please wait...");
				ListView_SetItemTextWth(hList, lvi.iItem, 1, lvi.pszText);
				UpdateSingleStation((WPARAM)hContact, 0);
				break;
			}

		case IDC_MWEBPAGE:
			LoadForecast((WPARAM)hContact, 0);	// read complete forecast
			break;

		case IDC_MTOGGLE:
			if (IsWindowVisible(GetDlgItem(hwndDlg,IDC_DATALIST)))
				SetDlgItemTextWth(hwndDlg, IDC_MTOGGLE, Translate("More Info"));
			else
				SetDlgItemTextWth(hwndDlg, IDC_MTOGGLE, Translate("Brief Info"));
			ShowWindow(GetDlgItem(hwndDlg,IDC_DATALIST), (int)!IsWindowVisible(
				GetDlgItem(hwndDlg,IDC_DATALIST)));
			ShowWindow(GetDlgItem(hwndDlg,IDC_MTEXT), (int)!IsWindowVisible(GetDlgItem(hwndDlg,IDC_MTEXT)));
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			if (pNmhdr->idFrom == IDC_MTEXT && pNmhdr->code == EN_LINK) 
			{
				ENLINK *enlink = (ENLINK *) lParam;
				TEXTRANGE tr;
				switch (enlink->msg) 
				{
				case WM_LBUTTONUP:
					tr.chrg = enlink->chrg;
					tr.lpstrText = mir_alloc(tr.chrg.cpMax - tr.chrg.cpMin + 8);
					SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					CallService(MS_UTILS_OPENURL, 1, (LPARAM) tr.lpstrText);
					mir_free(tr.lpstrText);
					break;
				}
			}
			break;
		}

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY: 
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));

		Utils_SaveWindowPosition(hwndDlg, NULL, WEATHERPROTONAME, "BriefInfo_");
		WindowList_Remove(hDataWindowList, hwndDlg);
		break;
	}

	return FALSE;
}

// set the title of the dialog and on the which rectangle
// also load brief info into message box
void LoadBriefInfoText(HWND hwndDlg, HANDLE hContact) 
{
	WEATHERINFO winfo;
	char str[4096], str2[4096];

	// load weather information from the contact into the WEATHERINFO struct
	winfo = LoadWeatherInfo(hContact);
	// check if data exist.  If not, display error message box
	if (!(BOOL)DBGetContactSettingByte(hContact, WEATHERPROTONAME, "IsUpdated", FALSE))
	{
		strcpy(str, Translate("No information available.\r\nPlease update weather condition first."));
	}
	else
		// set the display text and show the message box
		GetDisplay(&winfo, opt.bText, str);

	if (lpcp != CP_ACP)
	{
		SETTEXTEX textex; 
		textex.flags = ST_DEFAULT;
		textex.codepage = lpcp;

		SendMessage(GetDlgItem(hwndDlg, IDC_MTEXT), EM_SETTEXTEX, (WPARAM)&textex, (LPARAM)str);
	}
	else
		SetDlgItemText(hwndDlg, IDC_MTEXT, str);

	GetDisplay(&winfo, opt.bTitle, str);
	SetWindowTextWth(hwndDlg, str);
	GetDisplay(&winfo, "%c, %t", str);
	mir_snprintf(str2, SIZEOF(str2), "%s\n%s", winfo.city, str);
	SetDlgItemTextWth(hwndDlg, IDC_HEADERBAR, str2);
}

// show brief information dialog
// wParam = current contact
int BriefInfo(WPARAM wParam, LPARAM lParam) 
{
	// make sure that the contact is actually a weather one
	if(IsMyContact((HANDLE)wParam)) 
	{
		HWND hMoreDataDlg = WindowList_Find(hDataWindowList,(HANDLE)wParam);
		if (hMoreDataDlg != NULL) 
		{
			SetForegroundWindow(hMoreDataDlg);
			SetFocus(hMoreDataDlg);
		}
		else
		{
			hMoreDataDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BRIEF), NULL, DlgProcMoreData, 
				(LPARAM)wParam);
		}
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_DATALIST), 0);
		ShowWindow(GetDlgItem(hMoreDataDlg, IDC_MTEXT), 1);
		SetDlgItemTextWth(hMoreDataDlg, IDC_MTOGGLE, Translate("More Info"));
		return 1;
	}
	return 0;
}

INT_PTR BriefInfoSvc(WPARAM wParam, LPARAM lParam) 
{
	return BriefInfo(wParam, lParam);
}
