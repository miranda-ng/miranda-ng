/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

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
#define TRIGGERNAME		"Variables: String changed"
#define SETTING_TRIGGERTEXT		"trigger_TriggerText"
#define CHECKSTRINGDELAY	5000

typedef struct {
	DWORD triggerID;
	TCHAR *parsedText;
} TRG_VAR_CACHE;


#define SETTING_PARSESTRING		"action_ParseString"
#define SETTING_PARSEASYNC		"action_ParseAsync"

int ParseStringAction(DWORD actionID, REPORTINFO *ri);
INT_PTR CALLBACK DlgProcOptsParseString(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int ParseStringCondition(DWORD actionID, REPORTINFO *ri);
INT_PTR CALLBACK DlgProcOptsCondition(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);