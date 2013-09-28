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

#define JABBER_FEAT_GTALK_PMUC _T("http://www.google.com/xmpp/protocol/pmuc/v1")

#define MAIL_NOTIFICATIONS        LPGENT("GMail notifications")
#define FULL_NOTIFICATION_FORMAT  LPGENT("subject\n    %s\nfrom\n%s\n%s\n")
#define SHORT_NOTIFICATION_FORMAT LPGENT("subject\n    %s\nfrom\n%s")

#define PSEUDOCONTACT_LINK  "GTalkExtNotifyContact"
#define PSEUDOCONTACT_FLAG  "IsNotifyContact"
#define BACK_COLOR_SETTING  "BackColor"
#define TEXT_COLOR_SETTING  "TextColor"
#define TIMEOUT_SETTING     "Timeout"

DWORD ReadCheckboxes(HWND wnd, LPCSTR mod);
BOOL ReadCheckbox(HWND wnd, int id, DWORD controls);