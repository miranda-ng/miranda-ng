/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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

#include "stdafx.h"

#ifdef _DEBUG
int LogInit()
{
	FILE *fout = fopen("IEHistory.log", "wt");
	fclose(fout);
	return 0;
}
int Log(char *format, ...)
{
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen("IEHistory.log", "at");
	if (!fout) {
		//			MessageBox(0, "can't open file", NULL, MB_OK);
	}

	va_start(vararg, format);

	tBytes = _vsnprintf_s(str, sizeof(str), format, vararg);
	if (tBytes > 0) {
		str[tBytes] = 0;
	}

	va_end(vararg);
	if (str[mir_strlen(str) - 1] != '\n') {
		mir_strcat(str, "\n");
	}
	fputs(str, fout);
	fclose(fout);
	return 0;
}
#endif



int Info(char *title, char *format, ...)
{
	char str[4096];
	va_list vararg;
	int tBytes;
	va_start(vararg, format);
	tBytes = _snprintf_s(str, sizeof(str), format, vararg);
	if (tBytes > 0) {
		str[tBytes] = 0;
	}
	va_end(vararg);
	return MessageBoxA(nullptr, str, title, MB_OK | MB_ICONINFORMATION);
}

/*
Moves a control with regard to certain anchors (like delphi, c#, ...)
Somebody please improve on this code ...
*/

void ScreenToClient(HWND hWnd, LPRECT rect)
{
	POINT pt;
	int cx = rect->right - rect->left;
	int cy = rect->bottom - rect->top;
	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	rect->right = pt.x + cx;
	rect->bottom = pt.y + cy;
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (uint32_t)ll;
	pft->dwHighDateTime = ll >> 32;
}

void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
	FILETIME ft;
	SYSTEMTIME st;
	UnixTimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, &st);
	SystemTimeToTzSpecificLocalTime(nullptr, &st, pst);
}

MEVENT GetNeededEvent(MCONTACT contact, MEVENT hEvent, int num, int direction)
{
	for (int i = 0; i < num; ++i)
		hEvent = (direction == DIRECTION_BACK) ? db_event_prev(contact, hEvent) : db_event_next(contact, hEvent);

	return hEvent;
}

SearchResult SearchHistory(MCONTACT contact, MEVENT hFirstEvent, void *searchData, int direction, int type)
{
	if (hFirstEvent == NULL)
		hFirstEvent = (direction == DIRECTION_BACK) ? db_event_last(contact) : db_event_first(contact);

	int index = 0;
	MEVENT hEvent = hFirstEvent;
	void *buffer = nullptr;
	bool found = false;
	int oldSize, newSize;
	oldSize = newSize = 0;

	DBEVENTINFO dbEvent = {};
	while ((!found) && (hEvent)) {
		newSize = db_event_getBlobSize(hEvent);
		if (newSize > oldSize) {
			buffer = (wchar_t *)realloc(buffer, newSize);
			oldSize = newSize;
		}
		dbEvent.pBlob = (uint8_t*)buffer;
		dbEvent.cbBlob = newSize;
		if (db_event_get(hEvent, &dbEvent) == 0) { //successful
			switch (type) {
			case SEARCH_TEXT:
				{
					ptrW data(DbEvent_GetTextW(&dbEvent, CP_UTF8));
					wchar_t *tmp = wcsstr(data, (wchar_t *)searchData);
					if (tmp)
						found = true;
				}
				break;
			
			case SEARCH_TIME:
				{
					SYSTEMTIME time;
					TimeSearchData *data = (TimeSearchData *)searchData;
					UnixTimeToSystemTime((time_t)dbEvent.timestamp, &time);
					found = ((data->flags & TSDF_DATE_SET) || (data->flags & TSDF_TIME_SET)) ? true : false;
					if (data->flags & TSDF_DATE_SET)
						found = ((time.wYear == data->time.wYear) && (time.wMonth == data->time.wMonth) && (time.wDay == data->time.wDay));

					if (data->flags & TSDF_TIME_SET)
						found = found & ((time.wHour == data->time.wHour) && (time.wMinute == data->time.wMinute));
					break;
				}
			}
		}
		if (!found) {
			hEvent = GetNeededEvent(contact, hEvent, 1, direction);
			index++;
		}
	}
	free(buffer);
	SearchResult sr;
	sr.index = index;
	sr.hEvent = hEvent;
	return sr;
}
