// popups2.cpp : Defines the entry point for the DLL application.
//

#include "common.h"
#include "popups2.h"
#include "version.h"
#include "message_pump.h"
#include "options.h"
#include "popwin.h"
#include "notify.h"

#include "popup_history.h" //to be able to update the renderer

HMODULE hInst = 0;

MNOTIFYLINK *notifyLink = 0;

// used to work around a bug in neweventnotify and others with the address passed in the GetPluginData function
bool ignore_gpd_passed_addy = false;

FontID font_id_firstline = {0}, font_id_secondline = {0}, font_id_time = {0};
ColourID colour_id_bg = {0}, colour_id_border = {0}, colour_id_sidebar = {0}, colour_id_titleunderline = {0};
FontIDW font_id_firstlinew = {0}, font_id_secondlinew = {0}, font_id_timew = {0};
ColourIDW colour_id_bgw = {0}, colour_id_borderw = {0}, colour_id_sidebarw = {0}, colour_id_titleunderlinew = {0};

COLORREF colBg = GetSysColor(COLOR_3DSHADOW);
HFONT hFontFirstLine = 0, hFontSecondLine = 0, hFontTime = 0;
COLORREF colFirstLine = RGB(255, 0, 0), colSecondLine = 0, colTime = RGB(0, 0, 255), colBorder = RGB(0, 0, 0), 
	colSidebar = RGB(128, 128, 128), colTitleUnderline = GetSysColor(COLOR_3DSHADOW);
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,		//not transient
	0,					//doesn't replace anything built-in
	{ 0xefd15f16, 0x7ae4, 0x40d7, { 0xa8, 0xe3, 0xa4, 0x11, 0xed, 0x74, 0x7b, 0xd5 } } // {EFD15F16-7AE4-40d7-A8E3-A411ED747BD5}
};

PLUGINLINK *pluginLink;


extern "C" BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hInst = hModule;
	DisableThreadLibraryCalls(hInst);
    return TRUE;
}

extern "C" POPUPS2_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_POPUPS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


int ReloadFont(WPARAM wParam, LPARAM lParam) 
{

	if(ServiceExists(MS_FONT_GETW)) {
		LOGFONTW log_font;
		if(hFontFirstLine) DeleteObject(hFontFirstLine);
		colFirstLine = CallService(MS_FONT_GETW, (WPARAM)&font_id_firstlinew, (LPARAM)&log_font);
		hFontFirstLine = CreateFontIndirectW(&log_font);
		if(hFontSecondLine) DeleteObject(hFontSecondLine);
		colSecondLine = CallService(MS_FONT_GETW, (WPARAM)&font_id_secondlinew, (LPARAM)&log_font);
		hFontSecondLine = CreateFontIndirectW(&log_font);
		if(hFontTime) DeleteObject(hFontTime);
		colTime = CallService(MS_FONT_GETW, (WPARAM)&font_id_timew, (LPARAM)&log_font);
		hFontTime = CreateFontIndirectW(&log_font);

		colBg = CallService(MS_COLOUR_GETW, (WPARAM)&colour_id_bgw, 0);
		colBorder = CallService(MS_COLOUR_GETW, (WPARAM)&colour_id_borderw, 0);
		colSidebar = CallService(MS_COLOUR_GETW, (WPARAM)&colour_id_sidebarw, 0);
		colTitleUnderline = CallService(MS_COLOUR_GETW, (WPARAM)&colour_id_titleunderlinew, 0);
	} else

	{
		LOGFONTA log_font;
		if(hFontFirstLine) DeleteObject(hFontFirstLine);
		colFirstLine = CallService(MS_FONT_GET, (WPARAM)&font_id_firstline, (LPARAM)&log_font);
		hFontFirstLine = CreateFontIndirectA(&log_font);
		if(hFontSecondLine) DeleteObject(hFontSecondLine);
		colSecondLine = CallService(MS_FONT_GET, (WPARAM)&font_id_secondline, (LPARAM)&log_font);
		hFontSecondLine = CreateFontIndirectA(&log_font);
		if(hFontTime) DeleteObject(hFontTime);
		colTime = CallService(MS_FONT_GET, (WPARAM)&font_id_time, (LPARAM)&log_font);
		hFontTime = CreateFontIndirectA(&log_font);

		colBg = CallService(MS_COLOUR_GET, (WPARAM)&colour_id_bg, 0);
		colBorder = CallService(MS_COLOUR_GET, (WPARAM)&colour_id_border, 0);
		colSidebar = CallService(MS_COLOUR_GET, (WPARAM)&colour_id_sidebar, 0);
		colTitleUnderline = CallService(MS_COLOUR_GET, (WPARAM)&colour_id_titleunderline, 0);
	}

	return 0;
}

HANDLE hEventReloadFont = 0;

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	MNotifyGetLink();

	if (ServiceExists(MS_HPP_EG_WINDOW))
		lstPopupHistory.SetRenderer(RENDER_HISTORYPP);

	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/yapp/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
#ifdef _WIN64
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/yapp_x64.zip";
#endif
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_yapp.html";
		update.pbBetaVersionPrefix = (BYTE *)"YAPP version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	font_id_firstlinew.cbSize = sizeof(FontIDW);
	font_id_firstlinew.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_firstlinew.group, _T("Popups"));
	_tcscpy(font_id_firstlinew.name, _T("First line"));
	strcpy(font_id_firstlinew.dbSettingsGroup, MODULE);
	strcpy(font_id_firstlinew.prefix, "FontFirst");
	font_id_firstlinew.order = 0;
	FontRegisterT(&font_id_firstlinew);

	font_id_secondlinew.cbSize = sizeof(FontIDW);
	font_id_secondlinew.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_secondlinew.group, _T("Popups"));
	_tcscpy(font_id_secondlinew.name, _T("Second line"));
	strcpy(font_id_secondlinew.dbSettingsGroup, MODULE);
	strcpy(font_id_secondlinew.prefix, "FontSecond");
	font_id_secondlinew.order = 1;
	FontRegisterT(&font_id_secondlinew);

	font_id_timew.cbSize = sizeof(FontIDW);
	font_id_timew.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_timew.group, _T("Popups"));
	_tcscpy(font_id_timew.name, _T("Time"));
	strcpy(font_id_timew.dbSettingsGroup, MODULE);
	strcpy(font_id_timew.prefix, "FontTime");
	font_id_timew.order = 2;
	FontRegisterT(&font_id_timew);
		
	colour_id_bgw.cbSize = sizeof(ColourIDW);
	_tcscpy(colour_id_bgw.group, _T("Popups"));
	_tcscpy(colour_id_bgw.name, _T("Background"));
	strcpy(colour_id_bgw.dbSettingsGroup, MODULE);
	strcpy(colour_id_bgw.setting, "ColourBg");
	colour_id_bgw.defcolour = GetSysColor(COLOR_3DSHADOW);
	colour_id_bgw.order = 0;
	ColourRegisterT(&colour_id_bgw);

	colour_id_borderw.cbSize = sizeof(ColourIDW);
	_tcscpy(colour_id_borderw.group, _T("Popups"));
	_tcscpy(colour_id_borderw.name, _T("Border"));
	strcpy(colour_id_borderw.dbSettingsGroup, MODULE);
	strcpy(colour_id_borderw.setting, "ColourBorder");
	colour_id_borderw.defcolour = RGB(0, 0, 0);
	colour_id_borderw.order = 1;
	ColourRegisterT(&colour_id_borderw);

	colour_id_sidebarw.cbSize = sizeof(ColourIDW);
	_tcscpy(colour_id_sidebarw.group, _T("Popups"));
	_tcscpy(colour_id_sidebarw.name, _T("Sidebar"));
	strcpy(colour_id_sidebarw.dbSettingsGroup, MODULE);
	strcpy(colour_id_sidebarw.setting, "ColourSidebar");
	colour_id_sidebarw.defcolour = RGB(128, 128, 128);
	colour_id_sidebarw.order = 2;
	ColourRegisterT(&colour_id_sidebarw);

	colour_id_titleunderlinew.cbSize = sizeof(ColourIDW);
	_tcscpy(colour_id_titleunderlinew.group, _T("Popups"));
	_tcscpy(colour_id_titleunderlinew.name, _T("Title underline"));
	strcpy(colour_id_titleunderlinew.dbSettingsGroup, MODULE);
	strcpy(colour_id_titleunderlinew.setting, "ColourTitleUnderline");
	colour_id_titleunderlinew.defcolour = GetSysColor(COLOR_3DSHADOW);
	colour_id_titleunderlinew.order = 3;
	ColourRegisterT(&colour_id_titleunderlinew);
		
	ReloadFont(0, 0);

	hEventReloadFont = HookEvent(ME_FONT_RELOAD, ReloadFont);
	
	LoadModuleDependentOptions(); 

	if(GetModuleHandle(_T("neweventnotify")))
		ignore_gpd_passed_addy = true;

	return 0;
}

int PreShutdown(WPARAM wParam, LPARAM lParam) {
	DeinitMessagePump();
	DeinitNotify();
	return 0;
}

HANDLE hEventPreShutdown, hEventModulesLoaded;

extern "C" int POPUPS2_API Load(PLUGINLINK *link) {
	pluginLink = link;
	mir_getLP(&pluginInfo);

	InitMessagePump();
	InitOptions();
	InitNotify();

	hEventPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	/*
	// test popup classes
	PopupClass test = {0};
	test.cbSize = sizeof(test);
	test.flags = PCF_TCHAR;
	test.hIcon = LoadIcon(0, IDI_WARNING);
	test.colorBack = RGB(0, 0, 0);
	test.colorText = RGB(255, 255, 255);
	test.iSeconds = 10;
	test.ptszDescription = TranslateT("Test popup class");
	test.pszName = "popup/testclass";
	CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&test);
	*/

	return 0;
}

extern "C" int POPUPS2_API Unload()
{
	if(hEventReloadFont)
		UnhookEvent(hEventReloadFont);
	UnhookEvent(hEventPreShutdown);
	UnhookEvent(hEventModulesLoaded);
	DeinitNotify();
	DeleteObject(hFontFirstLine);
	DeleteObject(hFontSecondLine);
	DeleteObject(hFontTime);
	return 0;
}
