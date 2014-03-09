/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#define DEFAULT_LOGFILE	"NewStatusNotify.log"

typedef struct tagOPTIONS {
	// IDD_OPT_POPUP
	BYTE Colors;
	BYTE ShowGroup;
	BYTE ShowStatus;
	BYTE UseAlternativeText;
	BYTE ShowPreviousStatus;
	BYTE ReadAwayMsg;
	int PopupTimeout;
	UINT PopupConnectionTimeout;
	BYTE LeftClickAction;
	BYTE RightClickAction;
	// IDD_OPT_XPOPUP
	BYTE PDisableForMusic;
	BYTE PTruncateMsg;
	DWORD PMsgLen;
	// IDD_OPT_GENERAL
	BYTE FromOffline;
	BYTE AutoDisable;
	BYTE HiddenContactsToo;
	BYTE UseIndSnd;
	BYTE BlinkIcon;
	BYTE BlinkIcon_Status;
	BYTE Log;
	BYTE LogToDB, CheckMessageWindow;
	TCHAR LogFilePath[MAX_PATH];
	// IDD_AUTODISABLE
	BYTE OnlyGlobalChanges;
	BYTE DisableSoundGlobally;
	BYTE DisablePopupGlobally;
	// IDD_OPT_XLOG
	BYTE EnableLogging;
	BYTE PreventIdentical;
	BYTE KeepInHistory;
	BYTE LDisableForMusic;
	BYTE LTruncateMsg;
	DWORD LMsgLen;
	//IDD_OPT_SMPOPUP
	BYTE PopupOnConnect;
	BYTE IgnoreEmpty;
	// OTHER
	BYTE TempDisabled;
	BYTE PopupAutoDisabled;
	BYTE SoundAutoDisabled;
	BYTE EnableLastSeen;
} OPTIONS;

typedef struct tagTEMPLATES {
	BYTE PopupFlags;
	BYTE LogFlags;
	TCHAR PopupDelimiter[32];
	TCHAR PopupNewXstatus[256];
	TCHAR PopupNewMsg[256];
	TCHAR PopupRemove[256];
	TCHAR LogDelimiter[32];
	TCHAR LogNewXstatus[256];
	TCHAR LogNewMsg[256];
	TCHAR LogRemove[256];
	TCHAR LogOpening[256];
} TEMPLATES;

int OptionsInitialize(WPARAM wParam, LPARAM lParam); 
void LoadOptions(); 

#endif