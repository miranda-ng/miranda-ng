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

#include "common.h"

/************************* Load ***************************************/

static void TrimString(char *str)
{
	int len, start;
	len = lstrlenA(str);
	while (str[0]!='\0' && (unsigned char)str[len-1] <= ' ') str[--len] = 0;
	for (start = 0; str[start] && (unsigned char)str[start] <= ' '; ++start);
	MoveMemory(str, str+start, len-start+1);
}

static BOOL IsEmpty(const char *str)
{
	int i;
	for(i = 0;str[i]!='\0';i++)
		if (str[i]!=' ' && str[i]!='\r' && str[i]!='\n')
			return FALSE;
	return TRUE;
}

static void CleanupLanguage(char *szLanguage)
{
	char *p;
	/* remove any appended ' (default)' */
	p = strstr(szLanguage, " (default)");
	if (p!=NULL) *p = '\0';
}

static void CleanupAuthors(char *szAuthors)
{
	char *p, *p2;
	/* remove trailing dot (if any) */
	p = &szAuthors[lstrlenA(szAuthors)-1];
	if (*p == '.') *p = '\0';
	/* remove any extra info in parentheses, which is ok
	 * but makes the list very long for some packs */
	for (;;) {
		p = strchr(szAuthors, '(');
		p2 = strchr(szAuthors, ')');
		if (p == NULL || p2 == NULL) {
			p = strchr(szAuthors, '[');
			p2 = strchr(szAuthors, ']');
			if (p == NULL || p2 == NULL) break;
		}
		if (*(p-1) == ' ') --p;
		MoveMemory(p, p2+1, lstrlenA(p2+1)+1);
	}
}

static void CleanupEmail(char *szAuthorEmail)
{
	char c, *p, *pAt;
	/* replace ' dot ' with '.' (may be removed) */
	p = strstr(szAuthorEmail, " dot ");
	if (p!=NULL) {
		*p = '.';
		MoveMemory(p+1, p+5, lstrlenA(p+5)+1);
	}
	/* also allow ' at ' instead of '@' for obfuscation */
	p = strstr(szAuthorEmail, " at ");
	if (p!=NULL) {
		*p = '@';
		MoveMemory(p+1, p+4, lstrlenA(p+4)+1);
	}
	/* is valid? */
	pAt = strchr(szAuthorEmail, '@');
	if (pAt == NULL) {
		szAuthorEmail[0] = '\0';
		return;
	}
	/* strip-off extra text except exactly one email address
	 * this is needed as a click on the email addres brings up the mail client */
	for(c = ' ';;c = ',') {
		p = strchr(pAt, c);
		if (p!=NULL) *p = '\0';
		p = strrchr(szAuthorEmail, c);
		if (p!=NULL) MoveMemory(szAuthorEmail, p+1, lstrlenA(p+1)+1);
		if (c == ',') break;
	}
	p = strstr(szAuthorEmail, "__");
	if (p!=NULL) MoveMemory(szAuthorEmail, p+2, lstrlenA(p+2)+1);
	/* lower case */
	CharLowerA(szAuthorEmail);
	/* 'none' specified */
	if (!lstrcmpiA(szAuthorEmail, "none")) szAuthorEmail[0] = '\0';
}

static void CleanupLastModifiedUsing(char *szLastModifiedUsing, int nSize)
{
	char *p;
	/* remove 'Unicode', as it doesn't matter */
	p = strstr(szLastModifiedUsing, " Unicode");
	if (p!=NULL) MoveMemory(p, p+8, lstrlenA(p+8)+1);
	/* use 'Miranda IM' instead of 'Miranda' */
	p = strstr(szLastModifiedUsing, "Miranda");
	if (p!=NULL && strncmp(p+7, " IM", 3)) {
		MoveMemory(p+10, p+7, lstrlenA(p+7)+1);
		CopyMemory(p+7, " IM", 3);
	}
	/* use 'Plugin' instead of 'plugin' */
	p = strstr(szLastModifiedUsing, " plugin");
	if (p!=NULL) CopyMemory(p, " Plugin", 7);
	/* remove 'v' prefix */
	p = strstr(szLastModifiedUsing, " v0.");
	if (p!=NULL) MoveMemory(p+1, p+2, lstrlenA(p+2)+1);
	/* default if empty */
	if (!szLastModifiedUsing[0]) {
		DWORD v = CallService(MS_SYSTEM_GETVERSION, 0, 0);
		mir_snprintf(szLastModifiedUsing, nSize, "%d.%d.%d.%d", ((v >> 24) & 0xFF), ((v >> 16) & 0xFF), ((v >> 8) & 0xFF), (v & 0xFF));
	}
}

// pack struct should be initialized to zero before call
// pack->szFileName needs to be filled in before call
static BOOL LoadPackData(LANGPACK_INFO *pack, BOOL useRootFolder, const char *pszFileVersionHeader)
{
	FILE *fp;
	TCHAR szFileName[MAX_PATH];
	char line[4096], *pszColon, *buf;
	char szLanguageA[64]; /* same size as pack->szLanguage */
	/*
		Miranda Language Pack Version 1
		Language: (optional)
		Locale: 0809
		Authors: Miranda NG Development Team (multiple tags allowed)
		Author-email: project-info at miranda-ng.org (" at " instead of "@" allowed)
		Last-Modified-Using: Miranda IM 0.7
		Plugins-included: (multiple tags allowed)
		X-FLName: name as used on the file listing (non-standard extension)
		X-Version: 1.2.3.4 (non-standard extension)
		see 'LangMan-Translation.txt' for some header quidelines 
	*/
	if ( !GetPackPath( szFileName, SIZEOF(szFileName), useRootFolder, pack->szFileName))
		return FALSE;

	fp = _tfopen(szFileName, _T("rt"));
	if (fp  ==  NULL)
		return FALSE;

	fgets(line, sizeof(line), fp);
	TrimString(line);
	buf = line;

	if (strlen(line) >=  3 && line[0]  ==  '\xef' && line[1]  ==  '\xbb' && line[2]  ==  '\xbf') {
		pack->codepage = CP_UTF8;
		buf += 3;
	}

	if ( lstrcmpA(buf, pszFileVersionHeader )) {
		fclose(fp);
		return FALSE;
	}
	pack->flags = LPF_NOLOCALE;
	szLanguageA[0] = '\0';
	while( !feof( fp )) {
		if ( fgets(line, sizeof(line), fp)  ==  NULL) break;
		TrimString(line);
		if ( IsEmpty(line) || line[0] == ';' || line[0] == '\0') continue;
		if ( line[0] == '[' ) break;
		pszColon = strchr(line, ':');
		if ( pszColon  ==  NULL ) continue;
		*pszColon = '\0';
		TrimString(pszColon+1);
		if ( !lstrcmpA(line, "Language") && !pack->szLanguage[0] )
			lstrcpynA(szLanguageA, pszColon+1, sizeof(szLanguageA)); /* buffer safe */
		else if ( !lstrcmpA(line, "Last-Modified-Using") && !pack->szLastModifiedUsing[0] )
			lstrcpynA(pack->szLastModifiedUsing, pszColon+1, sizeof(pack->szLastModifiedUsing)); /* buffer safe */
		else if ( !lstrcmpA(line, "Authors")) {
			buf = pack->szAuthors+lstrlenA(pack->szAuthors); /* allow multiple tags */
			if ((sizeof(pack->szAuthors)-lstrlenA(pack->szAuthors))>0) /* buffer safe */
				mir_snprintf(buf, sizeof(pack->szAuthors)-lstrlenA(pack->szAuthors), (pack->szAuthors[0] == '\0')?"%s":" %s", pszColon+1);
		} else if ( !lstrcmpA(line, "Author-email") && !pack->szAuthorEmail[0])
			lstrcpynA(pack->szAuthorEmail, pszColon+1, sizeof(pack->szAuthorEmail)); /* buffer safe */
		else if ( !lstrcmpA(line, "Locale") && (pack->flags & LPF_NOLOCALE)) {
			pack->Locale = MAKELCID((USHORT)strtol(pszColon+1, NULL, 16), SORT_DEFAULT);
			if (pack->Locale) pack->flags &= ~LPF_NOLOCALE;
		}
		else if ( !lstrcmpA(line, "Plugins-included")) {
			buf = pack->szPluginsIncluded + lstrlenA(pack->szPluginsIncluded); /* allow multiple tags */
			if (( sizeof(pack->szPluginsIncluded)-lstrlenA(pack->szPluginsIncluded)) > 0 ) /* buffer safe */
				mir_snprintf(buf, sizeof(pack->szPluginsIncluded)-lstrlenA(pack->szPluginsIncluded), (pack->szPluginsIncluded[0] == '\0')?"%s":", %s", CharLowerA(pszColon+1));
		}
		else if ( !lstrcmpA(line, "X-Version") && !pack->szVersion[0] )
			lstrcpynA(pack->szVersion, pszColon+1, sizeof(pack->szVersion)); /* buffer safe */
		else if ( !lstrcmpA(line, "X-FLName") && !pack->szFLName[0] )
			lstrcpynA(pack->szFLName, pszColon+1, sizeof(pack->szFLName)); /* buffer safe */
	}
	CleanupLanguage(szLanguageA);
	CleanupAuthors(pack->szAuthors);
	CleanupEmail(pack->szAuthorEmail);
	CleanupLastModifiedUsing(pack->szLastModifiedUsing, sizeof(pack->szLastModifiedUsing));
	/* codepage */
	if (!(pack->flags&LPF_NOLOCALE))
		if (GetLocaleInfoA(pack->Locale, LOCALE_IDEFAULTANSICODEPAGE, line, 6))
			pack->codepage = (WORD)atoi(line); /* CP_ACP on error */
	/* language */

	MultiByteToWideChar(pack->codepage, 0, szLanguageA, -1, pack->szLanguage, SIZEOF(pack->szLanguage));

	/* ensure the pack always has a language name */
	if (!pack->szLanguage[0] && !GetLocaleInfo(pack->Locale, LOCALE_SENGLANGUAGE, pack->szLanguage, SIZEOF(pack->szLanguage))) {
		TCHAR *p;
		lstrcpyn(pack->szLanguage, pack->szFileName, SIZEOF(pack->szLanguage)); /* buffer safe */
		p = _tcsrchr(pack->szLanguage, _T('.'));
		if (p!=NULL) *p = '\0';
	}
	/* ensure the pack always has a filelisting name */
	if (!pack->szFLName[0])
		lstrcatA(lstrcpyA(pack->szFLName, szLanguageA), " Language Pack"); /* buffer safe */
	fclose(fp);
	return TRUE;
}

/************************* Enum ***************************************/

BOOL GetPackPath(TCHAR *pszPath, int nSize, BOOL useRootFolder, const TCHAR *pszFile)
{
	TCHAR *p;
	/* main path */
	if (!GetModuleFileName(NULL, pszPath, nSize)) return FALSE;
	p = _tcsrchr(pszPath, _T('\\'));
	if (p!=NULL) *(p+1) = _T('\0');
	/* subdirectory */
	if (!useRootFolder) {
		if (nSize<(lstrlen(pszPath)+10)) return FALSE;
		lstrcat(pszPath, _T("Languages\\"));
	}
	/* file name */
	if (pszFile!=NULL) {
		if (nSize<(lstrlen(pszFile)+11)) return FALSE;
		lstrcat(pszPath, pszFile);
	}
	return TRUE;
}

// callback is allowed to be NULL
// returns TRUE if any pack exists except default
BOOL EnumPacks(ENUM_PACKS_CALLBACK callback, const TCHAR *pszFilePattern, const char *pszFileVersionHeader, WPARAM wParam, LPARAM lParam)
{
	BOOL fPackFound = FALSE;
	BOOL res = FALSE;
	LANGPACK_INFO pack;
	WIN32_FIND_DATA wfd;	
	HANDLE hFind;

	mir_ptr<TCHAR> langpack(db_get_tsa(NULL, "LangMan", "Langpack"));

	/* langpacks in root folder */
	if (GetPackPath(pack.szFileName, SIZEOF(pack.szFileName), TRUE, pszFilePattern)) {
		hFind = FindFirstFile(pack.szFileName, &wfd);
		if (hFind!=INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;
				if ((lstrlen(wfd.cFileName)<4) || wfd.cFileName[lstrlen(wfd.cFileName)-4]!=_T('.')) continue;
				/* get data */
				ZeroMemory(&pack, sizeof(pack));
				lstrcpy(pack.szFileName, CharLower(wfd.cFileName)); /* buffer safe */
				if (LoadPackData(&pack, TRUE, pszFileVersionHeader)) {
					pack.ftFileDate = wfd.ftLastWriteTime;
					/* enabled? */
					if (!langpack) {
						if (!fPackFound) pack.flags |= LPF_ENABLED;
						fPackFound = TRUE;
					}
					/* callback */
					if (callback!=NULL) res = callback(&pack, wParam, lParam);
					if (!res) { FindClose(hFind); return FALSE; }
				}
			} while(FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}

	/* langpacks in languages folder */
	if (GetPackPath(pack.szFileName, SIZEOF(pack.szFileName), FALSE, pszFilePattern)) {
		hFind = FindFirstFile(pack.szFileName, &wfd);
		if (hFind!=INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;
				if (lstrlen(wfd.cFileName)<4 || wfd.cFileName[lstrlen(wfd.cFileName)-4]!=_T('.')) continue;
				/* get data */
				ZeroMemory(&pack, sizeof(pack));
				lstrcpy(pack.szFileName, CharLower(wfd.cFileName)); /* buffer safe */
				if (LoadPackData(&pack, FALSE, pszFileVersionHeader)) {
					pack.ftFileDate = wfd.ftLastWriteTime;
					/* enabled? */
					if (langpack) {
						if (!_tcscmp(pack.szFileName, langpack)) {
							if (!fPackFound) pack.flags |= LPF_ENABLED;
							fPackFound = TRUE;
						}
					}
					/* callback */
					if (callback!=NULL) res = callback(&pack, wParam, lParam);
					if (!res) { FindClose(hFind); return FALSE; }
				}
			} while(FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}

	/* default: English (GB) */
	if (callback!=NULL) {
		ZeroMemory(&pack, sizeof(pack));
		pack.Locale = LOCALE_USER_DEFAULT; /* miranda uses default locale in this case */
		lstrcpy(pack.szLanguage, _T("English (default)")); /* buffer safe */
		lstrcpyA(pack.szAuthors, "Miranda NG Development Team"); /* buffer safe */
		lstrcpyA(pack.szAuthorEmail, "project-info at miranda-ng.org"); /* buffer safe */
		CleanupEmail(pack.szAuthorEmail); /* correct " at " */
		CleanupLastModifiedUsing(pack.szLastModifiedUsing, sizeof(pack.szLastModifiedUsing));
		/* file date */
		if (GetModuleFileName(NULL, pack.szFileName, SIZEOF(pack.szFileName))) {
			HANDLE hFile;
			hFile = CreateFile(pack.szFileName, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (hFile!=INVALID_HANDLE_VALUE) {
				GetFileTime(hFile, NULL, NULL, &pack.ftFileDate);
				CloseHandle(hFile);
			}
		}
		pack.flags = LPF_NOLOCALE|LPF_DEFAULT;
		pack.Locale = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		if (!fPackFound) pack.flags |= LPF_ENABLED;
		/* callback */
		if (!callback(&pack, wParam, lParam)) return FALSE;
	}

	return fPackFound;
}

void MovePacks(const TCHAR *pszFilePattern)
{
	TCHAR szFrom[MAX_PATH], szDest[MAX_PATH], szDir[MAX_PATH], *pszFile;
	BOOL fDirCreated = FALSE;
	HANDLE hFind;
	WIN32_FIND_DATA wfd;

	/* main path */
	if (!GetModuleFileName(NULL, szDir, SIZEOF(szDir))) return;
	pszFile = _tcsrchr(szDir, _T('\\'));
	if (pszFile != NULL) *pszFile = _T('\0');

	if (!GetPackPath(szDest, SIZEOF(szDest), FALSE, _T(""))) return;

	/* move wrongly placed packs from 'root' to 'Language' */
	mir_sntprintf(szFrom, SIZEOF(szFrom), _T("%s\\%s"), szDir, pszFilePattern);
	hFind = FindFirstFile(szFrom, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) continue;
			if (lstrlen(wfd.cFileName)<4 || wfd.cFileName[lstrlen(wfd.cFileName) - 4] != _T('.')) continue;
			/* set first lp ad default */
			mir_ptr<TCHAR> langpack(db_get_tsa(NULL, "LangMan", "Langpack"));
			if (!langpack)
				db_set_ws(NULL, "LangMan", "Langpack", wfd.cFileName);
			/* ensure dir exists */
			if (!fDirCreated)
				fDirCreated = CreateDirectory(szDest, NULL);
			/* move file */
			if (GetPackPath(szDest, SIZEOF(szDest), FALSE, wfd.cFileName))
			{
				mir_sntprintf(szFrom, SIZEOF(szFrom), _T("%s\\%s"), szDir, wfd.cFileName);
				if (!MoveFile(szFrom, szDest) && GetLastError() == ERROR_ALREADY_EXISTS) {
					DeleteFile(szDest);
					MoveFile(szFrom, szDest);
				}
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}