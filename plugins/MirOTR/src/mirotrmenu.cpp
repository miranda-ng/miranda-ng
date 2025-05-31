#include "stdafx.h"

static int hMirOTRMenuObject;
static HGENMENU hStatusInfoItem, hHTMLConvMenuItem;
HWND hDummyPaintWin;

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct
{
	char *szServiceName;
}
MirOTRMenuExecParam, *lpMirOTRMenuExecParam;

////////////////////////////////////////////
// MirOTR MENU
///////////////////////////////////////////

static HGENMENU AddMirOTRMenuItem(TMO_MenuItem *pmi, const char *pszService)
{
	// add owner data
	lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)mir_calloc(sizeof(MirOTRMenuExecParam));
	cmep->szServiceName = mir_strdup(pszService);
	return Menu_AddItem(hMirOTRMenuObject, pmi, cmep);
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR MirOTRMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)wParam;
		//call with wParam=(MCONTACT)hContact
		CallService(cmep->szServiceName, lParam, 0);
	}
	return 0;
}

// true - ok,false ignore
INT_PTR MirOTRMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == nullptr)
		return FALSE;

	lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)pcpp->MenuItemOwnerData;
	if (cmep == nullptr) //this is rootsection...build it
		return TRUE;

	MCONTACT hContact = (MCONTACT)pcpp->wParam, hSub;
	if ((hSub = db_mc_getMostOnline(hContact)) != 0)
		hContact = hSub;

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	TrustLevel level = (TrustLevel)otr_context_get_trust(context);

	TMO_MenuItem mi;
	if (Menu_GetItemInfo(pcpp->MenuItemHandle, mi) == 0) {
		if (mi.flags & CMIF_HIDDEN) return FALSE;
		if (mi.flags & CMIF_NOTPRIVATE  && level == TRUST_PRIVATE) return FALSE;
		if (mi.flags & CMIF_NOTFINISHED && level == TRUST_FINISHED) return FALSE;
		if (mi.flags & CMIF_NOTUNVERIFIED  && level == TRUST_UNVERIFIED) return FALSE;
		if (mi.flags & CMIF_NOTNOTPRIVATE && level == TRUST_NOT_PRIVATE) return FALSE;

		if (pcpp->MenuItemHandle == hStatusInfoItem) {
			wchar_t text[128];

			switch (level) {
			case TRUST_PRIVATE:
				mir_snwprintf(text, L"%s [v%i]", TranslateW(LANG_STATUS_PRIVATE), context->protocol_version);
				Menu_ModifyItem(hStatusInfoItem, text, iconList[ICON_PRIVATE].hIcolib);
				break;

			case TRUST_UNVERIFIED:
				mir_snwprintf(text, L"%s [v%i]", TranslateW(LANG_STATUS_UNVERIFIED), context->protocol_version);
				Menu_ModifyItem(hStatusInfoItem, text, iconList[ICON_UNVERIFIED].hIcolib);
				break;

			case TRUST_FINISHED:
				Menu_ModifyItem(hStatusInfoItem, TranslateW(LANG_STATUS_FINISHED), iconList[ICON_UNVERIFIED].hIcolib);
				break;

			default:
				Menu_ModifyItem(hStatusInfoItem, TranslateW(LANG_STATUS_DISABLED), iconList[ICON_NOT_PRIVATE].hIcolib);
			}
		}
		else if (pcpp->MenuItemHandle == hHTMLConvMenuItem)
			Menu_SetChecked(hHTMLConvMenuItem, g_plugin.getByte(hContact, "HTMLConv", 0) != 0);
	}
	return TRUE;
}

INT_PTR FreeOwnerDataMirOTRMenu(WPARAM, LPARAM lParam)
{
	lpMirOTRMenuExecParam cmep = (lpMirOTRMenuExecParam)lParam;
	if (cmep != nullptr) {
		if (cmep->szServiceName) mir_free(cmep->szServiceName);
		mir_free(cmep);
	}
	return 0;
}

LRESULT CALLBACK PopupMenuWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_MEASUREITEM:
		if (Menu_MeasureItem(lParam)) return TRUE;
		break;
	case WM_DRAWITEM:
		if (Menu_DrawItem(lParam)) return TRUE;
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
	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hMirOTRMenuObject, hContact);

	SetWindowLongPtr(hDummyPaintWin, GWLP_USERDATA, (LONG_PTR)hContact);

	TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hDummyPaintWin, nullptr);
	DestroyMenu(hMenu);
}

void InitMirOTRMenu(void)
{
	WNDCLASS wc = { 0 };
	wc.hInstance = g_plugin.getInst();
	wc.lpfnWndProc = PopupMenuWndProc;
	wc.lpszClassName = L"MirOTRPopupMenuProcessor";
	RegisterClass(&wc);
	hDummyPaintWin = CreateWindowEx(0, L"MirOTRPopupMenuProcessor", nullptr, 0, 0, 0, 1, 1, nullptr, nullptr, g_plugin.getInst(), nullptr);

	CreateServiceFunction("MirOTRMenuExecService", MirOTRMenuExecService);
	CreateServiceFunction("MirOTRMenuCheckService", MirOTRMenuCheckService);

	// menu object
	CreateServiceFunction("MIROTRMENUS/FreeOwnerDataMirOTRMenu", FreeOwnerDataMirOTRMenu);

	hMirOTRMenuObject = Menu_AddObject("MirOTRMenu", LPGEN("MirOTR menu"), "MirOTRMenuCheckService", "MirOTRMenuExecService");
	Menu_ConfigureObject(hMirOTRMenuObject, MCO_OPT_FREE_SERVICE, "MIROTRMENUS/FreeOwnerDataMirOTRMenu");

	// menu items
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_DISABLED;
	mi.name.a = LPGEN("OTR Status");
	mi.position = 0;
	hStatusInfoItem = AddMirOTRMenuItem(&mi, nullptr);

	mi.flags = CMIF_NOTPRIVATE | CMIF_NOTUNVERIFIED;
	mi.name.a = LPGEN("&Start OTR session");
	mi.position = 100001;
	mi.hIcolibItem = iconList[ICON_UNVERIFIED].hIcolib;
	AddMirOTRMenuItem(&mi, MS_OTR_MENUSTART);

	mi.flags = CMIF_NOTNOTPRIVATE | CMIF_NOTFINISHED;
	mi.name.a = LPGEN("&Refresh OTR session");
	mi.position = 100002;
	mi.hIcolibItem = iconList[ICON_REFRESH].hIcolib;
	AddMirOTRMenuItem(&mi, MS_OTR_MENUREFRESH);

	mi.flags = CMIF_NOTNOTPRIVATE;
	mi.name.a = LPGEN("Sto&p OTR session");
	mi.position = 100003;
	mi.hIcolibItem = iconList[ICON_NOT_PRIVATE].hIcolib;
	AddMirOTRMenuItem(&mi, MS_OTR_MENUSTOP);

	mi.flags = CMIF_NOTNOTPRIVATE | CMIF_NOTFINISHED;
	mi.name.a = LPGEN("&Verify Fingerprint");
	mi.position = 200001;
	mi.hIcolibItem = iconList[ICON_OTR].hIcolib;
	AddMirOTRMenuItem(&mi, MS_OTR_MENUVERIFY);

	mi.flags = CMIF_CHECKED;
	mi.name.a = LPGEN("&Convert HTML (for Pidgin)");
	mi.position = 300001;
	mi.hIcolibItem = nullptr;
	hHTMLConvMenuItem = AddMirOTRMenuItem(&mi, MS_OTR_MENUTOGGLEHTML);
}

void UninitMirOTRMenu(void)
{
	DestroyWindow(hDummyPaintWin);
	hDummyPaintWin = nullptr;

	UnregisterClass(L"MirOTRPopupMenuProcessor", g_plugin.getInst());

	Menu_RemoveObject(hMirOTRMenuObject);
	hMirOTRMenuObject = 0;
}
