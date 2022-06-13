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

HANDLE g_hntfError, g_hntfWarning, g_hntfNotification;

int TreeDataSortFunc(const POPUPTREEDATA *p1, const POPUPTREEDATA *p2)
{
	if (int cmp = mir_wstrcmp(p1->pszTreeRoot, p2->pszTreeRoot))
		return cmp;
	return mir_wstrcmp(p1->pszDescription, p2->pszDescription);
}

LIST<POPUPTREEDATA> gTreeData(20, TreeDataSortFunc);

// interface
void LoadNotifications()
{
	POPUPNOTIFICATION notification = { 0 };
	notification.cbSize = sizeof(notification);
	notification.actionCount = 0;
	notification.lpActions = nullptr;

	mir_strncpy(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	mir_strncpy(notification.lpzName, "Warning", sizeof(notification.lpzName));
	notification.lchIcoLib = g_plugin.getIconHandle(IDI_MB_WARN);
	notification.colorBack = RGB(210, 210, 150);
	notification.colorText = RGB(0, 0, 0);
	notification.iSeconds = 10;
	g_hntfWarning = RegisterNotification(&notification);

	mir_strncpy(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	mir_strncpy(notification.lpzName, "Notification", sizeof(notification.lpzName));
	notification.lchIcoLib = g_plugin.getIconHandle(IDI_MB_INFO);
	notification.colorBack = RGB(230, 230, 230);
	notification.colorText = RGB(0, 0, 0);
	notification.iSeconds = 7;
	g_hntfNotification = RegisterNotification(&notification);

	mir_strncpy(notification.lpzGroup, "Misc", sizeof(notification.lpzName));
	mir_strncpy(notification.lpzName, "Error", sizeof(notification.lpzName));
	notification.lchIcoLib = g_plugin.getIconHandle(IDI_MB_STOP);
	notification.colorBack = RGB(191, 0, 0);
	notification.colorText = RGB(255, 245, 225);
	notification.iSeconds = -1;
	g_hntfError = RegisterNotification(&notification);
}

void FreePopupClass(POPUPTREEDATA *ptd)
{
	if (ptd->typ == 2) {
		mir_free(ptd->pupClass.pszName);
		mir_free(ptd->pupClass.pszDescription.w);
	}
	mir_free(ptd->pszTreeRoot);
	mir_free(ptd->pszDescription);
	mir_free(ptd);
}

void UnloadTreeData()
{
	for (auto &it : gTreeData)
		FreePopupClass(it);
	gTreeData.destroy();
}

void SaveNotificationSettings(POPUPTREEDATA *ptd, char *szModul)
{
	if (ptd->typ == 1) {
		char setting[2 * MAXMODULELABELLENGTH];

		mir_snprintf(setting, "{%s/%s}Timeout",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_w(0, szModul, setting, ptd->notification.iSeconds);

		mir_snprintf(setting, "{%s/%s}enabled",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_b(0, szModul, setting, ptd->enabled);

		mir_snprintf(setting, "{%s/%s}TimeoutVal",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_w(0, szModul, setting, ptd->timeoutValue);

		mir_snprintf(setting, "{%s/%s}disableWhen",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_b(0, szModul, setting, ptd->disableWhen);

		mir_snprintf(setting, "{%s/%s}leftAction",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_s(0, szModul, setting, ptd->leftAction);

		mir_snprintf(setting, "{%s/%s}rightAction",
			ptd->notification.lpzGroup,
			ptd->notification.lpzName);
		db_set_s(0, szModul, setting, ptd->rightAction);

		for (int i = 0; i < ptd->notification.actionCount; ++i) {
			POPUPNOTIFYACTION &p = ptd->notification.lpActions[i];
			if (!mir_strcmp(ptd->leftAction, p.lpzTitle))
				db_set(NULL, p.lpzLModule, p.lpzLSetting, &p.dbvLData);

			if (!mir_strcmp(ptd->rightAction, p.lpzTitle))
				db_set(NULL, p.lpzRModule, p.lpzRSetting, &p.dbvRData);
		}
	}
}

void LoadNotificationSettings(POPUPTREEDATA *ptd, char* szModul)
{
	if (ptd->typ == 1) {
		char setting[2 * MAXMODULELABELLENGTH];

		mir_snprintf(setting, "{%s/%s}enabled", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->enabled =
			(signed char)db_get_b(0, szModul, setting, TRUE);

		mir_snprintf(setting, "{%s/%s}Timeout", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->notification.iSeconds =
			(signed char)db_get_w(0, szModul, setting, ptd->notification.iSeconds);

		mir_snprintf(setting, "{%s/%s}TimeoutVal", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->timeoutValue =
			(signed char)db_get_w(0, szModul, setting,
			ptd->notification.iSeconds ? ptd->notification.iSeconds : 0);

		mir_snprintf(setting, "{%s/%s}disableWhen", ptd->notification.lpzGroup, ptd->notification.lpzName);
		ptd->disableWhen =
			db_get_b(0, szModul, setting, 0);

		mir_snprintf(setting, "{%s/%s}leftAction", ptd->notification.lpzGroup, ptd->notification.lpzName);
		char *szTmp = db_get_sa(0, szModul, setting, ptd->notification.lpzLAction);
		mir_strncpy(ptd->leftAction, szTmp, sizeof(ptd->leftAction));
		mir_free(szTmp); szTmp = nullptr;

		mir_snprintf(setting, "{%s/%s}rightAction", ptd->notification.lpzGroup, ptd->notification.lpzName);
		szTmp = db_get_sa(0, szModul, setting, ptd->notification.lpzRAction);
		mir_strncpy(ptd->rightAction, szTmp, sizeof(ptd->rightAction));
		mir_free(szTmp); szTmp = nullptr;
	}
}

HANDLE RegisterNotification(POPUPNOTIFICATION *notification)
{
	POPUPTREEDATA *ptd = (POPUPTREEDATA *)mir_alloc(sizeof(POPUPTREEDATA));
	ptd->signature = PopupNotificationData_SIGNATURE;
	ptd->typ = 1;
	ptd->pszTreeRoot = mir_a2u(notification->lpzGroup);
	ptd->pszDescription = mir_a2u(notification->lpzName);
	ptd->notification = *notification;
	ptd->hIcoLib = notification->lchIcoLib;
	if (!ptd->notification.lpzLAction) ptd->notification.lpzLAction = POPUP_ACTION_NOTHING;
	if (!ptd->notification.lpzRAction) ptd->notification.lpzRAction = POPUP_ACTION_DISMISS;
	LoadNotificationSettings(ptd, "PopupNotifications");

	// ugly hack to make reset always possible
	SaveNotificationSettings(ptd, "PopupNotifications");

	FontID fontid = {};
	mir_snprintf(fontid.group, PU_FNT_AND_COLOR "/%s", notification->lpzGroup);
	mir_strcpy(fontid.dbSettingsGroup, "PopupNotifications");
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.colour = ptd->notification.colorText;
	fontid.deffontsettings.size = -11;
	mir_strncpy(fontid.deffontsettings.szFace, "MS Shell Dlg", _countof(fontid.deffontsettings.szFace));
	fontid.deffontsettings.style = 0;
	mir_snprintf(fontid.name, "%s (colors only)", notification->lpzName);
	mir_snprintf(fontid.setting, "{%s/%s}text", notification->lpzGroup, notification->lpzName);
	fontid.deffontsettings.style = 0;
	g_plugin.addFont(&fontid);

	ColourID colourid = {};
	mir_snprintf(colourid.group, PU_FNT_AND_COLOR"/%s", notification->lpzGroup);
	mir_strcpy(colourid.dbSettingsGroup, "PopupNotifications");
	mir_snprintf(colourid.name, "%s (colors only)", notification->lpzName);
	mir_snprintf(colourid.setting, "{%s/%s}backColor", notification->lpzGroup, notification->lpzName);
	colourid.defcolour = ptd->notification.colorBack;
	g_plugin.addColor(&colourid);

	gTreeData.insert(ptd);
	return (HANDLE)ptd;
}

HANDLE FindTreeData(LPTSTR group, LPTSTR name, uint8_t typ)
{
	for (auto &p : gTreeData)
		if (p->typ == typ && (!group || (mir_wstrcmp(p->pszTreeRoot, group) == 0)) && (!name || (mir_wstrcmp(p->pszDescription, name) == 0)))
			return p;

	return nullptr;
}

void FillNotificationData(POPUPDATA2 *ppd, uint32_t *disableWhen)
{
	if (!IsValidNotification(ppd->lchNotification)) {
		*disableWhen = 0;
		return;
	}

	POPUPTREEDATA *ptd = (POPUPTREEDATA *)ppd->lchNotification;

	ppd->iSeconds = ptd->timeoutValue;
	*disableWhen = ptd->enabled ? ptd->disableWhen : 0xFFFFFFFF;

	LOGFONTA lf; // dummy to make FS happy (use LOGFONTA coz we use MS_FONT_GET)
	CMStringA szGroup(FORMAT, PU_FNT_AND_COLOR"/%s", ptd->notification.lpzGroup);
	CMStringA szName(FORMAT, "%s (colors only)", ptd->notification.lpzName);
	ppd->colorText = Font_Get(szGroup, szName, &lf);
	ppd->colorBack = Colour_Get(szGroup, szName);

	ppd->lchIcon = IcoLib_GetIconByHandle(ptd->hIcoLib);
}

bool IsValidNotification(HANDLE hNotification)
{
	if (!hNotification) return false;

	bool res = false;
	__try {
		if (((POPUPTREEDATA *)hNotification)->signature == PopupNotificationData_SIGNATURE)
			res = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		res = false;
	}
	return res;
}

bool PerformAction(HANDLE hNotification, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (!IsValidNotification(hNotification))
		return false;

	POPUPTREEDATA *ptd = (POPUPTREEDATA *)hNotification;
	char *lpzAction = nullptr;
	switch (message) {
	case WM_LBUTTONUP:
	case WM_COMMAND:
		lpzAction = ptd->leftAction;
		break;

	case WM_RBUTTONUP:
	case WM_CONTEXTMENU:
		lpzAction = ptd->rightAction;
		break;

	default:
		return false;
	}

	if (!mir_strcmp(lpzAction, POPUP_ACTION_NOTHING))
		return true;

	if (!mir_strcmp(lpzAction, POPUP_ACTION_DISMISS)) {
		PUDeletePopup(hwnd);
		return true;
	}

	for (int i = 0; i < ptd->notification.actionCount; ++i) {
		if (!(ptd->notification.lpActions[i].dwFlags & PNAF_CALLBACK))
			continue;
		if (mir_strcmp(ptd->notification.lpActions[i].lpzTitle, lpzAction))
			continue;

		ptd->notification.lpActions[i].pfnCallback(hwnd, message, wparam, lparam,
			ptd->notification.lpActions[i].dwCookie);
		return true;
	}

	return false;
}
