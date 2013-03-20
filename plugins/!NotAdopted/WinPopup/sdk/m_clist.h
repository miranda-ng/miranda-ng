/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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
*/

#ifndef M_CLIST_H__
#define M_CLIST_H__ 1

#ifdef _MSC_VER
	#pragma warning(disable:4201 4204)
#endif

#include "statusmodes.h"

#if defined _STATIC
	typedef struct _tagIntMenuItem* HGENMENU;
#else
	DECLARE_HANDLE(HGENMENU);
#endif

//sent when the user asks to change their status
//wParam=new status, from statusmodes.h
//lParam=protocol name, NULL if for all protocols (added in v0.3.1alpha)
//also sent due to a ms_clist_setstatusmode call
#define ME_CLIST_STATUSMODECHANGE       "CList/StatusModeChange"

//force a change of status mode
//wParam=new status, from statusmodes.h
#define MS_CLIST_SETSTATUSMODE			"CList/SetStatusMode"

//get the current status mode
//wParam=lParam=0
//returns the current status
//This is the status *as set by the user*, not any protocol-specific status
//All protocol modules will attempt to conform to this setting at all times
#define MS_CLIST_GETSTATUSMODE			"CList/GetStatusMode"

//gets a textual description of the given status mode (v0.1.0.1+)
//wParam=status mode, from statusmodes.h
//lParam=flags, below
//returns a static buffer of the description of the given status mode
//returns NULL if the status mode was unknown
#define GSMDF_PREFIXONLINE   1   //prefix "Online: " to all status modes that
                                 //imply online, eg "Online: Away"
#define GCMDF_UNICODE        2   //will return TCHAR* instead of char*
#if defined( _UNICODE )
	#define GCMDF_TCHAR       GCMDF_UNICODE      //will return TCHAR* instead of char*
#else
	#define GCMDF_TCHAR       0      //will return char*, as usual
#endif
#define GSMDF_UNTRANSLATED   4
#define MS_CLIST_GETSTATUSMODEDESCRIPTION  "CList/GetStatusModeDescription"

//add a new item to the main menu
//wParam=0
//lParam=(LPARAM)(CLISTMENUITEM*)&mi
//returns a handle to the new item, or NULL on failure
//the service that is called when the item is clicked is called with
//wParam=0, lParam=hwndContactList
//dividers are inserted every 100000 positions
//pszContactOwner is ignored for this service.
//there is a #define PUTPOSITIONSINMENU in clistmenus.c which, when set, will
//cause the position numbers to be placed in brackets after the menu items

// WARNING: do not use Translate(TS) for p(t)szName or p(t)szPopupName as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
typedef struct {
	int cbSize;	            //size in bytes of this structure
	union {
      char*  pszName;      //[TRANSLATED-BY-CORE] text of the menu item
		TCHAR* ptszName;     //Unicode text of the menu item
	};
	DWORD flags;            //set of CMIF_* flags
	int position;           //approx position on the menu. lower numbers go nearer the top
	union {
		HICON hIcon;         //icon to put by the item. If this was not loaded from
                           //a resource, you can delete it straight after the call
		HANDLE icolibItem;   //set CMIF_ICONFROMICOLIB to pass this value
	};
	char* pszService;       //name of service to call when the item gets selected
	union {
		char* pszPopupName;  //[TRANSLATED-BY-CORE] name of the popup menu that this item is on. If this
		TCHAR* ptszPopupName; //is NULL the item is on the root of the menu
		HGENMENU hParentMenu; // valid if CMIF_ROOTHANDLE is set. NULL or (HGENMENU)-1 means the root menu
	};

	int popupPosition;      //position of the popup menu on the root menu. Ignored
                           //if pszPopupName is NULL or the popup menu already
                           //existed
	DWORD hotKey;           //keyboard accelerator, same as lParam of WM_HOTKEY,0 for none
	char *pszContactOwner;  //contact menus only. The protocol module that owns
                           //the contacts to which this menu item applies. NULL if it
                           //applies to all contacts. If it applies to multiple but not all
                           //protocols, add multiple menu items or use ME_CLIST_PREBUILDCONTACTMENU
} CLISTMENUITEM;

#define HGENMENU_ROOT      (( HGENMENU )-1)

#define CMIF_GRAYED     1
#define CMIF_CHECKED    2
#define CMIF_HIDDEN     4    //only works on contact menus
#define CMIF_NOTOFFLINE 8	  //item won't appear for contacts that are offline
#define CMIF_NOTONLINE  16	  //          "      online
#define CMIF_NOTONLIST  32   //item won't appear on standard contacts
#define CMIF_NOTOFFLIST 64   //item won't appear on contacts that have the 'NotOnList' setting
#define CMIF_ROOTHANDLE 384  //means that hParentMenu member is set

#define CMIF_UNICODE        512      //will return TCHAR* instead of char*
#if defined( _UNICODE )
	#define CMIF_TCHAR       CMIF_UNICODE      //will return TCHAR* instead of char*
#else
	#define CMIF_TCHAR       0       //will return char*, as usual
#endif

#define CMIF_KEEPUNTRANSLATED  1024 // don't translate a menu item
#define CMIF_ICONFROMICOLIB    2048 // use icolibName instead of hIcon
#define CMIF_DEFAULT           4096 // this menu item is the default one

// for compatibility. since 0.8.0 they both mean nothing
#define CMIF_ROOTPOPUP  128   //root item for new popup(save return id for childs)
#define CMIF_CHILDPOPUP 256   //child for rootpopup menu

#define MS_CLIST_ADDMAINMENUITEM        "CList/AddMainMenuItem"

//add a new item to the user contact menus
//identical to clist/addmainmenuitem except when item is selected the service
//gets called with wParam=(WPARAM)(HANDLE)hContact
//pszContactOwner is obeyed.
//popup menus are not supported. pszPopupName and popupPosition are ignored.
//If ctrl is held down when right clicking, the menu position numbers will be
//displayed in brackets after the menu item text. This only works in debug
//builds.
#define MS_CLIST_ADDCONTACTMENUITEM     "CList/AddContactMenuItem"
#define MS_CLIST_ADDSTATUSMENUITEM      "CList/AddStatusMenuItem"

//modify an existing menu item     v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hMenuItem
//lParam=(LPARAM)(CLISTMENUITEM*)&clmi
//returns 0 on success, nonzero on failure
//hMenuItem will have been returned by clist/add*menuItem
//clmi.flags should contain cmim_ constants below specifying which fields to
//update. Fields without a mask flag cannot be changed and will be ignored
#define CMIM_NAME     0x80000000
#define CMIM_FLAGS	  0x40000000
#define CMIM_ICON     0x20000000
#define CMIM_HOTKEY   0x10000000
#define CMIM_ALL      0xF0000000
#define MS_CLIST_MODIFYMENUITEM         "CList/ModifyMenuItem"

//the context menu for a contact is about to be built     v0.1.0.1+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//modules should use this to change menu items that are specific to the
//contact that has them
#define ME_CLIST_PREBUILDCONTACTMENU    "CList/PreBuildContactMenu"

//sets the service to call when a contact is double-clicked
//wParam=0
//lParam=(LPARAM)(CLISTDOUBLECLICKACTION*)&dca
//contactType is one or more of the constants below
//pszService is called with wParam=hContact, lParam=0
//pszService will only be called if there is no outstanding event on the
//selected contact
//returns 0 on success, nonzero on failure
//in case of conflicts, the first module to have registered will get the
//double click, no others will. This service will return success even for
//duplicates.
/*
 Note: During development of 0.3.0.0 (2003/02/15) this service was completely dropped
 by default it always returns 1 to mark failure, see ME_CLIST_DOUBLECLICKED for
 a better implementation as a hook.
*/
typedef struct {
	int cbSize;
	char *pszContactOwner;	//name of protocol owning contact, or NULL for all
	DWORD flags;			//any of the CMIF_NOT... flags above
	char *pszService;		//service to call on double click
} CLISTDOUBLECLICKACTION;
#define MS_CLIST_SETDOUBLECLICKACTION   "CList/SetDoubleClickAction"

/*
wParam=(WPARAM)hContact
lParam=0

Event is fired when there is a double click on a CList contact,
it is upto the caller to check for the protocol & status
of the HCONTACT, it's not done for you anymore since it didn't make
sense to store all this information in memory, etc.

*/
#define ME_CLIST_DOUBLECLICKED "CList/DoubleClicked"
//gets the string that the contact list will use to represent a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=flags
//returns a pointer to the name, will always succeed, even if it needs to
//return "(Unknown Contact)"
//this pointer is to a statically allocated buffer which will be overwritten
//on every call to this service. Callers should make sure that they copy the
//information before they call this service again.
#define GCDNF_NOMYHANDLE     1      //will never return the user's custom name
#define GCDNF_UNICODE        2      //will return TCHAR* instead of char*
#define GCDNF_NOCACHE        4      //will not use the cache

#if defined( _UNICODE )
	#define GCDNF_TCHAR       GCDNF_UNICODE      //will return TCHAR* instead of char*
#else
	#define GCDNF_TCHAR       0      //will return char*, as usual
#endif

           //even if it's the one that should be displayed.  v0.1.2.0+
		   //v0.3.0.0+ if using GCDNF_NOMYHANDLE you must free your string
#define MS_CLIST_GETCONTACTDISPLAYNAME  "CList/GetContactDisplayName"

// Invalidates the display name cache
//wParam=(WPARAM)(HANDLE)hContact
//lParam=not used
#define MS_CLIST_INVALIDATEDISPLAYNAME  "CList/InvalidateDiplayName"

//adds an event to the contact list's queue
//wParam=0
//lParam=(LPARAM)(CLISTEVENT*)cle
//The contact list will flash hIcon next to the contact hContact (use NULL for
//a system message). szServiceName will be called when the user double clicks
//the icon, at which point the event will be removed from the contact list's
//queue automatically
//pszService is called with wParam=(WPARAM)(HWND)hwndContactList,
//lParam=(LPARAM)(CLISTEVENT*)cle. Its return value is ignored. cle is
//invalidated when your service returns, so take copies of any important
//information in it.
//hDbEvent should be unique since it and hContact are the identifiers used by
//clist/removeevent if, for example, your module implements a 'read next' that
//bypasses the double-click.
typedef struct {
	int cbSize;          //size in bytes of this structure
	HANDLE hContact;	 //handle to the contact to put the icon by
	HICON hIcon;		 //icon to flash
	DWORD flags;		 //...of course	
	union
	{
		HANDLE hDbEvent;	 //caller defined but should be unique for hContact
		char * lpszProtocol;
	};
	LPARAM lParam;		 //caller defined
	char *pszService;	 //name of the service to call on activation
	union {
		char  *pszTooltip;    //short description of the event to display as a
		TCHAR *ptszTooltip;    //tooltip on the system tray
	};
} CLISTEVENT;
#define CLEF_URGENT    1	//flashes the icon even if the user is occupied,
							//and puts the event at the top of the queue
#define CLEF_ONLYAFEW  2	//the icon will not flash for ever, only a few
							//times. This is for eg online alert
#define CLEF_UNICODE   4	//set pszTooltip as unicode

#define CLEF_PROTOCOLGLOBAL   8		//set event globally for protocol, hContact has to be NULL, 
									//lpszProtocol the protocol ID name to be set

#if defined( _UNICODE )
	#define CLEF_TCHAR       CLEF_UNICODE      //will use TCHAR* instead of char*
#else
	#define CLEF_TCHAR       0      //will return char*, as usual
#endif

#define MS_CLIST_ADDEVENT     "CList/AddEvent"

//removes an event from the contact list's queue
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(HANDLE)hDbEvent
//returns 0 if the event was successfully removed, or nonzero if the event
//was not found
#define MS_CLIST_REMOVEEVENT  "Clist/RemoveEvent"

//gets the details of an event in the queue             v0.1.2.1+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=iEvent
//returns a CLISTEVENT* on success, NULL on failure
//Returns the iEvent-th event from the queue for hContact, so iEvent=0 will
//get the event that will be got when the user clicks on that contact.
//Use hContact=INVALID_HANDLE_VALUE to search over all contacts, so iEvent=0
//will get the event that will be got if the user clicks the systray icon.
#define MS_CLIST_GETEVENT     "CList/GetEvent"

//process a WM_MEASUREITEM message for user context menus   v0.1.1.0+
//wParam, lParam, return value as for WM_MEASUREITEM
//This is for displaying the icons by the menu items. If you don't call this
//and clist/menudrawitem whne drawing a menu returned by one of the three menu
//services below then it'll work but you won't get any icons
#define MS_CLIST_MENUMEASUREITEM  "CList/MenuMeasureItem"

//process a WM_DRAWITEM message for user context menus      v0.1.1.0+
//wParam, lParam, return value as for WM_MEASUREITEM
//See comments for clist/menumeasureitem
#define MS_CLIST_MENUDRAWITEM     "CList/MenuDrawItem"

//builds the context menu for a specific contact            v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//returns a HMENU identifying the menu. This should be DestroyMenu()ed when
//finished with.
#define MS_CLIST_MENUBUILDCONTACT "CList/MenuBuildContact"

//gets the image list with all the useful icons in it     v0.1.1.0+
//wParam=lParam=0
//returns a HIMAGELIST
//the members of this image list are opaque, and you should trust what you
//are given
#define MS_CLIST_GETICONSIMAGELIST    "CList/GetIconsImageList"
#define IMAGE_GROUPOPEN     11
#define IMAGE_GROUPSHUT     12

//get the icon that should be associated with a contact     v0.1.2.0+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//returns an index into the contact list imagelist. See clist/geticonsimagelist
//If the contact is flashing an icon, this function will not return that
//flashing icon. Use me_clist_contacticonchanged to get info about that.
#define MS_CLIST_GETCONTACTICON   "CList/GetContactIcon"

//The icon of a contact in the contact list has changed    v0.1.2.0+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=iconId
//iconId is an offset into the clist's imagelist. See clist/geticonsimagelist
#define ME_CLIST_CONTACTICONCHANGED   "CList/ContactIconChanged"

/******************************* CLUI only *********************************/

// Stuff below here is ideally for the use of a CList UI module only.

//get a handle to the main Miranda menu						v0.1.1.0+
//wParam=lParam=0
//returns a HMENU. This need not be freed since it's owned by clist
#define MS_CLIST_MENUGETMAIN    "CList/MenuGetMain"

//get a handle to the Miranda status menu					v0.1.1.0+
//wParam=lParam=0
//returns a HMENU. This need not be freed since it's owned by clist
#define MS_CLIST_MENUGETSTATUS  "CList/MenuGetStatus"




//processes a menu selection from a menu                    v0.1.1.0+
//wParam=MAKEWPARAM(LOWORD(wParam from WM_COMMAND),flags)
//lParam=(LPARAM)(HANDLE)hContact
//returns TRUE if it processed the command, FALSE otherwise
//hContact is the currently selected contact. It it not used if this is a main
//menu command. If this is NULL and the command is a contact menu one, the
//command is ignored

#define CLISTMENUIDMIN	0x4000	  // reserved range for clist menu ids
#define CLISTMENUIDMAX	0x7fff
//////////////////////////////////////////////////////////////////////////
// NOTE:														v0.7.0.26+
// Due to it is generic practice to handle menu command via WM_COMMAND
// window message handle and practice to process it via calling service
// in form: CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) hContact))
// to ensure that WM_COMMAND was realy from clist menu not from other menu
// it is reserved range of menu ids from CLISTMENUIDMIN to CLISTMENUIDMAX
// the menu items with ids outside from such range will not be processed by service.
// Moreover if you process WM_COMMAND youself and your window contains self menu 
// please be sure that you will not call service for non-clist menu items.
// The simplest way is to ensure that your menus are not use item ids from such range.
// Otherwise, you HAVE TO distinguish WM_COMMAND from clist menus and from youê internal menu and
// DO NOT call MS_CLIST_MENUPROCESSCOMMAND for non clist menus. 


#define MPCF_CONTACTMENU   1	//test commands from a contact menu
#define MPCF_MAINMENU      2	//test commands from the main menu
#define MS_CLIST_MENUPROCESSCOMMAND "CList/MenuProcessCommand"

//processes a menu hotkey                                   v0.1.1.0+
//wParam=virtual key code
//lParam=MPCF_ flags
//returns TRUE if it processed the command, FALSE otherwise
//this should be called in WM_KEYDOWN
#define MS_CLIST_MENUPROCESSHOTKEY "CList/MenuProcessHotkey"

//process all the messages required for docking             v0.1.1.0+
//wParam=(WPARAM)(MSG*)&msg
//lParam=(LPARAM)(LRESULT*)&lResult
//returns TRUE if the message should not be processed further, FALSE otherwise
//only msg.hwnd, msg.message, msg.wParam and msg.lParam are used
//your wndproc should return lResult if and only if TRUE is returned
#define MS_CLIST_DOCKINGPROCESSMESSAGE  "CList/DockingProcessMessage"

//determines whether the contact list is docked             v0.1.1.0+
//wParam=lParam=0
//returns nonzero if the contact list is docked, of 0 if it is not
#define MS_CLIST_DOCKINGISDOCKED        "CList/DockingIsDocked"

//process all the messages required for the tray icon       v0.1.1.0+
//wParam=(WPARAM)(MSG*)&msg
//lParam=(LPARAM)(LRESULT*)&lResult
//returns TRUE if the message should not be processed further, FALSE otherwise
//only msg.hwnd, msg.message, msg.wParam and msg.lParam are used
//your wndproc should return lResult if and only if TRUE is returned
#define MS_CLIST_TRAYICONPROCESSMESSAGE  "CList/TrayIconProcessMessage"

//process all the messages required for hotkeys             v0.1.1.0+
//wParam=(WPARAM)(MSG*)&msg
//lParam=(LPARAM)(LRESULT*)&lResult
//returns TRUE if the message should not be processed further, FALSE otherwise
//only msg.hwnd, msg.message, msg.wParam and msg.lParam are used
//your wndproc should return lResult if and only if TRUE is returned
#define MS_CLIST_HOTKEYSPROCESSMESSAGE  "CList/HotkeysProcessMessage"

//toggles the show/hide status of the contact list          v0.1.1.0+
//wParam=lParam=0
//returns 0 on success, nonzero on failure
#define MS_CLIST_SHOWHIDE     "CList/ShowHide"

//temporarily disable the autohide feature         v0.1.2.1+
//wParam=lParam=0
//returns 0 on success, nonzero on failure
//This service will restart the autohide timer, so if you need to keep the
//window visible you'll have to be getting user input regularly and calling
//this function each time
#define MS_CLIST_PAUSEAUTOHIDE        "CList/PauseAutoHide"

//sent when the group get modified (created, renamed or deleted)
//or contact is moving from group to group 
//wParam=hContact - NULL if operation on group 
//lParam=pointer to CLISTGROUPCHANGE
typedef struct {
	int cbSize;	            //size in bytes of this structure
    TCHAR*  pszOldName;     //old group name
    TCHAR*  pszNewName;     //new group name
} CLISTGROUPCHANGE;

#define ME_CLIST_GROUPCHANGE       "CList/GroupChange"

//creates a new group and calls CLUI to display it          v0.1.1.0+
//wParam=hParentGroup
//lParam=groupName
//returns a handle to the new group
//hParentGroup is NULL to create the new group at the root, or can be the
//handle of the group of which the new group should be a subgroup.
//groupName is a TCHAR* pointing to the group name to create or NULL for 
//API to create unique name by itself
#define MS_CLIST_GROUPCREATE   "CList/GroupCreate"

//deletes a group and calls CLUI to display the change      v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=0
//returns 0 on success, nonzero on failure
#define MS_CLIST_GROUPDELETE   "CList/GroupDelete"

//change the expanded state flag for a group internally     v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=newState
//returns 0 on success, nonzero on failure
//newState is nonzero if the group is expanded, 0 if it's collapsed
//CLUI is not called when this change is made
#define MS_CLIST_GROUPSETEXPANDED  "CList/GroupSetExpanded"

//changes the flags for a group                             v0.1.2.1+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=MAKELPARAM(flags,flagsMask)
//returns 0 on success, nonzero on failure
//Only the flags given in flagsMask are altered.
//CLUI is called on changes to GROUPF_HIDEOFFLINE.
#define MS_CLIST_GROUPSETFLAGS   "CList/GroupSetFlags"

//get the name of a group                                   v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=(LPARAM)(int*)&isExpanded
//returns a static buffer pointing to the name of the group
//returns NULL if hGroup is invalid.
//this buffer is only valid until the next call to this service
//&isExpanded can be NULL if you don't want to know if the group is expanded
//or not.
#define MS_CLIST_GROUPGETNAME      "CList/GroupGetName"

//get the name of a group                                   v0.1.2.1+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=(LPARAM)(DWORD*)&flags
//returns a static buffer pointing to the name of the group
//returns NULL if hGroup is invalid.
//this buffer is only valid until the next call to this service
//&flags can be NULL if you don't want any of that info.
#define GROUPF_EXPANDED    0x04
#define GROUPF_HIDEOFFLINE 0x08
#define MS_CLIST_GROUPGETNAME2      "CList/GroupGetName2"

//move a group to directly before another group             v0.1.2.1+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=(LPARAM)(HANDLE)hBeforeGroup
//returns the new handle of the group on success, NULL on failure
//The order is represented by the order in which MS_CLUI_GROUPADDED is called,
//however UIs are free to ignore this order and sort alphabetically if they
//wish.
#define MS_CLIST_GROUPMOVEBEFORE   "CList/GroupMoveBefore"

//rename a group internally									v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hGroup
//lParam=(LPARAM)(char*)szNewName
//returns 0 on success, nonzero on failure
//this will fail if the group name is a duplicate of an existing name
//CLUI is not called when this change is made
#define MS_CLIST_GROUPRENAME       "CList/GroupRename"

//build a menu of the group tree                          v0.1.2.1+
//wParam=0
//lParam=0
//returns a HMENU on success, or NULL on failure
//The return value must be DestroyMenu()ed when you're done with it.
//NULL will be returned if the user doesn't have any groups
//The dwItemData of every menu item is the handle to that group.
//Menu item IDs are assigned starting at 100, in no particular order.
#define MS_CLIST_GROUPBUILDMENU    "CList/GroupBuildMenu"

//changes the 'hide offline contacts' flag and call CLUI    v0.1.1.0+
//wParam=newValue
//lParam=0
//returns 0 on success, nonzero on failure
//newValue is 0 to show all contacts, 1 to only show online contacts
//or -1 to toggle the value
#define MS_CLIST_SETHIDEOFFLINE  "CList/SetHideOffline"

//do the message processing associated with double clicking a contact v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=0
//returns 0 on success, nonzero on failure
#define MS_CLIST_CONTACTDOUBLECLICKED "CList/ContactDoubleClicked"

//do the processing for when some files are dropped on a contact    v0.1.2.1+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(char**)ppFiles
//returns 0 on success, nonzero on failure
//ppFiles is an array of fully qualified filenames, ending with a NULL.
#define MS_CLIST_CONTACTFILESDROPPED   "CList/ContactFilesDropped"

//change the group a contact belongs to       v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(HANDLE)hGroup
//returns 0 on success, nonzero on failure
//use hGroup=NULL to put the contact in no group
#define MS_CLIST_CONTACTCHANGEGROUP   "CList/ContactChangeGroup"

//determines the ordering of two contacts              v0.1.1.0+
//wParam=(WPARAM)(HANDLE)hContact1
//lParam=(LPARAM)(HANDLE)hContact2
//returns 0 if hContact1 is the same as hContact2
//returns +1 if hContact2 should be displayed after hContact1
//returns -1 if hContact1 should be displayed after hContact2
#define MS_CLIST_CONTACTSCOMPARE      "CList/ContactsCompare"

/*
	wParam=0 (not used)
	lParam=(LPARAM) &MIRANDASYSTRAYNOTIFY

	Affects: Show a message in a ballon tip against a protocol icon (if installed)
	Returns: 0 on success, non zero on failure
	Notes  : This service will not be created on systems that haven't got the Windows
			 support for ballontips, also note that it's upto Windows if it shows your
			 message and it keeps check of delays (don't be stupid about showing messages)

	Version: 0.3.1a
*/
#define NIIF_INFO           0x00000001
#define NIIF_WARNING        0x00000002
#define NIIF_ERROR          0x00000003
#define NIIF_ICON_MASK      0x0000000F
#define NIIF_NOSOUND        0x00000010
#define NIIF_INTERN_UNICODE 0x00000100


typedef struct {
	int cbSize;			// sizeof(MIRANDASYSTRAY)
	char *szProto;		// protocol to show under (may have no effect)
	union {
		char *szInfoTitle;	// only 64chars of it will be used
		TCHAR *tszInfoTitle; // used if NIIF_INTERN_UNICODE is specified
	};
	union {
      char *szInfo;		// only 256chars of it will be used
		TCHAR *tszInfo;   // used if NIIF_INTERN_UNICODE is specified
	};
	DWORD dwInfoFlags;	// see NIIF_* stuff
	UINT uTimeout;		// how long to show the tip for
} MIRANDASYSTRAYNOTIFY;
#define MS_CLIST_SYSTRAY_NOTIFY "Miranda/Systray/Notify"

#define SETTING_TOOLWINDOW_DEFAULT   1
#define SETTING_SHOWMAINMENU_DEFAULT 1
#define SETTING_SHOWCAPTION_DEFAULT  1
#define SETTING_CLIENTDRAG_DEFAULT   1
#define SETTING_ONTOP_DEFAULT        0
#define SETTING_MIN2TRAY_DEFAULT     1
#define SETTING_TRAY1CLICK_DEFAULT   (IsWinVer7Plus()?1:0)
#define SETTING_HIDEOFFLINE_DEFAULT  0
#define SETTING_HIDEEMPTYGROUPS_DEFAULT  0
#define SETTING_USEGROUPS_DEFAULT    1
#define SETTING_SORTBYSTATUS_DEFAULT 0
#define SETTING_SORTBYPROTO_DEFAULT  0
#define SETTING_TRANSPARENT_DEFAULT  0
#define SETTING_ALPHA_DEFAULT        200
#define SETTING_AUTOALPHA_DEFAULT    150
#define SETTING_CONFIRMDELETE_DEFAULT 1
#define SETTING_AUTOHIDE_DEFAULT     0
#define SETTING_HIDETIME_DEFAULT     30
#define SETTING_CYCLETIME_DEFAULT    4
#define SETTING_TRAYICON_DEFAULT     SETTING_TRAYICON_SINGLE
#define SETTING_ALWAYSSTATUS_DEFAULT 0
#define SETTING_ALWAYSMULTI_DEFAULT  0

#define SETTING_TRAYICON_SINGLE   0
#define SETTING_TRAYICON_CYCLE    1
#define SETTING_TRAYICON_MULTI    2

#define SETTING_STATE_HIDDEN      0
#define SETTING_STATE_MINIMIZED   1
#define SETTING_STATE_NORMAL      2

#define SETTING_BRINGTOFRONT_DEFAULT 0

#endif // M_CLIST_H__
