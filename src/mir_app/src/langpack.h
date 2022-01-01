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

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetDefaultCodePage(WPARAM, LPARAM);
static INT_PTR srvGetDefaultLocale(WPARAM, LPARAM);
static INT_PTR srvReloadLangpack(WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

#define LPF_ENABLED		(1<<0)  // pack is enabled
#define LPF_NOLOCALE	(1<<1)  // pack has no valid locale
#define LPF_DEFAULT		(1<<2)  // pack is the english default (no langpack)

/* Langpack Info */
struct LANGPACK_INFO
{
	wchar_t tszLanguage[64];
	LCID Locale;
	uint16_t codepage;
	CMStringA szAuthors, szLastModifiedUsing;
	FILETIME ftFileDate;
	wchar_t tszFileName[MAX_PATH]; /* just the file name itself */
	wchar_t tszFullPath[MAX_PATH]; /* full path to the langpack */
	uint8_t flags; /* see LPIF_* flags */
};

int LangpackOptionsInit(WPARAM wParam, LPARAM);
