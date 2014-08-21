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

#ifndef M_IEHISTORY_UTILS_H
#define M_IEHISTORY_UTILS_H

#include "stdafx.h"
#include "time.h"

#define ANCHOR_LEFT     0x000001
#define ANCHOR_RIGHT		0x000002
#define ANCHOR_TOP      0x000004
#define ANCHOR_BOTTOM   0x000008
#define ANCHOR_ALL      ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM

#define DIRECTION_BACK			0x000001
#define DIRECTION_FORWARD		0x000002

#define SEARCH_TEXT					0x000001
#define SEARCH_TIME					0x000002

/*
#define SIDE_LEFT				  0x000001
#define SIDE_TOP				  0x000002
#define SIDE_RIGHT			  0x000004
#define SIDE_BOTTOM		  	0x000008
#define SIDE_TOPLEFT		  SIDE_TOP | SIDE_LEFT
#define SIDE_TOPRIGHT		  SIDE_TOP | SIDE_RIGHT
#define SIDE_BOTTOMLEFT   SIDE_BOTTOM | SIDE_LEFT
#define SIDE_BOTTOMRIGHT	SIDE_BOTTOM | SIDE_RIGHT
*/

#define TSDF_TIME_SET						0x00000001
#define TSDF_DATE_SET						0x00000002

struct TimeSearchData{
	int flags;
	SYSTEMTIME time;
};

struct SearchResult{
	long index;
	HANDLE hEvent;
};

//#ifdef _DEBUG
int LogInit();
int Log(char *format, ...);
//#endif

void ScreenToClient(HWND hWnd, LPRECT rect);

int Info(char *title, char *format, ...);
TCHAR *GetContactName(HANDLE hContact, char *szProto);
void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors);
RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors);

void UnixTimeToFileTime(time_t t, LPFILETIME pft);
void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst);

HANDLE GetNeededEvent(HANDLE hLastFirstEvent, int index, int direction);
SearchResult SearchHistory(HANDLE hContact, HANDLE hFirstEvent,  void *searchData, int direction, int type);

extern void *(*MirandaMalloc)(size_t size);
extern void *(*MirandaRealloc)(void *data, size_t newSize); 
extern void (*MirandaFree) (void *data);

/*
static __inline int mir_snprintf(char *buffer, size_t count, const char* fmt, ...) {
	va_list va;
	int len;

	va_start(va, fmt);
	len = _vsnprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}
*/
#endif