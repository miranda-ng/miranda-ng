#include <commonheaders.h>
#include "cluiframes.h"

//========================== Frames
HANDLE hFrameMenuObject;
static HANDLE hPreBuildFrameMenuEvent;

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct{
	char *szServiceName;
	int Frameid;
	INT_PTR param1;
} FrameMenuExecParam, *lpFrameMenuExecParam;

INT_PTR FreeOwnerDataFrameMenu(WPARAM wParam, LPARAM lParam)
{
	lpFrameMenuExecParam cmep = (lpFrameMenuExecParam)lParam;
	if (cmep != NULL){
		mir_free(cmep->szServiceName);
		mir_free(cmep);
	}
	return 0;
}

static INT_PTR AddContextFrameMenuItem(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM *)lParam;

	TMO_MenuItem tmi;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return NULL;

	tmi.root = (mi->flags & CMIF_ROOTHANDLE) ? mi->hParentMenu : NULL;

	lpFrameMenuExecParam fmep = (lpFrameMenuExecParam)mir_alloc(sizeof(FrameMenuExecParam));
	if (fmep == NULL)
		return 0;

	fmep->szServiceName = mir_strdup(mi->pszService);
	fmep->Frameid = mi->popupPosition;
	fmep->param1 = (INT_PTR)mi->pszContactOwner;
	tmi.ownerdata = fmep;

	return CallService(MO_ADDNEWMENUITEM, (WPARAM)hFrameMenuObject, (LPARAM)&tmi);
}

static INT_PTR RemoveContextFrameMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM, wParam, 0);
	return 0;
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
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

//true - ok,false ignore
INT_PTR FrameMenuCheckService(WPARAM wParam, LPARAM lParam)
{
	PCheckProcParam pcpp = (PCheckProcParam)wParam;
	if (pcpp == NULL)
		return FALSE;

	TMO_MenuItem mi;
	if (CallService(MO_GETMENUITEM, (WPARAM)pcpp->MenuItemHandle, (LPARAM)&mi) == 0) {
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
	ListParam param = {0};
	param.MenuObjectHandle = hFrameMenuObject;
	param.wParam = wParam;
	param.lParam = lParam;

	HMENU hMenu = CreatePopupMenu();
	ContextFrameMenuNotify(wParam, -1);
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	return (INT_PTR)hMenu;
}

//========================== Frames end

int InitFramesMenus(void)
{
	CreateServiceFunction("FrameMenuExecService", FrameMenuExecService);
	CreateServiceFunction("FrameMenuCheckService", FrameMenuCheckService);
	CreateServiceFunction("FrameMenuFreeService", FreeOwnerDataFrameMenu);

	CreateServiceFunction(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM, RemoveContextFrameMenuItem);
	CreateServiceFunction("CList/AddContextFrameMenuItem", AddContextFrameMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDFRAMECONTEXT, BuildContextFrameMenu);
	CreateServiceFunction(MS_CLIST_FRAMEMENUNOTIFY, ContextFrameMenuNotify);
	hPreBuildFrameMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDFRAMEMENU);

	// frame menu object
	hFrameMenuObject = MO_CreateMenuObject("FrameMenu", LPGEN("Frame menu"), "FrameMenuCheckService", "FrameMenuExecService");
	MO_SetMenuObjectParam(hFrameMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "FrameMenuFreeService");
	return 0;
}
