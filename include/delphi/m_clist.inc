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

  GSMDF_UNICODE      = 2; // will return TCHAR* instead of AnsiChar*
  GSMDF_UNTRANSLATED = 4;

  {
    wParam : status_mode
    lParam : flags
    Affect : Get a textual description of the given status mode
    Returns: pointer to a static buffer of the description of the given status mode
             or NULL(0) if the mode was unknown.
    Version: v0.1.0.1+
  }
  MS_CLIST_GETSTATUSMODEDESCRIPTION:PAnsiChar = 'CList/GetStatusModeDescription';

type
// WARNING: do not use Translate(TS) for p(t)szName or p(t)szPopupName as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
  PCLISTMENUITEM = ^TCLISTMENUITEM;
  TCLISTMENUITEM = record
    cbSize         : int;     // size in bytes of this structure
    szName         : TChar;   // [TRANSLATED-BY-CORE] text of the menu item
    flags          : dword;
    position       : int;     // approx position on the menu, lower numbers go nearer the top
    hIcon          : THANDLE; // icon to put by the item, if this was *not* loaded from
                              // a resource, you can delete it straight after the call
    //icolibItem:THANDLE;  if CMIF_ICONFROMICOLIB used
    pszService     : PAnsiChar;   // name of the service to call when the service is clicked
    szPopupName    : TChar;   // [TRANSLATED-BY-CORE] name of the popup menu that this item is on, if this
                              // is NULL the iteem is on the root of the menu
		// or hParentMenu:HGENMENU - valid if CMIF_ROOTHANDLE is set. NULL or (HGENMENU)-1 means the root menu
    popupPosition  : int;     // position of the popup menu on the root menu, ignored
                              // if pszPopupName is NULL(0) or if the popup menu already exists
    hotKey         : dword;   // keyboard accelerator, same as lParam of WM_HOTKEY, 0 for none
    pszContactOwner: PAnsiChar;   // contact menus only, the protocol module that owns
                              // the contacts to which this to which this menu item
                              // applies, NULL(0) if it applies to all contacts.
                              // if it applies to multiple but not all protocols
                              // add multiple menu items or use ME_CLIST_PREBUILDCONTACTMENU
    hLangpack      : int;     // plugin's hLangpack (added automatically)
  end;

const
  HGENMENU_ROOT = HGENMENU(-1);

const
  CMIF_GRAYED     = 1;
  CMIF_CHECKED    = 2;
  CMIF_HIDDEN     = 4;   // only works on contact menus
  CMIF_NOTOFFLINE = 8;   // item won't appear for contacts that are offline
  CMIF_NOTONLINE  = 16;  //          "      online
  CMIF_NOTONLIST  = 32;  // item won't appear on standard contacts
  CMIF_NOTOFFLIST = 64;  // item won't appear on contacts that have the 'NotOnList' setting
  CMIF_ROOTHANDLE = 384; // means that hParentMenu member is set (since 0.8#26)
  CMIF_UNICODE    = 512; // will return TCHAR* instead of AnsiChar*
  CMIF_KEEPUNTRANSLATED = 1024; // don't translate a menu item

  CMIF_DEFAULT = 4096; // this menu item is the default one

// for compatibility. since 0.8.0 they both mean nothing
  CMIF_ROOTPOPUP  = 128; // root item for new popup (save return id for childs)
  CMIF_CHILDPOPUP = 256; // child for rootpopup menu

  {
    wParam : 0
    lParam : Pointer to a initalised TCLISTMENUITEM structure
    Affect : Add a new menu item to the main menu, see notes
    Returns: A handle to the new MENU item or NULL(0) on failure
    Notes  : The given TCLISTMENUITEM.pszService in is called when the item
             get clicked with :
             -
             wParam = 0, lParam = hwndContactList
  }
  MS_CLIST_ADDMAINMENUITEM:PAnsiChar = 'CList/AddMainMenuItem';

  {
    wParam : 0
    lParam : Pointer to a initalised TCLISTMENUITEM structure
    Affect : Add a new item to the user contact menus, see notes
    Notes  : exactly the same as MS_CLIST_ADDMAINMENUITEM except when an item
             is selected, the service gets called with wParam=hContact,
             pszContactOwner is obeyed.
             -
             Popup menus are not supported, pszPopupName and popupPosition
             are ignored. If CTRL is held down when right clicking the menu
             position numbers will be displayed in brackets after the menu item
             text, this only works in debug builds!
  }
  MS_CLIST_ADDCONTACTMENUITEM:PAnsiChar = 'CList/AddContactMenuItem';
  MS_CLIST_ADDSTATUSMENUITEM :PAnsiChar = 'CList/AddStatusMenuItem';
//adds a protocol menu item        v0.9+
  MS_CLIST_ADDPROTOMENUITEM  :PAnsiChar = 'CList/AddProtoMenuItem';

  CMIM_NAME   = $80000000;
  CMIM_FLAGS  = $40000000;
  CMIM_ICON   = $20000000;
  CMIM_HOTKEY = $10000000;
  CMIM_ALL    = $F0000000;

  {
    wParam : HMENUITEM
    lParam : Pointer to a initalised TCLISTMENUITEM
    Affect : Modify an existing menu item, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : hMenuItem will have been returned by MS_CLIST_ADD[MAIN]MENUITEM
             TCLISTMENUITEM.flags should contain CMIM_* constants (see above)
             to mark which fields should be updated, if it's not present, they
             can't be updated -- if flags do not exist for a field it can not
             be updated.
    Version: v0.1.0.1+
  }
  MS_CLIST_MODIFYMENUITEM:PAnsiChar = 'CList/ModifyMenuItem';

  {
    wParam : (HGENMENU)hMenuItem
    lParam : (bool) enable = TRUE, disable = FALSE
    Notes  : changes menu item's visibility
    Version: v0.94.2+
  }
  MS_CLIST_SHOWHIDEMENUITEM:PAnsiChar = 'CList/ShowHideMenuItem';

  {
    wParam : TMCONTACT
    lParam : 0
    Affect : the context menu for a contact is about to be built, see notes
    Notes  : modules should use this to change menu items that are specific
             to the contact that has them
    Version: v0.1.0.1+
  }
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

  GCDNF_NOMYHANDLE = 1; // will never return the user's custom name
  GCDNF_UNICODE    = 2; // will return TCHAR* instead of AnsiChar*
  GCDNF_NOCACHE    = 4; // will not use the cache
  {
    wParam : TMCONTACT
    lParam : flags
    Affect : Gets the string that the contact list will use to represent a contact
    Returns: Always a pointer
    Notes  : Returns a pointer to the name, will always succeed, even if it needs
             to return "(Unknown Contact)"
             -
             this pointer is a statically allocated buffer which will
             be overwritten on every call to this service, callers should make
             sure that they copy the information before they call it again
    Version: v0.1.2.0+, 0.2.0+ ONLY (0.3a supports the contacts module)
  }
  MS_CLIST_GETCONTACTDISPLAYNAME:PAnsiChar = 'CList/GetContactDisplayName';

  {
    Invalidates the display name cache
    wParam=(WPARAM)(HANDLE)hContact
    lParam=not used
  }
  MS_CLIST_INVALIDATEDISPLAYNAME:PAnsiChar = 'CList/InvalidateDiplayName';

type
  PCLISTEVENT = ^TCLISTEVENT;
  TCLISTEVENT = record
    cbSize    : int;       // size in bytes
    hContact  : TMCONTACT; // handle to the contact to put the icon by
    hIcon     : HICON;     // icon to flash!
    flags     : dword;
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
    wParam : 0
    lParam : Pointer to a TCLISTEVENT
    Affect : Add's an event to the list
    Notes  : The service will flash TCLISTEVENT.hIcon, next to the
             contact, TCLISTEVENT.hContact
             -
             pszService is called is called wParam=hwndContactList,
             lParam=pointer to a TCLISTEVENT.
             -
             the TCLISTEVENT data is invalidated after this service returns
             so copy anything from it if required.
             -
             TCLISTEVENT.pszService will also be called if the user
             double clicks on the icon, at which point it will be removed
             from the contact lists queue automatically.
             -
             TCLISTEVENT.hContact and TCLISTEVENT.hDBEvent should be unique.
  }
  MS_CLIST_ADDEVENT:PAnsiChar = 'CList/AddEvent';

  {
    wParam : TMCONTACT
    lParam : TMEVENT
    Affect : Remove an event from the contact list queue
    Returns: 0 on success, [non zero] on failure
  }
  MS_CLIST_REMOVEEVENT:PAnsiChar = 'Clist/RemoveEvent';

  {
    wParam : TMCONTACT
    lParam : iEvent
    Affect : Get the details of an event in the queue, see notes
    Returns: A CLISTEVENT* or NULL(0) on failure
    Notes  : Returns the iEvent'1st/2nd/3rd/nth elemented queried,
             e.g. iEvent=0 will get the event that will be returned if the
             user double clicks on that TMCONTACT
             -
             Use TMCONTACT=NULL, iEvent=0 for example to get the event
             the user will get if they double click on the tray.
    Version: v0.1.2.1+
  }
  MS_CLIST_GETEVENT:PAnsiChar = 'CList/GetEvent';

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
    wParam : TMCONTACT
    lParam : 0
    Affect : Built the context menu for a specific contact
    Returns: A HMENU handle identifying the menu, this should be DestroyMenu()ed
             when done.
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUBUILDCONTACT:PAnsiChar = 'CList/MenuBuildContact';

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
    lParam : 0
    Affect : Get the icon that should be associated with a contact
    Returns: an index into the contact list imagelist, if the icon
             is a flashing icon, this service won't return information about it
             see below
    Version: v0.1.2.0+
  }
  MS_CLIST_GETCONTACTICON:PAnsiChar = 'CList/GetContactIcon';

  {
    wParam : TMCONTACT
    lParam : ICON_ID
    Affect : The icon of a contact in the contact list has changed,
             ICON_ID is an index to what image has changed
    Version: v0.1.2.1+
  }
  ME_CLIST_CONTACTICONCHANGED:PAnsiChar = 'CList/ContactIconChanged';

//******************************* CLUI only *********************************

  {
    wParam : 0
    lParam : 0
    Affect : Get the handle to Miranda's main menu
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUGETMAIN:PAnsiChar = 'CList/MenuGetMain';

  {
    wParam : 0
    lParam : 0
    Affect : Get a handle to Miranda's status menu
    Version: v0.1.1.0+
  }
  MS_CLIST_MENUGETSTATUS:PAnsiChar = 'CList/MenuGetStatus';

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

  {
    checks that a group exists
    wParam : 0 (unused)
    lParam : (PWideChar)groupName
    returns 0 if a group is not found or group handle on success
  }
  MS_CLIST_GROUPEXISTS:PAnsiChar = 'CList/GroupExists';

  {
    wParam : HPARENTGROUP
    lParam : 0 or Pointer to new group name
    Affect : Create a new group and calls CLUI to display it, see notes
    Returns: A handle to the new group.
    Notes  : If HPARENTGROUP is NULL(0) it will create a group at the root.
    Version: v0.1.1.0+
  }
  MS_CLIST_GROUPCREATE:PAnsiChar = 'CList/GroupCreate';

  {
    wParam : HGROUP
    lParam : 0
    Affect : Delete a group and call CLUI to display the change
    Returns: 0 on success, [non zero] on failure
    Version: v0.1.1.0+
  }
  MS_CLIST_GROUPDELETE:PAnsiChar = 'CList/GroupDelete';

  {
    wParam : HGROUP
    lParam : newState
    Affect : Change the expanded state flag for a group internally, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : if newState is non zero then the group is expanded, 0 it's collapsed
             CLUI IS *NOT* called when the change is made.
    Version: v0.1.1.0+
  }
  MS_CLIST_GROUPSETEXPANDED:PAnsiChar = 'CList/GroupSetExpanded';

  {
    wParam : HGROUP
    lParam : MAKELPARAM(flags, flagsMask)
    Affect : Change the flag for a group, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : only if flags given in flagsmask are altered,
             CLUI is called on change to GROUPF_HIDEOFFLINE
    Version: v0.1.2.1+
  }
  MS_CLIST_GROUPSETFLAGS:PAnsiChar = 'CList/GroupSetFlags';

  {
    wParam : HGROUP
    lParam : Pointer to a integer to be filled with expanded state
    Affect : get the name of a group, see notes
    Returns: a static buffer pointing to the name of the group
             returns NULL(0) if HGROUP is invalid.
    Notes  : the returned buffer is only valid til the next call
             to this service, lParam can be NULL(0) if you don't
             want to know if the group is expanded
    Version: v0.1.1.0+
  }
  MS_CLIST_GROUPGETNAME:PAnsiChar = 'CList/GroupGetName';

  {
    wParam : HGROUP
    lParam : Pointer to flags
    Affect : Get the name of the group, see notes
    Returns: A static buffer pointing to the name of the group
             returns NULL(0) if HGROUP is invalid
    Note   : this buffer is only valid til the next call to this service
             flags can be NULL(0), otherwise it'll return GROUPF_* constants
    Version: v0.1.2.1+
  }
  MS_CLIST_GROUPGETNAME2:PAnsiChar = 'CList/GroupGetName2';

  GROUPF_EXPANDED    = $04;
  GROUPF_HIDEOFFLINE = $08;
  {
    wParam : HGROUP
    lParam : HBEFOREGROUP
    Affect : Move a group directly before another group
    Returns: the new handle of the group on success, NULL(0) on failure
    Notes  : the order is represented by the order in which MS_CLUI_GROUPADDED
             is called, however UI's are free to ignore this order and sort
             if they wish.
    Version: v0.1.2.1+
  }
  MS_CLIST_GROUPMOVEBEFORE:PAnsiChar = 'CList/GroupMoveBefore';

  {
    wParam : HGROUP
    lParam : Pointer to a null terminated string containing the new name
    Affect : Rename a group internally, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : this will fail if the group name is a duplicate of an existing
             a name, CLUI is not called when this change is made.
    Version: v0.1.1.0+
  }
  MS_CLIST_GROUPRENAME:PAnsiChar = 'CList/GroupRename';

  {
    wParam : 0
    lParam : 0
    Affect : Build a menu of the group tree, see notes
    Returns: Handle to the menu, NULL(0) on failure
    Notes  : NULL be returned if the user doesn't have any groups
             the dwItemData of every menu item is the handle to that group.
             Menu item ID's are assigned starting at 100 in no particular order
    Version: v0.1.2.1+
  }
  MS_CLIST_GROUPBUILDMENU:PAnsiChar = 'CList/GroupBuildMenu';

  {
    wParam : newValue
    lParam : 0
    Affect : Changes the 'hide offline contacts' flag and calls CLUI, see notes
    Returns: 0 success, [non zero] on failure
    Notes  : newValue is 0 to show all contacts, 1 to show only online contacts
             -1 to toggle the value
    Version: v0.1.1.0+
  }
  MS_CLIST_SETHIDEOFFLINE:PAnsiChar = 'CList/SetHideOffline';

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
