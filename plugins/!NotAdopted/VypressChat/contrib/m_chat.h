/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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


#if 0

/*
	This plugin provides event driven chat rooms for protocols that wish to use it.
	It is built for IRC, which I also develop and is naturally biased towards IRC,
	but it should work very well with other protocols too. I will try to explain as
	careful as possible in this document how to use chat.dll

	-- General guidelines --

	There is one rule a protocol MUST follow to use this:

	1. Do not touch contacts that has a byte "ChatRoom" set to ANYTHING other than 0! (Could be 1, 2, 3, ...)
	This is because chat.dll adds contacts to the clist using the protocol name 
	supplied by the protocol. But this will naturally not work well if the 
	protocol also tampers with the contacts. The value of the BYTE indicates which type of
	window/contact it is (see the GCW_* flags below). There is two exceptions to this rule:
		
		* You should continue to handle the right click menu items of these 
		contacts as usual, by hooking the menu prebuild hook etc. Chat.dll can not 
		handle this in an efficient manner!
		
		* You should also handle when the user deletes the contact/room from the 
		contact list, as the protocol will then most likely have to send some message
		to the server that the user has left the room.

	2. The chat.dll plugin keeps its own copies of strings passed. 
	
*/

  
//	Example of implementing point 1: 

//	This is a code snippet that is common in protocols:

  

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) 
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !lstrcmpi(szProto, PROTONAME)) 
		{
//			... do something with the hContact here;
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}


  
//	You should do this instead:


  
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) 
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !lstrcmpi(szProto, PROTONAME)) 
		{
			if(db_get_b(hContact, PROTONAME, "ChatRoom", 0) == 0)
			{
//				... do something with the hContact here;
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}


//	There is not more to it than that. To recapitulate: do not touch contacts where the
//	BYTE "ChatRoom" is set to 1, apart from the two exceptions mentioned!

//	... now onto how to use this thing!


#endif



//------------------------- SERVICES ------------------------
/*	
	-- Register with the chat module --

	The first thing that a protocol need to do is register with chat.dll. This is best done
	after ALL modules has loaded naturally. The registration is needed to make sure that 
	the protocol obey rule 1 mentioned above, but equally important to set protocol specific
	settings.

	wParam=  NULL
	lParam=  (LPARAM)(GCREGISTER *)gcr
	returns 0 on success or nonzero on failure
*/

#define GC_BOLD			0x0001		//enable the 'bold' button
#define GC_ITALICS		0x0002		//enable the 'italics' button	
#define GC_UNDERLINE	0x0004		//enable the 'underline' button
#define GC_COLOR		0x0008		//enable the 'foreground color' button
#define GC_BKGCOLOR		0x0010		//enable the 'background color' button
#define GC_ACKMSG		0x0020		//the protocol must ack. messages sent
#define GC_TYPNOTIF		0x0040		//enable typing notifications
#define GC_CHANMGR		0x0080		//enable the 'channel settings' button

typedef struct {
	int			cbSize;				//Set to sizeof(); 			
	DWORD		dwFlags;			//Use GC_* flags above
	const char *pszModule;			//This MUST be the protocol name as registered with Miranda IM 			
	const char *pszModuleDispName;	//This is the protocol name as it will be displayed to the user
	int			iMaxText;			//Max message length the protocol supports. Will limit the typing area
	int			nColors;			//Number of colors in the colorchooser menu for the color buttons. Max = 100
	COLORREF*	pColors;			//pointer to the first static COLORREF array. ie: COLORREF crCols[nColors]; pColors = &crCols[0];
 } GCREGISTER;
#define MS_GC_REGISTER  "GChat/Register"



/*
	-- Tell the chat module to create a new window --

	Create a new chat room and set various settings related to it. This is not 
	the same as actually joining the chat room.	The chat room will not be visible 
	to the user until the 'set up' phase is completed. See the MS_GC_EVENT for that.

	wParam=0
	lParam=(LPARAM)(GCWINDOW*)gcw

	returns 0 on success or nonzero on failure
*/
#define GCW_CHATROOM		1	
#define GCW_SERVER			2
#define GCW_PRIVMESS		3
	
typedef struct {
	int			cbSize;				//Set to sizeof();
	int			iType;				//Use one of the GCW_* flags above to set the general type of usage for the window
	const char *pszModule;			//The name of the protocol owning the window (the same as pszModule when you register)					
	const char *pszName;			//The name of the chat room as it will be displayed to the user
	const char *pszID;				//The unique identifier for the chat room
	const char *pszStatusbarText;	//Optional text to set in the statusbar, or NULL.
	BOOL		bDisableNickList;	//Disable the nicklist
	DWORD		dwItemData;			//Set user defined data for this chat room. Retrieve it by using the service below
 } GCWINDOW;
#define MS_GC_NEWCHAT  "GChat/NewChat"



/*
	-- Show an event --

	Events is what drives chat.dll! After having created the chat room with 
	MS_GC_NEWCHAT it is time to make it work for real. Start off by telling chat.dll
	what statuses a user can have by sending GC_EVENT_ADDGROUP as many times as needed. 
	Then send join events 
	(GC_EVENT_JOIN) to populate the user list. You will need to send one event 
	per user that should be added. When you are done with filling the user list 
	it is a good time to end the 'set up' phase and make the window visible by 
	calling a GC_EVENT_VISIBILITY event. A nice tip is to make sure the bAddToLog 
	member of GCEVENT is set to FALSE during the initial filling of the user list.

	The GCDEST structure and its members are always used, but what members of 
	GCEVENT to use is determined by what event is sent to chat.dll. bAddToLog and
	time is valid (at least where it makes sense). See the description of each event 
	for more information of what members that are valid.

	It is possible to send formatted text (bold, italics, underlined, foreground color 
	and background color) by using the following keywords in pszText:
	%cRRRGGGBBB	- set the foreground color
	%C			- set foreground color to default
	%fRRRGGGBBB	- set the background color
	%F			- set the background color to default
	%b			- enable bold
	%B			- disable bold
	%u			- enable underlined
	%U			- disable underlined
	%i			- enable italics
	%I			- disable italics
	%r			- reset all to default
	%%			- escape the formatting. Translates to %

	wParam=0
	lParam=(LPARAM)(GCEVENT *) gce
	
	returns 0 on success or nonzero on failure
*/


//	GC_EVENT_JOIN <pszNick> has joined
// A user is joining the channel, set bIsMe to indicate who is the user
//	pszNick		- Display name
//	pszUID		- Unique identifier
//	pszStatus	- Which group (status) to add the user to
//	bIsMe		- Is this the user? Used to indicate that the user has joined the channel
#define GC_EVENT_JOIN			0x0001

//	GC_EVENT_PART <pszNick> has left[: pszText]
//	A user left the chat room
//	pszUID		- Unique identifier
//	pszText		- part message
#define GC_EVENT_PART			0x0002

//	GC_EVENT_QUIT <pszNick> disconnected[: pszText]
//	A user disconnected, use pszID = NULL (of GCDEST) to broadcast to all windows.
//	pszUID		- Unique identifier
//	pszText		- part message
#define GC_EVENT_QUIT			0x0004

//	GC_EVENT_KICK <pszStatus> kicked <pszNick>
//	A user is kicking another user from the room
//	pszUID		- Unique identifier of the one being kicked
//	pszStatus	- Name of user doing the kick
#define GC_EVENT_KICK			0x0008

//	GC_EVENT_NICK <pszNick> is now known as <pszText>
//	A user changed his name
//	NOTE, see GC_EVENT_CHID also
//	pszUID		- Unique identifier of the one changing name
//	pszText		- New name of the user
#define GC_EVENT_NICK			0x0010

//	GC_EVENT_NOTICE Notice from <pszNick>: <pszText>
//	An IRC type notice, will be sent to the active window
//	pszUID		- Unique identifier
//	pszText		- Notice text
#define GC_EVENT_NOTICE			0x0020

//	GC_EVENT_MESSAGE 
//	A regular chat room message 
//	is outgoing or incoming
//	pszUID		- Unique identifier
//	pszText		- Message text, use the formatting variables above. 
//				  NOTE make sure % is translated to %% to avoid accidental formatting
#define GC_EVENT_MESSAGE		0x0040

//	GC_EVENT_TOPIC Topic is <pszText>
//	pszUID		- Unique identifier
//	pszText		- Topic text
#define GC_EVENT_TOPIC			0x0080

//	GC_EVENT_INFORMATION 
//	Informational style text
//	pszText		- Information text
#define GC_EVENT_INFORMATION	0x0100

//	GC_EVENT_ACTION 
//  An IRC Style action event. Same as GC_EVENT_MESSAGE otherwise
#define GC_EVENT_ACTION			0x0200

//	GC_EVENT_ADDSTATUS <pszText> enables '<pszStatus>' for <pszNick>
//	pszUID		- Unique identifier
//	pszText		- The one enabling the status for another user
//	pszStatus		- The status given
#define GC_EVENT_ADDSTATUS		0x0400

//	GC_EVENT_REMOVESTATUS <pszText> disables '<pszStatus>' for <pszNick>
//	pszUID		- Unique identifier
//	pszText		- The one disabling the status for another user
//	pszStatus		- The status taken
#define GC_EVENT_REMOVESTATUS	0x0800

//	GC_EVENT_CHID - not shown in the log
//	Change the unique identifier of a contact
//	pszUID		- Unique identifier
//	pszText		- The new unique identifier
#define GC_EVENT_CHID			0x1000

//	GC_EVENT_CHID - not shown in the log
//	Change the name of a window
//	pszText		- The new name
#define GC_EVENT_CHWINNAME		0x1001

//	GC_EVENT_ADDGROUP - not shown in the log
//	Add a new status group to the user list
//	pszStatus		- The new group name
#define GC_EVENT_ADDGROUP		0x1002

//	GC_EVENT_SETITEMDATA GC_EVENT_SETITEMDATA - not shown in the log
//	Get or set the user defined data of a window
//	dwItemData		- The itemdata to set or get
#define GC_EVENT_SETITEMDATA	0x1003 
#define GC_EVENT_GETITEMDATA	0x1004 

//	GC_EVENT_CONTROL  - not shown in the log
//	Call WINDOW_INITDONE after the initial setup is done.
//	Also use it to control aspects of a window if needed .
//	No members of GCEVENT used, send one of the below flags in wParam instead
#define WINDOW_INITDONE		1     //send when the window is joined and all users have ben added to the nicklist
#define WINDOW_VISIBLE		2     //make the room visible (most likely you will never use this)
#define WINDOW_HIDDEN		3     //make the room hidden (most likely you will never use this)
#define WINDOW_MAXIMIZE		4     //make the room maximized (most likely you will never use this)
#define WINDOW_MINIMIZE		5     //make the room minimized (most likely you will never use this)
#define WINDOW_CLEARLOG		6     //clear the log of the room
#define WINDOW_TERMINATE	7     //send to remove a window from chat.dll, 
#define WINDOW_OFFLINE		8     //send when the user leave the room
#define WINDOW_ONLINE		9     //send when the user join the room

#define GC_EVENT_CONTROL		0x1005 

//	GC_EVENT_SETSBTEXT - not shown in the log
//	Set the text of the statusbar
//	pszText		- text
#define GC_EVENT_SETSBTEXT		0x1006 

//	GC_EVENT_ACK - not shown in the log
//	Used to ack a outgoing message, when GC_ACKMSG is set
//	dwItemData		- The itemdata
#define GC_EVENT_ACK			0x1007

//	GC_EVENT_SENDMESSAGE - not shown in the log
//	Send a message from the window as if the user had typed it.
//	Used by IRC to broadcast /AME and /AMSG messages
//	pszText		- The text
#define GC_EVENT_SENDMESSAGE	0x1008 

typedef struct {
	char		*pszModule;			//Name of the protocol (same as you registered with)					
	char		*pszID;				//Unique identifier of the room corresponding to the event, or NULL to broadcast to all rooms.
	int			iType;				//Use GC_EVENT_* as defined above. Only one event per service call.
} GCDEST;

typedef struct {
	int			cbSize;				// Set to sizeof();	
	GCDEST*		pDest;				// pointer to a GCDEST structure
	const char *pszText;			// Text, usage depends on type of event (see above), max 2048 characters
	const char *pszNick;			// Nick, usage depends on type of event (see above)
	const char *pszUID;				// Unique identifier, usage depends on type of event (see above)
	const char *pszStatus;			// Status, usage depends on type of event (see above)
	const char *pszUserInfo;		// Additional user information that is displayed in the log only for join, part, quit and nick
	BOOL		bIsMe;				// Is this event related to the user?
	BOOL		bAddToLog;			// Should this event be added to the message log
	DWORD		dwItemData;			// User specified data
	time_t		time;				// Time of the event
 } GCEVENT;
#define MS_GC_EVENT  "GChat/NewEvent"




//------------------------- HOOKS ------------------------
/*
	-- user interaction --
	Hook this to receive notifications about user commands. The below flags will tell what sort of 
	user interaction is taking place and is set in iType of the GCDEST pointer member. The other 
	members of GCDEST will tell what protocol and chat room name it is.

	wParam=0
	lParam=(LPARAM)(GCEVENT *)pgch
	
	Returning nonzero from your hook will stop other hooks from being called.
*/
#define GC_USER_MESSAGE				1 // user typed a message, with \n delimiting lines, valid members: pszText
#define GC_USER_CHANMGR				2 // user clicked the chat room settings button
#define GC_USER_LOGMENU				3 // user has chosen a message log menu item, valid members: dwData
#define GC_USER_NICKLISTMENU		4 // user has chosen a user list menu item, valid members: dwData
#define GC_USER_TYPNOTIFY			5 // user is typing
#define GC_USER_PRIVMESS			6 // user wants to talk privately to user, valid members: pszText, pszUID
#define GC_USER_TERMINATE			7 // a chat window is about to be closed, useful for freeing the Item data which is passed in dwData, valid members: dwData
#define ME_GC_EVENT  "GChat/OutgoingEvent"

typedef struct {
	GCDEST*		pDest;				// Same meaning as for MS_GC_EVENT
	char *		pszText;			// Text
	char *		pszUID;				// Unique identifier
	DWORD		dwData;				// user data
 } GCHOOK;


/*
	-- Build the pop up menus --
	The user is activating a right click menu and the protocol should tell what 
	Items should be added to the menu. You should have a static array of struct gc_item's.
	When the hook is fired the protocol should set nItems to the number of gc_item's 
	it want to add and then set Item to point to that array.

	wParam=0
	lParam=(LPARAM)(GCMENUITEM *)gcmi

  	Returning nonzero from your hook will stop other hooks from being called.

*/

#define MENU_NEWPOPUP		1		// add submenu
#define MENU_POPUPITEM		2		// add item to current submenu
#define MENU_POPUPSEPARATOR	3		// add separator to current submenu
#define MENU_SEPARATOR		4		// add separator to menu
#define MENU_ITEM			5		// add item
struct gc_item {
	char *			pszDesc;		// Textual description of the menu item to add				
	DWORD			dwID;			// must not be 0, must be unique. Will be returned via the above hook when the user click the item
	int				uType;			// What kind of item is it?
	BOOL			bDisabled;		// should the item be disabled
 };

#define MENU_ON_LOG			1		// pop up menu on the log
#define MENU_ON_NICKLIST	2		// pop up menu on the user list
typedef struct {
	char *			pszModule;		// Set by chat.dll to the protocol name, do not change.				
	char *			pszID;			// The unique identifier of the window 
	char *			pszUID;			// The unique identifier of the user, if clicked in the user list
	int				Type;			// MENU_ON_LOG or MENU_ON_USERLIST, what menu type is it?
	int				nItems;			// set to number of items
	struct gc_item*	Item;			// pointer to the first in the array of gc_item's
 } GCMENUITEMS;
#define ME_GC_BUILDMENU  "GChat/BuildMenu"

