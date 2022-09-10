// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
// Copyright (c) 2000-08 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifndef M_SKIN_H__
#define M_SKIN_H__ 1

#ifndef M_CORE_H__
#include <m_core.h>
#endif

// event icons
#define SKINICON_EVENT_MESSAGE       100
#define SKINICON_EVENT_URL           101
#define SKINICON_EVENT_FILE          102

// msgbox icons
#define SKINICON_INFORMATION         150
#define SKINICON_WARNING             151
#define SKINICON_ERROR               152
#define SKINICON_FATAL               153

// other icons
#define SKINICON_OTHER_MIRANDA       200
#define SKINICON_OTHER_EXIT          201
#define SKINICON_OTHER_SHOWHIDE      202
#define SKINICON_OTHER_GROUPOPEN     203
#define SKINICON_OTHER_USERONLINE    204
#define SKINICON_OTHER_GROUPSHUT     205
#define SKINICON_OTHER_CONNECTING    206
#define SKINICON_OTHER_ADDCONTACT    207
#define SKINICON_OTHER_USERDETAILS   208
#define SKINICON_OTHER_HISTORY       209
#define SKINICON_OTHER_DOWNARROW     210
#define SKINICON_OTHER_FINDUSER      211
#define SKINICON_OTHER_OPTIONS       212
#define SKINICON_OTHER_SENDEMAIL     213
#define SKINICON_OTHER_DELETE        214
#define SKINICON_OTHER_RENAME        215
#define SKINICON_OTHER_SMS           216
#define SKINICON_OTHER_SEARCHALL     217
#define SKINICON_OTHER_TICK          218
#define SKINICON_OTHER_NOTICK        219
#define SKINICON_OTHER_HELP          220
#define SKINICON_OTHER_MIRANDAWEB    221
#define SKINICON_OTHER_TYPING        222
#define SKINICON_OTHER_SMALLDOT      223
#define SKINICON_OTHER_FILLEDBLOB    224
#define SKINICON_OTHER_EMPTYBLOB     225
#define SKINICON_OTHER_UNICODE       226
#define SKINICON_OTHER_ANSI          227
#define SKINICON_OTHER_UNDO          228
#define SKINICON_OTHER_WINDOW        229
#define SKINICON_OTHER_WINDOWS       230
#define SKINICON_OTHER_ACCMGR        231
#define SKINICON_OTHER_MAINMENU      232
#define SKINICON_OTHER_STATUS        233
#define SKINICON_CHAT_JOIN           234
#define SKINICON_CHAT_LEAVE          235
#define SKINICON_OTHER_STATUS_LOCKED 236
#define SKINICON_OTHER_GROUP         237
#define SKINICON_OTHER_ON            238
#define SKINICON_OTHER_OFF           239
#define SKINICON_OTHER_VISIBLE_ALL   240
#define SKINICON_OTHER_INVISIBLE_ALL 241
#define SKINICON_OTHER_FRAME         242
#define SKINICON_AUTH_ADD            243
#define SKINICON_AUTH_REQUEST        244
#define SKINICON_AUTH_GRANT          245
#define SKINICON_AUTH_REVOKE         246
#define SKINICON_OTHER_GROUPADD      247
#define SKINICON_OTHER_POPUP         248
#define SKINICON_OTHER_NOPOPUP       249
#define SKINICON_OTHER_KEYS          250
#define SKINICON_OTHER_SOUND         251
#define SKINICON_OTHER_EDIT          252        

/////////////////////////////////////////////////////////////////////////////////////////
// Miranda skin
// in all these functions idx = SKINICON_* constant

EXTERN_C MIR_APP_DLL(HICON)  Skin_LoadIcon(int idx, bool big = false);
EXTERN_C MIR_APP_DLL(HANDLE) Skin_GetIconHandle(int idx);

// status mode icons. NOTE: These are deprecated in favour of LoadSkinProtoIcon()
#define SKINICON_STATUS_OFFLINE     0
#define SKINICON_STATUS_ONLINE      1
#define SKINICON_STATUS_AWAY        2
#define SKINICON_STATUS_NA          3
#define SKINICON_STATUS_OCCUPIED    4
#define SKINICON_STATUS_DND         5
#define SKINICON_STATUS_FREE4CHAT   6
#define SKINICON_STATUS_INVISIBLE   7

/////////////////////////////////////////////////////////////////////////////////////////
// Loads an icon representing the status mode for a particular protocol.
// returns an hIcon for the new icon. Do *not* DestroyIcon() the return value
// returns NULL on failure
// if szProto is NULL the function will load the user's selected 'all protocols'
// status icon.

EXTERN_C MIR_APP_DLL(HICON) Skin_LoadProtoIcon(const char *szProto, int status, bool big = false);
EXTERN_C MIR_APP_DLL(HANDLE) Skin_GetProtoIcon(const char *szProto, int status);

/////////////////////////////////////////////////////////////////////////////////////////
// plays a registered sound
// returns 0 on success, nonzero otherwise

EXTERN_C MIR_APP_DLL(int) Skin_PlaySound(const char *name);

/////////////////////////////////////////////////////////////////////////////////////////
// plays the sound file (non-registered)
// returns 0 on success, nonzero otherwise

EXTERN_C MIR_APP_DLL(int) Skin_PlaySoundFile(const wchar_t *pwszFileName);

/////////////////////////////////////////////////////////////////////////////////////////
// wParam: 0 when playing sound (1 when sound is being previewed)
// lParam: (wchar_t*) pszSoundFile
// Affect: This hook is fired when the sound module needs to play a sound
// Note  : This event has default processing, if no one HookEvent()'s this event then it will
//         use the default hook code, which uses PlaySound()

#define ME_SKIN_PLAYINGSOUND "Skin/Sounds/Playing"

#endif //M_SKIN_H__
