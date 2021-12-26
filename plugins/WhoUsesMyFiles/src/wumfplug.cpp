#include "stdafx.h"

WUMF_OPTIONS WumfOptions = { 0 };
HGENMENU hMenuItem = nullptr;
HWND hDlg;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {80DCA515-973A-4A7E-8B85-5D8EC88FC5A7}
	{0x80dca515, 0x973a, 0x4a7e, {0x8b, 0x85, 0x5d, 0x8e, 0xc8, 0x8f, 0xc5, 0xa7}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int OptionsInit(WPARAM, LPARAM);

void LoadOptions()
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_WCHAR;
	memset(&WumfOptions, 0, sizeof(WumfOptions));
	if (g_plugin.getWString(OPT_FILE, &dbv) == 0) {
		wcsncpy(WumfOptions.LogFile, dbv.pwszVal, 255);
		db_free(&dbv);
	}
	else
		WumfOptions.LogFile[0] = '\0';

	WumfOptions.PopupsEnabled = g_plugin.getByte(POPUPS_ENABLED, TRUE);

	WumfOptions.UseDefColor = g_plugin.getByte(COLOR_DEF, TRUE);
	WumfOptions.UseWinColor = g_plugin.getByte(COLOR_WIN, FALSE);
	WumfOptions.SelectColor = g_plugin.getByte(COLOR_SET, FALSE);

	WumfOptions.ColorText = g_plugin.getDword(COLOR_TEXT, RGB(0, 0, 0));
	WumfOptions.ColorBack = g_plugin.getDword(COLOR_BACK, RGB(255, 255, 255));

	WumfOptions.DelayDef = g_plugin.getByte(DELAY_DEF, TRUE);
	WumfOptions.DelayInf = g_plugin.getByte(DELAY_INF, FALSE);
	WumfOptions.DelaySet = g_plugin.getByte(DELAY_SET, FALSE);
	WumfOptions.DelaySec = g_plugin.getByte(DELAY_SEC, 0);
	WumfOptions.LogToFile = g_plugin.getByte(LOG_INTO_FILE, FALSE);
	WumfOptions.LogFolders = g_plugin.getByte(LOG_FOLDER, TRUE);
	WumfOptions.AlertFolders = g_plugin.getByte(ALERT_FOLDER, TRUE);
}

void ExecuteMenu(HWND hWnd)
{
	HMENU hMenu = CreatePopupMenu();
	if (!hMenu) {
		msg(TranslateT("Error creating menu"));
		return;
	}
	AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About\0");
	AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);//------------------
	AppendMenu(hMenu, MF_STRING, IDM_SHOW, L"Show connections\0");
	AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);//------------------
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Dismiss popup\0");

	POINT point;
	GetCursorPos(&point);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_TOPALIGN, point.x, point.y, 0, hWnd, nullptr);
	PostMessage(hWnd, WM_USER, 0, 0);
	DestroyMenu(hMenu);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
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
	wchar_t text[512], title[512];

	if (!WumfOptions.AlertFolders && (w->dwAttr & FILE_ATTRIBUTE_DIRECTORY)) return;
	mir_snwprintf(title, L"%s (%s)", w->szComp, w->szUser);
	mir_snwprintf(text, L"%s (%s)", w->szPath, w->szPerm);
	ShowThePopup(w, title, text);
}

void ShowThePopup(PWumf w, LPTSTR title, LPTSTR text)
{
	POPUPDATAW ppd;
	ppd.lchContact = NULL;
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DRIVE));

	if (WumfOptions.DelayInf)
		ppd.iSeconds = -1;
	else if (WumfOptions.DelayDef)
		ppd.iSeconds = 0;
	else if (WumfOptions.DelaySet)
		ppd.iSeconds = WumfOptions.DelaySec;

	mir_wstrncpy(ppd.lpwzContactName, title, MAX_CONTACTNAME);
	mir_wstrncpy(ppd.lpwzText, text, MAX_SECONDLINE);
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
	PUAddPopupW(&ppd);
}

void ThreadProc(LPVOID)
{
	if (hDlg) {
		ShowWindow(hDlg, SW_SHOWNORMAL);
		SetForegroundWindow(hDlg);
		return;
	}

	hDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONNLIST), nullptr, ConnDlgProc);
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DRIVE)));
	ShowWindow(hDlg, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) == TRUE) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	hDlg = nullptr;
}

static INT_PTR WumfShowConnections(WPARAM, LPARAM)
{
	mir_forkthread(ThreadProc);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hWumfBut, 0);
	return 0;
}

static INT_PTR WumfMenuCommand(WPARAM, LPARAM)
{
	if (WumfOptions.PopupsEnabled == TRUE) {
		WumfOptions.PopupsEnabled = FALSE;
		Menu_ModifyItem(hMenuItem, LPGENW("Enable WUMF popups"), Skin_GetIconHandle(SKINICON_OTHER_NOPOPUP));
	}
	else {
		WumfOptions.PopupsEnabled = TRUE;
		Menu_ModifyItem(hMenuItem, LPGENW("Disable WUMF popups"), Skin_GetIconHandle(SKINICON_OTHER_POPUP));
	}

	g_plugin.setByte(POPUPS_ENABLED, (uint8_t)WumfOptions.PopupsEnabled);
	return 0;
}

int InitTopToolbar(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.hIconUp = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DRIVE));
	ttb.pszService = MS_WUMF_CONNECTIONSSHOW;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.name = ttb.pszTooltipUp = LPGEN("Show connections list");
	hWumfBut = g_plugin.addTTB(&ttb);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	LoadOptions();

	CreateServiceFunction(MS_WUMF_SWITCHPOPUP, WumfMenuCommand);
	CreateServiceFunction(MS_WUMF_CONNECTIONSSHOW, WumfShowConnections);

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xcfce6487, 0x907b, 0x4822, 0xb0, 0x49, 0x18, 0x4e, 0x47, 0x17, 0x0, 0x69);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Popups"), 1999990000);
	if (WumfOptions.PopupsEnabled == FALSE) {
		mi.name.a = LPGEN("Enable WUMF popups");
		mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_NOPOPUP);
	}
	else {
		mi.name.a = LPGEN("Disable WUMF popups");
		mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	}
	mi.pszService = MS_WUMF_SWITCHPOPUP;
	hMenuItem = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xbf93984c, 0xaa05, 0x447c, 0xbd, 0x5c, 0x5f, 0x43, 0x60, 0x92, 0x6a, 0x12);
	mi.name.a = LPGEN("WUMF: Show connections");
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DRIVE));
	mi.pszService = MS_WUMF_CONNECTIONSSHOW;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbar);

	if (IsUserAnAdmin())
		SetTimer(nullptr, 777, TIME, TimerProc);
	else
		MessageBox(nullptr, TranslateT("Plugin WhoUsesMyFiles requires admin privileges in order to work."), L"Miranda NG", MB_OK);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	KillTimer(nullptr, 777);
	if (hLogger)
		mir_closeLog(hLogger);
	FreeAll();
	return 0;
}
