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
	uint8_t Colors;
	uint8_t ShowGroup;
	uint8_t ShowStatus;
	uint8_t UseAlternativeText;
	uint8_t ShowPreviousStatus;
	uint8_t ReadAwayMsg;
	int PopupTimeout;
	UINT PopupConnectionTimeout;
	uint8_t LeftClickAction;
	uint8_t RightClickAction;
	// IDD_OPT_XPOPUP
	uint8_t PXOnConnect;
	uint8_t PXDisableForMusic;
	uint8_t PXMsgTruncate;
	uint32_t PXMsgLen;
	// IDD_OPT_GENERAL
	uint8_t FromOffline;
	uint8_t AutoDisable;
	uint8_t HiddenContactsToo;
	uint8_t UseIndSnd;
	uint8_t BlinkIcon;
	uint8_t BlinkIcon_Status;
	uint8_t BlinkIcon_ForMsgs;
	wchar_t LogFilePath[MAX_PATH];
	// IDD_AUTODISABLE
	uint8_t OnlyGlobalChanges;
	uint8_t DisableSoundGlobally;
	uint8_t DisablePopupGlobally;
	// IDD_OPT_LOG
	uint8_t LogToFile;
	uint8_t LogToDB;
	uint8_t LogToDB_WinOpen;
	uint8_t LogToDB_Remove;
	uint8_t LogPrevious;
	uint8_t SMsgLogToFile;
	uint8_t SMsgLogToDB;
	uint8_t SMsgLogToDB_WinOpen;
	uint8_t SMsgLogToDB_Remove;
	// IDD_OPT_XLOG
	uint8_t XLogToFile;
	uint8_t XLogToDB;
	uint8_t XLogToDB_WinOpen;
	uint8_t XLogToDB_Remove;
	uint8_t XLogDisableForMusic;
	// IDD_OPT_SMPOPUP
	uint8_t PSMsgOnConnect;
	uint8_t PSMsgTruncate;
	uint32_t PSMsgLen;
	// OTHER
	uint8_t TempDisabled;
	uint8_t PopupAutoDisabled;
	uint8_t SoundAutoDisabled;
	uint8_t EnableLastSeen;
} OPTIONS;

typedef struct tagTEMPLATES {
	uint8_t PopupXFlags;
	uint8_t PopupSMsgFlags;
	uint8_t LogXFlags;
	uint8_t LogSMsgFlags;
	wchar_t PopupXstatusChanged[256];
	wchar_t PopupXstatusRemoved[256];
	wchar_t PopupXMsgChanged[256];
	wchar_t PopupXMsgRemoved[256];
	wchar_t LogXstatusChanged[256];
	wchar_t LogXstatusRemoved[256];
	wchar_t LogXMsgChanged[256];
	wchar_t LogXMsgRemoved[256];
	wchar_t LogXstatusOpening[256];
	wchar_t LogSMsgChanged[256];
	wchar_t LogSMsgRemoved[256];
	wchar_t LogSMsgOpening[256];
} TEMPLATES;

int OptionsInitialize(WPARAM wParam, LPARAM lParam);
void LoadOptions();

#endif
