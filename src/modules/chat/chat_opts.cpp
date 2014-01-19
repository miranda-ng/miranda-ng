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

#include "..\..\core\commonheaders.h"

#include "chat.h"

#include <m_fontservice.h>

extern SESSION_INFO g_TabSession;

GlobalLogSettingsBase *g_Settings;
int g_cbSession, g_cbModuleInfo, g_iFontMode, g_iChatLang;
TCHAR *g_szFontGroup;

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
struct FontOptionsList
{
	LPCTSTR  szDescr;
	COLORREF defColour;
	LPCTSTR  szDefFace;
	BYTE     defCharset, defStyle; 
	char     defSize;
}

//remeber to put these in the Translate( ) template file too
static const fontOptionsList[] = {
	{ LPGENT("Timestamp"),                    RGB(50, 50, 240),   _T("Terminal"), DEFAULT_CHARSET, 0, -8 },
	{ LPGENT("Others nicknames"),             RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("Your nickname"),                RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("User has joined"),              RGB(90, 160, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User has left"),                RGB(160, 160, 90),  _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User has disconnected"),        RGB(160, 90, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User kicked ..."),              RGB(100, 100, 100), _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User is now known as ..."),     RGB(90, 90, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Notice from user"),             RGB(160, 130, 60),  _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Incoming message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Outgoing message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("The topic is ..."),             RGB(70, 70, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Information messages"),         RGB(130, 130, 195), _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User enables status for ..."),  RGB(70, 150, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User disables status for ..."), RGB(150, 70, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Action message"),               RGB(160, 90, 160),  _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Highlighted message"),          RGB(180, 150, 80),  _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ _T(""),                                 0,                  _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User list members (online)"),   RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("User list members (away)"),     RGB(170, 170, 170), _T("Verdana"), DEFAULT_CHARSET, 0, -12 }
};

static void LoadColors()
{
	g_Settings->crLogBackground = db_get_dw(NULL, "Chat", "ColorLogBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crUserListBGColor = db_get_dw(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crUserListSelectedBGColor = db_get_dw(NULL, "Chat", "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT));
}

void LoadLogFonts(void)
{
	for (int i=0; i < OPTIONS_FONTCOUNT; i++)
		LoadMsgDlgFont(i, &ci.aFonts[i].lf, &ci.aFonts[i].color);
	LoadColors();

	if (ci.hListBkgBrush != NULL)
		DeleteObject(ci.hListBkgBrush);
	ci.hListBkgBrush = CreateSolidBrush(g_Settings->crUserListBGColor);

	if (ci.hListSelectedBkgBrush != NULL)
		DeleteObject(ci.hListSelectedBkgBrush);
	ci.hListSelectedBkgBrush = CreateSolidBrush(g_Settings->crUserListSelectedBGColor);
}

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour)
{
	char str[32];
	int style;

	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font%dCol", i);
		*colour = db_get_dw(NULL, "ChatFonts", str, fontOptionsList[i].defColour);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "Font%dSize", i);
		lf->lfHeight = (char)db_get_b(NULL, "ChatFonts", str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSty", i);
		style = db_get_b(NULL, "ChatFonts", str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSet", i);
		lf->lfCharSet = db_get_b(NULL, "ChatFonts", str, fontOptionsList[i].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font%d", i);

		ptrT tszFace(db_get_tsa(NULL, "ChatFonts", str));
		if (tszFace == NULL)
			lstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
		else
			_tcsncpy_s(lf->lfFaceName, SIZEOF(lf->lfFaceName), tszFace, _TRUNCATE);
	}
}

void RegisterFonts(void)
{
	int index = 0;

	FontIDT fontid = { sizeof(fontid) };
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_NEEDRESTART;
	for (int i = 0; i < SIZEOF(fontOptionsList); i++, index++) {
		strncpy(fontid.dbSettingsGroup, "ChatFonts", sizeof(fontid.dbSettingsGroup));
		_tcsncpy(fontid.group, g_szFontGroup, SIZEOF(fontid.group));
		_tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));

		char idstr[10];
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", index);
		strncpy(fontid.prefix, idstr, sizeof(fontid.prefix));
		fontid.order = index;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));
		_tcsncpy(fontid.backgroundGroup, g_szFontGroup, SIZEOF(fontid.backgroundGroup));
		switch (i) {
		case 18:
		case 19:
			_tcsncpy_s(fontid.backgroundName, SIZEOF(fontid.backgroundName), LPGENT("Userlist background"), _TRUNCATE);
			break;
		case 17:
			if (g_iFontMode == FONTMODE_SKIP)
				continue;
			if (g_iFontMode == FONTMODE_USE) {
				_tcsncpy_s(fontid.name, SIZEOF(fontid.name), LPGENT("Message typing area"), _TRUNCATE);
				_tcsncpy_s(fontid.backgroundName, SIZEOF(fontid.backgroundName), LPGENT("Message background"), _TRUNCATE);
				fontid.deffontsettings.colour = RGB(0, 0, 40);
				break;
			}

			_tcsncpy_s(fontid.name, SIZEOF(fontid.name), LPGENT("Chat log symbols (Webdings)"), _TRUNCATE);
			fontid.deffontsettings.colour = RGB(170, 170, 170);
			// fall through
		default:
			_tcsncpy_s(fontid.backgroundName, SIZEOF(fontid.backgroundName), LPGENT("Group chat log background"), _TRUNCATE);
			break;
		}
		CallService("Font/RegisterW", (WPARAM)&fontid, g_iChatLang);
	}
}

// load icons from the skinning module if available

HICON LoadIconEx(char* pszIcoLibName, BOOL big)
{
	char szTemp[256];
	mir_snprintf(szTemp, SIZEOF(szTemp), "chat_%s", pszIcoLibName);
	return Skin_GetIcon(szTemp, big);
}

static void InitSetting(TCHAR** ppPointer, char* pszSetting, TCHAR* pszDefault)
{
	DBVARIANT dbv;
	if (!db_get_ts(NULL, "Chat", pszSetting, &dbv)) {
		replaceStrT(*ppPointer, dbv.ptszVal);
		db_free(&dbv);
	}
	else replaceStrT(*ppPointer, pszDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadGlobalSettings(void)
{
	g_Settings->bLogLimitNames = db_get_b(NULL, "Chat", "bLogLimitNames", 1) != 0;
	g_Settings->bShowTime = db_get_b(NULL, "Chat", "ShowTimeStamp", 1) != 0;
	g_Settings->bSoundsFocus = db_get_b(NULL, "Chat", "bSoundsFocus", 0) != 0;
	g_Settings->bShowTimeIfChanged = (BOOL)db_get_b(NULL, "Chat", "ShowTimeStampIfChanged", 0) != 0;
	g_Settings->bTimeStampEventColour = (BOOL)db_get_b(NULL, "Chat", "bTimeStampEventColour", 0) != 0;
	g_Settings->iEventLimit = db_get_w(NULL, "Chat", "LogLimit", 100);
	g_Settings->dwIconFlags = db_get_dw(NULL, "Chat", "IconFlags", 0x0000);
	g_Settings->dwTrayIconFlags = db_get_dw(NULL, "Chat", "TrayIconFlags", 0x1000);
	g_Settings->dwPopupFlags = db_get_dw(NULL, "Chat", "PopupFlags", 0x0000);
	g_Settings->LoggingLimit = db_get_w(NULL, "Chat", "LoggingLimit", 100);
	g_Settings->bLoggingEnabled = (BOOL)db_get_b(NULL, "Chat", "bLoggingEnabled", 0) != 0;
	g_Settings->bFlashWindow = (BOOL)db_get_b(NULL, "Chat", "FlashWindow", 0) != 0;
	g_Settings->bFlashWindowHighlight = (BOOL)db_get_b(NULL, "Chat", "bFlashWindowHighlight", false) != 0;
	g_Settings->bHighlightEnabled = (BOOL)db_get_b(NULL, "Chat", "bHighlightEnabled", 1) != 0;
	g_Settings->crUserListColor = db_get_dw(NULL, "ChatFonts", "Font18Col", RGB(0, 0, 0));
	g_Settings->crUserListHeadingsColor = db_get_dw(NULL, "ChatFonts", "Font19Col", RGB(170, 170, 170));
	g_Settings->bStripFormat = (BOOL)db_get_b(NULL, "Chat", "StripFormatting", 0) != 0;
	g_Settings->bTrayIconInactiveOnly = (BOOL)db_get_b(NULL, "Chat", "bTrayIconInactiveOnly", 1) != 0;
	g_Settings->bPopupInactiveOnly = (BOOL)db_get_b(NULL, "Chat", "PopupInactiveOnly", 1) != 0;
	g_Settings->bAddColonToAutoComplete = (BOOL)db_get_b(NULL, "Chat", "AddColonToAutoComplete", 1) != 0;
	g_Settings->iPopupStyle = db_get_b(NULL, "Chat", "PopupStyle", 1);
	g_Settings->iPopupTimeout = db_get_w(NULL, "Chat", "PopupTimeout", 3);
	g_Settings->crPUBkgColour = db_get_dw(NULL, "Chat", "PopupColorBG", GetSysColor(COLOR_WINDOW));
	g_Settings->crPUTextColour = db_get_dw(NULL, "Chat", "PopupColorText", 0);
	g_Settings->bShowContactStatus = db_get_b(NULL, "Chat", "ShowContactStatus", 0) != 0;
	g_Settings->bContactStatusFirst = db_get_b(NULL, "Chat", "ContactStatusFirst", 0) != 0;

	LoadColors();

	if (ci.OnLoadSettings)
		ci.OnLoadSettings();

	InitSetting(&g_Settings->pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	InitSetting(&g_Settings->pszTimeStampLog, "LogTimestamp", _T("[%d %b %y %H:%M]"));
	InitSetting(&g_Settings->pszIncomingNick, "HeaderIncoming", _T("%n:"));
	InitSetting(&g_Settings->pszOutgoingNick, "HeaderOutgoing", _T("%n:"));
	InitSetting(&g_Settings->pszHighlightWords, "HighlightWords", _T("%m"));

	InitSetting(&g_Settings->pszLogDir, "LogDirectory", _T("%miranda_logpath%\\%proto%\\%userid%.log"));
	g_Settings->bLogIndentEnabled = db_get_b(NULL, "Chat", "bLogIndentEnabled", 1) != 0;

	LOGFONT lf;
	if (g_Settings->MessageBoxFont)
		DeleteObject(g_Settings->MessageBoxFont);
	LoadMsgDlgFont(17, &lf, NULL);
	g_Settings->MessageBoxFont = CreateFontIndirect(&lf);

	if (g_Settings->UserListFont)
		DeleteObject(g_Settings->UserListFont);
	LoadMsgDlgFont(18, &lf, NULL);
	g_Settings->UserListFont = CreateFontIndirect(&lf);

	if (g_Settings->UserListHeadingsFont)
		DeleteObject(g_Settings->UserListHeadingsFont);
	LoadMsgDlgFont(19, &lf, NULL);
	g_Settings->UserListHeadingsFont = CreateFontIndirect(&lf);

	SetIndentSize();
}

static void FreeGlobalSettings(void)
{
	mir_free(g_Settings->pszTimeStamp);
	mir_free(g_Settings->pszTimeStampLog);
	mir_free(g_Settings->pszIncomingNick);
	mir_free(g_Settings->pszOutgoingNick);
	mir_free(g_Settings->pszHighlightWords);
	mir_free(g_Settings->pszLogDir);
	if (g_Settings->MessageBoxFont)
		DeleteObject(g_Settings->MessageBoxFont);
	if (g_Settings->UserListFont)
		DeleteObject(g_Settings->UserListFont);
	if (g_Settings->UserListHeadingsFont)
		DeleteObject(g_Settings->UserListHeadingsFont);
}

void SetIndentSize()
{
	if (g_Settings->bShowTime) {
		LOGFONT lf;
		LoadMsgDlgFont(0, &lf, NULL);
		HFONT hFont = CreateFontIndirect(&lf);
		int iText = GetTextPixelSize(MakeTimeStamp(g_Settings->pszTimeStamp, time(NULL)), hFont, TRUE);
		DeleteObject(hFont);
		g_Settings->LogTextIndent = iText * 12 / 10;
	}
	else g_Settings->LogTextIndent = 0;
}

int GetTextPixelSize(TCHAR* pszText, HFONT hFont, BOOL bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	int i = DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

int OptionsInit(void)
{
	LoadLogFonts();

	LOGFONT lf;
	LoadMsgDlgFont(18, &lf, NULL);
	lstrcpy(lf.lfFaceName, _T("MS Shell Dlg"));
	lf.lfUnderline = lf.lfItalic = lf.lfStrikeOut = 0;
	lf.lfHeight = -17;
	lf.lfWeight = FW_BOLD;
	g_Settings->NameFont = CreateFontIndirect(&lf);
	g_Settings->UserListFont = NULL;
	g_Settings->UserListHeadingsFont = NULL;
	g_Settings->MessageBoxFont = NULL;
	g_Settings->iWidth = db_get_dw(NULL, "Chat", "roomwidth", -1);
	g_Settings->iHeight = db_get_dw(NULL, "Chat", "roomheight", -1);

	g_Settings->iSplitterX = db_get_w(NULL, "Chat", "SplitterX", 105);
	if (g_Settings->iSplitterX <= 50)
		g_Settings->iSplitterX = 105;
	g_Settings->iSplitterY = db_get_w(NULL, "Chat", "SplitterY", 90);
	if (g_Settings->iSplitterY <= 65)
		g_Settings->iSplitterY = 90;

	SkinAddNewSoundEx("ChatMessage", LPGEN("Group chats"), LPGEN("Incoming message"));
	SkinAddNewSoundEx("ChatHighlight", LPGEN("Group chats"), LPGEN("Message is highlighted"));
	SkinAddNewSoundEx("ChatAction", LPGEN("Group chats"), LPGEN("User has performed an action"));
	SkinAddNewSoundEx("ChatJoin", LPGEN("Group chats"), LPGEN("User has joined"));
	SkinAddNewSoundEx("ChatPart", LPGEN("Group chats"), LPGEN("User has left"));
	SkinAddNewSoundEx("ChatKick", LPGEN("Group chats"), LPGEN("User has kicked some other user"));
	SkinAddNewSoundEx("ChatMode", LPGEN("Group chats"), LPGEN("User's status was changed"));
	SkinAddNewSoundEx("ChatNick", LPGEN("Group chats"), LPGEN("User has changed name"));
	SkinAddNewSoundEx("ChatNotice", LPGEN("Group chats"), LPGEN("User has sent a notice"));
	SkinAddNewSoundEx("ChatQuit", LPGEN("Group chats"), LPGEN("User has disconnected"));
	SkinAddNewSoundEx("ChatTopic", LPGEN("Group chats"), LPGEN("The topic has been changed"));
	return 0;
}

int OptionsUnInit(void)
{
	FreeGlobalSettings();
	DeleteObject(g_Settings->NameFont);
	return 0;
}
