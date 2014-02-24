/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
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

#ifndef M_CLUI_H__
#define M_CLUI_H__ 1

//this module was created in 0.1.1.0
//you probably shouldn't need to call anything in here. Look in
//ui/contactlist/m_clist.h instead

//gets the handle for the contact list window
//wParam = lParam = 0
//returns the HWND
//This call has a few very specific purposes internally in Miranda, and
//shouldn't be gratuitously used. In almost all cases there's another call to
//do whatever it is you are trying to do.
#define MS_CLUI_GETHWND     "CLUI/GetHwnd"

//get ContactTree hwnd
#define MS_CLUI_GETHWNDTREE     "CLUI/GetHwndTree"

//a new group was created. Add it to the list
//wParam = (WPARAM)(HANDLE)hGroup
//lParam = newGroup
//returns 0 on success, nonzero on failure
//newGroup is set to 1 if the user just created the group, and 0 otherwise
//this is also called when the contact list is being rebuilt
//new groups are always created with the name "New Group"
#define MS_CLUI_GROUPADDED  "CLUI/GroupCreated"

//change the icon for a contact
//wParam = (MCONTACT)hContact
//lParam = iconid
//returns 0 on sucess, nonzero on failure
//iconid is an offset in the image list. see clist/geticonsimagelist
#define MS_CLUI_CONTACTSETICON  "CLUI/ContactSetIcon"

//remove a contact from the list
//wParam = (MCONTACT)hContact
//lParam = 0
//returns 0 on success, nonzero on failure
//this is not necessarily the same as a contact being actually deleted, since
//if a contact goes offline while 'hide offline' is on, this will be called
#define MS_CLUI_CONTACTDELETED  "CLUI/ContactDeleted"

//add a contact to the list
//wParam = (MCONTACT)hContact
//lParam = iconId
//returns 0 on success, nonzero on failure
//The caller processes the 'hide offline' setting, so the callee should not do
//further processing based on the value of this setting.
//warning: this will be called to re-add a contact when they come online if
//'hide offline' is on, but it cannot determine if the contact is already on
//the list, so you may get requests to add a contact when it is already on the
//list, which you should ignore.
//You'll also get this whenever an event is added for a contact, since if the
//contact was offline it needs to be shown to display the message, even if hide
//offline is on.
//You should not re-sort the list on this call. A separate resort request will
//be sent
//iconid is an offset in the image list. see clist/geticonsimagelist
#define MS_CLUI_CONTACTADDED    "CLUI/ContactAdded"

//rename a contact in the list
//wParam = (MCONTACT)hContact
//lParam = 0
//returns 0 on success, nonzero on failure
//you should not re-sort the list on this call. A separate resort request will
//be sent
//you get the new name from clist/getcontactdisplayname
#define MS_CLUI_CONTACTRENAMED  "CLUI/ContactRenamed"

//start a rebuild of the contact list
//wParam = lParam = 0
//returns 0 on success, nonzero on failure
//this is the cue to clear the existing contents of the list
//expect to get a series of clui/groupadded calls followed by a series of
//clui/contactadded calls, then a clui/resortlist
#define MS_CLUI_LISTBEGINREBUILD  "CLUI/ListBeginRebuild"

//end a rebuild of the contact list
//wParam = lParam = 0
//returns 0 on success, nonzero on failure
//if you displayed an hourglass in beginrebuild, set it back here
//you do not need to explicitly sort the list
#define MS_CLUI_LISTENDREBUILD  "CLUI/ListEndRebuild"

//Gets a load of capabilities for the loaded CLUI    v0.1.2.1+
//wParam = capability, CLUICAPS_*
//lParam = 0
//returns the requested value, 0 if wParam is an unknown value
//If this service is not implemented, it is assumed to return 0 to all input

#define CLUICAPS_FLAGS1   0
#define CLUIF_HIDEEMPTYGROUPS   1   //the clist has a checkbox in its options
												//to set this, which will be hidden if this flag is not set. It is
												//up to the CLUI to provide support for it, but it just seemed insane
												//to me to have hide offline and hide empty in different pages.
												//The setting is "CList"/"HideEmptyGroups", a byte. A complete list
												//reload is sent whenever the user changes it.
#define CLUIF_DISABLEGROUPS     2   //can show list without groups. Adds option
												//to change "CList"/"UseGroups", a byte.
#define CLUIF_HASONTOPOPTION    4   //the clui options page provides a way to
												//change "CList"/"OnTop", a byte. When it is changed the clui should
												//also set the topmost flag on the window using SetWindowPos().
#define CLUIF_HASAUTOHIDEOPTION 8   //the clui options page provides a way to
												//change the list auto hide options. It should read and write the
												//byte "CList"/"AutoHide" and the word "CList"/"HideTime". No other
												//action is needed.

#define CLUICAPS_FLAGS2   1			//Returns info about extra icons
												//HIWORD is the first extra icon number, LOWORD is the extra icons count

#define MS_CLUI_GETCAPS         "CLUI/GetCaps"

//a contact is being dragged outside the main window     v0.1.2.0+
//wParam = (MCONTACT)hContact
//lParam = MAKELPARAM(screenX, screenY)
//return nonzero to make the cursor a 'can drop here', or zero for 'no'
#define ME_CLUI_CONTACTDRAGGING     "CLUI/ContactDragging"

//a contact has just been dropped outside the main window   v0.1.2.0+
//wParam = (MCONTACT)hContact
//lParam = MAKELPARAM(screenX, screenY)
//return nonzero if your hook processed this, so no other hooks get it
#define ME_CLUI_CONTACTDROPPED      "CLUI/ContactDropped"

//a contact that was being dragged outside the main window has gone back in to
//the main window.                                          v0.1.2.1+
//wParam = (MCONTACT)hContact
//lParam = 0
//return zero
#define ME_CLUI_CONTACTDRAGSTOP     "CLUI/ContactDragStop"

// return TRUE if Clist Module Support Metacontacts
#define MS_CLUI_METASUPPORT         "CLUI/MetaContactSupport"


#endif // M_CLUI_H__
