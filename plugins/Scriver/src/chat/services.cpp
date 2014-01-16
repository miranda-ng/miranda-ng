/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#include "../commonheaders.h"

void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground)
{
	HWND hParent = NULL;
	if (!si)
		return;

	//Do we need to create a window?
	if (si->hWnd == NULL) {
		hParent = GetParentWindow(si->windowData.hContact, TRUE);
		si->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), hParent, RoomWndProc, (LPARAM)si);
	}
	SendMessage(si->hWnd, DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(si->hWnd), CM_ACTIVATECHILD, 0, (LPARAM)si->hWnd);
	SendMessage(GetParent(si->hWnd), CM_POPUPWINDOW, 0, (LPARAM)si->hWnd);
	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_CHAT_MESSAGE));
}

void LoadModuleIcons(MODULEINFO *mi)
{
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);

	int overlayIcon = ImageList_AddIcon(hList, GetCachedIcon("chat_overlay"));
	ImageList_SetOverlayImage(hList, overlayIcon, 1);
	
	mi->hOnlineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_ONLINE);
	mi->hOnlineIcon = LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_ONLINE);
	int index = ImageList_AddIcon(hList, mi->hOnlineIcon); 
	mi->hOnlineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	mi->hOfflineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_OFFLINE);
	mi->hOfflineIcon = LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_OFFLINE);
	index = ImageList_AddIcon(hList, mi->hOfflineIcon); 
	mi->hOfflineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	ImageList_Destroy(hList);
}
