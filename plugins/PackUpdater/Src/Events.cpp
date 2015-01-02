/*
Copyright (C) 2011-2015 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

HANDLE Timer;
BOOL Silent;

int ModulesLoaded(WPARAM, LPARAM)
{
	Silent = true;
	HOTKEYDESC hkd = { 0 };
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "Check for pack updates";
	hkd.ptszDescription = LPGENT("Check for pack updates");
	hkd.ptszSection = LPGENT("Pack Updater");
	hkd.pszService = MODNAME"/CheckUpdates";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F10) | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;
	Hotkey_Register(&hkd);

	if (AllowUpdateOnStartup())
		DoCheck(UpdateOnStartup);

	Timer = CreateWaitableTimer(NULL, FALSE, NULL);
	InitTimer();

	return 0;
}

INT_PTR MenuCommand(WPARAM, LPARAM)
{
	Silent = false;
	DoCheck(TRUE);
	return 0;
}

INT_PTR EmptyFolder(WPARAM, LPARAM lParam)
{
	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_DELETE,
		tszRoot,
		_T(""),
		FOF_NOERRORUI |
		FOF_SILENT,
		false,
		0,
		_T("") };
	if (lParam)
		file_op.fFlags |= FOF_NOCONFIRMATION;
	SHFileOperation(&file_op);
	return 0;
}

INT OnPreShutdown(WPARAM, LPARAM)
{
	CancelWaitableTimer(Timer);
	CloseHandle(Timer);
	return 0;
}
