/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
*/

#ifndef _M_USERINFOEX_H_
#define _M_USERINFOEX_H_
/*************************************************************
 * PropertySheetPage Module
 */

/* UserInfo/AddPage v0.1.0.0+
If the hIcon member of te optiondialogpage is valid, the tree show it for nicer look.
Otherwise the default icon is displayed for this treeitem.
*/
#ifndef ODPF_UNICODE
 #define ODPF_UNICODE		8	// string fields in OPTIONSDIALOGPAGE are WCHAR*
#endif
#define ODPF_ICON			64	// the hIcon member of the option dialog page is valid

/* Handling notifications v0.1.0.4+
A dialogbox should call SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSP_CHANGED) on a PSN_INFOCHANGED notification if
there are unsafed changes and apply button should keep enabled. Otherwise the changed status
of the dialogbox is resetted as well as the changed status of the details dialog box itself if no page
called this message. Because UserinfoEx now looks for changes in the settings of a user to keep the
shown inforamtion up to date.
*/
#define PSP_CHANGED			2

/* PSM_GETBOLDFONT v0.1.0.3+
wParam=NULL
lParam=(HFONT*)&hFont
get bold dialog font. wParam is pointer to a handle that retrieves the boldfont.
You can also call GetWindowLongPtr(hDlg, DWLP_MSGRESULT) to get the font.
This function returns TRUE on success or FALSE otherwise.
*/
#ifndef PSM_GETBOLDFONT
 #define PSM_GETBOLDFONT	(WM_USER+102)
#endif

/* PSM_ISLOCKED v0.1.0.4+
Returns state of propertysheet. If it is locked, The PSM_CHANGED messages sent by a propertysheetpage does not
have any effect. To aVOID editcontrols, ... to check for changes on redrawing at a load of settings from database or
if another propertysheetpage is selected, a plugin should check this state and skip those checks to reduce stressing
the database if such a test if control's content changed does so.
wParam=NULL
lParam=NULL
You can also call GetWindowLongPtr(hDlg, DWLP_MSGRESULT) to get the handle.
This function returns TRUE if the PropertySheet is locked or FALSE otherwise.
*/
#define PSM_ISLOCKED		(WM_USER+901)

/* PSM_GETCONTACT v0.1.0.4+
You can get the handle to the contact the propertysheet is associated with by calling PSM_GETCONTACT
to the parent of your propertysheetpage - the propertysheet.
wParam=index or -1 for current item
lParam=(HANDLE*)&hContact
You can also call GetWindowLongPtr(hDlg, DWLP_MSGRESULT) to get the handle.
This function returns TRUE on success or FALSE otherwise.
*/
#define PSM_GETCONTACT		(WM_USER+903)

/* PSM_GETBASEPROTO v0.1.0.4+
You can get a pointer to the basic protocol module by sending PSM_GETBASEPROTO to the parent of your propertysheetpage.
wParam=index or -1 for current item
lParam=(LPCSTR*)&pszProto
The propertysheet loads the basic contact protocol on creation for a better handling
of owners (ICQ) protocol used for changing details on the server. Should also reduce database traffic.
You can also call GetWindowLongPtr(hDlg, DWLP_MSGRESULT) to get the protocol.
This function returns TRUE on success or FALSE otherwise.
*/
#define PSM_GETBASEPROTO	(WM_USER+905)

#define INDEX_CURPAGE		(-1)
/* short helper macros
*/
#define PSGetBoldFont(hPsp, hFont)		SNDMSG(GetParent((HWND)hPsp), PSM_GETBOLDFONT, (WPARAM)INDEX_CURPAGE, (LPARAM)(HFONT*)&hFont)
#define PSGetContact(hPsp, hContact)	SNDMSG(GetParent((HWND)hPsp), PSM_GETCONTACT, (WPARAM)INDEX_CURPAGE, (LPARAM)(HANDLE*)&hContact)
#define PSGetBaseProto(hPsp, pszProto)	SNDMSG(GetParent((HWND)hPsp), PSM_GETBASEPROTO, (WPARAM)INDEX_CURPAGE, (LPARAM)(LPCSTR*)&pszProto)

/* PspIsLocked	v0.1.1.0+
Changed function a bit, because sometimes SNDMSG does not return the right value.
Don't know why. But this works fine.
*/
static FORCEINLINE BOOLEAN PspIsLocked(HWND hPsp)
{
	HWND hPs = GetParent(hPsp);
	return ((BOOLEAN)SendMessage((hPs), PSM_ISLOCKED, 0, 0) || GetWindowLongPtr((hPs), DWLP_MSGRESULT) != 0);
}

/* PSM_GETPAGEHWND	v0.1.1.1+
retrieve the windowhandle for a propertysheetpage identified by its id
wParam=idDlg
lParam=hInstance
*/
#define PSM_GETPAGEHWND		(WM_USER+906)

#define PSGetPageHandle(hPsp, idDlg, hInst) SNDMSG(GetParent((HWND)hPsp), PSM_GETPAGEHWND, (WPARAM)idDlg, (LPARAM)hInst)

/* PSM_DLGMESSAGE v0.1.1.1+
Send a message to a specified propertypage of the details dialog.
This enables communication between propertypages without the need to know
the window handles of each page.
*/
typedef struct TDlgCommand {
	HINSTANCE hInst;
	WORD idDlg;
	WORD idDlgItem;
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
} DLGCOMMAND, *LPDLGCOMMAND;

#define PSM_DLGMESSAGE		(WM_USER+907)

#define PSSendDlgMessage(hPsp, pDlgCmd)	SNDMSG(GetParent((HWND)hPsp), PSM_DLGMESSAGE, NULL, (LPARAM)(LPDLGCOMMAND)pDlgCmd)


/* PSM_ISAEROMODE v0.8.2.1+
This message can be sent to the propertysheet (details dialog) to examine,
whether the aero adaption mode is enabled or not. This message should be used in
each propertysheet page's dialog procedure as follows:

	...
	switch (msg) {
		...
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORDLG:
			if (PSIsAeroMode(hDlg))
				return (INT_PTR)GetStockBrush(WHITE_BRUSH);
			break;
		...

This will draw a propertysheet page with white background, if aero adaption is enabled.
wParam=not used
lParam=(BOOL*)&bIsAero
*/
#define PSM_ISAEROMODE (WM_USER+908)
static FORCEINLINE BOOLEAN PSIsAeroMode(HWND hPsp)
{
	BOOLEAN bIsAero;
	SendMessage(GetParent(hPsp), PSM_ISAEROMODE,(WPARAM) NULL, (LPARAM)&bIsAero);
	return bIsAero;
}

/*************************************************************
 *	vCard Module
 */

/* UserInfo/vCardExport v0.1.0.4+
*/
#define MS_USERINFO_VCARD_IMPORT	"UserInfo/vCard/Import"

#define MS_USERINFO_VCARD_IMPORTALL	"UserInfo/vCard/ImportAll"
/* UserInfo/vCardImport v0.1.0.4+
*/
#define MS_USERINFO_VCARD_EXPORT	"UserInfo/vCard/Export"

/* UserInfo/vCardImport v0.1.0.4+
*/
#define MS_USERINFO_VCARD_EXPORTALL	"UserInfo/vCard/ExportAll"

/*************************************************************
 * time Module
 */

/* UserInfo/LocalTime v0.1.0.3+
Computes the local time for the desired contact and writes it to lpst.
wParam=(WPARAM)hContact
lParam=(LPSYSTEMTIME)lpst
The service gets your windows box's local time, reads your timezoneinformation (Windows setting)
and hContact's timezone from his user details. With these information contact's local time is computed
considering daylightsaving time.
Return values are TRUE for success and FALSE if anything went wrong.
*/
#define MS_USERINFO_LOCALTIME		"UserInfo/LocalTime"

/* UserInfo/LocalTime v0.7.0.1+
This service provides the timezone information for a given contact
as known by windows, too. All but the DaylightName and StandardName members
of the class are filled out and can be used. The values are read
from the windows registry and therefore keep up to date if the latest
windows hotfixes are installed. There is no default API in the windows SDK
to solve this problem.
wParam=(WPARAM)hContact
lParam=(TIME_ZONE_INFORMATION*)tzi
Return values are 0 for success and 1 if no valid timezone is set for the contact.
*/
#define MS_USERINFO_TIMEZONEINFO	"UserInfo/TimezoneInfo"

/*************************************************************
 *	Reminder module
 */

/* UserInfo/Reminder/Check v0.1.0.4+
This service checks if one of your contacts has birthday in the next few days
wParam = lParam = not used
*/
#define MS_USERINFO_REMINDER_CHECK	"UserInfo/Reminder/Check"


/* UserInfo/Reminder/Check v0.1.1.1+
This service creates a dialog, that lists all of the anniversaries
wParam = lParam = not used
*/
#define MS_USERINFO_REMINDER_LIST	"UserInfo/Reminder/List"


/* UserInfo/Reminder/Check v0.1.2.16+
This service compares birthday date which is set by the protocol module of each contact
to the first found custom set birthday date. If a difference is detected, the user is asked
whether to update the custom set birthday by the one of the protocol or not.

If no custom birthday is set yet and the protocol contains a valid birthday, it is copied to
primary custom module (e.g.: mBirthday or UserInfo).
wParam = handle to single contact or NULL to backup all
lParam = not used
*/
#define MS_USERINFO_REMINDER_AGGRASIVEBACKUP "UserInfo/Reminder/AggrassiveBackup"


/* UserInfo/Refresh v0.7.0.1+
This service calls PSS_GETINFO for all contacts in the contact list
wParam = not used
lParam = not used
*/
#define MS_USERINFO_REFRESH "UserInfo/Refresh"


/*************************************************************
 *	Uinfobuttonclass module
 */

// button styles
#define MBS_DEFBUTTON		0x00001000L			// default button
#define MBS_PUSHBUTTON		0x00002000L			// toggle button
#define MBS_FLAT           0x00004000L			// flat button
#define MBS_DOWNARROW		0x00008000L			// has arrow on the right

#define MBBF_UNICODE			1
#ifdef _UNICODE
 #define MBBF_TCHAR			MBBF_UNICODE
#else
 #define MBBF_TCHAR			0
#endif

// BUTTONADDTOOLTIP
// use lParam=MBF_UNICODE to set unicode tooltips
// for lParam=0 the string is interpreted as ansi

// message to explicitly translate the buttons text,
// as it is not done by default translation routine
// wParam=lParam=NULL
#define BUTTONTRANSLATE		(WM_USER+6)

/* UserInfo/MsgBox v0.1.0.4+
Slightly modified version of MButtonClass, to draw both text and icon in a button control
*/
#define UINFOBUTTONCLASS	L"UInfoButtonClass"

/*************************************************************
 *	contact info module
 */

// additional information which can be retrieved with this service
#define CNF_COPHONE			55	// returns company phone (string)
#define CNF_COFAX			40	// returns company fax (string)
#define CNF_COCELLULAR		41	// returns company cellular (string)
#define CNF_COEMAIL			42	// returns company email address (string)

/* CNF_BIRTHDATE v0.1.2.18+
returns a formated string with the birthdate in it
wParam - 1 for long dateformat, 0 for short dateformat
lParam - CONTACTINFO structure as for all other fields, too
returns 0 on success and 1 on failure
*/
#define CNF_BIRTHDATE		43	// returns date of birth (string)


/*************************************************************
 *	extended integration module
 */

/* UserInfo/Homepage/OpenURL v0.1.2.19+
This service reads the contact's homepage from UserInfo module or contact's protocol module
and opens the default browser to show it.
wParam=hContact	- handle to contact whose homepage is to show
lParam=not used
*/
#define MS_USERINFO_HOMEPAGE_OPENURL		"UserInfo/Homepage/OpenURL"

/*************************************************************/
#endif /*	_M_USERINFOEX_H_ */