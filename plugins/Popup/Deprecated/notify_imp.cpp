/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
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

#include "headers.h"

static HANDLE hhkShow=0, hhkUpdate=0, hhkRemove=0;

//static void PopupThread(void *arg);
static INT_PTR Popup2Show(WPARAM wParam, LPARAM lParam);
static INT_PTR Popup2Update(WPARAM wParam, LPARAM lParam);
static INT_PTR Popup2Remove(WPARAM wParam, LPARAM lParam);
static INT_PTR svcPopup2DefaultActions(WPARAM wParam, LPARAM lParam);
static INT_PTR __stdcall PopupNotifyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void LoadNotifyImp()
{
	hhkShow = HookEvent(ME_NOTIFY_SHOW, (MIRANDAHOOK)Popup2Show);
	hhkUpdate = HookEvent(ME_NOTIFY_UPDATE, (MIRANDAHOOK)Popup2Update);
	hhkRemove = HookEvent(ME_NOTIFY_REMOVE, (MIRANDAHOOK)Popup2Remove);

	CreateServiceFunction("Popup2/DefaultActions", svcPopup2DefaultActions);

	CreateServiceFunction(MS_POPUP2_SHOW, Popup2Show);
	CreateServiceFunction(MS_POPUP2_UPDATE, Popup2Update);
	CreateServiceFunction(MS_POPUP2_REMOVE, Popup2Remove);
}

void UnloadNotifyImp()
{
	UnhookEvent(hhkShow);
	UnhookEvent(hhkUpdate);
	UnhookEvent(hhkRemove);
}

static INT_PTR svcPopup2DefaultActions(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = (HWND)MNotifyGetDWord((HANDLE)lParam, "Popup2/hwnd", (DWORD)NULL);
	if (!hwnd) return 0;

	HANDLE hNotify = (HANDLE)PUGetPluginData(hwnd);
	if (!hNotify || (hNotify == INVALID_HANDLE_VALUE)) return 0;

	switch (wParam)
	{
		case 0:
		{ // send message
			if (HANDLE hContact = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, 0))
				CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
			SendMessage(hwnd, UM_DESTROYPOPUP, 0, 0);
			break;
		}
		case 1:
		{ // dismiss popup
			SendMessage(hwnd, UM_DESTROYPOPUP, 0, 0);
			break;
		}
	}
	return 0;
}

static INT_PTR __stdcall PopupNotifyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hNotify = (HANDLE)PUGetPluginData(hwnd);

	if (!hNotify || (hNotify == INVALID_HANDLE_VALUE))
		return DefWindowProc(hwnd, msg, wParam, lParam);

	switch (msg)
	{
		case UM_INITPOPUP:
		{
			MNotifyAddRef(hNotify);
			MNotifySetDWord(hNotify, "Popup2/hwnd", (DWORD)hwnd);
			break;
		}
		case UM_FREEPLUGINDATA:
		{
			int result = 0;
			if (WNDPROC PluginWndProc = (WNDPROC)MNotifyGetDWord(hNotify, NFOPT_POPUP2_WNDPROC, NULL))
				result = PluginWndProc(hwnd, msg, wParam, lParam);
			result = DefWindowProc(hwnd, msg, wParam, lParam);

			MNotifySetDWord(hNotify, "Popup2/hwnd", (DWORD)NULL);
			MNotifyRelease(hNotify);
			
			return result;
		}

		case WM_LBUTTONDOWN:
		{
			const char *service = MNotifyGetString(hNotify, NFOPT_POPUP2_LCLICKSVC, "Popup2/DefaultActions");
			DWORD cookie = MNotifyGetDWord(hNotify, NFOPT_POPUP2_LCLICKCOOKIE, 0);
			CallService(service, cookie, (LPARAM)hNotify);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			const char *service = MNotifyGetString(hNotify, NFOPT_POPUP2_LCLICKSVC, "Popup2/DefaultActions");
			DWORD cookie = MNotifyGetDWord(hNotify, NFOPT_POPUP2_LCLICKCOOKIE, 1);
			CallService(service, cookie, (LPARAM)hNotify);
			break;
		}
	}

	if (WNDPROC PluginWndProc = (WNDPROC)MNotifyGetDWord(hNotify, NFOPT_POPUP2_WNDPROC, NULL))
		return PluginWndProc(hwnd, msg, wParam, lParam);

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static INT_PTR Popup2Show(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	HANDLE hNotify = (HANDLE)lParam;
	if (!hNotify) return -1;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = 0;
	ppd2.lchContact = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, NULL);;
	ppd2.lchEvent = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, NULL);
	ppd2.lchIcon = (HICON)MNotifyGetDWord(hNotify, NFOPT_ICON, (DWORD)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
	
	if (PopUpOptions.UseWinColors)
	{
		ppd2.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd2.colorText = GetSysColor(COLOR_WINDOWTEXT);
	} else
	{
		ppd2.colorBack = fonts.clBack;
		ppd2.colorText = fonts.clText;
	}
	ppd2.colorBack = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, MNotifyGetDWord(hNotify, NFOPT_BACKCOLOR, ppd2.colorBack));
	ppd2.colorText = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, MNotifyGetDWord(hNotify, NFOPT_TEXTCOLOR, ppd2.colorText));
	ppd2.PluginWindowProc = (WNDPROC)MNotifyGetDWord(hNotify, NFOPT_POPUP2_WNDPROC, NULL);
	ppd2.PluginData = (LPVOID)MNotifyGetDWord(hNotify, NFOPT_POPUP2_PLUGINDATA, NULL);
	ppd2.iSeconds = (int)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TIMEOUT, MNotifyGetDWord(hNotify, NFOPT_TIMEOUT, PopUpOptions.Seconds));

	WCHAR *lpwzTitle, *lpwzText;
	char *lpzTitle, *lpzText;
	lpwzTitle = (WCHAR *)MNotifyGetWString(hNotify, NFOPT_TITLEW, NULL);
	lpzTitle = (char *)MNotifyGetString(hNotify, NFOPT_TITLE, NULL);
	lpwzText = (WCHAR *)MNotifyGetWString(hNotify, NFOPT_TEXTW, NULL);
	lpzText = (char *)MNotifyGetString(hNotify, NFOPT_TEXT, NULL);
	if (lpwzTitle && lpwzText)
	{
		ppd2.lpwzTitle = lpwzTitle;
		ppd2.lpwzText = lpwzText;
		ppd2.flags |= PU2_UNICODE;
	} else
	if (lpzTitle && lpzText)
	{
		ppd2.lpzTitle = lpzTitle;
		ppd2.lpzText = lpzText;
		ppd2.flags |= PU2_ANSI;
	} else
	{
		ppd2.lpzText = ppd2.lpzTitle = Translate("<empty>");
		ppd2.flags |= PU2_ANSI;
	}

	DWORD dwStatusMode = MNotifyGetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, 0x3ff);

	DWORD dwFlags = APF_RETURN_HWND;
	switch (PopUpOptions.CurrentStatus)
	{
		case ID_STATUS_OFFLINE:		if (!(dwStatusMode&0x001)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_ONLINE:		if (!(dwStatusMode&0x002)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_AWAY:		if (!(dwStatusMode&0x004)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_DND:			if (!(dwStatusMode&0x008)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_NA:			if (!(dwStatusMode&0x010)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_OCCUPIED:	if (!(dwStatusMode&0x020)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_FREECHAT:	if (!(dwStatusMode&0x040)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_INVISIBLE:	if (!(dwStatusMode&0x080)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_ONTHEPHONE:	if (!(dwStatusMode&0x100)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_OUTTOLUNCH:	if (!(dwStatusMode&0x200)) dwFlags |= APF_NO_POPUP; break;
	}

	ppd2.PluginData = hNotify;
	ppd2.PluginWindowProc = (WNDPROC)PopupNotifyWndProc;

	return PopUp_AddPopUp2((WPARAM)&ppd2, dwFlags);
}

static INT_PTR Popup2Update(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	HANDLE hNotify = (HANDLE)lParam;
	if (!hNotify) return -1;

	HWND hwnd = (HWND)MNotifyGetDWord(hNotify, "Popup2/hwnd", 0);
	if (!hwnd) return -1;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = 0;
	ppd2.lchContact = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, NULL);;
	ppd2.lchEvent = (HANDLE)MNotifyGetDWord(hNotify, NFOPT_CONTACT, NULL);
	ppd2.lchIcon = (HICON)MNotifyGetDWord(hNotify, NFOPT_ICON, (DWORD)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
	
	if (PopUpOptions.UseWinColors)
	{
		ppd2.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd2.colorText = GetSysColor(COLOR_WINDOWTEXT);
	} else
	{
		ppd2.colorBack = fonts.clBack;
		ppd2.colorText = fonts.clText;
	}
	ppd2.colorBack = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_BACKCOLOR, MNotifyGetDWord(hNotify, NFOPT_BACKCOLOR, ppd2.colorBack));
	ppd2.colorText = (COLORREF)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TEXTCOLOR, MNotifyGetDWord(hNotify, NFOPT_TEXTCOLOR, ppd2.colorText));
	ppd2.PluginWindowProc = (WNDPROC)MNotifyGetDWord(hNotify, NFOPT_POPUP2_WNDPROC, NULL);
	ppd2.PluginData = (LPVOID)MNotifyGetDWord(hNotify, NFOPT_POPUP2_PLUGINDATA, NULL);
	ppd2.iSeconds = (int)MNotifyGetDWord(hNotify, NFOPT_POPUP2_TIMEOUT, MNotifyGetDWord(hNotify, NFOPT_TIMEOUT, PopUpOptions.Seconds));

	WCHAR *lpwzTitle, *lpwzText;
	char *lpzTitle, *lpzText;
	lpwzTitle = (WCHAR *)MNotifyGetWString(hNotify, NFOPT_TITLEW, NULL);
	lpzTitle = (char *)MNotifyGetString(hNotify, NFOPT_TITLE, NULL);
	lpwzText = (WCHAR *)MNotifyGetWString(hNotify, NFOPT_TEXTW, NULL);
	lpzText = (char *)MNotifyGetString(hNotify, NFOPT_TEXT, NULL);
	if (lpwzTitle && lpwzText)
	{
		ppd2.lpwzTitle = lpwzTitle;
		ppd2.lpwzText = lpwzText;
		ppd2.flags |= PU2_UNICODE;
	} else
	if (lpzTitle && lpzText)
	{
		ppd2.lpzTitle = lpzTitle;
		ppd2.lpzText = lpzText;
		ppd2.flags |= PU2_ANSI;
	} else
	{
		ppd2.lpzText = ppd2.lpzTitle = Translate("<empty>");
		ppd2.flags |= PU2_ANSI;
	}

	DWORD dwStatusMode = MNotifyGetDWord(hNotify, NFOPT_POPUP2_STATUSMODE, 0x3ff);

	DWORD dwFlags = 0;
	switch (PopUpOptions.CurrentStatus)
	{
		case ID_STATUS_OFFLINE:		if (!(dwStatusMode&0x001)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_ONLINE:		if (!(dwStatusMode&0x002)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_AWAY:		if (!(dwStatusMode&0x004)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_DND:			if (!(dwStatusMode&0x008)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_NA:			if (!(dwStatusMode&0x010)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_OCCUPIED:	if (!(dwStatusMode&0x020)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_FREECHAT:	if (!(dwStatusMode&0x040)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_INVISIBLE:	if (!(dwStatusMode&0x080)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_ONTHEPHONE:	if (!(dwStatusMode&0x100)) dwFlags |= APF_NO_POPUP; break;
		case ID_STATUS_OUTTOLUNCH:	if (!(dwStatusMode&0x200)) dwFlags |= APF_NO_POPUP; break;
	}

	ppd2.PluginData = hNotify;
	ppd2.PluginWindowProc = (WNDPROC)PopupNotifyWndProc;

	return PopUp_Change2((WPARAM)hwnd, (LPARAM)&ppd2);
}

static INT_PTR Popup2Remove(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	HANDLE hNotify = (HANDLE)lParam;
	if (!hNotify) return -1;

	HWND hwnd = (HWND)MNotifyGetDWord(hNotify, "Popup2/hwnd", 0);
	if (!hwnd) return -1;

	SendMessage(hwnd, UM_DESTROYPOPUP, 0, 0);
	return 0;
}
