/* 
Copyright (C) 2006-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __M_HISTORYKEEPER_H__
# define __M_HISTORYKEEPER_H__


#define MIID_STATUS_MESSAGE_CHANGE_LOGGER { 0x821be252, 0xe20b, 0x41e7, { 0xa5, 0x1d, 0x3c, 0x34, 0x2e, 0x38, 0xae, 0x22 } }
#define MIID_STATUS_MESSAGE_CHANGE_NOTIFIER { 0xb628b23b, 0x47ae, 0x430e, { 0x94, 0x81, 0x15, 0x9f, 0xa7, 0x26, 0xc4, 0x3a } }
#define MIID_NICKNAME_CHANGE_LOGGER { 0x478be45e, 0xd331, 0x4d63, { 0xa6, 0x57, 0x85, 0xda, 0x45, 0xf8, 0xc, 0xe0 } }
#define MIID_NICKNAME_CHANGE_NOTIFIER { 0xc749d46a, 0x885e, 0x46bf, { 0xaa, 0x4c, 0xe1, 0xae, 0xc5, 0xc9, 0xd0, 0x93 } }

#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_NICKNAME_CHANGE 9001
#define EVENTTYPE_STATUSMESSAGE_CHANGE 9002
#define EVENTTYPE_CLIENT_CHANGE 9005
#define EVENTTYPE_XSTATUS_CHANGE 9006
#define EVENTTYPE_LISTENINGTO_CHANGE 9007
#define EVENTTYPE_XSTATUS_MESSAGE_CHANGE 9008
#define EVENTTYPE_IDLE_CHANGE 9009


/*
Return TRUE is Status Message History is enabled for this contact

wParam: hContact
lParam: ignored
*/
#define MS_SMH_ENABLED		"SMH/Enabled"


/*
Enable Status Message History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_SMH_ENABLE		"SMH/Enable"


/*
Disable Status Message History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_SMH_DISABLE		"SMH/Disable"


/*
Return TRUE is Nick History is enabled for this contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_ENABLED		"NickHistory/Enabled"


/*
Enable Nick History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_ENABLE		"NickHistory/Enable"


/*
Disable Nick History for a contact

wParam: hContact
lParam: ignored
*/
#define MS_NICKHISTORY_DISABLE		"NickHistory/Disable"



#endif // __M_HISTORYKEEPER_H__
