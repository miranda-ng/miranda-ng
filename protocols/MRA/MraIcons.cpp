#include "Mra.h"
#include "MraIcons.h"

void CMraProto::AddIcoLibItems(LPWSTR lpwszRootSectionName, LPWSTR lpwszSubSectionName, const GUI_DISPLAY_ITEM *pgdiItems, size_t dwCount, HANDLE *hResult)
{
	char szBuff[MAX_PATH];
	WCHAR wszSection[MAX_PATH], wszPath[MAX_FILEPATH];

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.pwszSection = wszSection;
	sid.pwszDefaultFile = wszPath;
	sid.cx = sid.cy = 16;
	sid.flags = SIDF_ALL_UNICODE;

	if (lpwszSubSectionName == NULL) lpwszSubSectionName = L"";
	GetModuleFileName(masMraSettings.hInstance, wszPath, SIZEOF(wszPath));
	mir_sntprintf(wszSection, SIZEOF(wszSection), L"%s%s%s", lpwszRootSectionName, m_tszUserName, lpwszSubSectionName);

	for (size_t i = 0;i<dwCount;i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s_%s", m_szModuleName, pgdiItems[i].lpszName);
		sid.pszName = szBuff;
		sid.pwszDescription = pgdiItems[i].lpwszDescr;
		sid.iDefaultIndex = -pgdiItems[i].defIcon;
		sid.hDefaultIcon = (HICON)LoadImage(masMraSettings.hInstance, MAKEINTRESOURCE(pgdiItems[i].defIcon), IMAGE_ICON, 0, 0, LR_SHARED);
		if (sid.hDefaultIcon == NULL) sid.hDefaultIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(pgdiItems[i].defIcon), IMAGE_ICON, 0, 0, LR_SHARED);
		hResult[i] = Skin_AddIcon(&sid);
	}
}

HICON IconLibGetIcon(HANDLE hIcon)
{
	return IconLibGetIconEx(hIcon, LR_SHARED);
}

HICON IconLibGetIconEx(HANDLE hIcon, DWORD dwFlags)
{
	HICON hiIcon = NULL;
	if (hIcon) {
		hiIcon = Skin_GetIconByHandle(hIcon);
		if ((dwFlags & LR_SHARED) == 0)
			hiIcon = CopyIcon(hiIcon);
	}
	return hiIcon;
}


//////////////////////////////////////////////////////////////////////////////////////

void CMraProto::IconsLoad()
{
	AddIcoLibItems(L"Protocols/", L"/MainMenu", gdiMenuItems, gdiMenuItemsCount, hMainMenuIcons);
	AddIcoLibItems(L"Protocols/", L"/ContactMenu", gdiContactMenuItems, gdiContactMenuItemsCount, hContactMenuIcons);

	// Advanced Status Icons initialization
	AddIcoLibItems(L"Protocols/", L"/Extra status", gdiExtraStatusIconsItems, gdiExtraStatusIconsItemsCount, hAdvancedStatusIcons);
}


void CMraProto::IconsUnLoad()
{
	// Advanced Status Icons initialization
	bzero(hAdvancedStatusIcons, sizeof(hAdvancedStatusIcons));
	bzero(hAdvancedStatusItems, sizeof(hAdvancedStatusItems));
}

void CMraProto::InitXStatusIcons()
{
	int iCurIndex;
	char szBuff[MAX_PATH];
	WCHAR wszSection[MAX_PATH], wszPath[MAX_FILEPATH];

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.pwszSection = wszSection;
	sid.pwszDefaultFile = wszPath;
	sid.cx = sid.cy = 16;
	sid.flags = SIDF_ALL_UNICODE;

	if (masMraSettings.hDLLXStatusIcons)
		GetModuleFileName(masMraSettings.hDLLXStatusIcons, wszPath, SIZEOF(wszPath));
	else
		bzero(wszPath, sizeof(wszPath));

	mir_sntprintf(wszSection, SIZEOF(wszSection), L"Status Icons/%s/Custom Status", m_tszUserName);

	hXStatusAdvancedStatusIcons[0] = NULL;
	for (size_t i = 1; i < MRA_XSTATUS_COUNT+1; i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s_xstatus%ld", m_szModuleName, i);
		iCurIndex = (IDI_XSTATUS1-1+i);
		sid.pszName = szBuff;
		sid.pwszDescription = lpcszXStatusNameDef[i];
		sid.iDefaultIndex = -iCurIndex;
		if (masMraSettings.hDLLXStatusIcons)
			sid.hDefaultIcon = (HICON)LoadImage(masMraSettings.hDLLXStatusIcons, MAKEINTRESOURCE(iCurIndex), IMAGE_ICON, 0, 0, LR_SHARED);
		else
			sid.hDefaultIcon = NULL;

		hXStatusAdvancedStatusIcons[i] = Skin_AddIcon(&sid);
	}
}

void CMraProto::DestroyXStatusIcons()
{
	char szBuff[MAX_PATH];

	for (size_t i = 1; i < MRA_XSTATUS_COUNT+1; i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "xstatus%ld", i);
		Skin_RemoveIcon(szBuff);
	}

	bzero(hXStatusAdvancedStatusIcons, sizeof(hXStatusAdvancedStatusIcons));
	bzero(hXStatusAdvancedStatusItems, sizeof(hXStatusAdvancedStatusItems));
}


