/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "stdafx.h"

#define TRAYICON_ID_BASE 100

// don't move to win2k.h, need new and old versions to work on 9x/2000/XP
#define NIF_STATE		0x00000008
#define NIF_INFO		0x00000010

int TrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount)
{
	HICON hIcon;
	int iIcon;
	HWND hwnd = pcli->hwndContactList;

	if (netProtoCount > 1) {
		if (averageMode > 0) {
			if (db_get_b(NULL, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
				if (db_get_b(NULL, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					iIcon = IconFromStatusMode(szChangedProto, averageMode, 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return Clist_TrayIconSetBaseInfo(hIcon, szChangedProto);
				}
				if (pcli->trayIcon == nullptr || pcli->trayIcon[0].szProto == nullptr) {
					iIcon = IconFromStatusMode(nullptr, averageMode, 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return Clist_TrayIconSetBaseInfo(hIcon, nullptr);
				}
				Clist_TrayIconDestroy(hwnd);
				pcli->pfnTrayIconInit(hwnd);
			}
			else {
				iIcon = IconFromStatusMode(nullptr, averageMode, 0, &hIcon);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return Clist_TrayIconSetBaseInfo(hIcon, nullptr);
			}
		}
		else {
			switch (db_get_b(NULL, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT)) {
			case SETTING_TRAYICON_CYCLE:
				Clist_TraySetTimer();
				iIcon = IconFromStatusMode(szChangedProto, Proto_GetStatus(szChangedProto), 0, &hIcon);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return Clist_TrayIconSetBaseInfo(hIcon, nullptr);

			case SETTING_TRAYICON_MULTI:
				if (!pcli->trayIcon)
					Clist_TrayIconRemove(nullptr, nullptr);
				else if (db_get_b(NULL, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					iIcon = IconFromStatusMode(szChangedProto, Proto_GetStatus(szChangedProto), 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return Clist_TrayIconSetBaseInfo(hIcon, szChangedProto);
				}
				Clist_TrayIconDestroy(hwnd);
				pcli->pfnTrayIconInit(hwnd);
				break;

			case SETTING_TRAYICON_SINGLE:
				ptrA szProto(db_get_sa(NULL, "CList", "PrimaryStatus"));
				iIcon = IconFromStatusMode(szProto, szProto ? Proto_GetStatus(szProto) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0), 0, &hIcon);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return Clist_TrayIconSetBaseInfo(hIcon, nullptr);
			}
		}
	}
	else {
		iIcon = IconFromStatusMode(nullptr, averageMode, 0, &hIcon);
		hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
		return Clist_TrayIconSetBaseInfo(hIcon, nullptr);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG*)wParam;
	if (msg->message == TIM_CALLBACK && msg->lParam == WM_MOUSEMOVE) {
		if (cfg::dat.bNoTrayTips) {
			*((LRESULT*)lParam) = 0;
			return TRUE;
		}
	}

	return coreCli.pfnTrayIconProcessMessage(wParam, lParam);
}
