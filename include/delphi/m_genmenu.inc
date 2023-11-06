{$IFNDEF M_GENMENU}
{$DEFINE M_GENMENU}
{
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

}

{
Analog to CLISTMENUITEM,but invented two params root and ownerdata.
root is used for creating any level popup menus,set to -1 to build
at first level and root=MenuItemHandle to place items in submenu
of this item.Must be used two new flags CMIF_ROOTPOPUP and CMIF_CHILDPOPUP
(defined in m_clist.h)

ownerdata is passed to callback services(ExecService and CheckService)
when building menu or processed command.
}

// GENMENU_MODULE
{
Changes:

28-04-2003
Moved all general stuff to genmenu.c(m_genmenu.h,genmenu.h),
so removed all frames stuff.


Changes:

28-12-2002

Contact menu item service called with wparam=hcontact,lparam=popupPosition -
plugin may add different menu items with some service.
(old behavior wparam=hcontact lparam=0)



25-11-2002    Full support of runtime build of all menus.
   Contact MS_CLIST_ADDCONTACTMENUITEM
           MS_CLIST_MENUBUILDCONTACT
           ME_CLIST_PREBUILDCONTACTMENU

   MainMenu  MS_CLIST_ADDMAINMENUITEM
             MS_CLIST_MENUBUILDMAIN
             ME_CLIST_PREBUILDMAINMENU

   FrameMenu MS_CLIST_ADDCONTEXTFRAMEMENUITEM
             MS_CLIST_REMOVECONTEXTFRAMEMENUITEM
             MS_CLIST_MENUBUILDFRAMECONTEXT
             ME_CLIST_PREBUILDFRAMEMENU

   For All menus may be used
           MS_CLIST_MODIFYMENUITEM

   All menus supported any level of popups
   (pszPopupName=(AnsiChar *)hMenuItem - for make child of popup)
}

// predefined menu object codes
const
  MO_MAIN = -1;
  MO_CONTACT = -2;
  MO_PROTO  = -3;
  MO_STATUS = -4;

// Group MENU
type
  PGroupMenuParam = ^TGroupMenuParam;
  TGroupMenuParam = record
    wParam:WPARAM;
    lParam:LPARAM;
  end;

const
{
  the Group menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDGROUPMENU:PAnsiChar = 'CList/PreBuildGroupMenu';

// TRAY MENU

{
  the tray menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDTRAYMENU:PAnsiChar = 'CList/PreBuildTrayMenu';

// STATUS MENU

{
  the status menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDSTATUSMENU:PAnsiChar = 'CList/PreBuildStatusMenu';

{
  the main menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDMAINMENU:PAnsiChar = 'CList/PreBuildMainMenu';

// GENMENU_MODULE

  SETTING_NOOFFLINEBOTTOM_DEFAULT = 0;

const
  CMIF_GRAYED     = 1;
  CMIF_CHECKED    = 2;
  CMIF_HIDDEN     = 4;   // only works on contact menus
  CMIF_NOTOFFLINE = 8;   // item won't appear for contacts that are offline
  CMIF_NOTONLINE  = 16;  //          "      online
  CMIF_NOTONLIST  = 32;  // item won't appear on standard contacts
  CMIF_NOTOFFLIST = 64;  // item won't appear on contacts that have the 'NotOnList' setting
  CMIF_UNICODE    = 512; // will return TCHAR* instead of AnsiChar*
  CMIF_KEEPUNTRANSLATED = 1024; // don't translate a menu item

  CMIF_DEFAULT = 4096; // this menu item is the default one

type
  PMO_MenuItem = ^TMO_MenuItem;
  TMO_MenuItem = record
    position  : int;
    pszService: PAnsiChar;
    root      : HGENMENU;
    szName    : TCHAR;
    flags     : integer;
    hIcon     : HICON;   // or hIcolibItem:THANDLE;
    pPlugin   : Pointer;
    uid       : TGUID;
  end;

{
This structure passed to CheckService.
}
type
  PCheckProcParam = ^TCheckProcParam;
  TCheckProcParam = record
     MenuItemOwnerData:^pointer;
     MenuItemHandle:HGENMENU;
     wParam:WPARAM; // from  ListParam.wParam when building menu
     lParam:LPARAM; // from  ListParam.lParam when building menu
  end;

type
  PMenuParam = ^TMenuParam;
  TMenuParam = record
    cbSize:integer;
    name  :PAnsiChar;
{
  This service called when module build menu(MO_BUILDMENU).
  Service called with params

  wparam=PCheckProcParam
  lparam=0
  if return==FALSE item is skiped.
}
    CheckService:PAnsiChar;
{
  This service called when user select menu item.
  Service called with params
  wparam=ownerdata
  lparam=lParam from MO_PROCESSCOMMAND
}
    ExecService:PAnsiChar;//called when processmenuitem called
  end;

type
  PProcessCommandParam = ^TProcessCommandParam;
  TProcessCommandParam = record
    menu  :HMENU;
    ident :int;
    lParam:LPARAM;
  end;

{
  Builds a menu from the menu object specified
  result hMenu
}

function Menu_Build(parent:HMENU; menuObject:integer; wParam:WPARAM; lParam:LPARAM) : HMENU; stdcall; external AppDLL;

{
  Passes custom lParam to the ExecMenuService for the specified menu item
  returns true if command is processed, false otherwise
}

function Menu_ProcessCommand(hMenuItem:HGENMENU; lParam:LPARAM) : integer; stdcall; external AppDLL;

{
  if menu not known call this
  LOWORD(wparam) menuident (from WM_COMMAND message)
  It automatically finds right menuobject and menuitem and calls Menu_ProcessCommand
  returns true if command is processed, false otherwise
}

function Menu_ProcessCommandById(iCommand:integer; lParam:LPARAM) : integer; stdcall; external AppDLL;

{
  creates a submenu
  returns=MenuObjectHandle on success,-1 on failure
}

function Menu_CreateRoot(menuObject:integer; const name:PWideChar; position:integer; icon:THANDLE; lang:integer) : THANDLE; stdcall; external AppDLL;

{
  returns=MenuObjectHandle on success,-1 on failure
}

function Menu_AddObject(const name, displayName, checkService, execService:PAnsiChar) : THANDLE; stdcall; external AppDLL;

{
  returns 0 on success,-1 on failure
  Note: you must free all ownerdata structures, before you
  call this service.MO_REMOVEMENUOBJECT NOT free it.
}

function Menu_RemoveObject(hMenuObject:integer) : integer; stdcall; external AppDLL;

{
  returns 0 on success,-1 on failure.
  You must free ownerdata before this call.
  If MenuItemHandle is root all child will be removed too.
}

function Menu_RemoveItem(hMenuItem:HGENMENU) : integer; stdcall; external AppDLL;

{
  return MenuItemHandle on success,-1 on failure
}

function Menu_AddItem(hMenuObject:integer; var tmi:TMO_MenuItem; pUserData:pointer) : HGENMENU; stdcall; external AppDLL;

{
  Useful to get and free ownerdata before delete menu item.
}

function Menu_GetItemData(hMenuItem:HGENMENU) : pointer; stdcall; external AppDLL;

{
  returns 0 on success,-1 on failure
}

function Menu_GetItemInfo(hMenuItem:HGENMENU; var mi:TMO_MenuItem) : integer; stdcall; external AppDLL;

{
  returns a menu handle on success or NULL on failure
}

function Menu_GetDefaultItem(hMenuItem:HGENMENU) : HGENMENU; stdcall; external AppDLL;

{
  processes a WM_DRAWITEM message for user context menus,
}

function Menu_DrawItem(dis:LPARAM) : integer; stdcall; external AppDll;

{
  processes a WM_MEASUREITEM message for user context menus, see notes
}

function Menu_MeasureItem(mis:LPARAM) : integer; stdcall; external AppDll;

{
  wparam=MenuObjectHandle
  lparam=vKey
  returns TRUE if it processed the command, FALSE otherwise
  this should be called in WM_KEYDOWN
}

function Menu_ProcessHotKey(hMenuObject:integer; vKey:DWORD) : integer; stdcall; external AppDLL;

{
  sets an option for the whole menu object
  returns TRUE if it processed the command, FALSE otherwise
}

const 
  MCO_OPT_USERDEFINEDITEMS = 1; // sets uniq name to menuitem(used to store it in database when enabled OPT_USERDEFINEDITEMS)
  MCO_OPT_FREE_SERVICE = 2; // sets FreeService for menuobject.
  MCO_OPT_ONADD_SERVICE = 3; // sets onAddService for menuobject.
  MCO_OPT_CHECK_SERVICE = 4;

function Menu_ConfigureObject(menu:THANDLE; option:integer; value:PAnsiChar) : integer; stdcall; external AppDLL;

{
  wparam=0
  lparam=*lpOptParam
  returns TRUE if it processed the command, FALSE otherwise
}

const 
  OPT_HOTKEY = 2;
  MCI_OPT_EXECPARAM = 3; // INT_PTR or void*, associated with this item
  MCI_OPT_UID = 4;

function Menu_ConfigureItem(menu:THANDLE; option:integer; value:PAnsiChar) : integer; stdcall; external AppDLL;

{
  wparam=char* szProtoName
  lparam=0
  returns HGENMENU of the root item or NULL
}

function Menu_GetProtocolRoot(const proto:PAnsiChar) : integer; stdcall; external AppDLL;

{$ENDIF}
