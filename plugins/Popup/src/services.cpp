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

#include "stdafx.h"

int num_classes = 0;			// for core class api support

//===== Popup/AddPopup
INT_PTR Popup_AddPopup(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded)
		return -1;

	POPUPDATA *ppd = (POPUPDATA*)wParam;
	if (!ppd)
		return -1;

	ptrW wszText(mir_a2u(ppd->lpzText)), wszTitle(mir_a2u(ppd->lpzContactName));

	POPUPDATA2 ppd2 = { sizeof(ppd2) };
	ppd2.flags = PU2_UNICODE;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.szTitle.w = wszTitle;
	ppd2.szText.w = wszText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;
	return Popup_AddPopup2((WPARAM)&ppd2, lParam);
}

//===== Popup/AddPopupW
INT_PTR Popup_AddPopupW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded)
		return -1;

	POPUPDATAW *ppd = (POPUPDATAW*)wParam;
	if (!ppd)
		return -1;

	POPUPDATA2 ppd2 = { 0 };
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_UNICODE;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.szTitle.w = ppd->lpwzContactName;
	ppd2.szText.w = ppd->lpwzText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;
	ppd2.lchNotification = ppd->hNotification;
	ppd2.actionCount = ppd->actionCount;
	ppd2.lpActions = ppd->lpActions;

	return Popup_AddPopup2((WPARAM)&ppd2, lParam);
}

//===== Popup/AddPopup2
static __forceinline uint32_t Proto_Status2Flag_My(uint32_t status)
{
	if (uint32_t res = Proto_Status2Flag(status))
		return res;
	return PF2_IDLE;
}

INT_PTR Popup_AddPopup2(WPARAM wParam, LPARAM lParam)
{
	/* NOTE: we will return 0 instead of -1 since tabSRMM stops using popup after first failure :/ */

	if (!gbPopupLoaded)
		return -1;

	POPUPDATA2 *ppdIn = (POPUPDATA2*)wParam;
	if (!ppdIn)
		return -1;

	if (NotifyEventHooks(hEventNotify, (WPARAM)ppdIn->lchContact, (LPARAM)ppdIn->PluginWindowProc))
		return 0;

	POPUPDATA2 ppdFixed = { 0 };
	POPUPDATA2 *ppd = &ppdFixed;
	memcpy(ppd, ppdIn, min(ppdIn->cbSize, sizeof(POPUPDATA2)));

	uint32_t disableWhen;
	FillNotificationData(ppd, &disableWhen);

	if (!(lParam & APF_NO_HISTORY))
		PopupHistoryAdd(ppd);

	if (PopupThreadIsFull())
		return -1;

	if (IsWorkstationLocked())
		return -1;

	// Check if contact handle is valid.
	char *proto = nullptr;
	if (ppd->lchContact)
		proto = Proto_GetBaseAccountName(ppd->lchContact);

	uint8_t bShowMode = proto ? db_get_b(ppd->lchContact, MODULENAME, "ShowMode", PU_SHOWMODE_AUTO) : PU_SHOWMODE_AUTO;

	if (bShowMode == PU_SHOWMODE_BLOCK)
		return -1;

	if (bShowMode != PU_SHOWMODE_FAVORITE) {
		if (!Popup_Enabled())
			return -1;

		if (PopupOptions.DisableWhenFullscreen && (bShowMode != PU_SHOWMODE_FULLSCREEN) && IsFullScreen())
			return -1;

		if (g_plugin.getDword(LPGEN("Global Status"), 0) & Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if ((disableWhen & 0x0000FFFF) & Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if (proto) {
			char prefix[128];
			mir_snprintf(prefix, LPGEN("Protocol Status") "/%s", Proto_GetBaseAccountName(ppd->lchContact));
			if (g_plugin.getDword(prefix, 0) & Proto_Status2Flag_My(Proto_GetStatus(proto)))
				return -1;
			if (((disableWhen >> 16) & 0xFFFF0000) & Proto_Status2Flag_My(Proto_GetStatus(proto)))
				return -1;
		}
	}

	if (lParam & APF_CUSTOM_POPUP)
		ppd->flags |= PU2_CUSTOM_POPUP;

	PopupWnd2 *wnd = new PopupWnd2(ppd, nullptr, false);
	if (lParam & APF_RETURN_HWND) {
		while (!wnd->m_bWindowCreated) Sleep(1);
		return (INT_PTR)wnd->getHwnd();
	}

	return 0;
}

//===== Popup/GetContact
INT_PTR Popup_GetContact(WPARAM wParam, LPARAM)
{
	if (!gbPopupLoaded) return -1;

	HWND hwnd = (HWND)wParam;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd)) return (INT_PTR)wnd->getContactPassed();
	return (INT_PTR)(-1);
}

//===== Popup/GetPluginData
INT_PTR Popup_GetPluginData(WPARAM wParam, LPARAM)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd))
		return (INT_PTR)wnd->getData();
	return (INT_PTR)(-1);
}

//===== Popup/ChangeTextW
INT_PTR Popup_ChangeTextW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd))
		return -1;

	wnd->callMethodSync(&PopupWnd2::m_updateText, lParam);
	return 0;
}

//===== Popup/ChangeW
INT_PTR Popup_ChangeW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd))
		return -1;

	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATAW_V2, lParam);
	return 0;
}

//===== Popup/Change2
INT_PTR Popup_Change2(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATA2, lParam);
	return 0;
}

INT_PTR Popup_ShowMessageW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam || !lParam) return -1;
	if (closing) return 0;

	POPUPDATA2 ppd2 = { 0 };
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_UNICODE;
	ppd2.szText.w = (wchar_t*)wParam;
	switch (lParam & 0x7fffffff) {
	case SM_ERROR:
		ppd2.lchIcon = g_plugin.getIcon(IDI_MB_STOP, 0);
		ppd2.colorBack = RGB(191, 0, 0);
		ppd2.colorText = RGB(255, 245, 225);
		ppd2.lchNotification = g_hntfError;
		ppd2.szTitle.w = TranslateT("Error");
		break;
	case SM_WARNING:
		ppd2.lchIcon = g_plugin.getIcon(IDI_MB_WARN, 0);
		ppd2.colorBack = RGB(210, 210, 150);
		ppd2.colorText = RGB(0, 0, 0);
		ppd2.lchNotification = g_hntfWarning;
		ppd2.szTitle.w = TranslateT("Warning");
		break;
	case SM_NOTIFY:
		ppd2.lchIcon = g_plugin.getIcon(IDI_MB_INFO, 0);
		ppd2.colorBack = RGB(230, 230, 230);
		ppd2.colorText = RGB(0, 0, 0);
		ppd2.lchNotification = g_hntfNotification;
		ppd2.szTitle.w = TranslateT("Notify");
		break;
	default: // No no no... you must give me a good value.
		return -1;
	}
	return Popup_AddPopup2((WPARAM)&ppd2, (lParam & 0x80000000) ? APF_NO_HISTORY : 0);
}

INT_PTR Popup_ShowMessage(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam || !lParam) return -1;
	if (closing) return 0;

	ptrW wszMsg(mir_a2u((char*)wParam));
	return Popup_ShowMessageW(wszMsg, lParam);
}


//===== Popup/RegisterActions
INT_PTR Popup_RegisterActions(WPARAM wParam, LPARAM lParam)
{
	POPUPACTION *actions = (POPUPACTION*)wParam;
	for (int i = 0; i < lParam; ++i)
		RegisterAction(&actions[i]);
	return 0;
}


INT_PTR Popup_RegisterNotification(WPARAM wParam, LPARAM)
{
	return (INT_PTR)RegisterNotification((POPUPNOTIFICATION*)wParam);
}


//===== Popup/UnhookEventAsync
struct SafeUnhookEventParam
{
	HWND hwndPopup;
	HANDLE hEvent;
};

static void CALLBACK SafeUnhookEventFunc(void *param)
{
	SafeUnhookEventParam *p = (SafeUnhookEventParam*)param;

	UnhookEvent(p->hEvent);
	PostMessage(p->hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0, (LPARAM)p->hEvent);
	delete p;
}

INT_PTR Popup_UnhookEventAsync(WPARAM wParam, LPARAM lParam)
{
	SafeUnhookEventParam *param = new SafeUnhookEventParam;
	param->hwndPopup = (HWND)wParam;
	param->hEvent = (HANDLE)lParam;
	CallFunctionAsync(SafeUnhookEventFunc, param);
	return 0;
}

//===== Popup/RegisterVfx (effekt name for drop down box)
INT_PTR Popup_RegisterVfx(WPARAM, LPARAM lParam)
{
	OptAdv_RegisterVfx((char *)lParam);
	return 0;
}

//===== Popup/RegisterClass		(for core class api support)
INT_PTR Popup_RegisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPCLASS *pc = (POPUPCLASS *)lParam;
	POPUPTREEDATA *ptd = (POPUPTREEDATA *)mir_calloc(sizeof(POPUPTREEDATA));
	ptd->signature = PopupNotificationData_SIGNATURE;
	ptd->typ = 2;
	memcpy(&ptd->pupClass, pc, sizeof(POPUPCLASS));
	if (pc->colorText == 0) // default text color
		ptd->pupClass.colorText = fonts.clText;
	if (pc->colorBack == 0) // default back color
		ptd->pupClass.colorBack = fonts.clBack;
	ptd->pszTreeRoot = mir_a2u(pc->pszName);
	ptd->pupClass.pszName = mir_strdup(pc->pszName);
	if (pc->flags & PCF_UNICODE) {
		ptd->pupClass.pszDescription.w = mir_wstrdup(pc->pszDescription.w);
		ptd->pszDescription = mir_wstrdup(pc->pszDescription.w);
	}
	else {
		ptd->pupClass.pszDescription.a = mir_strdup(pc->pszDescription.a);
		ptd->pszDescription = mir_a2u(pc->pszDescription.a);
	}
	LoadClassSettings(ptd, PU_MODULCLASS);

	// text & back colors
	mir_snwprintf(ptd->fid.group, L"%S/%s", PU_FNT_AND_COLOR, ptd->pszDescription);
	strncpy_s(ptd->fid.dbSettingsGroup, PU_MODULCLASS, _TRUNCATE);
	ptd->fid.flags = FIDF_DEFAULTVALID;
	ptd->fid.deffontsettings.charset = DEFAULT_CHARSET;
	ptd->fid.deffontsettings.size = -11;
	wcsncpy_s(ptd->fid.deffontsettings.szFace, L"Verdana", _TRUNCATE);
	wcsncpy_s(ptd->fid.name, _A2W(PU_FNT_NAME_TEXT), _TRUNCATE);
	mir_snprintf(ptd->fid.setting, "%s/TextCol", ptd->pupClass.pszName);  // result is "%s/TextCol"
	ptd->fid.deffontsettings.style = 0;
	ptd->fid.deffontsettings.colour = ptd->pupClass.colorText;
	g_plugin.addFont(&ptd->fid);

	mir_snwprintf(ptd->cid.group, L"%S/%s", PU_FNT_AND_COLOR, ptd->pszDescription);
	wcsncpy_s(ptd->cid.name, PU_COL_BACK_NAME, _TRUNCATE);
	strncpy_s(ptd->cid.dbSettingsGroup, PU_MODULCLASS, _TRUNCATE);
	mir_snprintf(ptd->cid.setting, "%s/BgCol", ptd->pupClass.pszName);
	ptd->cid.defcolour = ptd->pupClass.colorBack;
	g_plugin.addColor(&ptd->cid);

	gTreeData.insert(ptd);
	num_classes++;
	return (INT_PTR)ptd;
}

INT_PTR Popup_UnregisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPTREEDATA *ptd = (POPUPTREEDATA*)lParam;
	if (ptd == nullptr)
		return 1;

	for (auto &it : gTreeData)
		if (it == ptd) {
			gTreeData.removeItem(&it);
			FreePopupClass(ptd);
			return 0;
		}

	return 1;
}

//===== Popup/AddPopupClass		(for core class api support)
INT_PTR Popup_CreateClassPopup(WPARAM wParam, LPARAM lParam)
{
	POPUPDATACLASS *pdc = (POPUPDATACLASS *)lParam;
	if (!pdc)
		return 1;

	POPUPCLASS *pc;
	if (wParam)
		pc = (POPUPCLASS*)wParam;
	else {
		LPTSTR group = mir_a2u(pdc->pszClassName);
		POPUPTREEDATA *ptd = (POPUPTREEDATA *)FindTreeData(group, nullptr, 2);
		if (ptd)
			pc = &ptd->pupClass;
		else
			pc = nullptr;
		mir_free(group);
	}
	if (pc == nullptr)
		return 1;

	POPUPDATA2 ppd2 = { sizeof(ppd2) };
	ppd2.colorBack = pc->colorBack;
	ppd2.colorText = pc->colorText;
	ppd2.lchIcon = pc->hIcon;
	ppd2.iSeconds = pc->iSeconds;
	ppd2.PluginWindowProc = pc->PluginWindowProc;
	if (pc->flags & PCF_UNICODE) {
		ppd2.flags = PU2_UNICODE;
		ppd2.szTitle.w = (wchar_t*)pdc->szTitle.w;
		ppd2.szText.w = (wchar_t*)pdc->szText.w;
	}
	else {
		ppd2.flags = PU2_ANSI;
		ppd2.szTitle.a = (char *)pdc->szTitle.a;
		ppd2.szText.a = (char *)pdc->szText.a;
	}
	ppd2.lchContact = pdc->hContact;
	ppd2.PluginData = pdc->PluginData;

	return Popup_AddPopup2((WPARAM)&ppd2, pc->lParam);
}

INT_PTR Popup_DeletePopup(WPARAM, LPARAM lParam)
{
	return (INT_PTR)SendMessage((HWND)lParam, UM_DESTROYPOPUP, 0, 0);
}

INT_PTR Popup_LoadSkin(WPARAM, LPARAM lParam)
{
	if (lParam)
	{
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_a2u((char*)lParam);
	}

	const PopupSkin *skin = nullptr;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_wstrdup(skin->getName());
	}

	return 1;
}
