/*
Miranda Text Control - Plugin for Miranda IM

Copyright	© 2005 Victor Pavlychko (nullbie@gmail.com),
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

#include "stdafx.h"

HMODULE hMsfteditDll = nullptr;
HINSTANCE g_hInst = nullptr;

PCreateTextServices MyCreateTextServices = nullptr;

void MTextControl_RegisterClass();
void MTextControl_UnregisterClass();

void DestroyProxyWindow();

/////////////////////////////////////////////////////////////////////////////////////////

void UnloadEmfCache();

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD ul_reason_for_call, LPVOID)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		g_hInst = hInst;
		MyCreateTextServices = nullptr;
		hMsfteditDll = LoadLibrary(L"msftedit.dll");
		if (hMsfteditDll)
			MyCreateTextServices = (PCreateTextServices)GetProcAddress(hMsfteditDll, "CreateTextServices");

		LoadTextUsers();

		MTextControl_RegisterClass();
		break;

	case DLL_PROCESS_DETACH:
		DestroyProxyWindow();
		MTextControl_UnregisterClass();

		UnloadTextUsers();
		UnloadEmfCache();
		FreeLibrary(hMsfteditDll);
		break;
	}

	return TRUE;
}
