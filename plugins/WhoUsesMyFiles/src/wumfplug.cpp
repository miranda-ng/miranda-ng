#include "wumf.h"

HINSTANCE hInst;
WUMF_OPTIONS WumfOptions = { 0 };
HGENMENU hMenuItem = 0;
int hLangpack;
HWND hDlg;

static PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX), 
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {80DCA515-973A-4A7E-8B85-5D8EC88FC5A7}
	{0x80dca515, 0x973a, 0x4a7e, {0x8b, 0x85, 0x5d, 0x8e, 0xc8, 0x8f, 0xc5, 0xa7}}
};

void LoadOptions()
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_TCHAR;
	memset(&WumfOptions, 0, sizeof(WumfOptions));
	if (db_get_ts(NULL, MODULENAME, OPT_FILE, &dbv) == 0)
	{
		_tcsncpy(WumfOptions.LogFile, dbv.ptszVal, 255);
		db_free(&dbv);
	}
	else
		WumfOptions.LogFile[0] = '\0';

	WumfOptions.PopupsEnabled = db_get_b(NULL, MODULENAME, POPUPS_ENABLED, TRUE);

	WumfOptions.UseDefColor = db_get_b(NULL, MODULENAME, COLOR_DEF, TRUE);
	WumfOptions.UseWinColor = db_get_b(NULL, MODULENAME, COLOR_WIN, FALSE);
	WumfOptions.SelectColor = db_get_b(NULL, MODULENAME, COLOR_SET, FALSE);

	WumfOptions.ColorText = db_get_dw(NULL, MODULENAME, COLOR_TEXT, RGB(0,0,0));
	WumfOptions.ColorBack = db_get_dw(NULL, MODULENAME, COLOR_BACK, RGB(255,255,255));

	WumfOptions.DelayDef = db_get_b(NULL, MODULENAME, DELAY_DEF, TRUE);
	WumfOptions.DelayInf = db_get_b(NULL, MODULENAME, DELAY_INF, FALSE);
	WumfOptions.DelaySet = db_get_b(NULL, MODULENAME, DELAY_SET, FALSE);
	WumfOptions.DelaySec = db_get_b(NULL, MODULENAME, DELAY_SEC, 0);
	if (!ServiceExists(MS_POPUP_ADDPOPUPT)) {
		WumfOptions.DelayDef = TRUE;
		WumfOptions.DelaySet = FALSE;
		WumfOptions.DelayInf = FALSE;
	}
	WumfOptions.LogToFile = db_get_b(NULL, MODULENAME, LOG_INTO_FILE, FALSE);
	WumfOptions.LogFolders = db_get_b(NULL, MODULENAME, LOG_FOLDER, TRUE);
	WumfOptions.AlertFolders = db_get_b(NULL, MODULENAME, ALERT_FOLDER, TRUE);
	WumfOptions.LogUNC = db_get_b(NULL, MODULENAME, LOG_UNC, FALSE);
	WumfOptions.AlertUNC = db_get_b(NULL, MODULENAME, ALERT_UNC, FALSE);
	WumfOptions.LogComp = db_get_b(NULL, MODULENAME, LOG_COMP, FALSE);
	WumfOptions.AlertComp = db_get_b(NULL, MODULENAME, ALERT_COMP, FALSE);
}

void ExecuteMenu(HWND hWnd)
{
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) {
        msg(TranslateT("Error creating menu"));
        return;
    }
    AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About\0"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);//------------------
    AppendMenu(hMenu, MF_STRING, IDM_SHOW, _T("Show connections\0"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);//------------------
    AppendMenu(hMenu, MF_STRING, IDM_EXIT, _T("Dismiss popup\0"));

    POINT point;
    GetCursorPos(&point);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_TOPALIGN, point.x, point.y, 0, hWnd, NULL);
    PostMessage(hWnd, WM_USER, 0, 0);
    DestroyMenu(hMenu);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_ABOUT:
			break;
		case IDM_EXIT:
			PUDeletePopup(hWnd);
			break;	
		case IDM_SHOW:
			CallService(MS_WUMF_CONNECTIONSSHOW, 0, 0);
			return TRUE;
		}
		
		switch (HIWORD(wParam)) {
		case STN_CLICKED:
			PUDeletePopup(hWnd);
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU: 
		CallService(MS_WUMF_CONNECTIONSSHOW, 0, 0);
		break;		

	case UM_FREEPLUGINDATA:
		return TRUE; //TRUE or FALSE is the same, it gets ignored.
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowWumfPopup(PWumf w)
{
	TCHAR text[512], title[512];

	if (!WumfOptions.AlertFolders && (w->dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return;
	mir_sntprintf(title, SIZEOF(title), _T("%s (%s)"), w->szComp, w->szUser);
	mir_sntprintf(text, SIZEOF(text), _T("%s (%s)"), w->szPath, w->szPerm);
	ShowThePopup(w, title, text);
}

void ShowThePopup(PWumf w, LPTSTR title, LPTSTR text)
{
	POPUPDATAT ppd = {0};
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DRIVE));

	if (WumfOptions.DelayInf)
		ppd.iSeconds = -1; 
	else if (WumfOptions.DelayDef)
		ppd.iSeconds = 0;
	else if (WumfOptions.DelaySet)
		ppd.iSeconds = WumfOptions.DelaySec;

	mir_tstrncpy(ppd.lptzContactName, title, MAX_CONTACTNAME);
	mir_tstrncpy(ppd.lptzText, text, MAX_SECONDLINE);
	if (WumfOptions.UseWinColor) {
		ppd.colorBack = GetSysColor(COLOR_WINDOW);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if (WumfOptions.SelectColor) {
		ppd.colorBack = WumfOptions.ColorBack;
		ppd.colorText = WumfOptions.ColorText;
	}

	ppd.PluginWindowProc = PopupDlgProc;
	ppd.PluginData = w;
	PUAddPopupT(&ppd);
}

void ShowThePreview()
{
	if ( !ServiceExists(MS_POPUP_ADDPOPUPT)) {
		MessageBox(NULL, TranslateT("Popup plugin not found!"), TranslateT("Error"), MB_OK|MB_ICONSTOP);
		return;
	}

	if (WumfOptions.AlertFolders) {
		ShowThePopup(NULL, _T("Guest"), _T("C:\\My Share"));
		Sleep(300);
		ShowThePopup(NULL, _T("Guest"), _T("C:\\My Share\\Photos"));
		Sleep(300);
	}
	ShowThePopup(NULL, _T("Guest"), _T("C:\\Share\\My Photos\\photo.jpg"));
	Sleep(300);
	if (WumfOptions.AlertFolders) {
		ShowThePopup(NULL, _T("User"), _T("C:\\My Share"));
		Sleep(300);
		ShowThePopup(NULL, _T("User"), _T("C:\\My Share\\Movies"));
		Sleep(300);
	}
	ShowThePopup(NULL, _T("User"), _T("C:\\My Share\\Movies\\The Two Towers.avi"));
	Sleep(300);
	if (WumfOptions.AlertFolders) {
		ShowThePopup(NULL, _T("Administrator"), _T("C:\\Distributives"));
		Sleep(300);
		ShowThePopup(NULL, _T("Administrator"), _T("C:\\Distributives\\Win2k"));
		Sleep(300);
	}
	ShowThePopup(NULL, _T("Administrator"), _T("C:\\Distributives\\Win2k\\setup.exe"));
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

void ThreadProc(LPVOID)
{
	if (hDlg) {
		ShowWindow(hDlg, SW_SHOWNORMAL); 
		SetForegroundWindow(hDlg);
		return;
	}

	hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONNLIST), NULL, ConnDlgProc);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_DRIVE)));
	ShowWindow(hDlg, SW_SHOW); 

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) == TRUE) { 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	hDlg = NULL;
}

static INT_PTR WumfShowConnections(WPARAM,LPARAM)
{
	mir_forkthread(ThreadProc, NULL);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hWumfBut, 0);
	return 0;
}

static INT_PTR WumfMenuCommand(WPARAM,LPARAM)
{
	BOOL MajorTo0121 = FALSE;
	int iResult = 0;

	CLISTMENUITEM mi = { sizeof(mi) };
	if (WumfOptions.PopupsEnabled == TRUE) { 
		WumfOptions.PopupsEnabled = FALSE;
		mi.pszName = LPGEN("Enable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else {
		WumfOptions.PopupsEnabled = TRUE;
		mi.pszName = LPGEN("Disable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
	}
	db_set_b(NULL, MODULENAME, POPUPS_ENABLED, (BYTE)WumfOptions.PopupsEnabled);
	mi.flags = CMIM_NAME | CMIM_ICON;
	Menu_ModifyItem(hMenuItem, &mi);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

void DisableDelayOptions(HWND hwndDlg)
{
	CheckDlgButton(hwndDlg, IDC_DELAY_INF,BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DELAY_SET,BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DELAY_DEF,BST_CHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_INF), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SET), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_DEF), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TX_DELAY_SEC), FALSE);
}

void ChooseFile(HWND hDlg)
{
	TCHAR szFile[MAX_PATH]; szFile[0]=0;

	// Initialize OPENFILENAME
	OPENFILENAME ofn = {0};       // common dialog box structure
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = SIZEOF(szFile);
	ofn.lpstrFilter = _T("All files (*.*)\0*.*\0Text files (*.txt)\0*.txt\0Log files (*.log)\0*.log\0\0");
	ofn.nFilterIndex = 2;
	ofn.Flags = OFN_CREATEPROMPT;
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)) {
		HANDLE hf = CreateFile(szFile,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			SetDlgItemText(hDlg,IDC_FILE,szFile);
			mir_tstrncpy(WumfOptions.LogFile, szFile, MAX_PATH);
			CloseHandle(hf);
		}
	}
	else if (CommDlgExtendedError() != 0) {
		TCHAR str[256];
		mir_sntprintf(str, SIZEOF(str), TranslateT("Common Dialog Error 0x%lx"), CommDlgExtendedError());
		MessageBox(hDlg, str, TranslateT("Error"), MB_OK | MB_ICONSTOP);
	}
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WORD wControlId = LOWORD(wparam);
	WORD wNotifyCode = HIWORD(wparam);
	int seconds;

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_COLOR_WIN, WumfOptions.UseWinColor ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_COLOR_DEF, WumfOptions.UseDefColor ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_COLOR_SET, WumfOptions.SelectColor ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BACK), WumfOptions.SelectColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TEXT), WumfOptions.SelectColor);
		if (WumfOptions.SelectColor) {
			SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_SETCOLOUR,0,WumfOptions.ColorBack);
			SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_SETCOLOUR,0,WumfOptions.ColorText);
		}
		if ( !ServiceExists(MS_POPUP_ADDPOPUPT)) {
			DisableDelayOptions(hwndDlg);
			break;
		}
		CheckDlgButton(hwndDlg, IDC_DELAY_INF, WumfOptions.DelayInf ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_DEF, WumfOptions.DelayDef ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DELAY_SET, WumfOptions.DelaySet ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
		SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);
		//Logging & alerts
		CheckDlgButton(hwndDlg, IDC_LOG_FOLDER, WumfOptions.LogFolders ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALERT_FOLDER, WumfOptions.AlertFolders ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_UNC, WumfOptions.LogUNC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALERT_UNC, WumfOptions.AlertUNC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_COMP, WumfOptions.LogComp ? BST_CHECKED : BST_UNCHECKED);

		if (WumfOptions.LogToFile) {
			CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), TRUE);
			SetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile);
		}
		else {
			CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), FALSE);
			SetDlgItemText(hwndDlg, IDC_FILE, _T(""));
		}
		break;

	case WM_COMMAND:
		switch(wNotifyCode) {
		case BN_CLICKED :
			switch(wControlId) {
			case IDC_DELAY_SET:
			case IDC_DELAY_DEF:
			case IDC_DELAY_INF:
				WumfOptions.DelaySet = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_SET) == BST_CHECKED);	
				WumfOptions.DelayDef = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_DEF) == BST_CHECKED);	
				WumfOptions.DelayInf = (IsDlgButtonChecked(hwndDlg, IDC_DELAY_INF) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
				SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, TRUE);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_COLOR_SET:
			case IDC_COLOR_DEF:
			case IDC_COLOR_WIN:
				WumfOptions.SelectColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_SET) == BST_CHECKED);	
				WumfOptions.UseDefColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_DEF) == BST_CHECKED);	
				WumfOptions.UseWinColor = (IsDlgButtonChecked(hwndDlg, IDC_COLOR_WIN) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BACK),WumfOptions.SelectColor);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TEXT), WumfOptions.SelectColor);
				SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_SETCOLOUR,0,WumfOptions.ColorBack);
				SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_SETCOLOUR,0,WumfOptions.ColorText);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
				/* not implemented */
			case IDC_LOG_COMP:
			case IDC_ALERT_COMP:
			case IDC_LOG_UNC:
			case IDC_ALERT_UNC:
				MessageBox(NULL, TranslateT("Not implemented yet..."), _T("WUMF"), MB_OK | MB_ICONINFORMATION);
				break;
				/* end */
			case IDC_LOG_INTO_FILE:
				WumfOptions.LogToFile = (IsDlgButtonChecked(hwndDlg, IDC_LOG_INTO_FILE) == BST_CHECKED);	
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), WumfOptions.LogToFile);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), WumfOptions.LogToFile);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_SEL_FILE:
				ChooseFile(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_LOG_FOLDER:
				WumfOptions.LogFolders = (IsDlgButtonChecked(hwndDlg, IDC_LOG_FOLDER) == BST_CHECKED);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_ALERT_FOLDER:
				WumfOptions.AlertFolders = (IsDlgButtonChecked(hwndDlg, IDC_ALERT_FOLDER) == BST_CHECKED);
				break;
			case IDC_PREVIEW:
				ShowThePreview();
				break;
			case IDC_CONN:
				CallService(MS_WUMF_CONNECTIONSSHOW, 0, 0);
				break;
			}
			break;

		case CPN_COLOURCHANGED:						
			WumfOptions.ColorText = SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_GETCOLOUR,0,0);
			WumfOptions.ColorBack = SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_GETCOLOUR,0,0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case EN_CHANGE:
			switch(wControlId) {
			case IDC_DELAY_SEC:
				seconds = GetDlgItemInt(hwndDlg, IDC_DELAY_SEC, NULL, FALSE);
				if (seconds > LIFETIME_MAX)
					WumfOptions.DelaySec = LIFETIME_MAX;
				else if (seconds < LIFETIME_MIN)
					WumfOptions.DelaySec = LIFETIME_MIN;
				else if (seconds <= LIFETIME_MAX || seconds >= LIFETIME_MIN)
					WumfOptions.DelaySec = seconds;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_FILE:
				GetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile, SIZEOF(WumfOptions.LogFile));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		case EN_KILLFOCUS:
			switch(wControlId) {
			case IDC_DELAY_SEC:
				SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);								
				break;
			}
			break;
		}
		break;				

	case WM_NOTIFY: 
		switch(((LPNMHDR)lparam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lparam)->code) {
			case PSN_RESET:
				LoadOptions();
				return TRUE;

			case PSN_APPLY:
				db_set_dw(NULL, MODULENAME, COLOR_TEXT, (DWORD)WumfOptions.ColorText);
				db_set_dw(NULL, MODULENAME, COLOR_BACK, (DWORD)WumfOptions.ColorBack);
				db_set_b(NULL, MODULENAME, COLOR_DEF, (BYTE)WumfOptions.UseDefColor);
				db_set_b(NULL, MODULENAME, COLOR_WIN, (BYTE)WumfOptions.UseWinColor);
				db_set_b(NULL, MODULENAME, COLOR_SET, (BYTE)WumfOptions.SelectColor );
				db_set_b(NULL, MODULENAME, DELAY_DEF, (BYTE)WumfOptions.DelayDef);
				db_set_b(NULL, MODULENAME, DELAY_INF, (BYTE)WumfOptions.DelayInf);
				db_set_b(NULL, MODULENAME, DELAY_SET, (BYTE)WumfOptions.DelaySet);
				db_set_b(NULL, MODULENAME, DELAY_SEC, (BYTE)WumfOptions.DelaySec);
				db_set_b(NULL, MODULENAME, LOG_INTO_FILE, (BYTE)WumfOptions.LogToFile);
				db_set_b(NULL, MODULENAME, LOG_FOLDER, (BYTE)WumfOptions.LogFolders);
				db_set_b(NULL, MODULENAME, ALERT_FOLDER, (BYTE)WumfOptions.AlertFolders);
				db_set_b(NULL, MODULENAME, LOG_UNC, (BYTE)WumfOptions.LogUNC);
				db_set_b(NULL, MODULENAME, ALERT_UNC, (BYTE)WumfOptions.AlertUNC);
				db_set_b(NULL, MODULENAME, LOG_COMP, (BYTE)WumfOptions.LogComp);
				db_set_b(NULL, MODULENAME, ALERT_COMP, (BYTE)WumfOptions.AlertComp);
				GetDlgItemText(hwndDlg, IDC_FILE, WumfOptions.LogFile, SIZEOF(WumfOptions.LogFile));
				db_set_ts(NULL, MODULENAME, OPT_FILE, WumfOptions.LogFile);
			}
		}
		break;
	}
	return 0;
}

int InitTopToolbar(WPARAM,LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconUp = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DRIVE));
	ttb.pszService = MS_WUMF_CONNECTIONSSHOW;
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
	ttb.name = ttb.pszTooltipUp = LPGEN("Show connections list");
	hWumfBut = TopToolbar_AddButton(&ttb);
	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 945000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = LPGEN("Who uses my files");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszGroup = LPGEN("Services");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wparam, &odp);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	LoadOptions();

	CreateServiceFunction(MS_WUMF_SWITCHPOPUP, WumfMenuCommand);
	CreateServiceFunction(MS_WUMF_CONNECTIONSSHOW, WumfShowConnections);

	CLISTMENUITEM mi = { sizeof(mi) };
	if (WumfOptions.PopupsEnabled == FALSE) { 
		mi.pszName = LPGEN("Enable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else {
		mi.pszName = LPGEN("Disable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
	}
	mi.pszService = MS_WUMF_SWITCHPOPUP;
	mi.popupPosition = 1999990000;
	mi.pszPopupName = LPGEN("Popups");
	hMenuItem =  Menu_AddMainMenuItem(&mi);

	mi.pszName = LPGEN("WUMF: Show connections");
	mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_DRIVE));
	mi.pszService = MS_WUMF_CONNECTIONSSHOW;
	mi.popupPosition = 1999990000;
	mi.pszPopupName = NULL;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_OPT_INITIALISE,OptionsInit);
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbar);

	if (IsUserAnAdmin())
		SetTimer(NULL, 777, TIME, TimerProc);
	else
		MessageBox(NULL, TranslateT("Plugin WhoUsesMyFiles requires admin privileges in order to work."), _T("Miranda NG"), MB_OK);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	KillTimer(NULL, 777);
	if (hLogger)
		mir_closeLog(hLogger);
	FreeAll();
	return 0;
}
