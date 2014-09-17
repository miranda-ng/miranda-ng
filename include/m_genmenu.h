#ifndef M_GENMENU_H
#define M_GENMENU_H

#ifndef M_CLIST_H__
   #include <m_clist.h>
#endif

extern int hLangpack;

/*
  Main features:
  1) Independet from clist,may be used in any module.
  2) Module defined Exec and Check services.
  3) Menu with any level of popups,icons for root of popup.
  4) You may use measure/draw/processcommand even if menuobject is unknown.

  Idea of GenMenu module consists of that,
  it must be independet and offers only general menu purpose services:
  MO_CREATENEWMENUOBJECT
  MO_REMOVEMENUOBJECT
  MO_ADDNEWMENUITEM
  MO_REMOVEMENUITEM
  ...etc

  And then each module that want use and offer to others menu handling
  must create own services.For example i rewrited mainmenu and
  contactmenu code in clistmenus.c.If you look at code all functions
  are very identical, and vary only in check/exec services.

  So template set of function will like this:
  Remove<NameMenu>Item
  Add<NameMenu>Item
  Build<NameMenu>
  <NameMenu>ExecService
  <NameMenu>CheckService

  ExecService and CheckService used as callbacks when GenMenu must
  processcommand for menu item or decide to show or not item.This make
  GenMenu independet of which params must passed to service when user
  click on menu,this decide each module.
						28-04-2003 Bethoven

*/



/*
Analog to CLISTMENUITEM,but invented two params root and ownerdata.
root is used for creating any level popup menus,set to -1 to build
at first level and root=MenuItemHandle to place items in submenu
of this item.Must be used two new flags CMIF_ROOTPOPUP and CMIF_CHILDPOPUP
(defined in m_clist.h)

ownerdata is passed to callback services(ExecService and CheckService)
when building menu or processed command.
*/

/*GENMENU_MODULE*/
/*
Changes:

28-04-2003
Moved all general stuff to genmenu.c(m_genmenu.h,genmenu.h),
so removed all frames stuff.


Changes:

28-12-2002

Contact menu item service called with wparam=hcontact,lparam=popupPosition -
plugin may add different menu items with some service.
(old behavior wparam=hcontact lparam=0)



25-11-2002		Full support of runtime build of all menus.
				Contact		MS_CLIST_ADDCONTACTMENUITEM
								MS_CLIST_MENUBUILDCONTACT
								ME_CLIST_PREBUILDCONTACTMENU

				MainMenu		MS_CLIST_ADDMAINMENUITEM
								MS_CLIST_REMOVEMAINMENUITEM
								MS_CLIST_MENUBUILDMAIN
								ME_CLIST_PREBUILDMAINMENU

				FrameMenu	MS_CLIST_ADDCONTEXTFRAMEMENUITEM
								MS_CLIST_REMOVECONTEXTFRAMEMENUITEM
								MS_CLIST_MENUBUILDFRAMECONTEXT
								ME_CLIST_PREBUILDFRAMEMENU

				For All menus may be used
								MS_CLIST_MODIFYMENUITEM

				All menus supported any level of popups
				(pszPopupName=(char *)hMenuItem - for make child of popup)
*/

// SubGroup MENU

// Group MENU
typedef struct
{
	int wParam;
	int lParam;
}
GroupMenuParam,*lpGroupMenuParam;

//builds the SubGroup menu
//wParam=lParam=0
//returns a HMENU identifying the menu.
#define MS_CLIST_MENUBUILDSUBGROUP							"CList/MenuBuildSubGroup"

//add a new item to the SubGroup menus
//wParam=lpGroupMenuParam, params to call when exec menuitem
//lParam=(LPARAM)(CLISTMENUITEM*)&mi

__forceinline HGENMENU Menu_AddSubGroupMenuItem(lpGroupMenuParam gmp, CLISTMENUITEM *mi)
{	mi->hLangpack = hLangpack;
	return (HGENMENU)CallService("CList/AddSubGroupMenuItem", (WPARAM)gmp, (LPARAM)mi);
}

//the SubGroup menu is about to be built
//wParam=lParam=0
#define ME_CLIST_PREBUILDSUBGROUPMENU						"CList/PreBuildSubGroupMenu"

// Group MENU

//builds the Group menu
//wParam=lParam=0
//returns a HMENU identifying the menu.
#define MS_CLIST_MENUBUILDGROUP							"CList/MenuBuildGroup"

//add a new item to the Group menus
//wParam=lpGroupMenuParam, params to call when exec menuitem
//lParam=(LPARAM)(CLISTMENUITEM*)&mi

__forceinline HGENMENU Menu_AddGroupMenuItem(lpGroupMenuParam gmp, CLISTMENUITEM *mi)
{	mi->hLangpack = hLangpack;
	return (HGENMENU)CallService("CList/AddGroupMenuItem", (WPARAM)gmp, (LPARAM)mi);
}

//the Group menu is about to be built
//wParam=lParam=0
#define ME_CLIST_PREBUILDGROUPMENU						"CList/PreBuildGroupMenu"

// TRAY MENU

//builds the tray menu
//wParam=lParam=0
//returns a HMENU identifying the menu.
#define MS_CLIST_MENUBUILDTRAY						"CList/MenuBuildTray"

//add a new item to the tray menus
//wParam=0
//lParam=(LPARAM)(CLISTMENUITEM*)&mi

__forceinline HGENMENU Menu_AddTrayMenuItem(CLISTMENUITEM *mi)
{	mi->hLangpack = hLangpack;
	return (HGENMENU)CallService("CList/AddTrayMenuItem", 0, (LPARAM)mi);
}

//the tray menu is about to be built
//wParam=lParam=0
#define ME_CLIST_PREBUILDTRAYMENU					"CList/PreBuildTrayMenu"

// STATUS MENU

//the status menu is about to be built
//wParam=lParam=0
#define ME_CLIST_PREBUILDSTATUSMENU "CList/PreBuildStatusMenu"

//builds the main menu
//wParam=lParam=0
//returns a HMENU identifying the menu.
#define MS_CLIST_MENUBUILDMAIN						"CList/MenuBuildMain"

//the main menu is about to be built
//wParam=lParam=0
#define ME_CLIST_PREBUILDMAINMENU					"CList/PreBuildMainMenu"


/*GENMENU_MODULE*/

#define SETTING_NOOFFLINEBOTTOM_DEFAULT 0

typedef struct
{
	int cbSize;
	union
	{
		char *pszName;
		TCHAR *ptszName;
	};
	int position;
	HGENMENU root;
	int flags;
	union {
		HICON hIcon;
		HANDLE hIcolibItem;
	};
	DWORD hotKey;
	void *ownerdata;
	int   hLangpack;
}
	TMO_MenuItem,*PMO_MenuItem;

/*
This structure passed to CheckService.
*/
typedef struct
{
	void *MenuItemOwnerData;
	HGENMENU MenuItemHandle;
	WPARAM wParam;//from  ListParam.wParam when building menu
	LPARAM lParam;//from  ListParam.lParam when building menu
}
	TCheckProcParam,*PCheckProcParam;

//used in MO_BUILDMENU
typedef struct tagListParam
{
	int rootlevel;
	HANDLE MenuObjectHandle;
	WPARAM wParam;
	LPARAM lParam;
}
	ListParam,*lpListParam;

typedef struct
{
	HMENU menu;
	int ident;
	LPARAM lParam;
}
	ProcessCommandParam,*lpProcessCommandParam;

//wparam started hMenu
//lparam ListParam*
//result hMenu
#define MO_BUILDMENU						"MO/BuildMenu"

//wparam=MenuItemHandle
//lparam userdefined
//returns TRUE if it processed the command, FALSE otherwise
#define MO_PROCESSCOMMAND					"MO/ProcessCommand"

//if menu not known call this
//LOWORD(wparam) menuident (from WM_COMMAND message)
//returns TRUE if it processed the command, FALSE otherwise
//Service automatically find right menuobject and menuitem
//and call MO_PROCESSCOMMAND
#define MO_PROCESSCOMMANDBYMENUIDENT		"MO/ProcessCommandByMenuIdent"

//wparam=MenuItemHandle
//lparam=0
//returns 0 on success,-1 on failure.
//You must free ownerdata before this call.
//If MenuItemHandle is root all child will be removed too.
#define MO_REMOVEMENUITEM					"MO/RemoveMenuItem"

//wparam=MenuObjectHandle
//lparam=PMO_MenuItem
//return MenuItemHandle on success,-1 on failure
//Service supports old menu items (without CMIF_ROOTPOPUP or
//CMIF_CHILDPOPUP flag).For old menu items needed root will be created
//automatically.
#define MO_ADDNEWMENUITEM					"MO/AddNewMenuItem"

//wparam MenuItemHandle
//returns ownerdata on success,NULL on failure
//Useful to get and free ownerdata before delete menu item.
#define MO_MENUITEMGETOWNERDATA				"MO/MenuItemGetOwnerData"

//wparam MenuItemHandle
//lparam PMO_MenuItem
//returns 0 on success,-1 on failure
#define MO_MODIFYMENUITEM					"MO/ModifyMenuItem"

//wparam=MenuItemHandle
//lparam=PMO_MenuItem
//returns 0 and filled PMO_MenuItem structure on success and
//-1 on failure
#define MO_GETMENUITEM						"MO/GetMenuItem"

//wparam=MenuItemHandle
//lparam=0
//returns a menu handle on success or NULL on failure
#define MO_GETDEFAULTMENUITEM				"MO/GetDefaultMenuItem"

//wparam=MenuObjectHandle
//lparam=vKey
//returns TRUE if it processed the command, FALSE otherwise
//this should be called in WM_KEYDOWN
#define	MO_PROCESSHOTKEYS					"MO/ProcessHotKeys"

//set uniq name to menuitem(used to store it in database when enabled OPT_USERDEFINEDITEMS)
#define OPT_MENUITEMSETUNIQNAME								1

//Set FreeService for menuobject. When freeing menuitem it will be called with
//wParam=MenuItemHandle
//lParam=mi.ownerdata
#define OPT_MENUOBJECT_SET_FREE_SERVICE						2

//Set onAddService for menuobject.
#define OPT_MENUOBJECT_SET_ONADD_SERVICE					3

//Set menu check service
#define OPT_MENUOBJECT_SET_CHECK_SERVICE          4

//enable ability user to edit menuitems via options page.
#define OPT_USERDEFINEDITEMS 1

// szName = unique menu object identifier
// szDisplayName = menu display name (auto-translated by core)
// szCheckService = this service called when module build menu(MO_BUILDMENU).
//    Service is called with params wparam = PCheckProcParam, lparam = 0
//    if service returns FALSE, item is skipped.
// szExecService = this service called when user select menu item.
//    Service called with params wparam = ownerdata; lparam = lParam from MO_PROCESSCOMMAND
// 
// returns = MenuObjectHandle on success,-1 on failure

struct TMenuParam
{
	int cbSize;
	LPCSTR name, CheckService, ExecService;
};

__forceinline HANDLE MO_CreateMenuObject(LPCSTR szName, LPCSTR szDisplayName, LPCSTR szCheckService, LPCSTR szExecService)
{
	TMenuParam param = { sizeof(param), szName, szCheckService, szExecService };
	return (HANDLE)CallService("MO/CreateNewMenuObject", (WPARAM)szDisplayName, (LPARAM)&param);
}

//wparam=MenuObjectHandle
//lparam=0
//returns 0 on success,-1 on failure
//Note: you must free all ownerdata structures, before you
//call this service.MO_REMOVEMENUOBJECT NOT free it.
#define MO_REMOVEMENUOBJECT "MO/RemoveMenuObject"

// wparam=0
// lparam=*lpOptParam
// returns TRUE if it processed the command, FALSE otherwise
#define MO_SRV_SETOPTIONSMENUOBJECT "MO/SetOptionsMenuObject"

typedef struct tagOptParam
{
	HANDLE Handle;
	int Setting;
	INT_PTR Value;
}
	OptParam,*lpOptParam;

__forceinline void MO_SetMenuObjectParam(HANDLE hMenu, int iSetting, int iValue)
{
	OptParam param = { hMenu, iSetting, iValue };
	CallService(MO_SRV_SETOPTIONSMENUOBJECT, 0, (LPARAM)&param);
}
__forceinline void MO_SetMenuObjectParam(HANDLE hMenu, int iSetting, LPCSTR pszValue)
{
	OptParam param = { hMenu, iSetting, (INT_PTR)pszValue };
	CallService(MO_SRV_SETOPTIONSMENUOBJECT, 0, (LPARAM)&param);
}

//wparam=0
//lparam=*lpOptParam
//returns TRUE if it processed the command, FALSE otherwise
#define MO_SETOPTIONSMENUITEM "MO/SetOptionsMenuItem"

//wparam=char* szProtoName
//lparam=0
//returns HGENMENU of the root item or NULL
#define MO_GETPROTOROOTMENU "MO/GetProtoRootMenu"

__forceinline HGENMENU MO_GetProtoRootMenu( const char* szProtoName )
{
	return ( HGENMENU )CallService( MO_GETPROTOROOTMENU, ( WPARAM )szProtoName, 0 );
}

#endif
