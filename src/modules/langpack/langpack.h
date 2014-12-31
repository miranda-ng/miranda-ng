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

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvTranslateString(WPARAM wParam, LPARAM lParam);
static INT_PTR srvTranslateMenu(WPARAM wParam, LPARAM lParam);
static INT_PTR srvRegisterLP(WPARAM wParam, LPARAM lParam);
static INT_PTR srvGetDefaultCodePage(WPARAM, LPARAM);
static INT_PTR srvGetDefaultLocale(WPARAM, LPARAM);
static INT_PTR srvPcharToTchar(WPARAM wParam, LPARAM lParam);
static INT_PTR srvReloadLangpack(WPARAM wParam, LPARAM lParam);
static INT_PTR srvGetPluginLangpack(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////

#define LPF_ENABLED		(1<<0)  // pack is enabled
#define LPF_NOLOCALE	(1<<1)  // pack has no valid locale
#define LPF_DEFAULT		(1<<2)  // pack is the english default (no langpack)

/* Langpack Info */
struct LANGPACK_INFO
{
	TCHAR tszLanguage[64];
	LCID Locale;
	WORD codepage;
	CMStringA szAuthors, szAuthorEmail, szLastModifiedUsing;
	FILETIME ftFileDate;
	TCHAR tszFileName[MAX_PATH]; /* just the file name itself */
	TCHAR tszFullPath[MAX_PATH]; /* full path to the langpack */
	BYTE flags; /* see LPIF_* flags */
};

typedef BOOL(*ENUM_PACKS_CALLBACK) (LANGPACK_INFO *pack, WPARAM wParam, LPARAM lParam);
BOOL EnumLangpacks(ENUM_PACKS_CALLBACK callback, WPARAM wParam, LPARAM lParam);

int LangpackOptionsInit(WPARAM wParam, LPARAM);
