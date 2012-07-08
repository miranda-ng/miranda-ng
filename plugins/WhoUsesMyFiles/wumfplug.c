#include "wumf.h"

HINSTANCE hInst;
WUMF_OPTIONS WumfOptions = { 0 };
const char ModuleName[] = "WUMF Plugin";
HANDLE hMenuItem = 0;
extern HANDLE hLog;
static HWND hDlg;
static HANDLE hWumfBut;
extern PWumf list;
int hLangpack;

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), 
	"WUMF: Who Use My Files?", 
	PLUGIN_MAKE_VERSION(0,1,0,1),
	"Scans for network users of your shared files and notify you with popups. Uses PopUps Interoperability by Luca Santarelli. PopUps plugin must be activated!",
	"Nikolay Redko",
	"nike000@users.sf.net",
	"© 2003 Nike. Freeware. Please mail me all bugs & your suggestions.",	
	"http://miranda-im.org",
	UNICODE_AWARE,
	// {80DCA515-973A-4A7E-8B85-5D8EC88FC5A7}
	{ 0x80dca515, 0x973a, 0x4a7e, { 0x8b, 0x85, 0x5d, 0x8e, 0xc8, 0x8f, 0xc5, 0xa7 } }
};

void LoadOptions()
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_ASCIIZ;
	ZeroMemory(&WumfOptions, sizeof(WumfOptions));
	if (DBGetContactSetting(NULL, ModuleName, OPT_FILE, &dbv) == 0)
	{
		strncpy(WumfOptions.LogFile, dbv.pszVal, 255);
	}
	else
	{
		WumfOptions.LogFile[0] = '\0';
	};
	WumfOptions.PopupsEnabled = DBGetContactSettingByte(NULL,ModuleName, POPUPS_ENABLED, TRUE);

	WumfOptions.UseDefColor = DBGetContactSettingByte(NULL,ModuleName, COLOR_DEF, TRUE);
	WumfOptions.UseWinColor = DBGetContactSettingByte(NULL,ModuleName, COLOR_WIN, FALSE);
	WumfOptions.SelectColor = DBGetContactSettingByte(NULL,ModuleName, COLOR_SET, FALSE);

	WumfOptions.ColorText = DBGetContactSettingDword(NULL,ModuleName, COLOR_TEXT, RGB(0,0,0));
	WumfOptions.ColorBack = DBGetContactSettingDword(NULL,ModuleName, COLOR_BACK, RGB(255,255,255));
		
	WumfOptions.DelayDef = DBGetContactSettingByte(NULL,ModuleName, DELAY_DEF, TRUE);
	WumfOptions.DelayInf = DBGetContactSettingByte(NULL,ModuleName, DELAY_INF, FALSE);
	WumfOptions.DelaySet = DBGetContactSettingByte(NULL,ModuleName, DELAY_SET, FALSE);
	WumfOptions.DelaySec = DBGetContactSettingByte(NULL,ModuleName, DELAY_SEC, 0);
	if(!ServiceExists(MS_POPUP_ADDPOPUPEX))
	{
		WumfOptions.DelayDef = TRUE;
		WumfOptions.DelaySet = FALSE;
		WumfOptions.DelayInf = FALSE;
	}
	WumfOptions.LogToFile = DBGetContactSettingByte(NULL,ModuleName, LOG_INTO_FILE, FALSE);
	WumfOptions.LogFolders = DBGetContactSettingByte(NULL,ModuleName, LOG_FOLDER, TRUE);
	WumfOptions.AlertFolders = DBGetContactSettingByte(NULL,ModuleName, ALERT_FOLDER, TRUE);
	WumfOptions.LogUNC = DBGetContactSettingByte(NULL,ModuleName, LOG_UNC, FALSE);
	WumfOptions.AlertUNC = DBGetContactSettingByte(NULL,ModuleName, ALERT_UNC, FALSE);
	WumfOptions.LogComp = DBGetContactSettingByte(NULL,ModuleName, LOG_COMP, FALSE);
	WumfOptions.AlertComp = DBGetContactSettingByte(NULL,ModuleName, ALERT_COMP, FALSE);
	return;
}


void ExecuteMenu(HWND hWnd)
{
    HMENU hMenu;
    POINT point;

    hMenu=CreatePopupMenu();
    if(!hMenu)
    {
        MessageBox(NULL, "Error crerating menu", "WUMF",MB_OK);
        return;
    };
    AppendMenu(hMenu,MF_STRING,IDM_ABOUT,   "About\0");
    AppendMenu(hMenu,MF_SEPARATOR,0,NULL);//------------------
    AppendMenu(hMenu,MF_STRING,IDM_SHOW,    "Show connections\0");
    AppendMenu(hMenu,MF_SEPARATOR,0,NULL);//------------------
    AppendMenu(hMenu,MF_STRING,IDM_EXIT,    "Dismiss popup\0");

    GetCursorPos (&point);
    SetForegroundWindow (hWnd);

    TrackPopupMenu (hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_TOPALIGN,
                    point.x, point.y, 0, hWnd, NULL);

    PostMessage (hWnd, WM_USER, 0, 0);
    DestroyMenu(hMenu);
}

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	PWumf w = NULL;
	switch(message) {
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_ABOUT:
					break;
				case IDM_EXIT:
			       	PUDeletePopUp(hWnd);
			       	break;	
			    case IDM_SHOW:
			    	CallService(MS_WUMF_CONNECTIONSSHOW, (WPARAM)0, (LPARAM)0);
			    	return TRUE;
			}
			switch (HIWORD(wParam))
			{
				case STN_CLICKED:
					PUDeletePopUp(hWnd);
					return TRUE;
			}
			break;
		case WM_CONTEXTMENU: 
//			ExecuteMenu(hWnd);
            CallService(MS_WUMF_CONNECTIONSSHOW, (WPARAM)0, (LPARAM)0);
			break;		
		case UM_FREEPLUGINDATA: {
/*			w = (PWumf)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)w);
			if(w) free(w);*/
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
};

void ShowWumfPopUp(PWumf w)
{
	char text[512], title[512];

	if(!WumfOptions.AlertFolders && (w->dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return;
	mir_snprintf(title, sizeof(title)/sizeof(title[0]), "%s (%s)", w->szComp, w->szUser);
	mir_snprintf(text, sizeof(text)/sizeof(text[0]), "%s (%s)", w->szPath, w->szPerm);
    ShowThePopUp(w, title, text);
}
void ShowThePopUp(PWumf w, LPSTR title, LPSTR text)
{
	POPUPDATAEX ppd = { 0 };
	COLORREF colorBack;
	COLORREF colorText;

	colorBack = GetSysColor(COLOR_BTNFACE); 
	colorText = RGB(0,0,0);

	ppd.lchContact = NULL;
	ppd.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_DRIVE));
	ppd.iSeconds = -1; 

	if(WumfOptions.DelayInf)
	{
		ppd.iSeconds = -1; 
	}
	else if(WumfOptions.DelayDef)
	{
		ppd.iSeconds = 0;
	}
	else if(WumfOptions.DelaySet)
	{
		ppd.iSeconds = WumfOptions.DelaySec;
	}

	lstrcpyn(ppd.lpzContactName, title, 128);
	lstrcpyn(ppd.lpzText, text, 128);
	if(WumfOptions.UseWinColor)
	{
		ppd.colorBack = GetSysColor(COLOR_WINDOW);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
	}
	else if(WumfOptions.UseDefColor)
	{
		ppd.colorBack = 0L;
		ppd.colorText = 0L;
	}
	else if(WumfOptions.SelectColor)
	{
		ppd.colorBack = WumfOptions.ColorBack;
		ppd.colorText = WumfOptions.ColorText;
	}

	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.PluginData = w;
	if(CALLSERVICE_NOTFOUND == CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0))
		CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
}

void ShowThePreview()
{
	if(!ServiceExists(MS_POPUP_ADDPOPUP))
	{
		MessageBox(NULL,"PopUp plugin not found!", "WUMF plugin", MB_OK|MB_ICONSTOP);
		return;
	};
	if(WumfOptions.AlertFolders)
	{
			ShowThePopUp(NULL, "Guest", "C:\\My Share");
			Sleep(300);
			ShowThePopUp(NULL, "Guest", "C:\\My Share\\Photos");
			Sleep(300);
	}
	ShowThePopUp(NULL, "Guest", "C:\\Share\\My Photos\\photo.jpg");
	Sleep(300);
	if(WumfOptions.AlertFolders)
	{
			ShowThePopUp(NULL, "User", "C:\\My Share");
			Sleep(300);
			ShowThePopUp(NULL, "User", "C:\\My Share\\Movies");
			Sleep(300);
	}
	ShowThePopUp(NULL, "User", "C:\\My Share\\Movies\\The Two Towers.avi");
	Sleep(300);
	if(WumfOptions.AlertFolders)
	{
			ShowThePopUp(NULL, "Administrator", "C:\\Distributives");
			Sleep(300);
			ShowThePopUp(NULL, "Administrator", "C:\\Distributives\\Win2k");
			Sleep(300);
	}
	ShowThePopUp(NULL, "Administrator", "C:\\Distributives\\Win2k\\setup.exe");
};



BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	MSG msg; //Message pump.
	if(hDlg)
	{
		ShowWindow(hDlg, SW_SHOWNORMAL); 
		SetForegroundWindow(hDlg);
		return (int)(1);
	}
   	hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CONNLIST), NULL, (DLGPROC)ConnDlgProc);
   	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_DRIVE)));
   	ShowWindow(hDlg, SW_SHOW); 
	while(GetMessage(&msg, NULL, 0, 0) == TRUE) 
	{ 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	hDlg = NULL;
	ExitThread(0);
	
	return (int)(1);
}


static INT_PTR WumfShowConnections(WPARAM wParam,LPARAM lParam)
{
	DWORD threadID = 0;
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, (LPVOID)NULL,0,&threadID));
	Sleep(100);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hWumfBut, TTBST_RELEASED);
	return 0;
}

static INT_PTR WumfMenuCommand(WPARAM wParam,LPARAM lParam)
{
	BOOL MajorTo0121 = FALSE;
	int iResult = 0;
	CLISTMENUITEM mi = { 0 };

	mi.cbSize = sizeof(mi);
	if (WumfOptions.PopupsEnabled == TRUE) 
	{ 
		WumfOptions.PopupsEnabled = FALSE;
		mi.pszName = Translate("Enable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else 
	{
		WumfOptions.PopupsEnabled = TRUE;
		mi.pszName = Translate("Disable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
	}
	DBWriteContactSettingByte(NULL, ModuleName, POPUPS_ENABLED, (BYTE)WumfOptions.PopupsEnabled);
	mi.flags = CMIM_NAME | CMIM_ICON;
    iResult = CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
	return iResult;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
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
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_NOTE), TRUE);
}
void ChooseFile(HWND hDlg)
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for filename
	HANDLE hf;              // file handle
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner =hDlg;
	szFile[0]=0;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = "All files (*.*)\0*.*\0Text files (*.txt)\0*.txt\0Log files (*.log)\0*.log\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_CREATEPROMPT;
	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)==TRUE)
	{
		hf = CreateFile(ofn.lpstrFile, 
						GENERIC_WRITE,
						0, 
						(LPSECURITY_ATTRIBUTES) NULL,
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL,
						(HANDLE) NULL);
		if(hf!=INVALID_HANDLE_VALUE)
		{
			SetDlgItemText(hDlg,IDC_FILE,ofn.lpstrFile);
			lstrcpyn(WumfOptions.LogFile, ofn.lpstrFile, 255);
			CloseHandle(hf);
		}
	}
	else if(CommDlgExtendedError()!=0L)
	{
		char str[256];
		wsprintf(str,"Common Dialog Error 0x%lx",CommDlgExtendedError());
		MessageBox(hDlg,str,"Wumf plugin",MB_OK|MB_ICONSTOP);
	};

};

INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WORD wControlId = LOWORD(wparam);
	WORD wNotifyCode = HIWORD(wparam);
	int seconds;

	switch(msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_COLOR_WIN, WumfOptions.UseWinColor?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_COLOR_DEF, WumfOptions.UseDefColor?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_COLOR_SET, WumfOptions.SelectColor?BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BACK), WumfOptions.SelectColor);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_TEXT), WumfOptions.SelectColor);
			if(WumfOptions.SelectColor)
			{
				SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_SETCOLOUR,0,WumfOptions.ColorBack);
				SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_SETCOLOUR,0,WumfOptions.ColorText);
			}
			if(!ServiceExists(MS_POPUP_ADDPOPUPEX))
			{
				DisableDelayOptions(hwndDlg);
				break;
			}
			CheckDlgButton(hwndDlg, IDC_DELAY_INF, WumfOptions.DelayInf?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DELAY_DEF, WumfOptions.DelayDef?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DELAY_SET, WumfOptions.DelaySet?BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_SEC), WumfOptions.DelaySet);
			SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);
            //Logging & alerts
   			CheckDlgButton(hwndDlg, IDC_LOG_FOLDER, WumfOptions.LogFolders?BST_CHECKED:BST_UNCHECKED);
   			CheckDlgButton(hwndDlg, IDC_ALERT_FOLDER, WumfOptions.AlertFolders?BST_CHECKED:BST_UNCHECKED);
   			CheckDlgButton(hwndDlg, IDC_LOG_UNC, WumfOptions.LogUNC?BST_CHECKED:BST_UNCHECKED);
   			CheckDlgButton(hwndDlg, IDC_ALERT_UNC, WumfOptions.AlertUNC?BST_CHECKED:BST_UNCHECKED);
   			CheckDlgButton(hwndDlg, IDC_LOG_COMP, WumfOptions.LogComp?BST_CHECKED:BST_UNCHECKED);

			if(WumfOptions.LogToFile)
			{
				CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), TRUE);
				SetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile);
			}
			else 
			{
				CheckDlgButton(hwndDlg,IDC_LOG_INTO_FILE,BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEL_FILE), FALSE);
				SetDlgItemText(hwndDlg,IDC_FILE,"");
			};

			break;
		case WM_COMMAND:
			switch(wNotifyCode)
			{
				case BN_CLICKED :
					switch(wControlId)
					{
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
							MessageBox(NULL, "Not implemented yet...", "WUMF", MB_OK|MB_ICONINFORMATION);
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
/*						case IDC_LOG_COMP:
							WumfOptions.LogComp = (IsDlgButtonChecked(hwndDlg, IDC_LOG_COMP) == BST_CHECKED);
							break;
						case IDC_ALERT_COMP:
							WumfOptions.AlertComp = (IsDlgButtonChecked(hwndDlg, IDC_ALERT_COMP) == BST_CHECKED);
							break;
						case IDC_LOG_UNC:
							WumfOptions.LogUNC = (IsDlgButtonChecked(hwndDlg, IDC_LOG_UNC) == BST_CHECKED);
							break;
						case IDC_ALERT_UNC:
							WumfOptions.AlertUNC = (IsDlgButtonChecked(hwndDlg, IDC_ALERT_UNC) == BST_CHECKED);
							break;
*/						case IDC_PREVIEW:
							ShowThePreview();
							break;
						case IDC_CONN:
					    	CallService(MS_WUMF_CONNECTIONSSHOW, (WPARAM)0, (LPARAM)0);
							break;
					}
					break;
				case CPN_COLOURCHANGED:						
					WumfOptions.ColorText = SendDlgItemMessage(hwndDlg,IDC_COLOR_TEXT,CPM_GETCOLOUR,0,0);
					WumfOptions.ColorBack = SendDlgItemMessage(hwndDlg,IDC_COLOR_BACK,CPM_GETCOLOUR,0,0);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case EN_CHANGE:
					switch(wControlId)
					{
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
			    			GetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile, sizeof(WumfOptions.LogFile));
			    			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			    			break;
					}
					break;
				case EN_KILLFOCUS:
					switch(wControlId)
					{
						case IDC_DELAY_SEC:
							SetDlgItemInt(hwndDlg, IDC_DELAY_SEC, WumfOptions.DelaySec, FALSE);								
					        break;
					};
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
							DBWriteContactSettingByte(NULL,ModuleName, COLOR_DEF, (BYTE)WumfOptions.UseDefColor);
							DBWriteContactSettingByte(NULL,ModuleName, COLOR_WIN, (BYTE)WumfOptions.UseWinColor);
							DBWriteContactSettingByte(NULL,ModuleName, COLOR_SET, (BYTE)WumfOptions.SelectColor );
							DBWriteContactSettingDword(NULL,ModuleName, COLOR_TEXT, (DWORD)WumfOptions.ColorText);
							DBWriteContactSettingDword(NULL,ModuleName, COLOR_BACK, (DWORD)WumfOptions.ColorBack);
							DBWriteContactSettingByte(NULL,ModuleName, DELAY_DEF, (BYTE)WumfOptions.DelayDef);
							DBWriteContactSettingByte(NULL,ModuleName, DELAY_INF, (BYTE)WumfOptions.DelayInf);
							DBWriteContactSettingByte(NULL,ModuleName, DELAY_SET, (BYTE)WumfOptions.DelaySet);
							DBWriteContactSettingByte(NULL,ModuleName, DELAY_SEC, (BYTE)WumfOptions.DelaySec);
							DBWriteContactSettingByte(NULL,ModuleName, LOG_INTO_FILE, (BYTE)WumfOptions.LogToFile);
							DBWriteContactSettingByte(NULL,ModuleName, LOG_FOLDER, (BYTE)WumfOptions.LogFolders);
							DBWriteContactSettingByte(NULL,ModuleName, ALERT_FOLDER, (BYTE)WumfOptions.AlertFolders);
							DBWriteContactSettingByte(NULL,ModuleName, LOG_UNC, (BYTE)WumfOptions.LogUNC);
							DBWriteContactSettingByte(NULL,ModuleName, ALERT_UNC, (BYTE)WumfOptions.AlertUNC);
							DBWriteContactSettingByte(NULL,ModuleName, LOG_COMP, (BYTE)WumfOptions.LogComp);
							DBWriteContactSettingByte(NULL,ModuleName, ALERT_COMP, (BYTE)WumfOptions.AlertComp);
							GetDlgItemText(hwndDlg,IDC_FILE,WumfOptions.LogFile, 255);
							DBWriteContactSettingString(NULL,ModuleName, OPT_FILE, WumfOptions.LogFile);
							break;
					}
					break;
			}
			break;
	}
	return 0;
}

int InitTopToolbar(WPARAM wparam,LPARAM lparam)
{
	TTBButton ttb = { 0 };
	char buttonname[] = "WUMF: Show connections list";
	ttb.cbSize = sizeof(ttb);
	ttb.hIconUp = LoadIcon(hInst, MAKEINTRESOURCE(IDB_DRIVE));
	ttb.hIconDn = ttb.hIconUp;
	ttb.pszService = MS_WUMF_CONNECTIONSSHOW;
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
	ttb.name = buttonname;
	hWumfBut = TopToolbar_AddButton(&ttb);
	return 0;
}

int OptionsInit(WPARAM wparam,LPARAM lparam)
{
	OPTIONSDIALOGPAGE odp = { 0 };


    odp.cbSize=sizeof(odp);
    odp.position=945000000;
    odp.hInstance=hInst;
    odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
    odp.pszTitle="Wumf";
    odp.pfnDlgProc=OptionsDlgProc;
	odp.pszGroup=Translate("Plugins");
	odp.flags=ODPF_BOLDGROUPS;
    Options_AddPage(wparam, &odp);
	return 0;
}

__declspec(dllexport) int Load(void)
{
    CLISTMENUITEM mi = { 0 };

	mir_getLP(&pluginInfo);

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	LoadOptions();

	CreateServiceFunction(MS_WUMF_SWITCHPOPUP, WumfMenuCommand);
	CreateServiceFunction(MS_WUMF_CONNECTIONSSHOW, WumfShowConnections);
	if (WumfOptions.PopupsEnabled == FALSE) 
	{ 
		mi.pszName = Translate("Enable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_NOPOPUP));
	}
	else 
	{
		mi.pszName = Translate("Disable WUMF popups");
		mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_POPUP));
	}
	mi.pszService = MS_WUMF_SWITCHPOPUP;
	mi.popupPosition = 1999990000;
	mi.pszPopupName = Translate("PopUps");
	hMenuItem = (HANDLE)Menu_AddMainMenuItem(&mi);

	mi.pszName = Translate("WUMF: Show connections");
	mi.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_DRIVE));
	mi.pszService = MS_WUMF_CONNECTIONSSHOW;
	mi.popupPosition = 1999990000;
	mi.pszPopupName = NULL;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_OPT_INITIALISE,OptionsInit);
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbar);
   	setlocale( LC_ALL, ".ACP");
//   	_setmbcp(_MB_CP_ANSI);
	SetTimer(NULL, 777, TIME,(TIMERPROC) TimerProc);
	return 0;

}

__declspec(dllexport) int Unload(void)
{
    KillTimer(NULL, 777);
    CloseHandle(hLog);
	FreeAll();
	return 0;
}