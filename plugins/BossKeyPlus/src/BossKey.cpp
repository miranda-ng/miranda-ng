/*
	BossKey - Hide Miranda from your boss :)
	Copyright (C) 2002-2003 Goblineye Entertainment, (C) 2007-2010 Billy_Bons

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

#include "BossKey.h"

HINSTANCE g_hInstance;
CLIST_INTERFACE *pcli;
HGENMENU g_hMenuItem;
HANDLE g_hHideService;
HWINEVENTHOOK g_hWinHook;
HWND g_hListenWindow, g_hDlgPass, hOldForegroundWindow;
HWND_ITEM *g_pMirWnds; // a pretty simple linked list
HMODULE hDwmApi;
DWORD g_dwMirandaPID;
WORD g_wMask, g_wMaskAdv;
bool g_bWindowHidden, g_fPassRequested, g_TrayIcon;
char g_password[MAXPASSLEN + 1];
HKL oldLangID, oldLayout;
int protoCount, hLangpack;
PROTOACCOUNT **proto;
unsigned *oldStatus;
TCHAR **oldStatusMsg;
BYTE g_bOldSetting;

PFNDwmIsCompositionEnabled dwmIsCompositionEnabled;

void LanguageChanged(HWND hDlg);

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {4FAC353D-0A36-44A4-9064-6759C53AE782}
	{0x4fac353d, 0x0a36, 0x44a4, {0x90, 0x64, 0x67, 0x59, 0xc5, 0x3a, 0xe7, 0x82}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstance = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static BOOL IsAeroMode()
{
	BOOL result;
	return dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
}

INT_PTR CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static DWORD dwOldIcon = 0;
	HICON hIcon = 0;

	switch(uMsg){
	case WM_INITDIALOG:
		g_hDlgPass = hDlg;
		hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DLGPASSWD));
		dwOldIcon = SetClassLongPtr(hDlg, GCLP_HICON, (LONG)hIcon); // set alt+tab icon
		SendDlgItemMessage(hDlg,IDC_EDIT1,EM_LIMITTEXT,MAXPASSLEN,0);

		if (IsAeroMode())
		{
			SetWindowLongPtr(hDlg, GWL_STYLE, GetWindowLongPtr(hDlg, GWL_STYLE) | WS_DLGFRAME | WS_SYSMENU);
			SetWindowLongPtr(hDlg, GWL_EXSTYLE, GetWindowLongPtr(hDlg, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
			RECT rect;
			GetClientRect(hDlg, &rect);
			SetWindowPos(hDlg, 0, 0, 0, rect.right, rect.bottom + GetSystemMetrics(SM_CYCAPTION), SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER);
		}
		SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)hIcon);
		SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("Miranda NG is locked.\nEnter password to unlock it."));

		TranslateDialogDefault(hDlg);
		oldLangID = 0;
		SetTimer(hDlg,1,200,NULL);

		oldLayout = GetKeyboardLayout(0);
		if (MAKELCID((WORD)oldLayout & 0xffffffff,  SORT_DEFAULT) != (LCID)0x00000409)
			ActivateKeyboardLayout((HKL)0x00000409, 0);
		LanguageChanged(hDlg);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if (GetWindowLongPtr((HWND)lParam, GWLP_ID) != IDC_LANG)
			break;

		SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (INT_PTR)GetSysColorBrush(COLOR_HIGHLIGHT);

	case WM_COMMAND:
		{
			UINT uid = LOWORD(wParam);
			if (uid == IDOK){
				char password[MAXPASSLEN + 1] = {0};
				int passlen = GetDlgItemTextA(hDlg,IDC_EDIT1,password,SIZEOF(password));

				if (passlen == 0)
				{
					SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("Miranda NG is locked.\nEnter password to unlock it."));
					SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
					break;
				}
				else if (mir_strcmp(password, g_password))
				{
					SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("Password is not correct!\nPlease, enter correct password."));
					SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
					SetDlgItemText(hDlg, IDC_EDIT1, _T(""));
					break;
				}
				else EndDialog(hDlg, IDOK);
			}
			else if (uid == IDCANCEL)
				EndDialog(hDlg, IDCANCEL);
		}

	case WM_TIMER:
		LanguageChanged(hDlg);
		break;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		if (GetKeyboardLayout(0) != oldLayout)
			ActivateKeyboardLayout(oldLayout, 0);
		SetClassLongPtr(hDlg, GCLP_HICON, (long)dwOldIcon);
		DestroyIcon(hIcon);
		break;
	}
	return FALSE;
}

static void LanguageChanged(HWND hDlg)
{
	HKL LangID = GetKeyboardLayout(0);
	if (LangID != oldLangID)
	{
		char Lang[3] = {0};
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID(((WORD)LangID & 0xffffffff),  SORT_DEFAULT),  LOCALE_SABBREVLANGNAME, Lang, 2); 
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM)
{
	DWORD dwWndPID;
	GetWindowThreadProcessId(hWnd,&dwWndPID);

	if ((g_dwMirandaPID == dwWndPID) && hWnd != g_hDlgPass && IsWindowVisible(hWnd))
	{
		TCHAR szTemp[32];
		GetClassName(hWnd,szTemp,32);

		if (mir_tstrcmp(szTemp,_T("MirandaThumbsWnd")) == 0) // hide floating contacts
		{
			CallService("FloatingContacts/MainHideAllThumbs",0,0);
			g_bOldSetting |= OLD_FLTCONT;
		}
		else if (mir_tstrcmp(szTemp,_T("PopupWnd2")) == 0 || mir_tstrcmp(szTemp,_T("YAPPWinClass")) == 0) // destroy opened popups
			PUDeletePopup(hWnd);
		else
		{
			HWND_ITEM *node = new HWND_ITEM;
			node->hWnd = hWnd;
			// add to list
			node->next = g_pMirWnds;
			g_pMirWnds = node;
			ShowWindow(hWnd,SW_HIDE);
		}
	}
	return true;
}

TCHAR* GetDefStatusMsg(unsigned uStatus, const char* szProto)
{
	return (TCHAR*)CallService (MS_AWAYMSG_GETSTATUSMSGT, uStatus, (LPARAM)szProto);
}

void SetStatus(const char* szProto, unsigned status, TCHAR *tszAwayMsg)
{
	if ( tszAwayMsg && CallProtoService( szProto, PS_GETCAPS, PFLAGNUM_1, 0 ) & PF1_MODEMSGSEND )
	{
		if ( CallProtoService( szProto, PS_SETAWAYMSGT, status, (LPARAM) tszAwayMsg ) == CALLSERVICE_NOTFOUND )
		{
			char *szAwayMsg = mir_t2a(tszAwayMsg);
			CallProtoService( szProto, PS_SETAWAYMSG, status, (LPARAM)szAwayMsg );
			mir_free(szAwayMsg);
		}
	}
	CallProtoService( szProto, PS_SETSTATUS, status, 0 );
}

static int ChangeAllProtoStatuses(unsigned statusMode, TCHAR *msg)
{
	for (int i=0; i < protoCount; i++)
	{
		unsigned status = CallProtoService(proto[i]->szModuleName,PS_GETSTATUS,0,0);
		if (
			(g_wMask & OPT_ONLINEONLY) ? // check "Change only if current status is Online" option
			((status == ID_STATUS_ONLINE) || (status == ID_STATUS_FREECHAT)) // process only "online" and "free for chat"
			:
			((status > ID_STATUS_OFFLINE) && (status < ID_STATUS_IDLE) && (status != ID_STATUS_INVISIBLE))) // process all existing statuses except for "invisible" & "offline"
		{
			if (g_wMask & OPT_SETONLINEBACK){ // need to save old statuses & status messages
				oldStatus[i] = status;
				if (ProtoServiceExists(proto[i]->szModuleName, PS_GETMYAWAYMSG))
					oldStatusMsg[i] = (TCHAR*)ProtoCallService(proto[i]->szModuleName, PS_GETMYAWAYMSG, 0, SGMA_TCHAR);
				else
					oldStatusMsg[i] = GetDefStatusMsg(status, proto[i]->szModuleName);
			}
			SetStatus(proto[i]->szModuleName, statusMode, msg);
		}
	}
	return 0;
}

static int BackAllProtoStatuses(void)
{
	for (int i=0; i < protoCount; i++)
	{
		if ( oldStatus[i] )
		{
			SetStatus(proto[i]->szModuleName, oldStatus[i], oldStatusMsg[i]);
			if (oldStatusMsg[i])
			{
				mir_free(oldStatusMsg[i]);
				oldStatusMsg[i] = 0;
			}
			oldStatus[i] = 0;
		}
	}
	return 0;
}

static void CreateTrayIcon(bool create)
{
	NOTIFYICONDATA nim;
	DBVARIANT dbVar;
	if (!db_get_ts(NULL,MOD_NAME,"ToolTipText",&dbVar))
	{
		mir_sntprintf(nim.szTip, 64, _T("%s"), dbVar.ptszVal);
		db_free(&dbVar);
	}
	else
		mir_tstrcpy(nim.szTip, _T("Miranda NG"));

	nim.cbSize = sizeof(nim);
	nim.hWnd = g_hListenWindow;
	nim.uID = 100;
	nim.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nim.hIcon = Skin_GetIcon("hidemim");
	nim.uCallbackMessage = WM_USER + 24;
	Shell_NotifyIcon(create ? NIM_ADD : NIM_DELETE, &nim);
	g_TrayIcon = create;
}

static void RestoreOldSettings(void)
{
	if (g_bOldSetting & OLD_POPUP)
		CallService(MS_POPUP_QUERY, PUQS_ENABLEPOPUPS, 0);

	if (g_bOldSetting & OLD_SOUND)
		db_set_b(NULL,"Skin","UseSound", 1);

	if (g_bOldSetting & OLD_FLTCONT) // show Floating contacts if needed
	{
		if(ServiceExists("FloatingContacts/MainHideAllThumbs"))
			CallService("FloatingContacts/MainHideAllThumbs",0,0);
		else
			db_set_b(NULL,"FloatingContacts","HideAll", 0);
	}
	g_bOldSetting = 0;
}

LRESULT CALLBACK ListenWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg){
	case WM_WTSSESSION_CHANGE:
		if (wParam == WTS_SESSION_LOCK && g_wMaskAdv & OPT_HIDEIFLOCK && !g_bWindowHidden) // Windows locked
			PostMessage(hWnd,WM_USER + 40, 0, 0);
		return 0;

	case WM_USER + 24:
		if (lParam == WM_LBUTTONDBLCLK)
			PostMessage(hWnd,WM_USER + 52, 0, 0);
		return 0;

	case WM_USER+40: // hide
		{
			if (g_bWindowHidden || g_fOptionsOpen) // already hidden or in options, no hiding
				break;

			DWORD dwWndPID; // remember foreground window
			HWND hForegroundWnd = GetForegroundWindow();
			GetWindowThreadProcessId(hForegroundWnd,&dwWndPID);
			if (g_dwMirandaPID == dwWndPID)
				hOldForegroundWindow = hForegroundWnd;

			EnumWindows(EnumWindowsProc, 0);

			if (g_wMask & OPT_CHANGESTATUS) // is this even needed?
			{
				BYTE bReqMode = db_get_b(NULL, MOD_NAME, "stattype", 2);
				unsigned uMode = (STATUS_ARR_TO_ID[bReqMode]);
				DBVARIANT dbVar;
				if (g_wMask & OPT_USEDEFMSG || db_get_ts(NULL,MOD_NAME,"statmsg",&dbVar))
				{
					TCHAR *ptszDefMsg = GetDefStatusMsg(uMode, 0);
					ChangeAllProtoStatuses(uMode, ptszDefMsg);
					mir_free(ptszDefMsg);
				}
				else
				{
					if (ServiceExists(MS_VARS_FORMATSTRING))
					{
						TCHAR *ptszParsed = variables_parse(dbVar.ptszVal, 0, 0);
						ChangeAllProtoStatuses(uMode, ptszParsed);
						mir_free(ptszParsed);
					}
					else
						ChangeAllProtoStatuses(uMode, dbVar.ptszVal);
					db_free(&dbVar);
				}
			}

			pcli->pfnTrayIconDestroy (pcli->hwndContactList);
			pcli->pfnUninitTray();

			if (g_wMask & OPT_TRAYICON)
				CreateTrayIcon(true);

			// disable popups
			if (CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == 1)
			{
				// save current
				g_bOldSetting |= OLD_POPUP;
				CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
			}

			// disable sounds
			if ((g_wMask & OPT_DISABLESNDS) && db_get_b(NULL,"Skin","UseSound",1))
			{
				// save current
				g_bOldSetting |= OLD_SOUND;
				db_set_b(NULL,"Skin","UseSound",0);
			}

			g_bWindowHidden = true;

			g_bOldSetting |= OLD_WASHIDDEN;
			db_set_b(NULL, MOD_NAME, "OldSetting", g_bOldSetting);
		}
		return 0;

	case WM_USER+52: // back
		{
			if (!g_bWindowHidden || g_fPassRequested)
				break;

			if (g_wMask & OPT_REQPASS){  //password request
				DBVARIANT dbVar;
				if (!db_get_s(NULL,MOD_NAME, "password", &dbVar)) {
					g_fPassRequested = true;

					strncpy(g_password, dbVar.pszVal, MAXPASSLEN);
					db_free(&dbVar);

					int res = DialogBox(g_hInstance,(MAKEINTRESOURCE(IDD_PASSDIALOGNEW)),GetForegroundWindow(), DlgStdInProc);

					g_fPassRequested = false;
					if(res != IDOK) return 0;
				}
			}

			if (g_wMask & OPT_CHANGESTATUS && g_wMask & OPT_SETONLINEBACK) // set back to some status
				BackAllProtoStatuses();

			HWND_ITEM *pCurWnd = g_pMirWnds;
			while (pCurWnd != NULL)
			{
				HWND_ITEM *pNextWnd = pCurWnd->next;
				TCHAR szTemp[32];
				GetClassName(pCurWnd->hWnd,szTemp,32);

				if (IsWindow(pCurWnd->hWnd) && mir_tstrcmp(szTemp,_T("SysShadow")) != 0) // precaution
					ShowWindow(pCurWnd->hWnd, SW_SHOW);

				delete pCurWnd; // bye-bye
				pCurWnd = pNextWnd; // traverse to next item
			}
			g_pMirWnds = NULL;

			if (hOldForegroundWindow)
			{
				SetForegroundWindow(hOldForegroundWindow);
				hOldForegroundWindow = NULL;
			}

			RestoreOldSettings();

			if (g_TrayIcon) CreateTrayIcon(false);

			pcli->pfnInitTray();
			pcli->pfnTrayIconInit(pcli->hwndContactList);

			// force a redraw
			// should prevent drawing problems
			InvalidateRect(pcli->hwndContactList,NULL,true);
			UpdateWindow(pcli->hwndContactList);

			PostMessage(hWnd, WM_MOUSEMOVE, 0, (LPARAM)MAKELONG(2, 2)); // reset core's IDLE
			g_bWindowHidden = false;

			db_set_b(NULL, MOD_NAME, "OldSetting", 0);
		}
		return 0;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

static int MsgWinOpening(WPARAM, LPARAM) // hiding new message windows
{
	if (g_bWindowHidden)
		EnumWindows(EnumWindowsProc, 0);
	return 0;
}

VOID CALLBACK WinEventProc(HWINEVENTHOOK g_hWinHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (g_bWindowHidden && idObject == OBJID_WINDOW && (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) && (IsWindowVisible(hwnd)))
	{
		if(hwnd == pcli->hwndContactList)
			ShowWindow(hwnd, SW_HIDE);
		else
			EnumWindows(EnumWindowsProc, 0);
	}
}

INT_PTR BossKeyHideMiranda(WPARAM, LPARAM) // for service :)
{
	PostMessage(g_hListenWindow, WM_USER + ((g_bWindowHidden) ? (52) : (40)), 0, 0);
	return 0;
}

static TCHAR *HokeyVkToName(WORD vkKey)
{
	static TCHAR buf[32] = {0};
	DWORD code = MapVirtualKey(vkKey, 0) << 16;

	switch (vkKey)
	{
	case 0:
	case VK_CONTROL:
	case VK_SHIFT:
	case VK_MENU:
	case VK_LWIN:
	case VK_RWIN:
	case VK_PAUSE:
	case VK_CANCEL:
	case VK_CAPITAL:
		return _T("");

	case VK_DIVIDE:
	case VK_INSERT:
	case VK_HOME:
	case VK_PRIOR:
	case VK_DELETE:
	case VK_END:
	case VK_NEXT:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
	case VK_NUMLOCK:
		code |= (1UL << 24);
	}

	GetKeyNameText(code, buf, SIZEOF(buf));
	return buf;
}

static TCHAR *GetBossKeyText(void)
{
	WORD wHotKey = db_get_w(NULL,"SkinHotKeys", "Hide/Show Miranda", HOTKEYCODE(HOTKEYF_CONTROL, VK_F12));

	BYTE key = LOBYTE(wHotKey);
	BYTE shift = HIBYTE(wHotKey);
	static TCHAR buf[128] = {0};

	mir_sntprintf(buf, SIZEOF(buf), _T("%s%s%s%s%s"),
		(shift & HOTKEYF_CONTROL)	? _T("Ctrl + ")		: _T(""),
		(shift & HOTKEYF_SHIFT)		? _T("Shift + ")	: _T(""),
		(shift & HOTKEYF_ALT)		? _T("Alt + ")		: _T(""),
		(shift & HOTKEYF_EXT)		? _T("Win + ")		: _T(""),
		HokeyVkToName(key));

	return buf;
}

static IconItem iconList[] =
{
	{ LPGEN("Hide Miranda NG"), "hidemim", IDI_DLGPASSWD }
};

static int GenMenuInit(WPARAM, LPARAM) // Modify menu item text before to show the main menu
{
	if (g_hMenuItem) {
		TCHAR buf[128] = {0};
		mir_sntprintf(buf, SIZEOF(buf), _T("%s [%s]"), TranslateT("Hide"), GetBossKeyText());

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS | CMIF_TCHAR  | CMIM_NAME;
		mi.ptszName = buf;

		Menu_ModifyItem(g_hMenuItem, &mi);
	}
	return 0;
}

void BossKeyMenuItemInit(void) // Add menu item
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.position = 2000100000;
	mi.pszPopupName = 0;
	mi.hIcon = Skin_GetIcon("hidemim");
	mi.ptszName = LPGENT("Hide");
	mi.pszService = MS_BOSSKEY_HIDE;

	g_hMenuItem = Menu_AddMainMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDMAINMENU, GenMenuInit);
}

void BossKeyMenuItemUnInit(void) // Remove menu item
{
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuItem, 0);
	g_hMenuItem = 0;
}

void RegisterCoreHotKeys (void)
{
	HOTKEYDESC hotkey = {0};
	hotkey.cbSize = sizeof(HOTKEYDESC);
	hotkey.pszName = "Hide/Show Miranda";
	hotkey.pszDescription = LPGEN("Hide/Show Miranda");
	hotkey.pszSection = "BossKey";
	hotkey.pszService = MS_BOSSKEY_HIDE;
	hotkey.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F12);

	Hotkey_Register(&hotkey);
}

static int TopToolbarInit(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.pszService = MS_BOSSKEY_HIDE;
	ttb.pszTooltipUp = ttb.name = LPGEN("Hide Miranda NG");
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.hIconHandleUp = iconList[0].hIcolib;
	TopToolbar_AddButton(&ttb);
	return 0;
}

// Tabsrmm toolbar support
static int TabsrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	if (!strcmp(cbcd->pszModule, MOD_NAME))
		BossKeyHideMiranda(0, 0);

	return 0;
}

static int TabsrmmButtonsInit(WPARAM, LPARAM)
{
	BBButton bbd = {0};

	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = MOD_NAME;
	bbd.dwDefPos = 5000;
	bbd.ptszTooltip = LPGENT("Hide Miranda NG");
	bbd.bbbFlags = BBBF_ISRSIDEBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = iconList[0].hIcolib;
	CallService (MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
}

static TCHAR *VariablesBossKey(ARGUMENTSINFO *ai) {
	if (ai->cbSize < sizeof(ARGUMENTSINFO))	return NULL;
	if (ai->argc != 1) return NULL;

	ai->flags |= AIF_DONTPARSE;

	return GetBossKeyText();
}

static int EnumProtos(WPARAM wParam, LPARAM lParam)
{
	ProtoEnumAccounts(&protoCount, &proto);

	delete[] oldStatus;
	delete[] oldStatusMsg;

	oldStatus = new unsigned[protoCount];
	oldStatusMsg = new TCHAR* [protoCount];
	for (int i = 0; i < protoCount; i++)
	{
		oldStatus[i] = 0;
		oldStatusMsg[i] = 0;
	}
	return 0;
}

int MirandaLoaded(WPARAM,LPARAM)
{
	g_wMask = db_get_w(NULL,MOD_NAME,"optsmask",DEFAULTSETTING);

	RegisterCoreHotKeys();

	g_hWinHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_SHOW, NULL, WinEventProc, GetCurrentProcessId(), 0, 0);

	HookEvent(ME_TTB_MODULELOADED, TopToolbarInit);
	HookEvent(ME_OPT_INITIALISE, OptsDlgInit);
	HookEvent(ME_MSG_WINDOWEVENT, MsgWinOpening);
	HookEvent(ME_PROTO_ACCLISTCHANGED, EnumProtos);
	HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit);
	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);

	GetWindowThreadProcessId(pcli->hwndContactList, &g_dwMirandaPID);

	WNDCLASS winclass = {0};
	winclass.lpfnWndProc = ListenWndProc;
	winclass.hInstance = g_hInstance;
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszClassName = BOSSKEY_LISTEN_INFO;

	if (RegisterClass(&winclass))
	{
		g_hListenWindow = CreateWindow(BOSSKEY_LISTEN_INFO,BOSSKEY_LISTEN_INFO,WS_POPUP,0,0,5,5,pcli->hwndContactList,NULL,g_hInstance,NULL);
		WTSRegisterSessionNotification(g_hListenWindow, 0);
	}

	if (IsWinVerVistaPlus())
	{
		hDwmApi = LoadLibrary(_T("dwmapi.dll"));
		if (hDwmApi)
			dwmIsCompositionEnabled = (PFNDwmIsCompositionEnabled)GetProcAddress(hDwmApi,"DwmIsCompositionEnabled");
	}
	if (g_wMaskAdv & OPT_MENUITEM)
		BossKeyMenuItemInit();

	// Register token for variables plugin
	if (ServiceExists(MS_VARS_REGISTERTOKEN))
	{
		TOKENREGISTER tr = {0};
		tr.cbSize = sizeof(TOKENREGISTER);
		tr.memType = TR_MEM_OWNER;
		tr.flags = TRF_FIELD | TRF_TCHAR | TRF_PARSEFUNC;

		tr.tszTokenString = _T("bosskeyname");
		tr.parseFunctionT = VariablesBossKey;
		tr.szHelpText = LPGEN("BossKey")"\t"LPGEN("get the BossKey name");
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM) &tr);
	}

	EnumProtos(0, 0);
	InitIdleTimer();

	if (g_bOldSetting && !(g_wMaskAdv & OPT_RESTORE)) // Restore settings if Miranda was crushed or killed in hidden mode and "Restore hiding on startup after failure" option is disabled
		RestoreOldSettings();

	if ((g_wMaskAdv & OPT_HIDEONSTART) || (g_wMaskAdv & OPT_RESTORE && g_bOldSetting))
		BossKeyHideMiranda(0, 0);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	g_wMaskAdv = db_get_w(NULL,MOD_NAME,"optsmaskadv",0);
	g_bOldSetting = db_get_b(NULL, MOD_NAME, "OldSetting", 0);

	if ((g_bOldSetting & OLD_POPUP) && !(g_wMaskAdv & OPT_RESTORE)) // Restore popup settings if Miranda was crushed or killed in hidden mode and "Restore hiding on startup after failure" option is disabled
	{
		if (db_get_b(NULL, "Popup", "ModuleIsEnabled", 1) == 0)
			db_set_b(NULL, "Popup", "ModuleIsEnabled", 1);
	}
	if (g_wMaskAdv & OPT_HIDEONSTART && db_get_b(NULL, "Popup", "ModuleIsEnabled", 0)) // hack for disabling popup on startup if "Hide Miranda on startup" is enabled
	{
		g_bOldSetting |= OLD_POPUP;
		db_set_b(NULL, "Popup", "ModuleIsEnabled", 0);
	}

	Icon_Register(g_hInstance, "BossKey", iconList, SIZEOF(iconList));

	g_hHideService = CreateServiceFunction(MS_BOSSKEY_HIDE,BossKeyHideMiranda); // Create service

	HookEvent(ME_SYSTEM_MODULESLOADED,MirandaLoaded);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UninitIdleTimer();

	if (g_hWinHook != 0)
		UnhookWinEvent(g_hWinHook);

	DestroyServiceFunction(g_hHideService);

	if (g_hListenWindow)
	{
		WTSUnRegisterSessionNotification(g_hListenWindow);
		DestroyWindow(g_hListenWindow);
	}

	if (hDwmApi)
		FreeLibrary(hDwmApi);

	// free all sessions
	HWND_ITEM *pTemp = g_pMirWnds;
	while (pTemp != NULL)
	{
		HWND_ITEM *pNext = pTemp->next;
		delete pTemp;
		pTemp = pNext;
	}

	delete[] oldStatus;
	delete[] oldStatusMsg;

	return 0;
}
