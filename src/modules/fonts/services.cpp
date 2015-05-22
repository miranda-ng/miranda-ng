/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "FontService.h"

#define FontID_OLDSIZE (offsetof(FontID, backgroundGroup))
#define FontIDW_OLDSIZE (offsetof(FontIDW, backgroundGroup))

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
	if (fid->cbSize != sizeof(FontID) && fid->cbSize != FontID_OLDSIZE)
		return false;

	memset(fidw, 0, sizeof(FontIDW));
	fidw->cbSize = sizeof(FontIDW);
	strncpy_s(fidw->dbSettingsGroup, fid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(fidw->prefix, fid->prefix, _TRUNCATE);
	fidw->flags = fid->flags;
	fidw->order = fid->order;
	ConvertFontSettings(&fid->deffontsettings, &fidw->deffontsettings);

	MultiByteToWideChar(code_page, 0, fid->group, -1, fidw->group, 64);
	MultiByteToWideChar(code_page, 0, fid->name, -1, fidw->name, 64);

	if (fid->cbSize > FontID_OLDSIZE) {
		MultiByteToWideChar(code_page, 0, fid->backgroundGroup, -1, fidw->backgroundGroup, 64);
		MultiByteToWideChar(code_page, 0, fid->backgroundName, -1, fidw->backgroundName, 64);
	}
	return true;
}

bool ConvertColourID(ColourID *cid, ColourIDW *cidw)
{
	if (cid->cbSize != sizeof(ColourID))
		return false;

	cidw->cbSize = sizeof(ColourIDW);

	strncpy_s(cidw->dbSettingsGroup, cid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(cidw->setting, cid->setting, _TRUNCATE);
	cidw->flags = cid->flags;
	cidw->defcolour = cid->defcolour;
	cidw->order = cid->order;

	MultiByteToWideChar(code_page, 0, cid->group, -1, cidw->group, 64);
	MultiByteToWideChar(code_page, 0, cid->name, -1, cidw->name, 64);
	return true;
}

bool ConvertEffectID(EffectID *eid, EffectIDW *eidw)
{
	if (eid->cbSize != sizeof(EffectID))
		return false;

	eidw->cbSize = sizeof(EffectIDW);

	strncpy_s(eidw->dbSettingsGroup, eid->dbSettingsGroup, _TRUNCATE);
	strncpy_s(eidw->setting, eid->setting, _TRUNCATE);
	eidw->flags = eid->flags;
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

	WideCharToMultiByte(code_page, 0, lfw->lfFaceName, -1, lfa->lfFaceName, LF_FACESIZE, 0, 0);
}

static void GetDefaultFontSetting(LOGFONT *lf, COLORREF* colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	if (colour)
		*colour = GetSysColor(COLOR_WINDOWTEXT);

	lf->lfHeight = 10;

	HDC hdc = GetDC(0);
	lf->lfHeight = -MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(0, hdc);
}

int GetFontSettingFromDB(char *settings_group, char *prefix, LOGFONT *lf, COLORREF *colour, DWORD flags)
{
	GetDefaultFontSetting(lf, colour);

	char idstr[256];
	if (flags & FIDF_APPENDNAME)
		mir_snprintf(idstr, SIZEOF(idstr), "%sName", prefix);
	else
		strncpy_s(idstr, prefix, _TRUNCATE);

	int retval = 0;
	ptrT tszGroup(db_get_tsa(NULL, settings_group, idstr));
	if (tszGroup != NULL)
		_tcsncpy_s(lf->lfFaceName, tszGroup, _TRUNCATE);
	else
		retval = 1;

	if (colour) {
		mir_snprintf(idstr, SIZEOF(idstr), "%sCol", prefix);
		*colour = db_get_dw(NULL, settings_group, idstr, *colour);
	}

	mir_snprintf(idstr, SIZEOF(idstr), "%sSize", prefix);
	lf->lfHeight = (char)db_get_b(NULL, settings_group, idstr, lf->lfHeight);

	mir_snprintf(idstr, SIZEOF(idstr), "%sSty", prefix);
	BYTE style = (BYTE)db_get_b(NULL, settings_group, idstr,
		(lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf->lfItalic ? DBFONTF_ITALIC : 0) | (lf->lfUnderline ? DBFONTF_UNDERLINE : 0) | lf->lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (style & DBFONTF_STRIKEOUT) != 0;

	mir_snprintf(idstr, SIZEOF(idstr), "%sSet", prefix);
	lf->lfCharSet = db_get_b(NULL, settings_group, idstr, lf->lfCharSet);

	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	if (lf->lfHeight > 0) {
		HDC hdc = GetDC(0);
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

		ReleaseDC(0, hdc);
	}

	return retval;
}

int CreateFromFontSettings(FontSettingsT *fs, LOGFONT *lf)
{
	GetDefaultFontSetting(lf, 0);

	_tcsncpy_s(lf->lfFaceName, fs->szFace, _TRUNCATE);

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

void UpdateFontSettings(FontIDW *font_id, FontSettingsT *fontsettings)
{
	LOGFONT lf;
	COLORREF colour;
	if (GetFontSettingFromDB(font_id->dbSettingsGroup, font_id->prefix, &lf, &colour, font_id->flags) && (font_id->flags & FIDF_DEFAULTVALID)) {
		CreateFromFontSettings(&font_id->deffontsettings, &lf);
		colour = font_id->deffontsettings.colour;
	}

	fontsettings->style =
		(lf.lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	fontsettings->size = (char)lf.lfHeight;
	fontsettings->charset = lf.lfCharSet;
	fontsettings->colour = colour;
	_tcsncpy_s(fontsettings->szFace, lf.lfFaceName, _TRUNCATE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterFont service

static int sttRegisterFontWorker(FontIDW *font_id, int hLangpack)
{
	if (font_id->cbSize != sizeof(FontIDW) && font_id->cbSize != FontIDW_OLDSIZE)
		return -1;

	for (int i = 0; i < font_id_list.getCount(); i++) {
		FontInternal& F = font_id_list[i];
		if (!mir_tstrcmp(F.group, font_id->group) && !mir_tstrcmp(F.name, font_id->name) && !(F.flags & FIDF_ALLOWREREGISTER))
			return 1;
	}

	char idstr[256];
	mir_snprintf(idstr, SIZEOF(idstr), "%sFlags", font_id->prefix);
	db_set_dw(0, font_id->dbSettingsGroup, idstr, font_id->flags);

	FontInternal* newItem = new FontInternal;
	memset(newItem, 0, sizeof(FontInternal));
	memcpy(newItem, font_id, font_id->cbSize);
	newItem->hLangpack = hLangpack;

	if (!mir_tstrcmp(newItem->deffontsettings.szFace, _T("MS Shell Dlg"))) {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
		mir_tstrncpy(newItem->deffontsettings.szFace, lf.lfFaceName, SIZEOF(newItem->deffontsettings.szFace));
		if (!newItem->deffontsettings.size)
			newItem->deffontsettings.size = lf.lfHeight;
	}

	UpdateFontSettings(font_id, &newItem->value);
	font_id_list.insert(newItem);
	return 0;
}

INT_PTR RegisterFontW(WPARAM wParam, LPARAM lParam)
{
	return sttRegisterFontWorker((FontIDW*)wParam, (int)lParam);
}

INT_PTR RegisterFont(WPARAM wParam, LPARAM lParam)
{
	FontIDW temp;
	if (!ConvertFontID((FontID*)wParam, &temp)) return -1;
	return sttRegisterFontWorker(&temp, (int)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetFont service

static INT_PTR sttGetFontWorker(FontIDW *font_id, LOGFONT *lf)
{
	COLORREF colour;

	for (int i = 0; i < font_id_list.getCount(); i++) {
		FontInternal& F = font_id_list[i];
		if (!_tcsncmp(F.name, font_id->name, SIZEOF(F.name)) && !_tcsncmp(F.group, font_id->group, SIZEOF(F.group))) {
			if (GetFontSettingFromDB(F.dbSettingsGroup, F.prefix, lf, &colour, F.flags) && (F.flags & FIDF_DEFAULTVALID)) {
				CreateFromFontSettings(&F.deffontsettings, lf);
				colour = F.deffontsettings.colour;
			}

			return colour;
		}
	}

	GetDefaultFontSetting(lf, &colour);
	return colour;
}

INT_PTR GetFontW(WPARAM wParam, LPARAM lParam)
{
	return sttGetFontWorker((FontIDW*)wParam, (LOGFONT*)lParam);
}

INT_PTR GetFont(WPARAM wParam, LPARAM lParam)
{
	FontIDW temp;
	if (!ConvertFontID((FontID*)wParam, &temp))
		return -1;

	LOGFONT lftemp;
	int ret = sttGetFontWorker(&temp, &lftemp);
	ConvertLOGFONT(&lftemp, (LOGFONTA*)lParam);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleFonts(int hLangpack)
{
	for (int i = font_id_list.getCount() - 1; i >= 0; i--)
		if (font_id_list[i].hLangpack == hLangpack)
			font_id_list.remove(i);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterColour service

void UpdateColourSettings(ColourIDW *colour_id, COLORREF *colour)
{
	*colour = (COLORREF)db_get_dw(NULL, colour_id->dbSettingsGroup, colour_id->setting, colour_id->defcolour);
}

static INT_PTR sttRegisterColourWorker(ColourIDW *colour_id, int hLangpack)
{
	if (colour_id->cbSize != sizeof(ColourIDW))
		return -1;

	for (int i = 0; i < colour_id_list.getCount(); i++) {
		ColourInternal& C = colour_id_list[i];
		if (!mir_tstrcmp(C.group, colour_id->group) && !mir_tstrcmp(C.name, colour_id->name))
			return 1;
	}

	ColourInternal* newItem = new ColourInternal;
	memset(newItem, 0, sizeof(ColourInternal));
	memcpy(newItem, colour_id, sizeof(ColourIDW));
	newItem->hLangpack = hLangpack;
	UpdateColourSettings(colour_id, &newItem->value);
	colour_id_list.insert(newItem);
	return 0;
}

INT_PTR RegisterColourW(WPARAM wParam, LPARAM lParam)
{
	return sttRegisterColourWorker((ColourIDW*)wParam, (int)lParam);
}

INT_PTR RegisterColour(WPARAM wParam, LPARAM lParam)
{
	ColourIDW temp;
	if (!ConvertColourID((ColourID*)wParam, &temp)) return -1;
	return sttRegisterColourWorker(&temp, (int)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetColour service

static INT_PTR sttGetColourWorker(ColourIDW *colour_id)
{
	for (int i = 0; i < colour_id_list.getCount(); i++) {
		ColourInternal& C = colour_id_list[i];
		if (!mir_tstrcmp(C.group, colour_id->group) && !mir_tstrcmp(C.name, colour_id->name))
			return db_get_dw(NULL, C.dbSettingsGroup, C.setting, C.defcolour);
	}

	return -1;
}

INT_PTR GetColourW(WPARAM wParam, LPARAM)
{
	return sttGetColourWorker((ColourIDW*)wParam);
}

INT_PTR GetColour(WPARAM wParam, LPARAM)
{
	ColourIDW temp;
	if (!ConvertColourID((ColourID*)wParam, &temp)) return -1;
	return sttGetColourWorker(&temp);
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleColours(int hLangpack)
{
	for (int i = colour_id_list.getCount() - 1; i >= 0; i--)
		if (colour_id_list[i].hLangpack == hLangpack)
			colour_id_list.remove(i);
}

//////////////////////////////////////////////////////////////////////////
// Effects

void UpdateEffectSettings(EffectIDW *effect_id, FONTEFFECT *effectsettings)
{
	char str[256];
	mir_snprintf(str, SIZEOF(str), "%sEffect", effect_id->setting);
	effectsettings->effectIndex = db_get_b(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.effectIndex);

	mir_snprintf(str, SIZEOF(str), "%sEffectCol1", effect_id->setting);
	effectsettings->baseColour = db_get_dw(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.baseColour);

	mir_snprintf(str, SIZEOF(str), "%sEffectCol2", effect_id->setting);
	effectsettings->secondaryColour = db_get_dw(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.secondaryColour);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterEffect service

static INT_PTR sttRegisterEffectWorker(EffectIDW *effect_id, int hLangpack)
{
	if (effect_id->cbSize != sizeof(EffectIDW))
		return -1;

	for (int i = 0; i < effect_id_list.getCount(); i++) {
		EffectInternal& E = effect_id_list[i];
		if (!mir_tstrcmp(E.group, effect_id->group) && !mir_tstrcmp(E.name, effect_id->name))
			return 1;
	}

	EffectInternal* newItem = new EffectInternal;
	memset(newItem, 0, sizeof(EffectInternal));
	memcpy(newItem, effect_id, sizeof(EffectIDW));
	newItem->hLangpack = hLangpack;
	UpdateEffectSettings(effect_id, &newItem->value);
	effect_id_list.insert(newItem);
	return 0;
}

INT_PTR RegisterEffectW(WPARAM wParam, LPARAM lParam)
{
	return sttRegisterEffectWorker((EffectIDW*)wParam, (int)lParam);
}

INT_PTR RegisterEffect(WPARAM wParam, LPARAM lParam)
{
	EffectIDW temp;
	if (!ConvertEffectID((EffectID*)wParam, &temp)) return -1;
	return sttRegisterEffectWorker(&temp, (int)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetEffect service

static INT_PTR sttGetEffectWorker(EffectIDW *effect_id, FONTEFFECT *effect)
{
	for (int i = 0; i < effect_id_list.getCount(); i++) {
		EffectInternal& E = effect_id_list[i];
		if (!_tcsncmp(E.name, effect_id->name, SIZEOF(E.name)) && !_tcsncmp(E.group, effect_id->group, SIZEOF(E.group))) {
			FONTEFFECT temp;
			UpdateEffectSettings(effect_id, &temp);

			effect->effectIndex = temp.effectIndex;
			effect->baseColour = temp.baseColour;
			effect->secondaryColour = temp.secondaryColour;
			return TRUE;
		}
	}

	return FALSE;
}

INT_PTR GetEffectW(WPARAM wParam, LPARAM lParam)
{
	return sttGetEffectWorker((EffectIDW*)wParam, (FONTEFFECT*)lParam);
}

INT_PTR GetEffect(WPARAM wParam, LPARAM lParam)
{
	EffectIDW temp;
	if (!ConvertEffectID((EffectID*)wParam, &temp)) return -1;
	return sttGetEffectWorker(&temp, (FONTEFFECT*)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleEffects(int hLangpack)
{
	for (int i = effect_id_list.getCount() - 1; i >= 0; i--)
		if (effect_id_list[i].hLangpack == hLangpack)
			effect_id_list.remove(i);
}
