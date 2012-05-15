/*
Scriver

Copyright 2000-2009 Miranda ICQ/IM project,

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

#include "msgs.h"

typedef struct MessageSendQueueItemStruct
{
	HWND	hwndSender;
	HANDLE  hContact;
	char *	proto;
	HANDLE 	hSendId;
	int		timeout;
	char *	sendBuffer;
	int		sendBufferSize;
	int		codepage;
	int		flags;
	HWND	hwndErrorDlg;
	struct MessageSendQueueItemStruct *prev;
	struct MessageSendQueueItemStruct *next;
}MessageSendQueueItem;

void InitSendQueue();
void DestroySendQueue();
MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender);
TCHAR * GetSendBufferMsg(MessageSendQueueItem *item);
MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, HANDLE hContact);
MessageSendQueueItem* FindSendQueueItem(HANDLE hContact, HANDLE hSendId);
BOOL RemoveSendQueueItem(MessageSendQueueItem* item);
void ReportSendQueueTimeouts(HWND hwndSender);
void ReleaseSendQueueItems(HWND hwndSender);
int ReattachSendQueueItems(HWND hwndSender, HANDLE hContact);
void RemoveAllSendQueueItems();
void SendSendQueueItem(MessageSendQueueItem* item);

#endif
