/*
Miranda Floating Contacts plugin, created by Iavor Vajarov ( ivajarov@code.bg )
http://miranda-icq.sourceforge.net/

Miranda fonts and colors settings by Ranger.
Extended bonts and backgrounds settings by Oleksiy Shurubura


This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.

This plugin has been tested to work under Windows ME, 2000 and XP.

No warranty for any misbehaviour.
*/

#include "stdafx.h"

#include "../../utils/mir_fonts.h"

/////////////////////////////////////////////////////////////////////////////
// Globals

// TODO: move to some more approriate place, probably part of Thumbs manager
static void	LoadContacts(void);
static void LoadContact(MCONTACT hContact);

// Internal funcs
static void	CreateThumbWnd(wchar_t *ptszName, MCONTACT hContact, int nX, int nY);
static void	RegisterWindowClass(void);
static void	UnregisterWindowClass(void);
static void LoadDBSettings(void);
static void CreateThumbsFont(void);
static void CreateBackgroundBrush(void);
static int  GetContactStatus(MCONTACT hContact);
static void GetScreenRect(void);
extern void SetThumbsOpacity(uint8_t btAlpha);
static int  ClcStatusToPf2(int status);

HFONT      hFont[FLT_FONTIDS];
COLORREF   tColor[FLT_FONTIDS];
HIMAGELIST himlMiranda;
MCONTACT   hNewContact;

HPEN       hLTEdgesPen;
HPEN       hRBEdgesPen;
HBRUSH     hBkBrush;
uint32_t   bkColor;
HBITMAP    hBmpBackground;
uint16_t   nBackgroundBmpUse = CLB_STRETCH;

HWND       hwndMiranda;
BOOL       bVersionOK;
BOOL       bDockHorz = TRUE;
HMENU      hContactMenu;
RECT       rcScreen;
uint32_t   dwOfflineModes;
BOOL       bEnableTip;
UINT_PTR   ToTopTimerID;
BOOL       bIsCListShow;

HGENMENU	hMenuItemRemove, hMenuItemHideAll, hMainMenuItemHideAll;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {53C715A8-EB01-4136-A73C-441868610074}
	{0x53c715a8, 0xeb01, 0x4136, {0xa7, 0x3c, 0x44, 0x18, 0x68, 0x61, 0x0, 0x74}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////
// Options

FCOptions fcOpt = { 0 };

static void InitOptions()
{
	fcOpt.thumbAlpha = 255;
	fcOpt.bHideOffline = FALSE;
	fcOpt.bHideAll = FALSE;
	fcOpt.bHideWhenFullscreen = FALSE;
	fcOpt.bMoveTogether = FALSE;
	fcOpt.bFixedWidth = FALSE;
	fcOpt.nThumbWidth = 0;
	fcOpt.bShowTip = TRUE;
	fcOpt.TimeIn = 0;
	fcOpt.bToTop = TRUE;
	fcOpt.ToTopTime = TOTOPTIME_DEF;
	fcOpt.bHideWhenCListShow = FALSE;
	fcOpt.bUseSingleClick = FALSE;
	fcOpt.bShowIdle = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Fonts

static LPCTSTR s_fonts[FLT_FONTIDS] =
{
	{ LPGENW("Standard contacts") },
	{ LPGENW("Online contacts to whom you have a different visibility") },
	{ LPGENW("Offline contacts") },
	{ LPGENW("Offline contacts to whom you have a different visibility") },
	{ LPGENW("Contacts which are 'not on list'") }
};

///////////////////////////////////////////////////////
// Hooked events

static int OnContactDeleted(WPARAM hContact, LPARAM)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb) {
		pThumb->DeleteContactPos();
		thumbList.RemoveThumb(pThumb);
	}
	return 0;
}

static int OnContactIconChanged(WPARAM hContact, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb) {
		pThumb->RefreshContactIcon((int)lParam);
		//pThumb->ThumbSelect( FALSE );
		//SetTimer( pThumb->hwnd, TIMERID_SELECT_T, 1000, NULL );
	}

	return 0;
}

static int OnContactDrag(WPARAM hContact, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);

	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb == nullptr) {
		int idStatus = GetContactStatus(hContact);

		if (!fcOpt.bHideAll && !HideOnFullScreen() && (!fcOpt.bHideOffline || IsStatusVisible(idStatus)) && (!fcOpt.bHideWhenCListShow || !bIsCListShow)) {
			CreateThumbWnd(Clist_GetContactDisplayName(hContact), hContact, pt.x, pt.y);
			ThumbInfo *pNewThumb = thumbList.FindThumbByContact(hContact);
			ShowWindow(pNewThumb->hwnd, SW_SHOWNA);
			hNewContact = hContact;

			OnStatusChanged();
		}
	}
	else if (hContact == hNewContact)
		pThumb->PositionThumb(pt.x - 5, pt.y - 5);

	return hNewContact != NULL;
}

static int OnContactDrop(WPARAM hContact, LPARAM)
{
	RECT rcMiranda;
	RECT rcThumb;

	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);

	if (hNewContact == hContact && pThumb != nullptr) {
		hNewContact = NULL;

		GetWindowRect(hwndMiranda, &rcMiranda);
		pThumb->GetThumbRect(&rcThumb);
	}

	SaveContactsPos();
	return 1;
}

static int OnContactDragStop(WPARAM hContact, LPARAM)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb != nullptr && hNewContact == hContact) {
		thumbList.RemoveThumb(pThumb);
		hNewContact = NULL;
	}

	return 0;
}

static int OnSkinIconsChanged(WPARAM, LPARAM)
{
	// Get handle to the image list
	himlMiranda = Clist_GetImageList();

	// Update thumbs
	for (auto &it : thumbList)
		it->UpdateContent();

	return 0;
}

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	int idStatus = ID_STATUS_OFFLINE;
	BOOL bRefresh = TRUE;
	DBCONTACTWRITESETTING *pdbcws = (DBCONTACTWRITESETTING*)lParam;

	if (hContact == NULL) {
		if ((0 == strcmp(pdbcws->szModule, "CLC")) || (0 == strcmp(pdbcws->szModule, MODULENAME))) {
			LoadDBSettings();
			ApplyOptionsChanges();
		}

		return 0;
	}

	if (pThumb == nullptr)
		return 0;

	// Only on these 2 events we need to refresh
	if (0 == strcmp(pdbcws->szSetting, "Status"))
		idStatus = pdbcws->value.wVal;
	else if (0 == strcmp(pdbcws->szSetting, "ApparentMode"))
		idStatus = GetContactStatus(hContact);
	else if (0 == strcmp(pdbcws->szSetting, "Nick"))
		idStatus = GetContactStatus(hContact);
	else if (0 == strcmp(pdbcws->szSetting, "MyHandle"))
		idStatus = GetContactStatus(hContact);
	else if (fcOpt.bShowIdle && 0 == strcmp(pdbcws->szSetting, "IdleTS"))
		idStatus = GetContactStatus(hContact);
	else
		bRefresh = FALSE;

	// Detach call
	if (bRefresh)
		PostMessage(pThumb->hwnd, WM_REFRESH_CONTACT, 0, idStatus);

	return 0;
}

static int OnStatusModeChange(WPARAM wParam, LPARAM)
{
	for (auto &it : thumbList) {
		int idStatus = GetContactStatus(it->hContact);
		it->RefreshContactStatus(idStatus);
	}

	if (wParam == ID_STATUS_OFFLINE) {
		// Floating status window will use this
	}

	return 0;
}

static int OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(wParam);

	Menu_ShowItem(hMenuItemRemove, pThumb != nullptr);
	Menu_ShowItem(hMenuItemHideAll, !fcOpt.bHideAll);
	return 0;
}

///////////////////////////////////////////////////////
// General functions

static void LoadDBSettings()
{
	fcOpt.thumbAlpha = (uint8_t)((double)g_plugin.getByte("Opacity", 100) * 2.55);
	fcOpt.bHideOffline = (BOOL)g_plugin.getByte("HideOffline", 0);
	fcOpt.bHideAll = (BOOL)g_plugin.getByte("HideAll", 0);
	fcOpt.bHideWhenFullscreen = (BOOL)g_plugin.getByte("HideWhenFullscreen", 0);
	fcOpt.bMoveTogether = (BOOL)g_plugin.getByte("MoveTogether", 0);
	fcOpt.bFixedWidth = (BOOL)g_plugin.getByte("FixedWidth", 0);
	fcOpt.nThumbWidth = (uint32_t)g_plugin.getDword("Width", 0);
	dwOfflineModes = Clist::OfflineModes;
	fcOpt.bShowTip = (BOOL)g_plugin.getByte("ShowTip", 1);
	fcOpt.TimeIn = g_plugin.getWord("TimeIn", 0);
	fcOpt.bToTop = (BOOL)g_plugin.getByte("ToTop", 0);
	fcOpt.ToTopTime = g_plugin.getWord("ToTopTime", TOTOPTIME_DEF);
	fcOpt.bHideWhenCListShow = (BOOL)g_plugin.getByte("HideWhenCListShow", 0);
	fcOpt.bUseSingleClick = (BOOL)g_plugin.getByte("UseSingleClick", 0);
	fcOpt.bShowIdle = (BOOL)g_plugin.getByte("ShowIdle", 0);

	if (db_get_b(0, "ModernData", "HideBehind", 0))
		bIsCListShow = (db_get_b(0, "ModernData", "BehindEdge", 0) == 0);
	else
		bIsCListShow = (db_get_b(0, "CList", "State", 0) == 2);
}

void SendMsgDialog(HWND hwnd, wchar_t *pText)
{
	ThumbInfo *pThumb = thumbList.FindThumb(hwnd);
	if (pThumb != nullptr)
		CallService(MS_MSG_SENDMESSAGEW, (WPARAM)pThumb->hContact, (LPARAM)pText);
}

static void ShowContactMenu(HWND hwnd, POINT pt)
{
	ThumbInfo *pThumb = thumbList.FindThumb(hwnd);
	if (pThumb != nullptr) {
		hContactMenu = Menu_BuildContactMenu(pThumb->hContact);
		if (hContactMenu == nullptr)
			return;

		int idCommand = TrackPopupMenu(hContactMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
		Clist_MenuProcessCommand(idCommand, MPCF_CONTACTMENU, pThumb->hContact);
	}
}

static LRESULT __stdcall CommWndProc(HWND	hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumb(hwnd);

	switch (uMsg) {
	case WM_RBUTTONUP:
	{
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);

		if (pThumb) pThumb->ThumbDeselect(TRUE);

		ClientToScreen(hwnd, &pt);
		ShowContactMenu(hwnd, pt);
	}
	break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_NCPAINT:
		if (pThumb) {
			HDC hdc = GetWindowDC(hwnd);
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, nullptr);
			return 0;
		}

	case WM_PAINT:
		if (pThumb) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);
			break;
		}

	case WM_PRINT:
	case WM_PRINTCLIENT:
		if (pThumb) {
			BitBlt((HDC)wParam, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			break;
		}

	case WM_MEASUREITEM:
		Menu_MeasureItem(lParam);
		break;

	case WM_DRAWITEM:
		Menu_DrawItem(lParam);
		break;

	case WM_LBUTTONDOWN:
		if (pThumb)
			pThumb->OnLButtonDown();
		break;

	case WM_MOUSEMOVE:
		if (pThumb)
			pThumb->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONUP:
		if (pThumb)
			pThumb->OnLButtonUp();
		break;

	case WM_LBUTTONDBLCLK:
		// Popup message dialog
		if (!fcOpt.bUseSingleClick && pThumb)
			pThumb->PopupMessageDialog();
		break;

	case WM_RBUTTONDOWN:
		if (bEnableTip && fcOpt.bShowTip && pThumb)
			pThumb->KillTip();
		break;

	case WM_REFRESH_CONTACT:
		if (pThumb) {
			wcsncpy(pThumb->ptszName, Clist_GetContactDisplayName(pThumb->hContact), USERNAME_LEN - 1);
			pThumb->RefreshContactStatus((int)lParam);
			pThumb->ResizeThumb();
		}
		break;

	case WM_TIMER:
		if (pThumb)
			pThumb->OnTimer(wParam);
		break;

	case WM_HOTKEY:
		pThumb = thumbList.FindThumb((HWND)wParam);
		if (pThumb)
			pThumb->PopupMessageDialog();
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

extern void SetThumbsOpacity(uint8_t btAlpha)
{
	for (auto &it : thumbList)
		it->SetThumbOpacity(btAlpha);
}

static void GetScreenRect()
{
	rcScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	rcScreen.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	rcScreen.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + rcScreen.left;
	rcScreen.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + rcScreen.top;
}

void OnStatusChanged()
{
	int idStatus = ID_STATUS_OFFLINE;

	for (auto &it : thumbList) {
		idStatus = GetContactStatus(it->hContact);
		it->RefreshContactStatus(idStatus);
	}
}

static VOID CALLBACK ToTopTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	for (auto &it : thumbList)
		SetWindowPos(it->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

void ApplyOptionsChanges()
{
	CreateThumbsFont();
	CreateBackgroundBrush();

	if (!fcOpt.bToTop && ToTopTimerID) {
		KillTimer(nullptr, ToTopTimerID);
		ToTopTimerID = 0;
	}

	if (fcOpt.bToTop) {
		if (ToTopTimerID) KillTimer(nullptr, ToTopTimerID);
		fcOpt.ToTopTime = (fcOpt.ToTopTime < 1) ? 1 : fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime > TOTOPTIME_MAX) ? TOTOPTIME_MAX : fcOpt.ToTopTime;
		ToTopTimerID = SetTimer(nullptr, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}

	OnStatusChanged();

	for (auto &it : thumbList)
		it->ResizeThumb();
}

///////////////////////////////////////////////////////
// Window creation

static void RegisterWindowClass()
{
	WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wcx.lpfnWndProc = CommWndProc;
	wcx.hInstance = g_plugin.getInst();
	wcx.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wcx.lpszClassName = WND_CLASS;
	RegisterClassEx(&wcx);
}

static void UnregisterWindowClass()
{
	UnregisterClass(WND_CLASS, g_plugin.getInst());
}

static void CreateThumbWnd(wchar_t *ptszName, MCONTACT hContact, int nX, int nY)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb != nullptr)
		return;

	// Prepare for window creation
	HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, WND_CLASS, ptszName, WS_POPUP, nX, nY, 50, 20, nullptr, nullptr, g_plugin.getInst(), nullptr);
	if (hwnd == nullptr)
		return;

	pThumb = thumbList.AddThumb(hwnd, ptszName, hContact);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThumb);
	pThumb->ResizeThumb();

	pThumb->SetThumbOpacity(fcOpt.thumbAlpha);
	ShowWindow(hwnd, (fcOpt.bHideAll || HideOnFullScreen() || fcOpt.bHideOffline || (fcOpt.bHideWhenCListShow && bIsCListShow)) ? SW_HIDE : SW_SHOWNA);
	pThumb->PositionThumb(nX, nY);

	// force repaint
	pThumb->UpdateContent();
}

static void CreateThumbsFont()
{
	for (int nFontId = 0; nFontId < FLT_FONTIDS; nFontId++) {
		if (nullptr != hFont[nFontId]) {
			DeleteObject(hFont[nFontId]);
			hFont[nFontId] = nullptr;
		}

		LOGFONT lf;
		tColor[nFontId] = Font_GetW(LPGENW("Floating contacts"), s_fonts[nFontId], &lf);
		hFont[nFontId] = CreateFontIndirect(&lf);
	}
}

static void CreateBackgroundBrush()
{
	bkColor = g_plugin.getDword("BkColor", FLT_DEFAULT_BKGNDCOLOR);

	if (nullptr != hLTEdgesPen) {
		DeleteObject(hLTEdgesPen);
		hLTEdgesPen = nullptr;
	}

	if (nullptr != hRBEdgesPen) {
		DeleteObject(hRBEdgesPen);
		hRBEdgesPen = nullptr;
	}

	if (nullptr != hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = nullptr;
	}

	if (nullptr != hBkBrush) {
		SetClassLong((HWND)WND_CLASS, GCLP_HBRBACKGROUND, (LONG)NULL);
		DeleteObject(hBkBrush);
		hBkBrush = nullptr;
	}

	if (g_plugin.getByte("DrawBorder", FLT_DEFAULT_DRAWBORDER)) {
		COLORREF cr = (COLORREF)g_plugin.getDword("LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR);
		hLTEdgesPen = CreatePen(PS_SOLID, 1, cr);
		cr = (COLORREF)g_plugin.getDword("RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR);
		hRBEdgesPen = CreatePen(PS_SOLID, 1, cr);
	}

	if (g_plugin.getByte("BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP)) {
		ptrW tszBitmapName(g_plugin.getWStringA("BkBitmap"));
		if (tszBitmapName != NULL)
			hBmpBackground = Bitmap_Load(tszBitmapName);
	}
	nBackgroundBmpUse = (uint16_t)g_plugin.getWord("BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);

	// Create brush
	hBkBrush = CreateSolidBrush(bkColor);

	// Attach brush to the window
	SetClassLongPtr((HWND)WND_CLASS, GCLP_HBRBACKGROUND, (LONG_PTR)hBkBrush);
}

static int GetContactStatus(MCONTACT hContact)
{
	if (hContact == NULL) {
		assert(!"Contact is NULL");
		return 0;
	}

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return ID_STATUS_OFFLINE;

	return db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}


static int ClcStatusToPf2(int status)
{
	switch (status) {
	case ID_STATUS_ONLINE: return PF2_ONLINE;
	case ID_STATUS_AWAY: return PF2_SHORTAWAY;
	case ID_STATUS_DND: return PF2_HEAVYDND;
	case ID_STATUS_NA: return PF2_LONGAWAY;
	case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
	case ID_STATUS_FREECHAT: return PF2_FREECHAT;
	case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
	case ID_STATUS_OFFLINE: return MODEF_OFFLINE;
	}
	return 0;
}

BOOL IsStatusVisible(int status)
{
	return (0 == (dwOfflineModes & ClcStatusToPf2(status)));
}

void RegHotkey(MCONTACT hContact, HWND hwnd)
{
	ptrA szHotKey(g_plugin.getStringA(hContact, "Hotkey"));
	if (!szHotKey)
		return;
	
	if (szHotKey[0] == '\0')
		return;

	UINT nModifiers = 0;
	char chKey = 0;
	char szMod[2][20] = { 0 };
	char szKey[20] = { 0 };

	sscanf(szHotKey, "%[^'+']+%[^'+']+%[^'+']", szMod[0], szMod[1], szKey);

	for (int i = 0; i < 2; i++) {
		if (0 == strncmp(szMod[i], "ALT", 19))
			nModifiers = nModifiers | MOD_ALT;
		else if (0 == strncmp(szMod[i], "CTRL", 19))
			nModifiers = nModifiers | MOD_CONTROL;
		else if (0 == strncmp(szMod[i], "SHIFT", 19))
			nModifiers = nModifiers | MOD_SHIFT;
	}

	chKey = szKey[0];

	RegisterHotKey(hwnd, (INT_PTR)hwnd, nModifiers, VkKeyScan(chKey));
}

///////////////////////////////////////////////////////
// Contact settings

void SaveContactsPos()
{
	for (auto &it : thumbList) {
		SetLastError(0);

		RECT rc;
		it->GetThumbRect(&rc);

		if (0 == GetLastError())
			g_plugin.setDword(it->hContact, "ThumbsPos", DB_POS_MAKE_XY(rc.left, rc.top));
	}
}

static void LoadContacts()
{
	for (auto &hContact : Contacts())
		LoadContact(hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menus

static IconItemT g_iconList[] =
{
	{ LPGENW("Show all thumbs"), "flt_show", IDI_HIDE },
	{ LPGENW("Hide all thumbs"), "flt_hide", IDI_SHOW }
};

static INT_PTR OnMainMenu_HideAll(WPARAM, LPARAM)
{
	fcOpt.bHideAll = !fcOpt.bHideAll;
	g_plugin.setByte("HideAll", (uint8_t)fcOpt.bHideAll);

	OnStatusChanged();

	int i = (fcOpt.bHideAll) ? 0 : 1;
	Menu_ModifyItem(hMainMenuItemHideAll, g_iconList[i].tszDescr, g_iconList[i].hIcolib);
	return 0;
}

static INT_PTR OnContactMenu_Remove(WPARAM hContact, LPARAM)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb) {
		pThumb->DeleteContactPos();
		thumbList.RemoveThumb(pThumb);
	}

	DestroyMenu(hContactMenu);
	return 0;
}

static INT_PTR OnHotKey_HideWhenCListShow(WPARAM, LPARAM)
{
	fcOpt.bHideWhenCListShow = !fcOpt.bHideWhenCListShow;
	g_plugin.setByte("HideWhenCListShow", (uint8_t)fcOpt.bHideWhenCListShow);
	OnStatusChanged();
	return 0;
}

static void LoadMenus()
{
	CMenuItem mi(&g_plugin);

	// Remove thumb menu item
	SET_UID(mi, 0xbab83df0, 0xe126, 0x4d9a, 0xbc, 0xc3, 0x2b, 0xea, 0x84, 0x90, 0x58, 0xc8);
	mi.position = 0xFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_HIDE));
	mi.name.w = LPGENW("Remove thumb");
	mi.pszService = MODULENAME "/RemoveThumb";
	hMenuItemRemove = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, OnContactMenu_Remove);

	// Hide all thumbs main menu item
	SET_UID(mi, 0x9ce9983f, 0x782a, 0x4ec1, 0xb5, 0x9b, 0x41, 0x4e, 0x9d, 0x92, 0x8e, 0xcb);
	mi.pszService = MODULENAME "/MainHideAllThumbs";
	int i = (fcOpt.bHideAll) ? 0 : 1;
	mi.hIcolibItem = g_iconList[i].hIcolib;
	mi.name.w = g_iconList[i].tszDescr;
	hMainMenuItemHideAll = Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, OnMainMenu_HideAll);

	// Register hotkeys
	HOTKEYDESC hkd = {};
	hkd.szSection.a = "Floating Contacts";

	hkd.pszName = hkd.pszService = MODULENAME "/MainHideAllThumbs";
	hkd.szDescription.a = LPGEN("Show/Hide all thumbs");
	g_plugin.addHotkey(&hkd);

	hkd.pszName = hkd.pszService = MODULENAME "/HideWhenCListShow";
	hkd.szDescription.a = LPGEN("Hide when contact list is shown");
	g_plugin.addHotkey(&hkd);
	CreateServiceFunction(hkd.pszService, OnHotKey_HideWhenCListShow);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void LoadContact(MCONTACT hContact)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (hContact == NULL)
		return;

	uint32_t	dwPos = g_plugin.getDword(hContact, "ThumbsPos", (uint32_t)-1);
	if (dwPos != -1) {
		wchar_t	*ptName = Clist_GetContactDisplayName(hContact);
		if (ptName != nullptr) {
			int nX = DB_POS_GETX(dwPos);
			int nY = DB_POS_GETY(dwPos);

			CreateThumbWnd(ptName, hContact, nX, nY);
			pThumb->PositionThumb(nX, nY);
		}
	}
}

BOOL HideOnFullScreen()
{
	BOOL bFullscreen = FALSE;
	HWND hWnd = nullptr;

	if (fcOpt.bHideWhenFullscreen) {
		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);

		hWnd = GetForegroundWindow();

		while (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
			RECT WindowRect;
			GetWindowRect(hWnd, &WindowRect);

			if (w == WindowRect.right - WindowRect.left && h == WindowRect.bottom - WindowRect.top) {
				bFullscreen = TRUE;
				break;
			}

			hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
		}
	}

	return bFullscreen && fcOpt.bHideWhenFullscreen;
}

void ShowThumbsOnHideCList()
{
	if (!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())
		return;

	for (auto &it : thumbList)
		if (!fcOpt.bHideOffline || IsStatusVisible(GetContactStatus(it->hContact)))
			ShowWindow(it->hwnd, SW_SHOWNA);
}


void HideThumbsOnShowCList()
{
	if (!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())
		return;

	for (auto &it : thumbList)
		ShowWindow(it->hwnd, SW_HIDE);
}

static LRESULT __stdcall newMirandaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WINDOWPOSCHANGED) {
		WINDOWPOS *wp = (WINDOWPOS*)lParam;
		if (wp->flags & SWP_HIDEWINDOW) {
			bIsCListShow = FALSE;
			ShowThumbsOnHideCList();
		}
		else if (wp->flags & SWP_SHOWWINDOW) {
			bIsCListShow = TRUE;
			HideThumbsOnShowCList();
		}
		else if (!(wp->flags & SWP_NOMOVE)) {
			uint8_t method = db_get_b(0, "ModernData", "HideBehind", 0);
			if (method) {
				uint16_t wBehindEdgeBorderSize = db_get_w(0, "ModernData", "HideBehindBorderSize", 0);
				RECT rc = { wp->x, wp->y, wp->x + wp->cx, wp->y + wp->cy };
				RECT rcScr = { wBehindEdgeBorderSize*(2 - method), 0, GetSystemMetrics(SM_CXSCREEN) - wBehindEdgeBorderSize * (method - 1), GetSystemMetrics(SM_CYSCREEN) };
				RECT rcOverlap;
				BOOL isIntersect = IntersectRect(&rcOverlap, &rc, &rcScr);
				if (!isIntersect && bIsCListShow) {
					bIsCListShow = FALSE;
					ShowThumbsOnHideCList();
				}
				else if (isIntersect && !bIsCListShow) {
					bIsCListShow = TRUE;
					HideThumbsOnShowCList();
				}
			}
		}
	}
	return mir_callNextSubclass(hwnd, newMirandaWndProc, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnModulesLoded(WPARAM, LPARAM)
{
	HookEvent(ME_CLIST_CONTACTICONCHANGED, OnContactIconChanged);
	HookEvent(ME_SKIN_ICONSCHANGED, OnSkinIconsChanged);
	HookEvent(ME_CLUI_CONTACTDRAGGING, OnContactDrag);
	HookEvent(ME_CLUI_CONTACTDROPPED, OnContactDrop);
	HookEvent(ME_CLUI_CONTACTDRAGSTOP, OnContactDragStop);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	HookEvent(ME_CLIST_STATUSMODECHANGE, OnStatusModeChange);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);

	hwndMiranda = g_clistApi.hwndContactList;
	mir_subclassWindow(hwndMiranda, newMirandaWndProc);

	UINT_PTR dwStyle = SendMessageW(g_clistApi.hwndContactTree, CLM_GETEXSTYLE, 0, 0);
	if (dwStyle & CLS_EX_DISABLEDRAGDROP)
		MessageBox(hwndMiranda,
			TranslateT("Floating contacts plugin won't work until you uncheck the \"Disable drag and drop of items\" option in Options - Contact list"),
			TranslateT("Floating contacts"), MB_ICONWARNING | MB_OK);

	// No thumbs yet
	bEnableTip = ServiceExists("mToolTip/ShowTip");

	RegisterWindowClass();
	GetScreenRect();
	LoadDBSettings();
	CreateBackgroundBrush();
	CreateThumbsFont();
	LoadContacts();

	if (fcOpt.bToTop) {
		fcOpt.ToTopTime = (fcOpt.ToTopTime < 1) ? 1 : fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime > TOTOPTIME_MAX) ? TOTOPTIME_MAX : fcOpt.ToTopTime;
		ToTopTimerID = SetTimer(nullptr, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}
	return 0;
}

static int OnPreshutdown(WPARAM, LPARAM)
{
	while (thumbList.getCount() > 0)
		thumbList.RemoveThumb(&thumbList[0]);
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIconW(LPGENW("Floating contacts"), g_iconList, MODULENAME);
	LoadMenus();
	InitOptions();

	for (int i = 0; i < _countof(s_fonts); i++) {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
		if (i == FLT_FONTID_OFFINVIS || i == FLT_FONTID_INVIS)
			lf.lfItalic = !lf.lfItalic;

		COLORREF defColor = GetSysColor((i == FLT_FONTID_NOTONLIST) ? COLOR_3DSHADOW : COLOR_WINDOWTEXT);

		char szId[20];
		mir_snprintf(szId, "Font%d", i);
		FontService_RegisterFont(MODULENAME, szId, LPGENW("Floating contacts"), s_fonts[i], nullptr, nullptr, i + 1, false, &lf, defColor);
	}

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreshutdown);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialize);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (hLTEdgesPen)
		DeleteObject(hLTEdgesPen);
	if (hRBEdgesPen)
		DeleteObject(hRBEdgesPen);
	if (hBmpBackground)
		DeleteObject(hBmpBackground);
	if (hBkBrush) {
		SetClassLong((HWND)WND_CLASS, GCLP_HBRBACKGROUND, (LONG)NULL);
		DeleteObject(hBkBrush);
		hBkBrush = nullptr;
	}

	for (int nFontId = 0; nFontId < FLT_FONTIDS; nFontId++)
		if (nullptr != hFont[nFontId])
			DeleteObject(hFont[nFontId]);

	UnregisterWindowClass();
	return 0;
}
