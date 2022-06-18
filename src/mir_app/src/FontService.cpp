/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "FontService.h"

int code_page = CP_ACP;
HANDLE hFontReloadEvent, hColourReloadEvent;

void notifyOptions();

/////////////////////////////////////////////////////////////////////////////////////////

void ConvertFontSettings(FontSettings *fs, FontSettingsW *fsw)
{
	fsw->colour = fs->colour;
	fsw->size = fs->size;
	fsw->style = fs->style;
	fsw->charset = fs->charset;

	MultiByteToWideChar(code_page, 0, fs->szFace, -1, fsw->szFace, LF_FACESIZE);
}

bool ConvertFontID(FontID *fid, FontIDW *fidw)
{
	memset(fidw, 0, sizeof(FontIDW));
	strncpy_s(fidw->dbSettingsGroup, fid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(fidw->setting, fid->setting, _TRUNCATE);
	fidw->flags = fid->flags;
	fidw->order = fid->order;
	ConvertFontSettings(&fid->deffontsettings, &fidw->deffontsettings);

	MultiByteToWideChar(code_page, 0, fid->group, -1, fidw->group, 64);
	MultiByteToWideChar(code_page, 0, fid->name, -1, fidw->name, 64);

	MultiByteToWideChar(code_page, 0, fid->backgroundGroup, -1, fidw->backgroundGroup, 64);
	MultiByteToWideChar(code_page, 0, fid->backgroundName, -1, fidw->backgroundName, 64);
	return true;
}

bool ConvertColourID(ColourID *cid, ColourIDW *cidw)
{
	strncpy_s(cidw->dbSettingsGroup, cid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(cidw->setting, cid->setting, _TRUNCATE);
	cidw->defcolour = cid->defcolour;
	cidw->order = cid->order;

	MultiByteToWideChar(code_page, 0, cid->group, -1, cidw->group, 64);
	MultiByteToWideChar(code_page, 0, cid->name, -1, cidw->name, 64);
	return true;
}

bool ConvertEffectID(EffectID *eid, EffectIDW *eidw)
{
	strncpy_s(eidw->dbSettingsGroup, eid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(eidw->setting, eid->setting, _TRUNCATE);
	eidw->defeffect.effectIndex = eid->defeffect.effectIndex;
	eidw->defeffect.baseColour = eid->defeffect.baseColour;
	eidw->defeffect.secondaryColour = eid->defeffect.secondaryColour;
	eidw->order = eid->order;

	MultiByteToWideChar(code_page, 0, eid->group, -1, eidw->group, 64);
	MultiByteToWideChar(code_page, 0, eid->name, -1, eidw->name, 64);
	return true;
}

void ConvertLOGFONT(LOGFONTW *lfw, LOGFONTA *lfa)
{
	lfa->lfHeight = lfw->lfHeight;
	lfa->lfWidth = lfw->lfWidth;
	lfa->lfEscapement = lfw->lfEscapement;
	lfa->lfOrientation = lfw->lfOrientation;
	lfa->lfWeight = lfw->lfWeight;
	lfa->lfItalic = lfw->lfItalic;
	lfa->lfUnderline = lfw->lfUnderline;
	lfa->lfStrikeOut = lfw->lfStrikeOut;
	lfa->lfCharSet = lfw->lfCharSet;
	lfa->lfOutPrecision = lfw->lfOutPrecision;
	lfa->lfClipPrecision = lfw->lfClipPrecision;
	lfa->lfQuality = lfw->lfQuality;
	lfa->lfPitchAndFamily = lfw->lfPitchAndFamily;

	WideCharToMultiByte(code_page, 0, lfw->lfFaceName, -1, lfa->lfFaceName, LF_FACESIZE, nullptr, nullptr);
}

static void GetDefaultFontSetting(LOGFONT *lf, COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	if (colour)
		*colour = GetSysColor(COLOR_WINDOWTEXT);
}

int GetFontSettingFromDB(char *settings_group, char *prefix, LOGFONT *lf, COLORREF *colour, uint32_t flags)
{
	GetDefaultFontSetting(lf, colour);

	char idstr[256];
	if (flags & FIDF_APPENDNAME)
		mir_snprintf(idstr, "%sName", prefix);
	else
		strncpy_s(idstr, prefix, _TRUNCATE);

	int retval = 0;
	ptrW tszGroup(db_get_wsa(0, settings_group, idstr));
	if (tszGroup != nullptr)
		wcsncpy_s(lf->lfFaceName, tszGroup, _TRUNCATE);
	else
		retval = 1;

	if (colour) {
		mir_snprintf(idstr, "%sCol", prefix);
		*colour = db_get_dw(0, settings_group, idstr, *colour);
	}

	mir_snprintf(idstr, "%sSize", prefix);
	lf->lfHeight = (char)db_get_b(0, settings_group, idstr, lf->lfHeight);

	mir_snprintf(idstr, "%sSty", prefix);
	uint8_t style = (uint8_t)db_get_b(0, settings_group, idstr,
		(lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf->lfItalic ? DBFONTF_ITALIC : 0) | (lf->lfUnderline ? DBFONTF_UNDERLINE : 0) | lf->lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (style & DBFONTF_STRIKEOUT) != 0;

	mir_snprintf(idstr, "%sSet", prefix);
	lf->lfCharSet = db_get_b(0, settings_group, idstr, lf->lfCharSet);

	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	if (lf->lfHeight > 0) {
		HDC hdc = GetDC(nullptr);
		if (flags & FIDF_SAVEPOINTSIZE)
			lf->lfHeight = -MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		else { // assume SAVEACTUALHEIGHT
			HFONT hFont = CreateFontIndirect(lf);
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);
			lf->lfHeight = -(lf->lfHeight - tm.tmInternalLeading);

			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
		}

		ReleaseDC(nullptr, hdc);
	}

	return retval;
}

int CreateFromFontSettings(FontSettingsW *fs, LOGFONT *lf)
{
	GetDefaultFontSetting(lf, nullptr);

	wcsncpy_s(lf->lfFaceName, fs->szFace, _TRUNCATE);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = fs->style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (fs->style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (fs->style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (fs->style & DBFONTF_STRIKEOUT) != 0;
	lf->lfCharSet = fs->charset;
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	lf->lfHeight = fs->size;
	return 0;
}

void UpdateFontSettings(FontIDW *font_id, FontSettingsW *fontsettings)
{
	LOGFONT lf;
	COLORREF colour;
	if (GetFontSettingFromDB(font_id->dbSettingsGroup, font_id->setting, &lf, &colour, font_id->flags) && (font_id->flags & FIDF_DEFAULTVALID)) {
		CreateFromFontSettings(&font_id->deffontsettings, &lf);
		colour = font_id->deffontsettings.colour;
	}

	fontsettings->style =
		(lf.lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	fontsettings->size = (char)lf.lfHeight;
	fontsettings->charset = lf.lfCharSet;
	fontsettings->colour = colour;
	wcsncpy_s(fontsettings->szFace, lf.lfFaceName, _TRUNCATE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterFont service

static int sttRegisterFontWorker(FontIDW *font_id, HPLUGIN pPlugin)
{
	for (auto &F : font_id_list)
		if (!mir_wstrcmp(F->group, font_id->group) && !mir_wstrcmp(F->name, font_id->name) && !(F->flags & FIDF_ALLOWREREGISTER))
			return 1;

	char idstr[256];
	mir_snprintf(idstr, "%sFlags", font_id->setting);
	db_set_dw(0, font_id->dbSettingsGroup, idstr, font_id->flags);

	FontInternal* newItem = new FontInternal;
	memset(newItem, 0, sizeof(FontInternal));
	memcpy(newItem, font_id, sizeof(FontIDW));
	newItem->pPlugin = pPlugin;

	if (!mir_wstrcmp(newItem->deffontsettings.szFace, L"MS Shell Dlg")) {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
		mir_wstrncpy(newItem->deffontsettings.szFace, lf.lfFaceName, _countof(newItem->deffontsettings.szFace));
		if (!newItem->deffontsettings.size)
			newItem->deffontsettings.size = lf.lfHeight;
	}

	UpdateFontSettings(font_id, &newItem->value);
	font_id_list.insert(newItem);

	notifyOptions();
	return 0;
}

MIR_APP_DLL(int) Font_RegisterW(FontIDW *pFont, HPLUGIN pPlugin)
{
	return sttRegisterFontWorker(pFont, pPlugin);
}

MIR_APP_DLL(int) Font_Register(FontID *pFont, HPLUGIN pPlugin)
{
	FontIDW temp;
	if (!ConvertFontID(pFont, &temp)) return -1;
	return sttRegisterFontWorker(&temp, pPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetFont service

static COLORREF sttGetFontWorker(const wchar_t *wszGroup, const wchar_t *wszName, LOGFONTW *lf)
{
	COLORREF colour;

	for (auto &F : font_id_list) {
		if (!wcsncmp(F->name, wszName, _countof(F->name)) && !wcsncmp(F->group, wszGroup, _countof(F->group))) {
			if (GetFontSettingFromDB(F->dbSettingsGroup, F->setting, lf, &colour, F->flags) && (F->flags & FIDF_DEFAULTVALID)) {
				CreateFromFontSettings(&F->deffontsettings, lf);
				colour = F->deffontsettings.colour;
			}

			return colour;
		}
	}

	GetDefaultFontSetting(lf, &colour);
	return colour;
}

MIR_APP_DLL(COLORREF) Font_GetW(const wchar_t *wszGroup, const wchar_t *wszName, LOGFONTW *pFont)
{
	return sttGetFontWorker(wszGroup, wszName, pFont);
}

MIR_APP_DLL(COLORREF) Font_Get(const char *szGroup, const char *szName, LOGFONTA *pFont)
{
	LOGFONTW lftemp;
	COLORREF ret = sttGetFontWorker(_A2T(szGroup), _A2T(szName), &lftemp);
	ConvertLOGFONT(&lftemp, pFont);
	return ret;
}

static INT_PTR ReloadFonts(WPARAM, LPARAM)
{
	NotifyEventHooks(hFontReloadEvent, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterColour service

void UpdateColourSettings(ColourIDW *colour_id, COLORREF *colour)
{
	*colour = (COLORREF)db_get_dw(0, colour_id->dbSettingsGroup, colour_id->setting, colour_id->defcolour);
}

static INT_PTR sttRegisterColourWorker(ColourIDW *colour_id, HPLUGIN pPlugin)
{
	for (auto &C : colour_id_list)
		if (!mir_wstrcmp(C->group, colour_id->group) && !mir_wstrcmp(C->name, colour_id->name))
			return 1;

	ColourInternal* newItem = new ColourInternal;
	memset(newItem, 0, sizeof(ColourInternal));
	memcpy(newItem, colour_id, sizeof(ColourIDW));
	newItem->pPlugin = pPlugin;
	UpdateColourSettings(colour_id, &newItem->value);
	colour_id_list.insert(newItem);

	notifyOptions();
	return 0;
}

MIR_APP_DLL(int) Colour_RegisterW(ColourIDW *pFont, HPLUGIN pPlugin)
{
	return sttRegisterColourWorker(pFont, pPlugin);
}

MIR_APP_DLL(int) Colour_Register(ColourID *pFont, HPLUGIN pPlugin)
{
	ColourIDW temp;
	if (!ConvertColourID(pFont, &temp)) return -1;
	return sttRegisterColourWorker(&temp, pPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetColour service

static INT_PTR sttGetColourWorker(const wchar_t *wszGroup, const wchar_t *wszName)
{
	for (auto &C : colour_id_list)
		if (!mir_wstrcmp(C->group, wszGroup) && !mir_wstrcmp(C->name, wszName))
			return db_get_dw(0, C->dbSettingsGroup, C->setting, C->defcolour);

	return -1;
}

MIR_APP_DLL(COLORREF) Colour_GetW(const wchar_t *wszGroup, const wchar_t *wszName)
{
	return sttGetColourWorker(wszGroup, wszName);
}

MIR_APP_DLL(COLORREF) Colour_Get(const char *szGroup, const char *szName)
{
	return sttGetColourWorker(_A2T(szGroup), _A2T(szName));
}

static INT_PTR ReloadColours(WPARAM, LPARAM)
{
	NotifyEventHooks(hColourReloadEvent, 0, 0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Effects

void UpdateEffectSettings(EffectIDW *effect_id, FONTEFFECT *effectsettings)
{
	char str[256];
	mir_snprintf(str, "%sEffect", effect_id->setting);
	effectsettings->effectIndex = db_get_b(0, effect_id->dbSettingsGroup, str, effect_id->defeffect.effectIndex);

	mir_snprintf(str, "%sEffectCol1", effect_id->setting);
	effectsettings->baseColour = db_get_dw(0, effect_id->dbSettingsGroup, str, effect_id->defeffect.baseColour);

	mir_snprintf(str, "%sEffectCol2", effect_id->setting);
	effectsettings->secondaryColour = db_get_dw(0, effect_id->dbSettingsGroup, str, effect_id->defeffect.secondaryColour);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterEffect service

static int sttRegisterEffectWorker(EffectIDW *effect_id, HPLUGIN pPlugin)
{
	for (auto &E : effect_id_list)
		if (!mir_wstrcmp(E->group, effect_id->group) && !mir_wstrcmp(E->name, effect_id->name))
			return 1;

	EffectInternal* newItem = new EffectInternal;
	memset(newItem, 0, sizeof(EffectInternal));
	memcpy(newItem, effect_id, sizeof(EffectIDW));
	newItem->pPlugin = pPlugin;
	UpdateEffectSettings(effect_id, &newItem->value);
	effect_id_list.insert(newItem);

	notifyOptions();
	return 0;
}

MIR_APP_DLL(int) Effect_RegisterW(EffectIDW *pFont, HPLUGIN pPlugin)
{
	return sttRegisterEffectWorker(pFont, pPlugin);
}

MIR_APP_DLL(int) Effect_Register(EffectID *pFont, HPLUGIN pPlugin)
{
	EffectIDW temp;
	if (!ConvertEffectID(pFont, &temp)) return -1;
	return sttRegisterEffectWorker(&temp, pPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetEffect service

static INT_PTR sttGetEffectWorker(const wchar_t *wszGroup, const wchar_t *wszName, FONTEFFECT *effect)
{
	for (auto &E : effect_id_list) {
		if (!wcsncmp(E->name, wszName, _countof(E->name)) && !wcsncmp(E->group, wszGroup, _countof(E->group))) {
			UpdateEffectSettings(E, effect);
			return TRUE;
		}
	}

	return FALSE;
}

EXTERN_C MIR_APP_DLL(int) Effect_GetW(const wchar_t *wszGroup, const wchar_t *wszName, FONTEFFECT *pEffect)
{
	return sttGetEffectWorker(wszGroup, wszName, pEffect);
}

MIR_APP_DLL(int) Effect_Get(const char *szGroup, const char *szName, FONTEFFECT *pEffect)
{
	return sttGetEffectWorker(_A2T(szGroup), _A2T(szName), pEffect);
}

/////////////////////////////////////////////////////////////////////////////////////////

int OptInit(WPARAM, LPARAM);

static int OnPreShutdown(WPARAM, LPARAM)
{
	DestroyHookableEvent(hFontReloadEvent);
	DestroyHookableEvent(hColourReloadEvent);

	font_id_list.destroy();
	colour_id_list.destroy();
	return 0;
}

int LoadFontserviceModule(void)
{
	code_page = Langpack_GetDefaultCodePage();

	CreateServiceFunction(MS_FONT_RELOAD, ReloadFonts);
	CreateServiceFunction(MS_COLOUR_RELOAD, ReloadColours);

	hFontReloadEvent = CreateHookableEvent(ME_FONT_RELOAD);
	hColourReloadEvent = CreateHookableEvent(ME_COLOUR_RELOAD);

	// create generic fonts
	FontIDW fontid = {};
	strncpy(fontid.dbSettingsGroup, "Fonts", sizeof(fontid.dbSettingsGroup));
	wcsncpy_s(fontid.group, LPGENW("General"), _TRUNCATE);

	wcsncpy_s(fontid.name, LPGENW("Headers"), _TRUNCATE);
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSHEADER;
	strncpy_s(fontid.setting, "Header", _TRUNCATE);
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Generic text"), _TRUNCATE);
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSGENERAL;
	strncpy_s(fontid.setting, "Generic", _TRUNCATE);
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Small text"), _TRUNCATE);
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSSMALL;
	strncpy_s(fontid.setting, "Small", _TRUNCATE);
	g_plugin.addFont(&fontid);

	// do last for silly dyna plugin
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	return 0;
}
