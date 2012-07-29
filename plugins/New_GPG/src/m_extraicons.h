/*
 Copyright (C) 2009 Ricardo Pescuma Domenecci

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

#ifndef __M_EXTRAICONS_H__
#define __M_EXTRAICONS_H__

#define MIID_EXTRAICONSSERVICE { 0x62d80749, 0xf169, 0x4592, { 0xb4, 0x4d, 0x3d, 0xd6, 0xde, 0x9d, 0x50, 0xc5 } }


#define EXTRAICON_TYPE_CALLBACK 0 	// Similar to old clist callbacks, it fires 2 notifications
#define EXTRAICON_TYPE_ICOLIB 1 	// This extra icon will use only icons registered with icolib. No callbacks
									// needed. Just call MS_EXTRAICON_SET_ICON passing the name of the extraicon to set one.


typedef struct {
	int cbSize;
	int type;						// One of EXTRAICON_TYPE_*
	const char *name;				// Internal name. More than one plugin can register extra icons with the same name
									// if both have the same type. In this case, both will be handled as one.
									// This is usefull for ex for extra status, where icq and jabber can share the same slot.
									// If the types are different the second one will be denied.
	const char *description;		// [Translated by plugin] Description to be used in GUI
	const char *descIcon;			// [Optional] Name of an icon registered with icolib to be used in GUI.

	// If type == EXTRAICON_TYPE_CALLBACK this two must be set

	// Callback to add icons to clist, calling MS_CLIST_EXTRA_ADD_ICON
	// wParam=lParam=0
	MIRANDAHOOK RebuildIcons;

	// Callback to set the icon to clist, calling MS_CLIST_EXTRA_SET_ICON or MS_EXTRAICON_SET_ICON
	// wParam = HANDLE hContact
	// lParam = int slot
	MIRANDAHOOK ApplyIcon;

	// Other optional callbacks

	// [Optional] Callback called when extra icon was clicked
	// wParam = HANDLE hContact
	// lParam = int slot
	// param = onClickParam
	MIRANDAHOOKPARAM OnClick;

	LPARAM onClickParam;

} EXTRAICON_INFO;


// Register an extra icon
// wParam = (EXTRAICON_INFO *) Extra icon info
// lParam = 0
// Return: (HANDLE) id of extra icon on success, 0 on error
#define MS_EXTRAICON_REGISTER "ExtraIcon/Register"


typedef struct {
	int cbSize;
	HANDLE hExtraIcon;				// Value returned by MS_EXTRAICON_REGISTER
	HANDLE hContact;				// Contact to set the extra icon
	union {							// The icon to be set. This depends on the type of the extra icon:
		HANDLE hImage; 				// Value returned by MS_CLIST_EXTRA_ADD_ICON (if EXTRAICON_TYPE_CALLBACK)
		const char *icoName;		// Name of the icon registered with icolib (if EXTRAICON_TYPE_ICOLIB)
	};
} EXTRAICON;

// Set an extra icon icon
// wParam = (EXTRAICON *) Extra icon
// Return: 0 on success
#define MS_EXTRAICON_SET_ICON "ExtraIcon/SetIcon"



#ifndef _NO_WRAPPERS
#ifdef __cplusplus

static HANDLE ExtraIcon_Register(const char *name, const char *description, const char *descIcon,
								 MIRANDAHOOK RebuildIcons,
								 MIRANDAHOOK ApplyIcon,
								 MIRANDAHOOKPARAM OnClick = NULL, LPARAM onClickParam = NULL)
{
	if (!ServiceExists(MS_EXTRAICON_REGISTER))
		return NULL;

	EXTRAICON_INFO ei = {0};
	ei.cbSize = sizeof(ei);
	ei.type = EXTRAICON_TYPE_CALLBACK;
	ei.name = name;
	ei.description = description;
	ei.descIcon = descIcon;
	ei.RebuildIcons = RebuildIcons;
	ei.ApplyIcon = ApplyIcon;
	ei.OnClick = OnClick;
	ei.onClickParam = onClickParam;

	return (HANDLE) CallService(MS_EXTRAICON_REGISTER, (WPARAM) &ei, 0);
}

static HANDLE ExtraIcon_Register(const char *name, const char *description, const char *descIcon = NULL,
								 MIRANDAHOOKPARAM OnClick = NULL, LPARAM onClickParam = NULL)
{
	if (!ServiceExists(MS_EXTRAICON_REGISTER))
		return NULL;

	EXTRAICON_INFO ei = {0};
	ei.cbSize = sizeof(ei);
	ei.type = EXTRAICON_TYPE_ICOLIB;
	ei.name = name;
	ei.description = description;
	ei.descIcon = descIcon;
	ei.OnClick = OnClick;
	ei.onClickParam = onClickParam;

	return (HANDLE) CallService(MS_EXTRAICON_REGISTER, (WPARAM) &ei, 0);
}

static int ExtraIcon_SetIcon(HANDLE hExtraIcon, HANDLE hContact, HANDLE hImage)
{
	EXTRAICON ei = {0};
	ei.cbSize = sizeof(ei);
	ei.hExtraIcon = hExtraIcon;
	ei.hContact = hContact;
	ei.hImage = hImage;

	return CallService(MS_EXTRAICON_SET_ICON, (WPARAM) &ei, 0);
}

static int ExtraIcon_SetIcon(HANDLE hExtraIcon, HANDLE hContact, const char *icoName)
{
	EXTRAICON ei = {0};
	ei.cbSize = sizeof(ei);
	ei.hExtraIcon = hExtraIcon;
	ei.hContact = hContact;
	ei.icoName = icoName;

	return CallService(MS_EXTRAICON_SET_ICON, (WPARAM) &ei, 0);
}

#endif
#endif


#endif // __M_EXTRAICONS_H__
