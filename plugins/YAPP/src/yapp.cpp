// popups2.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

HMODULE hInst = 0;
bool bShutdown = false;

MNOTIFYLINK *notifyLink = 0;

// used to work around a bug in neweventnotify and others with the address passed in the GetPluginData function
bool ignore_gpd_passed_addy = false;

FontIDT font_id_firstline = {0}, font_id_secondline = {0}, font_id_time = {0};
ColourIDT colour_id_bg = {0}, colour_id_border = {0}, colour_id_sidebar = {0}, colour_id_titleunderline = {0};

COLORREF colBg = GetSysColor(COLOR_3DSHADOW);
HFONT hFontFirstLine = 0, hFontSecondLine = 0, hFontTime = 0;
COLORREF colFirstLine = RGB(255, 0, 0), colSecondLine = 0, colTime = RGB(0, 0, 255), colBorder = RGB(0, 0, 0), 
	colSidebar = RGB(128, 128, 128), colTitleUnderline = GetSysColor(COLOR_3DSHADOW);

int hLangpack;
CLIST_INTERFACE *pcli;

// toptoolbar button
HANDLE hTTButton;

// menu items
HGENMENU hMenuRoot, hMenuItem, hMenuItemHistory;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {EFD15F16-7AE4-40D7-A8E3-A411ED747BD5}
	{0xefd15f16, 0x7ae4, 0x40d7, {0xa8, 0xe3, 0xa4, 0x11, 0xed, 0x74, 0x7b, 0xd5}}
};

BOOL WINAPI DllMain(HMODULE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

int ReloadFont(WPARAM, LPARAM) 
{
	LOGFONT log_font;
	if (hFontFirstLine) DeleteObject(hFontFirstLine);
	colFirstLine = CallService(MS_FONT_GETT, (WPARAM)&font_id_firstline, (LPARAM)&log_font);
	hFontFirstLine = CreateFontIndirect(&log_font);
	if (hFontSecondLine) DeleteObject(hFontSecondLine);
	colSecondLine = CallService(MS_FONT_GETT, (WPARAM)&font_id_secondline, (LPARAM)&log_font);
	hFontSecondLine = CreateFontIndirect(&log_font);
	if (hFontTime) DeleteObject(hFontTime);
	colTime = CallService(MS_FONT_GETT, (WPARAM)&font_id_time, (LPARAM)&log_font);
	hFontTime = CreateFontIndirect(&log_font);

	colBg = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_bg, 0);
	colBorder = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_border, 0);
	colSidebar = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_sidebar, 0);
	colTitleUnderline = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_titleunderline, 0);
	return 0;
}

int TTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { 0 };
	ttb.pszService = "Popup/EnableDisableMenuCommand";
	ttb.lParamUp = 1;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	if (db_get_b(0, "Popup", "ModuleIsEnabled", 1))
		ttb.dwFlags |= TTBBF_PUSHED;
	ttb.name = LPGEN("Toggle Popups");
	ttb.hIconHandleUp = IcoLib_GetIconHandle(ICO_TB_POPUP_OFF);
	ttb.hIconHandleDn = IcoLib_GetIconHandle(ICO_TB_POPUP_ON);
	ttb.pszTooltipUp = LPGEN("Enable Popups");
	ttb.pszTooltipDn = LPGEN("Disable Popups");
	hTTButton = TopToolbar_AddButton(&ttb);
	return 0;
}

static void InitFonts()
{
	font_id_firstline.cbSize = sizeof(FontIDT);
	font_id_firstline.flags = FIDF_ALLOWEFFECTS;
	mir_tstrcpy(font_id_firstline.group, LPGENT("Popups"));
	mir_tstrcpy(font_id_firstline.name, LPGENT("First line"));
	mir_strcpy(font_id_firstline.dbSettingsGroup, MODULE);
	mir_strcpy(font_id_firstline.prefix, "FontFirst");
	mir_tstrcpy(font_id_firstline.backgroundGroup, _T("Popups"));
	mir_tstrcpy(font_id_firstline.backgroundName, _T("Background"));
	font_id_firstline.order = 0;
	FontRegisterT(&font_id_firstline);

	font_id_secondline.cbSize = sizeof(FontIDT);
	font_id_secondline.flags = FIDF_ALLOWEFFECTS;
	mir_tstrcpy(font_id_secondline.group, LPGENT("Popups"));
	mir_tstrcpy(font_id_secondline.name, LPGENT("Second line"));
	mir_strcpy(font_id_secondline.dbSettingsGroup, MODULE);
	mir_strcpy(font_id_secondline.prefix, "FontSecond");
	mir_tstrcpy(font_id_secondline.backgroundGroup, _T("Popups"));
	mir_tstrcpy(font_id_secondline.backgroundName, _T("Background"));
	font_id_secondline.order = 1;
	FontRegisterT(&font_id_secondline);

	font_id_time.cbSize = sizeof(FontIDT);
	font_id_time.flags = FIDF_ALLOWEFFECTS;
	mir_tstrcpy(font_id_time.group, LPGENT("Popups"));
	mir_tstrcpy(font_id_time.name, LPGENT("Time"));
	mir_strcpy(font_id_time.dbSettingsGroup, MODULE);
	mir_strcpy(font_id_time.prefix, "FontTime");
	mir_tstrcpy(font_id_time.backgroundGroup, _T("Popups"));
	mir_tstrcpy(font_id_time.backgroundName, _T("Background"));
	font_id_time.order = 2;
	FontRegisterT(&font_id_time);

	colour_id_bg.cbSize = sizeof(ColourIDT);
	mir_tstrcpy(colour_id_bg.group, LPGENT("Popups"));
	mir_tstrcpy(colour_id_bg.name, LPGENT("Background"));
	mir_strcpy(colour_id_bg.dbSettingsGroup, MODULE);
	mir_strcpy(colour_id_bg.setting, "ColourBg");
	colour_id_bg.defcolour = GetSysColor(COLOR_3DSHADOW);
	colour_id_bg.order = 0;
	ColourRegisterT(&colour_id_bg);

	colour_id_border.cbSize = sizeof(ColourIDT);
	mir_tstrcpy(colour_id_border.group, LPGENT("Popups"));
	mir_tstrcpy(colour_id_border.name, LPGENT("Border"));
	mir_strcpy(colour_id_border.dbSettingsGroup, MODULE);
	mir_strcpy(colour_id_border.setting, "ColourBorder");
	colour_id_border.defcolour = RGB(0, 0, 0);
	colour_id_border.order = 1;
	ColourRegisterT(&colour_id_border);

	colour_id_sidebar.cbSize = sizeof(ColourIDT);
	mir_tstrcpy(colour_id_sidebar.group, LPGENT("Popups"));
	mir_tstrcpy(colour_id_sidebar.name, LPGENT("Sidebar"));
	mir_strcpy(colour_id_sidebar.dbSettingsGroup, MODULE);
	mir_strcpy(colour_id_sidebar.setting, "ColourSidebar");
	colour_id_sidebar.defcolour = RGB(128, 128, 128);
	colour_id_sidebar.order = 2;
	ColourRegisterT(&colour_id_sidebar);

	colour_id_titleunderline.cbSize = sizeof(ColourIDT);
	mir_tstrcpy(colour_id_titleunderline.group, LPGENT("Popups"));
	mir_tstrcpy(colour_id_titleunderline.name, LPGENT("Title underline"));
	mir_strcpy(colour_id_titleunderline.dbSettingsGroup, MODULE);
	mir_strcpy(colour_id_titleunderline.setting, "ColourTitleUnderline");
	colour_id_titleunderline.defcolour = GetSysColor(COLOR_3DSHADOW);
	colour_id_titleunderline.order = 3;
	ColourRegisterT(&colour_id_titleunderline);

	ReloadFont(0, 0);
}

void InitMenuItems(void)
{
	bool isEnabled = db_get_b(0, "Popup", "ModuleIsEnabled", 1) == 1;

	hMenuRoot = Menu_CreateRoot(MO_MAIN, LPGENT("Popups"), 500010000, IcoLib_GetIcon(isEnabled ? ICO_POPUP_ON : ICO_POPUP_OFF, 0));
	Menu_ConfigureItem(hMenuRoot, MCI_OPT_UID, "043A641A-2767-4C57-AA57-9233D6F9DC54");

	CMenuItem mi;
	mi.flags = CMIF_TCHAR;
	mi.root = hMenuRoot;

	SET_UID(mi, 0x92c386ae, 0x6e81, 0x452d, 0xb5, 0x71, 0x87, 0x46, 0xe9, 0x2, 0x66, 0xe9);
	mi.hIcolibItem = IcoLib_GetIcon(ICO_HISTORY, 0);
	mi.pszService= MS_POPUP_SHOWHISTORY;
	mi.name.t = LPGENT("Popup History");
	hMenuItemHistory = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x4353d44e, 0x177, 0x4843, 0x88, 0x30, 0x25, 0x5d, 0x91, 0xad, 0xdf, 0x3f);
	mi.hIcolibItem = IcoLib_GetIcon(isEnabled ? ICO_POPUP_ON : ICO_POPUP_OFF, 0);
	mi.pszService = "Popup/EnableDisableMenuCommand";
	mi.name.t = (isEnabled ? LPGENT("Disable Popups") : LPGENT("Enable Popups"));
	hMenuItem = Menu_AddMainMenuItem(&mi);
}

int ModulesLoaded(WPARAM, LPARAM)
{
	MNotifyGetLink();

	if (ServiceExists(MS_HPP_EG_WINDOW))
		lstPopupHistory.SetRenderer(RENDER_HISTORYPP);

	HookEvent(ME_FONT_RELOAD, ReloadFont);
	HookEvent(ME_TTB_MODULELOADED, TTBLoaded);

	LoadModuleDependentOptions(); 

	if (GetModuleHandle(_T("neweventnotify")))
		ignore_gpd_passed_addy = true;

	return 0;
}

int PreShutdown(WPARAM, LPARAM)
{
	bShutdown = true;
	DeinitMessagePump();
	DeinitNotify();
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	InitMessagePump();
	InitOptions();
	InitNotify();
	InitFonts();
	InitIcons();
	InitMenuItems();

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	DeinitNotify();
	DeleteObject(hFontFirstLine);
	DeleteObject(hFontSecondLine);
	DeleteObject(hFontTime);
	return 0;
}
