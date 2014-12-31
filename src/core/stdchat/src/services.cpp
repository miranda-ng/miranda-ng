/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#include "chat.h"

void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground)
{
	if (!si)
		return;

	if (g_Settings.bTabsEnable) {
		// the session is not the current tab, so we copy the necessary
		// details into the SESSION_INFO for the tabbed window
		if (!si->hWnd) {
			g_TabSession.iEventCount = si->iEventCount;
			g_TabSession.iStatusCount = si->iStatusCount;
			g_TabSession.iType = si->iType;
			g_TabSession.nUsersInNicklist = si->nUsersInNicklist;
			g_TabSession.pLog = si->pLog;
			g_TabSession.pLogEnd = si->pLogEnd;
			g_TabSession.pMe = si->pMe;
			g_TabSession.dwFlags = si->dwFlags;
			g_TabSession.pStatuses = si->pStatuses;
			g_TabSession.ptszID = si->ptszID;
			g_TabSession.pszModule = si->pszModule;
			g_TabSession.ptszName = si->ptszName;
			g_TabSession.ptszStatusbarText = si->ptszStatusbarText;
			g_TabSession.ptszTopic = si->ptszTopic;
			g_TabSession.pUsers = si->pUsers;
			g_TabSession.hContact = si->hContact;
			g_TabSession.wStatus = si->wStatus;
			g_TabSession.lpCommands = si->lpCommands;
			g_TabSession.lpCurrentCommand = NULL;
		}

		// Do we need to create a tabbed window?
		if (g_TabSession.hWnd == NULL)
			g_TabSession.hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), NULL, RoomWndProc, (LPARAM)&g_TabSession);

		SetWindowLongPtr(g_TabSession.hWnd, GWL_EXSTYLE, GetWindowLongPtr(g_TabSession.hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW);

		// if the session was not the current tab we need to tell the window to
		// redraw to show the contents of the current SESSION_INFO
		if (!si->hWnd) {
			pci->SM_SetTabbedWindowHwnd(si, g_TabSession.hWnd);
			SendMessage(g_TabSession.hWnd, GC_ADDTAB, -1, (LPARAM)si);
			SendMessage(g_TabSession.hWnd, GC_TABCHANGE, 0, (LPARAM)&g_TabSession);
		}

		pci->SetActiveSession(si->ptszID, si->pszModule);

		if (!IsWindowVisible(g_TabSession.hWnd) || wp == WINDOW_HIDDEN)
			SendMessage(g_TabSession.hWnd, GC_EVENT_CONTROL + WM_USER + 500, wp, 0);
		else {
			if (IsIconic(g_TabSession.hWnd))
				ShowWindow(g_TabSession.hWnd, SW_NORMAL);

			PostMessage(g_TabSession.hWnd, WM_SIZE, 0, 0);
			if (si->iType != GCW_SERVER)
				SendMessage(g_TabSession.hWnd, GC_UPDATENICKLIST, 0, 0);
			else
				SendMessage(g_TabSession.hWnd, GC_UPDATETITLE, 0, 0);
			SendMessage(g_TabSession.hWnd, GC_REDRAWLOG, 0, 0);
			SendMessage(g_TabSession.hWnd, GC_UPDATESTATUSBAR, 0, 0);
			ShowWindow(g_TabSession.hWnd, SW_SHOW);
			if (bSetForeground)
				SetForegroundWindow(g_TabSession.hWnd);
		}
		SendMessage(g_TabSession.hWnd, WM_MOUSEACTIVATE, 0, 0);
		SetFocus(GetDlgItem(g_TabSession.hWnd, IDC_MESSAGE));
		return;
	}

	// Do we need to create a window?
	if (si->hWnd == NULL)
		si->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), NULL, RoomWndProc, (LPARAM)si);

	SetWindowLongPtr(si->hWnd, GWL_EXSTYLE, GetWindowLongPtr(si->hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW);
	if (!IsWindowVisible(si->hWnd) || wp == WINDOW_HIDDEN)
		SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, wp, 0);
	else {
		if (IsIconic(si->hWnd))
			ShowWindow(si->hWnd, SW_NORMAL);
		ShowWindow(si->hWnd, SW_SHOW);
		SetForegroundWindow(si->hWnd);
	}

	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_MESSAGE));
}
