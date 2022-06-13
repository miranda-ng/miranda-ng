/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"

static bool bModuleInit = false, bPopupsEnabled = true;

MIR_APP_DLL(bool) Popup_Enabled()
{
	if (!bModuleInit) {
		bModuleInit = true;
		bPopupsEnabled = db_get_b(0, "Popup", "ModuleIsEnabled", 1) != 0;
	}

	return bPopupsEnabled;
}

MIR_APP_DLL(void) Popup_Enable(bool bEnable)
{
	bPopupsEnabled = bEnable;
	db_set_b(0, "Popup", "ModuleIsEnabled", bEnable);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup v2.0

MIR_APP_DLL(HWND) Popup_Add(const POPUPDATA2 *ppdp, int flags)
{
	return (HWND)CallService(MS_POPUP_ADDPOPUP2, (WPARAM)ppdp, flags);
}

MIR_APP_DLL(void) Popup_Change(HWND hwndPopup, const POPUPDATA2 *pData)
{
	CallService(MS_POPUP_CHANGEPOPUP2, (WPARAM)hwndPopup, (LPARAM)pData);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popups v1.0

MIR_APP_DLL(HWND) PUAddPopup(POPUPDATA *ppdp, int flags)
{
	return (HWND)CallService(MS_POPUP_ADDPOPUP, (WPARAM)ppdp, flags);
}

MIR_APP_DLL(HWND) PUAddPopupW(POPUPDATAW *ppdp, int flags)
{
	return (HWND)CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp, flags);
}

MIR_APP_DLL(int) PUChangeW(HWND hWndPopup, POPUPDATAW *newData)
{
	return (int)CallService(MS_POPUP_CHANGEW, (WPARAM)hWndPopup, (LPARAM)newData);
}

MIR_APP_DLL(int) PUChangeTextW(HWND hWndPopup, const wchar_t *lpwzNewText)
{
	return (int)CallService(MS_POPUP_CHANGETEXTW, (WPARAM)hWndPopup, (LPARAM)lpwzNewText);
}

MIR_APP_DLL(int) PUDeletePopup(HWND hWndPopup)
{
	return (int)CallService(MS_POPUP_DESTROYPOPUP, 0, (LPARAM)hWndPopup);
}

MIR_APP_DLL(MCONTACT) PUGetContact(HWND hPopupWindow)
{
	return (MCONTACT)CallService(MS_POPUP_GETCONTACT, (WPARAM)hPopupWindow, 0);
}

MIR_APP_DLL(void*) PUGetPluginData(HWND hPopupWindow)
{
	return (void*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hPopupWindow, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Messages

MIR_APP_DLL(int) PUShowMessage(const char *lpzText, uint32_t kind)
{
	return (int)CallService(MS_POPUP_SHOWMESSAGE, (WPARAM)lpzText, (LPARAM)kind);
}

MIR_APP_DLL(int) PUShowMessageW(const wchar_t *lpwzText, uint32_t kind)
{
	return (int)CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)lpwzText, (LPARAM)kind);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Actions

MIR_APP_DLL(int) PURegisterActions(POPUPACTION *actions, int count)
{
	return (int)CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)actions, (LPARAM)count);
}

MIR_APP_DLL(HANDLE) PURegisterNotification(POPUPNOTIFICATION *notification)
{
	return (HANDLE)CallService(MS_POPUP_REGISTERNOTIFICATION, (WPARAM)notification, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup classes

MIR_APP_DLL(HANDLE) Popup_RegisterClass(POPUPCLASS *pc)
{
	return (HANDLE)CallService(MS_POPUP_REGISTERCLASS, 0, LPARAM(pc));
}

MIR_APP_DLL(void) Popup_UnregisterClass(HANDLE ppc)
{
	if (ppc)
		CallService(MS_POPUP_UNREGISTERCLASS, 0, LPARAM(ppc));
}

MIR_APP_DLL(HWND) Popup_AddClass(POPUPDATACLASS *pData)
{
	return (HWND)CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)pData);
}

MIR_APP_DLL(HWND) ShowClassPopup(const char *name, const char *title, const char *text)
{
	POPUPDATACLASS d = {};
	d.pszClassName = name;
	d.szTitle.a = title;
	d.szText.a = text;
	return (HWND)CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
}

MIR_APP_DLL(HWND) ShowClassPopupW(const char *name, const wchar_t *title, const wchar_t *text)
{
	POPUPDATACLASS d = {};
	d.pszClassName = name;
	d.szTitle.w = title;
	d.szText.w = text;
	return (HWND)CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
}
