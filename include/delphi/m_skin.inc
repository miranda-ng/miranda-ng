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

function Skin_LoadIcon(iconId:int; big:byte) : HICON; stdcall; external AppDLL;

function Skin_GetIconHandle(iconId:int) : THANDLE; stdcall; external AppDLL;

// status mode icons. NOTE: These are deprecated in favour of LoadSkinnedProtoIcon()
const
  SKINICON_STATUS_OFFLINE    = 0;
  SKINICON_STATUS_ONLINE     = 1;
  SKINICON_STATUS_AWAY       = 2;
  SKINICON_STATUS_NA         = 3;
  SKINICON_STATUS_OCCUPIED   = 4;
  SKINICON_STATUS_DND        = 5;
  SKINICON_STATUS_FREE4CHAT  = 6;
  SKINICON_STATUS_INVISIBLE  = 7;

function Skin_LoadProtoIcon(protoName:PAnsiChar; iconId:int; big:byte) : HICON; stdcall; external AppDLL;

{$ENDIF}
