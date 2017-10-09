{$IFNDEF M_CLIST}
{$DEFINE M_CLIST}

{$ifndef STATUSMODES}
    {$include statusmodes.inc}
{$endif}

type
  HGENMENU = THANDLE;

const
  {
    wParam : new_status
    lParam : 0
    Affect : Sent when the user acks to change their status, see notes
    Notes  : Also sent due to a MS_CLIST_SETSTATUSMODE
  }
  ME_CLIST_STATUSMODECHANGE:PAnsiChar = 'CList/StatusModeChange';

  {
    wParam : 0
    lParam : 0
    Affect : Get the current status mode, see notes
    Notes  : This is the status, as set by the user, not any protocol specific status
             all protocol modules will attempt to conform to this setting at ALL times.
  }
  MS_CLIST_GETSTATUSMODE:PAnsiChar = 'CList/GetStatusMode';

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

type
  PCLISTDOUBLECLICKACTION = ^TCLISTDOUBLECLICKACTION;
  TCLISTDOUBLECLICKACTION = record
    cbSize         : int;
    pszContactOwner: PAnsiChar; // name of the protocol owning the contact or NULL(0) for all
    flags          : dword;     // CMIF_NOT flags above
    pszService     : PAnsiChar; // service to call on double click, is called with wParam=hContact, lParam=0
  end;

const
  {
    wParam : 0
    lParam : Pointer to a initalised TCLISTDOUBLECLICKACTION structure
    Affect : Sets the service to call when a contact is double-clicked, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : in case of conflicts, the first module to have registered
             will get the double click, no others will, this service
             will return success even for duplicates
             -
             This service was dropped from development during 0.3.0.0, it is no
             longer supported, see ME_CLIST_DOUBLECLICKED
    Version: 0.1.2.2+, 0.2.0+ ONLY (not 3.0a)
  }
  MS_CLIST_SETDOUBLECLICKACTION:PAnsiChar = 'CList/SetDoubleClickAction';

  {
    wParam : TMCONTACT
    lParam : <none>
    Affect : Register with this event to be notified of a double click on the CList
             against a TMCONTACT, you will not be notified if there is a pending CList event
             that the double click clears, (i.e. flashing icon is presented to be clicked)
    Version: 0.3.0.0
  }
  ME_CLIST_DOUBLECLICKED:PAnsiChar = 'CList/DoubleClicked';

type
  PCLISTEVENT = ^TCLISTEVENT;
  TCLISTEVENT = record
    hContact  : TMCONTACT; // handle to the contact to put the icon by
    flags     : dword;
    hIcon     : HICON;     // icon to flash!
    hDBEvent  : TMEVENT;   // caller defined, but should be unique for hContact
                           // or pszProtocol:PAnsiChar
    lParam    : LPARAM;
    pszService: PAnsiChar; // name of service to call on activation
    szTooltip : TChar;     // short description of the event to display as a tooltip on the systray
  end;

const
  CLEF_URGENT         = 1; // flashes the icon even if the user is occupied, and puts
                           // the event at the top of the queue
  CLEF_ONLYAFEW       = 2; // icon will not flash forever, only a few times,
                           // e.g. online alert
  CLEF_UNICODE        = 4; // set pszTooltip as unicode
  CLEF_PROTOCOLGLOBAL = 8; // set event globally for protocol, hContact has to
                           // be NULL, lpszProtocol the protocol ID name to be set

  IMAGE_GROUPOPEN = 11;
  IMAGE_GROUPSHUT = 12;

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
    cbSize    :int;   //size in bytes of this structure
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
