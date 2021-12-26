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

#include "stdafx.h"

HGENMENU g_hMenuItem;
HWINEVENTHOOK g_hWinHook;
HWND g_hListenWindow, g_hDlgPass, hOldForegroundWindow;
HWND_ITEM *g_pMirWnds; // a pretty simple linked list
HMODULE hDwmApi;
DWORD g_dwMirandaPID;
uint16_t g_wMask, g_wMaskAdv;
bool g_bWindowHidden, g_fPassRequested, g_TrayIcon;
char g_password[MAXPASSLEN + 1];
HKL oldLangID, oldLayout;
int protoCount;
PROTOACCOUNT **proto;
unsigned *oldStatus;
wchar_t **oldStatusMsg;
uint8_t g_bOldSetting;

CMPlugin g_plugin;

PFNDwmIsCompositionEnabled dwmIsCompositionEnabled;

static void LanguageChanged(HWND hDlg);

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {4FAC353D-0A36-44A4-9064-6759C53AE782}
	{ 0x4fac353d, 0x0a36, 0x44a4, { 0x90, 0x64, 0x67, 0x59, 0xc5, 0x3a, 0xe7, 0x82 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static BOOL IsAeroMode()
{
	BOOL result;
	return dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
}

INT_PTR CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint32_t dwOldIcon = 0;
	HICON hIcon = nullptr;
	UINT uid;

	switch (uMsg) {
	case WM_INITDIALOG:
		g_hDlgPass = hDlg;
		hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DLGPASSWD));
		dwOldIcon = SetClassLongPtr(hDlg, GCLP_HICON, (INT_PTR)hIcon); // set alt+tab icon
		SendDlgItemMessage(hDlg, IDC_EDIT1, EM_LIMITTEXT, MAXPASSLEN, 0);

		if (IsAeroMode()) {
			SetWindowLongPtr(hDlg, GWL_STYLE, GetWindowLongPtr(hDlg, GWL_STYLE) | WS_DLGFRAME | WS_SYSMENU);
			SetWindowLongPtr(hDlg, GWL_EXSTYLE, GetWindowLongPtr(hDlg, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
			RECT rect;
			GetClientRect(hDlg, &rect);
			SetWindowPos(hDlg, nullptr, 0, 0, rect.right, rect.bottom + GetSystemMetrics(SM_CYCAPTION), SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER);
		}
		SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)hIcon);

		TranslateDialogDefault(hDlg);
		oldLangID = nullptr;
		SetTimer(hDlg, 1, 200, nullptr);

		oldLayout = GetKeyboardLayout(0);
		if (MAKELCID(LOWORD(oldLayout) & 0xffffffff, SORT_DEFAULT) != (LCID)0x00000409)
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
		uid = LOWORD(wParam);
		if (uid == IDOK) {
			char password[MAXPASSLEN + 1] = { 0 };
			int passlen = GetDlgItemTextA(hDlg, IDC_EDIT1, password, _countof(password));
			if (passlen == 0) {
				SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("Miranda NG is locked.\nEnter password to unlock it."));
				SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
				break;
			}
			else if (mir_strcmp(password, g_password)) {
				SetDlgItemText(hDlg, IDC_HEADERBAR, TranslateT("Password is not correct!\nPlease, enter correct password."));
				SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
				SetDlgItemText(hDlg, IDC_EDIT1, L"");
				break;
			}
			else EndDialog(hDlg, IDOK);
		}
		else if (uid == IDCANCEL)
			EndDialog(hDlg, IDCANCEL);

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
	if (LangID != oldLangID) {
		char Lang[3] = { 0 };
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LOWORD(LangID) & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM)
{
	DWORD dwWndPID;
	GetWindowThreadProcessId(hWnd, &dwWndPID);

	if ((g_dwMirandaPID == dwWndPID) && hWnd != g_hDlgPass && IsWindowVisible(hWnd)) {
		wchar_t szTemp[32];
		GetClassName(hWnd, szTemp, 32);

		if (!mir_wstrcmp(szTemp, L"MirandaThumbsWnd")) { // hide floating contacts
			CallService("FloatingContacts/MainHideAllThumbs", 0, 0);
			g_bOldSetting |= OLD_FLTCONT;
		}
		else if (!mir_wstrcmp(szTemp, L"PopupWnd2")) // destroy opened popups
			PUDeletePopup(hWnd);
		else {
			uint32_t threadId = GetWindowThreadProcessId(hWnd, 0);
			if (threadId != GetCurrentThreadId())
				return false;

			// add to list
			HWND_ITEM *node = new HWND_ITEM;
			node->hWnd = hWnd;
			node->next = g_pMirWnds;
			g_pMirWnds = node;
			ShowWindow(hWnd, SW_HIDE);
		}
	}
	return true;
}

wchar_t* GetDefStatusMsg(unsigned uStatus, const char* szProto)
{
	return (wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, uStatus, (LPARAM)szProto);
}

void SetStatus(const char* szProto, unsigned status, wchar_t *tszAwayMsg)
{
	if (tszAwayMsg && CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND)
		CallProtoService(szProto, PS_SETAWAYMSG, status, (LPARAM)tszAwayMsg);

	CallProtoService(szProto, PS_SETSTATUS, status, 0);
}

static int ChangeAllProtoStatuses(unsigned statusMode, wchar_t *msg)
{
	for (int i = 0; i < protoCount; i++) {
		unsigned status = Proto_GetStatus(proto[i]->szModuleName);
		if (
			(g_wMask & OPT_ONLINEONLY) ? // check "Change only if current status is Online" option
			((status == ID_STATUS_ONLINE) || (status == ID_STATUS_FREECHAT)) // process only "online" and "free for chat"
			: ((status > ID_STATUS_OFFLINE) && (status < ID_STATUS_IDLE) && (status != ID_STATUS_INVISIBLE))) // process all existing statuses except for "invisible" & "offline"
		{
			if (g_wMask & OPT_SETONLINEBACK) { // need to save old statuses & status messages
				oldStatus[i] = status;
				if (ProtoServiceExists(proto[i]->szModuleName, PS_GETMYAWAYMSG))
					oldStatusMsg[i] = (wchar_t*)CallProtoService(proto[i]->szModuleName, PS_GETMYAWAYMSG, 0, SGMA_UNICODE);
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
	for (int i = 0; i < protoCount; i++) {
		if (oldStatus[i]) {
			SetStatus(proto[i]->szModuleName, oldStatus[i], oldStatusMsg[i]);
			if (oldStatusMsg[i]) {
				mir_free(oldStatusMsg[i]);
				oldStatusMsg[i] = nullptr;
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
	if (!g_plugin.getWString("ToolTipText", &dbVar)) {
		wcsncpy_s(nim.szTip, dbVar.pwszVal, _TRUNCATE);
		db_free(&dbVar);
	}
	else wcsncpy_s(nim.szTip, L"Miranda NG", _TRUNCATE);

	nim.cbSize = sizeof(nim);
	nim.hWnd = g_hListenWindow;
	nim.uID = 100;
	nim.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nim.hIcon = IcoLib_GetIcon("hidemim");
	nim.uCallbackMessage = WM_USER + 24;
	Shell_NotifyIcon(create ? NIM_ADD : NIM_DELETE, &nim);
	g_TrayIcon = create;
}

static void RestoreOldSettings(void)
{
	if (g_bOldSetting & OLD_POPUP)
		Popup_Enable(true);

	if (g_bOldSetting & OLD_SOUND)
		db_set_b(0, "Skin", "UseSound", 1);

	if (g_bOldSetting & OLD_FLTCONT) // show Floating contacts if needed
	{
		if (ServiceExists("FloatingContacts/MainHideAllThumbs"))
			CallService("FloatingContacts/MainHideAllThumbs", 0, 0);
		else
			db_set_b(0, "FloatingContacts", "HideAll", 0);
	}
	g_bOldSetting = 0;
}

LRESULT CALLBACK ListenWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_WTSSESSION_CHANGE:
		if (wParam == WTS_SESSION_LOCK && g_wMaskAdv & OPT_HIDEIFLOCK && !g_bWindowHidden) // Windows locked
			PostMessage(hWnd, WM_USER + 40, 0, 0);
		return 0;

	case WM_USER + 24:
		if (lParam == WM_LBUTTONDBLCLK)
			PostMessage(hWnd, WM_USER + 52, 0, 0);
		return 0;

	case WM_USER + 40: // hide
		{
			if (g_bWindowHidden || g_fOptionsOpen) // already hidden or in options, no hiding
				break;

			DWORD dwWndPID; // remember foreground window
			HWND hForegroundWnd = GetForegroundWindow();
			GetWindowThreadProcessId(hForegroundWnd, &dwWndPID);
			if (g_dwMirandaPID == dwWndPID)
				hOldForegroundWindow = hForegroundWnd;

			EnumWindows(EnumWindowsProc, 0);

			if (g_wMask & OPT_CHANGESTATUS) // is this even needed?
			{
				uint8_t bReqMode = g_plugin.getByte("stattype", 2);
				unsigned uMode = (STATUS_ARR_TO_ID[bReqMode]);
				DBVARIANT dbVar;
				if (g_wMask & OPT_USEDEFMSG || g_plugin.getWString("statmsg", &dbVar)) {
					wchar_t *ptszDefMsg = GetDefStatusMsg(uMode, nullptr);
					ChangeAllProtoStatuses(uMode, ptszDefMsg);
					mir_free(ptszDefMsg);
				}
				else {
					if (ServiceExists(MS_VARS_FORMATSTRING)) {
						wchar_t *ptszParsed = variables_parse(dbVar.pwszVal, nullptr, 0);
						ChangeAllProtoStatuses(uMode, ptszParsed);
						mir_free(ptszParsed);
					}
					else ChangeAllProtoStatuses(uMode, dbVar.pwszVal);

					db_free(&dbVar);
				}
			}

			Clist_TrayIconDestroy(g_clistApi.hwndContactList);

			if (g_wMask & OPT_TRAYICON)
				CreateTrayIcon(true);

			// disable popups
			if (Popup_Enabled()) {
				// save current
				g_bOldSetting |= OLD_POPUP;
				Popup_Enable(false);
			}

			// disable sounds
			if ((g_wMask & OPT_DISABLESNDS) && db_get_b(0, "Skin", "UseSound", 1)) {
				// save current
				g_bOldSetting |= OLD_SOUND;
				db_set_b(0, "Skin", "UseSound", 0);
			}

			g_bWindowHidden = true;

			g_bOldSetting |= OLD_WASHIDDEN;
			g_plugin.setByte("OldSetting", g_bOldSetting);
		}
		return 0;

	case WM_USER + 52: // back
		{
			if (!g_bWindowHidden || g_fPassRequested)
				break;

			if (g_wMask & OPT_REQPASS) {  //password request
				DBVARIANT dbVar;
				if (!g_plugin.getString("password", &dbVar)) {
					g_fPassRequested = true;

					strncpy(g_password, dbVar.pszVal, MAXPASSLEN);
					db_free(&dbVar);

					int res = DialogBox(g_plugin.getInst(), (MAKEINTRESOURCE(IDD_PASSDIALOGNEW)), GetForegroundWindow(), DlgStdInProc);

					g_fPassRequested = false;
					if (res != IDOK) return 0;
				}
			}

			if (g_wMask & OPT_CHANGESTATUS && g_wMask & OPT_SETONLINEBACK) // set back to some status
				BackAllProtoStatuses();

			HWND_ITEM *pCurWnd = g_pMirWnds;
			while (pCurWnd != nullptr) {
				HWND_ITEM *pNextWnd = pCurWnd->next;
				wchar_t szTemp[32];
				GetClassName(pCurWnd->hWnd, szTemp, 32);

				if (IsWindow(pCurWnd->hWnd) && mir_wstrcmp(szTemp, L"SysShadow") != 0) // precaution
					ShowWindow(pCurWnd->hWnd, SW_SHOW);

				delete pCurWnd; // bye-bye
				pCurWnd = pNextWnd; // traverse to next item
			}
			g_pMirWnds = nullptr;

			if (hOldForegroundWindow) {
				SetForegroundWindow(hOldForegroundWindow);
				hOldForegroundWindow = nullptr;
			}

			RestoreOldSettings();

			if (g_TrayIcon) CreateTrayIcon(false);

			g_clistApi.pfnTrayIconInit(g_clistApi.hwndContactList);

			// force a redraw
			// should prevent drawing problems
			InvalidateRect(g_clistApi.hwndContactList, nullptr, true);
			UpdateWindow(g_clistApi.hwndContactList);

			PostMessage(hWnd, WM_MOUSEMOVE, 0, (LPARAM)MAKELONG(2, 2)); // reset core's IDLE
			g_bWindowHidden = false;

			g_plugin.setByte("OldSetting", 0);
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

VOID CALLBACK WinEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG idObject, LONG, DWORD, DWORD)
{
	if (g_bWindowHidden && idObject == OBJID_WINDOW && (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) && (IsWindowVisible(hwnd))) {
		if (hwnd == g_clistApi.hwndContactList)
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

static wchar_t* HotkeyVkToName(uint16_t vkKey)
{
	static wchar_t buf[32] = { 0 };
	uint32_t code = MapVirtualKey(vkKey, 0) << 16;

	switch (vkKey) {
	case 0:
	case VK_CONTROL:
	case VK_SHIFT:
	case VK_MENU:
	case VK_LWIN:
	case VK_RWIN:
	case VK_PAUSE:
	case VK_CANCEL:
	case VK_CAPITAL:
		return L"";

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

	GetKeyNameText(code, buf, _countof(buf));
	return buf;
}

static wchar_t* GetBossKeyText(void)
{
	uint16_t wHotKey = db_get_w(0, "SkinHotKeys", "Hide/Show Miranda", HOTKEYCODE(HOTKEYF_CONTROL, VK_F12));

	uint8_t key = LOBYTE(wHotKey);
	uint8_t shift = HIBYTE(wHotKey);

	static wchar_t buf[128];
	mir_snwprintf(buf, L"%s%s%s%s%s",
		(shift & HOTKEYF_CONTROL) ? L"Ctrl + " : L"",
		(shift & HOTKEYF_SHIFT) ? L"Shift + " : L"",
		(shift & HOTKEYF_ALT) ? L"Alt + " : L"",
		(shift & HOTKEYF_EXT) ? L"Win + " : L"",
		HotkeyVkToName(key));

	return buf;
}

static IconItem iconList[] =
{
	{ LPGEN("Hide Miranda NG"), "hidemim", IDI_DLGPASSWD }
};

static int GenMenuInit(WPARAM, LPARAM) // Modify menu item text before to show the main menu
{
	wchar_t buf[128];
	mir_snwprintf(buf, L"%s [%s]", TranslateT("Hide"), GetBossKeyText());
	Menu_ModifyItem(g_hMenuItem, buf);
	return 0;
}

static void BossKeyMenuItemInit(void) // Add menu item
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x42428114, 0xfac7, 0x44c2, 0x9a, 0x11, 0x18, 0xbe, 0x81, 0xd4, 0xa9, 0xe3);
	mi.flags = CMIF_UNICODE;
	mi.position = 2000100000;
	mi.hIcolibItem = IcoLib_GetIcon("hidemim");
	mi.name.w = LPGENW("Hide");
	mi.pszService = MS_BOSSKEY_HIDE;
	g_hMenuItem = Menu_AddMainMenuItem(&mi);
	Menu_ConfigureItem(g_hMenuItem, MCI_OPT_DISABLED, TRUE);

	HookEvent(ME_CLIST_PREBUILDMAINMENU, GenMenuInit);
}

static void RegisterCoreHotKeys(void)
{
	HOTKEYDESC hotkey = {};
	hotkey.pszName = "Hide/Show Miranda";
	hotkey.szDescription.a = LPGEN("Hide/Show Miranda");
	hotkey.szSection.a = "BossKey";
	hotkey.pszService = MS_BOSSKEY_HIDE;
	hotkey.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F12);

	g_plugin.addHotkey(&hotkey);
}

static int TopToolbarInit(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszService = MS_BOSSKEY_HIDE;
	ttb.pszTooltipUp = ttb.name = LPGEN("Hide Miranda NG");
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.hIconHandleUp = iconList[0].hIcolib;
	g_plugin.addTTB(&ttb);
	return 0;
}

// Tabsrmm toolbar support
static int TabsrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	if (!mir_strcmp(cbcd->pszModule, MODULENAME))
		BossKeyHideMiranda(0, 0);

	return 0;
}

static int TabsrmmButtonsInit(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.dwDefPos = 5000;
	bbd.pwszTooltip = LPGENW("Hide Miranda NG");
	bbd.bbbFlags = BBBF_ISRSIDEBUTTON | BBBF_CANBEHIDDEN;
	bbd.hIcon = iconList[0].hIcolib;
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

static wchar_t* VariablesBossKey(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return nullptr;
	if (ai->argc != 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	return GetBossKeyText();
}

static int EnumProtos(WPARAM, LPARAM)
{
	Proto_EnumAccounts(&protoCount, &proto);

	delete[] oldStatus;
	delete[] oldStatusMsg;

	oldStatus = new unsigned[protoCount];
	oldStatusMsg = new wchar_t*[protoCount];
	for (int i = 0; i < protoCount; i++) {
		oldStatus[i] = 0;
		oldStatusMsg[i] = nullptr;
	}
	return 0;
}

static int MirandaLoaded(WPARAM, LPARAM)
{
	g_wMask = g_plugin.getWord("optsmask", DEFAULTSETTING);

	RegisterCoreHotKeys();

	g_hWinHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_SHOW, nullptr, WinEventProc, GetCurrentProcessId(), 0, 0);

	HookEvent(ME_TTB_MODULELOADED, TopToolbarInit);
	HookEvent(ME_MSG_WINDOWEVENT, MsgWinOpening);
	HookEvent(ME_PROTO_ACCLISTCHANGED, EnumProtos);
	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonsInit);

	GetWindowThreadProcessId(g_clistApi.hwndContactList, &g_dwMirandaPID);

	WNDCLASS winclass = { 0 };
	winclass.lpfnWndProc = ListenWndProc;
	winclass.hInstance = g_plugin.getInst();
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszClassName = BOSSKEY_LISTEN_INFO;

	if (RegisterClass(&winclass)) {
		g_hListenWindow = CreateWindow(BOSSKEY_LISTEN_INFO, BOSSKEY_LISTEN_INFO, WS_POPUP, 0, 0, 5, 5, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);
		WTSRegisterSessionNotification(g_hListenWindow, 0);
	}

	if (IsWinVerVistaPlus()) {
		hDwmApi = LoadLibrary(L"dwmapi.dll");
		if (hDwmApi)
			dwmIsCompositionEnabled = (PFNDwmIsCompositionEnabled)GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
	}

	BossKeyMenuItemInit();

	// Register token for variables plugin
	if (ServiceExists(MS_VARS_REGISTERTOKEN)) {
		TOKENREGISTER tr = { 0 };
		tr.cbSize = sizeof(TOKENREGISTER);
		tr.memType = TR_MEM_OWNER;
		tr.flags = TRF_FIELD | TRF_TCHAR | TRF_PARSEFUNC;

		tr.szTokenString.w = L"bosskeyname";
		tr.parseFunctionW = VariablesBossKey;
		tr.szHelpText = LPGEN("BossKey") "\t" LPGEN("get the BossKey name");
		CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&tr);
	}

	EnumProtos(0, 0);

	if (g_bOldSetting && !(g_wMaskAdv & OPT_RESTORE)) // Restore settings if Miranda was crushed or killed in hidden mode and "Restore hiding on startup after failure" option is disabled
		RestoreOldSettings();

	if ((g_wMaskAdv & OPT_HIDEONSTART) || (g_wMaskAdv & OPT_RESTORE && g_bOldSetting))
		BossKeyHideMiranda(0, 0);

	g_plugin.impl.m_timer.Start(2000);
	return 0;
}

int CMPlugin::Load()
{
	g_wMaskAdv = g_plugin.getWord("optsmaskadv", 0);
	g_bOldSetting = g_plugin.getByte("OldSetting", 0);

	if ((g_bOldSetting & OLD_POPUP) && !(g_wMaskAdv & OPT_RESTORE)) // Restore popup settings if Miranda was crushed or killed in hidden mode and "Restore hiding on startup after failure" option is disabled
	{
		if (Popup_Enabled() == 0)
			Popup_Enable(true);
	}
	if (g_wMaskAdv & OPT_HIDEONSTART && Popup_Enabled()) // hack for disabling popup on startup if "Hide Miranda on startup" is enabled
	{
		g_bOldSetting |= OLD_POPUP;
		Popup_Enable(false);
	}

	g_plugin.registerIcon("BossKey", iconList);

	CreateServiceFunction(MS_BOSSKEY_HIDE, BossKeyHideMiranda); // Create service

	HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded);
	HookEvent(ME_OPT_INITIALISE, OptsDlgInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	impl.m_timer.Stop();

	if (g_hWinHook != nullptr)
		UnhookWinEvent(g_hWinHook);

	if (g_hListenWindow) {
		WTSUnRegisterSessionNotification(g_hListenWindow);
		DestroyWindow(g_hListenWindow);
	}

	if (hDwmApi)
		FreeLibrary(hDwmApi);

	// free all sessions
	HWND_ITEM *pTemp = g_pMirWnds;
	while (pTemp != nullptr) {
		HWND_ITEM *pNext = pTemp->next;
		delete pTemp;
		pTemp = pNext;
	}

	delete[] oldStatus;
	delete[] oldStatusMsg;

	return 0;
}
