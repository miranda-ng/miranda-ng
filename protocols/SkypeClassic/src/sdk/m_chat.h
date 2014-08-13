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



/*
	This plugin provides event driven chat rooms for protocols that wish to use it.
	It is built for IRC, which I also develop and is naturally biased towards IRC,
	but it should work very well with other protocols too. I will try to explain as
	careful as possible in this document how to use chat.dll

	-- General guidelines --

	There is ONE rule a protocol MUST follow to use this:

	1. Do NOT touch hContacts that has a byte "ChatRoom" set to ANYTHING other than 0! (Could be 1, 2, 3, ...)
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

	NOTE. Chat keeps its own copies of strings passed.


	* * Example of implementing this rule * *:
	* * This is a code snippet that is common in protocols * *:



	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !lstrcmpi(szProto, PROTONAME))
		{
			... do something with the hContact here;
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}



	* * You should do this instead * *:



	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !lstrcmpi(szProto, PROTONAME))
		{
			if (DBGetContactSettingByte(hContact, PROTONAME, "ChatRoom", 0) == 0)
			{
				... do something with the hContact here;
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}


	There is not more to it than that! To recapitulate: do NOT touch contacts where the
	BYTE "ChatRoom" is set to anything other than 0,






	OK, enough of the precautions, HOW DO YOU USE CHAT? In short you need to do FOUR things:

	1. REGISTER your protocol with Chat
	Only registered protocols can use Chat

	2. CREATE SESSIONS when your protocol are joining a group chat room. (One per room joined)
	These sessions will be put on the contact-list and are managed totally by chat.
	This is the reason you must obey to the "precautions" I mentioned above.
	Do not tamper directly with Chat's hContacts. Use Services provided	by Chat instead.

	3. SEND EVENTS to the sessions created in #3.
	These events reflect users joining/leaving/speaking etc.

	4. DESTROY SESSIONS when the user leaves the room (ie the session is not needed anymore).

	These four points are implemented in three services: MS_GC_REGISTER, MS_GC_NEWSESSION
	and MS_GC_EVENT.
*/


//------------------------- SERVICES ------------------------
/*
	Step 1. -- REGISTER with Chat --

	The first thing that a protocol need to do is register with Chat. This is best done
	when ALL modules has loaded (ME_SYSTEM_MODULESLOADED). The registration is
	needed to make sure that the protocol obey rule 1 mentioned above, but also to
	set protocol specific preferences.

	* Use MS_GC_REGISTER like this: CallService(MS_GC_REGISTER, 0, (LPARAM)(GCREGISTER *) &gcr;

	* returns 0 on success or error code on failure.
*/

// Flags
#define GC_BOLD            0x0001 // enable the 'bold' button
#define GC_ITALICS         0x0002 // enable the 'italics' button
#define GC_UNDERLINE	      0x0004 // enable the 'underline' button
#define GC_COLOR           0x0008 // enable the 'foreground color' button
#define GC_BKGCOLOR        0x0010 // enable the 'background color' button
#define GC_ACKMSG          0x0020 // the protocol must acknowlege messages sent
#define GC_TYPNOTIF        0x0040 // enable typing notifications.
#define GC_CHANMGR         0x0080 // enable the 'channel settings' button
#define GC_SINGLEFORMAT    0x0100 // the protocol supports only 1 formatting per message
#define GC_FONTSIZE        0x0200 // enable font size selection

// Error messages
#define GC_REGISTER_WRONGVER	1   // You appear to be using the wrong version of this API. Registration failed.
#define GC_REGISTER_ERROR		2   // An internal error occurred. Registration failed.
#define GC_REGISTER_NOUNICODE	3   // MS_GC_REGISTER returns this error if the Unicode version of chat
                                  // is not installed and GC_UNICODE is set. Registration failed

// GCREGISTER struct
struct GCREGISTER
{
	int        cbSize;            // Set to sizeof(GCREGISTER);
	DWORD      dwFlags;           // Use GC_* flags above to indicate features supported
	LPCSTR     pszModule;         // This MUST be the protocol name as registered with Miranda IM
	LPCTSTR    ptszDispName;      // This is the protocol's real name as it will be displayed to the user
	int        iMaxText;          // Max message length the protocol supports. Will limit the typing area input. 0 = no limit
	int        nColors;           // Number of colors in the colorchooser menu for the color buttons. Max = 100
	COLORREF*  pColors;           // pointer to the first item in a static COLORREF array containing the colors
	                              // that should be showed in the colorchooser menu.
	                              // ie:	COLORREF crCols[nColors];
	                              //	pColors = &crCols[0];
};

#define MS_GC_REGISTER  "GChat/Register"

/*
	Step 2. -- CREATE a new SESSION --

	Create a new session (chat room) and set various settings related to it.
	The chat room will not be shown to the user until the 'set up' phase is
	completed and SESSION_INITDONE is sent. See the MS_GC_EVENT for that.

	* Use MS_GC_NEWSESSION like this: CallService(MS_GC_NEWSESSION, 0, (LPARAM)(GCSESSION *) &gcr;

	* returns 0 on success or error code on failure
*/


// Session type
#define GCW_CHATROOM 1  // the session is a dedicated multi user chat room. ex "IRC channels".
                        // A hContact will be added for the session
#define GCW_SERVER   2  // the session is used as a network console. ex "IRC server window"
                        // A hContact will be added for the session, but it will default to being hidden (on the CList)
#define GCW_PRIVMESS 3  // NOT SUPPORTED YET! the session is a 1 to 1 session, but with additional
                        // support for adding more users etc. ex "MSN session".

// Error messages
#define GC_NEWSESSION_WRONGVER  1  // You appear to be using the wrong version of this API.
#define GC_NEWSESSION_ERROR     2  // An internal error occurred.

// GCSESSION structure
struct GCSESSION
{
	int     cbSize;             // set to sizeof(GCSESSION);
	int     iType;              // Use one of the GCW_* flags above to set the type of session
	LPCSTR pszModule;           // The name of the protocol owning the session (the same as pszModule when you register)
	LPCTSTR ptszName;			    // The name of the session as it will be displayed to the user
	LPCTSTR ptszID;             // The unique identifier for the session.
	LPCTSTR ptszStatusbarText;  // Optional text to set in the statusbar of the chat room window, or NULL.
	DWORD   dwFlags;
	INT_PTR dwItemData;         // Set user defined data for this session. Retrieve it by using the GC_EVENT_GETITEMDATA event
};

#define MS_GC_NEWSESSION  "GChat/NewChat"

/*
	Step 3 -- SEND an EVENT --

	Events is what drives Chat! After having created the session in Step 2
	it is time to make it work for real. Follow these guidelines:

	1. Start off by telling Chat what possible statuses a user can have (in the nicklist)
	by sending GC_EVENT_ADDGROUP as many times as needed. Also supply an icon
	to go with this status. Ex "Voice status" on IRC

	2.Then send "JOIN" events (GC_EVENT_JOIN) to populate the user list.
	You will need to send one event per user that should be added. As long as
	SESSION_INITDONE has not been sent these events will not show up in the log.

	3.When you are done with filling the user list it is a good time to end
	the set up phase and make the window visible by calling GC_EVENT_CONTROL event
	with wParam = SESSION_INITDONE.

	4.You will also want to send a GC_EVENT_CONTROL with wParam = SESSION_ONLINE to
	make the statusbar and the CList item go to "online" status

	You have now set up the session and made it active. A CList hContact has been added
	to the contact list and a chat room window is associated to the session. Send EVENTS to
	Chat users speaking, users joining and so on. See below for full
	list of what events are possible.

	IMPORTANT: For sending events you'll use the GCEVENT and GCDEST structures.	A GCDEST
	structure pointer is passed inside GCEVENT and it tells Chat what event type it is
	and what session it is related to. The GCDEST structure and its members are ALWAYS
	used (but the members can be NULL in some occasions). Depending on what type of event
	you are sending, the members of GCEVENT have different usage. Each event and how to
	use the members are discussed below. The "AddToLog" and "time" members are always valid
	and always mean the same. bAddToLog = TRUE means that the event is added to the disk log
	(at least when this makes sense). This can be used by Jabber for instance, when
	it needs to add channel history to the window, but without logging to disk.
	The "time" member is the timestamp of the event.(Tip. use the function time(NULL)
	to set the current time)

	NOTE. It is possible to send formatted text (bold, italics, underlined, foreground color
	and background color) to Chat by using the following identifiers in the text (pszText):

	%cXX		- set the foreground color ( XX is the zero based decimal index of the color registered in MS_GC_REGISTER.. Always use two digits )
	%C			- reset foreground color to default
	%fXX		- set the background color ( XX is the zero based decimal index of the color registered in MS_GC_REGISTER.. Always use two digits )
	%F			- reset the background color to default
	%b			- enable bold
	%B			- disable bold
	%u			- enable underlined
	%U			- disable underlined
	%i			- enable italics
	%I			- disable italics
	%r			- reset all to default
	%%			- escape the formatting. Translates to %

	IMPORTANT. If you have specified GC_COLOR or GC_BKGCOLOR when you registered you can expect to
	get these identifiers in the text you receive from Chat as well. Make sure % is ALWAYS
	translated to %% in text you send to Chat to avoid accidental formatting.
	NOTE. You will not get %cRRRGGGBBB back, instead you will get the index of the colour as
	registered with GC_REGISTER. Eg %c3 (the fourth colour of your index)

	* Use MS_GC_EVENT like this: CallService(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce;

	* returns 0 on success or error code on failure

*/

//	* List of possible events to send to Chat. Unlisted members are not valid	*
//	* for the event. Listed members are mandatory unless otherwise specified	*


//	GC_EVENT_JOIN - "<pszNick> has joined" (A user is joining the session)
//	pszNick		- Display name
//	pszUID		- Unique identifier of the user
//	pszStatus	- Which status does the user have. Should be a status previously
//					registered with GC_EVENT_ADDGROUP. Ex "Voice" in IRC
//	bIsMe		- Set to TRUE if it is the Miranda user
//					Chat needs to know which user in the userlist that is "self"
//					It cannot highlight a message containing the "own" nick without this info
// NOTE. if time == NULL, then the event will not be shown in the message log
#define GC_EVENT_JOIN			0x0001

//	GC_EVENT_PART - "<pszNick> has left: <pszText>" (A user left the session)
//	pszNick		- Display name
//	pszUID		- Unique identifier
//	pszText		- Optional part message, can be NULL
#define GC_EVENT_PART			0x0002

//	GC_EVENT_QUIT - "<pszNick> disconnected: pszText" (A user disconnected from the network)
//	pszID(in GCDEST)	- Should be NULL as a disconnect event is global.
//	pszNick				- Display name
//	pszUID				- Unique identifier
//	pszText				- Optional disconnect message, can be NULL
#define GC_EVENT_QUIT			0x0004

//	GC_EVENT_KICK - "<pszStatus> kicked <pszNick>: <pszText>" (A user is kicking another user from the room)
//	pszNick				- Display name of the one being being kicked
//	pszUID				- Unique identifier of the one being kicked
//	pszStatus			- Name of user who is doing the kicking
//	pszText				- Optional kick message, can be NULL
#define GC_EVENT_KICK			0x0008

//	GC_EVENT_NICK - "<pszNick> is now known as <pszText>" (A user changed his name)
//	NOTE, see GC_EVENT_CHUID also
//	pszID(in GCDEST)	- Should be NULL as a nick change event is global.
//	pszNick				- Old display name
//	pszUID				- Unique identifier
//	pszText				- New display name of the user. Color codes are not valid
#define GC_EVENT_NICK			0x0010

//	GC_EVENT_NOTICE - "Notice from <pszNick>: <pszText>" (An IRC type notice)
//	pszID(in GCDEST)	- Should be NULL to send to the active window
//	pszNick				- Display name
//	pszUID				- Unique identifier
//	pszText				- Notice text
#define GC_EVENT_NOTICE			0x0020

//	GC_EVENT_MESSAGE - "<pszNick>: <pszText> (A user is speaking)
//	pszNick		- Display name
//	pszUID		- Unique identifier
//	bIsMe		- Set to TRUE if it is the Miranda user
//	pszText				- Message text.
#define GC_EVENT_MESSAGE		0x0040

//	GC_EVENT_TOPIC  - "Topic is <pszText> (Set by: <pszNick>" (The room topic was changed/set)
//	pszNick		- Optional display name of who set the topic, can be NULL
//	pszUID		- Optional unique identifier of who set the topic, can be NULL
//	pszText		- Topic text
#define GC_EVENT_TOPIC			0x0080

//	GC_EVENT_INFORMATION  (Informational text) Ex a server response to a /WHO command in IRC
//	pszID(in GCDEST)	- NULL to send to the active window
//	pszText				- Information text
#define GC_EVENT_INFORMATION	0x0100

//	GC_EVENT_ACTION - "<pszNick> <pszText>" (An IRC Style action event)
//	pszNick		- Display name
//	pszUID		- Unique identifier
//	bIsMe		- Set to TRUE if it is the Miranda user
//	pszText		- Message text.
#define GC_EVENT_ACTION			0x0200

//	GC_EVENT_ADDSTATUS - "<pszText> enables '<pszStatus>' for <pszNick>" (A status change has occured for a user)
//	NOTE. Status changes are cumulative. The user will show in the nicklist with the highest status received.
//			Ex, IRC users can have "Op" and "Voice" statuses simultaneously but s/he will be displayed as "Op"
//	pszNick		- Display name of the one who receives a new status
//	pszUID		- Unique identifier of the one who receives a new status
//	pszText		- The display name of the one who is setting the status. Color codes are not valid
//	pszStatus	- The status. Should be a status previously
//					registered with GC_EVENT_ADDGROUP. Ex "Voice" in IRC
#define GC_EVENT_ADDSTATUS		0x0400

//	GC_EVENT_REMOVESTATUS - "<pszText> disables '<pszStatus>' for <pszNick>" (A status change has occured for a user)
//	NOTE. Status changes are cumulative. The user will show in the nicklist with the highest status received.
//			Ex, IRC users can have "Op" and "Voice" statuses simultaneously but s/he will be displayed as "Op"
//	pszNick		- Display name of the one who got a status mode disabled
//	pszUID		- Unique identifier of the one who got a status mode disabled
//	pszText		- The display name of the one disabling the status. Color codes are not valid
//	pszStatus		- The status. Should be a status previously
//					registered with GC_EVENT_ADDGROUP. Ex "Voice" in IRC
#define GC_EVENT_REMOVESTATUS	0x0800

//	GC_EVENT_CHUID - not shown in the log (Change the unique identifier of a contact)
//	pszID(in GCDEST)	- Should be NULL as a unique id's are global.
//	pszUID				- The current unique identifier
//	pszText				- The new unique identifier. Color codes are not valid
#define GC_EVENT_CHUID			0x1000

//	GC_EVENT_CHANGESESSIONAME - not shown in the log (Change the display name of a session)
//	pszText		- The new name. Color codes are not valid
#define GC_EVENT_CHANGESESSIONAME		0x1001

//	GC_EVENT_ADDGROUP - not shown in the log (Add a possible status mode to the nicklist, ex IRC uses "Op", "Voice", "Normal" etc )
//	NOTE. When adding several statuses, start with the highest status
//	pszStatus		- The new group name
//	dwItemData		- Optional HICON handle to a 10x10 icon. Set to NULL to use the built in icons.
#define GC_EVENT_ADDGROUP		0x1002

//	GC_EVENT_SETITEMDATA & GC_EVENT_SETITEMDATA - not shown in the log (Get/Set the user defined data of a session)
//	dwItemData		- The itemdata to set or get
#define GC_EVENT_SETITEMDATA	0x1003
#define GC_EVENT_GETITEMDATA	0x1004

//	GC_EVENT_SETSBTEXT - not shown in the log (Set the text of the statusbar for a chat room window)
//	pszText		- Statusbar text. Color codes are not valid
#define GC_EVENT_SETSBTEXT		0x1006

//	GC_EVENT_ACK - not shown in the log (Acknowledge a outgoing message, when GC_ACKMSG is set
#define GC_EVENT_ACK			0x1007

//	GC_EVENT_SENDMESSAGE - not shown in the log ("Fake" a message from a chat room as if the user had typed it). Used by IRC to broadcast /AME and /AMSG messages
//	pszText		- The text
#define GC_EVENT_SENDMESSAGE	0x1008

//	GC_EVENT_SETSTATUSEX - not shown in the log (Space or tab delimited list of pszUID's to indicate as away).
//  Used by IRC to mark users as away in the nicklist. If UIDs can contain spaces, use tabs
//	pszText		- Space or tab delimited list of pszUID's

#define GC_SSE_ONLYLISTED     0x0001  // processes only listed contacts, resets all contacts otherwise
#define GC_SSE_ONLINE         0x0002  // displays a contact online, otherwise away
#define GC_SSE_TABDELIMITED   0x0004  // use tabs as delimiters
#define GC_SSE_OFFLINE        0x0008  // displays a contact offline, otherwise away

#define GC_EVENT_SETSTATUSEX	0x1009

//	GC_EVENT_SETCONTACTSTATUS - sets status icon for contact
//	pszUID		- Unique identifier of the one who receives a new status
//	dwItemData	- (DWORD)ID_STATUS_* or zero to remove status icon
#define GC_EVENT_SETCONTACTSTATUS	0x100A

//	GC_EVENT_CONTROL  - not shown in the log (Control window associated to a session and the session itself)
//	NOTE 1: No members of GCEVENT are used, send one of the below flags in wParam instead,
//		Ex CallService(GC_EVENT_CONTROL, SESSION_INITDONE, (LPARAM)&gce);
//	NOTE 2: The first four control events are the only ones you should use most likely!
//		The ones below them are used by IRC to join channels hidden or maximized and show the server window from the system menu.
//		The SESSION_VISIBLE, SESSION_HIDDEN, SESSION_MAXIMIZE and SESSION_MINIMIZE events CAN replace SESSION_INITDONE but I urge you not to
//		do that as it will override any settings the user has made in the Chat options
//	NOTE 3: If pszID (of GCDEST) = NULL then this message will be broadcasted to all sessions, which can be usefule for terminating
//		all sessions when the protocol was disconnected
#define SESSION_INITDONE		1   // send this when the session is fully set up (all users have ben added to the nicklist)
#define SESSION_TERMINATE		7   // send to terminate a session and close the window associated with it
#define SESSION_OFFLINE			8   // send to set the session as "online" (hContact is set to Online etc)
#define SESSION_ONLINE			9   // send to set the session as "offline" (hContact is set to Offline etc)

#define WINDOW_VISIBLE			2   // make the room window visible
#define WINDOW_HIDDEN			3   // close the room window. Session is not terminated.
#define WINDOW_MAXIMIZE			4   // make the room window maximized
#define WINDOW_MINIMIZE			5   // make the room window minimized
#define WINDOW_CLEARLOG			6   // clear the log of the room window

#define GC_EVENT_CONTROL		0x1005

// Error messages
#define GC_EVENT_WRONGVER		1   // You appear to be using the wrong version of this API.
#define GC_EVENT_ERROR			2   // An internal error occurred.

// The GCDEST structure. It is passed to Chat inside GCEVENT.
struct GCDEST
{
	LPCSTR  pszModule;             // Name of the protocol (same as you registered with)
	LPCTSTR ptszID;                // Unique identifier of the session, or NULL to broadcast to all sessions as specified above
	int     iType;                 // Use GC_EVENT_* as defined above. Only one event per service call.
};

// The GCEVENT structure
struct GCEVENT
{
	int     cbSize;                // set to sizeof(GCEVENT);
	GCDEST *pDest;                 // pointer to a GCDEST structure which specifies the session to receive the event
	LPCTSTR ptszText;					 //
	LPCTSTR ptszNick;					 //
	LPCTSTR ptszUID;					 //
	LPCTSTR ptszStatus;				 //
	LPCTSTR ptszUserInfo;			 //

	BOOL    bIsMe;                 // Is this event from the Miranda user?
	DWORD   dwFlags;               // event flags: GCEF_ADDTOLOG, GCEF_NOTNOTIFY

	INT_PTR dwItemData;            // User specified data.
	DWORD   time;                  // Timestamp of the event
};

#define MS_GC_EVENT  "GChat/NewEvent"

// This hook is fired when MS_GC_EVENT is called, with the same wParam and lParam as above.
// It allows external plugins to intercept chat events and display then in other ways
#define ME_GC_HOOK_EVENT "GChat/HookEvent"

#define GCEF_ADDTOLOG       0x0001
#define GCEF_REMOVECONTACT  0x0002
// Added in Miranda NG 0.94.4+
#define GCEF_NOTNOTIFY      0x0004

// OK! That was about everything that you need to know about for operating Chat in a basic way.
// There are however some more things you will need to know about. Some you may use and some you may not need,

/*
	 -- GETTING info about a SESSION or session data --

	Use this service to get information on different aspects of the sessions that are registered with Chat.

	* Use MS_GC_GETINFO like this: CallService(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)(char *) pszModule);
	* returns -1 on failure and the sessioncount on success
*/

#define MS_GC_GETSESSIONCOUNT  "GChat/GetCount"

/*
	 -- GETTING info about a SESSION or session data --
	Use this service to get information on different aspects of the sessions that are registered with Chat.

	* Use MS_GC_GETINFO like this: CallService(MS_GC_GETINFO, 0, (LPARAM)(GC_INFO *) &gci;
	* returns 0 on success or error code on failure
*/

// Flags
#define GCF_BYINDEX   0x0001   // iItem is valid and should contain the index of the session to get
#define GCF_BYID      0x0002   // pszID is valid and should contain the ID of the session to get. This is the default if no
#define GCF_HCONTACT  0x0004   // hContact is valid
#define GCF_DATA      0x0008   // wItemData is valid
#define GCF_ID        0x0010   // pszID is valid.
#define GCF_NAME      0x0020   // pszName is valid
#define GCF_TYPE      0x0040   // iType is valid
#define GCF_COUNT     0x0080   // iCount is valid
#define GCF_USERS     0x0100   // pszUsers is valid

// The GC_INFO structure
struct GC_INFO
{
	DWORD     Flags;        // use a combination of the above flags
	int       iItem;        // session type (GCW_*)
	int       iType;        // session type (GCW_*)
	LPCSTR    pszModule;    // the module name as registered in MS_GC_REGISTER
	LPCTSTR   pszID;        // unique ID of the session
	LPTSTR    pszName;      // display name of the session
	INT_PTR   dwItemData;   // user specified data.
	int       iCount;       // count of users in the nicklist
	LPSTR     pszUsers;     // space separated string containing the UID's of the users in the user list.
	                        // NOTE. Use Mirandas mmi_free() on the returned string.
	MCONTACT  hContact;     // hContact for the session (can be NULL)
};

#define MS_GC_GETINFO  "GChat/GetInfo"

//------------------------- HOOKS ------------------------
/*
	-- user interaction --
	Hook this to receive notifications about when user take actions in a chat room window.
	Check for the below flags to find out what type of user interaction it is. See the
	to find out which members of GCHOOK that are valid.

	* wParam=0
	* lParam=(LPARAM)(GCEVENT *)pgch

	* Returning nonzero from your hook will stop other hooks from being called.
*/

#define GC_USER_MESSAGE          1 // user sent a message, with \n delimiting lines, pszText contains the text.
#define GC_USER_CHANMGR          2 // user clicked the settings button in a chat room
#define GC_USER_LOGMENU          3 // user has selected a message log menu item, dwData is valid. See ME_GC_BUILDMENU
#define GC_USER_NICKLISTMENU     4 // user has selected a userlist menu item, valid members: dwData. See ME_GC_BUILDMENU
#define GC_USER_TYPNOTIFY        5 // NOT IMPLEMENTED YET! user is typing
#define GC_USER_PRIVMESS         6 // user requests to send a private message to a user. pszUID is valid
#define GC_SESSION_TERMINATE     7 // the session is about to be terminated, the "user defined data" is passed in dwData, which can be good free'ing any allocated memory.
#define GC_USER_LEAVE            8 // user requests to leave the session
#define GC_USER_CLOSEWND         9 // user closed the window (this is usually not an indication that the protocol
                                   // should take action, but MSN may want to terminate the session here)
#define ME_GC_EVENT  "GChat/OutgoingEvent"

struct GCHOOK
{
	GCDEST *pDest;       // pointer to a GCDEST structure which specifies from which session the hook was triggered
	LPTSTR  ptszText;    // usage depends on type of event
	LPTSTR  ptszUID;     // unique identifier, usage depends on type of event
	INT_PTR dwData;      // user defined data, usage depends on type of event
};

/*
	-- Build the pop up menus --
	The user wants to show a right click (popup) menu and your protocol should tell what
	items should be added to the menu. You should create a static array of struct gc_item's.
	When you get this notification you should set "nItems" to the number of gc_item's
	you want to show on the user's popup menu and then set the "Item" member to point to that array.

	* wParam=0
	* lParam=(LPARAM)(GCMENUITEM *)gcmi

  	Returning nonzero from your hook will stop other hooks from being called.
*/

// type of item to add to the popup menu
#define MENU_NEWPOPUP       1      // add submenu
#define MENU_POPUPITEM      2      // add item to current submenu
#define MENU_POPUPSEPARATOR 3      // add separator to current submenu
#define MENU_SEPARATOR      4      // add separator to menu
#define MENU_ITEM           5      // add item
#define MENU_POPUPCHECK     6      // add checked item to current submenu
#define MENU_CHECK          7      // add checked item
#define MENU_POPUPHMENU     8      // add custom submenu to current submenu, use dwID to specify HMENU
#define MENU_HMENU          9      // add custom submenu, use dwID to specify HMENU

// type of menu that is being requested
#define MENU_ON_LOG         1      // pop up menu on the message log
#define MENU_ON_NICKLIST    2      // pop up menu on the user list

// contains info on a menuitem to be added
struct gc_item {
	TCHAR *pszDesc;     // Textual description of the menu item to add
	DWORD  dwID;        // when/if the user selects this menu item this
							  // value will be returned via the above hook, GC_USER_LOGMENU
						 	  // or GC_USER_NICKLISTMENU. Must not be 0 and must be unique.
	int    uType;       // What kind of menu item is it? Use MENU_* flags above
	BOOL   bDisabled;   // should the menu item be shown as disabled
};

typedef struct {
	LPSTR    pszModule; // Contains the protocol name, do NOT change.
	LPTSTR   pszID;     // The unique identifier of the session that triggered the hook, do NOT change.
	LPTSTR   pszUID;    // Contains the unique identifier if Type = MENU_ON_NICKLIST. do NOT change.
   int      Type;      // Type of menu. MENU_ON_* flags used. do NOT change.
   int      nItems;    // Set this to the number of menu items you want to add
   gc_item *Item;      // pointer to the first in the array of gc_item's
}
	GCMENUITEMS;

#define ME_GC_BUILDMENU  "GChat/BuildMenu"

/*
	* Example of how to add 2 items to the popup menu for the userlist *

	GCMENUITEMS *gcmi= (GCMENUITEMS*) lParam;
  	if (gcmi->Type == MENU_ON_NICKLIST)
	{
		static struct gc_item Item[] = {
				{Translate("User &details"), 1, MENU_ITEM, FALSE},
				{Translate("&Op"), 2, MENU_POPUPITEM, FALSE},
		};

		gcmi->nItems = sizeof(Item)/sizeof(Item[0]);
		gcmi->Item = &Item[0];
		gcmi->Item[gcmi->nItems-1].bDisabled = bFlag;

		return 0;
	}
*/

//////////////////////////////////////////////////////////////////////////
// Get Chat ToolTip Text for buddy
// wParam = (WPARAM)(TCHAR*) roomID parentdat->ptszID
// lParam = (WPARAM)(TCHAR*) userID ui1->pszUID
// result (int)(TCHAR*)mir_tstrdup("tooltip text")
// returns pointer to text of tooltip and starts owns it
#define MS_GC_PROTO_GETTOOLTIPTEXT "/GetChatToolTipText"
