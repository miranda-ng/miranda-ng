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

using namespace SendQueue;

static OBJLIST<Item> g_arQueue(1, PtrKeySortT);
static mir_cs g_csQueue;

Item* SendQueue::CreateItem(CMsgDialog *pDlg)
{
	Item *item = new Item();
	item->pDlg = pDlg;

	mir_cslock lck(g_csQueue);
	g_arQueue.insert(item);
	return item;
}

Item* SendQueue::FindOldestPendingItem(CMsgDialog *pDlg, MCONTACT hContact)
{
	mir_cslock lck(g_csQueue);
	for (auto &it : g_arQueue)
		if (it->pDlg == pDlg && it->hContact == hContact && it->hwndErrorDlg == nullptr)
			return it;

	return nullptr;
}

Item* SendQueue::FindItem(MCONTACT hContact, HANDLE hSendId)
{
	mir_cslock lock(g_csQueue);
	for (auto &it : g_arQueue)
		if (it->hContact == hContact && HANDLE(it->hSendId) == hSendId)
			return it;

	return nullptr;
}

bool SendQueue::RemoveItem(Item *item)
{
	auto *pDlg = item->pDlg;
	{
		mir_cslock lock(g_csQueue);
		g_arQueue.remove(item);
	}

	for (auto &it : g_arQueue)
		if (it->pDlg == pDlg)
			return false;

	return true;
}

void SendQueue::ReportTimeouts(CMsgDialog *pDlg)
{
	int timeout = g_plugin.iMsgTimeout * 1000;

	mir_cslock lock(g_csQueue);

	for (auto &it : g_arQueue.rev_iter()) {
		if (it->timeout >= timeout)
			continue;

		it->timeout += 1000;
		if (it->timeout < timeout || it->pDlg != pDlg || it->hwndErrorDlg != nullptr)
			continue;

		if (pDlg != nullptr) {
			pDlg->StopMessageSending();
			pDlg->ShowError(TranslateT("The message send timed out."), it);
		}
		else g_arQueue.removeItem(&it);
	}
}

void SendQueue::ReleaseItems(CMsgDialog *pDlg)
{
	mir_cslock lock(g_csQueue);

	for (auto &it : g_arQueue) {
		if (it->pDlg != pDlg)
			continue;

		it->pDlg = nullptr;

		if (it->hwndErrorDlg != nullptr)
			DestroyWindow(it->hwndErrorDlg);
		it->hwndErrorDlg = nullptr;
	}
}

int SendQueue::ReattachItems(CMsgDialog *pDlg, MCONTACT hContact)
{
	int count = 0;

	mir_cslock lock(g_csQueue);

	for (auto &it : g_arQueue) {
		if (it->hContact == hContact && it->pDlg == nullptr) {
			it->pDlg = pDlg;
			it->timeout = 0;
			count++;
		}
	}
	return count;
}

void SendQueue::RemoveAllItems()
{
	mir_cslock lock(g_csQueue);
	g_arQueue.destroy();
}

void SendQueue::SendItem(Item *item)
{
	item->timeout = 0;
	item->hSendId = ProtoChainSend(item->hContact, PSS_MESSAGE, item->pDlg->m_hQuoteEvent, (LPARAM)item->sendBuffer);
}
