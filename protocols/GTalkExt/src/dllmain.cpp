//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "stdafx.h"
#include "notifications.h"
#include "options.h"
#include "popups.h"

HINSTANCE hInst = 0;

DWORD itlsSettings = TLS_OUT_OF_INDEXES;
DWORD itlsRecursion = TLS_OUT_OF_INDEXES;
DWORD itlsPopupHook = TLS_OUT_OF_INDEXES;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;

	switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            if (((itlsSettings = TlsAlloc()) == TLS_OUT_OF_INDEXES) ||
				((itlsRecursion = TlsAlloc()) == TLS_OUT_OF_INDEXES) ||
				((itlsPopupHook = TlsAlloc()) == TLS_OUT_OF_INDEXES))
                return FALSE;
			break;

		case DLL_THREAD_ATTACH:
			TlsSetValue(itlsPopupHook,
				(PVOID)SetWindowsHookEx(WH_CALLWNDPROCRET, PopupHookProc, NULL, GetCurrentThreadId()));
			break;

		case DLL_THREAD_DETACH:
			UnhookWindowsHookEx((HHOOK)TlsGetValue(itlsPopupHook));
			break;

        case DLL_PROCESS_DETACH:
            TlsFree(itlsSettings);
			TlsFree(itlsRecursion);
			TlsFree(itlsPopupHook);
            break;
    }

	return TRUE;
}
