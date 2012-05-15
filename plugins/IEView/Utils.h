/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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
class Utils;

#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#include "ieview_common.h"

class Utils {
private:
	static unsigned hookNum;
	static unsigned serviceNum;
	static HANDLE* hHooks;
	static HANDLE* hServices;
	static wchar_t base_dir[MAX_PATH];
public:
	static const wchar_t *getBaseDir();
	static wchar_t* toAbsolute(wchar_t* relative);
	static void appendIcon(char **str, int *sizeAlloced, const char *iconFile);
	static void appendText(char **str, int *sizeAlloced, const char *fmt, ...);
	static void appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...);
	static void convertPath(char *path);
	static void convertPath(wchar_t *path);
	static char *dupString(const char *a);
	static char *dupString(const char *a, int l);
	static wchar_t *dupString(const wchar_t *a);
	static wchar_t *dupString(const wchar_t *a, int l);
	static wchar_t *convertToWCS(const char *a);
	static wchar_t *convertToWCS(const char *a, int cp);
	static char *convertToString(const wchar_t *a);
	static char *convertToString(const wchar_t *a, int cp);
	static char *escapeString(const char *a);
	static DWORD safe_wcslen(wchar_t *msg, DWORD maxLen);
	static char *UTF8Encode(const wchar_t *wtext);
	static char *UTF8Encode(const char *text);
	static void  UTF8Encode(const char *text, char *output, int maxLen);
	static void  UTF8Encode(const wchar_t *text, char *output, int maxLen);
	static int   detectURL(const wchar_t *text);
	static HANDLE hookEvent_Ex(const char *name, MIRANDAHOOK hook);
	static HANDLE createServiceFunction_Ex(const char *name, MIRANDASERVICE service);
	static void unhookEvents_Ex();
	static void destroyServices_Ex();
	static unsigned long forkThread(void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg);
	static wchar_t *urlEncode(const wchar_t *a);
	static wchar_t *urlEncode(const char *a);

};

#endif

