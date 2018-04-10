{
  Miranda IM: the free IM client for Microsoft  Windows

  Copyright 2000-2004 Miranda ICQ/IM project,
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
}

{$IFNDEF M_AVATARS}
{$DEFINE M_AVATARS}

const
  AVS_BITMAP_VALID    = 1;
  AVS_BITMAP_EXPIRED  = 2;    // the bitmap has been expired from the cache. (unused, currently)
  AVS_HIDEONCLIST     = 4;
  AVS_PREMULTIPLIED   = 8;    // set in the dwFlags member of the struct avatarCacheEntry
                              // for 32 bit transparent images when loaded with
                              // imgdecoder. These images can be rendered transparently
                              // using the AlphaBlend() API with AC_SRC_ALPHA
  AVS_PROTOPIC        = 16;   // picture is a protocol picture
  AVS_CUSTOMTRANSPBKG = 32;   // Bitmap was changed to set the background color transparent
  AVS_HASTRANSPARENCY = 64;   // Bitmap has at least one pixel transparent
  AVS_OWNAVATAR       = 128;  // is own avatar entry
  AVS_NOTREADY        = 4096;

type
  PavatarCacheEntry = ^TavatarCacheEntry;
  TavatarCacheEntry = record
    hContact    : TMCONTACT; // contacts handle, 0, if it is a protocol avatar
    hbmPic      : HBITMAP;  // bitmap handle of the picture itself
    dwFlags     : dword;    // see above for flag values
    bmHeight    : long;     // bitmap dimensions
    bmWidth     : long;
    t_lastAccess: dword;    // last access time (currently unused, but plugins should still
                            // use it whenever they access the avatar. may be
                            // used in the future to implement cache expiration
    lpDIBSection: pointer;
    szFilename  : array[0..MAX_PATH-1] of WideChar; // filename of the avatar (absolute path)
  end;

  {
    obtain the bitmap handle of the avatar for the given contact
    wParam = hContact
    lParam = 0;
    returns: pointer to a struct avatarCacheEntry *, NULL on failure
    if it returns a failure, the avatar may be ready later and the caller may
    receive a notification via ME_AV_AVATARCHANGED
    DONT modify the contents of the returned data structure
  }

const
  MS_AV_GETAVATARBITMAP:PAnsiChar = 'SV_Avatars/GetAvatar';

{$ENDIF}
