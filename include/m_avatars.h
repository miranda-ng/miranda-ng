/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-12 Miranda ICQ/IM project,
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

Avatar service

- load and maintain a cache of contact avatars.
- draw avatars to a given target device context
- maintain per protocol fallback images

The avatar service builds on top of Mirandas core bitmap loading service (MS_UTILS_LOADBITMAP).
However, if imgdecoder.dll is installed in mirandas main or Plugins directory, it can be used
to support PNG images. The avatar service loads 32bit PNG images and peforms alpha channel
premultiplication so that these images can be rendered by using the Win32 AlphaBlend() API.

The cache grows on demand only, that is, no avatars are PREloaded. An avatar is only loaded
if a plugin requests this by using the MS_AV_GETAVATAR service. Since avatars may update
asynchronously, the avatar iamge may not be ready when a plugin calls the service. In that
case, an event (ME_AV_AVATARCHANGED) is fired when a contacts avatar changes. This event
is also fired, when a contact avatar changes automatically.

The service takes care about protocol capabilites (does not actively fetch avatars for
protocols which do not report avatar capabilities via PF4_AVATARS or for protocols which
have been disabled in the option dialog). It also does not actively fetch avatars for
protocols which are in invisible status mode (may cause privacy issues and some protocols
like MSN don't allow any outbound client communication when in invisible status mode)
unless AF_FETCHIFPROTONOTVISIBLE is set.

- TODO
- maintain recent avatars (store the last hashes to avoid re-fetching)
- cache expiration, based on least recently used algorithm.

(c) 2005 by Nightwish, silvercircle@gmail.com

*/

#ifndef _M_AVATARS_H
#define _M_AVATARS_H

#include <m_core.h>

#define AVS_BITMAP_VALID 1
#define AVS_BITMAP_EXPIRED 2        // the bitmap has been expired from the cache. (unused, currently.
#define AVS_HIDEONCLIST 4
#define AVS_PREMULTIPLIED 8         // set in the dwFlags member of the struct avatarCacheEntry for 32 bit transparent
                                    // images when loaded with imgdecoder. These images can be rendered transparently
                                    // using the AlphaBlend() API with AC_SRC_ALPHA
#define AVS_PROTOPIC 16             // picture is a protocol picture
#define AVS_CUSTOMTRANSPBKG 32      // Bitmap was changed to set the background color transparent
#define AVS_HASTRANSPARENCY 64      // Bitmap has at least one pixel transparent
#define AVS_OWNAVATAR 128           // is own avatar entry
#define AVS_NOTREADY  4096

typedef struct avatarCacheEntry
{
	DWORD cbSize;                    // set to sizeof(struct)
	MCONTACT hContact;               // contacts handle, 0, if it is a protocol avatar
	HBITMAP hbmPic;                  // bitmap handle of the picutre itself
	DWORD dwFlags;                   // see above for flag values
	LONG bmHeight, bmWidth;          // bitmap dimensions
	DWORD t_lastAccess;              // last access time (currently unused, but plugins should still
                                    // use it whenever they access the avatar. may be used in the future
	                                 // to implement cache expiration
	LPVOID lpDIBSection;             // unused field
	TCHAR szFilename[MAX_PATH];      // filename of the avatar (absolute path)
}
	AVATARCACHEENTRY;

#define AVDRQ_FALLBACKPROTO            0x0001        // use the protocol picture as fallback (currently not used)
#define AVDRQ_FAILIFNOTCACHED          0x0002        // don't create a cache entry if it doesn't already exist. (currently not working)
#define AVDRQ_ROUNDEDCORNER            0x0004        // draw with rounded corners
#define AVDRQ_DRAWBORDER               0x0008        // draw a border around the picture
#define AVDRQ_PROTOPICT                0x0010        // draw a protocol picture (if available).
#define AVDRQ_HIDEBORDERONTRANSPARENCY 0x0020        // hide border if bitmap has transparency
#define AVDRQ_OWNPIC	               0x0040        // draw own avatar (szProto is valid - use "" for global avatar)
#define AVDRQ_RESPECTHIDDEN            0x0080        // don't draw images marked as hidden
#define AVDRQ_DONTRESIZEIFSMALLER      0x0100        // don't resize images that are smaller then the draw area
#define AVDRQ_FORCEFASTALPHA           0x0200        // force rendering with simple AlphaBlend (will use FI_Resample otherwise)
#define AVDRQ_FORCEALPHA               0x0400        // force with simple AlphaBlend (may use StretchBlt otherwise)
#define AVDRQ_AERO					   0x0800		 // draw on aero surface

// request to draw a contacts picture. See MS_AV_DRAWAVATAR service description

typedef struct _avatarDrawRequest
{
	DWORD    cbSize;                 // set this to sizeof(AVATARDRAWREQUEST) - mandatory, service will return failure code if cbSize is wrong
	MCONTACT hContact;               // the contact for which the avatar should be drawn. set it to 0 to draw a protocol picture
	HDC      hTargetDC;              // target device context
	RECT     rcDraw;                 // target rectangle. The avatar will be centered within the rectangle and scaled to fit.
	DWORD    dwFlags;                // flags (see above for valid bitflags)
	DWORD    dwReserved;             // for future use
	DWORD    dwInternal;             // don't use it
	COLORREF clrBorder;              // color for the border  (used with AVDRQ_DRAWBORDER)
	UCHAR    radius;                 // radius (used with AVDRQ_ROUNDEDCORNER)
	UCHAR    alpha;                  // alpha value for semi-transparent avatars (valid values form 1 to 255, if it is set to 0
	                                 // the avatar won't be transparent.
	char    *szProto;                // only used when AVDRQ_PROTOPICT or AVDRQ_OWNPIC is set
}
	AVATARDRAWREQUEST;

#define CACHE_BLOCKSIZE 20

#define AVS_MODULE "AVS_Settings"          // db settings module path
#define PPICT_MODULE "AVS_ProtoPics"   // protocol pictures are saved here

// obtain the bitmap handle of the avatar for the given contact
// wParam = (HANDLE)hContact
// lParam = 0;
// returns: pointer to a struct avatarCacheEntry *, NULL on failure
// if it returns a failure, the avatar may be ready later and the caller may receive
// a notification via ME_AV_AVATARCHANGED
// DONT modify the contents of the returned data structure

#define MS_AV_GETAVATARBITMAP "SV_Avatars/GetAvatar"

// obtain a avatar cache entry for one of my own avatars
// wParam = 0
// lParam = (char *)szProto  (protocol for which we need to obtain the own avatar information). Use "" to global
// returns: pointer to a struct avatarCacheEntry *, NULL on failure
// DONT modify the contents of the returned data structure

#define MS_AV_GETMYAVATAR "SV_Avatars/GetMyAvatar"

// protect the current contact picture from being overwritten by automatic
// avatar updates. Actually, it only backups the contact picture filename
// and will used the backuped version until the contact picture gets unlocked
// again. So this service does not disable avatar updates, but it "fakes"
// a locked contact picture to the users of the GetAvatar service.
//
// wParam = (HANDLE)hContact
// lParam = 1 -> lock the avatar, lParam = 0 -> unlock

#define MS_AV_PROTECTAVATAR "SV_Avatars/ProtectAvatar"

// set (and optionally protect) a local contact picture for the given hContact
//
// wParam = (HANDLE)hContact
// lParam = either a full picture filename or NULL. If lParam == NULL, the service
// will open a file selection dialog.

#define MS_AV_SETAVATAR "SV_Avatars/SetAvatar"
#define MS_AV_SETAVATARW "SV_Avatars/SetAvatarW"
#define MS_AV_SETAVATART MS_AV_SETAVATARW

// set a local picture for the given protocol
//
// wParam = (char *) protocol name or NULL for all protocols
// lParam = either a full picture filename or NULL. If lParam == NULL, the service
// will open a file selection dialog. If lParam == "" the avatar will be removed

#define MS_AV_SETMYAVATAR "SV_Avatars/SetMyAvatar"
#define MS_AV_SETMYAVATARW "SV_Avatars/SetMyAvatarW"
#define MS_AV_SETMYAVATART MS_AV_SETMYAVATARW

// see if is possible to set the avatar for the expecified protocol
//
// wParam = (char *) protocol name
// lParam = 0
// return = 1 if can set, 0 if can't

#define MS_AV_CANSETMYAVATAR "SV_Avatars/CanSetMyAvatar"

// Call avatar option dialog for contact
//
// wParam = (HANDLE)hContact

#define MS_AV_CONTACTOPTIONS "SV_Avatars/ContactOptions"

// draw an avatar picture
//
// wParam = 0 (not used)
// lParam = AVATARDRAWREQUEST *avdr
// draw a contact picture to a destination device context. see description of
// the AVATARDRAWREQUEST structure for more information on how to use this
// service.
// return value: 0 -> failure, avatar probably not available, or not ready. The drawing
// service DOES schedule an avatar update so your plugin will be notified by the ME_AV_AVATARCHANGED
// event when the requested avatar is ready for use.
//				 1 -> success. avatar was found and drawing should be ok.
//				-1 -> global avatar is incosistent

#define MS_AV_DRAWAVATAR "SV_Avatars/Draw"

// fired when a contacts avatar cached by avs changes
// it includes changes made by the user
// wParam = hContact
// lParam = struct avatarCacheEntry *cacheEntry
// the event CAN pass a NULL pointer in lParam which means that the avatar has changed,
// but is no longer valid (happens, when a contact removes his avatar, for example).
// DONT DESTROY the bitmap handle passed in the struct avatarCacheEntry *
//
// It is also possible that this event passes 0 as wParam (hContact), in which case,
// a protocol picture (pseudo - avatar) has been changed.

#define ME_AV_AVATARCHANGED "SV_Avatars/AvatarChanged"

typedef struct _contactAvatarChangedNotification {
	int      cbSize;             // sizeof()
	MCONTACT hContact;           // this might have to be set by the caller too
	int      format;             // PA_FORMAT_*
	TCHAR    filename[MAX_PATH]; // full path to filename which contains the avatar
	TCHAR    hash[128];          // avatar hash
} CONTACTAVATARCHANGEDNOTIFICATION;

// fired when the contacts avatar is changed by the contact
// wParam = hContact
// lParam = struct CONTACTAVATARCHANGEDNOTIFICATION *cacn
// the event CAN pass a NULL pointer in lParam which means that the contact deleted its avatar

#define ME_AV_CONTACTAVATARCHANGED "SV_Avatars/ContactAvatarChanged"

// fired when one of our own avatars was changed
// wParam = (char *)szProto (protocol for which a new avatar was set)
// lParam = AVATARCACHEENTRY *ace (new cache entry, NULL if the new avatar is not valid)

#define ME_AV_MYAVATARCHANGED "SV_Avatars/MyAvatarChanged"

// Service to be called by protocols to report an avatar has changed. Some avatar changes
// can be detected automatically, but some not (by now only Skype ones)
// wParam = (char *)szProto (protocol for which a new avatar was set)
// lParam = 0

#define MS_AV_REPORTMYAVATARCHANGED "SV_Avatars/ReportMyAvatarChanged"

/*
* flags for internal use ONLY
*/

#define AVH_MUSTNOTIFY     0x04             // node->dwFlags (loader thread must notify avatar history about change/delete event)
#define AVS_DELETENODEFOREVER 0x08



// Protocol services //////////////////////////////////////////////////////////////////////

/*
wParam=0
lParam=(const TCHAR*)Avatar file name or NULL to remove the avatar
return=0 for sucess
*/
#define PS_SETMYAVATAR "/SetMyAvatar"

/*
wParam=(TCHAR*)Buffer to file name
lParam=(int)Buffer size
return=0 for sucess
*/
#define PS_GETMYAVATAR "/GetMyAvatar"

#define PIP_NONE    0
#define PIP_SQUARE  1

// Avatar image max size
// lParam = (POINT*) maxSize (use -1 for no max)
// return 0 for success
#define AF_MAXSIZE  1

// Avatar image proportion
// lParam = 0
// return or of PIP_*
#define AF_PROPORTION 2

// Avatar format supported when setting avatars
// lParam = PA_FORMAT_*
// return = 1 (supported) or 0 (not supported)
#define AF_FORMATSUPPORTED 3

// Avatars are enabled for protocol?
// lParam = 0
// return = 1 (avatars ready) or 0 (disabled)
#define AF_ENABLED 4

// This protocol don't need delays for fetching contact avatars
// lParam = 0
// return = 1 (don't need) or 0 (need)
#define AF_DONTNEEDDELAYS 5

// Avatar file max size
// return size in bytes (0 for no limit)
#define AF_MAXFILESIZE 6

// The amount of time avs should wait after a download avatar failed for a contact
// lParam = 0
// return = the time, in ms
#define AF_DELAYAFTERFAIL 7

// Fetching avatars is allowed when protocol's status is invisible
// lParam = 0
// return = 1 (allowed) or 0 (depending on our protocol status mode)
#define AF_FETCHIFPROTONOTVISIBLE 8

// Fetching avatars is allowed when contact is in offline status
// lParam = 0
// return = 1 (allowed) or 0 (depending on remote contact status mode)
#define AF_FETCHIFCONTACTOFFLINE 9


/*
Query avatar caps for a protocol
wParam = One of AF_*
lParam = See descr of each AF_*
return = See descr of each AF_*. Return 0 by default
*/
#define PS_GETAVATARCAPS "/GetAvatarCaps"

#endif
