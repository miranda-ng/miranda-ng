// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-15 Miranda NG project (http://miranda-ng.org)
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

extern int hLangpack;

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
#define SKINICON_OTHER_LOADED        228
#define SKINICON_OTHER_NOTLOADED     229
#define SKINICON_OTHER_UNDO          230
#define SKINICON_OTHER_WINDOW        231
#define SKINICON_OTHER_WINDOWS       232
#define SKINICON_OTHER_ACCMGR        233
#define SKINICON_OTHER_MAINMENU      234
#define SKINICON_OTHER_STATUS        235
#define SKINICON_CHAT_JOIN           236
#define SKINICON_CHAT_LEAVE          237
#define SKINICON_OTHER_STATUS_LOCKED 238
#define SKINICON_OTHER_GROUP         239
#define SKINICON_OTHER_ON            240
#define SKINICON_OTHER_OFF           241
#define SKINICON_OTHER_LOADEDGRAY    242
#define SKINICON_OTHER_NOTLOADEDGRAY 243
#define SKINICON_OTHER_VISIBLE_ALL   244
#define SKINICON_OTHER_INVISIBLE_ALL 245
#define SKINICON_OTHER_FRAME         246
#define SKINICON_AUTH_ADD            247
#define SKINICON_AUTH_REQUEST        248
#define SKINICON_AUTH_GRANT          249
#define SKINICON_AUTH_REVOKE         250
#define SKINICON_OTHER_GROUPADD      251

/////////////////////////////////////////////////////////////////////////////////////////
// Miranda skin
// in all these functions idx = SKINICON_* constant
EXTERN_C MIR_APP_DLL(HICON)  Skin_LoadIcon(int idx, bool big = false);
EXTERN_C MIR_APP_DLL(HANDLE) Skin_GetIconHandle(int idx);
EXTERN_C MIR_APP_DLL(char*)  Skin_GetIconName(int idx);

// status mode icons. NOTE: These are deprecated in favour of LoadSkinProtoIcon()
#define SKINICON_STATUS_OFFLINE     0
#define SKINICON_STATUS_ONLINE      1
#define SKINICON_STATUS_AWAY        2
#define SKINICON_STATUS_NA          3
#define SKINICON_STATUS_OCCUPIED    4
#define SKINICON_STATUS_DND         5
#define SKINICON_STATUS_FREE4CHAT   6
#define SKINICON_STATUS_INVISIBLE   7
#define SKINICON_STATUS_ONTHEPHONE  8
#define SKINICON_STATUS_OUTTOLUNCH  9

/////////////////////////////////////////////////////////////////////////////////////////
// Loads an icon representing the status mode for a particular protocol.
// returns an hIcon for the new icon. Do *not* DestroyIcon() the return value
// returns NULL on failure
// if szProto is NULL the function will load the user's selected 'all protocols'
// status icon.
EXTERN_C MIR_APP_DLL(HICON) Skin_LoadProtoIcon(const char *szProto, int status, bool big = false);

/////////////////////////////////////////////////////////////////////////////////////////
// add a new sound so it has a default and can be changed in the options dialog
// wParam = hLangpack
// lParam = (LPARAM)(SKINSOUNDDESC*)ssd;
// returns 0 on success, nonzero otherwise

#define SSDF_UNICODE 0x0001

#if defined(_UNICODE)
	#define SSDF_TCHAR  SSDF_UNICODE
#else
	#define SSDF_TCHAR  0
#endif

typedef struct {
	int cbSize;
	const char *pszName;           // name to refer to sound when playing and in db
	union {
		const char *pszDescription;    // [TRANSLATED-BY-CORE] description for options dialog
		const TCHAR *ptszDescription;
	};
	union {
		const char *pszDefaultFile;    // default sound file to use
		const TCHAR *ptszDefaultFile;
	};
	union {
		const char *pszSection;        // [TRANSLATED-BY-CORE] section name used to group sounds (NULL is acceptable)
		const TCHAR *ptszSection;
	};
	DWORD dwFlags;
}
	SKINSOUNDDESCEX;

__forceinline INT_PTR SkinAddNewSoundEx(const char *name, const char *section, const char *description = NULL, const char *defaultFile = NULL)
{
	SKINSOUNDDESCEX ssd = { 0 };
	ssd.cbSize = sizeof(ssd);
	ssd.pszName = name;
	ssd.pszSection = section;
	ssd.pszDescription = description;
	ssd.pszDefaultFile = defaultFile;
	return CallService("Skin/Sounds/AddNew", hLangpack, (LPARAM)&ssd);
}

__forceinline INT_PTR SkinAddNewSoundExT(const char *name, const TCHAR *section, const TCHAR *description = NULL, const TCHAR *defaultFile = NULL)
{
	SKINSOUNDDESCEX ssd = { 0 };
	ssd.cbSize = sizeof(ssd);
	ssd.dwFlags = SSDF_TCHAR;
	ssd.pszName = name;
	ssd.ptszSection = section;
	ssd.ptszDescription = description;
	ssd.ptszDefaultFile = defaultFile;
	return CallService("Skin/Sounds/AddNew", hLangpack, (LPARAM)&ssd);
}

__forceinline INT_PTR Skin_AddSound(SKINSOUNDDESCEX *ssd)
{
	return CallService("Skin/Sounds/AddNew", hLangpack, (LPARAM)ssd);
}

#define MS_SKIN_PLAYSOUND        "Skin/Sounds/Play"

/////////////////////////////////////////////////////////////////////////////////////////
// plays a named sound event
// wParam = 0
// lParam = (LPARAM)(const char*)pszName
// pszName should have been added with Skin/Sounds/AddNew, but if not the
// function will not fail, it will play the Windows default sound instead.
__forceinline INT_PTR SkinPlaySound(const char *name) {
	return CallService(MS_SKIN_PLAYSOUND, 0, (LPARAM)name);
}

#define MS_SKIN_PLAYSOUNDFILE        "Skin/Sounds/PlayFile"

/////////////////////////////////////////////////////////////////////////////////////////
// plays any sound file
// wParam = 0
// lParam = (LPARAM)(const TCHAR*)ptszFileName
__forceinline INT_PTR SkinPlaySoundFile(const TCHAR *ptszFileName) {
	return CallService(MS_SKIN_PLAYSOUNDFILE, 0, (LPARAM)ptszFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(void) KillModuleSounds(int hLangpack);

/////////////////////////////////////////////////////////////////////////////////////////
// sent when the icons DLL has been changed in the options dialog, and everyone
// should re-make their image lists
// wParam = lParam = 0
#define ME_SKIN_ICONSCHANGED     "Skin/IconsChanged"

/////////////////////////////////////////////////////////////////////////////////////////
// wParam: 0 when playing sound (1 when sound is being previewed)
// lParam: (TCHAR*) pszSoundFile
// Affect: This hook is fired when the sound module needs to play a sound
// Note  : This event has default processing, if no one HookEvent()'s this event then it will
//         use the default hook code, which uses PlaySound()
#define ME_SKIN_PLAYINGSOUND  "Skin/Sounds/Playing"

#endif //M_SKIN_H__
