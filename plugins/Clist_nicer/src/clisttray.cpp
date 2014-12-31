/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "commonheaders.h"

#define TRAYICON_ID_BASE	100
#define TIM_CALLBACK	(WM_USER+1857)
#define TIM_CREATE		(WM_USER+1858)

extern HIMAGELIST hCListImages;

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
			if (cfg::getByte("CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
				if (cfg::getByte("CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					iIcon = IconFromStatusMode(szChangedProto, averageMode, 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return pcli->pfnTrayIconSetBaseInfo(hIcon, szChangedProto);
				}
				if (pcli->trayIcon == NULL || pcli->trayIcon[0].szProto == NULL) {
					iIcon = IconFromStatusMode(NULL, averageMode, 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
				}
				pcli->pfnTrayIconDestroy(hwnd);
				pcli->pfnTrayIconInit(hwnd);
			}
			else {
				iIcon = IconFromStatusMode(NULL, averageMode, 0, &hIcon);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
			}
		}
		else {
			switch (cfg::getByte("CList", "TrayIcon", SETTING_TRAYICON_DEFAULT)) {
			case SETTING_TRAYICON_CYCLE:
				iIcon = IconFromStatusMode(szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0), 0, &hIcon);
				pcli->cycleTimerId = SetTimer(NULL, 0, cfg::getWord("CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, pcli->pfnTrayCycleTimerProc);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);

			case SETTING_TRAYICON_MULTI:
				if (!pcli->trayIcon)
					pcli->pfnTrayIconRemove(NULL, NULL);
				else if (cfg::getByte("CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					iIcon = IconFromStatusMode(szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0), 0, &hIcon);
					hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
					return pcli->pfnTrayIconSetBaseInfo(hIcon, szChangedProto);
				}
				pcli->pfnTrayIconDestroy(hwnd);
				pcli->pfnTrayIconInit(hwnd);
				break;

			case SETTING_TRAYICON_SINGLE:
				ptrA szProto( db_get_sa(NULL, "CList", "PrimaryStatus"));
				iIcon = IconFromStatusMode(szProto, szProto ? CallProtoService(szProto, PS_GETSTATUS, 0, 0) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0), 0, &hIcon);
				hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
				return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
			}
		}
	}
	else {
		iIcon = IconFromStatusMode(NULL, averageMode, 0, &hIcon);
		hIcon = (hIcon) ? CopyIcon(hIcon) : ImageList_GetIcon(hCListImages, iIcon, ILD_NORMAL);
		return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG*)wParam;
	if (msg->message == TIM_CALLBACK && msg->lParam == WM_MOUSEMOVE ) {
		if (cfg::dat.bNoTrayTips) {
			*((LRESULT*)lParam) = 0;
			return TRUE;
		}
	}

	return coreCli.pfnTrayIconProcessMessage(wParam, lParam);
}
