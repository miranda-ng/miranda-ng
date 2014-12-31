/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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
*/
#include "globals.h"

HANDLE hIdleEvent = NULL;
HANDLE hNetlib;
UINT_PTR hIdleTimer = 0;

BOOL starting=1; // this flag will be cleared later

#ifdef UNICODE
bool *protoHasAwayMessageW=0;
bool HasAwayMessageW=0;
#endif

int *isWaitingForRestoreStatusMessage=0;

 int *reqStatus=0;
 int *courStatus=0;
 int *courAwayStatus=0;

 int *protoModes=0;

 short int *awayStatusesPerm=0;
 long *onlyIfBitsPerm=0;
 short int *awayStatuses=0;
 long *onlyIfBits=0;

 int aa_Status[] = {
	ID_STATUS_OFFLINE,
	ID_STATUS_ONLINE,
	ID_STATUS_AWAY,
	ID_STATUS_NA,
	ID_STATUS_OCCUPIED,
	ID_STATUS_DND,
	ID_STATUS_FREECHAT,
	ID_STATUS_INVISIBLE,
	ID_STATUS_ONTHEPHONE,
	ID_STATUS_OUTTOLUNCH
};

 int numStatuses = sizeof(aa_Status)/sizeof(aa_Status[0]);

HWND theDialog = 0;
HWND theDialogReconnect=0;
HWND theDialogIdle=0;
HWND theDialogIdleMessages=0;
HWND theDialogAA=0;


PROTOACCOUNT **accounts=0;
int protoCount=0;
int courProtocolSelection = 0;

int localeID=0;
int codePage=0;


TCHAR *VariableList[][2]={
	{NULL,LPGENT("Date")},
	{_T("Y"),LPGENT("year (4 digits)")},
	{_T("y"),LPGENT("year (2 digits)")},
	{_T("m"),LPGENT("month")},
	{_T("E"),LPGENT("name of month")},
	{_T("e"),LPGENT("short name of month")},
	{_T("d"),LPGENT("day")},
	{_T("W"),LPGENT("weekday (full)")},
	{_T("w"),LPGENT("weekday (abbreviated)")},
	{_T("L"),LPGENT("Localized Date (Long)")},
	{_T("l"),LPGENT("Localized Date (Short)")},
	{NULL,LPGENT("Time")},
	{_T("H"),LPGENT("hours (24)")},
	{_T("h"),LPGENT("hours (12)")},
	{_T("p"),LPGENT("AM/PM")},
	{_T("M"),LPGENT("minutes")},
	{_T("S"),LPGENT("seconds")},
	{_T("K"),LPGENT("Localized Time (Long)")},
	{_T("k"),LPGENT("Localized Time (no Seconds)")},
	{NULL,LPGENT("Time Zone")},
	{_T("z"),LPGENT("total bias")},
	{_T("Z"),LPGENT("time zone name")},
	{NULL,LPGENT("Format")},
	{_T("t"),LPGENT("tabulator")},
	{_T("b"),LPGENT("line break")},
};

HOTKEYDESC hotkeydescs[] = {
	{sizeof(HOTKEYDESC), "saa_benotidle", "Not Idle", SECTIONNAME, AA_IDLE_BENOTIDLESERVICE, 0, 0},
	{sizeof(HOTKEYDESC), "saa_beshortidle", "Short Idle", SECTIONNAME, AA_IDLE_BESHORTIDLESERVICE, 0, 0},
	{sizeof(HOTKEYDESC), "saa_belongidle", "Long Idle", SECTIONNAME, AA_IDLE_BELONGIDLESERVICE, 0, 0},
	{sizeof(HOTKEYDESC), "saa_reconnect", "Reconnect", SECTIONNAME, AA_IDLE_RECONNECTSERVICE, 0, 0},
  };
