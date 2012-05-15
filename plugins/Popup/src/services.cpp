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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/services.cpp $
Revision       : $Revision: 1651 $
Last change on : $Date: 2010-07-15 20:31:06 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

int num_classes = 0;			//for core class api support

// isWorkstationLocked() code from core
bool isWorkstationLocked()
{
	bool rc = false;

	if (OpenInputDesktop != NULL) {
		HDESK hDesk = OpenInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);
		if (hDesk == NULL)
			rc = true;
		else if (CloseDesktop != NULL)
			CloseDesktop(hDesk);
	}
	return rc;
}

// isFullScreen() code from core
static bool isFullScreen()
{
	RECT rcScreen = {0};

	rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
	rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

	if (MonitorFromWindow != NULL)
	{
		HMONITOR hMon = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(hMon, &mi))
			rcScreen = mi.rcMonitor;
	}

	HWND hWndDesktop = GetDesktopWindow();
	HWND hWndShell = GetShellWindow();

	// check foregroundwindow
	HWND hWnd = GetForegroundWindow();
	if (hWnd && hWnd != hWndDesktop && hWnd != hWndShell) 
	{
		TCHAR tszClassName[128] = _T("");
		GetClassName(hWnd, tszClassName, SIZEOF(tszClassName));
		if (_tcscmp(tszClassName, _T("WorkerW")))
		{
			RECT rect, rectw, recti;
			GetWindowRect(hWnd, &rectw);

			GetClientRect(hWnd, &rect);
			ClientToScreen(hWnd, (LPPOINT)&rect);
			ClientToScreen(hWnd, (LPPOINT)&rect.right);
			
			if (EqualRect(&rect, &rectw) && IntersectRect(&recti, &rect, &rcScreen) && 
				EqualRect(&recti, &rcScreen))
				return true;
		}
	}

	return false;
}

//===== PopUp/AddPopUp
INT_PTR PopUp_AddPopUp(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	POPUPDATA *ppd = (POPUPDATA*)wParam;
	if (!ppd) return -1;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_ANSI;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.lpzTitle = ppd->lpzContactName;
	ppd2.lpzText = ppd->lpzText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = PopUpOptions.Seconds;
	return PopUp_AddPopUp2((WPARAM)&ppd2, lParam);
/*
	PopupWnd2 *wnd = new PopupWnd2(ppd);

	if (lParam & APF_RETURN_HWND)
	{
		while (!wnd->bWindowCreated) Sleep(1);
		return (int)wnd->getHwnd();
	}

	return 1;
*/
}

//===== PopUp/AddPopUpEx
INT_PTR PopUp_AddPopUpEx(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	POPUPDATAEX_V2 *ppd = (POPUPDATAEX_V2*)wParam;
	if (!ppd) return -1;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_ANSI;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.lpzTitle = ppd->lpzContactName;
	ppd2.lpzText = ppd->lpzText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;

	if (lParam&APF_NEWDATA)
	{
		ppd2.lchNotification = ppd->hNotification;
		ppd2.actionCount = ppd->actionCount;
		ppd2.lpActions = ppd->lpActions;
//		ppd2.hbmAvatar = ppd->hbmAvatar;
	}

	return PopUp_AddPopUp2((WPARAM)&ppd2, lParam);

/*
	if (lParam & APF_RETURN_HWND)
	{
		while (!wnd->bWindowCreated) Sleep(1);
		return (int)wnd->getHwnd();
	}

	return 1;
*/
}

//===== PopUp/AddPopupW
INT_PTR PopUp_AddPopUpW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	POPUPDATAW_V2 *ppd = (POPUPDATAW_V2*)wParam;
	if (!ppd) return -1;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_UNICODE;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.lpwzTitle = ppd->lpwzContactName;
	ppd2.lpwzText = ppd->lpwzText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;
	ppd2.lchNotification = ppd->hNotification;

	if (lParam&APF_NEWDATA)
	{
		ppd2.actionCount = ppd->actionCount;
		ppd2.lpActions = ppd->lpActions;
//		ppd2.hbmAvatar = ppd->hbmAvatar;
	}

	return PopUp_AddPopUp2((WPARAM)&ppd2, lParam);

/*
	if (lParam & APF_RETURN_HWND)
	{
		while (!wnd->bWindowCreated) Sleep(1);
		return (int)wnd->getHwnd();
	}

	return 1;
*/
}

//===== PopUp/AddPopup2
static __forceinline DWORD Proto_Status2Flag_My(DWORD status)
{
	if (DWORD res = Proto_Status2Flag(status))
		return res;
	return PF2_IDLE;
}

INT_PTR PopUp_AddPopUp2(WPARAM wParam, LPARAM lParam)
{
	/* NOTE: we will return 0 instead of -1 since tabSRMM stops using popup after first failure :/ */

	if (!gbPopupLoaded) return -1;

	POPUPDATA2 *ppdIn = (POPUPDATA2 *)wParam;
	if (!ppdIn) return -1;

	POPUPDATA2 ppdFixed = {0};
	POPUPDATA2 *ppd = &ppdFixed;
	CopyMemory(ppd, ppdIn, min(ppdIn->cbSize, sizeof(POPUPDATA2)));

	DWORD disableWhen;
	FillNotificationData(ppd, &disableWhen);

	if (!(lParam&APF_NO_HISTORY))
		PopupHistoryAdd(ppd);

	if (PopupThreadIsFull())
		return -1;

		#ifdef _DEBUG
		char temp[128];
			OutputDebugStringA("isWorkstationLocked: \t");
			OutputDebugStringA(isWorkstationLocked() ? "true":"false");
			OutputDebugStringA("\n");
		#endif

	if (isWorkstationLocked())
		return -1;

	// Check if contact handle is valid.
	char *proto = NULL;
	if (ppd->lchContact)
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ppd->lchContact, 0);

	BYTE bShowMode = proto ? DBGetContactSettingByte(ppd->lchContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO) : PU_SHOWMODE_AUTO;

	if (bShowMode == PU_SHOWMODE_BLOCK)
		return -1;

	if (bShowMode != PU_SHOWMODE_FAVORITE)
	{
		if (!PopUpOptions.ModuleIsEnabled)
			return -1;
		#ifdef _DEBUG
			itoa(PopUpOptions.DisableWhenFullscreen,temp,10);
			OutputDebugStringA("PopUpOptions.DisableWhenFullscreen: \t");
			OutputDebugStringA(temp);
			OutputDebugStringA("\n");
			itoa(bShowMode,temp,10);
			OutputDebugStringA("bShowMode: \t");
			OutputDebugStringA(temp);
			OutputDebugStringA("\n");
			OutputDebugStringA("isFullScreen: \t");
			OutputDebugStringA(isFullScreen() ? "true":"false");
			OutputDebugStringA("\n");
		#endif

		if (PopUpOptions.DisableWhenFullscreen && (bShowMode != PU_SHOWMODE_FULLSCREEN) && isFullScreen())
			return -1;

		if (DBGetContactSettingDword(NULL, MODULNAME, "Global Status", 0) &
				Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if ((disableWhen & 0x0000FFFF) & Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if (proto)
		{
			char prefix[128];
			mir_snprintf(prefix, sizeof(prefix), "Protocol Status/%s", (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ppd->lchContact, 0));
			if (DBGetContactSettingDword(NULL, MODULNAME, prefix, 0) &
					Proto_Status2Flag_My(CallProtoService(proto, PS_GETSTATUS, 0, 0)))
				return -1;
			if (((disableWhen >> 16) & 0xFFFF0000) & Proto_Status2Flag_My(CallProtoService(proto, PS_GETSTATUS, 0, 0)))
				return -1;
		}
	}

	if (lParam&APF_CUSTOM_POPUP)
		ppd->flags |= PU2_CUSTOM_POPUP;
	PopupWnd2 *wnd = new PopupWnd2(ppd, NULL, false);

	if (lParam & APF_RETURN_HWND)
	{
		while (!wnd->m_bWindowCreated) Sleep(1);
		return (INT_PTR)wnd->getHwnd();
	}

	return 0;
}

//===== PopUp/GetContact
INT_PTR PopUp_GetContact(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	HWND hwnd = (HWND)wParam;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd)) return (INT_PTR)wnd->getContactPassed();
	return (INT_PTR)(-1);
}

//===== PopUp/GetPluginData
INT_PTR PopUp_GetPluginData(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	HWND hwnd = (HWND)wParam;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd)) return (INT_PTR)wnd->getData();
	return (INT_PTR)(-1);
}

//===== PopUp/IsSecondLineShown
INT_PTR PopUp_IsSecondLineShown(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

//===== PopUp/ChangeText
INT_PTR PopUp_ChangeText(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateText, lParam);
/*
	char *str = (char *)mir_alloc(sizeof(char) * (strlen((char *)lParam) + 1));
	strcpy(str, (char *)lParam);
	PostMessage((HWND)wParam, UM_CHANGEPOPUP, CPT_TEXT, (LPARAM)str);
*/
	return 0;
}

//===== PopUp/ChangeTextW
INT_PTR PopUp_ChangeTextW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateTextW, lParam);
/*
	WCHAR *str = (WCHAR *)mir_alloc(sizeof(WCHAR) * (wcslen((WCHAR *)lParam) + 1));
	wcscpy(str, (WCHAR *)lParam);
	PostMessage((HWND)wParam, UM_CHANGEPOPUP, CPT_TEXTW, (LPARAM)str);
*/
	return 0;
}

//===== PopUp/Change
INT_PTR PopUp_Change(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATAEX_V2, lParam);
/*
	POPUPDATAEX_V2 *ppdx = (POPUPDATAEX_V2 *)mir_alloc(sizeof(POPUPDATAEX_V2));
	memcpy(ppdx, (POPUPDATAEX_V2 *)lParam, sizeof(POPUPDATAEX_V2));
	PostMessage((HWND)wParam, UM_CHANGEPOPUP, CPT_DATA, (LPARAM)ppdx);
*/
	return 0;
}

//===== PopUp/ChangeW
INT_PTR PopUp_ChangeW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATAW_V2, lParam);
/*
	POPUPDATAW_V2 *ppdw = (POPUPDATAW_V2 *)mir_alloc(sizeof(POPUPDATAW_V2));
	memcpy(ppdw, (POPUPDATAW_V2 *)lParam, sizeof(POPUPDATAW_V2));
	PostMessage((HWND)wParam, UM_CHANGEPOPUP, CPT_DATA, (LPARAM)ppdw);
*/
	return 0;
}

//===== PopUp/Change2
INT_PTR PopUp_Change2(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATA2, lParam);
	return 0;
}

//===== PopUp/ShowMessage
INT_PTR PopUp_ShowMessage(WPARAM wParam, LPARAM lParam) {
	if(!gbPopupLoaded || !wParam || !lParam) return -1;
	if(closing) return 0;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize		= sizeof(ppd2);
	ppd2.flags		= PU2_ANSI;
	ppd2.lpzText	= (char*)wParam;
	switch (lParam&0x7fffffff) {
		case SM_ERROR:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_ERROR,0);
			ppd2.colorBack			= RGB(191,0,0);
			ppd2.colorText			= RGB(255,245,225);
			ppd2.lchNotification	= g_hntfError;
			ppd2.lpzTitle			= Translate("Error");
			break;
		case SM_WARNING:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_WARNING,0);
			ppd2.colorBack			= RGB(210,210,150);
			ppd2.colorText			= RGB(0,0,0);
			ppd2.lchNotification	= g_hntfWarning;
			ppd2.lpzTitle			= Translate("Warning");
			break;
		case SM_NOTIFY:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_NOTIFY,0);
			ppd2.colorBack			= RGB(230,230,230);
			ppd2.colorText			= RGB(0,0,0);
			ppd2.lchNotification	= g_hntfNotification;
			ppd2.lpzTitle			= Translate("Notify");
			break;
		default: //No no no... you must give me a good value.
			return -1;
	}
	return PopUp_AddPopUp2((WPARAM)&ppd2, (LPARAM)((lParam&0x80000000)?APF_NO_HISTORY:0));
}

INT_PTR PopUp_ShowMessageW(WPARAM wParam, LPARAM lParam) {
	if(!gbPopupLoaded || !wParam || !lParam) return -1;
	if(closing) return 0;

	POPUPDATA2 ppd2 = {0};
	ppd2.cbSize		= sizeof(ppd2);
	ppd2.flags		= PU2_UNICODE;
	ppd2.lpwzText	= (WCHAR*)wParam;
	switch (lParam&0x7fffffff) {
		case SM_ERROR:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_ERROR,0);
			ppd2.colorBack			= RGB(191,0,0);
			ppd2.colorText			= RGB(255,245,225);
			ppd2.lchNotification	= g_hntfError;
			ppd2.lpwzTitle			= TranslateW(L"Error");
			break;
		case SM_WARNING:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_WARNING,0);
			ppd2.colorBack			= RGB(210,210,150);
			ppd2.colorText			= RGB(0,0,0);
			ppd2.lchNotification	= g_hntfWarning;
			ppd2.lpwzTitle			= TranslateW(L"Warning");
			break;
		case SM_NOTIFY:
			ppd2.lchIcon			= IcoLib_GetIcon(ICO_MISC_NOTIFY,0);
			ppd2.colorBack			= RGB(230,230,230);
			ppd2.colorText			= RGB(0,0,0);
			ppd2.lchNotification	= g_hntfNotification;
			ppd2.lpwzTitle			= TranslateW(L"Notify");
			break;
		default: //No no no... you must give me a good value.
			return -1;
	}
	return PopUp_AddPopUp2((WPARAM)&ppd2, (LPARAM)((lParam&0x80000000)?APF_NO_HISTORY:0));
}

//===== PopUp/Query
INT_PTR PopUp_Query(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (closing)
		return 0;

	switch (wParam) {
		case PUQS_ENABLEPOPUPS: {
			if (PopUpOptions.ModuleIsEnabled) return 1; //They're already ON!!!
			else { //Module was disabled.
				svcEnableDisableMenuCommand(0,0);
				return 0;
			}
		}
		case PUQS_DISABLEPOPUPS: {
			if (!(PopUpOptions.ModuleIsEnabled)) return 1; //They're already OFF!!!
			else {
				svcEnableDisableMenuCommand(0,0);
				return 0;
			}
		}
		case PUQS_GETSTATUS:
			return (PopUpOptions.ModuleIsEnabled);
		default:
			return -1;
	}
	return 0;
}


//===== PopUp/RegisterActions
INT_PTR PopUp_RegisterActions(WPARAM wParam, LPARAM lParam)
{
	LPPOPUPACTION actions = (LPPOPUPACTION)wParam;
	for (int i = 0; i < lParam; ++i)
		RegisterAction(&actions[i]);
	return 0;
}


INT_PTR PopUp_RegisterNotification(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)RegisterNotification((LPPOPUPNOTIFICATION)wParam);
}


//===== PopUp/UnhookEventAsync
struct SafeUnhookEventParam
{
	HWND hwndPopup;
	HANDLE hEvent;
};

static void CALLBACK SafeUnhookEventFunc(ULONG_PTR dwParam)
{
	UnhookEvent(((SafeUnhookEventParam *)dwParam)->hEvent);
	PostMessage(((SafeUnhookEventParam *)dwParam)->hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0,
			(LPARAM)((SafeUnhookEventParam *)dwParam)->hEvent);
	delete (SafeUnhookEventParam *)dwParam;
}

INT_PTR PopUp_UnhookEventAsync(WPARAM wParam, LPARAM lParam)
{
	SafeUnhookEventParam *param = new SafeUnhookEventParam;
	param->hwndPopup = (HWND)wParam;
	param->hEvent = (HANDLE)lParam;
	QueueUserAPC(SafeUnhookEventFunc, hMainThread, (ULONG_PTR)param);
	return 0;
}

//===== PopUp/RegisterVfx (effekt name for drop down box)
INT_PTR PopUp_RegisterVfx(WPARAM wParam, LPARAM lParam)
{
	OptAdv_RegisterVfx((char *)lParam);
	return 0;
}

//===== PopUp/RegisterClass		(for core class api support)
INT_PTR PopUp_RegisterPopupClass(WPARAM wParam, LPARAM lParam) {
	char setting[256];
	POPUPCLASS    *pc	= (POPUPCLASS *)lParam;
	POPUPTREEDATA *ptd	= (POPUPTREEDATA *)mir_alloc(sizeof(POPUPTREEDATA));
	memset(ptd,0,sizeof(POPUPTREEDATA));
	ptd->cbSize		= sizeof(POPUPTREEDATA);
	ptd->signature	= 0/*PopupNotificationData_SIGNATURE*/;
	ptd->typ		= 2;
	memcpy(&ptd->pupClass, pc, sizeof(POPUPCLASS));
	ptd->pszTreeRoot					= mir_a2t(pc->pszName);
	ptd->pupClass.pszName				= mir_strdup(pc->pszName);
	if(pc->flags & PCF_UNICODE) {
		ptd->pupClass.pwszDescription	= mir_wstrdup(pc->pwszDescription);
		ptd->pszDescription				= mir_u2t(pc->pwszDescription);
	}
	else {
		ptd->pupClass.pszDescription	= mir_strdup (pc->pszDescription);
		ptd->pszDescription				= mir_a2t(pc->pszDescription);
	}
	LoadClassSettings(ptd, PU_MODULCLASS);

	//we ignore pc->colorText and use fonts.text as default (if no setting found in DB)
	mir_snprintf(setting, 256, "%s/TextCol", ptd->pupClass.pszName);
	ptd->pupClass.colorText = (COLORREF)DBGetContactSettingDword(NULL, PU_MODULCLASS, setting, fonts.clText/*pc->colorText*/);
	FontID fid = {0};
	fid.cbSize = sizeof(FontID);
	mir_snprintf(fid.group, sizeof(fid.group), "%s/%s", PU_FNT_AND_COLOR, ptd->pupClass.pszName);
	strcpy(fid.dbSettingsGroup, PU_MODULCLASS);
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.size = -11;
	strcpy(fid.deffontsettings.szFace, "Verdana");
	strcpy(fid.name, PU_FNT_NAME_TEXT);
	strcpy(fid.prefix, setting);
	mir_snprintf(fid.prefix, sizeof(fid.prefix), "%s/Text", ptd->pupClass.pszName);  // result is "%s/TextCol"
	fid.deffontsettings.style  = 0;
	fid.deffontsettings.colour = fonts.clText;
	CallService(MS_FONT_REGISTER, (WPARAM)&fid, 0);

	//we ignore pc->colorBack and use fonts.clBack as default (if no setting found in DB)
	mir_snprintf(setting, 256, "%s/BgCol", ptd->pupClass.pszName);
	ptd->pupClass.colorBack				= (COLORREF)DBGetContactSettingDword(NULL, PU_MODULCLASS, setting, (DWORD)fonts.clBack/*pc->colorBack*/);
	ColourID cid = {0};
	cid.cbSize = sizeof(ColourID);
	mir_snprintf(cid.group, sizeof(cid.group), "%s/%s", PU_FNT_AND_COLOR, ptd->pupClass.pszName);
	strcpy(cid.dbSettingsGroup, PU_MODULCLASS);
	strcpy(cid.name, PU_COL_BACK_NAME);
	mir_snprintf(cid.setting, sizeof(cid.setting), "%s/BgCol", ptd->pupClass.pszName);
	cid.defcolour = fonts.clBack;
	CallService(MS_COLOUR_REGISTER, (WPARAM)&cid, 0);

	gTreeData.insert(ptd);
	num_classes++;

	return 0;
}

//===== PopUp/AddPopupClass		(for core class api support)
INT_PTR PopUp_CreateClassPopup(WPARAM wParam, LPARAM lParam) {
	int ret = 1;
	POPUPDATACLASS *pdc = (POPUPDATACLASS *)lParam;
	if(pdc->cbSize != sizeof(POPUPDATACLASS)) return ret;

	POPUPCLASS		*pc  = NULL;
	POPUPTREEDATA	*ptd = NULL;

	if(wParam) pc = (POPUPCLASS *)wParam;
	else {
		LPTSTR group = mir_a2t(pdc->pszClassName);
		ptd = (POPUPTREEDATA *)FindTreeData(group, NULL, 2);
		if(ptd) pc = &ptd->pupClass;
	}
	if(pc) {
		POPUPDATA2 ppd2 = {0};
		ppd2.cbSize				= sizeof(POPUPDATA2);
		ppd2.colorBack			= pc->colorBack;
		ppd2.colorText			= pc->colorText;
		ppd2.lchIcon			= pc->hIcon;
		ppd2.iSeconds			= pc->iSeconds;
		ppd2.PluginWindowProc	= pc->PluginWindowProc;
		if(pc->flags & PCF_UNICODE) {
			ppd2.flags			= PU2_UNICODE;
			ppd2.lpwzTitle		= (WCHAR*)pdc->pwszTitle;
			ppd2.lpwzText		= (WCHAR*)pdc->pwszText;
		}
		else {
			ppd2.flags			= PU2_ANSI;
			ppd2.lpzTitle		= (char *)pdc->pszTitle;
			ppd2.lpzText		= (char *)pdc->pszText;
		}
		ppd2.lchContact			= pdc->hContact;
		ppd2.PluginData			= pdc->PluginData;
	
		ret = PopUp_AddPopUp2((WPARAM)&ppd2, 0);
	}
	return ret!=0 ? 1 : 0;
}

