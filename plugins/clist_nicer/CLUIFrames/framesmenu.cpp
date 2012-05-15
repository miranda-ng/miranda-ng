#include <commonheaders.h>
#include "cluiframes.h"

//==========================Frames
HANDLE hFrameMenuObject;
static HANDLE hPreBuildFrameMenuEvent;
extern int InitCustomMenus(void);

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct{
	char *szServiceName;
	int Frameid;
	INT_PTR param1;
}FrameMenuExecParam,*lpFrameMenuExecParam;

void FreeAndNil( void **p )
{
	if (p!=NULL)
	{
		if (*p!=NULL)
		{
			if (!IsBadCodePtr((FARPROC)*p))
			{
				mir_free(*p);
			}
			*p=NULL;
		}
	}
}

INT_PTR FreeOwnerDataFrameMenu (WPARAM wParam,LPARAM lParam)
{

	lpFrameMenuExecParam cmep;

	cmep=(lpFrameMenuExecParam)lParam;
	if (cmep!=NULL){
		FreeAndNil((void **)&cmep->szServiceName);
		FreeAndNil((void **)&cmep);
	}
	return(0);
}

static INT_PTR AddContextFrameMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi;

	if(mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;

	memset(&tmi,0,sizeof(tmi));

	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.position=mi->position;
	tmi.pszName=mi->pszName;

	if( mi->flags & CMIF_ROOTHANDLE )
		tmi.root = mi->hParentMenu;
	{
		lpFrameMenuExecParam fmep;
		fmep=(lpFrameMenuExecParam)mir_alloc(sizeof(FrameMenuExecParam));
		if (fmep==NULL){return(0);};
		fmep->szServiceName=mir_strdup(mi->pszService);
		fmep->Frameid=mi->popupPosition;
		fmep->param1=(INT_PTR)mi->pszContactOwner;

		tmi.ownerdata=fmep;
	}

	return(CallService(MO_ADDNEWMENUITEM,(WPARAM)hFrameMenuObject,(LPARAM)&tmi));
}

static INT_PTR RemoveContextFrameMenuItem(WPARAM wParam,LPARAM lParam)
{
	/* this do by free service
	lpFrameMenuExecParam fmep;
	fmep=(lpFrameMenuExecParam)CallService(MO_MENUITEMGETOWNERDATA,wParam,lParam);
	if (fmep!=NULL){
	if (fmep->szServiceName!=NULL){
	mir_free(fmep->szServiceName);
	fmep->szServiceName=NULL;
	};
	mir_free(fmep);
	}
	*/
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR FrameMenuExecService(WPARAM wParam,LPARAM lParam) {
	lpFrameMenuExecParam fmep=(lpFrameMenuExecParam)wParam;
	if (fmep==NULL)
		return(-1);

	CallService(fmep->szServiceName,lParam,fmep->param1);
	RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);   
	return(0);
}

//true - ok,false ignore
INT_PTR FrameMenuCheckService(WPARAM wParam,LPARAM lParam) {

	PCheckProcParam pcpp=(PCheckProcParam)wParam;
	lpFrameMenuExecParam fmep;
	TMO_MenuItem mi;

	if (pcpp==NULL){return(FALSE);};
	if (CallService(MO_GETMENUITEM,(WPARAM)pcpp->MenuItemHandle,(LPARAM)&mi)==0)
	{
		fmep = reinterpret_cast<lpFrameMenuExecParam>(mi.ownerdata);
		if (fmep!=NULL)
		{
			//pcpp->wParam  -  frameid
			if (((WPARAM)fmep->Frameid==pcpp->wParam)||fmep->Frameid==-1) return(TRUE);
		}
	}
	return(FALSE);
}

static INT_PTR ContextFrameMenuNotify(WPARAM wParam,LPARAM lParam)
{
	NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,lParam);
	return(0);
}

static INT_PTR BuildContextFrameMenu(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle=hFrameMenuObject;
	param.wParam=wParam;
	param.lParam=lParam;

	hMenu=CreatePopupMenu();
	//NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,-1);
	ContextFrameMenuNotify(wParam,-1);
	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	return (INT_PTR)hMenu;
}

//==========================Frames end
boolean InternalGenMenuModule=FALSE;

INT_PTR MeasureItemProxy(WPARAM wParam,LPARAM lParam)
{
	int val;
	if (InternalGenMenuModule) 
	{
		val=CallService(MS_INT_MENUMEASUREITEM,wParam,lParam);
		if (val) return(val);
	}

	return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);
}

INT_PTR DrawItemProxy(WPARAM wParam,LPARAM lParam)
{
	if (InternalGenMenuModule) 
	{
		int val;
		val=CallService(MS_INT_MENUDRAWITEM,wParam,lParam);
		if (val) return(val);
	}
	return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
}

int ProcessCommandProxy(WPARAM wParam,LPARAM lParam) 
{
	if (InternalGenMenuModule) {
		int val;
		val=CallService(MS_INT_MENUPROCESSCOMMAND,wParam,lParam);
		if (val) 
			return(val);
	}
	return CallService(MS_CLIST_MENUPROCESSCOMMAND,wParam,lParam);
}

INT_PTR ModifyMenuItemProxy(WPARAM wParam,LPARAM lParam)
{
	if (InternalGenMenuModule) 
	{
		int val;
		val=CallService(MS_INT_MODIFYMENUITEM,wParam,lParam);
		if (val) return(val);
	}

	return CallService(MS_CLIST_MODIFYMENUITEM,wParam,lParam);
}

int InitFramesMenus(void)
{
	TMenuParam tmp;

	if (!ServiceExists(MO_REMOVEMENUOBJECT))
		InternalGenMenuModule=TRUE;

	if (ServiceExists(MO_REMOVEMENUOBJECT))
	{
		CreateServiceFunction("FrameMenuExecService",FrameMenuExecService);
		CreateServiceFunction("FrameMenuCheckService",FrameMenuCheckService);
		CreateServiceFunction("FrameMenuFreeService",FreeOwnerDataFrameMenu);


		CreateServiceFunction(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM,RemoveContextFrameMenuItem);
		CreateServiceFunction(MS_CLIST_ADDCONTEXTFRAMEMENUITEM,AddContextFrameMenuItem);
		CreateServiceFunction(MS_CLIST_MENUBUILDFRAMECONTEXT,BuildContextFrameMenu);
		CreateServiceFunction(MS_CLIST_FRAMEMENUNOTIFY,ContextFrameMenuNotify);
		hPreBuildFrameMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDFRAMEMENU);

		//frame menu object
		memset(&tmp,0,sizeof(tmp));
		tmp.cbSize=sizeof(tmp);
		tmp.CheckService="FrameMenuCheckService";
		tmp.ExecService="FrameMenuExecService";
		tmp.name="Frames_Menu";
		hFrameMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,0,(LPARAM)&tmp);
		{
			OptParam op;
			op.Handle=hFrameMenuObject;
			op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
			op.Value=(INT_PTR)"FrameMenuFreeService";
			CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);
		}
	}
	return 0;
}

int UnitFramesMenu()
{
	return(0);
}
