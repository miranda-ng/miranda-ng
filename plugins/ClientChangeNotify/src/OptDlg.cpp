/*
	ClientChangeNotify - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

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

#include "Common.h"

// ================================================ Popup options ================================================

COptPage g_PopupOptPage(MOD_NAME, NULL);


void EnablePopupOptDlgControls()
{
	int I;
	g_PopupOptPage.PageToMem();
	int UsePopups = g_PopupOptPage.GetValue(IDC_POPUPOPTDLG_POPUPNOTIFY);
	for (I = 0; I < g_PopupOptPage.Items.GetSize(); I++)
	{
		switch (g_PopupOptPage.Items[I]->GetParam())
		{
			case IDC_POPUPOPTDLG_POPUPNOTIFY:
			{
				g_PopupOptPage.Items[I]->Enable(UsePopups);
			} break;
			case IDC_POPUPOPTDLG_DEFBGCOLOUR:
			{
				g_PopupOptPage.Items[I]->Enable(UsePopups && !g_PopupOptPage.GetValue(IDC_POPUPOPTDLG_DEFBGCOLOUR));
			} break;
			case IDC_POPUPOPTDLG_DEFTEXTCOLOUR:
			{
				g_PopupOptPage.Items[I]->Enable(UsePopups && !g_PopupOptPage.GetValue(IDC_POPUPOPTDLG_DEFTEXTCOLOUR));
			} break;
		}
	}
	if (g_PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY))
	{
		COptItem *ShowVer = g_PopupOptPage.Find(IDC_POPUPOPTDLG_SHOWVER);
		ShowVer->SetValue(1);
		ShowVer->Enable(false);
		ShowVer->MemToWnd(g_PopupOptPage.hWnd);
	}
	if (!bFingerprintExists)
	{ // disable these checkboxes if Fingerprint wasn't found
		g_PopupOptPage.Find(IDC_POPUPOPTDLG_VERCHGNOTIFY)->Enable(false);
		g_PopupOptPage.Find(IDC_POPUPOPTDLG_SHOWVER)->Enable(false);
	}
	if (PcreEnabled())
	{
		SetDlgItemText(g_PopupOptPage.GetWnd(), IDC_POPUPOPTDLG_STATIC_REGEXP, TranslateT("(you can use regular expressions here)"));
	}
	g_PopupOptPage.MemToPage(true);
	InvalidateRect(GetDlgItem(g_PopupOptPage.GetWnd(), IDC_POPUPOPTDLG_POPUPDELAY_SPIN), NULL, false); // update spin control
}

static struct {
	TCHAR *Text;
	int Action;
}
PopupActions[] = 
{
	LPGENT("Open message window"), PCA_OPENMESSAGEWND,
	LPGENT("Close popup"), PCA_CLOSEPOPUP,
	LPGENT("Open contact details window"), PCA_OPENDETAILS,
	LPGENT("Open contact menu"), PCA_OPENMENU,
	LPGENT("Open contact history"), PCA_OPENHISTORY,
	LPGENT("Do nothing"), PCA_DONOTHING
};

INT_PTR CALLBACK PopupOptDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int ChangeLock = 0;
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			ChangeLock++;
			g_PopupOptPage.SetWnd(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_POPUPOPTDLG_POPUPDELAY, EM_LIMITTEXT, 4, 0);
			SendDlgItemMessage(hwndDlg, IDC_POPUPOPTDLG_IGNORESTRINGS, EM_LIMITTEXT, IGNORESTRINGS_MAX_LEN, 0);
			SendDlgItemMessage(hwndDlg, IDC_POPUPOPTDLG_POPUPDELAY_SPIN, UDM_SETRANGE32, -1, 9999);

			HWND hLCombo = GetDlgItem(hwndDlg, IDC_POPUPOPTDLG_LCLICK_ACTION);
			HWND hRCombo = GetDlgItem(hwndDlg, IDC_POPUPOPTDLG_RCLICK_ACTION);
			int I;
			for (I = 0; I < lengthof(PopupActions); I++) {
				SendMessage(hLCombo, CB_SETITEMDATA, SendMessage(hLCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(PopupActions[I].Text)), PopupActions[I].Action);
				SendMessage(hRCombo, CB_SETITEMDATA, SendMessage(hRCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(PopupActions[I].Text)), PopupActions[I].Action);
			}
			g_PopupOptPage.DBToMemToPage();
			EnablePopupOptDlgControls();
			ChangeLock--;
		}
		return true;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code) {
		case PSN_APPLY: 
			g_PopupOptPage.PageToMemToDB();
			RecompileRegexps(*(TCString*)g_PopupOptPage.GetValue(IDC_POPUPOPTDLG_IGNORESTRINGS));
			return true;
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_POPUPOPTDLG_POPUPNOTIFY:
			case IDC_POPUPOPTDLG_DEFBGCOLOUR:
			case IDC_POPUPOPTDLG_DEFTEXTCOLOUR:
			case IDC_POPUPOPTDLG_VERCHGNOTIFY:
				EnablePopupOptDlgControls();
				// go through

			case IDC_POPUPOPTDLG_SHOWPREVCLIENT:
			case IDC_POPUPOPTDLG_SHOWVER:
			case IDC_POPUPOPTDLG_USESTATUSNOTIFYFLAG:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				return 0;
				
			case IDC_POPUPOPTDLG_POPUPPREVIEW:
				g_PreviewOptPage = new COptPage(g_PopupOptPage);
				g_PreviewOptPage->PageToMem();
				DBCONTACTWRITESETTING cws = {0};
				cws.szModule = "ICQ";
				cws.szSetting = DB_MIRVER;
				db_set_s(NULL, MOD_NAME, DB_OLDMIRVER, "ICQ Lite v5");
				ContactSettingChanged(NULL, (LPARAM)&cws); // simulate a version change
				delete g_PreviewOptPage;
				g_PreviewOptPage = NULL;
				break;
			}
			break;
		
		case EN_CHANGE:
			if (LOWORD(wParam) == IDC_POPUPOPTDLG_POPUPDELAY || LOWORD(wParam) == IDC_POPUPOPTDLG_IGNORESTRINGS)
				if (!ChangeLock && g_PopupOptPage.GetWnd())
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);

			break;

		case CBN_SELCHANGE:
			if ((LOWORD(wParam) == IDC_POPUPOPTDLG_LCLICK_ACTION) || (LOWORD(wParam) == IDC_POPUPOPTDLG_RCLICK_ACTION) || (LOWORD(wParam) == IDC_POPUPOPTDLG_BGCOLOUR) || (LOWORD(wParam) == IDC_POPUPOPTDLG_TEXTCOLOUR))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);

			break;
		}
		break;
		
	case WM_DESTROY:
		g_PopupOptPage.SetWnd(NULL);
		return 0;
	}
	return 0;
}

int OptionsDlgInit(WPARAM wParam, LPARAM lParam)
{
	if (bPopupExists) {
		OPTIONSDIALOGPAGE optDi = { sizeof(optDi) };
		optDi.position = 920000000;
		optDi.ptszTitle = LPGENT("ClientChangeNotify");
		optDi.pfnDlgProc = PopupOptDlg;
		optDi.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPOPTDLG);
		optDi.hInstance = g_hInstance;
		optDi.ptszGroup = LPGENT("Popups");
		optDi.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		Options_AddPage(wParam, &optDi);
	}
	return 0;
}

void InitOptions()
{
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_POPUPNOTIFY, "PopupNotify", DBVT_BYTE, 1));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_SHOWPREVCLIENT, "ShowPrevClient", DBVT_BYTE, 0, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_VERCHGNOTIFY, "VerChgNotify", DBVT_BYTE, 0, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_SHOWVER, "ShowVer", DBVT_BYTE, 1, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_USESTATUSNOTIFYFLAG, "UseStatusNotifyFlag", DBVT_BYTE, 1, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_POPUPPREVIEW, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Combobox(IDC_POPUPOPTDLG_LCLICK_ACTION, "PopupLClickAction", DBVT_BYTE, POPUP_DEF_LCLICKACTION, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Combobox(IDC_POPUPOPTDLG_RCLICK_ACTION, "PopupRClickAction", DBVT_BYTE, POPUP_DEF_RCLICKACTION, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_LCLICK, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_RCLICK, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Colourpicker(IDC_POPUPOPTDLG_BGCOLOUR, "PopupBGColour", POPUP_DEF_POPUP_BGCOLOUR, IDC_POPUPOPTDLG_DEFBGCOLOUR));
	g_PopupOptPage.Items.AddElem(new COptItem_Colourpicker(IDC_POPUPOPTDLG_TEXTCOLOUR, "PopupTextColour", POPUP_DEF_POPUP_TEXTCOLOUR, IDC_POPUPOPTDLG_DEFTEXTCOLOUR));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_DEFBGCOLOUR, "UseDefBGColour", DBVT_BYTE, POPUP_DEF_USEDEFBGCOLOUR, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Checkbox(IDC_POPUPOPTDLG_DEFTEXTCOLOUR, "UseDefTextColour", DBVT_BYTE, POPUP_DEF_USEDEFTEXTCOLOUR, 0, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_BGCOLOUR, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_TEXTCOLOUR, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_IntEdit(IDC_POPUPOPTDLG_POPUPDELAY, "PopupDelay", DBVT_WORD, true, POPUP_DEF_POPUPDELAY, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_SEC, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_DEFAULT, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_INFINITE, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_IGNORESTRINGS, IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Edit(IDC_POPUPOPTDLG_IGNORESTRINGS, DB_IGNORESUBSTRINGS, IGNORESTRINGS_MAX_LEN, _T("gmail;skype;/Miranda[0-9A-F]{8}/"), IDC_POPUPOPTDLG_POPUPNOTIFY));
	g_PopupOptPage.Items.AddElem(new COptItem_Generic(IDC_POPUPOPTDLG_STATIC_REGEXP, IDC_POPUPOPTDLG_POPUPNOTIFY));
}
