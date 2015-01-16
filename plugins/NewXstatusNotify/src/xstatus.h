/*
	NewXstatusNotify YM - Plugin for Miranda IM
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

#ifndef XSTATUS_H
#define XSTATUS_H

#define NOTIFY_NEW_XSTATUS		1
#define NOTIFY_REMOVE_XSTATUS	2
#define NOTIFY_NEW_MESSAGE		4
#define NOTIFY_REMOVE_MESSAGE	8
#define NOTIFY_OPENING_ML		16

#define TYPE_ICQ_XSTATUS		1
#define TYPE_JABBER_MOOD		2
#define TYPE_JABBER_ACTIVITY	3

#define XSTATUS_MUSIC			11

#define MAX_TITLE_LEN			256
#define MAX_TEXT_LEN			2048

// Database setting names
#define DB_LASTLOG					"LastLog"

// Sounds
#define XSTATUS_SOUND_CHANGED		"XStatusChanged"
#define XSTATUS_SOUND_MSGCHANGED	"XStatusMsgChanged"
#define XSTATUS_SOUND_REMOVED		"XStatusRemoved"
#define XSTATUS_SOUND_MSGREMOVED	"XStatusMsgRemoved"

// tabSRMM stuff (logging to message window)
#define EVENTTYPE_STATUSCHANGE		25368

// Default templates
#define DEFAULT_POPUP_CHANGED		TranslateT("changed %n to: %t")
#define DEFAULT_POPUP_REMOVED		TranslateT("removed %n")
#define DEFAULT_POPUP_MSGCHANGED	TranslateT("changed %n message to: %m")
#define DEFAULT_POPUP_MSGREMOVED	TranslateT("removed %n message")
#define DEFAULT_POPUP_SMSGCHANGED	TranslateT("changed status message to: %n")
#define DEFAULT_POPUP_SMSGREMOVED	TranslateT("removed status message")

#define DEFAULT_LOG_CHANGED			TranslateT("changed %n @ %t: %m")
#define DEFAULT_LOG_REMOVED			TranslateT("removed %n")
#define DEFAULT_LOG_MSGCHANGED		TranslateT("changed %n message @ %m")
#define DEFAULT_LOG_MSGREMOVED		TranslateT("removed %n message")
#define DEFAULT_LOG_OPENING			TranslateT("has %n @ %t: %m")

#define DEFAULT_LOG_SMSGCHANGED		TranslateT("changed status message @ %n")
#define DEFAULT_LOG_SMSGREMOVED		TranslateT("removed status message")
#define DEFAULT_LOG_SMSGOPENING		TranslateT("is %s and has status message @ %n")

// Variables help text
#define VARIABLES_HELP_TEXT TranslateT("These variables are available:\r\n\r\n\
%n\textra status name (xStatus, Mood, Activity)\r\n\
%t\textra status title\r\n\
%m\textra status message\r\n\
%c\tcustom nickname\r\n\
\\n\tline break\r\n\
\\t\ttab stop")

#define VARIABLES_SM_HELP_TEXT TranslateT("These variables are available:\r\n\r\n\
%n\tnew status message\r\n\
%o\told status message\r\n\
%c\tcustom nickname\r\n\
%s\tcurrent status\r\n\
\\n\tline break\r\n\
\\t\ttab stop")

typedef struct tagXSTATUSCHANGE
{
	MCONTACT hContact;
	char *szProto;
	int type;
	int action;
	TCHAR *stzTitle;
	TCHAR *stzText;
} XSTATUSCHANGE;

typedef struct tagDBEVENT
{
	MCONTACT hContact;
	MEVENT hDBEvent;
} DBEVENT;

typedef struct tagPROTOTEMPLATE
{
	char *ProtoName;
	TCHAR ProtoTemplateMsg[MAX_PATH];
	TCHAR ProtoTemplateRemoved[MAX_PATH];
} PROTOTEMPLATE;

TCHAR *GetDefaultXstatusName(int statusID, char *szProto, TCHAR *buff, int bufflen);
XSTATUSCHANGE *NewXSC(MCONTACT hContact, char *szProto, int xstatusType, int action, TCHAR *stzTitle, TCHAR *stzText);
void ExtraStatusChanged(XSTATUSCHANGE *xsc);
void FreeXSC(XSTATUSCHANGE *xsc);
int OnWindowEvent(WPARAM wParam, LPARAM lParam);

#endif
