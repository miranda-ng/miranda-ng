/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "chat.h"

#include <m_fontservice.h>

extern SESSION_INFO g_TabSession;

HICON g_hChatIcons[20];
uint32_t g_dwDiskLogFlags;
HPLUGIN g_pChatPlugin;
GlobalLogSettingsBase *g_Settings;
int g_cbSession, g_cbModuleInfo = sizeof(MODULEINFO), g_iFontMode;
wchar_t *g_szFontGroup;

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct FontOptionsList
{
	LPCTSTR  szDescr;
	COLORREF defColour;
	LPCTSTR  szDefFace;
	uint8_t     defCharset, defStyle; 
	char     defSize;
};

//remeber to put these in the Translate( ) template file too

static LOGFONT lfDefault;

static FontOptionsList fontOptionsList[] =
{
	{ LPGENW("Timestamp"),                    RGB(50, 50, 240),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Others nicknames"),             RGB(0, 0, 192),     lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("Your nickname"),                RGB(0, 0, 192),     lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("User has joined"),              RGB(90, 160, 90),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User has left"),                RGB(160, 160, 90),  lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User has disconnected"),        RGB(160, 90, 90),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User kicked ..."),              RGB(100, 100, 100), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User is now known as ..."),     RGB(90, 90, 160),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Notice from user"),             RGB(160, 130, 60),  lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Incoming message"),             RGB(90, 90, 90),    lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Outgoing message"),             RGB(90, 90, 90),    lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("The topic is ..."),             RGB(70, 70, 160),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Information messages"),         RGB(130, 130, 195), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User enables status for ..."),  RGB(70, 150, 70),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("User disables status for ..."), RGB(150, 70, 70),   lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Action message"),               RGB(160, 90, 160),  lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Highlighted message"),          RGB(180, 150, 80),  lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ L"",                                    0,                  lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Nick list members (online)"),   RGB(0, 0, 0),       lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Nick list members (away)"),     RGB(170, 170, 170), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 }
};

static void LoadColors()
{
	g_Settings->crUserListBGColor = db_get_dw(0, CHAT_MODULE, "ColorNicklistBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crUserListSelectedBGColor = db_get_dw(0, CHAT_MODULE, "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT));
}

void LoadLogFonts(void)
{
	for (int i=0; i < OPTIONS_FONTCOUNT; i++)
		LoadMsgDlgFont(i, &g_chatApi.aFonts[i].lf, &g_chatApi.aFonts[i].color);
	LoadColors();

	if (g_chatApi.hListBkgBrush != nullptr)
		DeleteObject(g_chatApi.hListBkgBrush);
	g_chatApi.hListBkgBrush = CreateSolidBrush(g_Settings->crUserListBGColor);

	if (g_chatApi.hListSelectedBkgBrush != nullptr)
		DeleteObject(g_chatApi.hListSelectedBkgBrush);
	g_chatApi.hListSelectedBkgBrush = CreateSolidBrush(g_Settings->crUserListSelectedBGColor);
}

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour)
{
	char str[32];
	int style;
	FontOptionsList &FO = fontOptionsList[i];

	if (colour) {
		mir_snprintf(str, "Font%dCol", i);
		*colour = db_get_dw(0, CHATFONT_MODULE, str, FO.defColour);
	}
	if (lf) {
		mir_snprintf(str, "Font%dSize", i);
		lf->lfHeight = (char)db_get_b(0, CHATFONT_MODULE, str, Utils_CorrectFontSize(FO.defSize));
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "Font%dSty", i);
		style = db_get_b(0, CHATFONT_MODULE, str, FO.defStyle);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, "Font%dSet", i);
		lf->lfCharSet = db_get_b(0, CHATFONT_MODULE, str, FO.defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "Font%d", i);

		wcsncpy_s(lf->lfFaceName, ptrW(db_get_wsa(0, CHATFONT_MODULE, str, FO.szDefFace)), _TRUNCATE);
	}
}

void RegisterFonts(void)
{
	int index = 0;

	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfDefault), &lfDefault, FALSE);

	FontIDW fontid = {};
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_NEEDRESTART;
	wcsncpy_s(fontid.backgroundGroup, g_szFontGroup, _TRUNCATE);
	wcsncpy_s(fontid.group, g_szFontGroup, _TRUNCATE);

	for (int i = 0; i < _countof(fontOptionsList); i++, index++) {
		FontOptionsList &FO = fontOptionsList[i];
		strncpy_s(fontid.dbSettingsGroup, CHATFONT_MODULE, _TRUNCATE);
		wcsncpy_s(fontid.name, FO.szDescr, _TRUNCATE);

		mir_snprintf(fontid.setting, "Font%d", index);
		fontid.order = index;

		switch (i) {
		case 18:
		case 19:
			wcsncpy_s(fontid.backgroundName, LPGENW("Nick list background"), _TRUNCATE);
			break;
		case 17:
			if (g_iFontMode == FONTMODE_SKIP)
				continue;
			if (g_iFontMode == FONTMODE_USE) {
				wcsncpy_s(fontid.name, LPGENW("Message typing area"), _TRUNCATE);
				wcsncpy_s(fontid.backgroundName, LPGENW("Message background"), _TRUNCATE);
				FO.defColour = RGB(0, 0, 40);
				break;
			}

			wcsncpy_s(fontid.name, LPGENW("Chat log symbols (Webdings)"), _TRUNCATE);
			FO.szDefFace = L"Webdings";
			FO.defColour = RGB(170, 170, 170);
			FO.defCharset = SYMBOL_CHARSET;
			__fallthrough;

		default:
			wcsncpy_s(fontid.backgroundName, LPGENW("Group chat log background"), _TRUNCATE);
			break;
		}
		wcsncpy_s(fontid.deffontsettings.szFace, FO.szDefFace, _TRUNCATE);
		fontid.deffontsettings.charset = FO.defCharset;
		fontid.deffontsettings.colour = FO.defColour;
		fontid.deffontsettings.size = Utils_CorrectFontSize(FO.defSize);
		fontid.deffontsettings.style = FO.defStyle;
		Font_RegisterW(&fontid, g_pChatPlugin);
	}
}

// load icons from the skinning module if available

HICON LoadIconEx(char* pszIcoLibName, bool big)
{
	char szTemp[256];
	mir_snprintf(szTemp, "chat_%s", pszIcoLibName);
	return IcoLib_GetIcon(szTemp, big);
}

static void InitSetting(wchar_t** ppPointer, char* pszSetting, wchar_t* pszDefault)
{
	DBVARIANT dbv;
	if (!db_get_ws(0, CHAT_MODULE, pszSetting, &dbv)) {
		replaceStrW(*ppPointer, dbv.pwszVal);
		db_free(&dbv);
	}
	else replaceStrW(*ppPointer, pszDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadGlobalSettings(void)
{
	g_Settings->LogIconSize = 10;
	g_Settings->bSoundsFocus = db_get_b(0, CHAT_MODULE, "SoundsFocus", 0) != 0;
	g_Settings->iEventLimit = db_get_w(0, CHAT_MODULE, "LogLimit", 100);
	g_Settings->dwIconFlags = db_get_dw(0, CHAT_MODULE, "IconFlags", 0x0000);
	g_Settings->dwTrayIconFlags = db_get_dw(0, CHAT_MODULE, "TrayIconFlags", 0x1000);
	g_Settings->dwPopupFlags = db_get_dw(0, CHAT_MODULE, "PopupFlags", 0x0000);
	g_Settings->LoggingLimit = db_get_w(0, CHAT_MODULE, "LoggingLimit", 100);
	g_Settings->bLoggingEnabled = db_get_b(0, CHAT_MODULE, "LoggingEnabled", 0) != 0;
	g_Settings->bHighlightEnabled = db_get_b(0, CHAT_MODULE, "HighlightEnabled", 1) != 0;
	g_Settings->crLogBackground = db_get_dw(0, CHAT_MODULE, "ColorLogBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crUserListColor = db_get_dw(0, CHATFONT_MODULE, "Font18Col", RGB(0, 0, 0));
	g_Settings->crUserListHeadingsColor = db_get_dw(0, CHATFONT_MODULE, "Font19Col", RGB(170, 170, 170));
	g_Settings->iPopupStyle = db_get_b(0, CHAT_MODULE, "PopupStyle", 1);
	g_Settings->iPopupTimeout = db_get_w(0, CHAT_MODULE, "PopupTimeout", 3);
	g_Settings->crPUBkgColour = db_get_dw(0, CHAT_MODULE, "PopupColorBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crPUTextColour = db_get_dw(0, CHAT_MODULE, "PopupColorText", 0);

	g_Settings->bTrayIconInactiveOnly = g_bChatTrayInactive;
	g_Settings->bPopupInactiveOnly = g_bChatPopupInactive;

	g_Settings->bShowTime = Chat::bShowTime;
	g_Settings->bStripFormat = Chat::bStripFormat;
	g_Settings->bLogLimitNames = Chat::bLogLimitNames;
	g_Settings->bLogIndentEnabled = Chat::bLogIndentEnabled;
	g_Settings->bShowTimeIfChanged = Chat::bShowTimeIfChanged;
	g_Settings->bShowContactStatus = Chat::bShowContactStatus;
	g_Settings->bContactStatusFirst = Chat::bContactStatusFirst;
	g_Settings->bTimeStampEventColour = Chat::bTimeStampEventColour;

	LoadColors();

	if (g_chatApi.OnLoadSettings)
		g_chatApi.OnLoadSettings();

	InitSetting(&g_Settings->pszTimeStamp, "HeaderTime", L"[%H:%M]");
	InitSetting(&g_Settings->pszTimeStampLog, "LogTimestamp", L"[%d %b %y %H:%M]");
	InitSetting(&g_Settings->pszIncomingNick, "HeaderIncoming", L"%n:");
	InitSetting(&g_Settings->pszOutgoingNick, "HeaderOutgoing", L"%n:");
	InitSetting(&g_Settings->pszHighlightWords, "HighlightWords", L"%m");

	InitSetting(&g_Settings->pszLogDir, "LogDirectory", L"%miranda_logpath%\\%proto%\\%userid%.log");

	LOGFONT lf;
	if (g_Settings->UserListFont)
		DeleteObject(g_Settings->UserListFont);
	LoadMsgDlgFont(18, &lf, nullptr);
	g_Settings->UserListFont = CreateFontIndirect(&lf);

	if (g_Settings->UserListHeadingsFont)
		DeleteObject(g_Settings->UserListHeadingsFont);
	LoadMsgDlgFont(19, &lf, nullptr);
	g_Settings->UserListHeadingsFont = CreateFontIndirect(&lf);

	SetIndentSize();

	g_dwDiskLogFlags = db_get_dw(0, CHAT_MODULE, "DiskLogFlags", GC_EVENT_ALL);
}

static void FreeGlobalSettings(void)
{
	if (g_Settings == nullptr)
		return;

	mir_free(g_Settings->pszTimeStamp);
	mir_free(g_Settings->pszTimeStampLog);
	mir_free(g_Settings->pszIncomingNick);
	mir_free(g_Settings->pszOutgoingNick);
	mir_free(g_Settings->pszHighlightWords);
	mir_free(g_Settings->pszLogDir);
	if (g_Settings->UserListFont)
		DeleteObject(g_Settings->UserListFont);
	if (g_Settings->UserListHeadingsFont)
		DeleteObject(g_Settings->UserListHeadingsFont);
	if (g_Settings->NameFont)
		DeleteObject(g_Settings->NameFont);
}

void SetIndentSize()
{
	if (g_Settings->bShowTime) {
		LOGFONT lf;
		LoadMsgDlgFont(0, &lf, nullptr);
		HFONT hFont = CreateFontIndirect(&lf);
		int iText = Chat_GetTextPixelSize(MakeTimeStamp(g_Settings->pszTimeStamp, time(0)), hFont, TRUE);
		DeleteObject(hFont);
		g_Settings->LogTextIndent = iText * 12 / 10;
	}
	else g_Settings->LogTextIndent = 0;
}

int Chat_GetTextPixelSize(wchar_t* pszText, HFONT hFont, BOOL bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(nullptr);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = {};
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(nullptr, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

int OptionsInit(void)
{
	LoadLogFonts();

	LOGFONT lf;
	LoadMsgDlgFont(18, &lf, nullptr);
	mir_wstrcpy(lf.lfFaceName, L"MS Shell Dlg");
	lf.lfUnderline = lf.lfItalic = lf.lfStrikeOut = 0;
	lf.lfHeight = -17;
	lf.lfWeight = FW_BOLD;
	g_Settings->NameFont = CreateFontIndirect(&lf);
	g_Settings->UserListFont = nullptr;
	g_Settings->UserListHeadingsFont = nullptr;
	g_Settings->iWidth = db_get_dw(0, CHAT_MODULE, "roomwidth", -1);
	g_Settings->iHeight = db_get_dw(0, CHAT_MODULE, "roomheight", -1);
	return 0;
}

int OptionsUnInit(void)
{
	FreeGlobalSettings();
	return 0;
}
