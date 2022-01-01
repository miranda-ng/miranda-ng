/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#ifndef M_GENMENU_H__
#include <m_genmenu.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// sent when the user asks to change their status
// wParam = new status, from statusmodes.h
// lParam = protocol name, NULL if for all protocols
// also sent due to a ms_clist_setstatusmode call

#define ME_CLIST_STATUSMODECHANGE       "CList/StatusModeChange"

/////////////////////////////////////////////////////////////////////////////////////////
// force a change of status mode
// iStatus = new status, from statusmodes.h

EXTERN_C MIR_APP_DLL(void) Clist_SetStatusMode(int iStatus);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the text associated with the given status

#define GSMDF_UNTRANSLATED 4 // don't tranlate the result
EXTERN_C MIR_APP_DLL(wchar_t*) Clist_GetStatusModeDescription(int iStatus, int iFlags /*GSMDF_*/);

/////////////////////////////////////////////////////////////////////////////////////////
// detects whether a protocol is visible or not

EXTERN_C MIR_APP_DLL(bool) Clist_GetProtocolVisibility(const char *szModuleName);

/////////////////////////////////////////////////////////////////////////////////////////
// get the current status mode
// wParam = lParam = 0
// returns the current status
// This is the status *as set by the user*, not any protocol-specific status
// All protocol modules will attempt to conform to this setting at all times

#define MS_CLIST_GETSTATUSMODE			"CList/GetStatusMode"

/////////////////////////////////////////////////////////////////////////////////////////
// MAIN MENU

// adds a new element into main menu

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddMainMenuItem(TMO_MenuItem *pmi);

// gets a handle to the main Miranda menu
// returns a HMENU. This need not to be freed since it's owned by clist

EXTERN_C MIR_APP_DLL(HMENU) Menu_GetMainMenu(void);

/////////////////////////////////////////////////////////////////////////////////////////
// the main menu is about to be built
// wParam = lParam = 0

#define ME_CLIST_PREBUILDMAINMENU "CList/PreBuildMainMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// CONTACT MENU

// adds a new element into contact menu

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddContactMenuItem(TMO_MenuItem *pmi, const char *pszProto = nullptr);

// builds the context menu for a specific contact
// returns a HMENU identifying the menu. This should be DestroyMenu()ed when
// finished with.

EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildContactMenu(MCONTACT hContact);

// the context menu for a contact is about to be built
// modules should use this to change menu items that are specific to the
// contact that has them

#define ME_CLIST_PREBUILDCONTACTMENU "CList/PreBuildContactMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// STATUS MENU

// get a handle to the Miranda status menu
// returns a HMENU. This need not be freed since it's owned by clist

EXTERN_C MIR_APP_DLL(HMENU) Menu_GetStatusMenu(void);

// adds an item to a status menu

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddStatusMenuItem(TMO_MenuItem *pmi, const char *pszProto = nullptr);

// the status menu is about to be built
// wParam = lParam = 0

#define ME_CLIST_PREBUILDSTATUSMENU "CList/PreBuildStatusMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL MENU

// adds an item to status or main menu, according to the option
EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddProtoMenuItem(TMO_MenuItem *pmi, const char *pszProto = nullptr);

// returns root protocol menu for an account (either in main menu or in status bar)
EXTERN_C MIR_APP_DLL(HGENMENU) Menu_GetProtocolMenu(const char *szModuleName);

// rebuilds all protocol menus
EXTERN_C MIR_APP_DLL(void) Menu_ReloadProtoMenus(void);

/////////////////////////////////////////////////////////////////////////////////////////
// GROUP MENU

struct GroupMenuParam
{
	int wParam;
	int lParam;
};

// builds the Group menu
// returns a HMENU identifying the menu.

EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildGroupMenu(void);

// adds a new item to the Group menus

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddGroupMenuItem(TMO_MenuItem *pmi, GroupMenuParam *gmp = nullptr);

// the Group menu is about to be built
// wParam = lParam = 0

#define ME_CLIST_PREBUILDGROUPMENU "CList/PreBuildGroupMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// SUBGROUP MENU

// builds the SubGroup menu
// returns a HMENU identifying the menu.

EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildSubGroupMenu(struct ClcGroup *group);

// adds a new item to the SubGroup menus
// wParam=GroupMenuParam*, params to call when exec menuitem
// lParam=(LPARAM)(TMO_MenuItem*)&mi

EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddSubGroupMenuItem(TMO_MenuItem *pmi, GroupMenuParam *gmp = nullptr);

// the SubGroup menu is about to be built
// wParam = lParam = 0

#define ME_CLIST_PREBUILDSUBGROUPMENU "CList/PreBuildSubGroupMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// TRAY MENU

// builds the tray menu
// returns a HMENU identifying the menu.
EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildTrayMenu(void);

// destroys a tray menu
MIR_APP_DLL(void) Menu_DestroyNestedMenu(HMENU hMenu);

// adds a new item to the tray menus
EXTERN_C MIR_APP_DLL(HGENMENU) Menu_AddTrayMenuItem(TMO_MenuItem *pmi);

// the tray menu is about to be built
// wParam = lParam = 0
#define ME_CLIST_PREBUILDTRAYMENU "CList/PreBuildTrayMenu"

/////////////////////////////////////////////////////////////////////////////////////////
// wParam = (WPARAM)hContact
// lParam = 0
// 
// Event is fired when there is a double click on a CList contact,
// it is upto the caller to check for the protocol & status
// of the MCONTACT, it's not done for you anymore since it didn't make
// sense to store all this information in memory, etc.

#define ME_CLIST_DOUBLECLICKED "CList/DoubleClicked"

/////////////////////////////////////////////////////////////////////////////////////////
// The contact list will flash hIcon next to the contact hContact (use NULL for
// a system message). szServiceName will be called when the user double clicks
// the icon, at which point the event will be removed from the contact list's
// queue automatically
// pszService is called with wParam = (WPARAM)(HWND)hwndContactList,
// lParam = (LPARAM)(CLISTEVENT*)cle. Its return value is ignored. cle is
// invalidated when your service returns, so take copies of any important
// information in it.
// hDbEvent should be unique since it and hContact are the identifiers used by
// clist/removeevent if, for example, your module implements a 'read next' that
// bypasses the double-click.

struct CLISTEVENT
{
	MCONTACT hContact;      // handle to the contact to put the icon by
	uint32_t flags;         // ...of course
	HICON hIcon;            // icon to flash
	MEVENT hDbEvent;        // caller defined but should be unique for hContact
	const char *moduleName; // for events with CLEF_PROTOCOLGLOBAL in flags
	LPARAM lParam;	         // caller defined
	const char *pszService; // name of the service to call on activation
	MAllCStrings szTooltip; // short description of the event to display as a tooltip on the system tray
};

#define CLEF_URGENT    1   // flashes the icon even if the user is occupied,
							      // and puts the event at the top of the queue
#define CLEF_ONLYAFEW  2   // the icon will not flash for ever, only a few
							      // times. This is for eg online alert
#define CLEF_UNICODE   4   // set pszTooltip as unicode

#define CLEF_PROTOCOLGLOBAL   8 // set event globally for protocol, hContact has to be NULL,
									     // moduleName the protocol ID name to be set

/////////////////////////////////////////////////////////////////////////////////////////
// gets the image list with all the useful icons in it
// the members of this image list are opaque, and you should trust what you are given

#define IMAGE_GROUPOPEN     (MAX_STATUS_COUNT+1)
#define IMAGE_GROUPSHUT     (MAX_STATUS_COUNT+2)

#ifdef _INC_COMMCTRL
EXTERN_C MIR_APP_DLL(HIMAGELIST) Clist_GetImageList(void);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// The icon of a contact in the contact list has changed
// wParam = (MCONTACT)hContact
// lParam = iconId
// iconId is an offset into the clist's imagelist. See clist/geticonsimagelist

#define ME_CLIST_CONTACTICONCHANGED   "CList/ContactIconChanged"

/////////////////////////////////////////////////////////////////////////////////////////
// processes a menu selection from a menu
// wParam = MAKEWPARAM(LOWORD(wParam from WM_COMMAND), flags)
// lParam = (LPARAM)(MCONTACT)hContact
// returns TRUE if it processed the command, FALSE otherwise
// hContact is the currently selected contact. It it not used if this is a main
// menu command. If this is NULL and the command is a contact menu one, the
// command is ignored
/////////////////////////////////////////////////////////////////////////////////////////
// Due to it is generic practice to handle menu command via WM_COMMAND
// window message handle and practice to process it via calling
// Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, hContact);
// to ensure that WM_COMMAND was really from clist menu not from other menu
// it is reserved range of menu ids from CLISTMENUIDMIN to CLISTMENUIDMAX
// the menu items with ids outside from such range will not be processed by service.
// Moreover if you process WM_COMMAND youself and your window contains self menu
// please be sure that you will not call service for non-clist menu items.
// The simplest way is to ensure that your menus are not use item ids from such range.
// Otherwise, you HAVE TO distinguish WM_COMMAND from clist menus and from you� internal menu and
// DO NOT call Clist_MenuProcessCommand for non clist menus.

#define CLISTMENUIDMIN	0x4000	  // reserved range for clist menu ids
#define CLISTMENUIDMAX	0x7fff

#define MPCF_CONTACTMENU   1	//test commands from a contact menu
#define MPCF_MAINMENU      2	//test commands from the main menu

EXTERN_C MIR_APP_DLL(BOOL) Clist_MenuProcessCommand(int menu_id, int flags, MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// processes a menu hotkey
// wParam = virtual key code
// lParam = MPCF_ flags
// returns TRUE if it processed the command, FALSE otherwise
// this should be called in WM_KEYDOWN

EXTERN_C MIR_APP_DLL(BOOL) Clist_MenuProcessHotkey(unsigned hotkey);

/////////////////////////////////////////////////////////////////////////////////////////
// determines whether the contact list is docked
// wParam = lParam = 0
// returns nonzero if the contact list is docked, of 0 if it is not

EXTERN_C MIR_APP_DLL(BOOL) Clist_IsDocked(void);

/////////////////////////////////////////////////////////////////////////////////////////
// Clist-related buttons management
/////////////////////////////////////////////////////////////////////////////////////////
// toggles the use groups mode of the contact list
// wParam = lParam = 0
// returns new groups mode

#define MS_CLIST_TOGGLEGROUPS "CList/ToggleGroups"

/////////////////////////////////////////////////////////////////////////////////////////
// toggles the empty groups display mode
// wParam = lParam = 0
// returns new empty groups mode

#define MS_CLIST_TOGGLEEMPTYGROUPS "CList/ToggleEmptyGroups"

/////////////////////////////////////////////////////////////////////////////////////////
// toggles the hidden users display mode
// wParam = lParam = 0
// returns new hidden users mode

#define MS_CLIST_TOGGLEHIDEOFFLINE "CList/ToggleHideOffline"

/////////////////////////////////////////////////////////////////////////////////////////
// toggles the hidden users display mode
// wParam = lParam = 0
// returns new hidden users mode

#define MS_CLIST_TOGGLEHIDEOFFLINEROOT "CList/ToggleHideOfflineRoot"

/////////////////////////////////////////////////////////////////////////////////////////
// toggles contact list visibility
// wParam = lParam = 0

#define MS_CLIST_SHOWHIDE "CList/ShowHide"

/////////////////////////////////////////////////////////////////////////////////////////
// deletes a contact from contact list
// wParam = MCONTACT - contact to be deleted
// lParam = 0

#define MS_CLIST_DELETECONTACT "CList/DeleteContactCommand"

/////////////////////////////////////////////////////////////////////////////////////////
// sent when the group get modified (created, renamed or deleted)
// or contact is moving from group to group
// wParam = hContact - NULL if operation on group
// lParam = pointer to CLISTGROUPCHANGE

struct CLISTGROUPCHANGE
{
	const wchar_t *pszOldName;  // old group name, NULL if a new group was created
	const wchar_t *pszNewName;  // new group name, NULL if an old group was deleted
};

#define ME_CLIST_GROUPCHANGE       "CList/GroupChange"

/////////////////////////////////////////////////////////////////////////////////////////
// checks that a group exists
// returns 0 if a group is not found or group handle on success

typedef int MGROUP;

EXTERN_C MIR_APP_DLL(MGROUP) Clist_GroupExists(LPCTSTR ptszGroupName);

/////////////////////////////////////////////////////////////////////////////////////////
// creates a new group and calls CLUI to display it
// returns a handle to the new group
// hParentGroup is NULL to create the new group at the root, or can be the
// handle of the group of which the new group should be a subgroup.
// groupName is a wchar_t* pointing to the group name to create or NULL for
// API to create unique name by itself

EXTERN_C MIR_APP_DLL(MGROUP) Clist_GroupCreate(MGROUP hParent, const wchar_t *ptszGroupName);

/////////////////////////////////////////////////////////////////////////////////////////
// a new group was created. Add it to the list
// this is also called when the contact list is being rebuilt
// new groups are always created with the name "New Group"

EXTERN_C MIR_APP_DLL(void) Clist_GroupAdded(MGROUP hGroup);

/////////////////////////////////////////////////////////////////////////////////////////
// deletes a group and calls CLUI to display the change
// returns 0 on success, nonzero on failure

EXTERN_C MIR_APP_DLL(int) Clist_GroupDelete(MGROUP hGroup);

/////////////////////////////////////////////////////////////////////////////////////////
// renames a group
// returns 0 on success, nonzero on failure

EXTERN_C MIR_APP_DLL(int) Clist_GroupRename(MGROUP hGroup, const wchar_t *ptszNewName);

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves a group's name
// returns a wchar_t* on success, NULL on failure
// if pdwFlags is not NULL, also stores group flags into it (one of GROUPF_* constants

#define GROUPF_EXPANDED    0x04
#define GROUPF_HIDEOFFLINE 0x08

EXTERN_C MIR_APP_DLL(wchar_t*) Clist_GroupGetName(MGROUP hGroup, uint32_t *pdwFlags = nullptr);

/////////////////////////////////////////////////////////////////////////////////////////
// change the expanded state flag for a group internally
// returns 0 on success, nonzero on failure
// newState is nonzero if the group is expanded, 0 if it's collapsed
// CLUI is not called when this change is made

EXTERN_C MIR_APP_DLL(int) Clist_GroupSetExpanded(MGROUP hGroup, int iNewState);

/////////////////////////////////////////////////////////////////////////////////////////
// changes the flags for a group
// iNewFlags = MAKELPARAM(flags, flagsMask)
// returns 0 on success, nonzero on failure
// Only the flags given in flagsMask are altered.
// CLUI is called on changes to GROUPF_HIDEOFFLINE.

EXTERN_C MIR_APP_DLL(int) Clist_GroupSetFlags(MGROUP hGroup, LPARAM iNewFlags);

/////////////////////////////////////////////////////////////////////////////////////////
// move a group to directly before another group
// returns the new handle of the group on success, NULL on failure
// The order is represented by the order in which MS_CLUI_GROUPADDED is called,
// however UIs are free to ignore this order and sort alphabetically if they wish.

EXTERN_C MIR_APP_DLL(int) Clist_GroupMoveBefore(MGROUP hGroup, MGROUP hGroupBefore);

/////////////////////////////////////////////////////////////////////////////////////////
// build a menu of the group tree
// returns a HMENU on success, or NULL on failure
// The return value must be DestroyMenu()ed when you're done with it.
// NULL will be returned if the user doesn't have any groups
// The dwItemData of every menu item is the handle to that group.

EXTERN_C MIR_APP_DLL(HMENU) Clist_GroupBuildMenu(int iStartMenuId = 100);

/////////////////////////////////////////////////////////////////////////////////////////
// gets group name for a contact
// might return NULL if a group isn't set

EXTERN_C MIR_APP_DLL(wchar_t*) Clist_GetGroup(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// sets group name for a contact

EXTERN_C MIR_APP_DLL(void) Clist_SetGroup(MCONTACT hContact, const wchar_t *pwszName);

/////////////////////////////////////////////////////////////////////////////////////////
// end a rebuild of the contact list

EXTERN_C MIR_APP_DLL(void) Clist_EndRebuild(void);

/////////////////////////////////////////////////////////////////////////////////////////
// does the message processing associated with double clicking a contact

EXTERN_C MIR_APP_DLL(void) Clist_ContactDoubleClicked(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the icon's index of specified contact in the internal image list or -1

EXTERN_C MIR_APP_DLL(int) Clist_GetContactIcon(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// change the group a contact belongs to
// returns 0 on success, nonzero on failure
// use hGroup = NULL to put the contact in no group

EXTERN_C MIR_APP_DLL(int) Clist_ContactChangeGroup(MCONTACT hContact, MGROUP hGroup);

/////////////////////////////////////////////////////////////////////////////////////////
// determines the ordering of two contacts
// returns 0 if hContact1 is the same as hContact2
// returns +1 if hContact2 should be displayed after hContact1
// returns -1 if hContact1 should be displayed after hContact2

EXTERN_C MIR_APP_DLL(int) Clist_ContactCompare(MCONTACT hContact1, MCONTACT hContact2);

/////////////////////////////////////////////////////////////////////////////////////////
// returns a static display name for a contact

#define GCDNF_NOMYHANDLE 1 // will never return the user's custom name
#define GCDNF_NOUNKNOWN  2 // will return nullptr instead of "Unknown contact" 
#define GCDNF_NOCACHE    4 // will not use the cache

EXTERN_C MIR_APP_DLL(wchar_t*) Clist_GetContactDisplayName(MCONTACT hContact, int mode = 0);

/////////////////////////////////////////////////////////////////////////////////////////
// DRAG-N-DROP SUPPORT
/////////////////////////////////////////////////////////////////////////////////////////
// a contact is being dragged outside the main window
// wParam = (MCONTACT)hContact
// lParam = MAKELPARAM(screenX, screenY)
// return nonzero to make the cursor a 'can drop here', or zero for 'no'

#define ME_CLUI_CONTACTDRAGGING     "CLUI/ContactDragging"

/////////////////////////////////////////////////////////////////////////////////////////
// a contact has just been dropped outside the main window
// wParam = (MCONTACT)hContact
// lParam = MAKELPARAM(screenX, screenY)
// return nonzero if your hook processed this, so no other hooks get it

#define ME_CLUI_CONTACTDROPPED      "CLUI/ContactDropped"

/////////////////////////////////////////////////////////////////////////////////////////
// a contact that was being dragged outside the main window has gone back in to the main window.
// wParam = (MCONTACT)hContact
// lParam = 0
// always returns zero

#define ME_CLUI_CONTACTDRAGSTOP     "CLUI/ContactDragStop"

/////////////////////////////////////////////////////////////////////////////////////////
// Affects: Show a message in a ballon tip against a protocol icon (if installed)
// Returns: 0 on success, non zero on failure
// Notes  : This service will not be created on systems that haven't got the Windows
// support for ballontips, also note that it's upto Windows if it shows your
// message and it keeps check of delays (don't be stupid about showing messages)

#define NIIF_INFO           0x00000001
#define NIIF_WARNING        0x00000002
#define NIIF_ERROR          0x00000003
#define NIIF_ICON_MASK      0x0000000F
#define NIIF_NOSOUND        0x00000010

EXTERN_C MIR_APP_DLL(int) Clist_TrayNotifyA(const char *szProto, const char *szInfoTitle, const char *szInfo, uint32_t dwInfoFlags, UINT uTimeout);
EXTERN_C MIR_APP_DLL(int) Clist_TrayNotifyW(const char *szProto, const wchar_t *wszInfoTitle, const wchar_t *wszInfo, uint32_t dwInfoFlags, UINT uTimeout);

/////////////////////////////////////////////////////////////////////////////////////////

#define SETTING_TOOLWINDOW_DEFAULT    1
#define SETTING_SHOWMAINMENU_DEFAULT  1
#define SETTING_SHOWCAPTION_DEFAULT   1
#define SETTING_CLIENTDRAG_DEFAULT    1
#define SETTING_ONTOP_DEFAULT         1
#define SETTING_MIN2TRAY_DEFAULT      1
#define SETTING_SORTBYSTATUS_DEFAULT  0
#define SETTING_SORTBYPROTO_DEFAULT   0
#define SETTING_TRANSPARENT_DEFAULT   0
#define SETTING_ALPHA_DEFAULT         200
#define SETTING_AUTOALPHA_DEFAULT     150
#define SETTING_AUTOHIDE_DEFAULT      0
#define SETTING_HIDETIME_DEFAULT      30
#define SETTING_CYCLETIME_DEFAULT     4
#define SETTING_TRAYICON_DEFAULT      SETTING_TRAYICON_SINGLE
#define SETTING_ALWAYSMULTI_DEFAULT   0

#define SETTING_TRAYICON_SINGLE       0
#define SETTING_TRAYICON_CYCLE        1
#define SETTING_TRAYICON_MULTI        2

#define SETTING_STATE_HIDDEN          0
#define SETTING_STATE_MINIMIZED       1
#define SETTING_STATE_NORMAL          2

#define SETTING_BRINGTOFRONT_DEFAULT 0

#endif // M_CLIST_H__
