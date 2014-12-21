/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#ifndef __notifications_h__
#define __notifications_h__

struct POPUPTREEDATA
{
	int cbSize;
	DWORD signature;
	LPTSTR pszTreeRoot;
	LPTSTR pszDescription;
	BYTE typ;
	union {
		POPUPNOTIFICATION	notification;
		POPUPCLASS			pupClass;
	};
	BYTE enabled;
	DWORD disableWhen;
	int timeoutValue;
	char leftAction[MAXMODULELABELLENGTH];
	char rightAction[MAXMODULELABELLENGTH];
	COLORREF colorBack;			//really needed ??
	COLORREF colorText;			//really needed ??
};

extern LIST<POPUPTREEDATA> gTreeData;
extern HANDLE g_hntfError, g_hntfWarning, g_hntfNotification;

int TreeDataSortFunc(const POPUPTREEDATA *p1, const POPUPTREEDATA *p2);
HANDLE FindTreeData(LPTSTR group, LPTSTR name, BYTE typ);
void FreePopupClass(POPUPTREEDATA *ptd);
void UnloadTreeData();

void LoadNotifications();
HANDLE RegisterNotification(POPUPNOTIFICATION *notification);

void FillNotificationData(POPUPDATA2 *ppd, DWORD *disableWhen);
bool PerformAction(HANDLE hNotification, HWND hwnd, UINT message, WPARAM wparal, LPARAM lparam);
bool IsValidNotification(HANDLE hNotification);

void LoadNotificationSettings(POPUPTREEDATA *ptd, char* szModul);
void SaveNotificationSettings(POPUPTREEDATA *ptd, char* szModul);

#endif // __notifications_h__
