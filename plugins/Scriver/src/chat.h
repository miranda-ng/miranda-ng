/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#ifndef _CHAT_H_
#define _CHAT_H_

#define EM_ACTIVATE (WM_USER+202)

#define TIMERID_MSGSEND    201
#define TIMERID_UNREAD     203
#define TIMEOUT_TYPEOFF  10000  // send type off after 10 seconds of inactivity
#define TIMEOUT_UNREAD     800  // multiple-send bombproofing: send max 3 messages every 4 seconds

#ifndef TVM_GETITEMSTATE
#define TVM_GETITEMSTATE        (TV_FIRST + 39)
#endif

#ifndef TreeView_GetItemState
#define TreeView_GetItemState(hwndTV, hti, mask) \
   (UINT)SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), (LPARAM)(mask))
#endif

#ifndef CFM_BACKCOLOR
#define CFM_BACKCOLOR		0x04000000
#endif

// structs

struct MODULEINFO : public GCModuleInfoBase
{
	HICON hOnlineIcon, hOnlineTalkIcon, hOnlineIconBig;
	HICON hOfflineIcon, hOfflineTalkIcon, hOfflineIconBig;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase
{
	BOOL isFirst;
};

struct SESSION_INFO : public GCSessionInfoBase
{
};

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	HFONT MessageBoxFont;
	bool bAddColonToAutoComplete;

};
extern GlobalLogSettings g_Settings;

// log.c
void LoadMsgLogBitmaps(void);
void FreeMsgLogBitmaps(void);

// manager.c
SESSION_INFO* SM_FindSessionAutoComplete(const char *pszModule, SESSION_INFO *currSession, SESSION_INFO *prevSession, const wchar_t *pszOriginal, const wchar_t *pszCurrent);
char          SM_GetStatusIndicator(SESSION_INFO *si, USERINFO *ui);

/////////////////////////////////////////////////////////////////////////////////////////

#define DEFLOGFILENAME L"%miranda_logpath%\\%proto%\\%userid%.log"
#endif
