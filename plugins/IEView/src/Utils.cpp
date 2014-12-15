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

#include "ieview_common.h"

wchar_t Utils::base_dir[MAX_PATH];

const wchar_t *Utils::getBaseDir()
{
	PathToAbsoluteT(_T("miranda32.exe"), base_dir);
	return base_dir;
}

wchar_t* Utils::toAbsolute(wchar_t* relative)
{
	const wchar_t* bdir = getBaseDir();
	long len = (int)wcslen(bdir);
	long tlen = len + (int)wcslen(relative);
	wchar_t* result = (wchar_t*)mir_alloc(sizeof(wchar_t)*(tlen + 1));
	if (result) {
		wcscpy(result, bdir);
		wcscpy(result + len, relative);
	}
	return result;
}

static int countNoWhitespace(const wchar_t *str)
{
	int c;
	for (c = 0; *str != '\n' && *str != '\r' && *str != '\t' && *str != ' ' && *str != '\0'; str++, c++);
	return c;
}

void Utils::appendText(char **str, int *sizeAlloced, const char *fmt, ...)
{
	va_list vararg;
	char *p;
	int size, len;

	if (str == NULL) return;

	if (*str == NULL || *sizeAlloced <= 0) {
		*sizeAlloced = size = 2048;
		*str = (char *)malloc(size);
		len = 0;
	}
	else {
		len = (int)strlen(*str);
		size = *sizeAlloced - len;
	}

	if (size < 128) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *)realloc(*str, *sizeAlloced);
	}
	p = *str + len;
	va_start(vararg, fmt);
	while (mir_vsnprintf(p, size - 1, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (char *)realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	p[size - 1] = '\0';
	va_end(vararg);
}

void Utils::appendText(wchar_t **str, int *sizeAlloced, const wchar_t *fmt, ...)
{
	va_list vararg;
	wchar_t *p;
	int size, len;

	if (str == NULL) return;

	if (*str == NULL || *sizeAlloced <= 0) {
		*sizeAlloced = size = 2048;
		*str = (wchar_t *)malloc(size);
		len = 0;
	}
	else {
		len = (int)wcslen(*str);
		size = *sizeAlloced - sizeof(wchar_t) * len;
	}

	if (size < 128) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (wchar_t *)realloc(*str, *sizeAlloced);
	}
	p = *str + len;
	va_start(vararg, fmt);
	while (mir_vsnwprintf(p, size / sizeof(wchar_t) - 1, fmt, vararg) == -1) {
		size += 2048;
		(*sizeAlloced) += 2048;
		*str = (wchar_t *)realloc(*str, *sizeAlloced);
		p = *str + len;
	}
	p[size / sizeof(wchar_t) - 1] = '\0';
	va_end(vararg);
}

void Utils::convertPath(char *path)
{
	if (path != NULL) {
		for (; *path != '\0'; path++) {
			if (*path == '\\') *path = '/';
		}
	}
}

void Utils::convertPath(wchar_t *path)
{
	if (path != NULL) {
		for (; *path != '\0'; path++) {
			if (*path == '\\') *path = '/';
		}
	}
}

int Utils::detectURL(const wchar_t *text)
{
	int i;
	for (i = 0; text[i] != '\0'; i++) {
		if (!((text[i] >= '0' && text[i] <= '9') || iswalpha(text[i]))) {
			break;
		}
	}
	if (i > 0 && text[i] == ':' && text[i + 1] == '/' && text[i + 2] == '/') {
		i += countNoWhitespace(text + i);
		for (; i > 0; i--) {
			if ((text[i - 1] >= '0' && text[i - 1] <= '9') || iswalpha(text[i - 1]) || text[i - 1] == '/') {
				break;
			}
		}
		return i;
	}
	return 0;
}

char *Utils::escapeString(const char *a)
{
	if (a == NULL)
		return NULL;

	int i, l, len = (int)strlen(a);
	for (i = l = 0; i < len; i++, l++) {
		if (a[i] == '\\' || a[i] == '\n' || a[i] == '\r' || a[i] == '\"'
			|| a[i] == '\'' || a[i] == '\b' || a[i] == '\t' || a[i] == '\f') {
			l++;
		}
	}
	char *out = (char*)mir_alloc(l + 1);
	for (i = l = 0; i < len; i++, l++) {
		if (a[i] == '\\' || a[i] == '\n' || a[i] == '\r' || a[i] == '\"'
			|| a[i] == '\'' || a[i] == '\b' || a[i] == '\t' || a[i] == '\f') {
			out[l++] = '\\';
		}
		out[l] = a[i];
	}
	out[l] = '\0';
	return out;
}

void Utils::appendIcon(char **str, int *sizeAlloced, const char *iconFile)
{
	Utils::appendText(str, sizeAlloced, "<img class=\"img\" src=\"file://%s/plugins/ieview/%s\"/> ", workingDirUtf8, iconFile);
}

bool Utils::DbEventIsForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

bool Utils::DbEventIsForHistory(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
	return et && (et->flags & DETF_HISTORY);
}