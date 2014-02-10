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

#define TEMP_WINDOW_CLASS_NAME _T("AntiShittyFullscreenDetectionWindowClass")

#define SENDER_COUNT 10

struct SENDER {
	LPCTSTR name;
	LPCTSTR addr;
};

struct MAIL_THREAD_NOTIFICATION {
	LPCTSTR subj;
	LPCTSTR snip;
	SENDER senders[SENDER_COUNT];
};

void UnreadMailNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount);
void UnreadThreadNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount, const MAIL_THREAD_NOTIFICATION *mtn);
void CloseNotifications(LPCSTR acc, LPCTSTR url, LPCTSTR jid, BOOL PopupsOnly);
DWORD ReadNotificationSettings(LPCSTR acc);
MCONTACT SetupPseudocontact(LPCTSTR jid, LPCTSTR unreadCount, LPCSTR acc, LPCTSTR displayName = NULL);
LPCSTR GetJidAcc(LPCTSTR jid);