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

static OBJLIST<MessageSendQueueItem> arQueue(1, PtrKeySortT);
static mir_cs queueMutex;

MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender)
{
	MessageSendQueueItem *item = new MessageSendQueueItem();
	item->hwndSender = hwndSender;

	mir_cslock lck(queueMutex);
	arQueue.insert(item);
	return item;
}

MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, MCONTACT hContact)
{
	mir_cslock lck(queueMutex);
	for (auto &it : arQueue)
		if (it->hwndSender == hwndSender && it->hContact == hContact && it->hwndErrorDlg == nullptr)
			return it;

	return nullptr;
}

MessageSendQueueItem* FindSendQueueItem(MCONTACT hContact, HANDLE hSendId)
{
	mir_cslock lock(queueMutex);
	for (auto &it : arQueue)
		if (it->hContact == hContact && HANDLE(it->hSendId) == hSendId)
			return it;

	return nullptr;
}

bool RemoveSendQueueItem(MessageSendQueueItem *item)
{
	HWND hwndSender = item->hwndSender;
	{
		mir_cslock lock(queueMutex);
		arQueue.remove(item);
	}

	for (auto &it : arQueue)
		if (it->hwndSender == hwndSender)
			return false;

	return true;
}

void ReportSendQueueTimeouts(HWND hwndSender)
{
	int timeout = g_plugin.getDword(SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);

	mir_cslock lock(queueMutex);

	for (auto &it : arQueue.rev_iter()) {
		if (it->timeout >= timeout)
			continue;
			
		it->timeout += 1000;
		if (it->timeout < timeout || it->hwndSender != hwndSender || it->hwndErrorDlg != nullptr)
			continue;

		if (hwndSender != nullptr) {
			CErrorDlg *pDlg = new CErrorDlg(TranslateT("The message send timed out."), hwndSender, it);
			PostMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)pDlg);
		}
		else arQueue.remove(arQueue.indexOf(&it));
	}
}

void ReleaseSendQueueItems(HWND hwndSender)
{
	mir_cslock lock(queueMutex);

	for (auto &it : arQueue) {
		if (it->hwndSender == hwndSender) {
			it->hwndSender = nullptr;

			if (it->hwndErrorDlg != nullptr)
				DestroyWindow(it->hwndErrorDlg);
			it->hwndErrorDlg = nullptr;
		}
	}
}

int ReattachSendQueueItems(HWND hwndSender, MCONTACT hContact)
{
	int count = 0;

	mir_cslock lock(queueMutex);

	for (auto &it : arQueue) {
		if (it->hContact == hContact && it->hwndSender == nullptr) {
			it->hwndSender = hwndSender;
			it->timeout = 0;
			count++;
		}
	}
	return count;
}

void RemoveAllSendQueueItems()
{
	mir_cslock lock(queueMutex);
	arQueue.destroy();
}

void SendSendQueueItem(MessageSendQueueItem* item)
{
	item->timeout = 0;
	item->hSendId = ProtoChainSend(item->hContact, PSS_MESSAGE, item->flags, (LPARAM)item->sendBuffer);
}
