/*

'Language Pack Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define LPF_ENABLED   0x01  // pack is enabled
#define LPF_NOLOCALE  0x02  // pack has no valid locale
#define LPF_DEFAULT   0x04  // pack is the english default (no langpack)

/* Langpack Info */
typedef struct {
	TCHAR szLanguage[64];
	LCID Locale;
	WORD codepage;
	char szAuthors[1024];
	char szAuthorEmail[128];
	char szLastModifiedUsing[64];
	char szPluginsIncluded[4080];
	char szVersion[21];
	char szFLName[128];
	FILETIME ftFileDate;
	TCHAR szFileName[MAX_PATH]; /* just the file name itself */
	BYTE flags; /* see LPIF_* flags */
} LANGPACK_INFO;

BOOL GetPackPath(TCHAR *pszPath, int nSize, BOOL useRootFolder, const TCHAR *pszFile);

void MovePacks(const TCHAR *pszFilePattern);

/* Enum */
typedef BOOL (*ENUM_PACKS_CALLBACK)(LANGPACK_INFO *pack, WPARAM wParam, LPARAM lParam);
BOOL EnumPacks(ENUM_PACKS_CALLBACK callback, const TCHAR *pszFilePattern, const char *pszFileVersionHeader, WPARAM wParam, LPARAM lParam);
