//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#pragma once

#include "resources.h"

static const LPTSTR _T(MAIL_NOTIFICATIONS) = _T("GMail notifications");
static const LPTSTR _T(FULL_NOTIFICATION_FORMAT) = _T("subject\n    %s\nfrom\n%s\n%s\n");
static const LPTSTR _T(SHORT_NOTIFICATION_FORMAT) = _T("subject\n    %s\nfrom\n%s");

static const LPSTR PSEUDOCONTACT_LINK = "GTalkExtNotifyContact";
static const LPSTR PSEUDOCONTACT_FLAG = "IsNotifyContact";
static const LPSTR BACK_COLOR_SETTING = "BackColor";
static const LPSTR TEXT_COLOR_SETTING = "TextColor";
static const LPSTR TIMEOUT_SETTING = "Timeout";

BOOL HookOptionsInitialization();
void UnhookOptionsInitialization();
DWORD ReadCheckboxes(HWND wnd, LPCSTR mod);
BOOL ReadCheckbox(HWND wnd, int id, DWORD controls);