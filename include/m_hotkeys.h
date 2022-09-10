/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_HOTKEYS_H__
#define M_HOTKEYS_H__ 1

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#define HKD_UNICODE         0x0001
#define HKF_MIRANDA_LOCAL   0x8000

struct HOTKEYDESC
{
	const char *pszName;          // name to refer to hotkey when playing and in db
	MAllCStrings szDescription;   // description for options dialog
	MAllCStrings szSection;       // section name used to group sounds (NULL is acceptable)
	const char *pszService;       // Service to call when HotKey Pressed
	uint16_t DefHotKey;           // default hot key for action
	uint32_t dwFlags;             // one of HKD_* constants
	LPARAM lParam;                // lParam to pass to service
};

/////////////////////////////////////////////////////////////////////////////////////////
// use this macro to defile hotkey codes like this:
// hkd.DefHotkey = HOTKEYCODE(HOTKEYF_SHIFT|HOTKEYF_EXT, 'A');

#define HOTKEYCODE(mod, vk) (MAKEWORD((vk), (mod)))

/////////////////////////////////////////////////////////////////////////////////////////
// Registers new hotkey
// Returns 0 on failure or hotkey atom id on success

EXTERN_C MIR_APP_DLL(int) Hotkey_Register(const HOTKEYDESC *hk, HPLUGIN pPlugin);

/////////////////////////////////////////////////////////////////////////////////////////
// Unregister existing hotkey

EXTERN_C MIR_APP_DLL(int) Hotkey_Unregister(const char *pszName);

/////////////////////////////////////////////////////////////////////////////////////////
// Checks if "manual" hotkey was activated and returns its id.
// Returns lParam associated with activated hotkey

EXTERN_C MIR_APP_DLL(int) Hotkey_Check(MSG *pEvent, const char *pszSection);

/////////////////////////////////////////////////////////////////////////////////////////
// Subclasss/unsubclass edit box to act as hotkey control
// You will get notification with LOWORD(wParam) == 0 when users sets hotkey.
// 
// Subclassed control processes HKM_SETHOTKEY and HKM_GETHOTKEY similarly to
// windows' hotkey, in all other it acts just like normal editbox (beware of
// standart notifications that occur on text updates!)
// 
// Subclass procedure uses GWLP_USERDATA to store internal information. Do not
// use it for own purposes.

EXTERN_C MIR_APP_DLL(void) Hotkey_Subclass(HWND hwndEdit);
EXTERN_C MIR_APP_DLL(void) Hotkey_Unsubclass(HWND hwndEdit);

/////////////////////////////////////////////////////////////////////////////////////////
// This event is fired when hotkeys were changed
// wParam = lParam = 0

#define ME_HOTKEYS_CHANGED "CoreHotkeys/Changed"

#endif // M_HOTKEYS_H__
