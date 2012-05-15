/*
Status Message Change Notify plugin for Miranda IM.

Copyright © 2004-2005 NoName
Copyright © 2005-2007 Daniel Vijge, Tomasz S³otwiñski, Ricardo Pescuma Domenecci

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

#ifndef __SMCNOTIFY_M_SMCNOTIFY_H
#define __SMCNOTIFY_M_SMCNOTIFY_H


#define EVENTTYPE_STATUSMESSAGE_CHANGE 9002

/*
Enable/Disable status message change notification

wParam=
0 - used internaly, toggle popups, lParam is ignored
1 - enable/disable popups from within other plugins, see lParam
lParam=0
0 - disable popups
1 - enable popups
*/
#define MS_SMCNOTIFY_POPUPS		"SMCNotify/Popups"


/*
Show List with all the contact that have a status message set

wParam=lParam=ignored
*/
#define MS_SMCNOTIFY_LIST		"SMCNotify/ShowList"


/*
Go To URL in Status Message
used just internaly for now

wParam=lParam=ignored
*/
#define MS_SMCNOTIFY_GOTOURL	"SMCNotify/GoToURL"



#endif // __SMCNOTIFY_M_SMCNOTIFY_H
