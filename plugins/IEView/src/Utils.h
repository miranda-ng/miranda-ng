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
	static void appendIcon(CMStringA &str, const char *iconFile);
	static void convertPath(char *path);
	static void convertPath(wchar_t *path);
	static char *escapeString(const char *a);
	static int   detectURL(const wchar_t *text);
	static bool DbEventIsForMsgWindow(DBEVENTINFO *dbei);
	static bool DbEventIsForHistory(DBEVENTINFO *dbei);
};

#endif

