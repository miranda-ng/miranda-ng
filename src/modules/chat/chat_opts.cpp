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

HANDLE g_hOptions = NULL;

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
struct FontOptionsList
{
	const TCHAR*  szDescr;
	COLORREF      defColour;
	const TCHAR*  szDefFace;
	BYTE          defCharset, defStyle; 
	char          defSize;
	COLORREF      colour;
	const TCHAR*  szFace;
	BYTE          charset, style;
	char          size;
}

//remeber to put these in the Translate( ) template file too
static const fontOptionsList[] = {
	{ LPGENT("Timestamp"),                    RGB(50, 50, 240),   _T("Terminal"), DEFAULT_CHARSET, 0, -8},
	{ LPGENT("Others nicknames"),             RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12},
	{ LPGENT("Your nickname"),                RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12},
	{ LPGENT("User has joined"),              RGB(90, 160, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User has left"),                RGB(160, 160, 90),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User has disconnected"),        RGB(160, 90, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User kicked ..."),              RGB(100, 100, 100), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User is now known as ..."),     RGB(90, 90, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Notice from user"),             RGB(160, 130, 60),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Incoming message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Outgoing message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("The topic is ..."),             RGB(70, 70, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Information messages"),         RGB(130, 130, 195), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User enables status for ..."),  RGB(70, 150, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User disables status for ..."), RGB(150, 70, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Action message"),               RGB(160, 90, 160),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Highlighted message"),          RGB(180, 150, 80),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Message typing area"),          RGB(0, 0, 40),      _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User list members (online)"),   RGB(0,0, 0),        _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User list members (away)"),     RGB(170, 170, 170), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
};

const int msgDlgFontCount = SIZEOF(fontOptionsList);

void LoadLogFonts(void)
{
	for (int i = 0; i < OPTIONS_FONTCOUNT; i++)
		LoadMsgDlgFont(i, &ci.aFonts[i].lf, &ci.aFonts[i].color);
}

void LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF* colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;

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
		if (db_get_ts(NULL, "ChatFonts", str, &dbv))
			lstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
		else {
			lstrcpyn(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
			db_free(&dbv);
		}
	}
}

void RegisterFonts(void)
{
	FontIDT fontid = { 0 };
	ColourIDT colourid;
	char idstr[10];
	int index = 0, i;

	fontid.cbSize = sizeof(FontIDT);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_NEEDRESTART;
	for (i = 0; i < msgDlgFontCount; i++, index++) {
		strncpy(fontid.dbSettingsGroup, "ChatFonts", sizeof(fontid.dbSettingsGroup));
		_tcsncpy(fontid.group, _T("Chat module"), SIZEOF(fontid.group));
		_tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", index);
		strncpy(fontid.prefix, idstr, sizeof(fontid.prefix));
		fontid.order = index;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));
		_tcsncpy(fontid.backgroundGroup, _T("Chat module"), SIZEOF(fontid.backgroundGroup));
		switch (i) {
		case 17:
			_tcsncpy(fontid.backgroundName, _T("Message background"), SIZEOF(fontid.backgroundName));
			break;
		case 18:
		case 19:
			_tcsncpy(fontid.backgroundName, _T("Userlist background"), SIZEOF(fontid.backgroundName));
			break;
		default:
			_tcsncpy(fontid.backgroundName, _T("Background"), SIZEOF(fontid.backgroundName));
			break;
		}
		FontRegisterT(&fontid);
	}

	colourid.cbSize = sizeof(ColourIDT);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "Chat", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "ColorLogBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Background"), SIZEOF(colourid.name));
	_tcsncpy(colourid.group, LPGENT("Chat module"), SIZEOF(colourid.group));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorMessageBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Message background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("User list background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistLines", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("User list lines"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistSelectedBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("User list background (selected)"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	ColourRegisterT(&colourid);
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
	ci.pSettings->LogLimitNames = db_get_b(NULL, "Chat", "LogLimitNames", 1);
	ci.pSettings->ShowTime = db_get_b(NULL, "Chat", "ShowTimeStamp", 1);
	ci.pSettings->SoundsFocus = db_get_b(NULL, "Chat", "SoundsFocus", 0);
	ci.pSettings->ShowTimeIfChanged = (BOOL)db_get_b(NULL, "Chat", "ShowTimeStampIfChanged", 0);
	ci.pSettings->TimeStampEventColour = (BOOL)db_get_b(NULL, "Chat", "TimeStampEventColour", 0);
	ci.pSettings->iEventLimit = db_get_w(NULL, "Chat", "LogLimit", 100);
	ci.pSettings->dwIconFlags = db_get_dw(NULL, "Chat", "IconFlags", 0x0000);
	ci.pSettings->dwTrayIconFlags = db_get_dw(NULL, "Chat", "TrayIconFlags", 0x1000);
	ci.pSettings->dwPopupFlags = db_get_dw(NULL, "Chat", "PopupFlags", 0x0000);
	ci.pSettings->LoggingLimit = db_get_w(NULL, "Chat", "LoggingLimit", 100);
	ci.pSettings->LoggingEnabled = (BOOL)db_get_b(NULL, "Chat", "LoggingEnabled", 0);
	ci.pSettings->FlashWindow = (BOOL)db_get_b(NULL, "Chat", "FlashWindow", 0);
	ci.pSettings->HighlightEnabled = (BOOL)db_get_b(NULL, "Chat", "HighlightEnabled", 1);
	ci.pSettings->crUserListColor = db_get_dw(NULL, "ChatFonts", "Font18Col", RGB(0, 0, 0));
	ci.pSettings->crUserListBGColor = db_get_dw(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW));
	ci.pSettings->crUserListSelectedBGColor = db_get_dw(NULL, "Chat", "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT));
	ci.pSettings->crUserListHeadingsColor = db_get_dw(NULL, "ChatFonts", "Font19Col", RGB(170, 170, 170));
	ci.pSettings->crLogBackground = db_get_dw(NULL, "Chat", "ColorLogBG", GetSysColor(COLOR_WINDOW));
	ci.pSettings->StripFormat = (BOOL)db_get_b(NULL, "Chat", "StripFormatting", 0);
	ci.pSettings->TrayIconInactiveOnly = (BOOL)db_get_b(NULL, "Chat", "TrayIconInactiveOnly", 1);
	ci.pSettings->PopupInactiveOnly = (BOOL)db_get_b(NULL, "Chat", "PopupInactiveOnly", 1);
	ci.pSettings->AddColonToAutoComplete = (BOOL)db_get_b(NULL, "Chat", "AddColonToAutoComplete", 1);
	ci.pSettings->iPopupStyle = db_get_b(NULL, "Chat", "PopupStyle", 1);
	ci.pSettings->iPopupTimeout = db_get_w(NULL, "Chat", "PopupTimeout", 3);
	ci.pSettings->crPUBkgColour = db_get_dw(NULL, "Chat", "PopupColorBG", GetSysColor(COLOR_WINDOW));
	ci.pSettings->crPUTextColour = db_get_dw(NULL, "Chat", "PopupColorText", 0);
	ci.pSettings->ShowContactStatus = db_get_b(NULL, "Chat", "ShowContactStatus", 0);
	ci.pSettings->ContactStatusFirst = db_get_b(NULL, "Chat", "ContactStatusFirst", 0);

	if (ci.OnLoadSettings)
		ci.OnLoadSettings();

	InitSetting(&ci.pSettings->pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	InitSetting(&ci.pSettings->pszTimeStampLog, "LogTimestamp", _T("[%d %b %y %H:%M]"));
	InitSetting(&ci.pSettings->pszIncomingNick, "HeaderIncoming", _T("%n:"));
	InitSetting(&ci.pSettings->pszOutgoingNick, "HeaderOutgoing", _T("%n:"));
	InitSetting(&ci.pSettings->pszHighlightWords, "HighlightWords", _T("%m"));

	TCHAR pszTemp[MAX_PATH];
	DBVARIANT dbv;
	ci.pSettings->pszLogDir = (TCHAR *)mir_realloc(ci.pSettings->pszLogDir, MAX_PATH*sizeof(TCHAR));
	if (!db_get_ts(NULL, "Chat", "LogDirectory", &dbv)) {
		lstrcpyn(pszTemp, dbv.ptszVal, MAX_PATH);
		db_free(&dbv);
	}
	else {
		TCHAR *tmpPath = Utils_ReplaceVarsT(_T("%miranda_logpath%\\Chat"));
		lstrcpyn(pszTemp, tmpPath, SIZEOF(pszTemp) - 1);
		mir_free(tmpPath);
	}

	PathToAbsoluteT(pszTemp, ci.pSettings->pszLogDir);

	ci.pSettings->LogIndentEnabled = (db_get_b(NULL, "Chat", "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

	LOGFONT lf;
	if (ci.pSettings->MessageBoxFont)
		DeleteObject(ci.pSettings->MessageBoxFont);
	LoadMsgDlgFont(17, &lf, NULL);
	ci.pSettings->MessageBoxFont = CreateFontIndirect(&lf);

	if (ci.pSettings->UserListFont)
		DeleteObject(ci.pSettings->UserListFont);
	LoadMsgDlgFont(18, &lf, NULL);
	ci.pSettings->UserListFont = CreateFontIndirect(&lf);

	if (ci.pSettings->UserListHeadingsFont)
		DeleteObject(ci.pSettings->UserListHeadingsFont);
	LoadMsgDlgFont(19, &lf, NULL);
	ci.pSettings->UserListHeadingsFont = CreateFontIndirect(&lf);
}

static void FreeGlobalSettings(void)
{
	mir_free(ci.pSettings->pszTimeStamp);
	mir_free(ci.pSettings->pszTimeStampLog);
	mir_free(ci.pSettings->pszIncomingNick);
	mir_free(ci.pSettings->pszOutgoingNick);
	mir_free(ci.pSettings->pszHighlightWords);
	mir_free(ci.pSettings->pszLogDir);
	if (ci.pSettings->MessageBoxFont)
		DeleteObject(ci.pSettings->MessageBoxFont);
	if (ci.pSettings->UserListFont)
		DeleteObject(ci.pSettings->UserListFont);
	if (ci.pSettings->UserListHeadingsFont)
		DeleteObject(ci.pSettings->UserListHeadingsFont);
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
	ci.pSettings->NameFont = CreateFontIndirect(&lf);
	ci.pSettings->UserListFont = NULL;
	ci.pSettings->UserListHeadingsFont = NULL;
	ci.pSettings->MessageBoxFont = NULL;
	ci.pSettings->iSplitterX = db_get_w(NULL, "Chat", "SplitterX", 105);
	ci.pSettings->iSplitterY = db_get_w(NULL, "Chat", "SplitterY", 90);
	ci.pSettings->iX = db_get_dw(NULL, "Chat", "roomx", -1);
	ci.pSettings->iY = db_get_dw(NULL, "Chat", "roomy", -1);
	ci.pSettings->iWidth = db_get_dw(NULL, "Chat", "roomwidth", -1);
	ci.pSettings->iHeight = db_get_dw(NULL, "Chat", "roomheight", -1);
	LoadGlobalSettings();

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

	if (ci.pSettings->LoggingEnabled)
		CreateDirectoryTreeT(ci.pSettings->pszLogDir);

	LOGFONT lf2;
	LoadMsgDlgFont(0, &lf2, NULL);
	HFONT hFont = CreateFontIndirect(&lf2);
	int iText = GetTextPixelSize(MakeTimeStamp(ci.pSettings->pszTimeStamp, time(NULL)), hFont, TRUE);
	DeleteObject(hFont);
	ci.pSettings->LogTextIndent = iText;
	ci.pSettings->LogTextIndent = ci.pSettings->LogTextIndent * 12 / 10;
	return 0;
}

int OptionsUnInit(void)
{
	FreeGlobalSettings();
	UnhookEvent(g_hOptions);
	DeleteObject(ci.pSettings->NameFont);
	return 0;
}
