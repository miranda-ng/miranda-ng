(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*)

{$IFNDEF M_CLIST}
{$DEFINE M_CLIST}

{$ifndef STATUSMODES}
    {$include statusmodes.inc}
{$endif}

const

    // for MS_CLIST_GETSTATUSMODEDESCRIPTION

    GSMDF_PREFIXONLINE                  = 1;  // prefix "Online :" for online submodes, e.g. 'away'

    // for MS_CLIST_ADDMAINMENUITEM

    CMIF_GRAYED                         = 1;
    CMIF_CHECKED                        = 2;
    CMIF_HIDDEN                         = 4;  // only works on contact menus
    CMIF_NOTOFFLINE                     = 8;  // item won't appear for contacts that are offline
    CMIF_NOTONLINE                      = 16; //          "      online
    CMIF_NOTONLIST                      = 32;  // item won't appear on standard contacts
    CMIF_NOTOFFLIST                     = 64;  // item won't appear on contacts that have the 'NotOnList' setting

    // for MS_CLIST_MODIFYMENUITEM

    CMIM_NAME                           = $80000000;
    CMIM_FLAGS	                        = $40000000;
    CMIM_ICON                           = $20000000;
    CMIM_HOTKEY                         = $10000000;
    CMIM_ALL                            = $F0000000;

    // for MS_CLIST_GETCONTACTDISPLAYNAME

    // will never return the user's custom name, even if that's the one to be displayed
    GCDNF_NOMYHANDLE                    = 1;

    // for MS_CLIST_ADDEVENT

    //flashes the icon even if the user is occupied, and puts the event
    // at the top of the queue
    CLEF_URGENT                         = 1;
    { icon will not flash forever, only a few times, e.g. online alert }
    CLEF_ONLYAFEW                       = 2;

    // for MS_CLIST_GETICONSIMAGELIST

    IMAGE_GROUPOPEN                     = 11;
    IMAGE_GROUPSHUT                     = 12;

    // for MS_CLIST_MENUPROCESSCOMMAND

    MPCF_CONTACTMENU                    = 1; // test commands from a contact menu
    MPCF_MAINMENU                       = 2; // test commands from the main menu

    // for MS_CLIST_GROUPGETNAME/2

    GROUPF_EXPANDED                     = $04;
    GROUPF_HIDEOFFLINE                  = $08;

    //

    SETTING_TOOLWINDOW_DEFAULT          = 1;
    SETTING_SHOWMAINMENU_DEFAULT        = 1;
    SETTING_SHOWCAPTION_DEFAULT         = 1;
    SETTING_CLIENTDRAG_DEFAULT          = 0;
    SETTING_ONTOP_DEFAULT               = 1;
    SETTING_MIN2TRAY_DEFAULT            = 1;
    SETTING_TRAY1CLICK_DEFAULT          = 0;
    SETTING_HIDEOFFLINE_DEFAULT         = 0;
    SETTING_HIDEEMPTYGROUPS_DEFAULT     = 0;
    SETTING_USEGROUPS_DEFAULT           = 1;
    SETTING_SORTBYSTATUS_DEFAULT        = 0;
    SETTING_TRANSPARENT_DEFAULT         = 0;
    SETTING_ALPHA_DEFAULT               = 200;
    SETTING_AUTOALPHA_DEFAULT           = 150;
    SETTING_CONFIRMDELETE_DEFAULT       = 1;
    SETTING_AUTOHIDE_DEFAULT            = 0;
    SETTING_HIDETIME_DEFAULT            = 30;
    SETTING_CYCLETIME_DEFAULT           = 4;
    SETTING_ALWAYSSTATUS_DEFAULT        = 0;
    SETTING_ALWAYSMULTI_DEFAULT         = 0;
    SETTING_TRAYICON_SINGLE             = 0;
    SETTING_TRAYICON_CYCLE              = 1;
    SETTING_TRAYICON_MULTI              = 2;
    SETTING_TRAYICON_DEFAULT            = SETTING_TRAYICON_SINGLE;
    SETTING_STATE_HIDDEN                = 0;
    SETTING_STATE_MINIMIZED             = 1;
    SETTING_STATE_NORMAL                = 2;

type

    PCLISTMENUITEM = ^TCLISTMENUITEM;
    TCLISTMENUITEM = record
        cbSize: int;        // size in bytes of this structure
        pszName: PChar;     // text of the menu item
        flags: DWORD;
        position: int;      // approx position on the menu, lower numbers go nearer the top
        hIcon: THandle;     // icon to put by the item, if this was *not* loaded from
                            // a resource, you can delete it straight after the call
        pszService: PChar;  // name of the service to call when the service is clicked
        pszPopupName: PChar;// name of the popup menu that this item is on, if this
                            // is NULL the iteem is on the root of the menu
        popupPosition: int; // position of the popup menu on the root menu, ignored
                            // if pszPopupName is NULL(0) or if the popup menu already exists
        hotKey: DWORD;      // keyboard accelerator, same as lParam of WM_HOTKEY, 0 for none
        pszContactOwner: PChar; // contact menus only, the protocol module that owns
                                // the contacts to which this to which this menu item
                                // applies, NULL(0) if it applies to all contacts.
                                // if it applies to multiple but not all protocols
                                // add multiple menu items or use ME_CLIST_PREBUILDCONTACTMENU
    end;

    PCLISTDOUBLECLICKACTION = ^TCLISTDOUBLECLICKACTION;
    TCLISTDOUBLECLICKACTION = record
        cbSize: int;
        pszContactOwner: PChar;     // name of the protocol owning the contact or NULL(0) for all
        flags: DWORD;               // CMIF_NOT flags above
        pszService: PChar;          // service to call on double click, is called with wParam=hContact, lParam=0
    end;

    PCLISTEVENT = ^TCLISTEVENT;
    TCLISTEVENT = record
        cbSize: int;                // size in bytes
        hContact: THandle;          // handle to the contact to put the icon by
        hIcon: THandle;             // icon to flash!
        flags: DWORD;
        hDBEvent: THandle;          // caller defined, but should be unique for hContact
        lParam: LPARAM;
        pszService: PChar;          // name of service to call on activation
        pszTooltip: PChar;          // short description of the event to display as a tooltip on the systray
    end;

const

    {
        wParam : new_status
        lParam : 0
        Affect : Sent when the user acks to change their status, see notes
        Notes  : Also sent due to a MS_CLIST_SETSTATUSMODE
    }
    ME_CLIST_STATUSMODECHANGE           = 'CList/StatusModeChange';

    {
        wParam : new_status
        lParam : 0
        Affect : Force a change of status mode, see statusmodes.inc
    }
    MS_CLIST_SETSTATUSMODE			    = 'CList/SetStatusMode';

    {
        wParam : 0
        lParam : 0
        Affect : Get the current status mode, see notes
        Notes  : This is the status, as set by the user, not any protocol specific status
                 all protocol modules will attempt to conform to this setting at ALL times.
    }
    MS_CLIST_GETSTATUSMODE			    = 'CList/GetStatusMode';

    {
        wParam : status_mode
        lParam : flags
        Affect : Get a textual description of the given status mode
        Returns: pointer to a static buffer of the description of the given status mode
                 or NULL(0) if the mode was unknown.
        Version: v0.1.0.1+
    }
    MS_CLIST_GETSTATUSMODEDESCRIPTION   = 'CList/GetStatusModeDescription';

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
    MS_CLIST_ADDMAINMENUITEM            = 'CList/AddMainMenuItem';

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
                 position numbers will be displayed in brackets afterr the menu item
                 text, this only works in debug builds!
    }
    MS_CLIST_ADDCONTACTMENUITEM         = 'CList/AddContactMenuItem';

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
    MS_CLIST_MODIFYMENUITEM             = 'CList/ModifyMenuItem';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : the context menu for a contact is about to be built, see notes
        Notes  : modules should use this to change menu items that are specific
                 to the contact that has them
        Version: v0.1.0.1+
    }
    ME_CLIST_PREBUILDCONTACTMENU        = 'CList/PreBuildContactMenu';

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
    MS_CLIST_SETDOUBLECLICKACTION       = 'CList/SetDoubleClickAction';

    {
        wParam : MCONTACT
        lParam : <none>
        Affect : Register with this event to be notified of a double click on the CList
                 against a MCONTACT, you will not be notified if there is a pending CList event
                 that the double click clears, (i.e. flashing icon is presented to be clicked)
        Version: 0.3.0.0
    }
    ME_CLIST_DOUBLECLICKED              = 'CList/DoubleClicked';

    {
        wParam : MCONTACT
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
    MS_CLIST_GETCONTACTDISPLAYNAME      = 'CList/GetContactDisplayName';

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
    MS_CLIST_ADDEVENT                   = 'CList/AddEvent';

    {
        wParam : MCONTACT
        lParam : HDBEVENT
        Affect : Remove an event from the contact list queue
        Returns: 0 on success, [non zero] on failure
    }
    MS_CLIST_REMOVEEVENT                = 'Clist/RemoveEvent';

    {
        wParam : MCONTACT
        lParam : iEvent
        Affect : Get the details of an event in the queue, see notes
        Returns: A CLISTEVENT* or NULL(0) on failure
        Notes  : Returns the iEvent'1st/2nd/3rd/nth elemented queried,
                 e.g. iEvent=0 will get the event that will be returned if the
                 user double clicks on that MCONTACT
                 -
                 Use MCONTACT=NULL, iEvent=0 for example to get the event
                 the user will get if they double click on the tray.
        Version: v0.1.2.1+
    }
    MS_CLIST_GETEVENT                   = 'CList/GetEvent';

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
    MS_CLIST_MENUMEASUREITEM            = 'CList/MenuMeasureItem';

    {
        wParam :
        lParam :
        Affect : Process a WM_DRAWITEM message for user context menus,
                 wParam, lParam should be passed from such message handler.
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUDRAWITEM               = 'CList/MenuDrawItem';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Built the context menu for a specific contact
        Returns: A HMENU handle identifying the menu, thhis should be DestroyMenu()ed
                 when done.
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUBUILDCONTACT           = 'CList/MenuBuildContact';

    {
        wParam : 0
        lParam : 0
        Affect : Get the image list handle with all the useful icons in it
        Version: v0.1.1.0+
    }
    MS_CLIST_GETICONSIMAGELIST          = 'CList/GetIconsImageList';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Get the icon that should be associated with a contact
        Returns: an index into the contact list imagelist, if the icon
                 is a flashing icon, this service won't return information about it
                 see below
        Version: v0.1.2.0+
    }
    MS_CLIST_GETCONTACTICON             = 'CList/GetContactIcon';

    {
        wParam : MCONTACT
        lParam : ICON_ID
        Affect : The icon of a contact in the contact list has changed,
                 ICON_ID is an index to what image has changed
        Version: v0.1.2.1+
    }
    ME_CLIST_CONTACTICONCHANGED         = 'CList/ContactIconChanged';

    // ideally only used by a CLIST UI module

    {
        wParam : 0
        lParam : 0
        Affect : Get the handle to Miranda's main menu
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUGETMAIN                = 'CList/MenuGetMain';

    {
        wParam : 0
        lParam : 0
        Affect : Get a handle to Miranda's status menu
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUGETSTATUS              = 'CList/MenuGetStatus';

    {
        wParam : MAKEWPARAM(LOWORD(wParam of WM_COMMAND),flags)
        lParam : MCONTACT
        Affect : Process a mennu selection from a menu, see notes
        Returns: True if it processed the command, False otherwise
        notes  : hContact is the currently selected contact, it is not used
                 if this is a main menu command, if this is NULL then the command
                 is a contact menu one, the command is ignored
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUPROCESSCOMMAND         = 'CList/MenuProcessCommand';

    {
        wParam : virtual key code
        lParam : MPCF_* flags
        Affect : Process a menu hotkey, see notes
        Returns: True if it processed the command, False otherwise
        Notes  : this should be called in WM_KEYDOWN
        Version: v0.1.1.0+
    }
    MS_CLIST_MENUPROCESSHOTKEY          = 'CList/MenuProcessHotkey';

    {
        wParam : Pointer to a MSG structurer
        lParam : Pointer to an LRESULT
        Affect : Process all the messages required for docking, see notes
        Returns: True if the message should NOT be processed anymore, False otherwise
        Notes  : only msg.hwnd, msg.message, msg.wParam and msg.lParam are used
                 your WndProc should return the lResult if AND only IF, TRUE is returned
        Version: v0.1.1.0+
    }
    MS_CLIST_DOCKINGPROCESSMESSAGE      = 'CList/DockingProcessMessage';

    {
        wParam : 0
        lParam : 0
        Affect : Determines wheter the contact list docked
        Returns: pnon zero] if the contact list is docked, or 0 if it's not
        Version: v0.1.1.0+
    }
    MS_CLIST_DOCKINGISDOCKED            = 'CList/DockingIsDocked';

    {
        wParam : Pointer to a TMSG
        lParam : Pointer to an LRESULT
        Affect : Process all the messages required for the tray icon, see notes
        Returns: TRUE if the message should not be processed anymore, False otherwise
        Notes  : Only msg.hwnd, msg.message, msg.wparam and msg.lParam are used
                 your WndProc should return LRESULT if and ONLY if TRUE is returned
        Version: v0.1.1.0+
    }
    MS_CLIST_TRAYICONPROCESSMESSAGE     = 'CList/TrayIconProcessMessage';

    {
        wParam : Pointer to TMSG
        lParam : Pointer to an LRESULT
        Affect : Process all the messages required for hotkeys, see notes
        Returns: True if the message should not be processed anymore or False otherwise
        Notes  : only msg.hwnd, msg.message, msg.wParam, msg.lParam are used
        Version: v0.1.1.0+
    }
    MS_CLIST_HOTKEYSPROCESSMESSAGE      = 'CList/HotkeysProcessMessage';

    {
        wParam : 0
        lParam : 0
        Affect : Toggles the show/hide status of the contact list
        Returns: 0 on success, [non zero] on failure
        Version: v0.1.1.0+
    }
    MS_CLIST_SHOWHIDE                   = 'CList/ShowHide';

    {
        wParam : 0
        lParam : 0
        Affect : temporarily disable the autohide feature, see notes
        Notes  : this service will restart the auto hide timer, so if you need
                 to keep the window visible you'll have to bee getting user input
                 or calling this service each time
        Version: v0.1.2.1+
    }
    MS_CLIST_PAUSEAUTOHIDE              = 'CList/PauseAutoHide';

    {
        wParam : HPARENTGROUP
        lParam : 0
        Affect : Create a new group and calls CLUI to display it, see notes
        Returns: A handle to the new group.
        Notes  : If HPARENTGROUP is NULL(0) it will create a group at the root.
        Version: v0.1.1.0+
    }
    MS_CLIST_GROUPCREATE                = 'CList/GroupCreate';

    {
        wParam : HGROUP
        lParam : 0
        Affect : Delete a group and call CLUI to display the change
        Returns: 0 on success, [non zero] on failure
        Version: v0.1.1.0+
    }
    MS_CLIST_GROUPDELETE                = 'CList/GroupDelete';

    {
        wParam : HGROUP
        lParam : newState
        Affect : Change the expanded state flag for a group internally, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : if newState is non zero then the group is expanded, 0 it's collapsed
                 CLUI IS *NOT* called when the change is made.
        Version: v0.1.1.0+
    }
    MS_CLIST_GROUPSETEXPANDED           = 'CList/GroupSetExpanded';

    {
        wParam : HGROUP
        lParam : MAKELPARAM(flags, flagsMask)
        Affect : Change the flag for a group, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : only if flags given in flagsmask are altered,
                 CLUI is called on change to GROUPF_HIDEOFFLINE
        Version: v0.1.2.1+
    }
    MS_CLIST_GROUPSETFLAGS              = 'CList/GroupSetFlags';

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
    MS_CLIST_GROUPGETNAME               = 'CList/GroupGetName';

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
    MS_CLIST_GROUPGETNAME2              = 'CList/GroupGetName2';

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
    MS_CLIST_GROUPMOVEBEFORE            = 'CList/GroupMoveBefore';

    {
        wParam : HGROUP
        lParam : Pointer to a null terminated string containing the new name
        Affect : Rename a group internally, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : this will fail if the group name is a duplicate of an existing
                 a name, CLUI is not called when this change is made.
        Version: v0.1.1.0+
    }
    MS_CLIST_GROUPRENAME                = 'CList/GroupRename';

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
    MS_CLIST_GROUPBUILDMENU             = 'CList/GroupBuildMenu';

    {
        wParam : newValue
        lParam : 0
        Affect : Changes the 'hide offline contacts' flag and calls CLUI, see notes
        Returns: 0 success, [non zero] on failure
        Notes  : newValue is 0 to show all contacts, 1 to show only online contacts
                 -1 to toggle the value
        Version: v0.1.1.0+
    }
    MS_CLIST_SETHIDEOFFLINE             = 'CList/SetHideOffline';

    {
        wParam : MCONTACT
        lParam : 0
        Affect : Do the message processing associated with the double clicking a contact
        Returns: 0 on success, [non zero] on failure
        Version: v0.1.1.0+
    }
    MS_CLIST_CONTACTDOUBLECLICKED       = 'CList/ContactDoubleClicked';

    {
        wParam : MCONTACT
        lParam : Pointer to an array of pchar's containing files/dirs
        Affect : Do the processing when some files are droppeed on a contact, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : the array is terminated when a NULL(0) entry is found
        Version: v0.1.2.1+
    }
    MS_CLIST_CONTACTFILESDROPPED        = 'CList/ContactFilesDropped';

    {
        wParam : MCONTACT
        lParam : HGROUP
        Affect : Change the group a contact belongs to, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : use hGroup=NULL(0) to remove any group association with the contact
        Version: v0.1.1.0+
    }
    MS_CLIST_CONTACTCHANGEGROUP         = 'CList/ContactChangeGroup';

    {
        wParam : HCONTACT_1
        lParam : HCONTACT_2
        Affect : Determine the ordering of two given contacts
        Returns: 0 if hContact1 is the same as hContact2
                 1 if hContact1 should be displayed before hContact2
                -1 if hContact1 should be displayed after hCotnact2
        Version: v0.1.1.0+
    }
    MS_CLIST_CONTACTSCOMPARE            = 'CList/ContactsCompare';

{$ENDIF}
