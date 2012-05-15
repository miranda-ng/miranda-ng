/* 
Copyright (C) 2006 MattJ, Ricardo Pescuma Domenecci

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


#ifndef __M_AVATARHISTORY_H__
# define __M_AVATARHISTORY_H__


#define EVENTTYPE_AVATAR_CHANGE 9003


/*
Return TRUE is Avatar History is enabled for this contact

wParam: hContact
lParam: ignored
*/
#define MS_AVATARHISTORY_ENABLED				"AvatarHistory/IsEnabled"


/*
Get cached avatar

wParam: (char *) protocol name
lParam: (char *) hash 
return: (TCHAR *) NULL if none is found or the path to the avatar. You need to free this string 
        with mir_free.
*/
#define MS_AVATARHISTORY_GET_CACHED_AVATAR		"AvatarHistory/GetCachedAvatar"





#endif // __M_AVATARHISTORY_H__
