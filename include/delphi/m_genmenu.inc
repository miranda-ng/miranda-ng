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

const
{
  builds the SubGroup menu
  wParam=lParam=0
  returns a HMENU identifying the menu.
}
  MS_CLIST_MENUBUILDSUBGROUP:PAnsiChar = 'CList/MenuBuildSubGroup';

{
  add a new item to the SubGroup menus
  wParam=lpGroupMenuParam, params to call when exec menuitem
  lParam=(LPARAM)(CLISTMENUITEM*)&mi
}
  MS_CLIST_ADDSUBGROUPMENUITEM:PAnsiChar = 'CList/AddSubGroupMenuItem';

{
  the SubGroup menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDSUBGROUPMENU:PAnsiChar = 'CList/PreBuildSubGroupMenu';

// SubGroup MENU

// Group MENU
type
  PGroupMenuParam = ^TGroupMenuParam;
  TGroupMenuParam = record
    wParam:WPARAM;
    lParam:LPARAM;
  end;

const
{
  builds the Group menu
  wParam=lParam=0
  returns a HMENU identifying the menu.
}
  MS_CLIST_MENUBUILDGROUP:PAnsiChar = 'CList/MenuBuildGroup';

{
  add a new item to the Group menus
  wParam=lpGroupMenuParam, params to call when exec menuitem
  lParam=(LPARAM)(CLISTMENUITEM*)&mi
}
  MS_CLIST_ADDGROUPMENUITEM:PAnsiChar = 'CList/AddGroupMenuItem';

{
  the Group menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDGROUPMENU:PAnsiChar = 'CList/PreBuildGroupMenu';


// TRAY MENU
{
  builds the tray menu
  wParam=lParam=0
  returns a HMENU identifying the menu.
}
  MS_CLIST_MENUBUILDTRAY:PAnsiChar = 'CList/MenuBuildTray';

{
  add a new item to the tray menus
  wParam=0
  lParam=(LPARAM)(CLISTMENUITEM*)&mi
}
  MS_CLIST_ADDTRAYMENUITEM:PAnsiChar = 'CList/AddTrayMenuItem';

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
  add a new item to the status menu
  wParam=0
  lParam=(LPARAM)(CLISTMENUITEM*)&mi
}
//!!  MS_CLIST_ADDSTATUSMENUITEM = 'CList/AddStatusMenuItem'; // defined in m_clist.inc

{
  builds the main menu
  wParam=lParam=0
  returns a HMENU identifying the menu.
}
  MS_CLIST_MENUBUILDMAIN:PAnsiChar = 'CList/MenuBuildMain';

{
  the main menu is about to be built
  wParam=lParam=0
}
  ME_CLIST_PREBUILDMAINMENU:PAnsiChar = 'CList/PreBuildMainMenu';

// GENMENU_MODULE

  SETTING_NOOFFLINEBOTTOM_DEFAULT = 0;

type
  PMO_MenuItem = ^TMO_MenuItem;
  TMO_MenuItem = record
    position :int;
    root     :HGENMENU;
    szName   :TCHAR;
    flags    :integer;
    hIcon    :HICON;   // or hIcolibItem:THANDLE;
    ownerdata:^pointer;
    hLangpack:int;
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

function Menu_Build(parent:HMENU; menuObject:THANDLE; wParam:WPARAM; lParam:LPARAM) : HMENU; stdcall;
              external AppDLL name 'Menu_Build';

{
  Passes custom lParam to the ExecMenuService for the specified menu item
  returns true if command is processed, false otherwise
}

function Menu_ProcessCommand(hMenuItem:HGENMENU; lParam:LPARAM) : integer; stdcall;
              external AppDLL name 'Menu_ProcessCommand';

{
  if menu not known call this
  LOWORD(wparam) menuident (from WM_COMMAND message)
  It automatically finds right menuobject and menuitem and calls Menu_ProcessCommand
  returns true if command is processed, false otherwise
}

function Menu_ProcessCommandById(iCommand:integer; lParam:LPARAM) : integer; stdcall;
              external AppDLL name 'Menu_ProcessCommandById';

{
  returns=MenuObjectHandle on success,-1 on failure
}

function Menu_AddObject(const name, displayName, checkService, execService:PAnsiChar) : THANDLE; stdcall;
              external AppDLL name 'Menu_AddObject';

{
  returns 0 on success,-1 on failure
  Note: you must free all ownerdata structures, before you
  call this service.MO_REMOVEMENUOBJECT NOT free it.
}

function Menu_RemoveObject(hMenuObject:THANDLE) : integer; stdcall;
              external AppDLL name 'Menu_RemoveObject';

{
  returns 0 on success,-1 on failure.
  You must free ownerdata before this call.
  If MenuItemHandle is root all child will be removed too.
}

function Menu_RemoveItem(hMenuItem:HGENMENU) : integer; stdcall;
              external AppDLL name 'Menu_RemoveItem';

{
  return MenuItemHandle on success,-1 on failure
}

function Menu_AddItem(hMenuObject:THANDLE; var tmi:TMO_MenuItem) : integer; stdcall;
              external AppDLL name 'Menu_AddItem';

{
  wparam MenuItemHandle
  returns ownerdata on success,NULL on failure
  Useful to get and free ownerdata before delete menu item.
}
const
  MO_MENUITEMGETOWNERDATA:PAnsiChar = 'MO/MenuItemGetOwnerData';

{
  wparam MenuItemHandle
  lparam PMO_MenuItem
  returns 0 on success,-1 on failure
}
  MO_MODIFYMENUITEM:PAnsiChar = 'MO/ModifyMenuItem';

{
  wparam=MenuItemHandle
  lparam=PMO_MenuItem
  returns 0 and filled PMO_MenuItem structure on success and -1 on failure
}
  MO_GETMENUITEM:PAnsiChar = 'MO/GetMenuItem';

{
  wparam=MenuItemHandle
  lparam=0
  returns a menu handle on success or NULL on failure
}
  MO_GETDEFAULTMENUITEM:PAnsiChar = 'MO/GetDefaultMenuItem';

{
  wparam=MenuObjectHandle
  lparam=vKey
  returns TRUE if it processed the command, FALSE otherwise
  this should be called in WM_KEYDOWN
}
  MO_PROCESSHOTKEYS:PAnsiChar = 'MO/ProcessHotKeys';

{
  process a WM_DRAWITEM message
  wparam=0
  lparam=LPDRAWITEMSTRUCT
  returns TRUE if it processed the command, FALSE otherwise
}
  MO_DRAWMENUITEM:PAnsiChar = 'MO/DrawMenuItem';

{
  process a WM_MEASUREITEM message
  wparam=0
  lparam=LPMEASUREITEMSTRUCT
  returns TRUE if it processed the command, FALSE otherwise
}
  MO_MEASUREMENUITEM:PAnsiChar = 'MO/MeasureMenuItem';

{
  sets an option for the whole menu object
  returns TRUE if it processed the command, FALSE otherwise
}

const 
  MCO_OPT_USERDEFINEDITEMS = 1; // sets uniq name to menuitem(used to store it in database when enabled OPT_USERDEFINEDITEMS)
  MCO_OPT_FREE_SERVICE = 2; // sets FreeService for menuobject.
  MCO_OPT_ONADD_SERVICE = 3; // sets onAddService for menuobject.
  MCO_OPT_CHECK_SERVICE = 4;

function Menu_ConfigureObject(menu:THANDLE; option:integer; value:PAnsiChar) : integer; stdcall;
              external AppDLL name 'Menu_ConfigureObject';

{
  wparam=0
  lparam=*lpOptParam
  returns TRUE if it processed the command, FALSE otherwise
}

const 
  OPT_USERDEFINEDITEMS = 1; // enables ability user to edit menuitems via options page.
  OPT_HOTKEY = 2;

function Menu_ConfigureItem(menu:THANDLE; option:integer; value:int_ptr) : integer; stdcall;
              external AppDLL name 'Menu_ConfigureItem';

{
  wparam=char* szProtoName
  lparam=0
  returns HGENMENU of the root item or NULL
}

function Menu_GetProtocolRoot(const proto:PAnsiChar) : integer; stdcall;
              external AppDLL name 'Menu_GetProtocolRoot';

{$ENDIF}
