{
Miranda FlashAvatars Plugin
Plugin support header file
Copyright (C) 2006 Big Muscle

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
}

{$IFNDEF M_FLASH}
{$DEFINE M_FLASH}
// Service functions

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
}
  MS_FAVATAR_DESTROY = 'FlashAvatar/Destroy';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
}
  MS_FAVATAR_MAKE = 'FlashAvatar/Make';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM LPRECT
}
  MS_FAVATAR_RESIZE = 'FlashAvatar/Resize';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM LPRECT
}
  MS_FAVATAR_SETPOS = 'FlashAvatar/SetPos';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
}
  MS_FAVATAR_GETINFO = 'FlashAvatar/GetInfo';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM BSTR
}
  MS_FAVATAR_SETEMOFACE = 'FlashAvatar/SetEmoFace';

{
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM COLORREF
}
  MS_FAVATAR_SETBKCOLOR	 = 'FlashAvatar/SetBkColor';

// Avatar emotion faces
  AV_SMILE   = 'smile';
  AV_SAD     = 'sad';
  AV_LAUGH   = 'laugh';
  AV_MAD     = 'mad';
  AV_CRY     = 'cry';
  AV_OFFLINE = 'offline';
  AV_BUSY    = 'busy';
  AV_LOVE    = 'love';
  AV_NORMAL  = 'stam';

// Avatar default size
  FAVATAR_WIDTH  = 52;
  FAVATAR_HEIGHT = 64;

type
  TFLASHAVATAR = record
    hContact     :TMCONTACT; // contact who flash avatar belongs to
    hWindow      :HWND;      // handle of flash avatar object
    hParentWindow:HWND;      // handle of flash avatar's parent object
    cUrl         :TChar;     // url of .swf file
    id           :int;       // unique number of plugin which wants to use avatar service
    cProto       :PAnsiChar; // contacts protocol
  end;

{$ENDIF}
