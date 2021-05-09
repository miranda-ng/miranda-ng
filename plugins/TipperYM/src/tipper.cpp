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

#include "stdafx.h"

CMPlugin g_plugin;

FontIDW fontTitle, fontLabels, fontValues, fontTrayTitle;
ColourIDW colourBg, colourBorder, colourAvatarBorder, colourDivider, colourSidebar;
HFONT hFontTitle, hFontLabels, hFontValues, hFontTrayTitle;

// hooked here so it's in the main thread
HANDLE hAvChangeEvent, hShowTipEvent, hHideTipEvent, hAckEvent, hFramesSBShow, hFramesSBHide;

HANDLE hFolderChanged, hSkinFolder;
wchar_t SKIN_FOLDER[256];

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {8392DF1D-9090-4F8E-9DF6-2FE058EDD800}
	{0x8392df1d, 0x9090, 0x4f8e, {0x9d, 0xf6, 0x2f, 0xe0, 0x58, 0xed, 0xd8, 0x00}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int ReloadFont(WPARAM, LPARAM)
{
	LOGFONT logFont;
	if (hFontTitle) DeleteObject(hFontTitle);
	opt.colTitle = Font_GetW(fontTitle, &logFont);
	hFontTitle = CreateFontIndirect(&logFont);

	if (hFontLabels) DeleteObject(hFontLabels);
	opt.colLabel = Font_GetW(fontLabels, &logFont);
	hFontLabels = CreateFontIndirect(&logFont);

	if (hFontValues) DeleteObject(hFontValues);
	opt.colValue = Font_GetW(fontValues, &logFont);
	hFontValues = CreateFontIndirect(&logFont);

	if (hFontTrayTitle) DeleteObject(hFontTrayTitle);
	opt.colTrayTitle = Font_GetW(fontTrayTitle, &logFont);
	hFontTrayTitle = CreateFontIndirect(&logFont);

	opt.colBg = Colour_GetW(colourBg);
	opt.colBorder = Colour_GetW(colourBorder);
	opt.colAvatarBorder = Colour_GetW(colourAvatarBorder);
	opt.colSidebar = Colour_GetW(colourSidebar);
	opt.colDivider = Colour_GetW(colourDivider);
	return 0;
}

// hack to hide tip when clist hides from timeout
static int SettingChanged(WPARAM, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	if (strcmp(dcws->szModule, "CList") != 0 || strcmp(dcws->szSetting, "State") != 0)
		return 0;

	// clist hiding
	if (dcws->value.type == DBVT_BYTE && dcws->value.bVal == 0)
		HideTip(0, 0);

	return 0;
}

// needed for msg_count_xxx substitutions
static int EventDeleted(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei = {};
	if (!db_event_get(lParam, &dbei))
		if (dbei.eventType == EVENTTYPE_MESSAGE)
			g_plugin.delSetting(wParam, "LastCountTS");

	return 0;
}

static int ReloadSkinFolder(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hSkinFolder, SKIN_FOLDER, _countof(SKIN_FOLDER), DEFAULT_SKIN_FOLDER);
	return 0;
}

static void InitFonts()
{
	mir_wstrcpy(colourBg.group, LPGENW("Tooltips"));
	mir_wstrcpy(colourBg.name, LPGENW("Background"));
	mir_strcpy(colourBg.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourBg.setting, "ColourBg");
	colourBg.defcolour = RGB(219, 219, 219);
	colourBg.order = 0;
	g_plugin.addColor(&colourBg);

	mir_wstrcpy(colourBorder.group, LPGENW("Tooltips"));
	mir_wstrcpy(colourBorder.name, LPGENW("Border"));
	mir_strcpy(colourBorder.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourBorder.setting, "BorderCol");
	colourBorder.defcolour = 0;
	colourBorder.order = 0;
	g_plugin.addColor(&colourBorder);

	mir_wstrcpy(colourAvatarBorder.group, LPGENW("Tooltips"));
	mir_wstrcpy(colourAvatarBorder.name, LPGENW("Avatar border"));
	mir_strcpy(colourAvatarBorder.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourAvatarBorder.setting, "AvBorderCol");
	colourAvatarBorder.defcolour = 0;
	colourAvatarBorder.order = 0;
	g_plugin.addColor(&colourAvatarBorder);

	mir_wstrcpy(colourDivider.group, LPGENW("Tooltips"));
	mir_wstrcpy(colourDivider.name, LPGENW("Dividers"));
	mir_strcpy(colourDivider.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourDivider.setting, "DividerCol");
	colourDivider.defcolour = 0;
	colourDivider.order = 0;
	g_plugin.addColor(&colourDivider);

	mir_wstrcpy(colourSidebar.group, LPGENW("Tooltips"));
	mir_wstrcpy(colourSidebar.name, LPGENW("Sidebar"));
	mir_strcpy(colourSidebar.dbSettingsGroup, MODULENAME);
	mir_strcpy(colourSidebar.setting, "SidebarCol");
	colourSidebar.defcolour = RGB(192, 192, 192);
	colourSidebar.order = 0;
	g_plugin.addColor(&colourSidebar);

	fontTitle.flags = FIDF_ALLOWEFFECTS | FIDF_DEFAULTVALID;
	mir_wstrcpy(fontTitle.group, LPGENW("Tooltips"));
	mir_wstrcpy(fontTitle.name, LPGENW("Title"));
	mir_strcpy(fontTitle.dbSettingsGroup, MODULENAME);
	mir_strcpy(fontTitle.setting, "FontFirst");
	mir_wstrcpy(fontTitle.backgroundGroup, LPGENW("Tooltips"));
	mir_wstrcpy(fontTitle.backgroundName, LPGENW("Background"));
	fontTitle.order = 0;
	fontTitle.deffontsettings.charset = DEFAULT_CHARSET;
	fontTitle.deffontsettings.size = -14;
	fontTitle.deffontsettings.style = DBFONTF_BOLD;
	fontTitle.deffontsettings.colour = RGB(255, 0, 0);

	fontLabels.flags = FIDF_ALLOWEFFECTS | FIDF_DEFAULTVALID;
	mir_wstrcpy(fontLabels.group, LPGENW("Tooltips"));
	mir_wstrcpy(fontLabels.name, LPGENW("Labels"));
	mir_strcpy(fontLabels.dbSettingsGroup, MODULENAME);
	mir_strcpy(fontLabels.setting, "FontLabels");
	mir_wstrcpy(fontLabels.backgroundGroup, LPGENW("Tooltips"));
	mir_wstrcpy(fontLabels.backgroundName, LPGENW("Background"));
	fontLabels.order = 1;
	fontLabels.deffontsettings.charset = DEFAULT_CHARSET;
	fontLabels.deffontsettings.size = -12;
	fontLabels.deffontsettings.style = DBFONTF_ITALIC;
	fontLabels.deffontsettings.colour = RGB(128, 128, 128);

	fontValues.flags = FIDF_ALLOWEFFECTS | FIDF_DEFAULTVALID;
	mir_wstrcpy(fontValues.group, LPGENW("Tooltips"));
	mir_wstrcpy(fontValues.name, LPGENW("Values"));
	mir_strcpy(fontValues.dbSettingsGroup, MODULENAME);
	mir_strcpy(fontValues.setting, "FontValues");
	mir_wstrcpy(fontValues.backgroundGroup, LPGENW("Tooltips"));
	mir_wstrcpy(fontValues.backgroundName, LPGENW("Background"));
	fontValues.order = 2;
	fontValues.deffontsettings.charset = DEFAULT_CHARSET;
	fontValues.deffontsettings.size = -12;
	fontValues.deffontsettings.style = 0;
	fontValues.deffontsettings.colour = RGB(0, 0, 0);

	fontTrayTitle.flags = FIDF_ALLOWEFFECTS | FIDF_DEFAULTVALID;
	mir_wstrcpy(fontTrayTitle.group, LPGENW("Tooltips"));
	mir_wstrcpy(fontTrayTitle.name, LPGENW("Tray title"));
	mir_strcpy(fontTrayTitle.dbSettingsGroup, MODULENAME);
	mir_strcpy(fontTrayTitle.setting, "FontTrayTitle");
	mir_wstrcpy(fontTrayTitle.backgroundGroup, LPGENW("Tooltips"));
	mir_wstrcpy(fontTrayTitle.backgroundName, LPGENW("Background"));
	fontTrayTitle.order = 0;
	fontTrayTitle.deffontsettings.charset = DEFAULT_CHARSET;
	fontTrayTitle.deffontsettings.size = -14;
	fontTrayTitle.deffontsettings.style = DBFONTF_BOLD;
	fontTrayTitle.deffontsettings.colour = RGB(0, 0, 0);

	g_plugin.addFont(&fontTitle);
	g_plugin.addFont(&fontLabels);
	g_plugin.addFont(&fontValues);
	g_plugin.addFont(&fontTrayTitle);

	HookEvent(ME_FONT_RELOAD, ReloadFont);
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	InitFonts();

	hAvChangeEvent = HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	hShowTipEvent = HookEvent(ME_CLC_SHOWINFOTIP, ShowTipHook);
	hHideTipEvent = HookEvent(ME_CLC_HIDEINFOTIP, HideTipHook);
	hAckEvent = HookEvent(ME_PROTO_ACK, ProtoAck);

	hFramesSBShow = HookEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP, FramesShowSBTip);
	hFramesSBHide = HookEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP, FramesHideSBTip);

	hFolderChanged = HookEvent(ME_FOLDERS_PATH_CHANGED, ReloadSkinFolder);

	hSkinFolder = FoldersRegisterCustomPathW(LPGEN("Skins"), LPGEN("Tipper"), MIRANDA_PATHW L"\\" DEFAULT_SKIN_FOLDER);
	FoldersGetCustomPathW(hSkinFolder, SKIN_FOLDER, _countof(SKIN_FOLDER), DEFAULT_SKIN_FOLDER);

	InitTipperSmileys();
	LoadOptions();
	ReloadFont(0, 0);
	ParseSkinFile(opt.szSkinName, true, false);

	// set 'time-in'
	CallService(MS_CLC_SETINFOTIPHOVERTIME, opt.iTimeIn, 0);

	// set Miranda start timestamp
	g_plugin.setDword("MirandaStartTS", (DWORD)time(0));
	return 0;
}

int Shutdown(WPARAM, LPARAM)
{
	if (hFramesSBShow) UnhookEvent(hFramesSBShow);
	if (hFramesSBHide) UnhookEvent(hFramesSBHide);
	if (hAvChangeEvent) UnhookEvent(hAvChangeEvent);
	if (hShowTipEvent) UnhookEvent(hShowTipEvent);
	if (hHideTipEvent) UnhookEvent(hHideTipEvent);
	if (hAckEvent) UnhookEvent(hAckEvent);

	if (hFolderChanged) UnhookEvent(hFolderChanged);

	DeinitMessagePump();
	DestroySkinBitmap();

	return 0;
}

static INT_PTR ReloadSkin(WPARAM wParam, LPARAM lParam)
{
	LoadOptions();
	opt.skinMode = (SkinMode)wParam;
	if (lParam != 0)
		wcscpy_s(opt.szSkinName, _A2T((char*)(lParam)));
	ParseSkinFile(opt.szSkinName, false, false);
	ReloadFont(0, 0);
	SaveOptions();

	g_plugin.setByte("SkinEngine", opt.skinMode);
	g_plugin.setWString("SkinName", opt.szSkinName);

	DestroySkinBitmap();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

IconItem iconList[] =
{
	{ LPGEN("Copy item"),      "copy_item",      IDI_ITEM      },
	{ LPGEN("Copy all items"), "copy_all_items", IDI_ITEM_ALL  },
	{ LPGEN("Arrow up"),       "arrow_up",       IDI_UP        },
	{ LPGEN("Arrow down"),     "arrow_down",     IDI_DOWN      },
	{ LPGEN("Separator"),      "separator",      IDI_SEPARATOR },
	{ LPGEN("Reload"),         "reload",         IDI_RELOAD    },
	{ LPGEN("Apply"),          "apply",          IDI_APPLY     }
};

int CMPlugin::Load()
{
	iCodePage = Langpack_GetDefaultCodePage();

	g_plugin.registerIcon(MODULENAME, iconList, MODULENAME);

	InitTranslations();
	InitMessagePump();

	// for compatibility with mToolTip status tooltips
	CreateServiceFunction(MS_TIPPER_SHOWTIP, ShowTip);
	CreateServiceFunction(MS_TIPPER_SHOWTIPW, ShowTipW);
	CreateServiceFunction(MS_TIPPER_HIDETIP, HideTip);
	CreateServiceFunction("mToolTip/ReloadSkin", ReloadSkin);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	HookEvent(ME_DB_EVENT_DELETED, EventDeleted);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DeinitOptions();
	DeleteObject(hFontTitle);
	DeleteObject(hFontLabels);
	DeleteObject(hFontValues);
	DeleteObject(hFontTrayTitle);

	DeinitTranslations();
	return 0;
}
