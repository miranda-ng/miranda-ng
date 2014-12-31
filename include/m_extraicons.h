/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2009 Ricardo Pescuma Domenecci

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

#ifndef EXTRA_ICON_COUNT
#define EXTRA_ICON_COUNT 10
#endif

#define EMPTY_EXTRA_ICON  0xFFFF

// Adding icon to extra image list.
// Used for EXTRAICON_TYPE_CALLBACK extra icons
//
// wparam = (HICON)hIcon
// lparam = 0 (unused)
// return HANDLE hImage on success or INVALID_HANDLE_VALUE on failure
#define MS_CLIST_EXTRA_ADD_ICON     "CListFrames/AddIconToExtraImageList"

//called with wparam=hContact
#define ME_CLIST_EXTRA_IMAGE_APPLY  "CListFrames/OnExtraImageApply"

//called with wparam=hContact lparam=extra
#define ME_CLIST_EXTRA_CLICK        "CListFrames/OnExtraClick"

//called with wparam=lparam=0
#define ME_CLIST_EXTRA_LIST_REBUILD "CListFrames/OnExtraListRebuild"

/*

There is 2 ways of registering with Extra Icons service:

1. Using callbacks
    This works similar to clist API. When you register you provide 2 callbacks, one to rebuild the icons
and one to apply the icons for a contact.
    In the RebuildIcons callback, all icons that will be used have to be registered calling
MS_CLIST_EXTRA_ADD_ICON service. The value returned by this service has to be stored and used in the
apply icons.
    The ApplyIcons callback will be called for all the needed contacts. Inside it, you must call
MS_EXTRAICON_SET_ICON to set the icon for the contact, sending the value returned by MS_CLIST_EXTRA_ADD_ICON
as the hImage.

2. Using icolib
    In this case no callback is needed and the plugin just need to call MS_EXTRAICON_SET_ICON passing the
icolib name in icoName when needed. If your plugin can have extra icons on startup, remember to do a loop
over all contacts to set the initial icon.


To register a new extra icon, you have to call MS_EXTRAICON_REGISTER passing the needed atributes.

*/

#define MIID_EXTRAICONSSERVICE { 0x62d80749, 0xf169, 0x4592, { 0xb4, 0x4d, 0x3d, 0xd6, 0xde, 0x9d, 0x50, 0xc5 } }


#define EXTRAICON_TYPE_CALLBACK 0 	// Similar to old clist callbacks, it fires 2 notifications
#define EXTRAICON_TYPE_ICOLIB 1 	// This extra icon will use only icons registered with icolib. No callbacks
									// needed. Just call MS_EXTRAICON_SET_ICON passing the name of the extraicon to set one.


typedef struct {
	int cbSize;
	int type;						// One of EXTRAICON_TYPE_*
	const char *name;				// Internal name. More than one plugin can register extra icons with the same name
									// if both have the same type. In this case, both will be handled as one.
									// If the types are different the second one will be denied.
	const char *description;		// [Translated by plugin] Description to be used in GUI
	const char *descIcon;			// [Optional] Name of an icon registered with icolib to be used in GUI.

	// If type == EXTRAICON_TYPE_CALLBACK this two must be set

	// Callback to add icons to clist, calling MS_CLIST_EXTRA_ADD_ICON
	// wParam=lParam=0
	MIRANDAHOOK RebuildIcons;

	// Callback to set the icon to clist, calling MS_CLIST_EXTRA_SET_ICON or MS_EXTRAICON_SET_ICON
	// wParam = MCONTACT hContact
	// lParam = 0
	MIRANDAHOOK ApplyIcon;

	// Other optional callbacks

	// [Optional] Callback called when extra icon was clicked
	// wParam = MCONTACT hContact
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
	HANDLE hExtraIcon;      // Value returned by MS_EXTRAICON_REGISTER
	MCONTACT hContact;        // Contact to set the extra icon
	union {                 // The icon to be set. This depends on the type of the extra icon:
		HANDLE hImage;       // Value returned by MS_CLIST_EXTRA_ADD_ICON (if EXTRAICON_TYPE_CALLBACK)
		                     // or the icolib icon handle (if EXTRAICON_TYPE_ICOLIB)
		const char *icoName; // Name of the icon registered with icolib (if EXTRAICON_TYPE_ICOLIB)
	};
} EXTRAICON;

// Set an extra icon icon
// wParam = (EXTRAICON *) Extra icon
// Return: 0 on success
#define MS_EXTRAICON_SET_ICON "ExtraIcon/SetIcon"

// Set an extra icon by icolib icon's name
// wParam = (EXTRAICON *) Extra icon
// Return: 0 on success
#define MS_EXTRAICON_SET_ICON_BY_NAME "ExtraIcon/SetIconByName"

#ifndef _NO_WRAPPERS
#ifdef __cplusplus

extern int hLangpack;

#pragma warning(disable:4505)

static HANDLE ExtraIcon_Register(
	const char *name, const char *description, const char *descIcon,
	MIRANDAHOOK RebuildIcons,
	MIRANDAHOOK ApplyIcon,
	MIRANDAHOOKPARAM OnClick = NULL, LPARAM onClickParam = 0)
{
	if (!ServiceExists(MS_EXTRAICON_REGISTER))
		return NULL;

	EXTRAICON_INFO ei = { sizeof(ei) };
	ei.type = EXTRAICON_TYPE_CALLBACK;
	ei.name = name;
	ei.description = description;
	ei.descIcon = descIcon;
	ei.RebuildIcons = RebuildIcons;
	ei.ApplyIcon = ApplyIcon;
	ei.OnClick = OnClick;
	ei.onClickParam = onClickParam;

	return (HANDLE) CallService(MS_EXTRAICON_REGISTER, (WPARAM) &ei, hLangpack);
}

static HANDLE ExtraIcon_Register(
	const char *name, const char *description, const char *descIcon = NULL,
	MIRANDAHOOKPARAM OnClick = NULL, LPARAM onClickParam = 0)
{
	if (!ServiceExists(MS_EXTRAICON_REGISTER))
		return NULL;

	EXTRAICON_INFO ei = { sizeof(ei) };
	ei.type = EXTRAICON_TYPE_ICOLIB;
	ei.name = name;
	ei.description = description;
	ei.descIcon = descIcon;
	ei.OnClick = OnClick;
	ei.onClickParam = onClickParam;

	return (HANDLE) CallService(MS_EXTRAICON_REGISTER, (WPARAM) &ei, hLangpack);
}

static int ExtraIcon_SetIcon(HANDLE hExtraIcon, MCONTACT hContact, HANDLE hImage)
{
	EXTRAICON ei = { sizeof(ei) };
	ei.hExtraIcon = hExtraIcon;
	ei.hContact = hContact;
	ei.hImage = hImage;

	return CallService(MS_EXTRAICON_SET_ICON, (WPARAM) &ei, 0);
}

static int ExtraIcon_SetIcon(HANDLE hExtraIcon, MCONTACT hContact, const char *icoName)
{
	EXTRAICON ei = { sizeof(ei) };
	ei.hExtraIcon = hExtraIcon;
	ei.hContact = hContact;
	ei.icoName = icoName;

	return CallService(MS_EXTRAICON_SET_ICON_BY_NAME, (WPARAM) &ei, 0);
}

static int ExtraIcon_Clear(HANDLE hExtraIcon, MCONTACT hContact)
{
	EXTRAICON ei = { sizeof(ei) };
	ei.hExtraIcon = hExtraIcon;
	ei.hContact = hContact;
	ei.icoName = NULL;

	return CallService(MS_EXTRAICON_SET_ICON, (WPARAM) &ei, 0);
}

#endif
#endif


#endif // __M_EXTRAICONS_H__
