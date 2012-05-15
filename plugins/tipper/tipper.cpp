// popups2.cpp : Defines the entry point for the DLL application.
//

#include "common.h"
#include "tipper.h"
#include "version.h"
#include "message_pump.h"
#include "options.h"
#include "popwin.h"
#include "str_utils.h"

HMODULE hInst;

FontIDT font_id_title, font_id_labels, font_id_values;
ColourIDT colour_id_bg, colour_id_border, colour_id_divider, colour_id_sidebar;

HFONT hFontTitle, hFontLabels, hFontValues;

// hooked here so it's in the main thread
HANDLE hAvChangeEvent, hAvContactChangeEvent, hShowTipEvent, hHideTipEvent;
HANDLE	hAckEvent, hFramesSBShow, hFramesSBHide, hSettingChangedEvent;
HANDLE hShowTipService, hShowTipWService, hHideTipService;

MM_INTERFACE mmi;
TIME_API tmi;

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

#ifndef _UNICODE
{ 0xedae7137, 0x1b6a, 0x4b8f, { 0xaa, 0xb0, 0x3b, 0x5a, 0x2f, 0x8, 0xf4, 0x62 } } // {EDAE7137-1B6A-4b8f-AAB0-3B5A2F08F462}
#else
{ 0x785c25e3, 0xc906, 0x434b, { 0x97, 0x23, 0xe2, 0x44, 0xe1, 0xbe, 0xca, 0x2 } } // {785C25E3-C906-434b-9723-E244E1BECA02}
#endif
};

PLUGINLINK *pluginLink;

extern "C" BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hInst = hModule;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}


static const MUUID interfaces[] = {MIID_TOOLTIPS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}



int ReloadFont(WPARAM wParam, LPARAM lParam) {

	LOGFONT log_font;
	if(hFontTitle) DeleteObject(hFontTitle);
	options.title_col = CallService(MS_FONT_GETT, (WPARAM)&font_id_title, (LPARAM)&log_font);
	hFontTitle = CreateFontIndirect(&log_font);
	if(hFontLabels) DeleteObject(hFontLabels);
	options.label_col = CallService(MS_FONT_GETT, (WPARAM)&font_id_labels, (LPARAM)&log_font);
	hFontLabels = CreateFontIndirect(&log_font);
	if(hFontValues) DeleteObject(hFontValues);
	options.value_col = CallService(MS_FONT_GETT, (WPARAM)&font_id_values, (LPARAM)&log_font);
	hFontValues = CreateFontIndirect(&log_font);

	options.bg_col = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_bg, 0);
	options.border_col = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_border, 0);
	options.sidebar_col = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_sidebar, 0);
	options.div_col = CallService(MS_COLOUR_GETT, (WPARAM)&colour_id_divider, 0);

	return 0;
}

// hack to hide tip when clist hides from timeout
int SettingChanged(WPARAM wParam, LPARAM lParam) {
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	if(strcmp(dcws->szModule, "CList") != 0 || strcmp(dcws->szSetting, "State") != 0) return 0;

	// clist hiding
	if(dcws->value.type == DBVT_BYTE && dcws->value.bVal == 0) {
		HideTip(0, 0);
	}

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	font_id_title.cbSize = sizeof(FontIDT);
	font_id_title.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_title.group, LPGENT("Tooltips"));
	_tcscpy(font_id_title.name, LPGENT("Title"));
	strcpy(font_id_title.dbSettingsGroup, MODULE);
	strcpy(font_id_title.prefix, "FontFirst");
	font_id_title.order = 0;

	font_id_title.deffontsettings.charset = DEFAULT_CHARSET;
	font_id_title.deffontsettings.size = -14;
	font_id_title.deffontsettings.style = DBFONTF_BOLD;
	font_id_title.deffontsettings.colour = RGB(255, 0, 0);
	font_id_title.flags |= FIDF_DEFAULTVALID;

	font_id_labels.cbSize = sizeof(FontIDT);
	font_id_labels.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_labels.group, LPGENT("Tooltips"));
	_tcscpy(font_id_labels.name, LPGENT("Labels"));
	strcpy(font_id_labels.dbSettingsGroup, MODULE);
	strcpy(font_id_labels.prefix, "FontLabels");
	font_id_labels.order = 1;

	font_id_labels.deffontsettings.charset = DEFAULT_CHARSET;
	font_id_labels.deffontsettings.size = -12;
	font_id_labels.deffontsettings.style = DBFONTF_ITALIC;
	font_id_labels.deffontsettings.colour = RGB(128, 128, 128);
	font_id_labels.flags |= FIDF_DEFAULTVALID;

	font_id_values.cbSize = sizeof(FontIDT);
	font_id_values.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(font_id_values.group, LPGENT("Tooltips"));
	_tcscpy(font_id_values.name, LPGENT("Values"));
	strcpy(font_id_values.dbSettingsGroup, MODULE);
	strcpy(font_id_values.prefix, "FontValues");
	font_id_values.order = 2;

	font_id_values.deffontsettings.charset = DEFAULT_CHARSET;
	font_id_values.deffontsettings.size = -12;
	font_id_values.deffontsettings.style = 0;
	font_id_values.deffontsettings.colour = RGB(0, 0, 0);
	font_id_values.flags |= FIDF_DEFAULTVALID;

	CallService(MS_FONT_REGISTERT, (WPARAM)&font_id_title, 0);
	CallService(MS_FONT_REGISTERT, (WPARAM)&font_id_labels, 0);
	CallService(MS_FONT_REGISTERT, (WPARAM)&font_id_values, 0);
	
	colour_id_bg.cbSize = sizeof(ColourIDT);
	_tcscpy(colour_id_bg.group, LPGENT("Tooltips"));
	_tcscpy(colour_id_bg.name, LPGENT("Background"));
	strcpy(colour_id_bg.dbSettingsGroup, MODULE);
	strcpy(colour_id_bg.setting, "ColourBg");
	colour_id_bg.defcolour = RGB(219, 219, 219);
	colour_id_bg.order = 0;

	colour_id_border.cbSize = sizeof(ColourIDT);
	_tcscpy(colour_id_border.group, LPGENT("Tooltips"));
	_tcscpy(colour_id_border.name, LPGENT("Border"));
	strcpy(colour_id_border.dbSettingsGroup, MODULE);
	strcpy(colour_id_border.setting, "BorderCol");
	colour_id_border.defcolour = 0;
	colour_id_border.order = 0;

	colour_id_divider.cbSize = sizeof(ColourIDT);
	_tcscpy(colour_id_divider.group, LPGENT("Tooltips"));
	_tcscpy(colour_id_divider.name, LPGENT("Dividers"));
	strcpy(colour_id_divider.dbSettingsGroup, MODULE);
	strcpy(colour_id_divider.setting, "DividerCol");
	colour_id_divider.defcolour = 0;
	colour_id_divider.order = 0;

	colour_id_sidebar.cbSize = sizeof(ColourIDT);
	_tcscpy(colour_id_sidebar.group, LPGENT("Tooltips"));
	_tcscpy(colour_id_sidebar.name, LPGENT("Sidebar"));
	strcpy(colour_id_sidebar.dbSettingsGroup, MODULE);
	strcpy(colour_id_sidebar.setting, "SidebarCol");
	colour_id_sidebar.defcolour = RGB(192, 192, 192);
	colour_id_sidebar.order = 0;

	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colour_id_bg, 0);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colour_id_border, 0);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colour_id_divider, 0);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colour_id_sidebar, 0);
	
	ReloadFont(0, 0);

	HookEvent(ME_FONT_RELOAD, ReloadFont);

	hAvChangeEvent = HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	hShowTipEvent = HookEvent(ME_CLC_SHOWINFOTIP, ShowTipHook);
	hHideTipEvent = HookEvent(ME_CLC_HIDEINFOTIP, HideTipHook);
	hAckEvent = HookEvent(ME_PROTO_ACK, ProtoAck);

	hFramesSBShow = HookEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP, FramesShowSBTip);
	hFramesSBHide = HookEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP, FramesHideSBTip);

	LoadOptions(); 

	// set 'time-in'
	CallService(MS_CLC_SETINFOTIPHOVERTIME, options.time_in, 0);

	return 0;
}

int Shutdown(WPARAM wParam, LPARAM lParam) {
	if(hFramesSBShow) UnhookEvent(hFramesSBShow);
	if(hFramesSBHide) UnhookEvent(hFramesSBHide);
	if(hAvChangeEvent) UnhookEvent(hAvChangeEvent);
	if(hAvContactChangeEvent) UnhookEvent(hAvContactChangeEvent);
	if(hShowTipEvent) UnhookEvent(hShowTipEvent);
	if(hHideTipEvent) UnhookEvent(hHideTipEvent);
	if(hAckEvent) UnhookEvent(hAckEvent);

	if(hShowTipService) DestroyServiceFunction(hShowTipService);
	if(hShowTipWService) DestroyServiceFunction(hShowTipWService);
	if(hHideTipService) DestroyServiceFunction(hHideTipService);

	DeinitMessagePump();

	return 0;
}

HANDLE hEventPreShutdown, hEventModulesLoaded;

extern "C" int TIPPER_API Load(PLUGINLINK *link) {
	pluginLink = link;
	set_codepage();

	mir_getMMI(&mmi);
	mir_getTMI(&tmi);

	// don't save status messages
	CallService(MS_DB_SETSETTINGRESIDENT, (WPARAM)TRUE, (LPARAM)MODULE "/TempStatusMsg");

	// Ensure that the common control DLL is loaded (for listview)
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	

	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	InitTranslations();
	InitMessagePump();
	InitOptions();

	// for compatibility with mToolTip status tooltips
	hShowTipService = CreateServiceFunction("mToolTip/ShowTip", ShowTip);
#ifdef _UNICODE
	hShowTipWService = CreateServiceFunction("mToolTip/ShowTipW", ShowTipW);
#endif
	hHideTipService = CreateServiceFunction("mToolTip/HideTip", HideTip);

	hEventPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	hSettingChangedEvent = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	return 0;
}

extern "C" int TIPPER_API Unload() {
	UnhookEvent(hSettingChangedEvent);
	UnhookEvent(hEventPreShutdown);
	UnhookEvent(hEventModulesLoaded);

	DeinitOptions();
	DeleteObject(hFontTitle);
	DeleteObject(hFontLabels);
	DeleteObject(hFontValues);
	DeinitTranslations();
	return 0;
}
