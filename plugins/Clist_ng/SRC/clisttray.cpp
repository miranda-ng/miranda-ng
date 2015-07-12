/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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

UNICODE done

*/
#include "commonheaders.h"

#define TRAYICON_ID_BASE    100
#define TIM_CALLBACK   (WM_USER+1857)
#define TIM_CREATE     (WM_USER+1858)

#define NIF_STATE       0x00000008
#define NIF_INFO        0x00000010

void TrayIconUpdateBase(const char *szChangedProto)
{
	int i,count,netProtoCount,changed = -1;
	PROTOACCOUNT **accs;
	int averageMode = 0;
	HWND hwnd = pcli->hwndContactList;

	if (pcli->cycleTimerId)
		KillTimer(NULL, pcli->cycleTimerId); pcli->cycleTimerId = 0;

	Proto_EnumAccounts( &count, &accs );
	for (i = 0,netProtoCount = 0; i < count; i++) {
		if ( !pcli->pfnGetProtocolVisibility( accs[i]->szModuleName ))
			continue;
		netProtoCount++;
		if (!lstrcmpA(szChangedProto, accs[i]->szModuleName ))
			pcli->cycleStep = i;
		if (averageMode == 0)
			averageMode = CallProtoService( accs[i]->szModuleName, PS_GETSTATUS, 0, 0);
		else if (averageMode != CallProtoService( accs[i]->szModuleName, PS_GETSTATUS, 0, 0)) {
			averageMode = -1; break;
		}
	}
	if (netProtoCount > 1) {
		if (averageMode > 0) {
			if (cfg::getByte("CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
				if (cfg::getByte("CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					HICON hIcon = 0;
					int iIcon = CLC::IconFromStatusMode(szChangedProto, averageMode, 0, &hIcon);
					if(hIcon)
						changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), szChangedProto);
					else
						changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), szChangedProto);
				}
				else if (pcli->trayIcon && pcli->trayIcon[0].szProto != NULL) {
					pcli->pfnTrayIconDestroy(hwnd);
					pcli->pfnTrayIconInit(hwnd);
				}
				else {
					HICON hIcon = 0;
					int iIcon = CLC::IconFromStatusMode(NULL, averageMode, 0, &hIcon);

					if(hIcon)
						changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), NULL);
					else
						changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), NULL);
				}
			} else {
				HICON hIcon = 0;
				int iIcon = CLC::IconFromStatusMode(NULL, averageMode, 0, &hIcon);

				if(hIcon)
					changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), NULL);
				else
					changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), NULL);
			}
		} else {
			switch (cfg::getByte("CList", "TrayIcon", SETTING_TRAYICON_DEFAULT)) {
			case SETTING_TRAYICON_SINGLE:
				{
					DBVARIANT dbv = {DBVT_DELETED};
					int iIcon = 0;
					HICON hIcon = 0;
					char *szProto;
					if (cfg::getString(NULL, "CList", "PrimaryStatus", &dbv))
						szProto = NULL;
					else
						szProto = dbv.pszVal;
					iIcon = CLC::IconFromStatusMode(szProto, szProto ? CallProtoService(szProto, PS_GETSTATUS, 0, 0) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0), 0, &hIcon);
					if(hIcon)
						changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), NULL);
					else
						changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), NULL);
					db_free(&dbv);
					break;
				}
			case SETTING_TRAYICON_CYCLE:
				{
					HICON hIcon = 0;
					int iIcon = CLC::IconFromStatusMode(szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0), 0, &hIcon);

					pcli->cycleTimerId = SetTimer(NULL, 0, cfg::getWord("CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, pcli->pfnTrayCycleTimerProc);
					if(hIcon)
						changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), NULL);
					else
						changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), NULL);
					break;
				}
			case SETTING_TRAYICON_MULTI:
				if ( !pcli->trayIcon )
					pcli->pfnTrayIconRemove(NULL, NULL);
				else if (cfg::getByte("CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					HICON hIcon = 0;
					int iIcon = CLC::IconFromStatusMode(szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0), 0, &hIcon);

					if(hIcon)
						changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), szChangedProto);
					else
						changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), szChangedProto);
				}
				else {
					pcli->pfnTrayIconDestroy(hwnd);
					pcli->pfnTrayIconInit(hwnd);
				}
				break;
			}
		}
	} else {
		HICON hIcon = 0;
		int iIcon = CLC::IconFromStatusMode(NULL, averageMode, 0, &hIcon);
		if ( hIcon )
			changed = pcli->pfnTrayIconSetBaseInfo(CopyIcon(hIcon), NULL);
		else
			changed = pcli->pfnTrayIconSetBaseInfo(ImageList_GetIcon(CLC::hClistImages, iIcon, ILD_NORMAL), NULL);
	}
	if (changed != -1 && pcli->trayIcon[changed].isBase)
		pcli->pfnTrayIconUpdate( pcli->trayIcon[changed].hBaseIcon, NULL, pcli->trayIcon[changed].szProto, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CLC::TrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG* msg = ( MSG* )wParam;
	if ( msg->message == TIM_CALLBACK && msg->lParam == WM_MOUSEMOVE ) {
		if ( cfg::dat.bNoTrayTips ) {
			*((LRESULT *) lParam) = 0;
			return TRUE;
	}	}

	return coreCli.pfnTrayIconProcessMessage(wParam, lParam);
}
