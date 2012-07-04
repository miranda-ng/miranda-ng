/*
CmdLine plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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

//this code is mostly taken from Miranda's langpack module.

/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project, 
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

#ifndef M_MIMCMD_LANGPACK_H
#define M_MIMCMD_LANGPACK_H

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <locale.h>


#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

struct LangPackEntry {
	unsigned linePos;
	DWORD englishHash;
	char *english;	  //not currently used, the hash does everything
	char *local;
	wchar_t *wlocal;
};

struct LangPackStruct {
	TCHAR filename[MAX_PATH];
	char  language[64];
	char  lastModifiedUsing[64];
	char  authors[256];
	char  authorEmail[128];
	struct LangPackEntry *entry;
	int entryCount;
	LCID localeID;
	DWORD defaultANSICp;
} static langPack = {0};

#ifdef Translate
#undef Translate
#define Translate(s) LangPackTranslateString(s, 0)
#endif

char *LangPackTranslateString(const char *szEnglish, const int W);
int LangPackGetDefaultCodePage();
int LangPackGetDefaultLocale();

int LangPackShutdown();
int LoadLangPackModule(char *mirandaPath);

int lpprintf(const char *format, ...);

#endif