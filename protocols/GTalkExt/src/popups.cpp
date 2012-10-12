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
#include "popups.h"
#include "options.h"

static const LPTSTR YAPP_WND_CLASS1 = _T("YAPPYAPPWindowClass");
static const LPTSTR YAPP_WND_CLASS2 = _T("YAPPWinClass");

extern BOOL isOriginalPopups;

LRESULT CALLBACK PopupHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PCWPRETSTRUCT cs = (PCWPRETSTRUCT)lParam;
	if ((HC_ACTION == nCode) &&
		(WM_CREATE == cs->message) &&
		(-1 != cs->lResult) &&
		ServiceExists(MS_POPUP_GETCONTACT)) {

			// with YAPP we can't call MS_POPUP_GETCONTACT on a random window
			TCHAR ClassName[32];
			GetClassName(cs->hwnd, ClassName, sizeof(ClassName) / sizeof(TCHAR));
			if (isOriginalPopups ||
				!lstrcmp(YAPP_WND_CLASS1, ClassName) ||
				!lstrcmp(YAPP_WND_CLASS2, ClassName)) {

					HANDLE hContact = (HANDLE)CallService(MS_POPUP_GETCONTACT, (WPARAM)cs->hwnd, 0);
					if (hContact != (HANDLE)-1 &&
						hContact != (HANDLE)0x80000000 &&
						DBGetContactSettingByte(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
							LPCSTR proto = (LPCSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
							DWORD checkboxes = ReadCheckboxes(0, proto);
							if (ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, checkboxes) &&
								ReadCheckbox(0, IDC_SUPRESSFOREIGN, checkboxes))
									PostMessage(cs->hwnd, WM_CLOSE, 0, 0);
					}
			}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

typedef PLUGININFOEX* (MIRANDAPLUGININFOEX) (DWORD mirandaVersion);

static GUID POPUP_GUID1 = {0x26a9125d, 0x7863, 0x4e01, {0xaf, 0xe, 0xd1, 0x4e, 0xf9, 0x5c, 0x50, 0x54}};
static GUID POPUP_GUID2 = {0x26a9125d, 0x7863, 0x4e01, {0xaf, 0xe, 0xd1, 0x4e, 0xf9, 0x5c, 0x50, 0x53}};

DWORD g_mirandaVersion = 0;

BOOL IsOriginalPopupModule(HMODULE hMod)
{
	MIRANDAPLUGININFOEX *MirandaPluginInfoEx = (MIRANDAPLUGININFOEX*)GetProcAddress(hMod, "MirandaPluginInfoEx");
	if (!MirandaPluginInfoEx) return FALSE;

	PLUGININFOEX *PluginInfoEx = MirandaPluginInfoEx(g_mirandaVersion);
	if (!PluginInfoEx) return FALSE;

	GUID *guid1 = (GUID*)&PluginInfoEx->uuid;
	GUID *guid2 = (GUID*)&POPUP_GUID1;
	GUID *guid3 = (GUID*)&POPUP_GUID2;
	return (IsEqualGUID(*guid1, *guid2) || IsEqualGUID(*guid1, *guid3));
}

extern BOOL isOriginalPopups;

void DetectPopupModule()
{
	DWORD bytesNeeded;
	if (!EnumProcessModules(GetCurrentProcess(), NULL, 0, &bytesNeeded))
		return;

	HMODULE *mods = (HMODULE*)malloc(bytesNeeded);
	__try {
		if (!EnumProcessModules(GetCurrentProcess(), mods, bytesNeeded, &bytesNeeded))
			return;

		for (DWORD i = 0; i < (bytesNeeded / sizeof(HMODULE)); i++)
			if (IsOriginalPopupModule(mods[i])) {
				isOriginalPopups = TRUE;
				break;
			}
	}
	__finally {
		free(mods);
	}
}
