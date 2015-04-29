// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Rate Management stuff
// -----------------------------------------------------------------------------

#include "stdafx.h"

//
// Rate Level 1 Management
/////////////////////////////

rates::rates(CIcqProto *ppro, BYTE *pBuffer, size_t wLen)
{
	nGroups = 0;
	memset(&groups, 0, MAX_RATES_GROUP_COUNT * sizeof(rates_group));
	this->ppro = ppro;

	// Parse Rate Data Block
	WORD wCount;
	unpackWord(&pBuffer, &wCount);
	wLen -= 2;

	if (wCount > MAX_RATES_GROUP_COUNT) {
		// just sanity check
		ppro->debugLogA("Rates: Error: Data packet contains too many rate groups!");
		wCount = MAX_RATES_GROUP_COUNT;
	}

	nGroups = wCount;
	// Parse Group details
	int i;
	for (i = 0; i < wCount; i++) {
		rates_group *pGroup = &groups[i];

		if (wLen >= 35) {
			pBuffer += 2; // Group ID
			unpackDWord(&pBuffer, &pGroup->dwWindowSize);
			unpackDWord(&pBuffer, &pGroup->dwClearLevel);
			unpackDWord(&pBuffer, &pGroup->dwAlertLevel);
			unpackDWord(&pBuffer, &pGroup->dwLimitLevel);
			pBuffer += 8;
			unpackDWord(&pBuffer, &pGroup->dwMaxLevel);
			pBuffer += 5;
			wLen -= 35;
		}
		else { // packet broken, put some basic defaults
			pGroup->dwWindowSize = 10;
			pGroup->dwMaxLevel = 5000;
		}
		pGroup->rCurrentLevel = pGroup->dwMaxLevel;
	}
	// Parse Group associated pairs
	for (i = 0; i < wCount; i++) {
		rates_group *pGroup = &groups[i];

		if (wLen < 4)
			break;

		pBuffer += 2; // Group ID
		WORD wNum;
		unpackWord(&pBuffer, &wNum);
		wLen -= 4;
		if (wLen < (size_t)wNum * 4)
			break;

		pGroup->nPairs = wNum;
		pGroup->pPairs = (WORD*)SAFE_MALLOC(wNum * 4);
		for (size_t n = 0; n < (size_t)wNum * 2; n++) {
			WORD wItem;
			unpackWord(&pBuffer, &wItem);
			pGroup->pPairs[n] = wItem;
		}

		wLen -= wNum * 4;
	}
}

rates::~rates()
{
	for (int i = 0; i < nGroups; i++)
		SAFE_FREE((void**)&groups[i].pPairs);

	nGroups = 0;
}

WORD rates::getGroupFromSNAC(WORD wFamily, WORD wCommand)
{
	if (this) {
		for (int i = 0; i < nGroups; i++) {
			rates_group* pGroup = &groups[i];

			for (int j = 0; j < 2 * pGroup->nPairs; j += 2) {
				if (pGroup->pPairs[j] == wFamily && pGroup->pPairs[j + 1] == wCommand) { // we found the group
					return (WORD)(i + 1);
				}
			}
		}
		// Legacy ICQ server
	}

	return 0; // Failure
}

WORD rates::getGroupFromPacket(icq_packet *pPacket)
{
	if (this) {
		if (pPacket->nChannel == ICQ_DATA_CHAN && pPacket->wLen >= 0x10) {
			WORD wFamily, wCommand;
			BYTE *pBuf = pPacket->pData + 6;

			unpackWord(&pBuf, &wFamily);
			unpackWord(&pBuf, &wCommand);

			return getGroupFromSNAC(wFamily, wCommand);
		}
	}
	return 0;
}

rates_group* rates::getGroup(WORD wGroup)
{
	if (this && wGroup && wGroup <= nGroups)
		return &groups[wGroup - 1];

	return NULL;
}

int rates::getNextRateLevel(WORD wGroup)
{
	rates_group *pGroup = getGroup(wGroup);

	if (pGroup) {
		int nLevel = pGroup->rCurrentLevel*(pGroup->dwWindowSize - 1) / pGroup->dwWindowSize + (GetTickCount() - pGroup->tCurrentLevel) / pGroup->dwWindowSize;

		return nLevel < (int)pGroup->dwMaxLevel ? nLevel : pGroup->dwMaxLevel;
	}
	return -1; // Failure
}

int rates::getDelayToLimitLevel(WORD wGroup, int nLevel)
{
	rates_group *pGroup = getGroup(wGroup);

	if (pGroup)
		return (getLimitLevel(wGroup, nLevel) - pGroup->rCurrentLevel)*pGroup->dwWindowSize + pGroup->rCurrentLevel;

	return 0; // Failure
}

void rates::packetSent(icq_packet *pPacket)
{
	if (this) {
		WORD wGroup = getGroupFromPacket(pPacket);

		if (wGroup)
			updateLevel(wGroup, getNextRateLevel(wGroup));
	}
}

void rates::updateLevel(WORD wGroup, int nLevel)
{
	rates_group *pGroup = getGroup(wGroup);

	if (pGroup) {
		pGroup->rCurrentLevel = nLevel;
		pGroup->tCurrentLevel = GetTickCount();

		ppro->debugLogA("Rates: New level %d for #%d", nLevel, wGroup);
	}
}

int rates::getLimitLevel(WORD wGroup, int nLevel)
{
	rates_group *pGroup = getGroup(wGroup);

	if (pGroup) {
		switch (nLevel) {
		case RML_CLEAR:
			return pGroup->dwClearLevel;

		case RML_ALERT:
			return pGroup->dwAlertLevel;

		case RML_LIMIT:
			return pGroup->dwLimitLevel;

		case RML_IDLE_10:
			return pGroup->dwClearLevel + ((pGroup->dwMaxLevel - pGroup->dwClearLevel) / 10);

		case RML_IDLE_30:
			return pGroup->dwClearLevel + (3 * (pGroup->dwMaxLevel - pGroup->dwClearLevel) / 10);

		case RML_IDLE_50:
			return pGroup->dwClearLevel + ((pGroup->dwMaxLevel - pGroup->dwClearLevel) / 2);

		case RML_IDLE_70:
			return pGroup->dwClearLevel + (7 * (pGroup->dwMaxLevel - pGroup->dwClearLevel) / 10);
		}
	}
	return 9999; // some high number - without rates we allow anything
}

void rates::initAckPacket(icq_packet *pPacket)
{
	serverPacketInit(pPacket, 10 + nGroups * sizeof(WORD));
	packFNACHeader(pPacket, ICQ_SERVICE_FAMILY, ICQ_CLIENT_RATE_ACK);
	for (WORD wGroup = 1; wGroup <= nGroups; wGroup++)
		packWord(pPacket, wGroup);
}

//
// Rate Level 2 Management
/////////////////////////////

rates_queue_item::rates_queue_item(CIcqProto *ppro, WORD wGroup) : bCreated(FALSE), dwUin(0), szUid(NULL)
{
	this->ppro = ppro;
	this->wGroup = wGroup;
}

rates_queue_item::~rates_queue_item()
{
	if (bCreated) {
		SAFE_FREE(&szUid);
		bCreated = FALSE;
	}
}

BOOL rates_queue_item::isEqual(rates_queue_item *pItem)
{
	// the same event (equal address of _vftable) for the same contact
	return (pItem->hContact == this->hContact) && (*(void**)pItem == *(void**)this);
}

rates_queue_item* rates_queue_item::copyItem(rates_queue_item *pDest)
{
	if (!pDest)
		pDest = new rates_queue_item(ppro, wGroup);

	pDest->hContact = hContact;
	pDest->dwUin = dwUin;
	pDest->szUid = dwUin ? null_strdup(szUid) : NULL;
	pDest->bCreated = TRUE;

	return pDest;
}

void rates_queue_item::execute()
{
	ppro->debugLogA("Rates: Error executing abstract event.");
}

BOOL rates_queue_item::isOverRate(int nLevel)
{
	mir_cslock l(ppro->m_ratesMutex);

	if (ppro->m_rates)
		return ppro->m_rates->getNextRateLevel(wGroup) < ppro->m_rates->getLimitLevel(wGroup, nLevel);

	return FALSE;
}

rates_queue::rates_queue(CIcqProto *_ppro, const char *_descr, int nLimitLevel, int nWaitLevel, int nDuplicates) :
	lstPending(1)
{
	ppro = _ppro;
	szDescr = _descr;
	limitLevel = nLimitLevel;
	waitLevel = nWaitLevel;
	duplicates = nDuplicates;
}

rates_queue::~rates_queue()
{
	cleanup();
}

// links to functions that are under Rate Control
struct rate_delay_args
{
	int nDelay;
	rates_queue *queue;
	IcqRateFunc delaycode;
};

void __cdecl CIcqProto::rateDelayThread(rate_delay_args *pArgs)
{
	SleepEx(pArgs->nDelay, TRUE);
	(pArgs->queue->*pArgs->delaycode)();
	SAFE_FREE((void**)&pArgs);
}

void rates_queue::initDelay(int nDelay, IcqRateFunc delaycode)
{
	ppro->debugLogA("Rates: Delay %dms", nDelay);

	rate_delay_args *pArgs = (rate_delay_args*)SAFE_MALLOC(sizeof(rate_delay_args)); // This will be freed in the new thread
	pArgs->queue = this;
	pArgs->nDelay = nDelay;
	pArgs->delaycode = delaycode;

	ppro->ForkThread((CIcqProto::MyThreadFunc)&CIcqProto::rateDelayThread, pArgs);
}

void rates_queue::cleanup()
{
	mir_cslock l(csLists);
	for (int i = 0; i < lstPending.getCount(); i++)
		delete lstPending[i];
	lstPending.destroy();
}

void rates_queue::processQueue()
{
	if (!ppro->icqOnline()) {
		cleanup();
		return;
	}

	// take from queue, execute
	mir_cslockfull l(csLists);
	if (lstPending.getCount() == 0)
		return;

	rates_queue_item *item = lstPending[0];
	{
		mir_cslockfull rlck(ppro->m_ratesMutex);
		if (item->isOverRate(limitLevel)) { // the rate is higher, keep sleeping
			int nDelay = ppro->m_rates->getDelayToLimitLevel(item->wGroup, ppro->m_rates->getLimitLevel(item->wGroup, waitLevel));

			l.unlock();
			rlck.unlock();
			if (nDelay < 10) nDelay = 10;
			initDelay(nDelay, &rates_queue::processQueue);
			return;
		}
	}

	lstPending.remove(int(0));
	bool bSetupTimer = lstPending.getCount() != 0;

	l.unlock();

	if (ppro->icqOnline()) {
		ppro->debugLogA("Rates: Resuming %s.", szDescr);
		item->execute();
	}
	else ppro->debugLogA("Rates: Discarding %s.", szDescr);

	if (bSetupTimer) { // in queue remained some items, setup timer
		int nDelay;
		{
			mir_cslockfull rlck(ppro->m_ratesMutex);
			nDelay = ppro->m_rates->getDelayToLimitLevel(item->wGroup, waitLevel);
		}

		if (nDelay < 10) nDelay = 10;
		initDelay(nDelay, &rates_queue::processQueue);
	}
	delete item;
}

void rates_queue::putItem(rates_queue_item *pItem, int nMinDelay)
{
	if (!ppro->icqOnline())
		return;

	ppro->debugLogA("Rates: Delaying %s.", szDescr);
	{
		mir_cslock l(csLists);
		if (lstPending.getCount()) {
			for (int i = 0; i < lstPending.getCount(); i++) {
				if (lstPending[i]->isEqual(pItem)) {
					if (duplicates == 1) // keep existing, ignore new
						return;

					if (duplicates == -1) { // discard existing, append new item
						delete lstPending[i];
						lstPending.remove(i);
					}
				}
			}
		}

		lstPending.insert(pItem->copyItem());
	}

	if (lstPending.getCount() != 1)
		return;
	
	// queue was empty setup timer
	int nDelay;
	{	mir_cslock rlck(ppro->m_ratesMutex);
		nDelay = ppro->m_rates->getDelayToLimitLevel(pItem->wGroup, waitLevel);
	}

	if (nDelay < 10) nDelay = 10;
	if (nDelay < nMinDelay) nDelay = nMinDelay;
	initDelay(nDelay, &rates_queue::processQueue);
}

int CIcqProto::handleRateItem(rates_queue_item *item, int nQueueType, int nMinDelay, BOOL bAllowDelay)
{
	rates_queue *pQueue = NULL;
	{
		mir_cslock rlck(m_ratesMutex);
		switch (nQueueType) {
		case RQT_REQUEST:
			pQueue = m_ratesQueue_Request;
			break;
		case RQT_RESPONSE:
			pQueue = m_ratesQueue_Response;
			break;
		}

		if (pQueue) {
			if (bAllowDelay && (item->isOverRate(pQueue->waitLevel) || nMinDelay)) {
				// limit reached or min delay configured, add to queue
				pQueue->putItem(item, nMinDelay);
				return 1;
			}
		}
	}

	item->execute();
	return 0;
}
