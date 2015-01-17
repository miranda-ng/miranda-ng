/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

#ifndef M_EXCHANGE_HOOKED_EVENTS_H
#define M_EXCHANGE_HOOKED_EVENTS_H

extern HANDLE hModulesLoaded;
extern HANDLE hOptionsInitialise;
extern HANDLE hPreShutdown;

extern UINT_PTR hCheckTimer;
extern UINT_PTR hReconnectTimer;

int HookEvents();
int UnhookEvents();

void FirstTimeCheck();

int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int OnOptionsInitialise(WPARAM wParam, LPARAM lParam);
int OnSystemPreShutdown(WPARAM wParam, LPARAM lParam);

int UpdateTimers();
int KillTimers();
VOID CALLBACK OnCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnReconnectTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnFirstCheckTimer(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);

#endif