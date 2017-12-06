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

#include "stdafx.h"

static MessageSendQueueItem *global_sendQueue = nullptr;
static mir_cs queueMutex;

MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender)
{
	MessageSendQueueItem *item = (MessageSendQueueItem*)mir_calloc(sizeof(MessageSendQueueItem));

	mir_cslock lck(queueMutex);
	item->hwndSender = hwndSender;
	item->next = global_sendQueue;
	if (global_sendQueue != nullptr)
		global_sendQueue->prev = item;

	global_sendQueue = item;
	return item;
}

MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, MCONTACT hContact)
{
	mir_cslock lck(queueMutex);
	for (MessageSendQueueItem *item = global_sendQueue; item != nullptr; item = item->next)
		if (item->hwndSender == hwndSender && item->hContact == hContact && item->hwndErrorDlg == nullptr)
			return item;

	return nullptr;
}

MessageSendQueueItem* FindSendQueueItem(MCONTACT hContact, HANDLE hSendId)
{
	mir_cslock lock(queueMutex);
	for (MessageSendQueueItem *item = global_sendQueue; item != nullptr; item = item->next)
		if (item->hContact == hContact && HANDLE(item->hSendId) == hSendId)
			return item;

	return nullptr;
}

BOOL RemoveSendQueueItem(MessageSendQueueItem* item)
{
	HWND hwndSender = item->hwndSender;

	mir_cslock lock(queueMutex);
	if (item->prev != nullptr)
		item->prev->next = item->next;
	else
		global_sendQueue = item->next;

	if (item->next != nullptr)
		item->next->prev = item->prev;

	mir_free(item->sendBuffer);
	mir_free(item->proto);
	mir_free(item);

	for (item = global_sendQueue; item != nullptr; item = item->next)
		if (item->hwndSender == hwndSender)
			return FALSE;

	return TRUE;
}

void ReportSendQueueTimeouts(HWND hwndSender)
{
	MessageSendQueueItem *item, *item2;
	int timeout = db_get_dw(0, SRMM_MODULE, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	mir_cslock lock(queueMutex);

	for (item = global_sendQueue; item != nullptr; item = item2) {
		item2 = item->next;
		if (item->timeout < timeout) {
			item->timeout += 1000;
			if (item->timeout >= timeout) {
				if (item->hwndSender == hwndSender && item->hwndErrorDlg == nullptr) {
					if (hwndSender != nullptr) {
						CErrorDlg *pDlg = new CErrorDlg(TranslateT("The message send timed out."), hwndSender, item);
						PostMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)pDlg);
					}
					else {
						/* TODO: Handle errors outside messaging window in a better way */
						RemoveSendQueueItem(item);
					}
				}
			}
		}
	}
}

void ReleaseSendQueueItems(HWND hwndSender)
{
	mir_cslock lock(queueMutex);

	for (MessageSendQueueItem *item = global_sendQueue; item != nullptr; item = item->next) {
		if (item->hwndSender == hwndSender) {
			item->hwndSender = nullptr;
			if (item->hwndErrorDlg != nullptr)
				DestroyWindow(item->hwndErrorDlg);

			item->hwndErrorDlg = nullptr;
		}
	}
}

int ReattachSendQueueItems(HWND hwndSender, MCONTACT hContact)
{
	int count = 0;

	mir_cslock lock(queueMutex);

	for (MessageSendQueueItem *item = global_sendQueue; item != nullptr; item = item->next) {
		if (item->hContact == hContact && item->hwndSender == nullptr) {
			item->hwndSender = hwndSender;
			item->timeout = 0;
			count++;
		}
	}
	return count;
}

void RemoveAllSendQueueItems()
{
	MessageSendQueueItem *item, *item2;
	mir_cslock lock(queueMutex);
	for (item = global_sendQueue; item != nullptr; item = item2) {
		item2 = item->next;
		RemoveSendQueueItem(item);
	}
}

void SendSendQueueItem(MessageSendQueueItem* item)
{
	mir_cslockfull lock(queueMutex);
	item->timeout = 0;

	if (item->prev != nullptr) {
		item->prev->next = item->next;
		if (item->next != nullptr)
			item->next->prev = item->prev;

		item->next = global_sendQueue;
		item->prev = nullptr;
		if (global_sendQueue != nullptr)
			global_sendQueue->prev = item;

		global_sendQueue = item;
	}
	lock.unlock();

	item->hSendId = ProtoChainSend(item->hContact, PSS_MESSAGE, item->flags, (LPARAM)item->sendBuffer);
}
