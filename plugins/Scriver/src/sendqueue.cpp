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

MessageSendQueueItem* CreateSendQueueItem(CMsgDialog *pDlg)
{
	MessageSendQueueItem *item = new MessageSendQueueItem();
	item->pDlg = pDlg;

	mir_cslock lck(queueMutex);
	arQueue.insert(item);
	return item;
}

MessageSendQueueItem* FindOldestPendingSendQueueItem(CMsgDialog *pDlg, MCONTACT hContact)
{
	mir_cslock lck(queueMutex);
	for (auto &it : arQueue)
		if (it->pDlg == pDlg && it->hContact == hContact && it->hwndErrorDlg == nullptr)
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
	auto *pDlg = item->pDlg;
	{
		mir_cslock lock(queueMutex);
		arQueue.remove(item);
	}

	for (auto &it : arQueue)
		if (it->pDlg == pDlg)
			return false;

	return true;
}

void ReportSendQueueTimeouts(CMsgDialog *pDlg)
{
	int timeout = g_plugin.iMsgTimeout * 1000;

	mir_cslock lock(queueMutex);

	for (auto &it : arQueue.rev_iter()) {
		if (it->timeout >= timeout)
			continue;
			
		it->timeout += 1000;
		if (it->timeout < timeout || it->pDlg != pDlg || it->hwndErrorDlg != nullptr)
			continue;

		if (pDlg != nullptr) {
			pDlg->StopMessageSending();
			pDlg->ShowError(TranslateT("The message send timed out."), it);
		}
		else arQueue.removeItem(&it);
	}
}

void ReleaseSendQueueItems(CMsgDialog *pDlg)
{
	mir_cslock lock(queueMutex);

	for (auto &it : arQueue) {
		if (it->pDlg != pDlg)
			continue;

		it->pDlg = nullptr;

		if (it->hwndErrorDlg != nullptr)
			DestroyWindow(it->hwndErrorDlg);
		it->hwndErrorDlg = nullptr;
	}
}

int ReattachSendQueueItems(CMsgDialog *pDlg, MCONTACT hContact)
{
	int count = 0;

	mir_cslock lock(queueMutex);

	for (auto &it : arQueue) {
		if (it->hContact == hContact && it->pDlg == nullptr) {
			it->pDlg = pDlg;
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
