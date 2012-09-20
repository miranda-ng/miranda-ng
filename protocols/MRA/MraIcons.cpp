#include "Mra.h"
#include "MraIcons.h"

GUI_DISPLAY_ITEM gdiMenuItems[] = 
{
	{ MRA_GOTO_INBOX,         MRA_GOTO_INBOX_STR,         IDI_INBOX,          &CMraProto::MraGotoInbox }, 
	{ MRA_SHOW_INBOX_STATUS,  MRA_SHOW_INBOX_STATUS_STR,  IDI_MAIL_NOTIFY,    &CMraProto::MraShowInboxStatus }, 
	{ MRA_EDIT_PROFILE,       MRA_EDIT_PROFILE_STR,       IDI_PROFILE,        &CMraProto::MraEditProfile }, 
	{ MRA_MY_ALBUM,           MRA_MY_ALBUM_STR,           IDI_MRA_PHOTO,      &CMraProto::MyAlbum }, 
	{ MRA_MY_BLOG,            MRA_MY_BLOG_STR,            IDI_MRA_BLOGS,      &CMraProto::MyBlog }, 
	{ MRA_MY_BLOGSTATUS,      MRA_MY_BLOGSTATUS_STR,      IDI_BLOGSTATUS,     &CMraProto::MyBlogStatus }, 
	{ MRA_MY_VIDEO,           MRA_MY_VIDEO_STR,           IDI_MRA_VIDEO,      &CMraProto::MyVideo }, 
	{ MRA_MY_ANSWERS,         MRA_MY_ANSWERS_STR,         IDI_MRA_ANSWERS,    &CMraProto::MyAnswers }, 
	{ MRA_MY_WORLD,           MRA_MY_WORLD_STR,           IDI_MRA_WORLD,      &CMraProto::MyWorld }, 
	{ MRA_ZHUKI,              MRA_ZHUKI_STR,              IDI_MRA_ZHUKI,      &CMraProto::MraZhuki }, 
	{ MRA_CHAT,               MRA_CHAT_STR,               IDI_MRA_CHAT,       &CMraProto::MraChat }, 
	{ MRA_WEB_SEARCH,         MRA_WEB_SEARCH_STR,         IDI_MRA_WEB_SEARCH, &CMraProto::MraWebSearch }, 
	{ MRA_UPD_ALL_USERS_INFO, MRA_UPD_ALL_USERS_INFO_STR, IDI_PROFILE,        &CMraProto::MraUpdateAllUsersInfo }, 
	{ MRA_CHK_USERS_AVATARS,  MRA_CHK_USERS_AVATARS_STR,  IDI_PROFILE,        &CMraProto::MraCheckUpdatesUsersAvt }, 
	{ MRA_REQ_AUTH_FOR_ALL,   MRA_REQ_AUTH_FOR_ALL_STR,   IDI_AUTHRUGUEST,    &CMraProto::MraRequestAuthForAll }
};

GUI_DISPLAY_ITEM gdiContactMenuItems[] = 
{
	{ MRA_REQ_AUTH,           MRA_REQ_AUTH_STR,           IDI_AUTHRUGUEST,    &CMraProto::MraRequestAuthorization }, 
	{ MRA_GRANT_AUTH,         MRA_GRANT_AUTH_STR,         IDI_AUTHGRANT,      &CMraProto::MraGrantAuthorization }, 
	{ MRA_SEND_POSTCARD,      MRA_SEND_POSTCARD_STR,      IDI_MRA_POSTCARD,   &CMraProto::MraSendPostcard }, 
	{ MRA_VIEW_ALBUM,         MRA_VIEW_ALBUM_STR,         IDI_MRA_PHOTO,      &CMraProto::MraViewAlbum }, 
	{ MRA_READ_BLOG,          MRA_READ_BLOG_STR    ,      IDI_MRA_BLOGS,      &CMraProto::MraReadBlog }, 
	{ MRA_REPLY_BLOG_STATUS,  MRA_REPLY_BLOG_STATUS_STR,  IDI_BLOGSTATUS,     &CMraProto::MraReplyBlogStatus }, 
	{ MRA_VIEW_VIDEO,         MRA_VIEW_VIDEO_STR,         IDI_MRA_VIDEO,      &CMraProto::MraViewVideo }, 
	{ MRA_ANSWERS,            MRA_ANSWERS_STR,            IDI_MRA_ANSWERS,    &CMraProto::MraAnswers }, 
	{ MRA_WORLD,              MRA_WORLD_STR,              IDI_MRA_WORLD,      &CMraProto::MraWorld }, 
	{ MRA_SEND_NUDGE,         MRA_SENDNUDGE_STR,          IDI_MRA_ALARM,      NULL }
};

GUI_DISPLAY_ITEM gdiExtraStatusIconsItems[]  = 
{
   { ADV_ICON_DELETED_ID,        ADV_ICON_DELETED_STR,      	(INT_PTR)IDI_ERROR, NULL }, 
   { ADV_ICON_NOT_ON_SERVER_ID,  ADV_ICON_NOT_ON_SERVER_STR, 	IDI_AUTHGRANT,      NULL }, 
   { ADV_ICON_NOT_AUTHORIZED_ID, ADV_ICON_NOT_AUTHORIZED_STR, 	IDI_AUTHRUGUEST,    NULL }, 
   { ADV_ICON_PHONE_ID,          ADV_ICON_PHONE_STR,           IDI_MRA_PHONE,      NULL }, 
   { ADV_ICON_BLOGSTATUS_ID,     ADV_ICON_BLOGSTATUS_STR,   	IDI_BLOGSTATUS,     NULL }, 
};

//////////////////////////////////////////////////////////////////////////////////////

static void AddIcoLibItems(LPWSTR lpwszSubSectionName, GUI_DISPLAY_ITEM *pgdiItems, size_t dwCount)
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
	mir_sntprintf(wszSection, SIZEOF(wszSection), L"Protocols/MRA/%s", lpwszSubSectionName);

	for (size_t i = 0;i<dwCount;i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "MRA_%s", pgdiItems[i].lpszName);
		sid.pszName = szBuff;
		sid.pwszDescription = pgdiItems[i].lpwszDescr;
		sid.iDefaultIndex = -pgdiItems[i].defIcon;
		sid.hDefaultIcon = (HICON)LoadImage(masMraSettings.hInstance, MAKEINTRESOURCE(pgdiItems[i].defIcon), IMAGE_ICON, 0, 0, LR_SHARED);
		if (sid.hDefaultIcon == NULL) sid.hDefaultIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(pgdiItems[i].defIcon), IMAGE_ICON, 0, 0, LR_SHARED);
		pgdiItems[i].hIconHandle = Skin_AddIcon(&sid);
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

void IconsLoad()
{
	AddIcoLibItems(L"MainMenu", gdiMenuItems, SIZEOF(gdiMenuItems));
	AddIcoLibItems(L"ContactMenu", gdiContactMenuItems, SIZEOF(gdiContactMenuItems));

	// Advanced Status Icons initialization
	AddIcoLibItems(L"Extra status", gdiExtraStatusIconsItems, SIZEOF(gdiExtraStatusIconsItems));
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

/////////////////////////////////////////////////////////////////////////////////////////

void CMraProto::InitMenus()
{
	// Main menu initialization
	HICON hMainIcon = (HICON)LoadImage(masMraSettings.hInstance, MAKEINTRESOURCE(IDI_MRA), IMAGE_ICON, 0, 0, LR_SHARED);
	CListCreateMenu(200001, 500085000, hMainIcon, NULL, TRUE, gdiMenuItems, SIZEOF(gdiMenuItems), hMainMenuItems);

	// Contact menu initialization
	CListCreateMenu(2000060000, -500050000, NULL, NULL, FALSE, gdiContactMenuItems, SIZEOF(gdiContactMenuItems), hContactMenuItems);
}

void CMraProto::CListCreateMenu(LONG lPosition, LONG lPopupPosition, HICON hMainIcon, LPSTR pszContactOwner, BOOL bIsMain, const GUI_DISPLAY_ITEM *pgdiItems, size_t dwCount, HANDLE *hResult)
{
	if (!pgdiItems || !dwCount || !hResult)
		return;

	char szServiceFunction[MAX_PATH], *pszServiceFunctionName;
	strncpy(szServiceFunction, m_szModuleName, sizeof(szServiceFunction));
	pszServiceFunctionName = szServiceFunction + strlen(m_szModuleName);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);

	HGENMENU (*fnAddFunc)(CLISTMENUITEM*);
	if (bIsMain) {
		fnAddFunc = Menu_AddProtoMenuItem;

		HGENMENU hRootMenu = MO_GetProtoRootMenu(m_szModuleName);
		if (hRootMenu == NULL) {
			mi.ptszName = m_tszUserName;
			mi.position = -1999901008;
			mi.hParentMenu = HGENMENU_ROOT;
			mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.hIcon = hMainIcon;
			hRootMenu = Menu_AddProtoMenuItem(&mi);
		}
		mi.hParentMenu = hRootMenu;
		mi.flags = CMIF_UNICODE | CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP;
	}
	else {
		fnAddFunc = Menu_AddContactMenuItem;
		mi.ptszPopupName = m_tszUserName;
		mi.flags = CMIF_UNICODE | CMIF_ICONFROMICOLIB;
	}

	mi.popupPosition = lPopupPosition;
	mi.pszService = szServiceFunction;

	for (size_t i = 0; i < dwCount; i++) {
		memmove(pszServiceFunctionName, pgdiItems[i].lpszName, lstrlenA(pgdiItems[i].lpszName)+1);
		if (pgdiItems[i].lpFunc)
			CreateObjectSvc(szServiceFunction, pgdiItems[i].lpFunc);
		mi.position = int(lPosition + i);
		mi.icolibItem = pgdiItems[i].hIconHandle;
		mi.ptszName = pgdiItems[i].lpwszDescr;
		hResult[i] = fnAddFunc(&mi);
	}
}
