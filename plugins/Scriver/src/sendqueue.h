/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

#ifndef SRMM_SENDQUEUE_H
#define SRMM_SENDQUEUE_H

struct MessageSendQueueItem
{
	HWND	hwndSender;
	MCONTACT hContact;
	char  *proto;
	HANDLE hSendId;
	int    timeout;
	char  *sendBuffer;
	int    sendBufferSize;
	int    codepage;
	int    flags;
	HWND   hwndErrorDlg;
	
	MessageSendQueueItem *prev, *next;
};

MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender);
TCHAR * GetSendBufferMsg(MessageSendQueueItem *item);
MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, MCONTACT hContact);
MessageSendQueueItem* FindSendQueueItem(MCONTACT hContact, HANDLE hSendId);
BOOL RemoveSendQueueItem(MessageSendQueueItem* item);
void ReportSendQueueTimeouts(HWND hwndSender);
void ReleaseSendQueueItems(HWND hwndSender);
int ReattachSendQueueItems(HWND hwndSender, MCONTACT hContact);
void RemoveAllSendQueueItems();
void SendSendQueueItem(MessageSendQueueItem* item);

#endif
