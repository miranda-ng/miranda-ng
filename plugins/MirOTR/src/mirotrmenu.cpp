#include "stdafx.h"

static HANDLE hMirOTRMenuObject;
static HGENMENU hStatusInfoItem, hHTMLConvMenuItem;
HWND hDummyPaintWin;

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct
{
	char *szServiceName;
}
MirOTRMenuExecParam,*lpMirOTRMenuExecParam;

////////////////////////////////////////////
// MirOTR MENU
///////////////////////////////////////////

static INT_PTR RemoveMirOTRMenuItem(WPARAM wParam, LPARAM)
{
	Menu_RemoveItem((HGENMENU)wParam);
	return 0;
}

static INT_PTR AddMirOTRMenuItem(WPARAM, LPARAM lParam)
{
	MIROTRMENUITEM *mi=(MIROTRMENUITEM*)lParam;
	if ( mi->cbSize != sizeof( MIROTRMENUITEM ))
		return 0;

	TMO_MenuItem tmi = { 0 };
	tmi.flags = mi->flags;
	tmi.hIcon = mi->hIcon;
	tmi.hIcolibItem = mi->icolibItem;
	tmi.position = mi->position;
	tmi.name.t = mi->ptszName;
	tmi.root = mi->root;

	//owner data
	lpMirOTRMenuExecParam cmep = ( lpMirOTRMenuExecParam )mir_calloc(sizeof(MirOTRMenuExecParam));
	cmep->szServiceName = mir_strdup( mi->pszService );
	tmi.ownerdata = cmep;
	return (INT_PTR)Menu_AddItem(hMirOTRMenuObject, &tmi);
}

static INT_PTR BuildMirOTRMenu(WPARAM hContact, LPARAM)
{
	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hMirOTRMenuObject, hContact);
	return (INT_PTR)hMenu;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR MirOTRMenuExecService(WPARAM wParam,LPARAM lParam)
{
	if (wParam!=0) {
		lpMirOTRMenuExecParam cmep=(lpMirOTRMenuExecParam)wParam;
		//call with wParam=(MCONTACT)hContact
		CallService(cmep->szServiceName,lParam,0);
	}
	return 0;
}

// true - ok,false ignore
INT_PTR MirOTRMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == NULL)
		return FALSE;

	lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)pcpp->MenuItemOwnerData;
	if (cmep == NULL) //this is rootsection...build it
		return TRUE;

	MCONTACT hContact = (MCONTACT)pcpp->wParam, hSub;
	if ((hSub = db_mc_getMostOnline(hContact)) != 0)
		hContact = hSub;

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	TrustLevel level = (TrustLevel)otr_context_get_trust(context);

	TMO_MenuItem mi;
	if (CallService(MO_GETMENUITEM, (WPARAM)pcpp->MenuItemHandle, (LPARAM)&mi) == 0) {
		if (mi.flags & CMIF_HIDDEN) return FALSE;
		if (mi.flags & CMIF_NOTPRIVATE  && level == TRUST_PRIVATE) return FALSE;
		if (mi.flags & CMIF_NOTFINISHED && level == TRUST_FINISHED) return FALSE;
		if (mi.flags & CMIF_NOTUNVERIFIED  && level == TRUST_UNVERIFIED) return FALSE;
		if (mi.flags & CMIF_NOTNOTPRIVATE && level == TRUST_NOT_PRIVATE) return FALSE;

		if (pcpp->MenuItemHandle == hStatusInfoItem) {
			TCHAR text[128];

			switch (level) {
			case TRUST_PRIVATE:
				mir_sntprintf(text, _T("%s [v%i]"), TranslateT(LANG_STATUS_PRIVATE), context->protocol_version);
				Menu_ModifyItem(hStatusInfoItem, text, IcoLib_GetIconHandle(ICON_PRIVATE));
				break;

			case TRUST_UNVERIFIED:
				mir_sntprintf(text, _T("%s [v%i]"), TranslateT(LANG_STATUS_UNVERIFIED), context->protocol_version);
				Menu_ModifyItem(hStatusInfoItem, text, IcoLib_GetIconHandle(ICON_UNVERIFIED));
				break;

			case TRUST_FINISHED:
				Menu_ModifyItem(hStatusInfoItem, TranslateT(LANG_STATUS_FINISHED), IcoLib_GetIconHandle(ICON_UNVERIFIED));
				break;

			default:
				Menu_ModifyItem(hStatusInfoItem, TranslateT(LANG_STATUS_DISABLED), IcoLib_GetIconHandle(ICON_NOT_PRIVATE));
			}
		}
		else if (pcpp->MenuItemHandle == hHTMLConvMenuItem) {
			int flags = db_get_b(hContact, MODULENAME, "HTMLConv", 0) ? CMIF_CHECKED : 0;
			Menu_ModifyItem(hHTMLConvMenuItem, NULL, INVALID_HANDLE_VALUE, flags);
		}
	}
	return TRUE;
}

INT_PTR FreeOwnerDataMirOTRMenu(WPARAM, LPARAM lParam)
{
	lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)lParam;
	if (cmep != NULL) {
		if (cmep->szServiceName) mir_free(cmep->szServiceName);
		mir_free(cmep);
	}
	return 0;
}

INT_PTR OnAddMenuItemMirOTRMenu(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (!mii || mii->cbSize != sizeof(MENUITEMINFO))
		return 0;

	TMO_MenuItem mi;
	if (CallService(MO_GETMENUITEM, (WPARAM)lParam, (LPARAM)&mi) == 0) {
		if (mi.flags & CMIF_DISABLED) {
			mii->fMask |= MIIM_STATE;
			mii->fState |= MF_DISABLED;
		}
	}
	return 1;
}

LRESULT CALLBACK PopupMenuWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_MEASUREITEM:
		if (CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam)) return TRUE;
		break;
	case WM_DRAWITEM:
		if (CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam)) return TRUE;
		break;
	case WM_COMMAND:
		if (Menu_ProcessCommandById(wParam, GetWindowLongPtr(hwnd, GWLP_USERDATA)))
			return TRUE;
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ShowOTRMenu(MCONTACT hContact, POINT pt)
{
	HMENU menu = (HMENU)CallService(MS_MIROTR_MENUBUILDMIROTR, hContact, 0);
	SetWindowLongPtr(hDummyPaintWin, GWLP_USERDATA, (LONG_PTR)hContact);
	TrackPopupMenu(menu, 0, pt.x, pt.y, 0, hDummyPaintWin, 0);
	DestroyMenu(menu);
}

void InitMirOTRMenu(void)
{
	WNDCLASS wc = { 0 };
	wc.hInstance = hInst;
	wc.lpfnWndProc = PopupMenuWndProc;
	wc.lpszClassName = _T("MirOTRPopupMenuProcessor");
	RegisterClass(&wc);
	hDummyPaintWin = CreateWindowEx(0, _T("MirOTRPopupMenuProcessor"), NULL, 0, 0, 0, 1, 1, 0, 0, hInst, 0);

	CreateServiceFunction("MirOTRMenuExecService", MirOTRMenuExecService);
	CreateServiceFunction("MirOTRMenuCheckService", MirOTRMenuCheckService);

	// menu object
	CreateServiceFunction("MIROTRMENUS/FreeOwnerDataMirOTRMenu", FreeOwnerDataMirOTRMenu);
	CreateServiceFunction("MIROTRMENUS/OnAddMenuItemMirOTRMenu", OnAddMenuItemMirOTRMenu);

	CreateServiceFunction(MS_MIROTR_ADDMIROTRMENUITEM, AddMirOTRMenuItem);
	CreateServiceFunction(MS_MIROTR_MENUBUILDMIROTR, BuildMirOTRMenu);
	CreateServiceFunction(MS_MIROTR_REMOVEMIROTRMENUITEM, RemoveMirOTRMenuItem);

	hMirOTRMenuObject = Menu_AddObject("MirOTRMenu", LPGEN("MirOTR menu"), "MirOTRMenuCheckService", "MirOTRMenuExecService");
	Menu_ConfigureObject(hMirOTRMenuObject, MCO_OPT_FREE_SERVICE, "MIROTRMENUS/FreeOwnerDataMirOTRMenu");
	Menu_ConfigureObject(hMirOTRMenuObject, MCO_OPT_ONADD_SERVICE, "MIROTRMENUS/OnAddMenuItemMirOTRMenu");

	// menu items
	MIROTRMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);

	mi.flags = CMIF_DISABLED | CMIF_TCHAR;
	mi.ptszName = LPGENT("OTR Status");
	mi.position = 0;
	hStatusInfoItem = (HGENMENU)AddMirOTRMenuItem(0, (LPARAM)&mi);

	mi.flags = CMIF_TCHAR | CMIF_NOTPRIVATE | CMIF_NOTUNVERIFIED;
	mi.ptszName = LANG_MENU_START;
	mi.position = 100001;
	mi.pszService = MS_OTR_MENUSTART;
	mi.icolibItem = IcoLib_GetIconHandle(ICON_UNVERIFIED);
	AddMirOTRMenuItem(0, (LPARAM)&mi);

	mi.flags = CMIF_TCHAR | CMIF_NOTNOTPRIVATE | CMIF_NOTFINISHED;
	mi.ptszName = LANG_MENU_REFRESH;
	mi.position = 100002;
	mi.pszService = MS_OTR_MENUREFRESH;
	mi.icolibItem = IcoLib_GetIconHandle(ICON_FINISHED);
	AddMirOTRMenuItem(0, (LPARAM)&mi);

	mi.flags = CMIF_TCHAR | CMIF_NOTNOTPRIVATE;
	mi.ptszName = LANG_MENU_STOP;
	mi.position = 100003;
	mi.pszService = MS_OTR_MENUSTOP;
	mi.icolibItem = IcoLib_GetIconHandle(ICON_NOT_PRIVATE);
	AddMirOTRMenuItem(0, (LPARAM)&mi);

	mi.flags = CMIF_TCHAR | CMIF_NOTNOTPRIVATE | CMIF_NOTFINISHED;
	mi.ptszName = LANG_MENU_VERIFY;
	mi.position = 200001;
	mi.pszService = MS_OTR_MENUVERIFY;
	mi.icolibItem = IcoLib_GetIconHandle(ICON_PRIVATE);
	AddMirOTRMenuItem(0, (LPARAM)&mi);

	mi.flags = CMIF_TCHAR | CMIF_CHECKED;
	mi.ptszName = LANG_MENU_TOGGLEHTML;
	mi.position = 300001;
	mi.pszService = MS_OTR_MENUTOGGLEHTML;
	hHTMLConvMenuItem = (HGENMENU)AddMirOTRMenuItem(0, (LPARAM)&mi);
}

void UninitMirOTRMenu(void)
{
	DestroyWindow(hDummyPaintWin);
	hDummyPaintWin = 0;

	UnregisterClass(_T("MirOTRPopupMenuProcessor"), hInst);
	
	Menu_RemoveObject(hMirOTRMenuObject);
	hMirOTRMenuObject = 0;
}
