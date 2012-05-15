/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#include "commonheaders.h"
#include "FontService.h"

COLORREF GetColorFromDefault(COLORREF cl);

#if defined( _UNICODE )
void ConvertFontSettings( FontSettings* fs, TFontSettings* fsw)
{
	fsw->colour = fs->colour;
	fsw->size = fs->size;
	fsw->style = fs->style;
	fsw->charset = fs->charset;

	MultiByteToWideChar( code_page, 0, fs->szFace, -1, fsw->szFace, LF_FACESIZE);
}

void ConvertFontID( FontID *fid, TFontID* fidw )
{
	memset(fidw, 0, sizeof(TFontID));
	fidw->cbSize = sizeof(TFontID);
	strcpy(fidw->dbSettingsGroup, fid->dbSettingsGroup);
	strcpy(fidw->prefix, fid->prefix);
	fidw->flags = fid->flags;
	fidw->order = fid->order;
	ConvertFontSettings(&fid->deffontsettings, &fidw->deffontsettings);

	MultiByteToWideChar( code_page, 0, fid->group, -1, fidw->group, 64);
	MultiByteToWideChar( code_page, 0, fid->name, -1, fidw->name, 64);
	if (fid->cbSize >= FontID_SIZEOF_V2A) {
		MultiByteToWideChar( code_page, 0, fid->backgroundGroup, -1, fidw->backgroundGroup, 64);
		MultiByteToWideChar( code_page, 0, fid->backgroundName, -1, fidw->backgroundName, 64);
	}
}

void ConvertColourID(ColourID *cid, TColourID* cidw)
{
	cidw->cbSize = sizeof(TColourID);

	strcpy(cidw->dbSettingsGroup, cid->dbSettingsGroup);
	strcpy(cidw->setting, cid->setting);
	cidw->flags = cid->flags;
	cidw->defcolour = cid->defcolour;
	cidw->order = cid->order;

	MultiByteToWideChar( code_page, 0, cid->group, -1, cidw->group, 64);
	MultiByteToWideChar( code_page, 0, cid->name, -1, cidw->name, 64);
}

void ConvertEffectID(EffectID *eid, TEffectID* eidw)
{
    eidw->cbSize = sizeof(TEffectID);

    strcpy(eidw->dbSettingsGroup, eid->dbSettingsGroup);
    strcpy(eidw->setting, eid->setting);
    eidw->flags = eid->flags;
    eidw->defeffect.effectIndex = eid->defeffect.effectIndex;
    eidw->defeffect.baseColour = eid->defeffect.baseColour;
    eidw->defeffect.secondaryColour = eid->defeffect.secondaryColour;
    eidw->order = eid->order;

    MultiByteToWideChar( code_page, 0, eid->group, -1, eidw->group, 64);
    MultiByteToWideChar( code_page, 0, eid->name, -1, eidw->name, 64);
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

	WideCharToMultiByte( code_page, 0, lfw->lfFaceName, -1, lfa->lfFaceName, LF_FACESIZE, 0, 0);
}
#endif

static void GetDefaultFontSetting(LOGFONT* lf, COLORREF* colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	if ( colour )
		*colour = GetSysColor(COLOR_WINDOWTEXT);

	lf->lfHeight = 10;
	
	HDC hdc = GetDC(0);
	lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(0, hdc);
}

int GetFontSettingFromDB(char *settings_group, char *prefix, LOGFONT* lf, COLORREF * colour, DWORD flags)
{
	DBVARIANT dbv;
	char idstr[256];
	BYTE style;
	int retval = 0;

	GetDefaultFontSetting(lf, colour);

	if(flags & FIDF_APPENDNAME) mir_snprintf(idstr, SIZEOF(idstr), "%sName", prefix);
	else mir_snprintf(idstr, SIZEOF(idstr), "%s", prefix);

	if ( !DBGetContactSettingTString(NULL, settings_group, idstr, &dbv )) {
		_tcscpy(lf->lfFaceName, dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
   else retval = 1;

	if (colour) {
		mir_snprintf(idstr, SIZEOF(idstr), "%sCol", prefix);
		*colour = DBGetContactSettingDword(NULL, settings_group, idstr, *colour);
	}

	mir_snprintf(idstr, SIZEOF(idstr), "%sSize", prefix);
	lf->lfHeight = (char)DBGetContactSettingByte(NULL, settings_group, idstr, lf->lfHeight);


	//wsprintf(idstr, "%sFlags", prefix);
	//if(DBGetContactSettingDword(NULL, settings_group, idstr, 0) & FIDF_SAVEACTUALHEIGHT) {
	//	HDC hdc = GetDC(0);
	//	lf->lfHeight = -lf->lfHeight;
	//	ReleaseDC(0, hdc);
	//}

	mir_snprintf(idstr, SIZEOF(idstr), "%sSty", prefix);
	style = (BYTE) DBGetContactSettingByte(NULL, settings_group, idstr, 
		(lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf->lfItalic ? DBFONTF_ITALIC : 0) | (lf->lfUnderline ? DBFONTF_UNDERLINE : 0) | lf->lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (style & DBFONTF_STRIKEOUT) != 0;

	mir_snprintf(idstr, SIZEOF(idstr), "%sSet", prefix);
	lf->lfCharSet = DBGetContactSettingByte(NULL, settings_group, idstr, lf->lfCharSet);

	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	if(lf->lfHeight > 0) {
		HDC hdc = GetDC(0);
		if(flags & FIDF_SAVEPOINTSIZE) {
			lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
		} else { // assume SAVEACTUALHEIGHT
			TEXTMETRIC tm;
			HFONT hFont = CreateFontIndirect(lf);
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

			GetTextMetrics(hdc, &tm);

			lf->lfHeight = -(lf->lfHeight - tm.tmInternalLeading);

			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
		}
		//lf->lfHeight = -MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(0, hdc);
	}

	return retval;
}

int CreateFromFontSettings(TFontSettings* fs, LOGFONT* lf )
{
	GetDefaultFontSetting(lf, 0);

	_tcscpy(lf->lfFaceName, fs->szFace);

	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = fs->style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (fs->style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (fs->style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = (fs->style & DBFONTF_STRIKEOUT) != 0;;
	lf->lfCharSet = fs->charset;
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	lf->lfHeight = fs->size;
	return 0;
}

void UpdateFontSettings(TFontID* font_id, TFontSettings* fontsettings)
{
	LOGFONT lf;
	COLORREF colour;
	if ( GetFontSettingFromDB(font_id->dbSettingsGroup, font_id->prefix, &lf, &colour, font_id->flags) && (font_id->flags & FIDF_DEFAULTVALID)) {
		CreateFromFontSettings(&font_id->deffontsettings, &lf );
		colour = GetColorFromDefault(font_id->deffontsettings.colour);
	}

	fontsettings->style =
		(lf.lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	fontsettings->size = (char)lf.lfHeight;
	fontsettings->charset = lf.lfCharSet;
	fontsettings->colour = colour;
	_tcscpy(fontsettings->szFace, lf.lfFaceName);
}

static COLORREF sttMixColor(COLORREF cl1, COLORREF cl2, int q)
{
	return RGB(
			(GetRValue(cl1) * q + GetRValue(cl2) * (255 - q)) / 255,
			(GetGValue(cl1) * q + GetGValue(cl2) * (255 - q)) / 255,
			(GetBValue(cl1) * q + GetBValue(cl2) * (255 - q)) / 255
		);
}

COLORREF GetColorFromDefault(COLORREF cl)
{
/*
	if (cl & 0x80000000)
		return GetSysColor(cl & 0x7fffffff);

	if (cl & 0x40000000)
	{
		switch (cl)
		{
		case MIRCOLOR_BTNHALF:	return sttMixColor(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNTEXT), 128);
		case MIRCOLOR_WNDHALF:	return sttMixColor(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_WINDOWTEXT), 128);
		case MIRCOLOR_SELHALF:	return sttMixColor(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_HIGHLIGHTTEXT), 128);
		case MIRCOLOR_INBACK:	return sttMixColor(GetSysColor(COLOR_WINDOW), RGB(0,0,255), 245);
		case MIRCOLOR_INTEXT:	return GetSysColor(COLOR_WINDOWTEXT);
		case MIRCOLOR_INHALF:	return sttMixColor(GetColorFromDefault(MIRCOLOR_INBACK), GetColorFromDefault(MIRCOLOR_INTEXT), 128);
		case MIRCOLOR_OUTBACK:	return sttMixColor(GetSysColor(COLOR_WINDOW), RGB(0,255,0), 245);
		case MIRCOLOR_OUTTEXT:	return GetSysColor(COLOR_WINDOWTEXT);
		case MIRCOLOR_OUTHALF:	return sttMixColor(GetColorFromDefault(MIRCOLOR_OUTBACK), GetColorFromDefault(MIRCOLOR_OUTTEXT), 128);
		}
	}
*/
	return cl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterFont service

static int sttRegisterFontWorker( TFontID* font_id )
{
	for ( int i = 0; i < font_id_list.getCount(); i++ ) {
		TFontID& F = font_id_list[i];
		if ( !lstrcmp( F.group, font_id->group ) && !lstrcmp( F.name, font_id->name ) && !( F.flags & FIDF_ALLOWREREGISTER ))
			return 1;
	}

	char idstr[256];
	mir_snprintf(idstr, SIZEOF(idstr), "%sFlags", font_id->prefix);
	DBWriteContactSettingDword(0, font_id->dbSettingsGroup, idstr, font_id->flags);
	{	
		TFontID* newItem = new TFontID;
		memset( newItem, 0, sizeof( TFontID ));
		memcpy( newItem, font_id, font_id->cbSize);

		if (!lstrcmp(newItem->deffontsettings.szFace, _T("MS Shell Dlg")))
		{
			LOGFONT lf;
			SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
			lstrcpyn(newItem->deffontsettings.szFace, lf.lfFaceName, SIZEOF(newItem->deffontsettings.szFace));
			if (!newItem->deffontsettings.size)
				newItem->deffontsettings.size = lf.lfHeight;
		}

		UpdateFontSettings( font_id, &newItem->value );
		font_id_list.insert( newItem );
	}
	return 0;
}

#if defined( _UNICODE )
INT_PTR RegisterFontW(WPARAM wParam, LPARAM )
{
	return sttRegisterFontWorker(( TFontID* )wParam );
}
#endif

INT_PTR RegisterFont(WPARAM wParam, LPARAM)
{
	#if defined( _UNICODE )
		TFontID temp;
		ConvertFontID( ( FontID* )wParam, &temp );
		return sttRegisterFontWorker( &temp );
	#else
		return sttRegisterFontWorker(( TFontID* )wParam );
	#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetFont service

static int sttGetFontWorker( TFontID* font_id, LOGFONT* lf )
{
	COLORREF colour;

	for ( int i = 0; i < font_id_list.getCount(); i++ ) {
		TFontID& F = font_id_list[i];
		if ( !_tcsncmp( F.name, font_id->name, SIZEOF(F.name)) && !_tcsncmp( F.group, font_id->group, SIZEOF(F.group))) {
			if ( GetFontSettingFromDB( F.dbSettingsGroup, F.prefix, lf, &colour, F.flags) && ( F.flags & FIDF_DEFAULTVALID )) {
				CreateFromFontSettings( &F.deffontsettings, lf );
				colour = GetColorFromDefault(F.deffontsettings.colour);
			}

			return (int)colour;
	}	}

	GetDefaultFontSetting( lf, &colour );
	return (int)colour;
}

#if defined( _UNICODE )
INT_PTR GetFontW(WPARAM wParam, LPARAM lParam)
{
	return sttGetFontWorker(( TFontID* )wParam, ( LOGFONT* )lParam );
}
#endif

INT_PTR GetFont(WPARAM wParam, LPARAM lParam)
{
	#if defined( _UNICODE )
		TFontID temp;
		LOGFONT lftemp;
		ConvertFontID((FontID *)wParam, &temp);
		{	int ret = sttGetFontWorker( &temp, &lftemp );
			ConvertLOGFONT( &lftemp, ( LOGFONTA* )lParam );
			return ret;
		}
	#else
		return sttGetFontWorker(( TFontID* )wParam, ( LOGFONT* )lParam );
	#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterColour service

void UpdateColourSettings( TColourID* colour_id, COLORREF *colour)
{
	*colour = ( COLORREF )DBGetContactSettingDword(NULL, colour_id->dbSettingsGroup, colour_id->setting, GetColorFromDefault(colour_id->defcolour) );
}

static int sttRegisterColourWorker( TColourID* colour_id )
{
	for ( int i = 0; i < colour_id_list.getCount(); i++ ) {
		TColourID& C = colour_id_list[i];
		if ( !_tcscmp( C.group, colour_id->group ) && !_tcscmp( C.name, colour_id->name ))
			return 1;
	}

	TColourID* newItem = new TColourID;
	memcpy( newItem, colour_id, sizeof( TColourID ));
	UpdateColourSettings( colour_id, &newItem->value );
	colour_id_list.insert( newItem );
	return 0;
}

#if defined( _UNICODE )
INT_PTR RegisterColourW(WPARAM wParam, LPARAM)
{
	return sttRegisterColourWorker(( TColourID* )wParam );
}
#endif

INT_PTR RegisterColour(WPARAM wParam, LPARAM)
{
	#if defined( _UNICODE )
		TColourID temp;
		ConvertColourID(( ColourID* )wParam, &temp );
		return sttRegisterColourWorker( &temp );
	#else
		return sttRegisterColourWorker(( TColourID* )wParam );
	#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetColour service

static int sttGetColourWorker( TColourID* colour_id )
{
	int i;

	for ( i = 0; i < colour_id_list.getCount(); i++ ) {
		TColourID& C = colour_id_list[i];
		if ( !_tcscmp( C.group, colour_id->group ) && !_tcscmp( C.name, colour_id->name ))
			return (int)DBGetContactSettingDword(NULL, C.dbSettingsGroup, C.setting, GetColorFromDefault(C.defcolour));
	}

	return -1;
}

#if defined( _UNICODE )
INT_PTR GetColourW(WPARAM wParam, LPARAM)
{
	return sttGetColourWorker(( TColourID* )wParam );
}
#endif

INT_PTR GetColour(WPARAM wParam, LPARAM)
{
	#if defined( _UNICODE )
		TColourID temp;
		ConvertColourID(( ColourID* )wParam, &temp );
		return sttGetColourWorker( &temp );
	#else
		return sttGetColourWorker(( TColourID* )wParam );
	#endif
}


//////////////////////////////////////////////////////////////////////////
// Effects

void UpdateEffectSettings(TEffectID* effect_id, TEffectSettings* effectsettings)
{
   char str[256];

   mir_snprintf(str, SIZEOF(str), "%sEffect", effect_id->setting);
   effectsettings->effectIndex = DBGetContactSettingByte(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.effectIndex);

   mir_snprintf(str, SIZEOF(str), "%sEffectCol1", effect_id->setting);
   effectsettings->baseColour = DBGetContactSettingDword(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.baseColour);

   mir_snprintf(str, SIZEOF(str), "%sEffectCol2", effect_id->setting);
   effectsettings->secondaryColour = DBGetContactSettingDword(NULL, effect_id->dbSettingsGroup, str, effect_id->defeffect.secondaryColour);

}

/////////////////////////////////////////////////////////////////////////////////////////
// RegisterFont service

static int sttRegisterEffectWorker( TEffectID* effect_id )
{
    for ( int i = 0; i < effect_id_list.getCount(); i++ ) {
        TEffectID& E = effect_id_list[i];
        if ( !_tcscmp( E.group, effect_id->group ) && !_tcscmp( E.name, effect_id->name ))
            return 1;
    }

    TEffectID* newItem = new TEffectID;
    memcpy( newItem, effect_id, sizeof( TEffectID ));
    UpdateEffectSettings( effect_id, &newItem->value );
    effect_id_list.insert( newItem );
    return 0;
}

#if defined( _UNICODE )
INT_PTR RegisterEffectW(WPARAM wParam, LPARAM lParam)
{
    return sttRegisterEffectWorker(( TEffectID* )wParam );
}
#endif

INT_PTR RegisterEffect(WPARAM wParam, LPARAM lParam)
{
#if defined( _UNICODE )
    TEffectID temp;
    ConvertEffectID( ( EffectID* )wParam, &temp );
    return sttRegisterEffectWorker( &temp );
#else
    return sttRegisterEffectWorker(( TEffectID* )wParam );
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetEffect service

static int sttGetEffectWorker( TEffectID* effect_id, FONTEFFECT* effect )
{
	for ( int i = 0; i < effect_id_list.getCount(); i++ ) {
		TEffectID& E = effect_id_list[i];
		if ( !_tcsncmp( E.name, effect_id->name, SIZEOF(E.name)) && !_tcsncmp( E.group, effect_id->group, SIZEOF(E.group))) 
		{
			TEffectSettings temp;
			UpdateEffectSettings( effect_id, &temp );

			effect->effectIndex = temp.effectIndex;
			effect->baseColour  = temp.baseColour;
			effect->secondaryColour = temp.secondaryColour;

			return (int) TRUE;
	}	}	

	return (int)FALSE;
}

#if defined( _UNICODE )
INT_PTR GetEffectW(WPARAM wParam, LPARAM lParam)
{
    return sttGetEffectWorker(( TEffectID* )wParam, ( FONTEFFECT* )lParam );
}
#endif

INT_PTR GetEffect(WPARAM wParam, LPARAM lParam)
{
#if defined( _UNICODE )
    TEffectID temp;
    ConvertEffectID((EffectID *)wParam, &temp);
    return sttGetEffectWorker( &temp, ( FONTEFFECT* )lParam );
#else
    return sttGetEffectWorker(( TEffectID* )wParam, ( FONTEFFECT* )lParam );
#endif
}
