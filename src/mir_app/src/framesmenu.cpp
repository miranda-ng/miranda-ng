#include "stdafx.h"

#include <m_cluiframes.h>

//========================== Frames
HANDLE hFrameMenuObject;
static HANDLE hPreBuildFrameMenuEvent;

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct{
	char *szServiceName;
	int Frameid;
	INT_PTR param1;
}FrameMenuExecParam, *lpFrameMenuExecParam;

INT_PTR FreeOwnerDataFrameMenu(WPARAM, LPARAM lParam)
{
	lpFrameMenuExecParam cmep = (lpFrameMenuExecParam)lParam;
	if (cmep != NULL){
		mir_free(cmep->szServiceName);
		mir_free(cmep);
	}
	return 0;
}

static INT_PTR AddContextFrameMenuItem(WPARAM, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;

	TMO_MenuItem tmi;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return NULL;

	tmi.root = (mi->flags & CMIF_ROOTHANDLE) ? mi->hParentMenu : NULL;

	lpFrameMenuExecParam fmep = (lpFrameMenuExecParam)mir_alloc(sizeof(FrameMenuExecParam));
	if (fmep == NULL)
		return 0;

	fmep->szServiceName = mir_strdup(mi->pszService);
	fmep->Frameid = 0; // mi->popupPosition; !!!!!!!!!!!!!!!!!!!!!!!!!!
	fmep->param1 = (INT_PTR)mi->pszContactOwner;
	tmi.ownerdata = fmep;
	return (INT_PTR)Menu_AddItem(hFrameMenuObject, &tmi);
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR FrameMenuExecService(WPARAM wParam, LPARAM lParam)
{
	lpFrameMenuExecParam fmep = (lpFrameMenuExecParam)wParam;
	if (fmep == NULL)
		return -1;

	CallService(fmep->szServiceName, lParam, fmep->param1);
	return 0;
}

//true - ok,false ignore
INT_PTR FrameMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == NULL)
		return FALSE;

	TMO_MenuItem mi;
	if (Menu_GetItemInfo(pcpp->MenuItemHandle, mi) == 0) {
		lpFrameMenuExecParam fmep = (lpFrameMenuExecParam)mi.ownerdata;
		if (fmep != NULL) {
			//pcpp->wParam  -  frameid
			if (((WPARAM)fmep->Frameid == pcpp->wParam) || fmep->Frameid == -1)
				return TRUE;
		}
	}
	return FALSE;
}

static INT_PTR ContextFrameMenuNotify(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hPreBuildFrameMenuEvent, wParam, lParam);
	return 0;
}

static INT_PTR BuildContextFrameMenu(WPARAM wParam, LPARAM lParam)
{
	ContextFrameMenuNotify(wParam, -1);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hFrameMenuObject, wParam, lParam);
	return (INT_PTR)hMenu;
}

//========================== Frames end

int InitFramesMenus(void)
{
	CreateServiceFunction("FrameMenuExecService", FrameMenuExecService);
	CreateServiceFunction("FrameMenuCheckService", FrameMenuCheckService);
	CreateServiceFunction("FrameMenuFreeService", FreeOwnerDataFrameMenu);

	CreateServiceFunction("CList/AddContextFrameMenuItem", AddContextFrameMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDFRAMECONTEXT, BuildContextFrameMenu);
	CreateServiceFunction(MS_CLIST_FRAMEMENUNOTIFY, ContextFrameMenuNotify);
	hPreBuildFrameMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDFRAMEMENU);

	// frame menu object
	hFrameMenuObject = Menu_AddObject("FrameMenu", LPGEN("Frame menu"), "FrameMenuCheckService", "FrameMenuExecService");
	Menu_ConfigureObject(hFrameMenuObject, MCO_OPT_FREE_SERVICE, "FrameMenuFreeService");
	return 0;
}
