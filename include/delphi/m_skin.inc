{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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
}

{$IFNDEF M_SKIN}
{$DEFINE M_SKIN}

const
  // event icons
  SKINICON_EVENT_MESSAGE = 100;
  SKINICON_EVENT_URL     = 101;
  SKINICON_EVENT_FILE    = 102;

// msgbox icons
  SKINICON_INFORMATION = 150;
  SKINICON_WARNING     = 151;
  SKINICON_ERROR       = 152;
  SKINICON_FATAL       = 153;

  // other icons
  SKINICON_OTHER_MIRANDA       = 200;
  SKINICON_OTHER_EXIT          = 201;
  SKINICON_OTHER_SHOWHIDE      = 202;
  SKINICON_OTHER_GROUPOPEN     = 203;
  SKINICON_OTHER_USERONLINE    = 204;
  SKINICON_OTHER_GROUPSHUT     = 205;
  SKINICON_OTHER_CONNECTING    = 206;
  SKINICON_OTHER_ADDCONTACT    = 207;
  SKINICON_OTHER_USERDETAILS   = 208;
  SKINICON_OTHER_HISTORY       = 209;
  SKINICON_OTHER_DOWNARROW     = 210;
  SKINICON_OTHER_FINDUSER      = 211;
  SKINICON_OTHER_OPTIONS       = 212;
  SKINICON_OTHER_SENDEMAIL     = 213;
  SKINICON_OTHER_DELETE        = 214;
  SKINICON_OTHER_RENAME        = 215;
  SKINICON_OTHER_SMS           = 216;
  SKINICON_OTHER_SEARCHALL     = 217;
  SKINICON_OTHER_TICK          = 218;
  SKINICON_OTHER_NOTICK        = 219;
  SKINICON_OTHER_HELP          = 220;
  SKINICON_OTHER_MIRANDAWEB    = 221;
  SKINICON_OTHER_TYPING        = 222;
  SKINICON_OTHER_SMALLDOT      = 223;
  SKINICON_OTHER_FILLEDBLOB    = 224;
  SKINICON_OTHER_EMPTYBLOB     = 225;
  SKINICON_OTHER_UNICODE       = 226;
  SKINICON_OTHER_ANSI          = 227;
  SKINICON_OTHER_LOADED        = 228;
  SKINICON_OTHER_NOTLOADED     = 229;
  SKINICON_OTHER_UNDO          = 230;
  SKINICON_OTHER_WINDOW        = 231;
  SKINICON_OTHER_WINDOWS       = 232;
  SKINICON_OTHER_ACCMGR        = 233;
  SKINICON_OTHER_MAINMENU      = 234;
  SKINICON_OTHER_STATUS        = 235;
  SKINICON_CHAT_JOIN           = 236;
  SKINICON_CHAT_LEAVE          = 237;
  SKINICON_OTHER_STATUS_LOCKED = 238;
  SKINICON_OTHER_GROUP         = 239;
  SKINICON_OTHER_ON            = 240;
  SKINICON_OTHER_OFF           = 241;
  SKINICON_OTHER_LOADEDGRAY    = 242;
  SKINICON_OTHER_NOTLOADEDGRAY = 243;
  SKINICON_OTHER_VISIBLE_ALL   = 244;
  SKINICON_OTHER_INVISIBLE_ALL = 245;
  SKINICON_OTHER_FRAME         = 246;

  SKINICON_AUTH_ADD            = 247;
  SKINICON_AUTH_REQUEST        = 248;
  SKINICON_AUTH_GRANT          = 249;
  SKINICON_AUTH_REVOKE         = 250;
{
  SKINICON_EVENT_MESSAGE_BIG     = 300;
  SKINICON_EVENT_URL_BIG         = 301;
  SKINICON_EVENT_FILE_BIG        = 302;
  SKINICON_OTHER_MIRANDA_BIG     = 303;
  SKINICON_OTHER_ADDCONTACT_BIG  = 304;
  SKINICON_OTHER_USERDETAILS_BIG = 305;
  SKINICON_OTHER_HISTORY_BIG     = 306;
  SKINICON_OTHER_FINDUSER_BIG    = 307;
  SKINICON_OTHER_OPTIONS_BIG     = 308;
  SKINICON_OTHER_ACCMGR_BIG      = 309;
}
  // menu icons are owned by the module that uses them so are not and should
  // not be skinnable. Except exit and show/hide

  // status mode icons. NOTE: These are deprecated in favour of LoadSkinnedProtoIcon()
  SKINICON_STATUS_OFFLINE    = 0;
  SKINICON_STATUS_ONLINE     = 1;
  SKINICON_STATUS_AWAY       = 2;
  SKINICON_STATUS_NA         = 3;
  SKINICON_STATUS_OCCUPIED   = 4;
  SKINICON_STATUS_DND        = 5;
  SKINICON_STATUS_FREE4CHAT  = 6;
  SKINICON_STATUS_INVISIBLE  = 7;
  SKINICON_STATUS_ONTHEPHONE = 8;
  SKINICON_STATUS_OUTTOLUNCH = 9;

const
  SSDF_UNICODE  = $0001;
type
  PSKINSOUNDDESCEX = ^TSKINSOUNDDESCEX;
  TSKINSOUNDDESCEX = record
    cbSize        : int;
    pszName       : PAnsiChar; // name to refer to sound when playing and in DB
    pszDescription: TChar; // [TRANSLATED-BY-CORE] description to use for it in options dialog
    pszDefaultFile: TChar; // the default sound file to use, WITHOUT path
    pszSection    : TChar; // [TRANSLATED-BY-CORE] section name used to group sounds (NULL is acceptable)
    dwFlags       : dword; // if Miranda ver >=9.0
  end;

const
  {
    wParam : ICON_ID
    lParam : 0 - return HICON
             1 - return icolib handle
             2 - return big icon HICON
             3 - return icolib icon name
    Affect : Load an icon from the user's custom skin lib, or from the exe
             if there isn't one loaded, see notes
    Return : HICON for the new icon, do *not* DestroyIcon() the return value
             returns NULL(0) if ICON_ID is invalid, but always success for a valid ID.
  }
  MS_SKIN_LOADICON:PAnsiChar = 'Skin/Icons/Load';

  {
    wParam : null terminated string containing the protocol name
    lParam : status_wanted
    Affect : Load an icon representing the status_wanted for a particular protocol, see notes
    Returns: an HICON for the new icon, do NOT DestroyIcon() the return value
             returns NULL(0) on failure.
    Notes  : If wParam is NULL(0) the service will load the user's selected
             'all protocols' status icon
  }
  MS_SKIN_LOADPROTOICON   :PAnsiChar = 'Skin/Icons/LoadProto';
  MS_SKIN_LOADPROTOICONBIG:PAnsiChar = 'Skin/Icons/LoadProtoBig';

  {
    wParam : hLangpack
    lParam : Pointer to a initialised SKINSOUNDDESC
    Affect : Add a new sound so it has a default and can be changed in the
             options dialog
    Returns: 0 on success, [non zero] on failure
  }
  MS_SKIN_ADDNEWSOUND:PAnsiChar = 'Skin/Sounds/AddNew';

  {
    wParam : 0
    lParam : Pointer to a null terminated string containing the name of the
             sound to play
    Affect : plays a named sound event, play name should of been added
             with MS_SKIN_ADDNEWSOUND,  see notes
    Notes  : function will not fail, it will play the Windows
  }
  MS_SKIN_PLAYSOUND:PAnsiChar = 'Skin/Sounds/Play';

  {
    wParam = 0
    lParam = (WideChar *) ptszFileName
    Affect: plays any sound file
  }
  MS_SKIN_PLAYSOUNDFILE:PAnsiChar = 'Skin/Sounds/PlayFile';

  {
    wParam : 0
    lParam : 0
    Affect : Sent when the icons DLL has been changed in the options dialog
             and everyone should remake their image lists.
  }
  ME_SKIN_ICONSCHANGED:PAnsiChar = 'Skin/IconsChanged';

  {
    wParam: 0 when playing sound (1 when sound is being previewed)
    lParam: (AnsiChar*) pszSoundFile
    Affect: This hook is fired when the sound module needs to play a sound
    Note  : This event has default processing, if no one HookEvent()'s this
        event then it will use the default hook code, which uses PlaySound()
    Version: 0.3.4a (2004/09/15)
  }
  ME_SKIN_PLAYINGSOUND:PAnsiChar = 'Skin/Sounds/Playing';

{$ENDIF}
