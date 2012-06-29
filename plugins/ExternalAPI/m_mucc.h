/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

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
#ifndef M_MUCC_INCLUDED
#define M_MUCC_INCLUDED

#define MUCC_WINDOW_CHATROOM		1
#define MUCC_WINDOW_CHATLIST		2
	
#define MUCC_WF_BOLD			0x0001		//enable the 'bold' button
#define MUCC_WF_ITALIC			0x0002		//enable the 'italics' button	
#define MUCC_WF_UNDERLINE		0x0004		//enable the 'underline' button
#define MUCC_WF_COLOR			0x0008		//enable the 'foreground color' button
#define MUCC_WF_BKGCOLOR		0x0010		//enable the 'background color' button
#define MUCC_WF_TYPNOTIF		0x0020		//enable typing notifications

typedef struct {
	int			cbSize;
	int	   		iType;					//Window type, MUCW_CHATROOM or MUCW_CHATLIST
	DWORD		dwFlags;				//Window look & feel flags
	const char *pszModule;				//Identifier of the module owning the window
	const char *pszModuleName;			//Name of the module as displayed to the user
	const char *pszID;					//Unique identifier of the chat room
	const char *pszName;				//Name of the chat room
	const char *pszUID;					//Identifier of the user
	const char *pszNick;				//Nick name used by the user in the chat room
	const char *pszStatusbarText;		//Optional text to set in the statusbar
} MUCCWINDOW;

#define MS_MUCC_NEW_WINDOW  "MUCC/NewWindow"

typedef struct {
	const char *pszID;					//Unique identifier of the group or the chat room
	const char *pszName;				//Name of the group or the chat room
	const char *pszNick;				//Name of the user
	const char *pszText;				//Text, usage depends on type of event
	int			iCount;					//Number of users in the chat room
	DWORD		dwFlags;				//Item flags
} MUCCQUERYITEM;

typedef struct {
	int			cbSize;
	int			iType;					//Query type, one of MUCC_EVENT_QUERY_* values
	const char *pszModule;				//Name of the module
	const char *pszParent;				//Parent of groups or the group chat rooms belong to
	int			iItemsNum;				//Number of items pItems iws pointing to
	MUCCQUERYITEM *pItems;				//Pointer to array of result items
	DWORD		dwFlags;				//Item flags
	int			iPage;					//Number of page in room list
	int			iLastPage;				//1 if the group has any child or there is a next page in room list, 0 otherwise
} MUCCQUERYRESULT;

#define MS_MUCC_QUERY_RESULT "MUCC/QueryResult"

/*
 * The following fields must be set for all events:
 * cbSize		- size of the structure
 * pszModule	- module name
 * iType		- event type (id)
 */
/*
 * Incoming messages
 */
#define MUCC_EVENT_MESSAGE			0x0001
/* pszID		- room ID
 * pszUID		- user UID (login etc.)
 * pszNick		- user nick (i.e. the name to be displayed)
 * pszText		- message
 * iFont		- font index
 * iFontSize	- font size
 * color		- text color
 * dwFlags		- flags
 * time			- time
 * bIsMe		- TRUE if this is an outgoing message, FALSE otherwise
 */
#define MUCC_EVENT_STATUS			0x0002
/* pszID		- room ID
 * pszUID		- user UID (login etc.)
 * pszNick		- user nick (i.e. the name to be displayed)
 * pszText		- unused
 * dwData		- status 
 * dwFlags
 * bIsMe
 */
#define MUCC_EVENT_INVITATION		0x0003
/*
*/
#define MUCC_EVENT_ERROR			0x0004

#define MUCC_EVENT_TOPIC			0x0005
/* pszID
 * pszText
 */
#define MUCC_EVENT_ROOM_INFO		0x0006

/*
 * Incoming & outgoing messages
 */
#define MUCC_EVENT_JOIN				0x0080
#define MUCC_EVENT_LEAVE			0x0081
/*
 * Outgoing messages
 */
#define MUCC_EVENT_INVITE			0x0102
#define MUCC_EVENT_START_PRIV		0x0103
#define MUCC_EVENT_REGISTER_NICK	0x0105
#define MUCC_EVENT_REMOVE_NICK		0x0106
#define MUCC_EVENT_REGISTER_ROOM	0x0107
#define MUCC_EVENT_REMOVE_ROOM		0x0108
#define MUCC_EVENT_KICK_BAN			0x0109
#define MUCC_EVENT_SET_USER_ROLE	0x010A
#define MUCC_EVENT_UNBAN			0x010B
/*
 * Queries
 */ 
#define MUCC_EVENT_QUERY_GROUPS		0x0120
#define MUCC_EVENT_QUERY_ROOMS		0x0121
#define MUCC_EVENT_QUERY_USER_ROOMS 0x0122
#define MUCC_EVENT_QUERY_USER_NICKS 0x0123
#define MUCC_EVENT_QUERY_SEARCH		0x0124
#define MUCC_EVENT_QUERY_CONTACTS	0x0130
#define MUCC_EVENT_QUERY_USERS		0x0131
/*
	Event flags
*/
#define MUCC_EF_FONT_BOLD			0x000100	// Bold font flag (MUCC_EVENT_MESSAGE)
#define MUCC_EF_FONT_ITALIC			0x000200	// Italic font flag (MUCC_EVENT_MESSAGE)
#define MUCC_EF_FONT_UNDERLINE		0x000400	// Underlined font flags (MUCC_EVENT_MESSAGE)
/*
#define MUCC_EF_FONT_NAME			0x000800
#define MUCC_EF_FONT_SIZE			0x001000
#define MUCC_EF_FONT_COLOR			0x002000
*/
#define MUCC_EF_ROOM_PUBLIC			0x000001
#define MUCC_EF_ROOM_NICKNAMES		0x000002
#define MUCC_EF_ROOM_PERMANENT		0x000004
#define MUCC_EF_ROOM_MEMBERS_ONLY	0x000008
#define MUCC_EF_ROOM_MODERATED		0x000020
#define MUCC_EF_ROOM_OFFICIAL		0x000040
#define MUCC_EF_ROOM_NAME			0x100000

#define MUCC_EF_USER_OWNER			0x000001
#define MUCC_EF_USER_ADMIN			0x000002
#define MUCC_EF_USER_REGISTERED		0x000004
#define MUCC_EF_USER_MEMBER			0x000008
#define MUCC_EF_USER_MODERATOR		0x000010
#define MUCC_EF_USER_BANNED			0x000020
#define MUCC_EF_USER_GLOBALOWNER	0x000040

typedef struct {
	int			cbSize;
	int			iType;				// Event type, one of MUCC_EVENT_* values
	DWORD		dwFlags;			// Event flags - MUCC_EF_*
	int			iFont;				// Text font index
	int			iFontSize;			// Text font size
	COLORREF	color;				// Text color
	const char *pszModule;			// Name of the module
	const char *pszID;				// Unique identifier of the chat room corresponding to the event,
	const char *pszName;			// Name of the chat room visible to the user
	const char *pszUID;				// User identifier, usage depends on type of event
	const char *pszNick;			// Nick, usage depends on type of event
	const char *pszText;			// Text, usage depends on type of event
	DWORD		dwData;				// DWORD data e.g. status
	BOOL		bIsMe;				// TRUE if the event is related to the user
	time_t		time;				// Time of the event
} MUCCEVENT;

#define MS_MUCC_EVENT  "MUCC/Event"
#define ME_MUCC_EVENT  "MUCC/OutgoingEvent"

#endif

