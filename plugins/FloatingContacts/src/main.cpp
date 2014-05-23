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

#include "stdhdr.h"

#include "../Utils/mir_fonts.h"

// Globals

// TODO: move to some more approriate place, probably part of Thumbs manager
static void	LoadContacts(void);
static void LoadContact(MCONTACT hContact);

// Internal funcs
static void	RepaintWindow(HWND hwnd, HDC hdc);
static void	LoadMenus();
static void	CreateThumbWnd(TCHAR *ptszName, MCONTACT hContact, int nX, int nY);
static void	RegisterWindowClass(void);
static void	UnregisterWindowClass(void);
static void LoadDBSettings(void);
static void CreateThumbsFont(void);
static void CreateBackgroundBrush(void);
static int  GetContactStatus(MCONTACT hContact);
static void GetScreenRect(void);
extern void SetThumbsOpacity(BYTE btAlpha);
static int  ClcStatusToPf2(int status);

static VOID CALLBACK ToTopTimerProc ( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

HINSTANCE  hInst;
HFONT      hFont[FLT_FONTIDS];
COLORREF   tColor[FLT_FONTIDS];
HIMAGELIST himlMiranda;
MCONTACT   hNewContact;

HPEN       hLTEdgesPen;
HPEN       hRBEdgesPen;
HBRUSH     hBkBrush;
DWORD      bkColor;
HBITMAP    hBmpBackground;
WORD       nBackgroundBmpUse = CLB_STRETCH;

HWND       hwndMiranda;
BOOL       bVersionOK;
BOOL       bDockHorz = TRUE;
HMENU      hContactMenu;
RECT       rcScreen;
DWORD      dwOfflineModes;
BOOL       bEnableTip;
UINT_PTR   ToTopTimerID;
BOOL       bIsCListShow;

HGENMENU	hMenuItemRemove, hMenuItemHideAll, hMainMenuItemHideAll;

int hLangpack;
CLIST_INTERFACE *pcli;

//Options

FCOptions fcOpt = {0};

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

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {53C715A8-EB01-4136-A73C-441868610074}
	{0x53c715a8, 0xeb01, 0x4136, {0xa7, 0x3c, 0x44, 0x18, 0x68, 0x61, 0x0, 0x74}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

///////////////////////////////////////////////////////
// Load / unload

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Fonts
									  
static LPCTSTR s_fonts[FLT_FONTIDS]  = 
{
	{ LPGENT("Standard contacts") },
	{ LPGENT("Online contacts to whom you have a different visibility") },
	{ LPGENT("Offline contacts") },
	{ LPGENT("Offline contacts to whom you have a different visibility") },
	{ LPGENT("Contacts which are 'not on list'") }
};

///////////////////////////////////////////////////////
// Hooked events

static int OnContactDeleted(WPARAM hContact, LPARAM lParam)
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

static int OnContactDrag(WPARAM hContact, LPARAM lParam)
{
	POINT pt;
	GetCursorPos(&pt);

	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb == NULL) {
		int idStatus = GetContactStatus(hContact);

		if ( !fcOpt.bHideAll && !HideOnFullScreen() && ( !fcOpt.bHideOffline || IsStatusVisible( idStatus ))  && (!fcOpt.bHideWhenCListShow || !bIsCListShow)) {
			CreateThumbWnd(pcli->pfnGetContactDisplayName(hContact, 0), hContact, pt.x, pt.y);
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

static int OnContactDrop(WPARAM hContact, LPARAM lParam)
{
	RECT rcMiranda;
	RECT rcThumb;

	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);

	if (hNewContact == hContact && pThumb != NULL) {
		hNewContact = NULL;

		GetWindowRect(hwndMiranda, &rcMiranda);
		pThumb->GetThumbRect(&rcThumb);
	}

	SaveContactsPos();
	return 1;
}

static int OnContactDragStop(WPARAM hContact, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb != NULL && hNewContact == hContact) {
		thumbList.RemoveThumb(pThumb);
		hNewContact = NULL;
	}

	return 0;
}

static int OnSkinIconsChanged(WPARAM wParam, LPARAM lParam)
{
	// Get handle to the image list
	himlMiranda = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);

	// Update thumbs
	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i].UpdateContent();

	return 0;
}

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	int idStatus = ID_STATUS_OFFLINE;
	BOOL bRefresh = TRUE;
	DBCONTACTWRITESETTING *pdbcws = (DBCONTACTWRITESETTING*)lParam;

	if (hContact == NULL) {
		if (( 0 == _stricmp( pdbcws->szModule, "CLC" )) || ( 0 == _stricmp( pdbcws->szModule, MODULE )) ) {
			LoadDBSettings();
			ApplyOptionsChanges();
		}

		return 0;
	}

	if ( pThumb == NULL )
		return 0;

	// Only on these 2 events we need to refresh
	if (0 == _stricmp(pdbcws->szSetting, "Status"))
		idStatus = pdbcws->value.wVal;
	else if (0 == _stricmp(pdbcws->szSetting, "Nick"))
		idStatus = GetContactStatus(hContact);
	else if (0 == _stricmp(pdbcws->szSetting, "MyHandle"))
		idStatus = GetContactStatus(hContact);
	else if (fcOpt.bShowIdle && 0 == _stricmp( pdbcws->szSetting, "IdleTS"))
		idStatus = GetContactStatus(hContact);
	else
		bRefresh = FALSE;

	// Detach call
	if (bRefresh)
		PostMessage(pThumb->hwnd, WM_REFRESH_CONTACT, 0, idStatus);

	return 0;
}

static int OnStatusModeChange(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < thumbList.getCount(); ++i) {
		int idStatus = GetContactStatus(thumbList[i].hContact);
		thumbList[i].RefreshContactStatus(idStatus);
	}

	if (wParam == ID_STATUS_OFFLINE)
	{
		// Floating status window will use this
	}

	return 0;
}

static int OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(wParam);

	Menu_ShowItem(hMenuItemRemove, pThumb != NULL);
	Menu_ShowItem(hMenuItemHideAll, !fcOpt.bHideAll);
	return 0;
}

///////////////////////////////////////////////////////
// General functions

static void LoadDBSettings()
{
	fcOpt.thumbAlpha = (BYTE)((double)db_get_b( NULL, MODULE, "Opacity", 100 ) * 2.55);
	fcOpt.bHideOffline = (BOOL)db_get_b( NULL, MODULE, "HideOffline", 0);
	fcOpt.bHideAll = (BOOL)db_get_b( NULL, MODULE, "HideAll", 0);
	fcOpt.bHideWhenFullscreen = (BOOL)db_get_b( NULL, MODULE, "HideWhenFullscreen", 0);
	fcOpt.bMoveTogether = (BOOL)db_get_b( NULL, MODULE, "MoveTogether", 0);
	fcOpt.bFixedWidth = (BOOL)db_get_b( NULL, MODULE, "FixedWidth", 0);
	fcOpt.nThumbWidth = (DWORD)db_get_dw( NULL, MODULE, "Width", 0);
	dwOfflineModes = db_get_dw( NULL, "CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES);
	fcOpt.bShowTip = (BOOL)db_get_b( NULL, MODULE, "ShowTip", 1);
	fcOpt.TimeIn = (WORD)db_get_w( NULL, MODULE, "TimeIn", 0);
	fcOpt.bToTop = (BOOL)db_get_b( NULL, MODULE, "ToTop", 0);
	fcOpt.ToTopTime = (WORD)db_get_w( NULL, MODULE, "ToTopTime", TOTOPTIME_DEF);
	fcOpt.bHideWhenCListShow = (BOOL)db_get_b( NULL, MODULE, "HideWhenCListShow", 0);
	fcOpt.bUseSingleClick = (BOOL)db_get_b( NULL, MODULE, "UseSingleClick", 0);
	fcOpt.bShowIdle = (BOOL)db_get_b( NULL, MODULE, "ShowIdle", 0);

	if (db_get_b(NULL, "ModernData", "HideBehind", 0))
		bIsCListShow = (db_get_b(NULL, "ModernData", "BehindEdge", 0) == 0);
	else
		bIsCListShow = (db_get_b(NULL, "CList", "State", 0) == 2);
}

void SendMsgDialog( HWND hwnd, TCHAR *pText )
{
	ThumbInfo *pThumb = thumbList.FindThumb( hwnd );
	if ( pThumb != NULL )
		CallService(MS_MSG_SENDMESSAGET, (WPARAM)pThumb->hContact, (LPARAM)pText );
}

static void ShowContactMenu( HWND hwnd, POINT pt )
{
	ThumbInfo *pThumb = thumbList.FindThumb( hwnd );
	if ( pThumb != NULL ) {
		hContactMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)pThumb->hContact, 0);
		if ( hContactMenu == NULL )
			return;

		int idCommand = TrackPopupMenu( hContactMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0 , hwnd, NULL );
		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM( idCommand , MPCF_CONTACTMENU), (LPARAM)pThumb->hContact );
	}
}

static LRESULT __stdcall CommWndProc(HWND	hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumb(hwnd);

	switch(uMsg) {
	case WM_RBUTTONUP:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			if (pThumb) pThumb->ThumbDeselect(TRUE);

			ClientToScreen(hwnd, &pt);
			ShowContactMenu( hwnd, pt );
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_NCPAINT:
		if (pThumb) {
			HDC hdc = GetWindowDC( hwnd );
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow( hwnd, hdc );
			ReleaseDC( hwnd, hdc );
			ValidateRect( hwnd, NULL );
			return 0;
		}

	case WM_PAINT:
		if (pThumb) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow( hwnd, hdc );
			EndPaint(hwnd, &ps);
			break;
		}

	case WM_PRINT:
	case WM_PRINTCLIENT:
		if (pThumb) {
			BitBlt((HDC)wParam, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow(hwnd, (HDC)wParam);
			break;
		}

	case WM_MEASUREITEM:
		CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		break;

	case WM_DRAWITEM:
		CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		if (pThumb) pThumb->OnLButtonDown( LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEMOVE:
		if (pThumb) pThumb->OnMouseMove( LOWORD(lParam), HIWORD(lParam), wParam);
		break;

	case WM_LBUTTONUP:
		if (pThumb) pThumb->OnLButtonUp();
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
			_tcsncpy( pThumb->ptszName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)pThumb->hContact, (LPARAM)GCDNF_TCHAR ), USERNAME_LEN - 1);
			pThumb->RefreshContactStatus((int)lParam);
			pThumb->ResizeThumb();
		}
		break;

	case WM_TIMER:
		if (pThumb)
			pThumb->OnTimer(wParam);
		break;

	case WM_HOTKEY:
		ThumbInfo *pThumb = thumbList.FindThumb((HWND)wParam);
		if (pThumb)
			pThumb->PopupMessageDialog();
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

extern void SetThumbsOpacity( BYTE btAlpha )
{
	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i].SetThumbOpacity(btAlpha);
}

static void GetScreenRect()
{
	rcScreen.left = GetSystemMetrics( SM_XVIRTUALSCREEN );
	rcScreen.top = GetSystemMetrics( SM_YVIRTUALSCREEN );
	rcScreen.right = GetSystemMetrics( SM_CXVIRTUALSCREEN ) + rcScreen.left;
	rcScreen.bottom = GetSystemMetrics( SM_CYVIRTUALSCREEN ) + rcScreen.top;
}

void OnStatusChanged()
{
	int idStatus = ID_STATUS_OFFLINE;

	for (int i = 0; i < thumbList.getCount(); ++i) {
		idStatus = GetContactStatus(thumbList[i].hContact);
		thumbList[i].RefreshContactStatus(idStatus);
	}
}

void ApplyOptionsChanges()
{
	CreateThumbsFont();
	CreateBackgroundBrush();

	if (!fcOpt.bToTop && ToTopTimerID) {
		KillTimer(NULL, ToTopTimerID);
		ToTopTimerID = 0;
	}

	if (fcOpt.bToTop) {
		if (ToTopTimerID) KillTimer(NULL, ToTopTimerID);
		fcOpt.ToTopTime = (fcOpt.ToTopTime<1)?1:fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime>TOTOPTIME_MAX)?TOTOPTIME_MAX:fcOpt.ToTopTime;
		ToTopTimerID = SetTimer(NULL, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}

	OnStatusChanged();

	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i].ResizeThumb();
}

///////////////////////////////////////////////////////
// Window creation

static void RegisterWindowClass()
{
	WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wcx.lpfnWndProc = CommWndProc;
	wcx.hInstance = hInst;
	wcx.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wcx.lpszClassName	= WND_CLASS;
	RegisterClassEx(&wcx);
}

static void UnregisterWindowClass()
{
	UnregisterClass(WND_CLASS, hInst);
}

static void CreateThumbWnd(TCHAR *ptszName, MCONTACT hContact, int nX, int nY)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb != NULL)
		return;

	// Prepare for window creation
	HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, WND_CLASS, ptszName, WS_POPUP, nX, nY, 50, 20, NULL, NULL, hInst, NULL);
	if (hwnd == NULL)
		return;

	pThumb = thumbList.AddThumb(hwnd, ptszName, hContact);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThumb);
	pThumb->ResizeThumb();

	pThumb->SetThumbOpacity(fcOpt.thumbAlpha);
	ShowWindow( hwnd, (fcOpt.bHideAll || HideOnFullScreen() || fcOpt.bHideOffline || (fcOpt.bHideWhenCListShow && bIsCListShow)) ? SW_HIDE : SW_SHOWNA);
	pThumb->PositionThumb(nX, nY);

	// force repaint
	pThumb->UpdateContent();
}

static void CreateThumbsFont()
{
	for (int nFontId = 0; nFontId < FLT_FONTIDS; nFontId++) {
		if (NULL != hFont[nFontId]) {
			DeleteObject(hFont[nFontId]);
			hFont[nFontId] = NULL;
		}

		LOGFONT lf;
		FontService_GetFont( LPGENT("Floating contacts"), s_fonts[nFontId], &tColor[nFontId], &lf);

		HDC hdc = GetDC(NULL);
		LONG height = lf.lfHeight;
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(NULL, hdc);

		hFont[nFontId] = CreateFontIndirect(&lf);

		lf.lfHeight = height;
	}
}
									 
static void CreateBackgroundBrush()
{
	bkColor = db_get_dw(NULL, MODULE, "BkColor", FLT_DEFAULT_BKGNDCOLOR);

	if (NULL != hLTEdgesPen) {
		DeleteObject(hLTEdgesPen);
		hLTEdgesPen = NULL;
	}

	if (NULL != hRBEdgesPen) {
		DeleteObject(hRBEdgesPen);
		hRBEdgesPen = NULL;
	}

	if (NULL != hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = NULL;
	}

	if (NULL != hBkBrush) {
		SetClassLong((HWND)WND_CLASS, GCLP_HBRBACKGROUND, (LONG)NULL);
		DeleteObject( hBkBrush );
		hBkBrush = NULL;
	}

	if ( db_get_b(NULL, MODULE, "DrawBorder", FLT_DEFAULT_DRAWBORDER)) {
		COLORREF cr = (COLORREF)db_get_dw(NULL, MODULE, "LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR);
		hLTEdgesPen = CreatePen(PS_SOLID, 1, cr);
		cr = (COLORREF)db_get_dw(NULL, MODULE, "RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR);
		hRBEdgesPen = CreatePen(PS_SOLID, 1, cr);
	}
	
	if (db_get_b(NULL, MODULE, "BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP)) {
		DBVARIANT dbv;
		if ( !db_get_ts(NULL, MODULE, "BkBitmap", &dbv)) {
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (LPARAM)dbv.ptszVal);
			db_free(&dbv);
		}
	}
	nBackgroundBmpUse = (WORD)db_get_w(NULL, MODULE, "BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);

	// Create brush
	hBkBrush	 = CreateSolidBrush(bkColor);

	// Attach brush to the window
	SetClassLong((HWND)WND_CLASS, GCLP_HBRBACKGROUND, (LONG)hBkBrush);
}

static int GetContactStatus(MCONTACT hContact)
{
	if (hContact == NULL) {
		assert( !"Contact is NULL" );
		return 0;
	}

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return ID_STATUS_OFFLINE;

	return db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}


static int ClcStatusToPf2(int status)
{
	switch(status) {
		case ID_STATUS_ONLINE: return PF2_ONLINE;
		case ID_STATUS_AWAY: return PF2_SHORTAWAY;
		case ID_STATUS_DND: return PF2_HEAVYDND;
		case ID_STATUS_NA: return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
		case ID_STATUS_FREECHAT: return PF2_FREECHAT;
		case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
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
	char szBuf[MAX_PATH] = {0};

	DBVARIANT dbv;
	if (db_get_s(hContact, MODULE, "Hotkey", &dbv)) return;
	strncpy(szBuf, dbv.pszVal, MAX_PATH - 1);
	db_free( &dbv );

	if (szBuf[0] !=  '\0') {
		UINT nModifiers = 0;
		char chKey = 0;
		char szMod[2][20] = {0};
		char szKey[20] = {0};

		sscanf( szBuf, "%[^'+']+%[^'+']+%[^'+']", szMod[0], szMod[1], szKey );

		for (int i=0; i < 2; i++) {
			if ( 0 == strncmp( szMod[ i ], "ALT", 19 ))
				nModifiers = nModifiers | MOD_ALT;
			else if ( 0 == strncmp( szMod[ i ], "CTRL", 19 ))
				nModifiers = nModifiers | MOD_CONTROL;
			else if ( 0 == strncmp( szMod[ i ], "SHIFT", 19 ))
				nModifiers = nModifiers | MOD_SHIFT;
		}

		chKey = szKey[ 0 ];

		RegisterHotKey( hwnd, (int)hwnd, nModifiers, VkKeyScan( chKey ));
	}
}

///////////////////////////////////////////////////////
// Contact settings

void SaveContactsPos()
{
	for (int i = 0; i < thumbList.getCount(); ++i) {
		SetLastError( 0);

		RECT rc;
		thumbList[i].GetThumbRect(&rc);

		if (0 == GetLastError())
			db_set_dw(thumbList[i].hContact, MODULE, "ThumbsPos", DB_POS_MAKE_XY(rc.left, rc.top));
	}
}

static void LoadContacts()
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		LoadContact(hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menus

static INT_PTR OnMainMenu_HideAll(WPARAM wParam, LPARAM lParam)
{
	fcOpt.bHideAll = !fcOpt.bHideAll;
	db_set_b(NULL, MODULE, "HideAll", (BYTE)fcOpt.bHideAll);
	OnStatusChanged();

	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	clmi.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(fcOpt.bHideAll ? IDI_SHOW : IDI_HIDE));
	clmi.ptszName = fcOpt.bHideAll ? LPGENT("Show all thumbs") : LPGENT("Hide all thumbs");
	Menu_ModifyItem(hMainMenuItemHideAll, &clmi);
	return 0;
}

static INT_PTR OnContactMenu_Remove(WPARAM hContact, LPARAM lParam)
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (pThumb) {
		pThumb->DeleteContactPos();
		thumbList.RemoveThumb(pThumb);
	}

	DestroyMenu(hContactMenu);
	return 0;
}

static INT_PTR OnHotKey_HideWhenCListShow(WPARAM wParam, LPARAM lParam)
{
	fcOpt.bHideWhenCListShow = !fcOpt.bHideWhenCListShow;
	db_set_b(NULL, MODULE, "HideWhenCListShow", (BYTE)fcOpt.bHideWhenCListShow);
	OnStatusChanged();
	return 0;
}

static void LoadMenus()
{
	// Remove thumb menu item
	CreateServiceFunction(MODULE "/RemoveThumb", OnContactMenu_Remove);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 0xFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_HIDE));
	mi.ptszName = LPGENT("Remove thumb");
	mi.pszService = MODULE "/RemoveThumb";
	hMenuItemRemove = Menu_AddContactMenuItem(&mi);

	// Hide all thumbs main menu item
	CreateServiceFunction(MODULE "/MainHideAllThumbs", OnMainMenu_HideAll );

	mi.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( fcOpt.bHideAll ? IDI_SHOW : IDI_HIDE ));
	mi.ptszName = fcOpt.bHideAll ? LPGENT("Show all thumbs") : LPGENT("Hide all thumbs");
	mi.pszService = MODULE "/MainHideAllThumbs";
	Menu_AddMainMenuItem(&mi);

	// Register hotkeys
	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszSection = "Floating Contacts";

	hkd.pszName = MODULE "/MainHideAllThumbs";
	hkd.pszDescription = LPGEN("Show/Hide all thumbs");
	hkd.pszService = MODULE "/MainHideAllThumbs";
	Hotkey_Register(&hkd);

	CreateServiceFunction(MODULE "/HideWhenCListShow", OnHotKey_HideWhenCListShow );
	hkd.pszName = MODULE "/HideWhenCListShow";
	hkd.pszDescription = LPGEN("Hide when contact list is shown");
	hkd.pszService = MODULE "/HideWhenCListShow";
	Hotkey_Register(&hkd);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void LoadContact( MCONTACT hContact )
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact(hContact);
	if (hContact == NULL)
		return;

	DWORD	dwPos = db_get_dw( hContact, MODULE, "ThumbsPos", (DWORD)-1);
	if ( dwPos != -1) {
		TCHAR	*ptName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, (LPARAM)GCDNF_TCHAR);
		if ( ptName != NULL ) {
			int nX = DB_POS_GETX( dwPos );
			int nY = DB_POS_GETY( dwPos );

			CreateThumbWnd(ptName, hContact, nX, nY);
			pThumb->PositionThumb(nX, nY);
		}
	}
}

BOOL HideOnFullScreen()
{
	BOOL bFullscreen = FALSE;
	HWND hWnd = 0;

	if ( fcOpt.bHideWhenFullscreen ) {
		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);

		hWnd = GetForegroundWindow();

		while (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
			RECT WindowRect;
			GetWindowRect(hWnd, &WindowRect);

			if ( w == WindowRect.right - WindowRect.left && h == WindowRect.bottom - WindowRect.top) {
				bFullscreen = TRUE;
				break;
			}

			hWnd = GetNextWindow( hWnd, GW_HWNDNEXT );
		}
	}

	return bFullscreen && fcOpt.bHideWhenFullscreen;
}

static VOID CALLBACK ToTopTimerProc ( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	for (int i = 0; i < thumbList.getCount(); ++i)
		SetWindowPos(thumbList[i].hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

void ShowThumbsOnHideCList()
{
	if (!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())
		return;

	for (int i = 0; i < thumbList.getCount(); ++i)
		if ( !fcOpt.bHideOffline || IsStatusVisible( GetContactStatus(thumbList[i].hContact)))
			ShowWindow(thumbList[i].hwnd, SW_SHOWNA);
}


void HideThumbsOnShowCList()
{
	if (!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())
		return;

	for (int i = 0; i < thumbList.getCount(); ++i)
		ShowWindow(thumbList[i].hwnd, SW_HIDE);
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
			BYTE method = db_get_b(NULL, "ModernData", "HideBehind", 0);
			if (method) {
				WORD wBehindEdgeBorderSize = db_get_w(NULL, "ModernData", "HideBehindBorderSize", 0);
				RECT rc = {wp->x, wp->y, wp->x+wp->cx, wp->y+wp->cy};
				RECT rcScreen = {wBehindEdgeBorderSize*(2-method), 0, GetSystemMetrics(SM_CXSCREEN)-wBehindEdgeBorderSize*(method-1), GetSystemMetrics(SM_CYSCREEN)};
				RECT rcOverlap;
				BOOL isIntersect;

				isIntersect = IntersectRect( &rcOverlap, &rc, &rcScreen );
				if ( !isIntersect && bIsCListShow ) {
					bIsCListShow = FALSE;
					ShowThumbsOnHideCList();
				}
				else if ( isIntersect && !bIsCListShow ) {
					bIsCListShow = TRUE;
					HideThumbsOnShowCList();
				}
			}
		}
	}
	return mir_callNextSubclass(hwnd, newMirandaWndProc, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnModulesLoded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_CLIST_CONTACTICONCHANGED, OnContactIconChanged);
	HookEvent(ME_SKIN_ICONSCHANGED, OnSkinIconsChanged);
	HookEvent(ME_CLUI_CONTACTDRAGGING, OnContactDrag);
	HookEvent(ME_CLUI_CONTACTDROPPED, OnContactDrop);
	HookEvent(ME_CLUI_CONTACTDRAGSTOP, OnContactDragStop);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialize);
	HookEvent(ME_CLIST_STATUSMODECHANGE, OnStatusModeChange);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	
	hwndMiranda = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	mir_subclassWindow(hwndMiranda, newMirandaWndProc);

	// No thumbs yet
	bEnableTip = ServiceExists("mToolTip/ShowTip");

	RegisterWindowClass();
	InitFileDropping();
	GetScreenRect();
	LoadDBSettings();
	CreateBackgroundBrush();
	CreateThumbsFont();
	LoadContacts();
	LoadMenus();

	if (fcOpt.bToTop) {
		fcOpt.ToTopTime = (fcOpt.ToTopTime<1)?1:fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime>TOTOPTIME_MAX)?TOTOPTIME_MAX:fcOpt.ToTopTime;
		ToTopTimerID = SetTimer(NULL, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}
	return 0;
}

static int OnPreshutdown(WPARAM, LPARAM)
{
	while (thumbList.getCount() > 0)
		thumbList.RemoveThumb(&thumbList[0]);
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	InitOptions();

	for (int i=0; i < SIZEOF(s_fonts); i++) {
		LOGFONT lf;
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
		if (i == FLT_FONTID_OFFINVIS || i == FLT_FONTID_INVIS)
			lf.lfItalic = !lf.lfItalic;

		COLORREF defColor = GetSysColor((i == FLT_FONTID_NOTONLIST) ? COLOR_3DSHADOW : COLOR_WINDOWTEXT);

		char szId[20];
		mir_snprintf(szId, SIZEOF(szId), "Font%d", i);
		FontService_RegisterFont(MODULE, szId, LPGENT("Floating contacts"), s_fonts[i], i+1, false, &lf, defColor);
	}

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreshutdown);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload()
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
		hBkBrush = NULL;
	}

	for (int nFontId = 0; nFontId < FLT_FONTIDS; nFontId++)
		if (NULL != hFont[nFontId])
			DeleteObject(hFont[nFontId]);

	UnregisterWindowClass();

	FreeFileDropping();
	return 0;
}
