{$IFNDEF M_CLIST}
{$DEFINE M_CLIST}

{$ifndef STATUSMODES}
    {$include statusmodes.inc}
{$endif}

type
  HGENMENU = THANDLE;

function Clist_SetStatusMode(status:int) : int; stdcall; external AppDll;

function Menu_BuildContactMenu(hContact:TMCONTACT) : HMENU; stdcall; external AppDll;

{
  Affect : Modify an existing menu item, see notes
  Returns: 0 on success, [non zero] on failure
}

function Menu_ModifyItem(hMenu:HGENMENU; const name:PWideChar; icon:THANDLE=INVALID_HANDLE_VALUE; flags:int=-1) : int; stdcall; external AppDll;

{
  Notes  : changes menu item's visibility
}

procedure Menu_ShowItem(hMenu:HGENMENU; bShow:BYTE); stdcall; external AppDll;

  {
    wParam : TMCONTACT
    lParam : 0
    Affect : the context menu for a contact is about to be built, see notes
    Notes  : modules should use this to change menu items that are specific
             to the contact that has them
    Version: v0.1.0.1+
  }
const
  ME_CLIST_PREBUILDCONTACTMENU:PAnsiChar = 'CList/PreBuildContactMenu';

const
  {
    wParam : TMCONTACT
    lParam : <none>
    Affect : Register with this event to be notified of a double click on the CList
             against a TMCONTACT, you will not be notified if there is a pending CList event
             that the double click clears, (i.e. flashing icon is presented to be clicked)
    Version: 0.3.0.0
  }
  ME_CLIST_DOUBLECLICKED:PAnsiChar = 'CList/DoubleClicked';

  function Clist_GetImageList : HIMAGELIST; stdcall; external AppDll;

  {
    wParam : TMCONTACT
    lParam : ICON_ID
    Affect : The icon of a contact in the contact list has changed,
             ICON_ID is an index to what image has changed
    Version: v0.1.2.1+
  }
const
  ME_CLIST_CONTACTICONCHANGED:PAnsiChar = 'CList/ContactIconChanged';

//******************************* CLUI only *********************************

  CLISTMENUIDMIN = $4000; // reserved range for clist menu ids
  CLISTMENUIDMAX = $7FFF;

  MPCF_CONTACTMENU = 1; // test commands from a contact menu
  MPCF_MAINMENU    = 2; // test commands from the main menu
  {
    Affect : Process a menu selection from a menu, see notes
    Returns: True if it processed the command, False otherwise
    notes  : hContact is the currently selected contact, it is not used
             if this is a main menu command, if this is NULL then the command
             is a contact menu one, the command is ignored
  }
  function Clist_MenuProcessCommand(menuid,flags:int; hContact:TMCONTACT) : integer; stdcall; external AppDll;

  {
    Affect : Process a menu hotkey, see notes
    Returns: True if it processed the command, False otherwise
    Notes  : this should be called in WM_KEYDOWN
  }
  function Clist_MenuProcessHotkey(virtKey:uint) : integer; stdcall; external AppDll;

  { 
    returns the icon's index of specified contact in the internal image list or -1
  }
  function Clist_GetContactIcon(hContact:TMCONTACT) : integer; stdcall; external AppDll;

  {
    returns a static display name for a contact
  }
  function Clist_GetContactDisplayName(hContact:TMCONTACT; mode:int):PWideChar; stdcall; external AppDll;

  {
    removes an event
  }
  function Clist_RemoveEvent(hContact:TMCONTACT;hDbEvent:TMEVENT) : integer; stdcall; external AppDll;

  {
    wParam : 0
    lParam : 0
    Affect : Toggles the show/hide status of the contact list
    Returns: 0 on success, [non zero] on failure
    Version: v0.1.1.0+
  }
const
  MS_CLIST_SHOWHIDE:PAnsiChar = 'CList/ShowHide';

{
  sent when the group get modified (created, renamed or deleted)
  or contact is moving from group to group
  wParam=hContact - NULL if operation on group
  lParam=pointer to CLISTGROUPCHANGE
}
type
  CLISTGROUPCHANGE = record
    pszOldName:TChar; //old group name
    pszNewName:TChar; //new group name
  end;

const
  ME_CLIST_GROUPCHANGE:PAnsiChar = 'CList/GroupChange';

  GROUPF_EXPANDED    = $04;
  GROUPF_HIDEOFFLINE = $08;

function Clist_GroupCreate(hParentGroup:integer; groupName:PWideChar) : integer; stdcall; external AppDll;

procedure Clist_ContactDoubleClicked(hContact:TMCONTACT); stdcall; external AppDll;

{$ENDIF}
