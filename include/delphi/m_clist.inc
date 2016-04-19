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
    wParam : new_status
    lParam : 0
    Affect : Force a change of status mode, see statusmodes.inc
  }
  MS_CLIST_SETSTATUSMODE:PAnsiChar = 'CList/SetStatusMode';

  {
    wParam : 0
    lParam : 0
    Affect : Get the current status mode, see notes
    Notes  : This is the status, as set by the user, not any protocol specific status
             all protocol modules will attempt to conform to this setting at ALL times.
  }
  MS_CLIST_GETSTATUSMODE:PAnsiChar = 'CList/GetStatusMode';

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

  {
    Invalidates the display name cache
    wParam=(WPARAM)(HANDLE)hContact
    lParam=not used
  }
  MS_CLIST_INVALIDATEDISPLAYNAME:PAnsiChar = 'CList/InvalidateDiplayName';

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

  {
    wParam : ControlID
    lParam : Pointer to MEASUREITEMSTRUCT struct
    Affect : Process a WM_MEASUREITEM message for user context menus, see notes
    Notes  : just because wParam, lParam is defined here, only pass them
             opaquely to this service, as is.
             -
             This is just to draw icons, if it is not called, the icons
             will not be drawn
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUMEASUREITEM:PAnsiChar = 'CList/MenuMeasureItem';

  {
    wParam :
    lParam :
    Affect : Process a WM_DRAWITEM message for user context menus,
             wParam, lParam should be passed from such message handler.
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUDRAWITEM:PAnsiChar = 'CList/MenuDrawItem';

  {
    wParam : 0
    lParam : 0
    Affect : Get the image list handle with all the useful icons in it
    Version: v0.1.1.0+
  }
  MS_CLIST_GETICONSIMAGELIST:PAnsiChar = 'CList/GetIconsImageList';
  IMAGE_GROUPOPEN = 11;
  IMAGE_GROUPSHUT = 12;

  {
    wParam : TMCONTACT
    lParam : ICON_ID
    Affect : The icon of a contact in the contact list has changed,
             ICON_ID is an index to what image has changed
    Version: v0.1.2.1+
  }
  ME_CLIST_CONTACTICONCHANGED:PAnsiChar = 'CList/ContactIconChanged';

//******************************* CLUI only *********************************

  CLISTMENUIDMIN = $4000; // reserved range for clist menu ids
  CLISTMENUIDMAX = $7FFF;
  {
    NOTE:  v0.7.0.26+
    Due to it is generic practice to handle menu command via WM_COMMAND
    window message handle and practice to process it via calling service
    in form: CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),
             MPCF_CONTACTMENU),(LPARAM) hContact))
    to ensure that WM_COMMAND was realy from clist menu not from other menu
    it is reserved range of menu ids from CLISTMENUIDMIN to CLISTMENUIDMAX
    the menu items with ids outside from such range will not be processed by service.
    Moreover if you process WM_COMMAND youself and your window contains self menu
    please be sure that you will not call service for non-clist menu items.
    The simplest way is to ensure that your menus are not use item ids from such range.
    Otherwise, you HAVE TO distinguish WM_COMMAND from clist menus and from your
    internal menu and  DO NOT call MS_CLIST_MENUPROCESSCOMMAND for non clist menus.
  }
  MPCF_CONTACTMENU = 1; // test commands from a contact menu
  MPCF_MAINMENU    = 2; // test commands from the main menu
  {
    wParam : MAKEWPARAM(LOWORD(wParam of WM_COMMAND),flags)
    lParam : TMCONTACT
    Affect : Process a menu selection from a menu, see notes
    Returns: True if it processed the command, False otherwise
    notes  : hContact is the currently selected contact, it is not used
             if this is a main menu command, if this is NULL then the command
             is a contact menu one, the command is ignored
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUPROCESSCOMMAND:PAnsiChar = 'CList/MenuProcessCommand';

  {
    wParam : virtual key code
    lParam : MPCF_* flags
    Affect : Process a menu hotkey, see notes
    Returns: True if it processed the command, False otherwise
    Notes  : this should be called in WM_KEYDOWN
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUPROCESSHOTKEY:PAnsiChar = 'CList/MenuProcessHotkey';

  {
    wParam : Pointer to a MSG structurer
    lParam : Pointer to an LRESULT
    Affect : Process all the messages required for docking, see notes
    Returns: True if the message should NOT be processed anymore, False otherwise
    Notes  : only msg.hwnd, msg.message, msg.wParam and msg.lParam are used
             your WndProc should return the lResult if AND only IF, TRUE is returned
    Version: v0.1.1.0+
  }
  MS_CLIST_DOCKINGPROCESSMESSAGE:PAnsiChar = 'CList/DockingProcessMessage';

  {
    wParam : 0
    lParam : 0
    Affect : Determines wheter the contact list docked
    Returns: pnon zero] if the contact list is docked, or 0 if it's not
    Version: v0.1.1.0+
  }
  MS_CLIST_DOCKINGISDOCKED:PAnsiChar = 'CList/DockingIsDocked';

  {
    wParam : Pointer to TMSG
    lParam : Pointer to an LRESULT
    Affect : Process all the messages required for hotkeys, see notes
    Returns: True if the message should not be processed anymore or False otherwise
    Notes  : only msg.hwnd, msg.message, msg.wParam, msg.lParam are used
    Version: v0.1.1.0+
  }
  MS_CLIST_HOTKEYSPROCESSMESSAGE:PAnsiChar = 'CList/HotkeysProcessMessage';

  {
    wParam : 0
    lParam : 0
    Affect : Toggles the show/hide status of the contact list
    Returns: 0 on success, [non zero] on failure
    Version: v0.1.1.0+
  }
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

const
  {
    wParam : TMCONTACT
    lParam : 0
    Affect : Do the message processing associated with the double clicking a contact
    Returns: 0 on success, [non zero] on failure
    Version: v0.1.1.0+
  }
  MS_CLIST_CONTACTDOUBLECLICKED:PAnsiChar = 'CList/ContactDoubleClicked';

  {
    wParam : TMCONTACT
    lParam : Pointer to an array of PAnsiChar's containing files/dirs
    Affect : Do the processing when some files are droppeed on a contact, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : the array is terminated when a NULL(0) entry is found
    Version: v0.1.2.1+
  }
  MS_CLIST_CONTACTFILESDROPPED:PAnsiChar = 'CList/ContactFilesDropped';

  {
    wParam : TMCONTACT
    lParam : HGROUP
    Affect : Change the group a contact belongs to, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : use hGroup=NULL(0) to remove any group association with the contact
    Version: v0.1.1.0+
  }
  MS_CLIST_CONTACTCHANGEGROUP:PAnsiChar = 'CList/ContactChangeGroup';

  {
    wParam : HCONTACT_1
    lParam : HCONTACT_2
    Affect : Determine the ordering of two given contacts
    Returns: 0 if hContact1 is the same as hContact2
             1 if hContact1 should be displayed before hContact2
            -1 if hContact1 should be displayed after hCotnact2
    Version: v0.1.1.0+
  }
  MS_CLIST_CONTACTSCOMPARE:PAnsiChar = 'CList/ContactsCompare';

  {
    wParam=0 (not used)
    lParam=(LPARAM) &MIRANDASYSTRAYNOTIFY

    Affects: Show a message in a ballon tip against a protocol icon (if installed)
    Returns: 0 on success, non zero on failure
    Notes  : This service will not be created on systems that haven't got the Windows
         support for ballontips, also note that it's upto Windows if it shows your
         message and it keeps check of delays (don't be stupid about showing messages)

    Version: 0.3.1a
  }
  NIIF_INFO           = $00000001;
  NIIF_WARNING        = $00000002;
  NIIF_ERROR          = $00000003;
  NIIF_ICON_MASK      = $0000000F;
  NIIF_NOSOUND        = $00000010;
  NIIF_INTERN_UNICODE = $00000100;

type
  PMIRANDASYSTRAYNOTIFY = ^TMIRANDASYSTRAYNOTIFY;
  TMIRANDASYSTRAYNOTIFY = record
    cbSize     :int;       // sizeof(MIRANDASYSTRAY)
    szProto    :PAnsiChar; // protocol to show under (may have no effect)
    szInfoTitle:TChar;     // only 64chars of it will be used, TCHAR if NIIF_INTERN_UNICODE is specified
    szInfo     :TChar;     // only 256chars of it will be used, TCHAR if NIIF_INTERN_UNICODE is specified
    dwInfoFlags:dword;     // see NIIF_* stuff
    uTimeout   :uint;      // how long to show the tip for
  end;

const
  MS_CLIST_SYSTRAY_NOTIFY:PAnsiChar = 'Miranda/Systray/Notify';

const
  SETTING_TOOLWINDOW_DEFAULT      = 1;
  SETTING_SHOWMAINMENU_DEFAULT    = 1;
  SETTING_SHOWCAPTION_DEFAULT     = 1;
  SETTING_CLIENTDRAG_DEFAULT      = 0;
  SETTING_ONTOP_DEFAULT           = 1;
  SETTING_MIN2TRAY_DEFAULT        = 1;
  SETTING_TRAY1CLICK_DEFAULT      = 0;
  SETTING_HIDEOFFLINE_DEFAULT     = 0;
  SETTING_HIDEEMPTYGROUPS_DEFAULT = 0;
  SETTING_USEGROUPS_DEFAULT       = 1;
  SETTING_SORTBYSTATUS_DEFAULT    = 0;
  SETTING_SORTBYPROTO_DEFAULT     = 0;
  SETTING_TRANSPARENT_DEFAULT     = 0;
  SETTING_ALPHA_DEFAULT           = 200;
  SETTING_AUTOALPHA_DEFAULT       = 150;
  SETTING_CONFIRMDELETE_DEFAULT   = 1;
  SETTING_AUTOHIDE_DEFAULT        = 0;
  SETTING_HIDETIME_DEFAULT        = 30;
  SETTING_CYCLETIME_DEFAULT       = 4;
  SETTING_ALWAYSSTATUS_DEFAULT    = 0;
  SETTING_ALWAYSMULTI_DEFAULT     = 0;

  SETTING_TRAYICON_SINGLE         = 0;
  SETTING_TRAYICON_CYCLE          = 1;
  SETTING_TRAYICON_MULTI          = 2;
  SETTING_TRAYICON_DEFAULT        = SETTING_TRAYICON_SINGLE;

  SETTING_STATE_HIDDEN            = 0;
  SETTING_STATE_MINIMIZED         = 1;
  SETTING_STATE_NORMAL            = 2;
  SETTING_BRINGTOFRONT_DEFAULT    = 0;

{$ENDIF}
