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

#include "commonheaders.h"

static MessageSendQueueItem *global_sendQueue = NULL;
static mir_cs queueMutex;

TCHAR * GetSendBufferMsg(MessageSendQueueItem *item)
{
	TCHAR *szMsg = NULL;
	size_t len = strlen(item->sendBuffer);

	if (item->flags & PREF_UTF)
		szMsg = mir_utf8decodeW(item->sendBuffer);
	else {
		szMsg = (TCHAR *)mir_alloc(item->sendBufferSize - len - 1);
		memcpy(szMsg, item->sendBuffer + len + 1, item->sendBufferSize - len - 1);
	}

	return szMsg;
}

MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender)
{
	MessageSendQueueItem *item = (MessageSendQueueItem*)mir_calloc(sizeof(MessageSendQueueItem));

	mir_cslock lck(queueMutex);
	item->hwndSender = hwndSender;
	item->next = global_sendQueue;
	if (global_sendQueue != NULL)
		global_sendQueue->prev = item;

	global_sendQueue = item;
	return item;
}

MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, MCONTACT hContact)
{
	mir_cslock lck(queueMutex);
	for (MessageSendQueueItem *item = global_sendQueue; item != NULL; item = item->next)
		if (item->hwndSender == hwndSender && item->hContact == hContact && item->hwndErrorDlg == NULL)
			return item;

	return NULL;
}

MessageSendQueueItem* FindSendQueueItem(MCONTACT hContact, HANDLE hSendId)
{
	mir_cslock lock(queueMutex);
	for (MessageSendQueueItem *item = global_sendQueue; item != NULL; item = item->next)
		if (item->hContact == hContact && item->hSendId == hSendId)
			return item;

	return NULL;
}

BOOL RemoveSendQueueItem(MessageSendQueueItem* item)
{
	HWND hwndSender = item->hwndSender;

	mir_cslock lock(queueMutex);
	if (item->prev != NULL)
		item->prev->next = item->next;
	else
		global_sendQueue = item->next;

	if (item->next != NULL)
		item->next->prev = item->prev;
	
	mir_free(item->sendBuffer);
	mir_free(item->proto);
	mir_free(item);

	for (item = global_sendQueue; item != NULL; item = item->next)
		if (item->hwndSender == hwndSender)
			return FALSE;

	return TRUE;
}

void ReportSendQueueTimeouts(HWND hwndSender)
{
	MessageSendQueueItem *item, *item2;
	int timeout = db_get_dw(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	mir_cslock lock(queueMutex);

	for (item = global_sendQueue; item != NULL; item = item2) {
		item2 = item->next;
		if (item->timeout < timeout) {
			item->timeout += 1000;
			if (item->timeout >= timeout) {
				if (item->hwndSender == hwndSender && item->hwndErrorDlg == NULL) {
					if (hwndSender != NULL) {
						ErrorWindowData *ewd = (ErrorWindowData *) mir_alloc(sizeof(ErrorWindowData));
						ewd->szName = GetNickname(item->hContact, item->proto);
						ewd->szDescription = mir_tstrdup(TranslateT("The message send timed out."));
						ewd->szText = GetSendBufferMsg(item);
						ewd->hwndParent = hwndSender;
						ewd->queueItem = item;
						PostMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)ewd);
					} else {
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

	for (MessageSendQueueItem *item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hwndSender == hwndSender) {
			item->hwndSender = NULL;
			if (item->hwndErrorDlg != NULL)
				DestroyWindow(item->hwndErrorDlg);

			item->hwndErrorDlg = NULL;
		}
	}
}

int ReattachSendQueueItems(HWND hwndSender, MCONTACT hContact)
{
	int count = 0;

	mir_cslock lock(queueMutex);

	for (MessageSendQueueItem *item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hContact == hContact && item->hwndSender == NULL) {
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
	for (item = global_sendQueue; item != NULL; item = item2) {
		item2 = item->next;
		RemoveSendQueueItem(item);
	}
}

void SendSendQueueItem(MessageSendQueueItem* item)
{
	mir_cslockfull lock(queueMutex);
	item->timeout = 0;

	if (item->prev != NULL) {
		item->prev->next = item->next;
		if (item->next != NULL)
			item->next->prev = item->prev;

		item->next = global_sendQueue;
		item->prev = NULL;
		if (global_sendQueue != NULL)
			global_sendQueue->prev = item;

		global_sendQueue = item;
	}
	lock.unlock();

	item->hSendId = (HANDLE)CallContactService(item->hContact, PSS_MESSAGE, item->flags, (LPARAM)item->sendBuffer);
}
