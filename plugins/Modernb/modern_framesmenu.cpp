#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"

//==========================Frames
HANDLE hFrameMenuObject;

//contactmenu exec param(ownerdata)
//also used in checkservice
typedef struct{
	char *szServiceName;
	int Frameid;
	INT_PTR param1;
}FrameMenuExecParam,*lpFrameMenuExecParam;

void FreeAndNil( void **p )
{
	if ( p == NULL )
		return;

	if ( *p != NULL ) {
		mir_free( *p );
		*p = NULL;
	}	}

static INT_PTR AddContextFrameMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi={0};

	if(mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;

	memset(&tmi,0,sizeof(tmi));

	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.position=mi->position;
	tmi.pszName=mi->pszName;

	if ( mi->flags & CMIF_ROOTHANDLE ) tmi.root = mi->hParentMenu;
	{
		lpFrameMenuExecParam fmep;
		fmep=(lpFrameMenuExecParam)mir_alloc(sizeof(FrameMenuExecParam));
		if (fmep==NULL){return(0);};
		memset(fmep,0,sizeof(FrameMenuExecParam));
		fmep->szServiceName=mir_strdup(mi->pszService);
		fmep->Frameid=mi->popupPosition;
		fmep->param1=(INT_PTR)mi->pszContactOwner;

		tmi.ownerdata=fmep;
	};

	return(CallService(MO_ADDNEWMENUITEM,(WPARAM)hFrameMenuObject,(LPARAM)&tmi));
}


static INT_PTR RemoveContextFrameMenuItem(WPARAM wParam,LPARAM lParam)
{
	lpFrameMenuExecParam fmep;
	fmep=(lpFrameMenuExecParam)CallService(MO_MENUITEMGETOWNERDATA,wParam,lParam);
	if (fmep!=NULL){
		if (fmep->szServiceName!=NULL){
			mir_free_and_nill(fmep->szServiceName);
		};
		mir_free_and_nill(fmep);
	}

	if (lParam!=1)
		CallService(MO_REMOVEMENUITEM,wParam,0);

	return 0;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR FrameMenuExecService(WPARAM wParam,LPARAM lParam) {
	lpFrameMenuExecParam fmep=(lpFrameMenuExecParam)wParam;
	if (fmep==NULL){return(-1);};
	CallService(fmep->szServiceName,lParam,fmep->param1);	

	return(0);
};

//true - ok,false ignore
INT_PTR FrameMenuCheckService(WPARAM wParam,LPARAM lParam) {

	PCheckProcParam pcpp=(PCheckProcParam)wParam;
	lpFrameMenuExecParam fmep;
	TMO_MenuItem mi;

	if (pcpp==NULL){return(FALSE);};
	if (CallService(MO_GETMENUITEM,(WPARAM)pcpp->MenuItemHandle,(LPARAM)&mi)==0)
	{
		fmep=(lpFrameMenuExecParam)mi.ownerdata;
		if (fmep!=NULL)
		{
			//pcpp->wParam  -  frameid
			if (((WPARAM)fmep->Frameid==pcpp->wParam)||fmep->Frameid==-1) return(TRUE);	
		};

	};
	return(FALSE);
};

static INT_PTR ContextFrameMenuNotify(WPARAM wParam,LPARAM lParam)
{
	NotifyEventHooks(g_CluiData.hEventPreBuildFrameMenu,wParam,lParam);
	return(0);
};

static INT_PTR BuildContextFrameMenu(WPARAM wParam,LPARAM lParam)
{
	ListParam param = { 0 };
	param.MenuObjectHandle=hFrameMenuObject;
	param.wParam=wParam;
	param.lParam=lParam;

	HMENU hMenu=CreatePopupMenu();
	//NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,-1);
	ContextFrameMenuNotify(wParam,-1);
	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	return (INT_PTR)hMenu;
}

//==========================Frames end
boolean InternalGenMenuModule=FALSE;

int MeasureItemProxy(WPARAM wParam,LPARAM lParam) {

	int val;
	if (InternalGenMenuModule) 
	{

		val=CallService(MS_INT_MENUMEASUREITEM,wParam,lParam);
		if (val) return(val);
	};
	return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);


};


int DrawItemProxy(WPARAM wParam,LPARAM lParam) {
	if (InternalGenMenuModule) 
	{
		int val;
		val=CallService(MS_INT_MENUDRAWITEM,wParam,lParam);
		if (val) return(val);
	}
	return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);

};



int ProcessCommandProxy(WPARAM wParam,LPARAM lParam) {
	if (InternalGenMenuModule) 
	{
		int val;
		val=CallService(MS_INT_MENUPROCESSCOMMAND,wParam,lParam);
		if (val) return(val);
	};

	return CallService(MS_CLIST_MENUPROCESSCOMMAND,wParam,lParam);

};

int ModifyMenuItemProxy(WPARAM wParam,LPARAM lParam) {
	if (InternalGenMenuModule) 
	{
		int val;
		val=CallService(MS_INT_MODIFYMENUITEM,wParam,lParam);
		if (val) return(val);
	};

	return CallService(MS_CLIST_MODIFYMENUITEM,wParam,lParam);

};


int InitFramesMenus(void)
{
	TMenuParam tmp;

	if (!ServiceExists(MO_REMOVEMENUOBJECT))
	{

		InitCustomMenus();
		InternalGenMenuModule=TRUE;
	};

	if (ServiceExists(MO_REMOVEMENUOBJECT))
	{
		CreateServiceFunction("FrameMenuExecService",FrameMenuExecService);
		CreateServiceFunction("FrameMenuCheckService",FrameMenuCheckService);

		CreateServiceFunction(MS_CLIST_REMOVECONTEXTFRAMEMENUITEM,RemoveContextFrameMenuItem);
		CreateServiceFunction(MS_CLIST_ADDCONTEXTFRAMEMENUITEM,AddContextFrameMenuItem);
		CreateServiceFunction(MS_CLIST_MENUBUILDFRAMECONTEXT,BuildContextFrameMenu);
		CreateServiceFunction(MS_CLIST_FRAMEMENUNOTIFY,ContextFrameMenuNotify);
		

		//frame menu object
		memset(&tmp,0,sizeof(tmp));
		tmp.cbSize=sizeof(tmp);
		tmp.CheckService="FrameMenuCheckService";
		tmp.ExecService="FrameMenuExecService";
		tmp.name="FrameMenu";
		hFrameMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,0,(LPARAM)&tmp);
	}
	return 0;
}
int UnitFramesMenu()
{

	/*
	if (ServiceExists(MO_REMOVEMENUOBJECT)) 
	{
	CallService(MO_REMOVEMENUOBJECT,hFrameMenuObject,0);
	if (InternalGenMenuModule){UnitGenMenu();};
	};
	*/
	return(0);
};
