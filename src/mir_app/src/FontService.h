/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

#include "m_fontservice.h"

// a font identifier structure - used for registering a font, and getting one out again

struct FontInternal : public FontIDW
{
	FontSettingsW value;
	int hLangpack;

	__inline wchar_t* getName() const { return TranslateW_LP(name, hLangpack); }

	__inline bool isHeader() const
	{
		if ((flags & FIDF_CLASSMASK) == FIDF_CLASSHEADER)
			return true;

		if ((flags & FIDF_CLASSMASK) == 0)
			if (wcsstr(name, L"Incoming nick") || wcsstr(name, L"Outgoing nick") || wcsstr(name, L"Incoming timestamp") || wcsstr(name, L"Outgoing timestamp"))
				return true;
		return false;
	}
};

struct ColourInternal : public ColourIDW
{
	__inline wchar_t* getName() const { return TranslateW_LP(name, hLangpack); }

	COLORREF value;
	int hLangpack;
};

struct EffectInternal : public EffectIDW
{
	__inline wchar_t* getName() const { return TranslateW_LP(name, hLangpack); }

	int hLangpack;
};

/////////////////////////////////////////////////////////////////////////////////////////
// global data & functions

typedef struct
{
	char *paramName;
	wchar_t *groupName;
}
	TreeItem;

extern OBJLIST<FontInternal>   font_id_list;
extern OBJLIST<ColourInternal> colour_id_list;
extern OBJLIST<EffectInternal> effect_id_list;

extern int code_page;
extern HANDLE hFontReloadEvent, hColourReloadEvent;

int  CreateFromFontSettings(FontSettingsW *fs, LOGFONT *lf);
