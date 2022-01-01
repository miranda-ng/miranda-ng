/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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
// Import and export theme settings between files and the database

#include "stdafx.h"

#define CURRENT_THEME_VERSION 5
#define THEME_COOKIE 25099837

struct {
	char*	szIniSection;
	char*	szIniName;
	char*	szDbModule;
	char*	szDbSetting;
	uint32_t	dwDef;
}
static _extSettings[] =
{
	{ "Message Log", "BackgroundColor", FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR },
	{ "Message Log", "IncomingBG", FONTMODULE, "inbg", SRMSGDEFSET_BKGINCOLOUR },
	{ "Message Log", "OutgoingBG", FONTMODULE, "outbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ "Message Log", "OldIncomingBG", FONTMODULE, "oldinbg", SRMSGDEFSET_BKGINCOLOUR },
	{ "Message Log", "OldOutgoingBG", FONTMODULE, "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ "Message Log", "StatusBG", FONTMODULE, "statbg", SRMSGDEFSET_BKGCOLOUR },
	{ "Message Log", "InputBG", FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR },
	{ "Message Log", "HgridColor", FONTMODULE, "hgrid", SRMSGDEFSET_BKGCOLOUR },
	{ "Message Log", "DWFlags", SRMSGMOD_T, "mwflags", MWF_LOG_DEFAULT },
	{ CHAT_MODULE, "UserListBG", CHAT_MODULE, "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR },
	{ "Message Log", "LeftIndent", SRMSGMOD_T, "IndentAmount", 20 },
	{ "Message Log", "RightIndent", SRMSGMOD_T, "RightIndent", 20 }
};

/**
 * new in TabSRMM3 / theme version 5
 * don't read these values from themes with version < 5
 */
struct
{
	char*	szIniSection;
	char*	szIniName;
	char*	szDbModule;
	char*	szDbSetting;
	uint32_t	dwDef;
}
static _extSettings_v5[] =
{
	{ "CommonClrs", "IP_High", FONTMODULE, "ipfieldsbgHigh", 0xf0f0f0 },
	{ "CommonClrs", "IP_Low", FONTMODULE, "ipfieldsbg", 0x62caff },
	{ "CommonClrs", "TB_High", FONTMODULE, "tbBgHigh", 0 },
	{ "CommonClrs", "TB_Low", FONTMODULE, "tbBgLow", 0 },
	{ "CommonClrs", "FillColor", FONTMODULE, "fillColor", 0 },
	{ "CommonClrs", "RichBorders", FONTMODULE, "cRichBorders", 0 },
	{ "CommonClrs", "GenericTxt", FONTMODULE, "genericTxtClr", RGB(20, 20, 20) },
	{ "AeroMode", "Style", SRMSGMOD_T, "aerostyle", CSkin::AERO_EFFECT_MILK },
	{ "AeroMode", "AeroGlowColor", FONTMODULE, "aeroGlow", RGB(40, 40, 255) },

	{ "Colored Tabs", "NormalText", SRMSGMOD_T, "tab_txt_normal", RGB(1, 1, 1) },
	{ "Colored Tabs", "ActiveText", SRMSGMOD_T, "tab_txt_active", RGB(1, 1, 1) },
	{ "Colored Tabs", "HottrackText", SRMSGMOD_T, "tab_txt_hottrack", RGB(1, 1, 1) },
	{ "Colored Tabs", "UnreadText", SRMSGMOD_T, "tab_txt_unread", RGB(1, 1, 1) },

	{ "Colored Tabs", "NormalBG", SRMSGMOD_T, "tab_bg_normal", 0xf0f0f0 },
	{ "Colored Tabs", "ActiveBG", SRMSGMOD_T, "tab_bg_active", 0xf0f0f0 },
	{ "Colored Tabs", "HottrackBG", SRMSGMOD_T, "tab_bg_hottrack", 0xf0f0f0 },
	{ "Colored Tabs", "UnreadBG", SRMSGMOD_T, "tab_bg_unread", 0xf0f0f0 },
	{ CHAT_MODULE, "Background", CHAT_MODULE, "ColorLogBG", SRMSGDEFSET_BKGCOLOUR }
};

/*
 * this loads a font definition from an INI file.
 * i = font number
 * szKey = ini section (e.g. [Font10])
 * *lf = pointer to a LOGFONT structure which will receive the font definition
 * *colour = pointer to a COLORREF which will receive the color of the font definition
 */
static void TSAPI LoadLogfontFromINI(int i, char *szKey, LOGFONTW *lf, COLORREF *colour, const char *szIniFilename)
{
	int style;
	char bSize;

	if (colour)
		*colour = GetPrivateProfileIntA(szKey, "Color", 0, szIniFilename);

	if (lf) {
		HDC hdc = GetDC(nullptr);
		if (i == H_MSGFONTID_DIVIDERS)
			lf->lfHeight = 5;
		else {
			bSize = (char)GetPrivateProfileIntA(szKey, "Size", -12, szIniFilename);
			if (bSize > 0)
				lf->lfHeight = -MulDiv(bSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			else
				lf->lfHeight = bSize;
		}

		ReleaseDC(nullptr, hdc);

		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		style = (int)GetPrivateProfileIntA(szKey, "Style", 0, szIniFilename);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		if (i == MSGFONTID_SYMBOLS_IN || i == MSGFONTID_SYMBOLS_OUT)
			lf->lfCharSet = SYMBOL_CHARSET;
		else
			lf->lfCharSet = (uint8_t)GetPrivateProfileIntA(szKey, "Set", DEFAULT_CHARSET, szIniFilename);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		if (i == MSGFONTID_SYMBOLS_IN || i == MSGFONTID_SYMBOLS_OUT) {
			wcsncpy_s(lf->lfFaceName, L"Webdings", _TRUNCATE);
			lf->lfCharSet = SYMBOL_CHARSET;
		}
		else GetPrivateProfileStringW(_A2T(szKey), L"Face", L"Tahoma", lf->lfFaceName, _countof(lf->lfFaceName), _A2T(szIniFilename));

		/*
		 * filter out font attributes from the message input area font
		 * (can be disabled by db tweak)
		 */
		if (!mir_strcmp(szKey, "Font16") && M.GetByte("inputFontFix", 1) == 1) {
			lf->lfWeight = FW_NORMAL;
			lf->lfItalic = 0;
			lf->lfUnderline = 0;
			lf->lfStrikeOut = 0;
		}
	}
}

static struct _tagFontBlocks {
	char *szModule;
	int iFirst;
	int iCount;
	char *szIniTemp;
	char *szBLockname;
} fontBlocks[] = {
	FONTMODULE, 0, MSGDLGFONTCOUNT, "Font%d", "StdFonts",
	FONTMODULE, 100, IPFONTCOUNT, "IPFont%d", "MiscFonts",
	CHATFONT_MODULE, 0, CHATFONTCOUNT, "ChatFont%d", CHATFONT_MODULE,
	nullptr, 0, 0, nullptr
};

int TSAPI CheckThemeVersion(const wchar_t *szIniFilename)
{
	int cookie = GetPrivateProfileInt(L"TabSRMM Theme", L"Cookie", 0, szIniFilename);
	int version = GetPrivateProfileInt(L"TabSRMM Theme", L"Version", 0, szIniFilename);

	if (version >= CURRENT_THEME_VERSION && cookie == THEME_COOKIE)
		return 1;
	return 0;
}

void TSAPI WriteThemeToINI(const wchar_t *szIniFilenameT, CMsgDialog *dat)
{
	int i, n = 0;
	DBVARIANT dbv;
	char szBuf[100], szTemp[100], szAppname[100];
	COLORREF def;
	char *szIniFilename = mir_u2a(szIniFilenameT);

	WritePrivateProfileStringA("TabSRMM Theme", "Version", _itoa(CURRENT_THEME_VERSION, szBuf, 10), szIniFilename);
	WritePrivateProfileStringA("TabSRMM Theme", "Cookie", _itoa(THEME_COOKIE, szBuf, 10), szIniFilename);

	while (fontBlocks[n].szModule) {
		int firstIndex = fontBlocks[n].iFirst;
		char *szModule = fontBlocks[n].szModule;
		WritePrivateProfileStringA(fontBlocks[n].szBLockname, "Valid", "1", szIniFilename);
		for (i = 0; i < fontBlocks[n].iCount; i++) {
			mir_snprintf(szTemp, "Font%d", firstIndex + i);
			mir_snprintf(szAppname, fontBlocks[n].szIniTemp, firstIndex + i);
			if (!db_get_s(0, szModule, szTemp, &dbv)) {
				WritePrivateProfileStringA(szAppname, "Face", dbv.pszVal, szIniFilename);
				db_free(&dbv);
			}
			mir_snprintf(szTemp, "Font%dCol", firstIndex + i);
			WritePrivateProfileStringA(szAppname, "Color", _itoa(db_get_dw(0, szModule, szTemp, 0), szBuf, 10), szIniFilename);
			mir_snprintf(szTemp, "Font%dSty", firstIndex + i);
			WritePrivateProfileStringA(szAppname, "Style", _itoa(db_get_b(0, szModule, szTemp, 0), szBuf, 10), szIniFilename);
			mir_snprintf(szTemp, "Font%dSize", firstIndex + i);
			WritePrivateProfileStringA(szAppname, "Size", _itoa(db_get_b(0, szModule, szTemp, 0), szBuf, 10), szIniFilename);
			mir_snprintf(szTemp, "Font%dSet", firstIndex + i);
			WritePrivateProfileStringA(szAppname, "Set", _itoa(db_get_b(0, szModule, szTemp, 0), szBuf, 10), szIniFilename);
		}
		n++;
	}
	def = SRMSGDEFSET_BKGCOLOUR;

	for (auto &it : _extSettings)
		WritePrivateProfileStringA(it.szIniSection, it.szIniName, _itoa(db_get_dw(0, it.szDbModule, it.szDbSetting, it.dwDef), szBuf, 10), szIniFilename);

	for (auto &it : _extSettings_v5)
		WritePrivateProfileStringA(it.szIniSection, it.szIniName, _itoa(db_get_dw(0, it.szDbModule, it.szDbSetting, it.dwDef), szBuf, 10), szIniFilename);

	WritePrivateProfileStringA("Message Log", "VGrid", _itoa(M.GetByte("wantvgrid", 0), szBuf, 10), szIniFilename);
	WritePrivateProfileStringA("Message Log", "ExtraMicroLF", _itoa(M.GetByte("extramicrolf", 0), szBuf, 10), szIniFilename);

	for (i = 0; i < TMPL_MAX; i++) {
		T2Utf szLTR((dat == nullptr) ? LTR_Active.szTemplates[i] : dat->m_pContainer->m_ltr_templates->szTemplates[i]);
		WritePrivateProfileStringA("Templates", TemplateNames[i], szLTR, szIniFilename);

		T2Utf szRTL((dat == nullptr) ? RTL_Active.szTemplates[i] : dat->m_pContainer->m_rtl_templates->szTemplates[i]);
		WritePrivateProfileStringA("RTLTemplates", TemplateNames[i], szRTL, szIniFilename);
	}
	for (i = 0; i < CUSTOM_COLORS; i++) {
		mir_snprintf(szTemp, "cc%d", i + 1);
		if (dat == nullptr)
			WritePrivateProfileStringA("Custom Colors", szTemp, _itoa(M.GetDword(szTemp, 0), szBuf, 10), szIniFilename);
		else
			WritePrivateProfileStringA("Custom Colors", szTemp, _itoa(dat->m_pContainer->m_theme.custom_colors[i], szBuf, 10), szIniFilename);
	}
	for (i = 0; i <= 7; i++)
		WritePrivateProfileStringA("Nick Colors", _itoa(i, szBuf, 10), _itoa(g_Settings.nickColors[i], szTemp, 10), szIniFilename);

	mir_free(szIniFilename);
}

void TSAPI ReadThemeFromINI(const wchar_t *szIniFilenameT, TContainerData *dat, int noAdvanced, uint32_t dwFlags)
{
	char szBuf[512], szTemp[100], szAppname[100];
	int i, n = 0;
	int version;
	COLORREF def;
	char *szIniFilename = mir_u2a(szIniFilenameT);
	char szTemplateBuffer[TEMPLATE_LENGTH * 3 + 2];
	char bSize = 0;
	int charset;

	if ((version = GetPrivateProfileIntA("TabSRMM Theme", "Version", 0, szIniFilename)) == 0)        // no version number.. assume 1
		version = 1;

	HDC hdc = GetDC(nullptr);
	if (dat == nullptr) {
		while (fontBlocks[n].szModule && (dwFlags & THEME_READ_FONTS)) {
			char *szModule = fontBlocks[n].szModule;
			int firstIndex = fontBlocks[n].iFirst;

			if (n != 1 && !(dwFlags & THEME_READ_FONTS)) {
				n++;
				continue;
			}
			if (GetPrivateProfileIntA(fontBlocks[n].szBLockname, "Valid", 0, szIniFilename) == 0 && n != 0) {
				n++;
				continue;
			}
			for (i = 0; i < fontBlocks[n].iCount; i++) {
				mir_snprintf(szTemp, "Font%d", firstIndex + i);
				mir_snprintf(szAppname, fontBlocks[n].szIniTemp, firstIndex + i);
				if (GetPrivateProfileStringA(szAppname, "Face", "Verdana", szBuf, sizeof(szBuf), szIniFilename) != 0) {
					if (i == MSGFONTID_SYMBOLS_IN || i == MSGFONTID_SYMBOLS_OUT)
						strncpy_s(szBuf, "Arial", _TRUNCATE);
					db_set_s(0, szModule, szTemp, szBuf);
				}

				mir_snprintf(szTemp, "Font%dCol", firstIndex + i);
				db_set_dw(0, szModule, szTemp, GetPrivateProfileIntA(szAppname, "Color", GetSysColor(COLOR_WINDOWTEXT), szIniFilename));

				mir_snprintf(szTemp, "Font%dSty", firstIndex + i);
				db_set_b(0, szModule, szTemp, (uint8_t)(GetPrivateProfileIntA(szAppname, "Style", 0, szIniFilename)));

				mir_snprintf(szTemp, "Font%dSize", firstIndex + i);
				bSize = (char)GetPrivateProfileIntA(szAppname, "Size", -10, szIniFilename);
				if (bSize > 0)
					bSize = -MulDiv(bSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				db_set_b(0, szModule, szTemp, bSize);

				mir_snprintf(szTemp, "Font%dSet", firstIndex + i);
				charset = GetPrivateProfileIntA(szAppname, "Set", 0, szIniFilename);
				if (i == MSGFONTID_SYMBOLS_IN || i == MSGFONTID_SYMBOLS_OUT)
					charset = 0;
				db_set_b(0, szModule, szTemp, (uint8_t)charset);
			}
			n++;
		}
		def = SRMSGDEFSET_BKGCOLOUR;
		ReleaseDC(nullptr, hdc);

		if (dwFlags & THEME_READ_FONTS) {
			COLORREF defclr;

			for (auto &it : _extSettings)
				db_set_dw(0, it.szDbModule, it.szDbSetting, GetPrivateProfileIntA(it.szIniSection, it.szIniName, it.dwDef, szIniFilename));

			if (version >= 5)
				for (auto &it : _extSettings_v5)
					db_set_dw(0, it.szDbModule, it.szDbSetting, GetPrivateProfileIntA(it.szIniSection, it.szIniName, it.dwDef, szIniFilename));

			db_set_b(0, SRMSGMOD_T, "wantvgrid", (uint8_t)(GetPrivateProfileIntA("Message Log", "VGrid", 0, szIniFilename)));
			db_set_b(0, SRMSGMOD_T, "extramicrolf", (uint8_t)(GetPrivateProfileIntA("Message Log", "ExtraMicroLF", 0, szIniFilename)));

			for (i = 0; i < CUSTOM_COLORS; i++) {
				mir_snprintf(szTemp, "cc%d", i + 1);
				db_set_dw(0, SRMSGMOD_T, szTemp, GetPrivateProfileIntA("Custom Colors", szTemp, RGB(224, 224, 224), szIniFilename));
			}
			for (i = 0; i <= 7; i++) {
				if (i == 5)
					defclr = GetSysColor(COLOR_HIGHLIGHT);
				else if (i == 6)
					defclr = GetSysColor(COLOR_HIGHLIGHTTEXT);
				else
					defclr = g_Settings.UserListColors[CHAT_STATUS_NORMAL];
				g_Settings.nickColors[i] = GetPrivateProfileIntA("Nick Colors", _itoa(i, szTemp, 10), defclr, szIniFilename);
				mir_snprintf(szTemp, "NickColor%d", i);
				db_set_dw(0, CHAT_MODULE, szTemp, g_Settings.nickColors[i]);
			}
		}
	}
	else {
		int SY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(nullptr, hdc);
		if (!noAdvanced) {
			for (i = 0; i < MSGDLGFONTCOUNT; i++) {
				mir_snprintf(szTemp, "Font%d", i);
				LoadLogfontFromINI(i, szTemp, &dat->m_theme.logFonts[i], &dat->m_theme.fontColors[i], szIniFilename);
				wsprintfA(dat->m_theme.rtfFonts + (i * RTFCACHELINESIZE), "\\f%u\\cf%u\\b%d\\i%d\\ul%d\\fs%u", i, i, dat->m_theme.logFonts[i].lfWeight >= FW_BOLD ? 1 : 0, dat->m_theme.logFonts[i].lfItalic, dat->m_theme.logFonts[i].lfUnderline, 2 * abs(dat->m_theme.logFonts[i].lfHeight) * 74 / SY); //!!!!!!!!
			}
			wsprintfA(dat->m_theme.rtfFonts + (MSGDLGFONTCOUNT * RTFCACHELINESIZE), "\\f%u\\cf%u\\b%d\\i%d\\ul%d\\fs%u", MSGDLGFONTCOUNT, MSGDLGFONTCOUNT, 0, 0, 0, 0); //!!!!!!!!!
		}
		dat->m_theme.bg = GetPrivateProfileIntA("Message Log", "BackgroundColor", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.inbg = GetPrivateProfileIntA("Message Log", "IncomingBG", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.outbg = GetPrivateProfileIntA("Message Log", "OutgoingBG", RGB(224, 224, 224), szIniFilename);

		dat->m_theme.oldinbg = GetPrivateProfileIntA("Message Log", "OldIncomingBG", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.oldoutbg = GetPrivateProfileIntA("Message Log", "OldOutgoingBG", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.statbg = GetPrivateProfileIntA("Message Log", "StatusBG", RGB(224, 224, 224), szIniFilename);

		dat->m_theme.inputbg = GetPrivateProfileIntA("Message Log", "InputBG", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.hgrid = GetPrivateProfileIntA("Message Log", "HgridColor", RGB(224, 224, 224), szIniFilename);
		dat->m_theme.dwFlags = GetPrivateProfileIntA("Message Log", "DWFlags", MWF_LOG_DEFAULT, szIniFilename);
		db_set_b(0, SRMSGMOD_T, "wantvgrid", (uint8_t)(GetPrivateProfileIntA("Message Log", "VGrid", 0, szIniFilename)));
		db_set_b(0, SRMSGMOD_T, "extramicrolf", (uint8_t)(GetPrivateProfileIntA("Message Log", "ExtraMicroLF", 0, szIniFilename)));

		dat->m_theme.left_indent = GetPrivateProfileIntA("Message Log", "LeftIndent", 0, szIniFilename);
		dat->m_theme.right_indent = GetPrivateProfileIntA("Message Log", "RightIndent", 0, szIniFilename);

		for (i = 0; i < CUSTOM_COLORS; i++) {
			mir_snprintf(szTemp, "cc%d", i + 1);
			dat->m_theme.custom_colors[i] = GetPrivateProfileIntA("Custom Colors", szTemp, RGB(224, 224, 224), szIniFilename);
		}
	}

	if (version >= 3) {
		if (!noAdvanced && dwFlags & THEME_READ_TEMPLATES) {
			for (i = 0; i < TMPL_MAX; i++) {
				wchar_t *decoded = nullptr;

				GetPrivateProfileStringA("Templates", TemplateNames[i], "[undef]", szTemplateBuffer, TEMPLATE_LENGTH * 3, szIniFilename);

				if (mir_strcmp(szTemplateBuffer, "[undef]")) {
					if (dat == nullptr)
						db_set_utf(0, TEMPLATES_MODULE, TemplateNames[i], szTemplateBuffer);
					decoded = mir_utf8decodeW(szTemplateBuffer);
					if (dat == nullptr)
						wcsncpy_s(LTR_Active.szTemplates[i], decoded, _TRUNCATE);
					else
						wcsncpy_s(dat->m_ltr_templates->szTemplates[i], decoded, _TRUNCATE);
					mir_free(decoded);
				}

				GetPrivateProfileStringA("RTLTemplates", TemplateNames[i], "[undef]", szTemplateBuffer, TEMPLATE_LENGTH * 3, szIniFilename);

				if (mir_strcmp(szTemplateBuffer, "[undef]")) {
					if (dat == nullptr)
						db_set_utf(0, RTLTEMPLATES_MODULE, TemplateNames[i], szTemplateBuffer);
					decoded = mir_utf8decodeW(szTemplateBuffer);
					if (dat == nullptr)
						wcsncpy_s(RTL_Active.szTemplates[i], decoded, _TRUNCATE);
					else
						wcsncpy_s(dat->m_rtl_templates->szTemplates[i], decoded, _TRUNCATE);
					mir_free(decoded);
				}
			}
		}
	}

	mir_free(szIniFilename);
}

/*
 * iMode = 0 - GetOpenFilename, otherwise, GetSaveAs...
 */

const wchar_t* TSAPI GetThemeFileName(int iMode)
{
	static wchar_t szFilename[MAX_PATH];
	OPENFILENAME ofn = { 0 };
	wchar_t szInitialDir[MAX_PATH];

	wcsncpy_s(szInitialDir, M.getSkinPath(), _TRUNCATE);

	szFilename[0] = 0;

	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s%c*.tabsrmm%c%c", TranslateT("TabSRMM themes"), 0, 0, 0);
	ofn.lpstrFilter = filter;
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFilename;
	ofn.lpstrInitialDir = szInitialDir;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
	ofn.lpstrDefExt = L"tabsrmm";
	if (!iMode) {
		if (GetOpenFileName(&ofn))
			return szFilename;
		else
			return nullptr;
	}
	else {
		if (GetSaveFileName(&ofn))
			return szFilename;
		else
			return nullptr;
	}
}
