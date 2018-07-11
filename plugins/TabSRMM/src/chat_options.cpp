/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-18 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// group chat options and generic font handling

#include "stdafx.h"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct FontOptionsList
{
	wchar_t *szDescr;
	COLORREF defColour;
	wchar_t *szDefFace;
	BYTE     defCharset, defStyle;
	char     defSize;
	COLORREF colour;
	wchar_t  szFace[LF_FACESIZE];
	BYTE     charset, style;
	char     size;
};

struct ColorOptionsList
{
	int		order;
	wchar_t *tszGroup;
	wchar_t *tszName;
	char    *szSetting;
	COLORREF def;
};

/*
* note: bits 24-31 in default color indicates that color is a system color index
* (GetSysColor(default_color & 0x00ffffff)), not a rgb value.
*/
static ColorOptionsList _clrs[] = {
	{ 0, LPGENW("Message sessions"), LPGENW("Input area background"), "inputbg", SRMSGDEFSET_BKGCOLOUR },
	{ 1, LPGENW("Message sessions"), LPGENW("Log background"), SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR },
	{ 0, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Outgoing background"), "outbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 1, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Incoming background"), "inbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 2, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Status background"), "statbg", SRMSGDEFSET_BKGCOLOUR },
	{ 3, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Incoming background(old)"), "oldinbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 4, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Outgoing background(old)"), "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 5, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), LPGENW("Horizontal Grid Lines"), "hgrid", RGB(224, 224, 224) },
	{ 0, LPGENW("Message sessions") L"/" LPGENW("Info Panel"), LPGENW("Panel background low"), "ipfieldsbg", 0x62caff },
	{ 1, LPGENW("Message sessions") L"/" LPGENW("Info Panel"), LPGENW("Panel background high"), "ipfieldsbgHigh", 0xf0f0f0 },
	{ 0, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Toolbar background high"), "tbBgHigh", 0 },
	{ 1, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Toolbar background low"), "tbBgLow", 0 },
	{ 2, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Window fill color"), "fillColor", 0 },
	{ 3, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Text area borders"), "cRichBorders", 0 },
	{ 4, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Aero glow effect"), "aeroGlow", RGB(40, 40, 255) },
	{ 4, LPGENW("Message sessions") L"/" LPGENW("Common colors"), LPGENW("Generic text color (only when fill color is set)"), "genericTxtClr", 0xff000000 | COLOR_BTNTEXT },
};

static ColorOptionsList _tabclrs[] = {
	{ 0, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Normal text"), "tab_txt_normal", 0xff000000 | COLOR_BTNTEXT },
	{ 1, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Active text"), "tab_txt_active", 0xff000000 | COLOR_BTNTEXT },
	{ 2, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Hovered text"), "tab_txt_hottrack", 0xff000000 | COLOR_HOTLIGHT },
	{ 3, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Unread text"), "tab_txt_unread", 0xff000000 | COLOR_HOTLIGHT },

	{ 4, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Normal background"), "tab_bg_normal", 0xff000000 | COLOR_3DFACE },
	{ 5, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Active background"), "tab_bg_active", 0xff000000 | COLOR_3DFACE },
	{ 6, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Hovered background"), "tab_bg_hottrack", 0xff000000 | COLOR_3DFACE },
	{ 7, LPGENW("Message sessions") L"/" LPGENW("Tabs"), LPGENW("Unread background"), "tab_bg_unread", 0xff000000 | COLOR_3DFACE }
};

extern LOGFONT lfDefault;

static FontOptionsList IM_fontOptionsList[] = {
	{ LPGENW(">> Outgoing messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("* Message Input Area"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Status changes"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Dividers"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Error and warning messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Symbols (incoming)"), RGB(50, 50, 50), L"Webdings", SYMBOL_CHARSET, 0, -12 },
	{ LPGENW("* Symbols (outgoing)"), RGB(50, 50, 50), L"Webdings", SYMBOL_CHARSET, 0, -12 },
};

static FontOptionsList IP_fontOptionsList[] = {
	{ LPGENW("Nickname"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("UIN"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Status"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Protocol"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Contacts local time"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Window caption (skinned mode)"), RGB(255, 255, 255), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
};

static FontOptionsList *fontOptionsList = IM_fontOptionsList;
static int fontCount = MSGDLGFONTCOUNT;

struct branch_t
{
	wchar_t*    szDescr;
	char*     szDBName;
	int       iMode;
	BYTE      bDefault;
	HTREEITEM hItem;
};
static branch_t branch1[] = {
	{ LPGENW("Open new chat rooms in the default container"), "DefaultContainer", 0, 1, nullptr },
	{ LPGENW("Flash window when someone speaks"), "FlashWindow", 0, 0, nullptr },
	{ LPGENW("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, 1, nullptr },
	{ LPGENW("Create tabs or windows for highlight events"), "CreateWindowOnHighlight", 0, 0, nullptr },
	{ LPGENW("Activate chat window on highlight"), "AnnoyingHighlight", 0, 0, nullptr },
	{ LPGENW("Show list of users in the chat room"), "ShowNicklist", 0, 1, nullptr },
	{ LPGENW("Colorize nicknames in member list"), "ColorizeNicks", 0, 1, nullptr },
	{ LPGENW("Show button menus when right clicking the buttons"), "RightClickFilter", 0, 1, nullptr },
	{ LPGENW("Show topic as status message on the contact list"), "TopicOnClist", 0, 1, nullptr },
	{ LPGENW("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0, nullptr },
	{ LPGENW("Sync splitter position with standard IM sessions"), "SyncSplitter", 0, 0, nullptr },
	{ LPGENW("Show contact's status modes if supported by the protocol"), "ShowContactStatus", 0, 1, nullptr },
	{ LPGENW("Display contact's status icon before user role icon"), "ContactStatusFirst", 0, 0, nullptr },
	{ LPGENW("Use IRC style status indicators in the nick list"), "ClassicIndicators", 0, 0, nullptr },
	{ LPGENW("Use alternative sorting method in member list"), "AlternativeSorting", 0, 1, nullptr }
};

static branch_t branch2[] = {
	{ LPGENW("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, nullptr },
	{ LPGENW("Timestamp only when event time differs"), "ShowTimeStampIfChanged", 0, 0, nullptr },
	{ LPGENW("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0, nullptr },
	{ LPGENW("Indent the second line of a message"), "LogIndentEnabled", 0, 1, nullptr },
	{ LPGENW("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, 1, nullptr },
	{ LPGENW("Add a colon (:) to auto-completed user names"), "AddColonToAutoComplete", 0, 1, nullptr },
	{ LPGENW("Add a comma instead of a colon to auto-completed user names"), "UseCommaAsColon", 0, 0, nullptr },
	{ LPGENW("Start private conversation on double click in nick list (insert nick if unchecked)"), "DoubleClick4Privat", 0, 0, nullptr },
	{ LPGENW("Strip colors from messages in the log"), "StripFormatting", 0, 0, nullptr },
	{ LPGENW("Enable the 'event filter' for new rooms"), "FilterEnabled", 0, 0, nullptr },
	{ LPGENW("Use IRC style status indicators in the log"), "LogClassicIndicators", 0, 0, nullptr },
	{ LPGENW("Allow clickable user names in the message log"), "ClickableNicks", 0, 1, nullptr },
	{ LPGENW("Add new line after names"), "NewlineAfterNames", 0, 0, nullptr },
	{ LPGENW("Colorize user names in message log"), "ColorizeNicksInLog", 0, 1, nullptr },
	{ LPGENW("Scale down icons to 10x10 pixels in the chat log"), "ScaleIcons", 0, 1, nullptr }
};

void LoadMsgDlgFont(int section, int i, LOGFONT *lf, COLORREF* colour, char *szMod)
{
	char str[32];
	int db_idx = (section == FONTSECTION_IM) ? i : i + 100;

	FontOptionsList *fol = fontOptionsList;
	switch (section) {
	case FONTSECTION_IM: fol = IM_fontOptionsList; break;
	case FONTSECTION_IP: fol = IP_fontOptionsList; break;
	}

	if (colour) {
		mir_snprintf(str, "Font%dCol", db_idx);
		*colour = M.GetDword(szMod, str, fol[i].defColour);
	}

	if (lf) {
		mir_snprintf(str, "Font%dSize", db_idx);
		lf->lfHeight = (char)M.GetByte(szMod, str, fol[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "Font%dSty", db_idx);
		int style = M.GetByte(szMod, str, fol[i].defStyle);
		if (i == MSGFONTID_MESSAGEAREA && section == FONTSECTION_IM && M.GetByte("inputFontFix", 1) == 1) {
			lf->lfWeight = FW_NORMAL;
			lf->lfItalic = 0;
			lf->lfUnderline = 0;
			lf->lfStrikeOut = 0;
		}
		else {
			lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
			lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
			lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
			lf->lfStrikeOut = style & FONTF_STRIKEOUT ? 1 : 0;
		}
		mir_snprintf(str, "Font%dSet", db_idx);
		lf->lfCharSet = M.GetByte(szMod, str, fol[i].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "Font%d", db_idx);
		if ((i == 17 && !mir_strcmp(szMod, CHATFONT_MODULE)) || ((i == 20 || i == 21) && !mir_strcmp(szMod, FONTMODULE))) {
			lf->lfCharSet = SYMBOL_CHARSET;
			wcsncpy_s(lf->lfFaceName, L"Webdings", _TRUNCATE);
		}
		else {
			ptrW tszDefFace(db_get_wsa(0, szMod, str));
			if (tszDefFace == nullptr)
				wcsncpy_s(lf->lfFaceName, fol[i].szDefFace, _TRUNCATE);
			else
				wcsncpy_s(lf->lfFaceName, tszDefFace, _TRUNCATE);
		}
	}
}

static IconItem _icons[] =
{
	{ LPGEN("Window Icon"), "chat_window", IDI_CHANMGR },
	{ LPGEN("Icon overlay"), "chat_overlay", IDI_OVERLAY },

	{ LPGEN("Status 1 (10x10)"), "chat_status0", IDI_STATUS0 },
	{ LPGEN("Status 2 (10x10)"), "chat_status1", IDI_STATUS1 },
	{ LPGEN("Status 3 (10x10)"), "chat_status2", IDI_STATUS2 },
	{ LPGEN("Status 4 (10x10)"), "chat_status3", IDI_STATUS3 },
	{ LPGEN("Status 5 (10x10)"), "chat_status4", IDI_STATUS4 },
	{ LPGEN("Status 6 (10x10)"), "chat_status5", IDI_STATUS5 }
};

static IconItem _logicons[] =
{
	{ LPGEN("Message in (10x10)"), "chat_log_message_in", IDI_MESSAGE },
	{ LPGEN("Message out (10x10)"), "chat_log_message_out", IDI_MESSAGEOUT },
	{ LPGEN("Action (10x10)"), "chat_log_action", IDI_ACTION },
	{ LPGEN("Add Status (10x10)"), "chat_log_addstatus", IDI_ADDSTATUS },
	{ LPGEN("Remove Status (10x10)"), "chat_log_removestatus", IDI_REMSTATUS },
	{ LPGEN("Join (10x10)"), "chat_log_join", IDI_JOIN },
	{ LPGEN("Leave (10x10)"), "chat_log_part", IDI_PART },
	{ LPGEN("Quit (10x10)"), "chat_log_quit", IDI_QUIT },
	{ LPGEN("Kick (10x10)"), "chat_log_kick", IDI_KICK },
	{ LPGEN("Notice (10x10)"), "chat_log_notice", IDI_NOTICE },
	{ LPGEN("Nickchange (10x10)"), "chat_log_nick", IDI_NICK },
	{ LPGEN("Topic (10x10)"), "chat_log_topic", IDI_TOPIC },
	{ LPGEN("Highlight (10x10)"), "chat_log_highlight", IDI_HIGHLIGHT },
	{ LPGEN("Information (10x10)"), "chat_log_info", IDI_INFO }
};

// add icons to the skinning module
void Chat_AddIcons(void)
{
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions") "/" LPGEN("Group chat windows"), _icons, _countof(_icons), 0, g_plugin.m_hLang);
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions") "/" LPGEN("Group chat log"), _logicons, _countof(_logicons), 0, g_plugin.m_hLang);
	g_chatApi.MM_IconsChanged();
}

// get icon by name from the core icon library service
HICON LoadIconEx(char *pszIcoLibName)
{
	char szTemp[256];
	mir_snprintf(szTemp, "chat_%s", pszIcoLibName);
	return IcoLib_GetIcon(szTemp);
}

static wchar_t *chatcolorsnames[] =
{
	LPGENW("Voiced"),
	LPGENW("Half operators"),
	LPGENW("Channel operators"),
	LPGENW("Extended mode 1"),
	LPGENW("Extended mode 2"),
	LPGENW("Selection background"),
	LPGENW("Selected text"),
	LPGENW("Incremental search highlight")
};

void RegisterFontServiceFonts()
{
	LOGFONT lf;
	FontIDW fid = { 0 };
	ColourIDW cid = { 0 };

	fid.cbSize = sizeof(FontIDW);
	cid.cbSize = sizeof(ColourIDW);

	strncpy(fid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));

	for (int i = 0; i < _countof(IM_fontOptionsList); i++) {
		fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
		LoadMsgDlgFont(FONTSECTION_IM, i, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(fid.prefix, "Font%d", i);
		fid.order = i;
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		wcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		wcsncpy(fid.backgroundGroup, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), _countof(fid.backgroundGroup));
		wcsncpy(fid.group, LPGENW("Message sessions") L"/" LPGENW("Single Messaging"), _countof(fid.group));
		switch (i) {
		case MSGFONTID_MYMSG:
		case 1:
		case MSGFONTID_MYNAME:
		case MSGFONTID_MYTIME:
		case 21:
			wcsncpy(fid.backgroundName, LPGENW("Outgoing background"), _countof(fid.backgroundName));
			break;
		case 8:
		case 9:
		case 12:
		case 13:
			wcsncpy(fid.backgroundName, LPGENW("Outgoing background(old)"), _countof(fid.backgroundName));
			break;
		case 10:
		case 11:
		case 14:
		case 15:
			wcsncpy(fid.backgroundName, LPGENW("Incoming background(old)"), _countof(fid.backgroundName));
			break;
		case MSGFONTID_MESSAGEAREA:
			wcsncpy(fid.group, LPGENW("Message sessions"), _countof(fid.group));
			wcsncpy(fid.backgroundGroup, LPGENW("Message sessions"), _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, LPGENW("Input area background"), _countof(fid.backgroundName));
			break;
		case 17:
			fid.flags |= FIDF_DISABLESTYLES;
			fid.flags &= ~FIDF_ALLOWEFFECTS;
			wcsncpy(fid.backgroundName, LPGENW("Status background"), _countof(fid.backgroundName));
			break;
		case 18:
			wcsncpy(fid.backgroundGroup, LPGENW("Message sessions"), _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, LPGENW("Log background"), _countof(fid.backgroundName));
			break;
		case 19:
			wcsncpy(fid.backgroundName, L"", _countof(fid.backgroundName));
			break;
		default:
			wcsncpy(fid.backgroundName, LPGENW("Incoming background"), _countof(fid.backgroundName));
			break;
		}
		g_plugin.addFont(&fid);
	}

	fontOptionsList = IP_fontOptionsList;
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	wcsncpy(fid.group, LPGENW("Message sessions") L"/" LPGENW("Info Panel"), _countof(fid.group));
	wcsncpy(fid.backgroundGroup, LPGENW("Message sessions") L"/" LPGENW("Info Panel"), _countof(fid.backgroundGroup));
	wcsncpy(fid.backgroundName, LPGENW("Fields background"), _countof(fid.backgroundName));
	for (int i = 0; i < IPFONTCOUNT; i++) {
		LoadMsgDlgFont(FONTSECTION_IP, i, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(fid.prefix, "Font%d", i + 100);
		fid.order = i + 100;
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		fid.deffontsettings.charset = lf.lfCharSet;
		wcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		if (i == IPFONTCOUNT - 1) {
			wcsncpy(fid.backgroundGroup, L"", _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, L"", _countof(fid.backgroundName));
			wcsncpy(fid.group, LPGENW("Message sessions"), _countof(fid.group));
		}
		g_plugin.addFont(&fid);
	}

	wcsncpy(cid.group, LPGENW("Message sessions") L"/" LPGENW("Group chats"), _countof(cid.group));
	strncpy(cid.dbSettingsGroup, CHAT_MODULE, _countof(cid.dbSettingsGroup));
	for (int i = 0; i <= 7; i++) {
		mir_snprintf(cid.setting, "NickColor%d", i);
		wcsncpy(cid.name, chatcolorsnames[i], _countof(cid.name));
		cid.order = i + 1;
		switch (i) {
		case 5:
			cid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
			break;
		case 6:
			cid.defcolour = GetSysColor(COLOR_HIGHLIGHTTEXT);
			break;
		default:
			cid.defcolour = RGB(0, 0, 0);
			break;
		}
		g_plugin.addColor(&cid);
	}
	cid.order++;
	wcsncpy_s(cid.name, LPGENW("Nick list background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorNicklistBG", _TRUNCATE);
	cid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	g_plugin.addColor(&cid);

	cid.order++;
	wcsncpy_s(cid.name, LPGENW("Group chat log background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorLogBG", _TRUNCATE);
	g_plugin.addColor(&cid);

	// static colors (info panel, tool bar background etc...)
	strncpy(fid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));
	strncpy(cid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));

	for (auto &it : _clrs) {
		cid.order = it.order;
		wcsncpy(cid.group, it.tszGroup, _countof(fid.group));
		wcsncpy(cid.name, it.tszName, _countof(cid.name));
		strncpy(cid.setting, it.szSetting, _countof(cid.setting));
		if (it.def & 0xff000000)
			cid.defcolour = GetSysColor(it.def & 0x000000ff);
		else
			cid.defcolour = it.def;
		g_plugin.addColor(&cid);
	}

	strncpy(cid.dbSettingsGroup, SRMSGMOD_T, _countof(fid.dbSettingsGroup));

	// text and background colors for tabs
	for (auto &it : _tabclrs) {
		cid.order = it.order;
		wcsncpy(cid.group, it.tszGroup, _countof(fid.group));
		wcsncpy(cid.name, it.tszName, _countof(cid.name));
		strncpy(cid.setting, it.szSetting, _countof(cid.setting));
		if (it.def & 0xff000000)
			cid.defcolour = GetSysColor(it.def & 0x000000ff);
		else
			cid.defcolour = it.def;

		g_plugin.addColor(&cid);
	}
}

int FontServiceFontsChanged(WPARAM, LPARAM)
{
	PluginConfig.reloadSettings();
	CSkin::initAeroEffect();
	CacheMsgLogIcons();
	CacheLogFonts();
	FreeTabConfig();
	ReloadTabConfig();
	Skin->setupAeroSkins();
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM)
{
	char szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)M.getUserDir());
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDListA((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

static HWND CreateToolTip(HWND hwndParent, LPCWSTR ptszText, LPCTSTR ptszTitle)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, nullptr, g_plugin.getInst(), nullptr);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = g_plugin.getInst();
	ti.lpszText = (LPWSTR)ptszText;
	GetClientRect(hwndParent, &ti.rect);
	ti.rect.left = -65;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM)640);
	return hwndTT;
}

/////////////////////////////////////////////////////////////////////////////////////////
// base class for chat options

class CChatBaseOptionDlg : public CDlgBase
{

protected:
	CChatBaseOptionDlg(int iDlg) :
		CDlgBase(g_plugin, iDlg)
	{
		m_OnFinishWizard = Callback(this, &CChatBaseOptionDlg::onFinalOptions);
	}

	void onFinalOptions(void*)
	{
		g_chatApi.ReloadSettings();
		g_chatApi.MM_IconsChanged();
		g_chatApi.MM_FontsChanged();
		Chat_UpdateOptions();
		SM_ReconfigureFilters();

		PluginConfig.reloadSettings();
		CacheMsgLogIcons();
		CacheLogFonts();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat - Settings

class CChatSettingsDlg : public CChatBaseOptionDlg
{
	HTREEITEM hListHeading1 = nullptr;
	HTREEITEM hListHeading2 = nullptr;

	CCtrlTreeView treeCheck;

	HTREEITEM InsertBranch(wchar_t* pszDescr, BOOL bExpanded)
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.item.pszText = TranslateW(pszDescr);
		tvis.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
		tvis.item.state = (bExpanded ? TVIS_EXPANDED : 0) | TVIS_BOLD;
		tvis.item.iImage = tvis.item.iSelectedImage = (bExpanded ? IMG_GRPOPEN : IMG_GRPCLOSED);
		return treeCheck.InsertItem(&tvis);
	}

	void FillBranch(HTREEITEM hParent, branch_t *branch, size_t nValues, DWORD defaultval)
	{
		if (hParent == nullptr)
			return;

		TVINSERTSTRUCT tvis = {};

		for (int i = 0; i < nValues; i++) {
			tvis.hParent = hParent;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.item.pszText = TranslateW(branch[i].szDescr);
			if (branch[i].iMode)
				tvis.item.iImage = tvis.item.iSelectedImage = ((((M.GetDword(CHAT_MODULE, branch[i].szDBName, defaultval) & branch[i].iMode) & branch[i].iMode) != 0) ? IMG_CHECK : IMG_NOCHECK);
			else
				tvis.item.iImage = tvis.item.iSelectedImage = ((M.GetByte(CHAT_MODULE, branch[i].szDBName, branch[i].bDefault) != 0) ? IMG_CHECK : IMG_NOCHECK);
			branch[i].hItem = treeCheck.InsertItem(&tvis);
		}
	}

	void SaveBranch(branch_t *branch, int nValues)
	{
		TVITEMEX tvi = { 0 };
		BYTE bChecked;
		DWORD iState = 0;

		for (int i = 0; i < nValues; i++) {
			tvi.mask = TVIF_HANDLE | TVIF_IMAGE;
			tvi.hItem = branch[i].hItem;
			treeCheck.GetItem(&tvi);
			bChecked = ((tvi.iImage == IMG_CHECK) ? 1 : 0);
			if (branch[i].iMode) {
				if (bChecked)
					iState |= branch[i].iMode;
				if (iState & GC_EVENT_ADDSTATUS)
					iState |= GC_EVENT_REMOVESTATUS;
				db_set_dw(0, CHAT_MODULE, branch[i].szDBName, iState);
			}
			else db_set_b(0, CHAT_MODULE, branch[i].szDBName, bChecked);
		}
	}

public:
	CChatSettingsDlg() :
		CChatBaseOptionDlg(IDD_OPTIONS1),
		treeCheck(this, IDC_CHECKBOXES)
	{}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(treeCheck.GetHwnd(), GWL_STYLE, GetWindowLongPtr(treeCheck.GetHwnd(), GWL_STYLE) | (TVS_NOHSCROLL));
		
		// Replace image list
		treeCheck.SetImageList(CreateStateImageList(), TVSIL_NORMAL);

		hListHeading1 = InsertBranch(TranslateT("Appearance and functionality of chat room windows"), TRUE);
		hListHeading2 = InsertBranch(TranslateT("Appearance of the message log"), TRUE);

		FillBranch(hListHeading1, branch1, _countof(branch1), 0x0000);
		FillBranch(hListHeading2, branch2, _countof(branch2), 0x0000);

		ptrW pszGroup(db_get_wsa(0, CHAT_MODULE, "AddToGroup"));
		SetDlgItemText(m_hwnd, IDC_GROUP, (pszGroup != nullptr) ? pszGroup : TranslateT("Chat rooms"));
		return true;
	}

	bool OnApply() override
	{
		int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_GROUP));
		if (iLen > 0) {
			wchar_t *pszText = (wchar_t*)mir_alloc((iLen + 2) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_GROUP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "AddToGroup", pszText);
			mir_free(pszText);
		}
		else db_set_ws(0, CHAT_MODULE, "AddToGroup", L"");

		SaveBranch(branch1, _countof(branch1));
		SaveBranch(branch2, _countof(branch2));
		return true;
	}

	void OnDestroy() override
	{
		BYTE b = treeCheck.GetItemState(hListHeading1, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch1Exp", b);

		b = treeCheck.GetItemState(hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch2Exp", b);

		TreeViewDestroy(treeCheck.GetHwnd());
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == IDC_CHECKBOXES)
			return TreeViewHandleClick(m_hwnd, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat - Log

class CChatLogOptionDlg : public CChatBaseOptionDlg
{
	CTimer m_timer;
	CCtrlButton btnBaseDir, btnFontChoose;
	CCtrlCheck chkLogging;
	HWND hPathTip = nullptr;

public:
	CChatLogOptionDlg() :
		CChatBaseOptionDlg(IDD_OPTIONS2),
		m_timer(this, 1),
		btnBaseDir(this, IDC_MUC_OPENLOGBASEDIR),
		chkLogging(this, IDC_LOGGING),
		btnFontChoose(this, IDC_FONTCHOOSE)
	{
		btnBaseDir.OnClick = Callback(this, &CChatLogOptionDlg::onClick_BaseDir);
		btnFontChoose.OnClick = Callback(this, &CChatLogOptionDlg::onClick_FontChoose);

		chkLogging.OnChange = Callback(this, &CChatLogOptionDlg::onChange_Logging);

		m_timer.OnEvent = Callback(this, &CChatLogOptionDlg::onTimer);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LogLimit", 100), 0));
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN3, UDM_SETRANGE, 0, MAKELONG(255, 10));
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN3, UDM_SETPOS, 0, MAKELONG(M.GetByte(CHAT_MODULE, "NicklistRowDist", 12), 0));
		SetDlgItemText(m_hwnd, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(m_hwnd, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(m_hwnd, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(m_hwnd, IDC_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(m_hwnd, IDC_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, g_Settings.pszLogDir);
		Utils::enableDlgControl(m_hwnd, IDC_LOGDIRECTORY, g_Settings.bLoggingEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_FONTCHOOSE, g_Settings.bLoggingEnabled);
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN4, UDM_SETRANGE, 0, MAKELONG(10000, 0));
		SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN4, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100), 0));
		Utils::enableDlgControl(m_hwnd, IDC_LIMIT, g_Settings.bLoggingEnabled);

		CMStringW tszTooltipText(FORMAT,
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
			// contact vars
			L"%nick%", TranslateT("nick of current contact (if defined)"),
			L"%proto%", TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
			L"%accountname%", TranslateT("user-defined account name of current contact (if defined)."),
			L"%userid%", TranslateT("user ID of current contact (if defined). It is like UIN for ICQ, JID for Jabber, etc."),
			// global vars
			L"%miranda_path%", TranslateT("path to Miranda root folder"),
			L"%miranda_profilesdir%", TranslateT("path to folder containing Miranda profiles"),
			L"%miranda_profilename%", TranslateT("name of current Miranda profile (filename, without extension)"),
			L"%miranda_userdata%", TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
			L"%miranda_logpath%", TranslateT("will return parsed string %miranda_userdata%\\Logs"),
			L"%appdata%", TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
			L"%username%", TranslateT("username for currently logged-on Windows user"),
			L"%mydocuments%", TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
			L"%desktop%", TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
			L"%xxxxxxx%", TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"),
			// date/time vars
			L"%d%", TranslateT("day of month, 1-31"),
			L"%dd%", TranslateT("day of month, 01-31"),
			L"%m%", TranslateT("month number, 1-12"),
			L"%mm%", TranslateT("month number, 01-12"),
			L"%mon%", TranslateT("abbreviated month name"),
			L"%month%", TranslateT("full month name"),
			L"%yy%", TranslateT("year without century, 01-99"),
			L"%yyyy%", TranslateT("year with century, 1901-9999"),
			L"%wday%", TranslateT("abbreviated weekday name"),
			L"%weekday%", TranslateT("full weekday name"));

		hPathTip = CreateToolTip(GetDlgItem(m_hwnd, IDC_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"));
		if (hPathTip)
			m_timer.Start(3000);
		return true;
	}

	bool OnApply() override
	{
		char *pszText = nullptr;

		int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LOGDIRECTORY));
		if (iLen > 0) {
			wchar_t *pszText1 = (wchar_t*)mir_alloc(iLen * sizeof(wchar_t) + 2);
			GetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, pszText1, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText1);
			mir_free(pszText1);
			g_Settings.bLoggingEnabled = IsDlgButtonChecked(m_hwnd, IDC_LOGGING) == BST_CHECKED;
			db_set_b(0, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);
		}
		else {
			db_unset(0, CHAT_MODULE, "LogDirectory");
			db_set_b(0, CHAT_MODULE, "LoggingEnabled", 0);
		}
		g_chatApi.SM_InvalidateLogDirectories();

		iLen = SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN4, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

		iLen = SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
		if (iLen > 0)
			db_set_b(0, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
		else
			db_unset(0, CHAT_MODULE, "NicklistRowDist");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LOGTIMESTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_LOGTIMESTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "LogTimestamp", pszText);
		}
		else db_unset(0, CHAT_MODULE, "LogTimestamp");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_TIMESTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_TIMESTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderTime", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderTime");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_INSTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_INSTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderIncoming", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderIncoming");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_OUTSTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_OUTSTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderOutgoing", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderOutgoing");

		iLen = SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LogLimit", (WORD)iLen);
		mir_free(pszText);

		if (g_chatApi.hListBkgBrush)
			DeleteObject(g_chatApi.hListBkgBrush);
		g_chatApi.hListBkgBrush = CreateSolidBrush(M.GetDword(CHAT_MODULE, "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR));
		return true;
	}

	void OnDestroy() override
	{
		if (hPathTip) {
			KillTimer(m_hwnd, 0);
			DestroyWindow(hPathTip);
			hPathTip = nullptr;
		}
	}

	// open the base directory for MUC logs, using a standard file selector
	// dialog. Simply allows the user to view what log files are there
	// and possibly delete archived logs.

	void onClick_BaseDir(CCtrlButton*)
	{
		wchar_t	tszTemp[MAX_PATH + 20];
		wcsncpy_s(tszTemp, g_Settings.pszLogDir, _TRUNCATE);

		wchar_t *p = tszTemp;
		while (*p && (*p == '\\' || *p == '.'))
			p++;

		if (*p)
			if (wchar_t *p1 = wcschr(p, '\\'))
				*p1 = 0;

		wchar_t tszInitialDir[_MAX_DRIVE + _MAX_PATH + 10];
		mir_snwprintf(tszInitialDir, L"%s%s", M.getChatLogPath(), p);
		if (!PathFileExists(tszInitialDir))
			wcsncpy_s(tszInitialDir, M.getChatLogPath(), _TRUNCATE);

		wchar_t	tszReturnName[MAX_PATH]; tszReturnName[0] = 0;
		mir_snwprintf(tszTemp, L"%s%c*.*%c%c", TranslateT("All files"), 0, 0, 0);

		OPENFILENAME ofn = { 0 };
		ofn.lpstrInitialDir = tszInitialDir;
		ofn.lpstrFilter = tszTemp;
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.lpstrFile = tszReturnName;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
		ofn.lpstrDefExt = L"log";
		GetOpenFileName(&ofn);
	}

	void onClick_FontChoose(CCtrlButton*)
	{
		wchar_t tszDirectory[MAX_PATH];

		LPMALLOC psMalloc;
		if (FAILED(CoGetMalloc(1, &psMalloc)))
			return;

		BROWSEINFO bi = {};
		bi.hwndOwner = m_hwnd;
		bi.pszDisplayName = tszDirectory;
		bi.lpszTitle = TranslateT("Select folder");
		bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)tszDirectory;

		LPITEMIDLIST idList = SHBrowseForFolder(&bi);
		if (idList) {
			const wchar_t *szUserDir = M.getUserDir();
			SHGetPathFromIDList(idList, tszDirectory);
			mir_wstrcat(tszDirectory, L"\\");

			wchar_t tszTemp[MAX_PATH];
			PathToRelativeW(tszDirectory, tszTemp, szUserDir);
			SetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, mir_wstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
		}
		psMalloc->Free(idList);
		psMalloc->Release();
	}

	void onChange_Logging(CCtrlCheck*)
	{
		bool bChecked = chkLogging.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_LOGDIRECTORY, bChecked);
		Utils::enableDlgControl(m_hwnd, IDC_FONTCHOOSE, bChecked);
		Utils::enableDlgControl(m_hwnd, IDC_LIMIT, bChecked);
	}

	void onTimer(CTimer*)
	{
		if (IsWindow(hPathTip))
			KillTimer(hPathTip, 4); // It will prevent tooltip autoclosing
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat - Events

#define NR_GC_EVENTS 12

static UINT _eventorder[] =
{
	GC_EVENT_ACTION,
	GC_EVENT_MESSAGE,
	GC_EVENT_NICK,
	GC_EVENT_JOIN,
	GC_EVENT_PART,
	GC_EVENT_TOPIC,
	GC_EVENT_ADDSTATUS,
	GC_EVENT_INFORMATION,
	GC_EVENT_QUIT,
	GC_EVENT_KICK,
	GC_EVENT_NOTICE,
	GC_EVENT_HIGHLIGHT
};

class CChatEventOptionDlg : public CChatBaseOptionDlg
{

public:
	CChatEventOptionDlg() :
		CChatBaseOptionDlg(IDD_OPTIONS3)
	{}

	bool OnInitDialog() override
	{
		DWORD dwFilterFlags = M.GetDword(CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
		DWORD dwTrayFlags = M.GetDword(CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
		DWORD dwPopupFlags = M.GetDword(CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
		DWORD dwLogFlags = M.GetDword(CHAT_MODULE, "DiskLogFlags", GC_EVENT_ALL);

		for (int i = 0; i < _countof(_eventorder); i++) {
			if (_eventorder[i] != GC_EVENT_HIGHLIGHT) {
				CheckDlgButton(m_hwnd, IDC_1 + i, dwFilterFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(m_hwnd, IDC_L1 + i, dwLogFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
			}
			CheckDlgButton(m_hwnd, IDC_P1 + i, dwPopupFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_T1 + i, dwTrayFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
		}

		SendDlgItemMessage(m_hwnd, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("No markers"));
		SendDlgItemMessage(m_hwnd, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("Show as icons"));
		SendDlgItemMessage(m_hwnd, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("Show as text symbols"));

		SendDlgItemMessage(m_hwnd, IDC_LOGICONTYPE, CB_SETCURSEL, (g_Settings.bLogSymbols ? 2 : (g_Settings.dwIconFlags ? 1 : 0)), 0);

		CheckDlgButton(m_hwnd, IDC_TRAYONLYFORINACTIVE, M.GetByte(CHAT_MODULE, "TrayIconInactiveOnly", 0) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		DWORD dwFilterFlags = 0, dwTrayFlags = 0,
			dwPopupFlags = 0, dwLogFlags = 0;

		for (int i = 0; i < _countof(_eventorder); i++) {
			if (_eventorder[i] != GC_EVENT_HIGHLIGHT) {
				dwFilterFlags |= (IsDlgButtonChecked(m_hwnd, IDC_1 + i) ? _eventorder[i] : 0);
				dwLogFlags |= (IsDlgButtonChecked(m_hwnd, IDC_L1 + i) ? _eventorder[i] : 0);
			}
			dwPopupFlags |= (IsDlgButtonChecked(m_hwnd, IDC_P1 + i) ? _eventorder[i] : 0);
			dwTrayFlags |= (IsDlgButtonChecked(m_hwnd, IDC_T1 + i) ? _eventorder[i] : 0);
		}
		db_set_dw(0, CHAT_MODULE, "FilterFlags", dwFilterFlags);
		db_set_dw(0, CHAT_MODULE, "PopupFlags", dwPopupFlags);
		db_set_dw(0, CHAT_MODULE, "TrayIconFlags", dwTrayFlags);
		db_set_dw(0, CHAT_MODULE, "DiskLogFlags", dwLogFlags);

		LRESULT lr = SendDlgItemMessage(m_hwnd, IDC_LOGICONTYPE, CB_GETCURSEL, 0, 0);

		db_set_dw(0, CHAT_MODULE, "IconFlags", lr == 1 ? 1 : 0);
		db_set_b(0, CHAT_MODULE, "LogSymbols", lr == 2 ? 1 : 0);

		db_set_b(0, CHAT_MODULE, "TrayIconInactiveOnly", IsDlgButtonChecked(m_hwnd, IDC_TRAYONLYFORINACTIVE) ? 1 : 0);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat - Highlight

class CHighlighOptionDlg : public CChatBaseOptionDlg
{
	CCtrlEdit edtNick, edtText;
	CCtrlCheck chkNick, chkText;

public:
	CHighlighOptionDlg() :
		CChatBaseOptionDlg(IDD_OPTIONS4),
		edtNick(this, IDC_HIGHLIGHTNICKPATTERN),
		edtText(this, IDC_HIGHLIGHTNICKPATTERN),
		chkNick(this, IDC_HIGHLIGHTNICKENABLE),
		chkText(this, IDC_HIGHLIGHTTEXTENABLE)
	{
		chkNick.OnChange = Callback(this, &CHighlighOptionDlg::onChange_Nick);
		chkText.OnChange = Callback(this, &CHighlighOptionDlg::onChange_Text);
	}

	bool OnInitDialog() override
	{
		ptrW wszText(db_get_wsa(0, CHAT_MODULE, "HighlightWords"));
		if (wszText)
			edtText.SetText(wszText);

		wszText = db_get_wsa(0, CHAT_MODULE, "HighlightNames");
		if (wszText)
			edtNick.SetText(wszText);

		DWORD dwFlags = M.GetByte(CHAT_MODULE, "HighlightEnabled", CMUCHighlight::MATCH_TEXT);

		chkNick.SetState(dwFlags & CMUCHighlight::MATCH_NICKNAME); onChange_Nick(0);
		chkText.SetState(dwFlags & CMUCHighlight::MATCH_TEXT); onChange_Text(0);
		
		::CheckDlgButton(m_hwnd, IDC_HIGHLIGHTNICKUID, dwFlags & CMUCHighlight::MATCH_UIN ? BST_CHECKED : BST_UNCHECKED);
		::CheckDlgButton(m_hwnd, IDC_HIGHLIGHTME, M.GetByte(CHAT_MODULE, "HighlightMe", 1) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	void onChange_Text(CCtrlCheck*) 
	{
		bool bChecked = chkText.GetState();
		edtText.Enable(bChecked);
		Utils::enableDlgControl(m_hwnd, IDC_HIGHLIGHTME, bChecked);
	}

	void onChange_Nick(CCtrlCheck*)
	{
		bool bChecked = chkNick.GetState();
		edtNick.Enable(bChecked);
		Utils::enableDlgControl(m_hwnd, IDC_HIGHLIGHTNICKUID, chkNick.GetState());
	}

	bool OnApply() override
	{
		db_set_ws(0, CHAT_MODULE, "HighlightNames", ptrW(edtNick.GetText()));
		db_set_ws(0, CHAT_MODULE, "HighlightWords", ptrW(edtText.GetText()));

		BYTE dwFlags = (chkNick.GetState() ? CMUCHighlight::MATCH_NICKNAME : 0) | (chkText.GetState() ? CMUCHighlight::MATCH_TEXT : 0);
		if (dwFlags & CMUCHighlight::MATCH_NICKNAME)
			dwFlags |= (::IsDlgButtonChecked(m_hwnd, IDC_HIGHLIGHTNICKUID) ? CMUCHighlight::MATCH_UIN : 0);

		db_set_b(0, CHAT_MODULE, "HighlightEnabled", dwFlags);
		db_set_b(0, CHAT_MODULE, "HighlightMe", ::IsDlgButtonChecked(m_hwnd, IDC_HIGHLIGHTME) ? 1 : 0);
		g_Settings.Highlight->init();
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

void Chat_Options(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 910000000;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Group chats");
	
	odp.szTab.a = LPGEN("Settings");
	odp.pDialog = new CChatSettingsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Log formatting");
	odp.pDialog = new CChatLogOptionDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Events and filters");
	odp.pDialog = new CChatEventOptionDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS4);
	odp.szTab.a = LPGEN("Highlighting");
	odp.pDialog = new CHighlighOptionDlg();
	g_plugin.addOptions(wParam, &odp);
}
