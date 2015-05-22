/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

HMODULE hInst;

FontIDT fontTitle, fontLabels, fontValues, fontTrayTitle;
ColourIDT colourBg, colourBorder, colourAvatarBorder, colourDivider, colourSidebar;
HFONT hFontTitle, hFontLabels, hFontValues, hFontTrayTitle;

// hooked here so it's in the main thread
HANDLE hAvChangeEvent, hShowTipEvent, hHideTipEvent, hAckEvent, hFramesSBShow, hFramesSBHide;
HANDLE hSettingChangedEvent, hEventDeleted;
HANDLE hShowTipService, hShowTipWService, hHideTipService;
HANDLE hReloadFonts = NULL;

HANDLE hFolderChanged, hSkinFolder;
TCHAR SKIN_FOLDER[256];

CLIST_INTERFACE *pcli = NULL;
FI_INTERFACE *fii = NULL;
TIME_API tmi;
int hLangpack;

PLUGININFOEX pluginInfoEx =
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
	// {8392DF1D-9090-4F8E-9DF6-2FE058EDD800}
	{0x8392df1d, 0x9090, 0x4f8e, {0x9d, 0xf6, 0x2f, 0xe0, 0x58, 0xed, 0xd8, 0x00}}
};

bool WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

int ReloadFont(WPARAM wParam, LPARAM lParam)
{
	LOGFONT logFont;
	if (hFontTitle) DeleteObject(hFontTitle);
	opt.colTitle = CallService(MS_FONT_GETT, (WPARAM)&fontTitle, (LPARAM)&logFont);
	hFontTitle = CreateFontIndirect(&logFont);
	if (hFontLabels) DeleteObject(hFontLabels);
	opt.colLabel = CallService(MS_FONT_GETT, (WPARAM)&fontLabels, (LPARAM)&logFont);
	hFontLabels = CreateFontIndirect(&logFont);
	if (hFontValues) DeleteObject(hFontValues);
	opt.colValue = CallService(MS_FONT_GETT, (WPARAM)&fontValues, (LPARAM)&logFont);
	hFontValues = CreateFontIndirect(&logFont);
	if (hFontTrayTitle) DeleteObject(hFontTrayTitle);
	opt.colTrayTitle = CallService(MS_FONT_GETT, (WPARAM)&fontTrayTitle, (LPARAM)&logFont);
	hFontTrayTitle = CreateFontIndirect(&logFont);

	opt.colBg = CallService(MS_COLOUR_GETT, (WPARAM)&colourBg, 0);
	opt.colBorder = CallService(MS_COLOUR_GETT, (WPARAM)&colourBorder, 0);
	opt.colAvatarBorder = CallService(MS_COLOUR_GETT, (WPARAM)&colourAvatarBorder, 0);
	opt.colSidebar = CallService(MS_COLOUR_GETT, (WPARAM)&colourSidebar, 0);
	opt.colDivider = CallService(MS_COLOUR_GETT, (WPARAM)&colourDivider, 0);

	return 0;
}

// hack to hide tip when clist hides from timeout
int SettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	if (mir_strcmp(dcws->szModule, "CList") != 0 || mir_strcmp(dcws->szSetting, "State") != 0)
		return 0;

	// clist hiding
	if (dcws->value.type == DBVT_BYTE && dcws->value.bVal == 0)
		HideTip(0, 0);

	return 0;
}

// needed for msg_count_xxx substitutions
int EventDeleted(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	if (!db_event_get(lParam, &dbei))
		if (dbei.eventType == EVENTTYPE_MESSAGE)
			db_unset(wParam, MODULE, "LastCountTS");

	return 0;
}

int ReloadSkinFolder(WPARAM wParam, LPARAM lParam)
{
	FoldersGetCustomPathT(hSkinFolder, SKIN_FOLDER, SIZEOF(SKIN_FOLDER), _T(DEFAULT_SKIN_FOLDER));
	return 0;
}

void InitFonts()
{
	colourBg.cbSize = sizeof(ColourIDT);
	_tcscpy(colourBg.group, LPGENT("Tooltips"));
	_tcscpy(colourBg.name, LPGENT("Background"));
	strcpy(colourBg.dbSettingsGroup, MODULE);
	strcpy(colourBg.setting, "ColourBg");
	colourBg.defcolour = RGB(219, 219, 219);
	colourBg.order = 0;
	ColourRegisterT(&colourBg);

	colourBorder.cbSize = sizeof(ColourIDT);
	_tcscpy(colourBorder.group, LPGENT("Tooltips"));
	_tcscpy(colourBorder.name, LPGENT("Border"));
	strcpy(colourBorder.dbSettingsGroup, MODULE);
	strcpy(colourBorder.setting, "BorderCol");
	colourBorder.defcolour = 0;
	colourBorder.order = 0;
	ColourRegisterT(&colourBorder);

	colourAvatarBorder.cbSize = sizeof(ColourIDT);
	_tcscpy(colourAvatarBorder.group, LPGENT("Tooltips"));
	_tcscpy(colourAvatarBorder.name, LPGENT("Avatar border"));
	strcpy(colourAvatarBorder.dbSettingsGroup, MODULE);
	strcpy(colourAvatarBorder.setting, "AvBorderCol");
	colourAvatarBorder.defcolour = 0;
	colourAvatarBorder.order = 0;
	ColourRegisterT(&colourAvatarBorder);

	colourDivider.cbSize = sizeof(ColourIDT);
	_tcscpy(colourDivider.group, LPGENT("Tooltips"));
	_tcscpy(colourDivider.name, LPGENT("Dividers"));
	strcpy(colourDivider.dbSettingsGroup, MODULE);
	strcpy(colourDivider.setting, "DividerCol");
	colourDivider.defcolour = 0;
	colourDivider.order = 0;
	ColourRegisterT(&colourDivider);

	colourSidebar.cbSize = sizeof(ColourIDT);
	_tcscpy(colourSidebar.group, LPGENT("Tooltips"));
	_tcscpy(colourSidebar.name, LPGENT("Sidebar"));
	strcpy(colourSidebar.dbSettingsGroup, MODULE);
	strcpy(colourSidebar.setting, "SidebarCol");
	colourSidebar.defcolour = RGB(192, 192, 192);
	colourSidebar.order = 0;
	ColourRegisterT(&colourSidebar);

	fontTitle.cbSize = sizeof(FontIDT);
	fontTitle.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(fontTitle.group, LPGENT("Tooltips"));
	_tcscpy(fontTitle.name, LPGENT("Title"));
	strcpy(fontTitle.dbSettingsGroup, MODULE);
	strcpy(fontTitle.prefix, "FontFirst");
	_tcscpy(fontTitle.backgroundGroup, LPGENT("Tooltips"));
	_tcscpy(fontTitle.backgroundName, LPGENT("Background"));
	fontTitle.order = 0;

	fontTitle.deffontsettings.charset = DEFAULT_CHARSET;
	fontTitle.deffontsettings.size = -14;
	fontTitle.deffontsettings.style = DBFONTF_BOLD;
	fontTitle.deffontsettings.colour = RGB(255, 0, 0);
	fontTitle.flags |= FIDF_DEFAULTVALID;

	fontLabels.cbSize = sizeof(FontIDT);
	fontLabels.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(fontLabels.group, LPGENT("Tooltips"));
	_tcscpy(fontLabels.name, LPGENT("Labels"));
	strcpy(fontLabels.dbSettingsGroup, MODULE);
	strcpy(fontLabels.prefix, "FontLabels");
	_tcscpy(fontLabels.backgroundGroup, LPGENT("Tooltips"));
	_tcscpy(fontLabels.backgroundName, LPGENT("Background"));
	fontLabels.order = 1;

	fontLabels.deffontsettings.charset = DEFAULT_CHARSET;
	fontLabels.deffontsettings.size = -12;
	fontLabels.deffontsettings.style = DBFONTF_ITALIC;
	fontLabels.deffontsettings.colour = RGB(128, 128, 128);
	fontLabels.flags |= FIDF_DEFAULTVALID;

	fontValues.cbSize = sizeof(FontIDT);
	fontValues.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(fontValues.group, LPGENT("Tooltips"));
	_tcscpy(fontValues.name, LPGENT("Values"));
	strcpy(fontValues.dbSettingsGroup, MODULE);
	strcpy(fontValues.prefix, "FontValues");
	_tcscpy(fontValues.backgroundGroup, LPGENT("Tooltips"));
	_tcscpy(fontValues.backgroundName, LPGENT("Background"));
	fontValues.order = 2;

	fontValues.deffontsettings.charset = DEFAULT_CHARSET;
	fontValues.deffontsettings.size = -12;
	fontValues.deffontsettings.style = 0;
	fontValues.deffontsettings.colour = RGB(0, 0, 0);
	fontValues.flags |= FIDF_DEFAULTVALID;

	fontTrayTitle.cbSize = sizeof(FontIDT);
	fontTrayTitle.flags = FIDF_ALLOWEFFECTS;
	_tcscpy(fontTrayTitle.group, LPGENT("Tooltips"));
	_tcscpy(fontTrayTitle.name, LPGENT("Tray title"));
	strcpy(fontTrayTitle.dbSettingsGroup, MODULE);
	strcpy(fontTrayTitle.prefix, "FontTrayTitle");
	_tcscpy(fontTrayTitle.backgroundGroup, LPGENT("Tooltips"));
	_tcscpy(fontTrayTitle.backgroundName, LPGENT("Background"));
	fontTrayTitle.order = 0;

	fontTrayTitle.deffontsettings.charset = DEFAULT_CHARSET;
	fontTrayTitle.deffontsettings.size = -14;
	fontTrayTitle.deffontsettings.style = DBFONTF_BOLD;
	fontTrayTitle.deffontsettings.colour = RGB(0, 0, 0);
	fontTrayTitle.flags |= FIDF_DEFAULTVALID;

	FontRegisterT(&fontTitle);
	FontRegisterT(&fontLabels);
	FontRegisterT(&fontValues);
	FontRegisterT(&fontTrayTitle);

	hReloadFonts = HookEvent(ME_FONT_RELOAD, ReloadFont);
}

int ModulesLoaded(WPARAM, LPARAM)
{
	InitFonts();
	
	hAvChangeEvent = HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	hShowTipEvent = HookEvent(ME_CLC_SHOWINFOTIP, ShowTipHook);
	hHideTipEvent = HookEvent(ME_CLC_HIDEINFOTIP, HideTipHook);
	hAckEvent = HookEvent(ME_PROTO_ACK, ProtoAck);

	hFramesSBShow = HookEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP, FramesShowSBTip);
	hFramesSBHide = HookEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP, FramesHideSBTip);

	hFolderChanged = HookEvent(ME_FOLDERS_PATH_CHANGED, ReloadSkinFolder);

	hSkinFolder = FoldersRegisterCustomPathT(LPGEN("Skins"), LPGEN("Tipper"), MIRANDA_PATHT _T("\\") _T(DEFAULT_SKIN_FOLDER));
	FoldersGetCustomPathT(hSkinFolder, SKIN_FOLDER, SIZEOF(SKIN_FOLDER), _T(DEFAULT_SKIN_FOLDER));

	InitTipperSmileys();
	LoadOptions();
	ReloadFont(0, 0);
	ParseSkinFile(opt.szSkinName, true, false);

	// set 'time-in'
	CallService(MS_CLC_SETINFOTIPHOVERTIME, opt.iTimeIn, 0);

	// set Miranda start timestamp
	db_set_dw(0, MODULE, "MirandaStartTS", (DWORD)time(0));
	return 0;
}

int Shutdown(WPARAM wParam, LPARAM lParam)
{
	if (hFramesSBShow) UnhookEvent(hFramesSBShow);
	if (hFramesSBHide) UnhookEvent(hFramesSBHide);
	if (hAvChangeEvent) UnhookEvent(hAvChangeEvent);
	if (hShowTipEvent) UnhookEvent(hShowTipEvent);
	if (hHideTipEvent) UnhookEvent(hHideTipEvent);
	if (hAckEvent) UnhookEvent(hAckEvent);

	if (hShowTipService) DestroyServiceFunction(hShowTipService);
	if (hShowTipWService) DestroyServiceFunction(hShowTipWService);
	if (hHideTipService) DestroyServiceFunction(hHideTipService);

	if (hFolderChanged) UnhookEvent(hFolderChanged);

	DeinitMessagePump();
	DestroySkinBitmap();

	return 0;
}

HANDLE hEventPreShutdown, hEventModulesLoaded;

extern "C" int __declspec(dllexport) Load(void)
{
	CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&fii);
	mir_getLP(&pluginInfoEx);
	mir_getCLI();
	mir_getTMI(&tmi);

	if (ServiceExists(MS_LANGPACK_GETCODEPAGE))
		iCodePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	InitTranslations();
	InitMessagePump();
	InitOptions();

	// for compatibility with mToolTip status tooltips
	hShowTipService = CreateServiceFunction("mToolTip/ShowTip", ShowTip);

	hShowTipWService = CreateServiceFunction("mToolTip/ShowTipW", ShowTipW);

	hHideTipService = CreateServiceFunction("mToolTip/HideTip", HideTip);

	hEventPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	hSettingChangedEvent = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	hEventDeleted = HookEvent(ME_DB_EVENT_DELETED, EventDeleted);

	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	UnhookEvent(hSettingChangedEvent);
	UnhookEvent(hEventDeleted);
	UnhookEvent(hEventPreShutdown);
	UnhookEvent(hEventModulesLoaded);
	UnhookEvent(hReloadFonts);

	DeinitOptions();
	DeleteObject(hFontTitle);
	DeleteObject(hFontLabels);
	DeleteObject(hFontValues);
	DeleteObject(hFontTrayTitle);

	DeinitTranslations();
	return 0;
}
