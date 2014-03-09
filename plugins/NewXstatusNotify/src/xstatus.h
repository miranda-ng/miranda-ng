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
#define NOTIFY_NEW_MESSAGE		2
#define NOTIFY_REMOVE			4
#define NOTIFY_OPENING_ML		8

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
#define XSTATUS_SOUND_REMOVED		"XStatusRemove"

// tabSRMM stuff (logging to message window)
#define EVENTTYPE_STATUSCHANGE			25368

// Default templates
#define DEFAULT_POPUP_DELIMITER		_T("%B")
#define DEFAULT_POPUP_NEW			TranslateT("changed %N to: %T%D%I") 
#define DEFAULT_POPUP_CHANGEMSG		TranslateT("changed %N message to:%D%I") 
#define DEFAULT_POPUP_REMOVE		TranslateT("removed %N") 
#define DEFAULT_POPUP_STATUSMESSAGE	LPGEN("changed his/her status message to %n")

#define DEFAULT_LOG_DELIMITER		_T(": ")
#define DEFAULT_LOG_NEW				TranslateT("changed %N @ %T%D%I") 
#define DEFAULT_LOG_CHANGEMSG		TranslateT("changed %N message @ %I")
#define DEFAULT_LOG_REMOVE			TranslateT("removed %N") 
#define DEFAULT_LOG_OPENING			TranslateT("has %N @ %T%D%I")

// Variables help text
#define VARIABLES_HELP_TEXT TranslateT("These variables are available:\r\n\r\n\
%N\textra status name (Xstatus, Mood, Activity)\r\n\
%T\textra status title\r\n\
%I\textra status text\r\n\
%D\tdelimiter\r\n\
%B\tline break (can be used as delimiter)")

#define VARIABLES_SM_HELP_TEXT TranslateT("These variables are available:\r\n\r\n\
%n\tNew Status Message\r\n\
%o\tOld Status Message\r\n\
%c\tCustom Nickname\r\n\
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
	HANDLE hDBEvent;
} DBEVENT;

typedef struct tagPROTOTEMPLATE
{
	TCHAR *ProtoName;
	TCHAR ProtoTemplate[MAX_PATH];
} PROTOTEMPLATE;

TCHAR *GetDefaultXstatusName(int statusID, char *szProto, TCHAR *buff, int bufflen);
XSTATUSCHANGE *NewXSC(MCONTACT hContact, char *szProto, int xstatusType, int action, TCHAR *stzTitle, TCHAR *stzText);
void ExtraStatusChanged(XSTATUSCHANGE *xsc);
void FreeXSC(XSTATUSCHANGE *xsc);
int  OnWindowEvent(WPARAM wParam, LPARAM lParam);

#endif