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
	if (!fout)
	{
		//			MessageBox(0, "can't open file", NULL, MB_OK);
	}

	va_start(vararg, format);

	tBytes = _vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
	{
		str[tBytes] = 0;
	}

	va_end(vararg);
	if (str[strlen(str) - 1] != '\n')
	{
		strcat(str, "\n");
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
	tBytes = snprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
	{
		str[tBytes] = 0;
	}
	va_end(vararg);
	return MessageBoxA(0, str, title, MB_OK | MB_ICONINFORMATION);
}

/*
returns the name of a contact
*/

TCHAR *GetContactName(MCONTACT contact)
{
	CONTACTINFO ctInfo = { sizeof(ctInfo) };
	//	if(db_mc_isMeta(contact))
	//		contact=db_mc_getMostOnline(contact);
	ctInfo.szProto = GetContactProto(contact);
	ctInfo.dwFlag = CNF_DISPLAY;
#ifdef _UNICODE
	ctInfo.dwFlag += CNF_UNICODE;
#endif
	ctInfo.hContact = contact;
	if (CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ctInfo)){
		return NULL;
	}
	TCHAR* buffer = _tcsdup(ctInfo.pszVal);
	mir_free(ctInfo.pszVal);
	return buffer;
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

void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors)
{
	RECT rParent;
	RECT rChild;

	if (parentPos->flags & SWP_NOSIZE)
	{
		return;
	}
	GetWindowRect(parentPos->hwnd, &rParent);
	rChild = AnchorCalcPos(window, &rParent, parentPos, anchors);
	MoveWindow(window, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, FALSE);
}

RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors)
{
	RECT rChild;
	RECT rTmp;

	GetWindowRect(window, &rChild);
	ScreenToClient(parentPos->hwnd, &rChild);

	int cx = rParent->right - rParent->left;
	int cy = rParent->bottom - rParent->top;
	if ((cx == parentPos->cx) && (cy == parentPos->cy))
	{
		return rChild;
	}
	if (parentPos->flags & SWP_NOSIZE)
	{
		return rChild;
	}

	rTmp.left = parentPos->x - rParent->left;
	rTmp.right = (parentPos->x + parentPos->cx) - rParent->right;
	rTmp.bottom = (parentPos->y + parentPos->cy) - rParent->bottom;
	rTmp.top = parentPos->y - rParent->top;

	cx = (rTmp.left) ? -rTmp.left : rTmp.right;
	cy = (rTmp.top) ? -rTmp.top : rTmp.bottom;

	rChild.right += cx;
	rChild.bottom += cy;
	//expanded the window accordingly, now we need to enforce the anchors
	if ((anchors & ANCHOR_LEFT) && (!(anchors & ANCHOR_RIGHT)))
	{
		rChild.right -= cx;
	}
	if ((anchors & ANCHOR_TOP) && (!(anchors & ANCHOR_BOTTOM)))
	{
		rChild.bottom -= cy;
	}
	if ((anchors & ANCHOR_RIGHT) && (!(anchors & ANCHOR_LEFT)))
	{
		rChild.left += cx;
	}
	if ((anchors & ANCHOR_BOTTOM) && (!(anchors & ANCHOR_TOP)))
	{
		rChild.top += cy;
	}
	return rChild;
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
	FILETIME ft;
	SYSTEMTIME st;
	UnixTimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, &st);
	SystemTimeToTzSpecificLocalTime(NULL, &st, pst);
}

MEVENT GetNeededEvent(MEVENT hEvent, int num, int direction)
{
	int i;
	typedef MEVENT(__stdcall *db_event_step_t)(MCONTACT hContact, MEVENT hDbEvent);
	db_event_step_t db_event_step;
	if (direction == DIRECTION_BACK)
		db_event_step = db_event_prev;
	else
		db_event_step = db_event_next;

	for (i = 0; i < num; ++i)
		hEvent = db_event_step(0, hEvent);

	return hEvent;
}

SearchResult SearchHistory(MCONTACT contact, MEVENT hFirstEvent, void *searchData, int direction, int type)
{
	if (hFirstEvent == NULL){
		typedef MEVENT(__stdcall *db_event_start_t)(MCONTACT contact);
		db_event_start_t db_event_start = (direction == DIRECTION_BACK) ? db_event_last : db_event_first;
		hFirstEvent = db_event_start(contact);
	}
	int index = 0;
	MEVENT hEvent = hFirstEvent;
	void *buffer = NULL;
	TCHAR *search;
	bool found = false;
	int oldSize, newSize;
	oldSize = newSize = 0;

	DBEVENTINFO dbEvent = { 0 };
	dbEvent.cbSize = sizeof(dbEvent);

	while ((!found) && (hEvent)){
		newSize = db_event_getBlobSize(hEvent);
		if (newSize > oldSize)
		{
			buffer = (TCHAR *)realloc(buffer, newSize);
			oldSize = newSize;
		}
		dbEvent.pBlob = (PBYTE)buffer;
		dbEvent.cbBlob = newSize;
		if (db_event_get(hEvent, &dbEvent) == 0){ //successful
			switch (type)
			{
			case SEARCH_TEXT:
			{
#ifdef _UNICODE
				wchar_t TEMP[2048];
				size_t size = strlen((char *)dbEvent.pBlob) + 1;
				if (size < dbEvent.cbBlob)
				{
					search = (wchar_t *)&dbEvent.pBlob[size];
				}
				else{
					MultiByteToWideChar(CP_ACP, 0, (char *)buffer, (int)size, TEMP, 2048);
					search = TEMP;
				}
#else
				search = (char *) buffer;
#endif
				TCHAR *data = (TCHAR *)searchData;
				TCHAR *tmp = _tcsstr(search, data);
				if (tmp)
				{
					found = true;
				}
				break;
			}
			case SEARCH_TIME:
			{
				SYSTEMTIME time;
				TimeSearchData *data = (TimeSearchData *)searchData;
				UnixTimeToSystemTime((time_t)dbEvent.timestamp, &time);
				found = ((data->flags & TSDF_DATE_SET) || (data->flags & TSDF_TIME_SET)) ? true : false;
				if (data->flags & TSDF_DATE_SET)
				{
					found = ((time.wYear == data->time.wYear) && (time.wMonth == data->time.wMonth) && (time.wDay == data->time.wDay));
				}
				if (data->flags & TSDF_TIME_SET)
				{
					found = found & ((time.wHour == data->time.wHour) && (time.wMinute == data->time.wMinute));
				}
				break;
			}
			}
		}
		if (!found){
			hEvent = GetNeededEvent(hEvent, 1, direction);
			index++;
		}
	}
	free(buffer);
	SearchResult sr;
	sr.index = index;
	sr.hEvent = hEvent;
	return sr;
}
