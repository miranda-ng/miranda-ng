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
//  Functions that handles list of used server IDs, sends low-level packets for SSI information
// -----------------------------------------------------------------------------

#include "icqoscar.h"

// SERVER-LIST UPDATE BOARD
//

void CIcqProto::servlistBeginOperation(int operationCount, int bImport)
{
	if (operationCount)
	{ // check if we should send operation begin packet
		if (!servlistEditCount)
			icq_sendServerBeginOperation(bImport);
		// update count of active operations
		servlistEditCount += operationCount;
#ifdef _DEBUG
		debugLogA("Server-List: Begin operation processed (%d operations active)", servlistEditCount);
#endif
	}
}

void CIcqProto::servlistEndOperation(int operationCount)
{
	if (operationCount)
	{
		if (operationCount > servlistEditCount)
		{ // sanity check
			debugLogA("Error: Server-List End operation is not paired!");
			operationCount = servlistEditCount;
		}
		// update count of active operations
		servlistEditCount -= operationCount;
		// check if we should send operation end packet
		if (!servlistEditCount)
			icq_sendServerEndOperation();
#ifdef _DEBUG
		debugLogA("Server-List: End operation processed (%d operations active)", servlistEditCount);
#endif
	}
}

void __cdecl CIcqProto::servlistQueueThread(void *param)
{
	int* queueState = ( int* )param;

#ifdef _DEBUG
	debugLogA("Server-List: Starting Update board.");
#endif

	SleepEx(50, FALSE);
	// handle server-list requests queue
	servlistQueueMutex->Enter();
	while (servlistQueueCount)
	{
		ssiqueueditems* pItem = NULL;
		int bItemDouble;
		WORD wItemAction;
		icq_packet groupPacket = {0};
		icq_packet groupPacket2 = {0}; 
		cookie_servlist_action* pGroupCookie = NULL;
		int nEndOperations;

		// first check if the state is calm
		while (*queueState) 
		{
			int i;
			time_t tNow = time(NULL);
			int bFound = FALSE;

			for (i = 0; i < servlistQueueCount; i++)
			{ // check if we do not have some expired items to handle, otherwise keep sleeping
				if ((servlistQueueList[i]->tAdded + servlistQueueList[i]->dwTimeout) < tNow)
				{ // got expired item, stop sleep even when changes goes on
					bFound = TRUE;
					break;
				}
			}
			if (bFound) break;
			// reset queue state, keep sleeping
			*queueState = FALSE; 
			servlistQueueMutex->Leave();
			SleepEx(100, TRUE);
			servlistQueueMutex->Enter();
		}
		if (!icqOnline())
		{ // do not try to send packets if offline
			servlistQueueMutex->Leave();
			SleepEx(100, TRUE);
			servlistQueueMutex->Enter();
			continue;
		}
#ifdef _DEBUG
		debugLogA("Server-List: %d items in queue.", servlistQueueCount);
#endif
		// take the oldest item (keep the board FIFO)
		pItem = servlistQueueList[0]; // take first (queue contains at least one item here)
		wItemAction = (WORD)(pItem->pItems[0]->dwOperation & SSOF_ACTIONMASK);
		bItemDouble = pItem->pItems[0]->dwOperation & SSOG_DOUBLE;
		// check item rate - too high -> sleep
		m_ratesMutex->Enter();
		{
			WORD wRateGroup = m_rates->getGroupFromSNAC(ICQ_LISTS_FAMILY, wItemAction);
			int nRateLevel = bItemDouble ? RML_IDLE_30 : RML_IDLE_10;

			while (m_rates->getNextRateLevel(wRateGroup) < m_rates->getLimitLevel(wRateGroup, nRateLevel))
			{ // the rate is higher, keep sleeping
				int nDelay = m_rates->getDelayToLimitLevel(wRateGroup, nRateLevel);

				m_ratesMutex->Leave();
				// do not keep the queue frozen
				servlistQueueMutex->Leave();
				if (nDelay < 10) nDelay = 10;
#ifdef _DEBUG
				debugLogA("Server-List: Delaying %dms [Rates]", nDelay);
#endif
				SleepEx(nDelay, FALSE);
				// check if the rate is now ok
				servlistQueueMutex->Enter();
				m_ratesMutex->Enter();
			}    
		}
		m_ratesMutex->Leave();
		{ // setup group packet(s) & cookie
			int totalSize = 0;
			int i;
			cookie_servlist_action *pGroupCookie;
			DWORD dwGroupCookie;
			// determine the total size of the packet
			for(i = 0; i < pItem->nItems; i++)
				totalSize += pItem->pItems[i]->packet.wLen - 0x10;

			// process begin & end operation flags
			{
				int bImportOperation = FALSE;
				int nBeginOperations = 0;

				nEndOperations = 0;
				for(i = 0; i < pItem->nItems; i++)
				{ // collect begin & end operation flags
					if (pItem->pItems[i]->dwOperation & SSOF_BEGIN_OPERATION)
						nBeginOperations++;
					if (pItem->pItems[i]->dwOperation & SSOF_END_OPERATION)
						nEndOperations++;
					// check if the operation is import
					if (pItem->pItems[i]->dwOperation & SSOF_IMPORT_OPERATION)
						bImportOperation = TRUE;
				}
				// really begin operation if requested
				if (nBeginOperations)
					servlistBeginOperation(nBeginOperations, bImportOperation);
			}

			if (pItem->nItems > 1)
			{ // pack all packet's data, create group cookie
				pGroupCookie = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				pGroupCookie->dwAction = SSA_ACTION_GROUP;
				pGroupCookie->dwGroupCount = pItem->nItems;
				pGroupCookie->pGroupItems = (cookie_servlist_action**)SAFE_MALLOC(pItem->nItems * sizeof(cookie_servlist_action*));
				for (i = 0; i < pItem->nItems; i++)
				{ // build group cookie data - assign cookies datas
					pGroupCookie->pGroupItems[i] = pItem->pItems[i]->cookie;
					// release the separate cookie id
					FreeCookieByData(CKT_SERVERLIST, pItem->pItems[i]->cookie);
				}
				// allocate cookie id
				dwGroupCookie = AllocateCookie(CKT_SERVERLIST, wItemAction, 0, pGroupCookie);
				// prepare packet data
				serverPacketInit(&groupPacket, (WORD)(totalSize + 0x0A)); // FLAP size added inside
				packFNACHeader(&groupPacket, ICQ_LISTS_FAMILY, wItemAction, 0, dwGroupCookie);
				for (i = 0; i < pItem->nItems; i++)
					packBuffer(&groupPacket, pItem->pItems[i]->packet.pData + 0x10, (WORD)(pItem->pItems[i]->packet.wLen - 0x10));

				if (bItemDouble)
				{ // prepare second packet
					wItemAction = ((servlistgroupitemdouble*)(pItem->pItems[0]))->wAction2;
					totalSize = 0;
					// determine the total size of the packet
					for(i = 0; i < pItem->nItems; i++)
						totalSize += ((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.wLen - 0x10;

					pGroupCookie = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
					pGroupCookie->dwAction = SSA_ACTION_GROUP;
					pGroupCookie->dwGroupCount = pItem->nItems;
					pGroupCookie->pGroupItems = (cookie_servlist_action**)SAFE_MALLOC(pItem->nItems * sizeof(cookie_servlist_action*));
					for (i = 0; i < pItem->nItems; i++)
						pGroupCookie->pGroupItems[i] = pItem->pItems[i]->cookie;
					// allocate cookie id
					dwGroupCookie = AllocateCookie(CKT_SERVERLIST, wItemAction, 0, pGroupCookie);
					// prepare packet data
					serverPacketInit(&groupPacket2, (WORD)(totalSize + 0x0A)); // FLAP size added inside
					packFNACHeader(&groupPacket2, ICQ_LISTS_FAMILY, wItemAction, 0, dwGroupCookie);
					for (i = 0; i < pItem->nItems; i++)
						packBuffer(&groupPacket2, ((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.pData + 0x10, (WORD)(((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.wLen - 0x10));
				}
			}
			else
			{ // just send the one packet, do not create action group
				pGroupCookie = pItem->pItems[0]->cookie;
				memcpy(&groupPacket, &pItem->pItems[0]->packet, sizeof(icq_packet));
				if (bItemDouble)
					memcpy(&groupPacket2, &((servlistgroupitemdouble*)(pItem->pItems[0]))->packet2, sizeof(icq_packet));
			}

			{ // remove grouped item from queue & release grouped item
				servlistQueueCount--;
				servlistQueueList[0] = servlistQueueList[servlistQueueCount];

				for (i = 0; i < pItem->nItems; i++)
				{ // release memory
					if (pItem->nItems > 1)
					{ // free the packet only if we created the group packet
						SAFE_FREE((void**)&pItem->pItems[i]->packet.pData);
						if (pItem->pItems[i]->dwOperation & SSOG_DOUBLE)
							SAFE_FREE((void**)&((servlistgroupitemdouble*)(pItem->pItems[i]))->packet2.pData);
					}
					SAFE_FREE((void**)&pItem->pItems[i]);
					break;
				}
				SAFE_FREE((void**)&pItem);
				// resize the queue
				if (servlistQueueSize > servlistQueueCount + 6)
				{
					servlistQueueSize -= 4;
					servlistQueueList = (ssiqueueditems**)SAFE_REALLOC(servlistQueueList, servlistQueueSize * sizeof(ssiqueueditems*));
				}
			}
		}
		servlistQueueMutex->Leave();
		// send group packet
		sendServPacket(&groupPacket);
		// send second group packet (if present)
		if (bItemDouble)
			sendServPacket(&groupPacket2);
		// process end operation marks
		if (nEndOperations)
			servlistEndOperation(nEndOperations);
		// loose the loop a bit
		SleepEx(100, TRUE);
		servlistQueueMutex->Enter();
	}
	// clean-up thread
	CloseHandle(servlistQueueThreadHandle);
	servlistQueueThreadHandle = NULL;
	servlistQueueMutex->Leave();
#ifdef _DEBUG
	debugLogA("Server-List: Update Board ending.");
#endif
}

void CIcqProto::servlistQueueAddGroupItem(servlistgroupitem* pGroupItem, int dwTimeout)
{
	icq_lock l(servlistQueueMutex);

	{ // add the packet to queue
		DWORD dwMark = pGroupItem->dwOperation & SSOF_GROUPINGMASK;
		ssiqueueditems* pItem = NULL;

		// try to find compatible item
		for (int i = 0; i < servlistQueueCount; i++)
		{
			if ((servlistQueueList[i]->pItems[0]->dwOperation & SSOF_GROUPINGMASK) == dwMark && servlistQueueList[i]->nItems < MAX_SERVLIST_PACKET_ITEMS)
			{ // found compatible item, check if it does not contain operation for the same server-list item
				pItem = servlistQueueList[i];

				for (int j = 0; j < pItem->nItems; j++)
					if (pItem->pItems[j]->cookie->wContactId == pGroupItem->cookie->wContactId &&
						pItem->pItems[j]->cookie->wGroupId == pGroupItem->cookie->wGroupId)
					{
						pItem = NULL;
						break;
					}
					// cannot send two operations for the same server-list record in one packet, look for another
					if (!pItem) continue;

#ifdef _DEBUG
					debugLogA("Server-List: Adding packet to item #%d with operation %x.", i, servlistQueueList[i]->pItems[0]->dwOperation);
#endif
					break;
			}
		}
		if (!pItem)
		{ // compatible item was not found, create new one, add to queue
			pItem = (ssiqueueditems*)SAFE_MALLOC(sizeof(ssiqueueditems));
			pItem->tAdded = time(NULL);
			pItem->dwTimeout = dwTimeout;

			if (servlistQueueCount == servlistQueueSize)
			{ // resize the queue - it is too small
				servlistQueueSize += 4;
				servlistQueueList = (ssiqueueditems**)SAFE_REALLOC(servlistQueueList, servlistQueueSize * sizeof(ssiqueueditems*));     
			}
			// really add to queue
			servlistQueueList[servlistQueueCount++] = pItem;
#ifdef _DEBUG
			debugLogA("Server-List: Adding new item to queue.");
#endif
		} 
		else if (pItem->dwTimeout > dwTimeout)
		{ // if the timeout of currently added packet is shorter, update the previous one
			pItem->dwTimeout = dwTimeout;
		}
		// add GroupItem to queueditems (pItem)
		pItem->pItems[pItem->nItems++] = pGroupItem;
	}
	// wake up board thread (keep sleeping or start new one)  
	if (!servlistQueueThreadHandle)
	{
		// create new board thread
		servlistQueueThreadHandle = ForkThreadEx(&CIcqProto::servlistQueueThread, &servlistQueueState, 0);
	}
	else // signal thread, that queue was changed during sleep
		servlistQueueState = TRUE;
}

int CIcqProto::servlistHandlePrimitives(DWORD dwOperation)
{
	if (dwOperation & SSO_BEGIN_OPERATION)
	{ // operation starting, no action ready yet
		servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);
		return TRUE;
	}
	else if (dwOperation & SSO_END_OPERATION)
	{ // operation ending without action
		servlistEndOperation(1);
		return TRUE;
	}

	return FALSE;
}

void CIcqProto::servlistPostPacket(icq_packet* packet, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout)
{
	cookie_servlist_action* pCookie;

	if (servlistHandlePrimitives(dwOperation))
		return;

	if (!FindCookie(dwCookie, NULL, (void**)&pCookie))
		return; // invalid cookie

	if (dwOperation & SSOF_SEND_DIRECTLY)
	{ // send directly - this is for some special cases
		// begin operation if requested
		if (dwOperation & SSOF_BEGIN_OPERATION)
			servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);

		// send the packet
		sendServPacket(packet);

		// end operation if requested
		if (dwOperation & SSOF_END_OPERATION)
			servlistEndOperation(1);
	}
	else
	{ // add to server-list update board
		servlistgroupitem* pGroupItem;

		// prepare group item
		pGroupItem = (servlistgroupitem*)SAFE_MALLOC(sizeof(servlistgroupitem));
		pGroupItem->dwOperation = dwOperation;
		pGroupItem->cookie = pCookie;
		// packet data are alloced, keep them until they are sent
		memcpy(&pGroupItem->packet, packet, sizeof(icq_packet));

		servlistQueueAddGroupItem(pGroupItem, dwTimeout);
	}
}

void CIcqProto::servlistPostPacketDouble(icq_packet* packet1, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout, icq_packet* packet2, WORD wAction2)
{
	cookie_servlist_action* pCookie;

	if (servlistHandlePrimitives(dwOperation))
		return;

	if (!FindCookie(dwCookie, NULL, (void**)&pCookie))
		return; // invalid cookie

	if (dwOperation & SSOF_SEND_DIRECTLY)
	{ // send directly - this is for some special cases
		// begin operation if requested
		if (dwOperation & SSOF_BEGIN_OPERATION)
			servlistBeginOperation(1, dwOperation & SSOF_IMPORT_OPERATION);

		// send the packets
		sendServPacket(packet1);
		sendServPacket(packet2);

		// end operation if requested
		if (dwOperation & SSOF_END_OPERATION)
			servlistEndOperation(1);
	}
	else
	{ // add to server-list update board
		servlistgroupitemdouble* pGroupItem;

		// prepare group item
		pGroupItem = (servlistgroupitemdouble*)SAFE_MALLOC(sizeof(servlistgroupitemdouble));
		pGroupItem->dwOperation = dwOperation;
		pGroupItem->cookie = pCookie;
		pGroupItem->wAction2 = wAction2;
		// packets data are alloced, keep them until they are sent
		memcpy(&pGroupItem->packet, packet1, sizeof(icq_packet));
		memcpy(&pGroupItem->packet2, packet2, sizeof(icq_packet));

		servlistQueueAddGroupItem((servlistgroupitem*)pGroupItem, dwTimeout);
	}
}

void CIcqProto::servlistProcessLogin()
{
	// reset edit state counter
	servlistEditCount = 0;

	/// TODO: preserve queue state in DB! restore here!

	// if the server-list queue contains items and thread is not running, start it
	if (servlistQueueCount && !servlistQueueThreadHandle)
		servlistQueueThreadHandle = ForkThreadEx(&CIcqProto::servlistQueueThread, &servlistQueueState, 0);
}

// HERE ENDS SERVER-LIST UPDATE BOARD IMPLEMENTATION //
///////////////////////////////////////////////////////
//===================================================//

// PENDING SERVER-LIST OPERATIONS
//
#define ITEM_PENDING_CONTACT    0x01
#define ITEM_PENDING_GROUP      0x02

#define CALLBACK_RESULT_CONTINUE  0x00
#define CALLBACK_RESULT_POSTPONE  0x0D
#define CALLBACK_RESULT_PURGE     0x10


#define SPOF_AUTO_CREATE_ITEM     0x01

int CIcqProto::servlistPendingFindItem(int nType, MCONTACT hContact, const char *pszGroup)
{
	if (servlistPendingList)
		for (int i = 0; i < servlistPendingCount; i++)
			if (servlistPendingList[i]->nType == nType)
			{ 
				if (((nType == ITEM_PENDING_CONTACT) && (servlistPendingList[i]->hContact == hContact)) ||
					((nType == ITEM_PENDING_GROUP) && (!strcmpnull(servlistPendingList[i]->szGroup, pszGroup))))
					return i;
			}
			return -1;
}


void CIcqProto::servlistPendingAddItem(servlistpendingitem *pItem)
{
	if (servlistPendingCount >= servlistPendingSize) // add new
	{
		servlistPendingSize += 10;
		servlistPendingList = (servlistpendingitem**)SAFE_REALLOC(servlistPendingList, servlistPendingSize * sizeof(servlistpendingitem*));
	}

	servlistPendingList[servlistPendingCount++] = pItem;
}


servlistpendingitem* CIcqProto::servlistPendingRemoveItem(int nType, MCONTACT hContact, const char *pszGroup)
{ // unregister pending item, trigger pending operations
	int iItem;
	servlistpendingitem *pItem = NULL;

	icq_lock l(servlistMutex);

	if ((iItem = servlistPendingFindItem(nType, hContact, pszGroup)) != -1)
	{ // found, remove from the pending list
		pItem = servlistPendingList[iItem];

		servlistPendingList[iItem] = servlistPendingList[--servlistPendingCount];

		if (servlistPendingCount + 10 < servlistPendingSize)
		{
			servlistPendingSize -= 5;
			servlistPendingList = (servlistpendingitem**)SAFE_REALLOC(servlistPendingList, servlistPendingSize * sizeof(servlistpendingitem*));
		}
		// was the first operation was created automatically to postpone ItemAdd?
		if (pItem->operations && pItem->operations[0].flags & SPOF_AUTO_CREATE_ITEM)
		{ // yes, add new item
			servlistpendingitem *pNewItem = (servlistpendingitem*)SAFE_MALLOC(sizeof(servlistpendingitem));

			if (pNewItem)
			{ // move the remaining operations
#ifdef _DEBUG
				if (pItem->nType == ITEM_PENDING_CONTACT)
					debugLogA("Server-List: Resuming contact %x operation.", pItem->hContact);
				else
					debugLogA("Server-List: Resuming group \"%s\" operation.", pItem->szGroup);
#endif

				pNewItem->nType = pItem->nType;
				pNewItem->hContact = pItem->hContact;
				pNewItem->szGroup = null_strdup(pItem->szGroup);
				pNewItem->wContactID = pItem->wContactID;
				pNewItem->wGroupID = pItem->wGroupID;
				pNewItem->operationsCount = pItem->operationsCount - 1;
				pNewItem->operations = (servlistpendingoperation*)SAFE_MALLOC(pNewItem->operationsCount * sizeof(servlistpendingoperation));
				memcpy(pNewItem->operations, pItem->operations + 1, pNewItem->operationsCount * sizeof(servlistpendingoperation));
				pItem->operationsCount = 1;

				servlistPendingAddItem(pNewItem);
				// clear the flag
				pItem->operations[0].flags &= ~SPOF_AUTO_CREATE_ITEM;
			}
		}
	}
#ifdef _DEBUG
	else
		debugLogA("Server-List Error: Trying to remove non-existing pending %s.", nType == ITEM_PENDING_CONTACT ? "contact" : "group");
#endif

	return pItem;
}


void CIcqProto::servlistPendingAddContactOperation(MCONTACT hContact, LPARAM param, PENDING_CONTACT_CALLBACK callback, DWORD flags)
{ // add postponed operation (add contact, update contact, regroup resume, etc.)
	// - after contact is added
	int iItem;
	servlistpendingitem *pItem = NULL;

	icq_lock l(servlistMutex);

	if ((iItem = servlistPendingFindItem(ITEM_PENDING_CONTACT, hContact, NULL)) != -1)
		pItem = servlistPendingList[iItem];

	if (pItem)
	{
		int iOperation = pItem->operationsCount++;

		pItem->operations = (servlistpendingoperation*)SAFE_REALLOC(pItem->operations, pItem->operationsCount * sizeof(servlistpendingoperation));
		pItem->operations[iOperation].param = param;
		pItem->operations[iOperation].callback = (PENDING_GROUP_CALLBACK)callback;
		pItem->operations[iOperation].flags = flags;
	}
	else
	{
		debugLogA("Server-List Error: Trying to add pending operation to a non existing contact.");
	}
}


void CIcqProto::servlistPendingAddGroupOperation(const char *pszGroup, LPARAM param, PENDING_GROUP_CALLBACK callback, DWORD flags)
{ // add postponed operation - after group is added
	int iItem;
	servlistpendingitem *pItem = NULL;

	icq_lock l(servlistMutex);

	if ((iItem = servlistPendingFindItem(ITEM_PENDING_GROUP, NULL, pszGroup)) != -1)
		pItem = servlistPendingList[iItem];

	if (pItem)
	{
		int iOperation = pItem->operationsCount++;

		pItem->operations = (servlistpendingoperation*)SAFE_REALLOC(pItem->operations, pItem->operationsCount * sizeof(servlistpendingoperation));
		pItem->operations[iOperation].param = param;
		pItem->operations[iOperation].callback = callback;
		pItem->operations[iOperation].flags = flags;
	}
	else
	{
		debugLogA("Server-List Error: Trying to add pending operation to a non existing group.");
	}
}


int CIcqProto::servlistPendingAddContact(MCONTACT hContact, WORD wContactID, WORD wGroupID, LPARAM param, PENDING_CONTACT_CALLBACK callback, int bDoInline, LPARAM operationParam, PENDING_CONTACT_CALLBACK operationCallback)
{
	int iItem;
	servlistpendingitem *pItem = NULL;

	servlistMutex->Enter();

	if ((iItem = servlistPendingFindItem(ITEM_PENDING_CONTACT, hContact, NULL)) != -1)
		pItem = servlistPendingList[iItem];

	if (pItem)
	{
#ifdef _DEBUG
		debugLogA("Server-List: Pending contact %x already in list; adding as operation.", hContact);
#endif
		servlistPendingAddContactOperation(hContact, param, callback, SPOF_AUTO_CREATE_ITEM);

		if (operationCallback)
			servlistPendingAddContactOperation(hContact, operationParam, operationCallback, 0);

		servlistMutex->Leave();

		return 0; // Pending
	}

#ifdef _DEBUG
	debugLogA("Server-List: Starting contact %x operation.", hContact);
#endif

	pItem = (servlistpendingitem *)SAFE_MALLOC(sizeof(servlistpendingitem));
	pItem->nType = ITEM_PENDING_CONTACT;
	pItem->hContact = hContact;
	pItem->wContactID = wContactID;
	pItem->wGroupID = wGroupID;

	servlistPendingAddItem(pItem);

	if (operationCallback)
		servlistPendingAddContactOperation(hContact, operationParam, operationCallback, 0);

	servlistMutex->Leave();

	if (bDoInline)
	{ // not postponed, called directly if requested
		(this->*callback)(hContact, wContactID, wGroupID, param, PENDING_RESULT_INLINE);
	}

	return 1; // Ready
}


int CIcqProto::servlistPendingAddGroup(const char *pszGroup, WORD wGroupID, LPARAM param, PENDING_GROUP_CALLBACK callback, int bDoInline, LPARAM operationParam, PENDING_GROUP_CALLBACK operationCallback)
{
	int iItem;
	servlistpendingitem *pItem = NULL;

	servlistMutex->Enter();

	if ((iItem = servlistPendingFindItem(ITEM_PENDING_GROUP, NULL, pszGroup)) != -1)
		pItem = servlistPendingList[iItem];

	if (pItem)
	{
#ifdef _DEBUG
		debugLogA("Server-List: Pending group \"%s\" already in list; adding as operation.", pszGroup);
#endif
		servlistPendingAddGroupOperation(pszGroup, param, callback, SPOF_AUTO_CREATE_ITEM);

		if (operationCallback)
			servlistPendingAddGroupOperation(pszGroup, operationParam, operationCallback, 0);

		servlistMutex->Leave();

		return 0; // Pending
	}

#ifdef _DEBUG
	debugLogA("Server-List: Starting group \"%s\" operation.", pszGroup);
#endif

	pItem = (servlistpendingitem *)SAFE_MALLOC(sizeof(servlistpendingitem));
	pItem->nType = ITEM_PENDING_GROUP;
	pItem->szGroup = null_strdup(pszGroup);
	pItem->wGroupID = wGroupID;

	servlistPendingAddItem(pItem);

	if (operationCallback)
		servlistPendingAddGroupOperation(pszGroup, operationParam, operationCallback, 0);

	servlistMutex->Leave();

	if (bDoInline)
	{ // not postponed, called directly if requested
		(this->*callback)(pszGroup, wGroupID, param, PENDING_RESULT_INLINE);
	}

	return 1; // Ready
}


void CIcqProto::servlistPendingRemoveContact(MCONTACT hContact, WORD wContactID, WORD wGroupID, int nResult)
{
#ifdef _DEBUG
	debugLogA("Server-List: %s contact %x operation.", (nResult != PENDING_RESULT_PURGE) ? "Ending" : "Purging", hContact);
#endif

	servlistpendingitem *pItem = servlistPendingRemoveItem(ITEM_PENDING_CONTACT, hContact, NULL);

	if (pItem)
	{ // process pending operations
		if (pItem->operations)
		{
			for (int i = 0; i < pItem->operationsCount; i++)
			{
				int nCallbackState = (this->*(PENDING_CONTACT_CALLBACK)(pItem->operations[i].callback))(hContact, wContactID, wGroupID, pItem->operations[i].param, nResult);

				if (nResult != PENDING_RESULT_PURGE && nCallbackState == CALLBACK_RESULT_POSTPONE)
				{ // any following pending operations cannot be processed now, move them to the new pending contact
					for (int j = i + 1; j < pItem->operationsCount; j++)
						servlistPendingAddContactOperation(hContact, pItem->operations[j].param, (PENDING_CONTACT_CALLBACK)(pItem->operations[j].callback), pItem->operations[j].flags);
					break;
				}
				else if (nCallbackState == CALLBACK_RESULT_PURGE)
				{ // purge all following operations - fatal failure occured
					nResult = PENDING_RESULT_PURGE;
				}
			}
			SAFE_FREE((void**)&pItem->operations);
		}
		// release item's memory
		SAFE_FREE((void**)&pItem);
	}
	else
		debugLogA("Server-List Error: Trying to remove a non existing pending contact.");
}


void CIcqProto::servlistPendingRemoveGroup(const char *pszGroup, WORD wGroupID, int nResult)
{
#ifdef _DEBUG
	debugLogA("Server-List: %s group \"%s\" operation.", (nResult != PENDING_RESULT_PURGE) ? "Ending" : "Purging", pszGroup);
#endif

	servlistpendingitem *pItem = servlistPendingRemoveItem(ITEM_PENDING_GROUP, NULL, pszGroup);

	if (pItem)
	{ // process pending operations
		if (pItem->operations)
		{
			for (int i = 0; i < pItem->operationsCount; i++)
			{
				int nCallbackState = (this->*pItem->operations[i].callback)(pItem->szGroup, wGroupID, pItem->operations[i].param, nResult);

				if (nResult != PENDING_RESULT_PURGE && nCallbackState == CALLBACK_RESULT_POSTPONE)
				{ // any following pending operations cannot be processed now, move them to the new pending group
					for (int j = i + 1; j < pItem->operationsCount; j++)
						servlistPendingAddGroupOperation(pItem->szGroup, pItem->operations[j].param, pItem->operations[j].callback, pItem->operations[j].flags);
					break;
				}
				else if (nCallbackState == CALLBACK_RESULT_PURGE)
				{ // purge all following operations - fatal failure occured
					nResult = PENDING_RESULT_PURGE;
				}
			}
			SAFE_FREE((void**)&pItem->operations);
		}
		// release item's memory
		SAFE_FREE((void**)&pItem->szGroup);
		SAFE_FREE((void**)&pItem);
	}
	else
		debugLogA("Server-List Error: Trying to remove a non existing pending group.");
}


// Remove All pending operations
void CIcqProto::servlistPendingFlushOperations()
{
	icq_lock l(servlistMutex);

	for (int i = servlistPendingCount; i; i--)
	{ // purge all items
		servlistpendingitem *pItem = servlistPendingList[i - 1];

		if (pItem->nType == ITEM_PENDING_CONTACT)
			servlistPendingRemoveContact(pItem->hContact, 0, 0, PENDING_RESULT_PURGE);
		else if (pItem->nType == ITEM_PENDING_GROUP)
			servlistPendingRemoveGroup(pItem->szGroup, 0, PENDING_RESULT_PURGE);
	}
	// release the list completely
	SAFE_FREE((void**)&servlistPendingList);
	servlistPendingCount = 0;
	servlistPendingSize = 0;
}

// END OF SERVER-LIST PENDING OPERATIONS
////


// used for adding new contacts to list - sync with visible items
void CIcqProto::AddJustAddedContact(MCONTACT hContact)
{
	icq_lock l(servlistMutex);

	if (nJustAddedCount >= nJustAddedSize)
	{
		nJustAddedSize += 10;
		pdwJustAddedList = (MCONTACT*)SAFE_REALLOC(pdwJustAddedList, nJustAddedSize * sizeof(MCONTACT));
	}

	pdwJustAddedList[nJustAddedCount] = hContact;
	nJustAddedCount++;  
}


// was the contact added during this serv-list load
BOOL CIcqProto::IsContactJustAdded(MCONTACT hContact)
{
	icq_lock l(servlistMutex);

	if (pdwJustAddedList)
	{
		for (int i = 0; i<nJustAddedCount; i++)
		{
			if (pdwJustAddedList[i] == hContact)
				return TRUE;
		}
	}

	return FALSE;
}


void CIcqProto::FlushJustAddedContacts()
{
	icq_lock l(servlistMutex);

	SAFE_FREE((void**)&pdwJustAddedList);
	nJustAddedSize = 0;
	nJustAddedCount = 0;
}


// Add a server ID to the list of reserved IDs.
// To speed up the process, no checks is done, if
// you try to reserve an ID twice, it will be added again.
// You should call CheckServerID before reserving an ID.
void CIcqProto::ReserveServerID(WORD wID, int bGroupType, int bFlags)
{
	servlistMutex->Enter();
	if (nServerIDListCount >= nServerIDListSize)
	{
		nServerIDListSize += 100;
		pdwServerIDList = (DWORD*)SAFE_REALLOC(pdwServerIDList, nServerIDListSize * sizeof(DWORD));
	}

	pdwServerIDList[nServerIDListCount] = wID | (bGroupType & 0x00FF0000) | (bFlags & 0xFF000000);
	nServerIDListCount++;
	servlistMutex->Leave();

	if (!bIsSyncingCL)
		StoreServerIDs();
}


// Remove a server ID from the list of reserved IDs.
// Used for deleting contacts and other modifications.
void CIcqProto::FreeServerID(WORD wID, int bGroupType)
{
	DWORD dwId = wID | (bGroupType & 0x00FF0000);

	icq_lock l(servlistMutex);

	if (pdwServerIDList)
	{
		for (int i = 0; i<nServerIDListCount; i++)
		{
			if ((pdwServerIDList[i] & 0x00FFFFFF) == dwId)
			{ // we found it, so remove
				for (int j = i+1; j<nServerIDListCount; j++)
					pdwServerIDList[j-1] = pdwServerIDList[j];

				nServerIDListCount--;
			}
		}
	}
}


// Returns true if dwID is reserved
BOOL CIcqProto::CheckServerID(WORD wID, unsigned int wCount)
{
	icq_lock l(servlistMutex);

	if (pdwServerIDList)
	{
		for (int i = 0; i<nServerIDListCount; i++)
		{
			if (((pdwServerIDList[i] & 0xFFFF) >= wID) && ((pdwServerIDList[i] & 0xFFFF) <= wID + wCount))
				return TRUE;
		}
	}

	return FALSE;
}

void CIcqProto::FlushServerIDs()
{
	icq_lock l(servlistMutex);

	SAFE_FREE((void**)&pdwServerIDList);
	nServerIDListCount = 0;
	nServerIDListSize = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct GroupReserveIdsEnumParam
{
	CIcqProto *ppro;
	char *szModule;
};

static int GroupReserveIdsEnumProc(const char *szSetting,LPARAM lParam)
{ 
	if (szSetting && strlennull(szSetting)<5)
	{ 
		// it is probably server group
		GroupReserveIdsEnumParam *param = (GroupReserveIdsEnumParam*)lParam;
		char val[MAX_PATH+2]; // dummy
		if (db_get_static(0, param->szModule, szSetting, val, MAX_PATH))
			if (db_get_static_utf(0, param->szModule, szSetting, val, MAX_PATH))
				return 0; // we failed also, invalid setting

		param->ppro->ReserveServerID((WORD)strtoul(szSetting, NULL, 0x10), SSIT_GROUP, 0);
#ifdef _DEBUG
		param->ppro->debugLogA("Loaded group %u:'%s'", strtoul(szSetting, NULL, 0x10), val);
#endif
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load all known server IDs from DB to list
void CIcqProto::LoadServerIDs()
{
	WORD wSrvID;
	int nGroups = 0, nContacts = 0, nPermits = 0, nDenys = 0, nIgnores = 0, nUnhandled = 0;

	servlistMutex->Enter();
	if (wSrvID = getWord(DBSETTING_SERVLIST_AVATAR, 0))
		ReserveServerID(wSrvID, SSIT_ITEM, 0);
	if (wSrvID = getWord(DBSETTING_SERVLIST_PHOTO, 0))
		ReserveServerID(wSrvID, SSIT_ITEM, 0);
	if (wSrvID = getWord(DBSETTING_SERVLIST_PRIVACY, 0))
		ReserveServerID(wSrvID, SSIT_ITEM, 0);
	if (wSrvID = getWord(DBSETTING_SERVLIST_METAINFO, 0))
		ReserveServerID(wSrvID, SSIT_ITEM, 0);
	if (wSrvID = getWord("SrvImportID", 0))
		ReserveServerID(wSrvID, SSIT_ITEM, 0);

	int nStart = nServerIDListCount;

	char szModule[MAX_PATH];
	mir_snprintf(szModule, SIZEOF(szModule), "%sSrvGroups", m_szModuleName);
	GroupReserveIdsEnumParam param = { this, szModule };

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = &GroupReserveIdsEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)&param;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

	nGroups = nServerIDListCount - nStart;

	MCONTACT hContact = db_find_first(m_szModuleName);

	while (hContact)
	{ // search all our contacts, reserve their server IDs
		if (wSrvID = getWord(hContact, DBSETTING_SERVLIST_ID, 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM, 0);
			nContacts++;
		}
		if (wSrvID = getWord(hContact, DBSETTING_SERVLIST_DENY, 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM, 0);
			nDenys++;
		}
		if (wSrvID = getWord(hContact, DBSETTING_SERVLIST_PERMIT, 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM, 0);
			nPermits++;
		}
		if (wSrvID = getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0))
		{
			ReserveServerID(wSrvID, SSIT_ITEM, 0);
			nIgnores++;
		}

		hContact = db_find_next(hContact, m_szModuleName);
	}
	servlistMutex->Leave();

	DBVARIANT dbv = {0};
	if (!getSetting(NULL, DBSETTING_SERVLIST_UNHANDLED, &dbv))
	{
		int dataLen = dbv.cpbVal;
		BYTE *pData = dbv.pbVal;

		while (dataLen >= 4)
		{
			BYTE bGroupType;
			BYTE bFlags;

			unpackLEWord(&pData, &wSrvID);
			unpackByte(&pData, &bGroupType);
			unpackByte(&pData, &bFlags);

			ReserveServerID(wSrvID, bGroupType, bFlags); 
			dataLen -= 4;
			nUnhandled++;
		}

		db_free(&dbv);
	}

	debugLogA("Loaded SSI: %d contacts, %d groups, %d permit, %d deny, %d ignore, %d unknown items.", nContacts, nGroups, nPermits, nDenys, nIgnores, nUnhandled);
}


void CIcqProto::StoreServerIDs() /// TODO: allow delayed
{
	BYTE *pUnhandled = NULL;
	int cbUnhandled = 0;

	servlistMutex->Enter();
	if (pdwServerIDList)
		for (int i = 0; i<nServerIDListCount; i++)
			if ((pdwServerIDList[i] & 0xFF000000) == SSIF_UNHANDLED)
			{
				ppackLEWord(&pUnhandled, &cbUnhandled, pdwServerIDList[i] & 0xFFFF);
				ppackByte(&pUnhandled, &cbUnhandled, (pdwServerIDList[i] & 0x00FF0000) >> 0x10);
				ppackByte(&pUnhandled, &cbUnhandled, (pdwServerIDList[i] & 0xFF000000) >> 0x18);
			}
			servlistMutex->Leave();

			if (pUnhandled)
				setSettingBlob(NULL, DBSETTING_SERVLIST_UNHANDLED, pUnhandled, cbUnhandled);
			else
				delSetting(DBSETTING_SERVLIST_UNHANDLED);

			SAFE_FREE((void**)&pUnhandled);
}


// Generate server ID with wCount IDs free after it, for sub-groups.
WORD CIcqProto::GenerateServerID(int bGroupType, int bFlags, int wCount)
{
	WORD wId;

	while (TRUE)
	{
		// Randomize a new ID
		// Max value is probably 0x7FFF, lowest value is probably 0x0001 (generated by Icq2Go)
		// We use range 0x1000-0x7FFF.
		wId = (WORD)RandRange(0x1000, 0x7FFF);

		if (!CheckServerID(wId, wCount))
			break;
	}

	ReserveServerID(wId, bGroupType, bFlags);

	return wId;
}


/***********************************************
*
*  --- Low-level packet sending functions ---
*
*/

struct doubleServerItemObject
{
	WORD wAction;
	icq_packet packet;
};

DWORD CIcqProto::icq_sendServerItem(DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wItemId, const char *szName, BYTE *pTLVs, int nTlvLength, WORD wItemType, DWORD dwOperation, DWORD dwTimeout, void **doubleObject)
{ // generic packet
	icq_packet packet;
	int nNameLen;
	WORD wTLVlen = (WORD)nTlvLength;

	// Prepare item name length
	nNameLen = strlennull(szName);

	// Build the packet
	serverPacketInit(&packet, (WORD)(nNameLen + 20 + wTLVlen));
	packFNACHeader(&packet, ICQ_LISTS_FAMILY, wAction, 0, dwCookie);
	packWord(&packet, (WORD)nNameLen);
	if (nNameLen) 
		packBuffer(&packet, (LPBYTE)szName, (WORD)nNameLen);
	packWord(&packet, wGroupId);
	packWord(&packet, wItemId);   
	packWord(&packet, wItemType); 
	packWord(&packet, wTLVlen);
	if (wTLVlen)
		packBuffer(&packet, pTLVs, wTLVlen);

	if (!doubleObject)
	{ // Send the packet and return the cookie
		servlistPostPacket(&packet, dwCookie, dwOperation | wAction, dwTimeout);
	}
	else
	{
		if (*doubleObject)
		{ // Send both packets and return the cookie
			doubleServerItemObject* helper = (doubleServerItemObject*)*doubleObject;

			servlistPostPacketDouble(&helper->packet, dwCookie, dwOperation | helper->wAction, dwTimeout, &packet, wAction);
			SAFE_FREE(doubleObject);
		}
		else
		{ // Create helper object, return the cookie
			doubleServerItemObject* helper = (doubleServerItemObject*)SAFE_MALLOC(sizeof(doubleServerItemObject));

			if (helper)
			{
				helper->wAction = wAction;
				memcpy(&helper->packet, &packet, sizeof(icq_packet));
				*doubleObject = helper;
			}
			else // memory alloc failed
				return 0;
		}
	}

	// Force reload of server-list after change
	setWord("SrvRecordCount", 0);

	return dwCookie;
}


DWORD CIcqProto::icq_sendServerContact(MCONTACT hContact, DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wContactId, DWORD dwOperation, DWORD dwTimeout, void **doubleObject)
{
	DWORD dwUin;
	uid_str szUid;
	icq_packet pBuffer;
	char *szNick = NULL, *szNote = NULL;
	BYTE *pData = NULL, *pMetaToken = NULL, *pMetaTime = NULL;
	int nNickLen, nNoteLen, nDataLen = 0, nMetaTokenLen = 0, nMetaTimeLen = 0;
	WORD wTLVlen;
	BYTE bAuth;
	int bDataTooLong = FALSE;

	// Prepare UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		debugLogA("Buddy upload failed (UID missing).");
		return 0;
	}

	bAuth = getByte(hContact, "Auth", 0);
	szNick = getSettingStringUtf(hContact, "CList", "MyHandle", NULL);
	szNote = getSettingStringUtf(hContact, "UserInfo", "MyNotes", NULL);

	DBVARIANT dbv;

	if (!getSetting(hContact, DBSETTING_METAINFO_TOKEN, &dbv))
	{
		nMetaTokenLen = dbv.cpbVal;
		pMetaToken = (BYTE*)_alloca(dbv.cpbVal);
		memcpy(pMetaToken, dbv.pbVal, dbv.cpbVal);

		db_free(&dbv);
	}
	if (!getSetting(hContact, DBSETTING_METAINFO_TIME, &dbv))
	{
		nMetaTimeLen = dbv.cpbVal;
		pMetaTime = (BYTE*)_alloca(dbv.cpbVal);
		for (int i = 0; i < dbv.cpbVal; i++)
			pMetaTime[i] = dbv.pbVal[dbv.cpbVal - i - 1];

		db_free(&dbv);
	}

	if (!getSetting(hContact, DBSETTING_SERVLIST_DATA, &dbv))
	{ // read additional server item data
		nDataLen = dbv.cpbVal;
		pData = (BYTE*)_alloca(nDataLen);
		memcpy(pData, dbv.pbVal, nDataLen);

		db_free(&dbv);
	}

	nNickLen = strlennull(szNick);
	nNoteLen = strlennull(szNote);

	// Limit the strings
	if (nNickLen > MAX_SSI_TLV_NAME_SIZE)
	{
		bDataTooLong = TRUE;
		nNickLen = null_strcut(szNick, MAX_SSI_TLV_NAME_SIZE);
	}
	if (nNoteLen > MAX_SSI_TLV_COMMENT_SIZE)
	{
		bDataTooLong = TRUE;
		nNoteLen = null_strcut(szNote, MAX_SSI_TLV_COMMENT_SIZE);
	}
	if (bDataTooLong)
	{ // Inform the user
		/// TODO: do something with this for Manage Server-List dialog.
		if (wAction != ICQ_LISTS_REMOVEFROMLIST) // do not report this when removing from list
			icq_LogMessage(LOG_WARNING, LPGEN("The contact's information was too big and was truncated."));
	}

	// Build the packet
	wTLVlen = (nNickLen?4+nNickLen:0) + (nNoteLen?4+nNoteLen:0) + (bAuth?4:0) + nDataLen + (nMetaTokenLen?4+nMetaTokenLen:0) + (nMetaTimeLen?4+nMetaTimeLen:0);

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wTLVlen);
	pBuffer.wLen = wTLVlen;

	if (nNickLen)
		packTLV(&pBuffer, SSI_TLV_NAME, (WORD)nNickLen, (LPBYTE)szNick);  // Nickname TLV

	if (nNoteLen)
		packTLV(&pBuffer, SSI_TLV_COMMENT, (WORD)nNoteLen, (LPBYTE)szNote);  // Comment TLV

	if (nMetaTokenLen)
		packTLV(&pBuffer, SSI_TLV_METAINFO_TOKEN, (WORD)nMetaTokenLen, pMetaToken);

	if (nMetaTimeLen)
		packTLV(&pBuffer, SSI_TLV_METAINFO_TIME, (WORD)nMetaTimeLen, pMetaTime);

	if (pData)
		packBuffer(&pBuffer, pData, (WORD)nDataLen);

	if (bAuth) // icq5 gives this as last TLV
		packDWord(&pBuffer, 0x00660000);  // "Still waiting for auth" TLV

	SAFE_FREE((void**)&szNick);
	SAFE_FREE((void**)&szNote);

	return icq_sendServerItem(dwCookie, wAction, wGroupId, wContactId, strUID(dwUin, szUid), pBuffer.pData, wTLVlen, SSI_ITEM_BUDDY, dwOperation, dwTimeout, doubleObject);
}


DWORD CIcqProto::icq_sendSimpleItem(DWORD dwCookie, WORD wAction, DWORD dwUin, char* szUID, WORD wGroupId, WORD wItemId, WORD wItemType, DWORD dwOperation, DWORD dwTimeout)
{ // for privacy items
	return icq_sendServerItem(dwCookie, wAction, wGroupId, wItemId, strUID(dwUin, szUID), NULL, 0, wItemType, dwOperation, dwTimeout, NULL);
}


DWORD CIcqProto::icq_sendServerGroup(DWORD dwCookie, WORD wAction, WORD wGroupId, const char *szName, void *pContent, int cbContent, DWORD dwOperationFlags)
{
	WORD wTLVlen;
	icq_packet pBuffer; // I reuse the ICQ packet type as a generic buffer
	// I should be ashamed! ;)

	if (strlennull(szName) == 0 && wGroupId != 0)
	{
		debugLogA("Group upload failed (GroupName missing).");
		return 0; // without name we could not change the group
	}

	// Calculate buffer size
	wTLVlen = (cbContent?4+cbContent:0);

	// Initialize our handy data buffer
	pBuffer.wPlace = 0;
	pBuffer.pData = (BYTE *)_alloca(wTLVlen);
	pBuffer.wLen = wTLVlen;

	if (wTLVlen)
		packTLV(&pBuffer, SSI_TLV_SUBITEMS, (WORD)cbContent, (LPBYTE)pContent);  // Groups TLV

	return icq_sendServerItem(dwCookie, wAction, wGroupId, 0, szName, pBuffer.pData, wTLVlen, SSI_ITEM_GROUP, SSOP_GROUP_ACTION | dwOperationFlags, 400, NULL);
}


DWORD CIcqProto::icq_modifyServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wAction, DWORD dwOperation, WORD wItemId, WORD wType)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
	DWORD dwCookie;

	if (ack)
	{
		ack->dwAction = dwOperation; // remove privacy item
		ack->hContact = hContact;
		ack->wContactId = wItemId;

		dwCookie = AllocateCookie(CKT_SERVERLIST, wAction, hContact, ack);
	}
	else // cookie failed
		return 0;

	return icq_sendSimpleItem(dwCookie, wAction, dwUin, szUid, 0, wItemId, wType, SSOP_ITEM_ACTION, 400);
}


DWORD CIcqProto::icq_removeServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType)
{
	return icq_modifyServerPrivacyItem(hContact, dwUin, szUid, ICQ_LISTS_REMOVEFROMLIST, SSA_PRIVACY_REMOVE, wItemId, wType);
}


DWORD CIcqProto::icq_addServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType)
{
	return icq_modifyServerPrivacyItem(hContact, dwUin, szUid, ICQ_LISTS_ADDTOLIST, SSA_PRIVACY_ADD, wItemId, wType);
}

/*****************************************
*
*     --- Contact DB Utilities ---
*
*/


/// TODO: do not check by plugin version, check by ServListStructures version!
int CIcqProto::IsServerGroupsDefined()
{
	int iRes = 1;

	if (getDword("Version", 0) < 0x00030608)
	{ // group cache & linking data too old, flush, reload from server
		char szModule[MAX_PATH];

		// flush obsolete linking data
		mir_snprintf(szModule, SIZEOF(szModule), "%sGroups", m_szModuleName);
		CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szModule);

		iRes = 0; // no groups defined, or older version
	}
	// store our current version
	setDword("Version", ICQ_PLUG_VERSION & 0x00FFFFFF);

	return iRes;
}


void CIcqProto::FlushSrvGroupsCache()
{
	char szModule[MAX_PATH];

	mir_snprintf(szModule, SIZEOF(szModule), "%sSrvGroups", m_szModuleName);
	CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szModule);
}


// Look thru DB and collect all ContactIDs from a group
void* CIcqProto::collectBuddyGroup(WORD wGroupID, int *count)
{
	WORD* buf = NULL;
	int cnt = 0;
	MCONTACT hContact;
	WORD wItemID;

	hContact = db_find_first(m_szModuleName);

	while (hContact)
	{ // search all contacts
		if (wGroupID == getWord(hContact, DBSETTING_SERVLIST_GROUP, 0))
		{ // add only buddys from specified group
			wItemID = getWord(hContact, DBSETTING_SERVLIST_ID, 0);

			if (wItemID)
			{ // valid ID, add
				cnt++;
				buf = (WORD*)SAFE_REALLOC(buf, cnt*sizeof(WORD));
				buf[cnt-1] = wItemID;
				if (!count) break;
			}
		}

		hContact = db_find_next(hContact, m_szModuleName);
	}

	if (count)
		*count = cnt<<1; // we return size in bytes
	return buf;
}


// Look thru DB and collect all GroupIDs
void* CIcqProto::collectGroups(int *count)
{
	WORD* buf = NULL;
	int cnt = 0;
	int i;
	MCONTACT hContact;
	WORD wGroupID;

	hContact = db_find_first(m_szModuleName);

	while (hContact)
	{ // search all contacts
		if (wGroupID = getWord(hContact, DBSETTING_SERVLIST_GROUP, 0))
		{ // add only valid IDs
			for (i = 0; i<cnt; i++)
			{ // check for already added ids
				if (buf[i] == wGroupID) break;
			}

			if (i == cnt)
			{ // not preset, add
				cnt++;
				buf = (WORD*)SAFE_REALLOC(buf, cnt*sizeof(WORD));
				buf[i] = wGroupID;
			}
		}

		hContact = db_find_next(hContact, m_szModuleName);
	}

	*count = cnt<<1;
	return buf;
}


static int GroupLinksEnumProc(const char *szSetting,LPARAM lParam)
{
	// check link target, add if match
	if (db_get_w(NULL, ((char**)lParam)[2], szSetting, 0) == (WORD)((char**)lParam)[1])
	{
		char** block = (char**)SAFE_MALLOC(2*sizeof(char*));
		block[1] = null_strdup(szSetting);
		block[0] = ((char**)lParam)[0];
		((char**)lParam)[0] = (char*)block;
	}
	return 0;
}

void CIcqProto::removeGroupPathLinks(WORD wGroupID)
{ // remove miranda grouppath links targeting to this groupid
	char szModule[MAX_PATH];
	char* pars[3];

	mir_snprintf(szModule, SIZEOF(szModule), "%sGroups", m_szModuleName);

	pars[0] = NULL;
	pars[1] = (char*)wGroupID;
	pars[2] = szModule;

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = &GroupLinksEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)pars;

	if (!CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces))
	{ // we found some links, remove them
		char** list = (char**)pars[0];
		while (list)
		{
			void* bet;

			db_unset(NULL, szModule, list[1]);
			SAFE_FREE((void**)&list[1]);
			bet = list;
			list = (char**)list[0];
			SAFE_FREE((void**)&bet);
		}
	}
}


char *CIcqProto::getServListGroupName(WORD wGroupID)
{
	char szModule[MAX_PATH];
	char szGroup[16];

	if (!wGroupID)
	{
		debugLogA("Warning: Cannot get group name (Group ID missing)!");
		return NULL;
	}

	mir_snprintf(szModule, SIZEOF(szModule), "%sSrvGroups", m_szModuleName);
	_itoa(wGroupID, szGroup, 0x10);

	if (!CheckServerID(wGroupID, 0))
	{ // check if valid id, if not give empty and remove
		debugLogA("Removing group %u from cache...", wGroupID);
		db_unset(NULL, szModule, szGroup);
		return NULL;
	}

	return getSettingStringUtf(NULL, szModule, szGroup, NULL);
}


void CIcqProto::setServListGroupName(WORD wGroupID, const char *szGroupName)
{
	char szModule[MAX_PATH];
	char szGroup[16];

	if (!wGroupID)
	{
		debugLogA("Warning: Cannot set group name (Group ID missing)!");
		return;
	}

	mir_snprintf(szModule, SIZEOF(szModule), "%sSrvGroups", m_szModuleName);
	_itoa(wGroupID, szGroup, 0x10);

	if (szGroupName)
		db_set_utf(NULL, szModule, szGroup, szGroupName);
	else
	{
		db_unset(NULL, szModule, szGroup);
		removeGroupPathLinks(wGroupID);
	}
	return;
}


WORD CIcqProto::getServListGroupLinkID(const char *szPath)
{
	char szModule[MAX_PATH];
	WORD wGroupId;

	mir_snprintf(szModule, SIZEOF(szModule), "%sGroups", m_szModuleName);

	wGroupId = db_get_w(NULL, szModule, szPath, 0);

	if (wGroupId && !CheckServerID(wGroupId, 0))
	{ // known, check if still valid, if not remove
		debugLogA("Removing group \"%s\" from cache...", szPath);
		db_unset(NULL, szModule, szPath);
		wGroupId = 0;
	}

	return wGroupId;
}


void CIcqProto::setServListGroupLinkID(const char *szPath, WORD wGroupID)
{
	char szModule[MAX_PATH];

	mir_snprintf(szModule, SIZEOF(szModule), "%sGroups", m_szModuleName);

	if (wGroupID)
		db_set_w(NULL, szModule, szPath, wGroupID);
	else
		db_unset(NULL, szModule, szPath);
}

// determine if the specified clist group path exists
//!! this function is not thread-safe due to the use of cli->pfnGetGroupName()
int CIcqProto::getCListGroupExists(const char *szGroup)
{
	if (!szGroup)
		return 0;

	int size = strlennull(szGroup) + 2;
	TCHAR *tszGroup = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(szGroup, tszGroup, size))
		for (int i = 1; TRUE; i++) {
			TCHAR *tszGroupName = (TCHAR*)pcli->pfnGetGroupName(i, NULL);
			if (!tszGroupName)
				break;

			// we have found the group
			if (!_tcscmp(tszGroup, tszGroupName))
				return i;
		}

	return 0;
}


int CIcqProto::moveContactToCListGroup(MCONTACT hContact, const char *szGroup)
{
	HANDLE hGroup = Clist_CreateGroup(0, ptrT( mir_utf8decodeT(szGroup)));

	if (ServiceExists(MS_CLIST_CONTACTCHANGEGROUP))
		return CallService(MS_CLIST_CONTACTCHANGEGROUP, hContact, (LPARAM)hGroup);
	else /// TODO: is this neccessary ?
		return db_set_utf(hContact, "CList", "Group", szGroup);
}


// utility function which counts > on start of a server group name
static int countGroupNameLevel(const char *szGroupName)
{
	int nNameLen = strlennull(szGroupName);
	int i = 0;

	while (i<nNameLen)
	{
		if (szGroupName[i] != '>')
			return i;

		i++;
	}
	return -1;
}

static int countCListGroupLevel(const char *szClistName)
{
	int nNameLen = strlennull(szClistName);
	int i, level = 0;

	for (i = 0; i < nNameLen; i++)
		if (szClistName[i] == '\\') level++;

	return level;
}

int CIcqProto::getServListGroupLevel(WORD wGroupId)
{
	char *szGroupName = getServListGroupName(wGroupId);
	int cnt = -1;

	if (szGroupName)
	{ // groupid is valid count group name level
		if (m_bSsiSimpleGroups)
			cnt = countCListGroupLevel(szGroupName);
		else
			cnt = countGroupNameLevel(szGroupName);

		SAFE_FREE((void**)&szGroupName);
	}

	return cnt;
}


// demangle group path
char *CIcqProto::getServListGroupCListPath(WORD wGroupId)
{
	char *szGroup = NULL;

	if (szGroup = getServListGroupName(wGroupId))
	{ // this groupid is valid
		if (!m_bSsiSimpleGroups)
			while (strstrnull(szGroup, "\\")) *strstrnull(szGroup, "\\") = '_'; // remove invalid char

		if (getServListGroupLinkID(szGroup) == wGroupId)
		{ // this grouppath is known and is for this group, set user group
			return szGroup;
		}
		else if (m_bSsiSimpleGroups)
		{ // with simple groups it is mapped 1:1, give real serv-list group name
			setServListGroupLinkID(szGroup, wGroupId);
			return szGroup;
		}
		else
		{ // advanced groups, determine group level
			int nGroupLevel = getServListGroupLevel(wGroupId);

			if (nGroupLevel > 0)
			{ // it is probably a sub-group locate parent group
				WORD wParentGroupId = wGroupId;
				int nParentGroupLevel;

				do
				{ // we look for parent group at the correct level
					wParentGroupId--;
					nParentGroupLevel = getServListGroupLevel(wParentGroupId);
				} while ((nParentGroupLevel >= nGroupLevel) && (nParentGroupLevel != -1));

				if (nParentGroupLevel == -1)
				{ // that was not a sub-group, it was just a group starting with >
					setServListGroupLinkID(szGroup, wGroupId);
					return szGroup;
				}

				{ // recursively determine parent group clist path
					char *szParentGroup = getServListGroupCListPath(wParentGroupId);

					/// FIXME: properly handle ~N suffixes
					szParentGroup = (char*)SAFE_REALLOC(szParentGroup, strlennull(szGroup) + strlennull(szParentGroup) + 2);
					strcat(szParentGroup, "\\");
					strcat(szParentGroup, (char*)szGroup + nGroupLevel);
					/*          if (strstrnull(szGroup, "~"))
					{ // check if the ~ was not added to obtain unique servlist item name
					char *szUniqueMark = strrchr(szParentGroup, '~');

					*szUniqueMark = '\0';
					// not the same group without ~, return it
					if (getServListGroupLinkID(szParentGroup) != wGroupId)
					*szUniqueMark = '~';
					} */ /// FIXME: this is necessary, but needs group loading changes
					SAFE_FREE((void**)&szGroup);
					szGroup = szParentGroup;


					if (getServListGroupLinkID(szGroup) == wGroupId)
					{ // known path, give
						return szGroup;
					}
					else
					{ // unknown path, setup a link
						setServListGroupLinkID(szGroup, wGroupId);
						return szGroup;
					}
				}
			}
			else
			{ // normal group, setup a link
				setServListGroupLinkID(szGroup, wGroupId);
				return szGroup;
			}
		}
	}
	return NULL;
}


static int SrvGroupNamesEnumProc(const char *szSetting, LPARAM lParam)
{ // check server-group cache item
	const char **params = (const char**)lParam;
	CIcqProto *ppro = (CIcqProto*)params[0];
	char *szGroupName = ppro->getSettingStringUtf(NULL, params[3], szSetting, NULL);

	if (!strcmpnull(szGroupName, params[2]))
		params[1] = szSetting; // do not need the real value, just arbitrary non-NULL

	SAFE_FREE(&szGroupName);
	return 0;
}

char* CIcqProto::getServListUniqueGroupName(const char *szGroupName, int bAlloced)
{ // enum ICQSrvGroups and create unique name if neccessary
	DBCONTACTENUMSETTINGS dbces;
	char szModule[MAX_PATH];
	char *pars[4];
	int uniqueID = 1;
	char *szGroupNameBase = (char*)szGroupName;
	char *szNewGroupName = NULL;

	if (!bAlloced)
		szGroupNameBase = null_strdup(szGroupName);
	null_strcut(szGroupNameBase, m_wServerListRecordNameMaxLength);

	mir_snprintf(szModule, SIZEOF(szModule), "%sSrvGroups", m_szModuleName);

	do {
		pars[0] = (char*)this;
		pars[1] = NULL;
		pars[2] = szNewGroupName ? szNewGroupName : szGroupNameBase;
		pars[3] = szModule;

		dbces.pfnEnumProc = &SrvGroupNamesEnumProc;
		dbces.szModule = szModule;
		dbces.lParam = (LPARAM)pars;
		CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

		if (pars[1])
		{ // the groupname already exists, create another
			SAFE_FREE((void**)&szNewGroupName);

			char szUnique[10];
			_itoa(uniqueID++, szUnique, 10);
			null_strcut(szGroupNameBase, m_wServerListRecordNameMaxLength - strlennull(szUnique) - 1);
			szNewGroupName = (char*)SAFE_MALLOC(strlennull(szUnique) + strlennull(szGroupNameBase) + 2);
			if (szNewGroupName)
			{
				strcpy(szNewGroupName, szGroupNameBase);
				strcat(szNewGroupName, "~");
				strcat(szNewGroupName, szUnique);
			}
		}
	} while (pars[1] && szNewGroupName);

	if (szNewGroupName)
	{
		SAFE_FREE(&szGroupNameBase);
		return szNewGroupName;
	}
	if (szGroupName != szGroupNameBase)
	{
		SAFE_FREE(&szGroupNameBase);
		return (char*)szGroupName;
	}
	else
		return szGroupNameBase;
}


// this is the second part of recursive event-driven procedure
int CIcqProto::servlistCreateGroup_gotParentGroup(const char *szGroup, WORD wGroupID, LPARAM param, int nResult)
{
	cookie_servlist_action* clue = (cookie_servlist_action*)param;
	char *szSubGroupName = clue->szGroupName;
	char *szSubGroup;
	int wSubGroupLevel = -1;
	WORD wSubGroupID;

	SAFE_FREE((void**)&clue);

	if (nResult == PENDING_RESULT_PURGE)
	{ // only cleanup
		return CALLBACK_RESULT_CONTINUE;
	}

	szSubGroup = (char*)SAFE_MALLOC(strlennull(szGroup) + strlennull(szSubGroupName) + 2);
	if (szSubGroup)
	{
		strcpy(szSubGroup, szGroup);
		strcat(szSubGroup, "\\");
		strcat(szSubGroup, szSubGroupName);
	}

	if (nResult == PENDING_RESULT_SUCCESS) // if we got an id count level
		wSubGroupLevel = getServListGroupLevel(wGroupID);

	if (wSubGroupLevel == -1)
	{ // something went wrong, give the id and go away
		servlistPendingRemoveGroup(szSubGroup, wGroupID, PENDING_RESULT_FAILED);

		SAFE_FREE((void**)&szSubGroupName);
		SAFE_FREE((void**)&szSubGroup);
		return CALLBACK_RESULT_CONTINUE;
	}
	wSubGroupLevel++; // we are a sub-group
	wSubGroupID = wGroupID + 1;

	// check if on that id is not group of the same or greater level, if yes, try next
	while (CheckServerID(wSubGroupID, 0) && (getServListGroupLevel(wSubGroupID) >= wSubGroupLevel))
	{
		wSubGroupID++;
	}

	if (!CheckServerID(wSubGroupID, 0))
	{ // the next id is free, so create our group with that id
		cookie_servlist_action *ack;
		DWORD dwCookie;
		char *szSubGroupItem = (char*)SAFE_MALLOC(strlennull(szSubGroupName) + wSubGroupLevel + 1);

		if (szSubGroupItem)
		{
			int i;

			for (i=0; i < wSubGroupLevel; i++)
				szSubGroupItem[i] = '>';

			strcpy(szSubGroupItem + wSubGroupLevel, szSubGroupName);
			szSubGroupItem[strlennull(szSubGroupName) + wSubGroupLevel] = '\0';
			SAFE_FREE((void**)&szSubGroupName);
			// check and create unique group name (Miranda does allow more subgroups with the same name!)
			szSubGroupItem = getServListUniqueGroupName(szSubGroupItem, TRUE);

			if (ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action)))
			{ // we have cookie good, go on
#ifdef _DEBUG
				debugLogA("Server-List: Creating sub-group \"%s\", parent group \"%s\".", szSubGroupItem, szGroup);
#endif
				ReserveServerID(wSubGroupID, SSIT_GROUP, 0);

				ack->wGroupId = wSubGroupID;
				ack->szGroupName = szSubGroupItem; // we need that name
				ack->szGroup = szSubGroup;
				ack->dwAction = SSA_GROUP_ADD;
				dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, 0, ack);

				icq_sendServerGroup(dwCookie, ICQ_LISTS_ADDTOLIST, ack->wGroupId, szSubGroupItem, NULL, 0, SSOF_BEGIN_OPERATION);
				return CALLBACK_RESULT_CONTINUE;
			}
			SAFE_FREE((void**)&szSubGroupItem);
		}
	}
	// we failed to create sub-group give parent groupid
	icq_LogMessage(LOG_ERROR, LPGEN("Failed to create the correct sub-group, the using closest parent group."));

	servlistPendingRemoveGroup(szSubGroup, wGroupID, PENDING_RESULT_FAILED);

	SAFE_FREE((void**)&szSubGroupName);
	SAFE_FREE((void**)&szSubGroup);
	return CALLBACK_RESULT_CONTINUE;
}


int CIcqProto::servlistCreateGroup_Ready(const char *szGroup, WORD groupID, LPARAM param, int nResult)
{
	WORD wGroupID = 0;

	if (nResult == PENDING_RESULT_PURGE)
		return CALLBACK_RESULT_CONTINUE;

	if (wGroupID = getServListGroupLinkID(szGroup))
	{ // the path is known, continue the process
		servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_SUCCESS);
		return CALLBACK_RESULT_CONTINUE;
	}

	if (!strstrnull(szGroup, "\\") || m_bSsiSimpleGroups)
	{ // a root group can be simply created without problems; simple groups are mapped directly
		cookie_servlist_action* ack;
		DWORD dwCookie;

		if (ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action)))
		{ // we have cookie good, go on
#ifdef _DEBUG
			debugLogA("Server-List: Creating root group \"%s\".", szGroup);
#endif
			ack->wGroupId = GenerateServerID(SSIT_GROUP, 0);
			ack->szGroup = null_strdup(szGroup); // we need that name
			// check if the groupname is unique - just to be sure, Miranda should handle that!
			ack->szGroupName = getServListUniqueGroupName(ack->szGroup, FALSE);
			ack->dwAction = SSA_GROUP_ADD;
			dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, 0, ack);

			icq_sendServerGroup(dwCookie, ICQ_LISTS_ADDTOLIST, ack->wGroupId, ack->szGroup, NULL, 0, SSOF_BEGIN_OPERATION);

			return CALLBACK_RESULT_POSTPONE;
		}
	}
	else
	{ // this is a sub-group
		char* szSub = null_strdup(szGroup); // create subgroup, recursive, event-driven, possibly relocate
		cookie_servlist_action* ack;
		char *szLast;

		if (strstrnull(szSub, "\\"))
		{ // determine parent group
			szLast = strrchr(szSub, '\\') + 1;

			szLast[-1] = '\0'; 
		}
		// make parent group id
		ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
		if (ack)
		{
			ack->szGroupName = null_strdup(szLast); // groupname
			servlistCreateGroup(szSub, (LPARAM)ack, &CIcqProto::servlistCreateGroup_gotParentGroup);
			SAFE_FREE((void**)&szSub);

			return CALLBACK_RESULT_POSTPONE;
		}

		SAFE_FREE((void**)&szSub); 
	}
	servlistPendingRemoveGroup(szGroup, groupID, PENDING_RESULT_FAILED);

	return CALLBACK_RESULT_CONTINUE;
}


// create group with this path, a bit complex task
// this supposes that all server groups are known
void CIcqProto::servlistCreateGroup(const char *szGroupPath, LPARAM param, PENDING_GROUP_CALLBACK callback)
{
	char *szGroup = (char*)szGroupPath;

	if (!strlennull(szGroup)) szGroup = DEFAULT_SS_GROUP;

	servlistPendingAddGroup(szGroup, 0, 0, &CIcqProto::servlistCreateGroup_Ready, TRUE, param, callback);
}


/*****************************************
*
*    --- Server-List Operations ---
*
*/

int CIcqProto::servlistAddContact_gotGroup(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action* ack = (cookie_servlist_action*)lParam;

	if (ack) SAFE_FREE(&ack->szGroup);

	if (nResult == PENDING_RESULT_PURGE)
	{ // only cleanup
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	if (!ack || !wGroupID) // something went wrong
	{
		if (ack) servlistPendingRemoveContact(ack->hContact, 0, wGroupID, PENDING_RESULT_FAILED);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	WORD wItemID = getWord(ack->hContact, DBSETTING_SERVLIST_ID, 0);

	if (wItemID) /// TODO: redundant ???
	{ // Only add the contact if it doesnt already have an ID
		servlistPendingRemoveContact(ack->hContact, wItemID, wGroupID, PENDING_RESULT_SUCCESS);
		debugLogA("Failed to add contact to server side list (%s)", "already there");
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	wItemID = GenerateServerID(SSIT_ITEM, 0);

	ack->dwAction = SSA_CONTACT_ADD;
	ack->wGroupId = wGroupID;
	ack->wContactId = wItemID;

	DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, ack->hContact, ack);

	icq_sendServerContact(ack->hContact, dwCookie, ICQ_LISTS_ADDTOLIST, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT | SSOF_BEGIN_OPERATION, 400, NULL);

	return CALLBACK_RESULT_CONTINUE;
}


// Need to be called when Pending Contact is active
int CIcqProto::servlistAddContact_Ready(MCONTACT hContact, WORD wContactID, WORD wGroupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action* ack = (cookie_servlist_action*)lParam;

	if (nResult == PENDING_RESULT_PURGE)
	{ // removing obsolete items, just free the memory
		SAFE_FREE((void**)&ack->szGroup);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	WORD wItemID = getWord(ack->hContact, DBSETTING_SERVLIST_ID, 0);

	if (wItemID)
	{ // Only add the contact if it doesn't already have an ID
		servlistPendingRemoveContact(ack->hContact, wItemID, getWord(hContact, DBSETTING_SERVLIST_GROUP, 0), PENDING_RESULT_SUCCESS);
		debugLogA("Failed to add contact to server side list (%s)", "already there");
		SAFE_FREE((void**)&ack->szGroup);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	// obtain a correct groupid first
	servlistCreateGroup(ack->szGroup, lParam, &CIcqProto::servlistAddContact_gotGroup);

	return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be added to server list, if group does not exist, create one
void CIcqProto::servlistAddContact(MCONTACT hContact, const char *pszGroup)
{
	DWORD dwUin;
	uid_str szUid;
	cookie_servlist_action* ack;

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{ // Could not do anything without uid
		debugLogA("Failed to add contact to server side list (%s)", "no UID");
		return;
	}

	if (!(ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action))))
	{ // Could not do anything without cookie
		debugLogA("Failed to add contact to server side list (%s)", "malloc failed");
		return;
	}
	else
	{
		ack->hContact = hContact;
		ack->szGroup = null_strdup(pszGroup);
		// call thru pending operations - makes sure the contact is ready to be added
		servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistAddContact_Ready, TRUE);
		return;
	}
}


int CIcqProto::servlistRemoveContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
	WORD wGroupID;
	WORD wItemID;
	cookie_servlist_action* ack = (cookie_servlist_action*)lParam;
	DWORD dwCookie;

	if (nResult == PENDING_RESULT_PURGE)
	{
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	// Get the contact's group ID
	if (!(wGroupID = getWord(hContact, DBSETTING_SERVLIST_GROUP, 0)))
	{ // Could not find a usable group ID
		servlistPendingRemoveContact(hContact, contactID, groupID, PENDING_RESULT_FAILED);

		debugLogA("Failed to remove contact from server side list (%s)", "no group ID");
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	// Get the contact's item ID
	if (!(wItemID = getWord(hContact, DBSETTING_SERVLIST_ID, 0)))
	{ // Could not find usable item ID
		servlistPendingRemoveContact(hContact, contactID, wGroupID, PENDING_RESULT_FAILED);

		debugLogA("Failed to remove contact from server side list (%s)", "no item ID");
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	ack->dwAction = SSA_CONTACT_REMOVE;
	ack->hContact = hContact;
	ack->wGroupId = wGroupID;
	ack->wContactId = wItemID;

	dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, hContact, ack);

	icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT | SSOF_BEGIN_OPERATION, 400, NULL);

	return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be removed from server list, remove group if it remain empty
void CIcqProto::servlistRemoveContact(MCONTACT hContact)
{
	DWORD dwUin;
	uid_str szUid;
	cookie_servlist_action* ack;

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		// Could not do anything without uid
		debugLogA("Failed to remove contact from server side list (%s)", "no UID");
		return;
	}

	if (!(ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action))))
	{ // Could not do anything without cookie
		debugLogA("Failed to remove contact from server side list (%s)", "malloc failed");
		return;
	}
	else
	{
		ack->hContact = hContact;
		// call thru pending operations - makes sure the contact is ready to be removed
		servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistRemoveContact_Ready, TRUE);
		return;
	}
}


int CIcqProto::servlistMoveContact_gotTargetGroup(const char *szGroup, WORD wNewGroupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)lParam;

	if (ack) SAFE_FREE(&ack->szGroup);

	if (nResult == PENDING_RESULT_PURGE)
	{ // removing obsolete items, just free the memory
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	if (!ack || !wNewGroupID || !ack->hContact) // something went wrong
	{
		if (ack) servlistPendingRemoveContact(ack->hContact, 0, 0, PENDING_RESULT_FAILED);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	WORD wItemID = getWord(ack->hContact, DBSETTING_SERVLIST_ID, 0);
	WORD wGroupID = getWord(ack->hContact, DBSETTING_SERVLIST_GROUP, 0);

	if (!wItemID) 
	{ // We have no ID, so try to simply add the contact to serv-list 
		debugLogA("Unable to move contact (no ItemID) -> trying to add");
		// we know the GroupID, so directly call add
		return servlistAddContact_gotGroup(szGroup, wNewGroupID, lParam, nResult);
	}

	if (wGroupID == wNewGroupID)
	{ // Only move the contact if it had different GroupID
		servlistPendingRemoveContact(ack->hContact, wItemID, wNewGroupID, PENDING_RESULT_SUCCESS);
		debugLogA("Contact not moved to group on server side list (same Group)");
		return CALLBACK_RESULT_CONTINUE;
	}

	ack->szGroupName = NULL;
	ack->dwAction = SSA_CONTACT_SET_GROUP;
	ack->wGroupId = wGroupID;
	ack->wContactId = wItemID;
	ack->wNewContactId = GenerateServerID(SSIT_ITEM, 0); // icq5 recreates also this, imitate
	ack->wNewGroupId = wNewGroupID;
	ack->lParam = 0; // we use this as a sign

	DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, ack->hContact, ack);
	DWORD dwCookie2 = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_ADDTOLIST, ack->hContact, ack);

	{ // imitate icq5, previously here was different order, but AOL changed and it ceased to work
		void *doubleObject = NULL;

		icq_sendServerContact(ack->hContact, dwCookie2, ICQ_LISTS_ADDTOLIST, wNewGroupID, ack->wNewContactId, SSO_CONTACT_SETGROUP | SSOF_BEGIN_OPERATION, 500, &doubleObject);
		icq_sendServerContact(ack->hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupID, wItemID, SSO_CONTACT_SETGROUP | SSOF_BEGIN_OPERATION, 500, &doubleObject);
	}
	return CALLBACK_RESULT_CONTINUE;
}


int CIcqProto::servlistMoveContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)lParam;

	if (nResult == PENDING_RESULT_PURGE)
	{ // removing obsolete items, just free the memory
		SAFE_FREE(&ack->szGroup);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	WORD wItemID = getWord(ack->hContact, DBSETTING_SERVLIST_ID, 0);
	WORD wGroupID = getWord(ack->hContact, DBSETTING_SERVLIST_GROUP, 0);

	if (!wGroupID && wItemID)
	{ // Only move the contact if it had an GroupID
		servlistPendingRemoveContact(ack->hContact, contactID, groupID, PENDING_RESULT_FAILED);

		debugLogA("Failed to move contact to group on server side list (%s)", "no Group");
		SAFE_FREE(&ack->szGroup);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	// obtain a correct target groupid first
	servlistCreateGroup(ack->szGroup, lParam, &CIcqProto::servlistMoveContact_gotTargetGroup);

	return CALLBACK_RESULT_POSTPONE;
}


// Called when contact should be moved from one group to another, create new, remove empty
void CIcqProto::servlistMoveContact(MCONTACT hContact, const char *pszNewGroup)
{
	DWORD dwUin;
	uid_str szUid;

	if (!hContact) return; // we do not move us, caused our uin was wrongly added to list

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{ // Could not do anything without uin
		debugLogA("Failed to move contact to group on server side list (%s)", "no UID");
		return;
	}

	if ((pszNewGroup != NULL) && (pszNewGroup[0]!='\0') && !getCListGroupExists(pszNewGroup))
	{ // the contact moved to non existing group, do not do anything: MetaContact hack
		debugLogA("Contact not moved - probably hiding by MetaContacts.");
		return;
	}

	if (!getWord(hContact, DBSETTING_SERVLIST_ID, 0)) /// FIXME:::: this should be in _ready
	{ // the contact is not stored on the server, check if we should try to add
		if (!getByte("ServerAddRemove", DEFAULT_SS_ADDSERVER) ||
			db_get_b(hContact, "CList", "Hidden", 0))
			return;
	}
	cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

	if (!ack)
	{ // Could not do anything without cookie
		debugLogA("Failed to add contact to server side list (%s)", "malloc failed");
		return;
	}
	else
	{
		ack->hContact = hContact;
		ack->szGroup = null_strdup(pszNewGroup);
		// call thru pending operations - makes sure the contact is ready to be moved
		servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistMoveContact_Ready, TRUE);
		return;
	}
}


int CIcqProto::servlistUpdateContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)lParam;

	if (nResult == PENDING_RESULT_PURGE)
	{ // removing obsolete items, just free the memory
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}
	WORD wItemID;
	WORD wGroupID;

	// Get the contact's group ID
	if (!(wGroupID = getWord(hContact, DBSETTING_SERVLIST_GROUP, 0)))
	{
		servlistPendingRemoveContact(hContact, contactID, groupID, PENDING_RESULT_FAILED);
		// Could not find a usable group ID
		debugLogA("Failed to update contact's details on server side list (%s)", "no group ID");
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	// Get the contact's item ID
	if (!(wItemID = getWord(hContact, DBSETTING_SERVLIST_ID, 0)))
	{
		servlistPendingRemoveContact(hContact, contactID, wGroupID, PENDING_RESULT_FAILED);
		// Could not find usable item ID
		debugLogA("Failed to update contact's details on server side list (%s)", "no item ID");
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	ack->dwAction = SSA_CONTACT_UPDATE;
	ack->wContactId = wItemID;
	ack->wGroupId = wGroupID;
	ack->hContact = hContact;

	DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, hContact, ack);

	// There is no need to send ICQ_LISTS_CLI_MODIFYSTART or
	// ICQ_LISTS_CLI_MODIFYEND when just changing nick name
	icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_UPDATEGROUP, wGroupID, wItemID, SSOP_ITEM_ACTION | SSOF_CONTACT, 400, NULL);

	return CALLBACK_RESULT_POSTPONE;
}


// Is called when a contact' details has been changed locally to update
// the server side details.
void CIcqProto::servlistUpdateContact(MCONTACT hContact)
{
	DWORD dwUin;
	uid_str szUid;

	// Get UID
	if (getContactUid(hContact, &dwUin, &szUid))
	{
		// Could not set nickname on server without uid
		debugLogA("Failed to update contact's details on server side list (%s)", "no UID");
		return;
	}
	cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

	if (!ack)
	{
		// Could not allocate cookie - use old fake
		debugLogA("Failed to update contact's details on server side list (%s)", "malloc failed");
		return;
	}
	else
	{
		ack->hContact = hContact;
		// call thru pending operations - makes sure the contact is ready to be updated
		servlistPendingAddContact(hContact, 0, 0, (LPARAM)ack, &CIcqProto::servlistUpdateContact_Ready, TRUE);
		return;
	}
}


int CIcqProto::servlistRenameGroup_Ready(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)lParam;

	if (nResult == PENDING_RESULT_PURGE)
	{ // only cleanup
		if (ack) SAFE_FREE(&ack->szGroupName);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}

	if (!ack || !wGroupID) // something went wrong
	{
		servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_FAILED);

		if (ack) SAFE_FREE(&ack->szGroupName);
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}
	void *groupData;
	int groupSize;

	if (groupData = collectBuddyGroup(wGroupID, &groupSize))
	{
		ack->dwAction = SSA_GROUP_RENAME;
		ack->wGroupId = wGroupID;
		ack->szGroup = null_strdup(szGroup); // we need this name
		// check if the new name is unique, create unique groupname if necessary
		ack->szGroupName = getServListUniqueGroupName(ack->szGroupName, TRUE);

		DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);

		icq_sendServerGroup(dwCookie, ICQ_LISTS_UPDATEGROUP, wGroupID, ack->szGroupName, groupData, groupSize, 0);
		SAFE_FREE(&groupData);
	}
	return CALLBACK_RESULT_POSTPONE;
}


void CIcqProto::servlistRenameGroup(char *szGroup, WORD wGroupId, char *szNewGroup)
{
	char *szNewGroupName;
	int nGroupLevel = getServListGroupLevel(wGroupId);

	if (nGroupLevel == -1) return; // we failed to prepare group

	if (!m_bSsiSimpleGroups)
	{
		char *szGroupName = szGroup;
		int i = nGroupLevel;
		while (i)
		{ // find correct part of grouppath
			szGroupName = strstrnull(szGroupName, "\\");
			if (!szGroupName) return; // failed to get correct part of the grouppath
			szGroupName++;
			i--;
		}
		szNewGroupName = szNewGroup;
		i = nGroupLevel;
		while (i)
		{ // find correct part of new grouppath
			szNewGroupName = strstrnull(szNewGroupName, "\\");
			if (!szNewGroupName) return; // failed to get correct part of the new grouppath
			szNewGroupName++;
			i--;
		}
		// truncate possible sub-groups
		char *szLast = strstrnull(szGroupName, "\\");
		if (szLast)
			szLast[0] = '\0';
		szLast = strstrnull(szNewGroupName, "\\");
		if (szLast)
			szLast[0] = '\0';

		// this group was not changed, nothing to rename
		if (!strcmpnull(szGroupName, szNewGroupName)) return;

		szGroupName = szNewGroupName;
		szNewGroupName = (char*)SAFE_MALLOC(strlennull(szGroupName) + 1 + nGroupLevel);
		if (!szNewGroupName) return; // Failure

		for (i = 0; i < nGroupLevel; i++)
		{ // create level prefix
			szNewGroupName[i] = '>';
		}
		strcat(szNewGroupName, szGroupName);
	}
	else // simple groups do not require any conversion
		szNewGroupName = null_strdup(szNewGroup);

	cookie_servlist_action* ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
	if (!ack)
	{ // cookie failed
		debugLogA("Error: Failed to allocate cookie");

		SAFE_FREE(&szNewGroupName);
		return;
	}
	// store new group name for future use
	ack->szGroupName = szNewGroupName;
	// call thru pending operations - makes sure the group is ready for rename
	servlistPendingAddGroup(szGroup, wGroupId, (LPARAM)ack, &CIcqProto::servlistRenameGroup_Ready, TRUE);
}


int CIcqProto::servlistRemoveGroup_Ready(const char *szGroup, WORD groupID, LPARAM lParam, int nResult)
{
	cookie_servlist_action *ack = (cookie_servlist_action*)lParam;

	if (nResult == PENDING_RESULT_PURGE)
	{ // only cleanup
		SAFE_FREE((void**)&ack);
		return CALLBACK_RESULT_CONTINUE;
	}
	WORD wGroupID = getServListGroupLinkID(szGroup);
	char *szGroupName;

	if (wGroupID && (szGroupName = getServListGroupName(wGroupID)))
	{ // the group is valid, check if it is empty
		void *groupData = collectBuddyGroup(wGroupID, NULL);

		if (groupData)
		{ // the group is not empty, cannot delete
			SAFE_FREE(&groupData);
			SAFE_FREE(&szGroupName);
			// end operation
			servlistPendingRemoveGroup(szGroup, wGroupID, PENDING_RESULT_SUCCESS);
			// cleanup
			SAFE_FREE((void**)&ack);
			return CALLBACK_RESULT_CONTINUE;
		}

		if (!CheckServerID((WORD)(wGroupID+1), 0) || getServListGroupLevel((WORD)(wGroupID+1)) == 0)
		{ // is next id an sub-group, if yes, we cannot delete this group
			ack->dwAction = SSA_GROUP_REMOVE;
			ack->wContactId = 0;
			ack->wGroupId = wGroupID;
			ack->hContact = NULL;
			ack->szGroup = null_strdup(szGroup); // we need that name
			ack->szGroupName = szGroupName;
			DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_REMOVEFROMLIST, 0, ack);

			icq_sendServerGroup(dwCookie, ICQ_LISTS_REMOVEFROMLIST, ack->wGroupId, ack->szGroupName, NULL, 0, 0);
		}
		return CALLBACK_RESULT_POSTPONE;
	}
	// end operation
	servlistPendingRemoveGroup(szGroup, groupID, PENDING_RESULT_SUCCESS);
	// cleanup
	SAFE_FREE((void**)&ack);
	return CALLBACK_RESULT_CONTINUE;
}


void CIcqProto::servlistRemoveGroup(const char *szGroup, WORD wGroupId)
{
	if (!szGroup) return;

	cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

	if (!ack)
	{ // cookie failed
		debugLogA("Error: Failed to allocate cookie");
		return;
	}

	// call thru pending operations - makes sure the group is ready for removal
	servlistPendingAddGroup(szGroup, wGroupId, (LPARAM)ack, &CIcqProto::servlistRemoveGroup_Ready, TRUE);
}


/*void CIcqProto::servlistMoveGroup(const char *szGroup, WORD wNewGroupId)
{
// relocate the group
}*/


void CIcqProto::resetServContactAuthState(MCONTACT hContact, DWORD dwUin)
{
	WORD wContactId = getWord(hContact, DBSETTING_SERVLIST_ID, 0);
	WORD wGroupId = getWord(hContact, DBSETTING_SERVLIST_GROUP, 0);

	if (wContactId && wGroupId)
	{
		cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

		if (ack)
		{ // we have cookie good, go on
			ack->hContact = hContact;
			ack->wContactId = wContactId;
			ack->wGroupId = wGroupId;
			ack->dwAction = SSA_CONTACT_FIX_AUTH;

			DWORD dwCookie = AllocateCookie(CKT_SERVERLIST, 0, hContact, ack);

			{
				void *doubleObject = NULL;

				icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_REMOVEFROMLIST, wGroupId, wContactId, SSO_CONTACT_FIXAUTH | SSOF_BEGIN_OPERATION | SSOF_END_OPERATION, 200, &doubleObject);
				delSetting(hContact, DBSETTING_METAINFO_TOKEN);
				delSetting(hContact, DBSETTING_METAINFO_TIME);
				delSetting(hContact, DBSETTING_SERVLIST_DATA);
				icq_sendServerContact(hContact, dwCookie, ICQ_LISTS_ADDTOLIST, wGroupId, wContactId, SSO_CONTACT_FIXAUTH | SSOF_BEGIN_OPERATION | SSOF_END_OPERATION, 200, &doubleObject);
			}
		}
		else
			debugLogA("Error: Failed to allocate cookie");
	}
}

/*****************************************
*
*   --- Miranda Contactlist Hooks ---
*
*/

int CIcqProto::ServListDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;

	// TODO: Queue changes that occur while offline
	if (!icqOnline() || !m_bSsiEnabled || bIsSyncingCL)
		return 0;

#ifdef _DEBUG
	if (cws->value.type == DBVT_DELETED)
		debugLogA("DB-Events: Module \"%s\", setting \"%s\" deleted.", cws->szModule, cws->szSetting);
	else
		debugLogA("DB-Events: Module \"%s\", setting \"%s\" changed, data type %x.", cws->szModule, cws->szSetting, cws->value.type);
#endif

	if (!strcmpnull(cws->szModule, "CList"))
	{
		// Has contact been renamed?
		if (!strcmpnull(cws->szSetting, "MyHandle") && getByte("StoreServerDetails", DEFAULT_SS_STORE))
			servlistUpdateContact(hContact); // Update contact's details in server-list

		// Has contact been moved to another group?
		if (!strcmpnull(cws->szSetting, "Group") && getByte("StoreServerDetails", DEFAULT_SS_STORE)) {
			char* szNewGroup = getContactCListGroup(hContact); // Read group from DB
			SAFE_FREE(&szNewGroup);
		}
	}
	else if (!strcmpnull(cws->szModule, "UserInfo")) {
		// Update contact's details in server-list
		if (!strcmpnull(cws->szSetting, "MyNotes") && getByte("StoreServerDetails", DEFAULT_SS_STORE))
			servlistUpdateContact(hContact);
	}

	return 0;
}


int CIcqProto::ServListDbContactDeleted(WPARAM hContact, LPARAM lParam)
{
#ifdef _DEBUG
	debugLogA("DB-Events: Contact %x deleted.", hContact);
#endif

	DeleteFromContactsCache(hContact);

	if ( !icqOnline() && m_bSsiEnabled)
		// contact was deleted only locally - retrieve full list on next connect
		setWord(hContact, "SrvRecordCount", 0);

	if ( !icqOnline() || !m_bSsiEnabled)
		return 0;

	// we need all server contacts on local buddy list
	DWORD dwUIN;
	uid_str szUID;
	if (getContactUid(hContact, &dwUIN, &szUID))
		return 0;

	WORD wContactID = getWord(hContact, DBSETTING_SERVLIST_ID, 0);
	WORD wGroupID = getWord(hContact, DBSETTING_SERVLIST_GROUP, 0);
	WORD wVisibleID = getWord(hContact, DBSETTING_SERVLIST_PERMIT, 0);
	WORD wInvisibleID = getWord(hContact, DBSETTING_SERVLIST_DENY, 0);
	WORD wIgnoreID = getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0);

	// Remove from queue for user details request
	icq_DequeueUser(dwUIN);

	// Close all opened peer connections
	CloseContactDirectConns(hContact);

	if ((wGroupID && wContactID) || wVisibleID || wInvisibleID || wIgnoreID) {
		if (wContactID) // delete contact from server
			servlistRemoveContact(hContact);

		if (wVisibleID) // detete permit record
			icq_removeServerPrivacyItem(hContact, dwUIN, szUID, wVisibleID, SSI_ITEM_PERMIT);

		if (wInvisibleID) // delete deny record
			icq_removeServerPrivacyItem(hContact, dwUIN, szUID, wInvisibleID, SSI_ITEM_DENY);

		if (wIgnoreID) // delete ignore record
			icq_removeServerPrivacyItem(hContact, dwUIN, szUID, wIgnoreID, SSI_ITEM_IGNORE);
	}

	return 0;
}


int CIcqProto::ServListCListGroupChange(WPARAM hContact, LPARAM lParam)
{
	CLISTGROUPCHANGE *grpchg = (CLISTGROUPCHANGE*)lParam;

	if (!icqOnline() || !m_bSsiEnabled || bIsSyncingCL)
		return 0;

	// only change server-list if it is allowed
	if (!getByte("StoreServerDetails", DEFAULT_SS_STORE))
		return 0;


	if (hContact == NULL)
	{ // change made to group
		if (grpchg->pszNewName == NULL && grpchg->pszOldName != NULL)
		{ // group removed
			char *szOldName = tchar_to_utf8(grpchg->pszOldName);
			WORD wGroupId = getServListGroupLinkID(szOldName);

#ifdef _DEBUG
			debugLogA("CList-Events: Group %x:\"%s\" deleted.", wGroupId, szOldName);
#endif
			if (wGroupId)
			{ // the group is known, remove from server
				servlistPostPacket(NULL, 0, SSO_BEGIN_OPERATION, 100); // start server modifications here
				servlistRemoveGroup(szOldName, wGroupId);
			}
			SAFE_FREE(&szOldName);
		}
		else if (grpchg->pszNewName != NULL && grpchg->pszOldName != NULL)
		{ // group renamed
			char *szNewName = tchar_to_utf8(grpchg->pszNewName);
			char *szOldName = tchar_to_utf8(grpchg->pszOldName);
			WORD wGroupId = getServListGroupLinkID(szOldName);

#ifdef _DEBUG
			debugLogA("CList-Events: Group %x:\"%s\" changed to \"%s\".", wGroupId, szOldName, szNewName);
#endif
			if (wGroupId)
			{ // group is known, rename on server
				servlistRenameGroup(szOldName, wGroupId, szNewName);
			}
			SAFE_FREE(&szOldName);
			SAFE_FREE(&szNewName);
		}
	}
	else
	{ // change to contact
		if (IsICQContact(hContact))
		{ // our contact, fine move on the server as well
			char *szNewName = grpchg->pszNewName ? tchar_to_utf8(grpchg->pszNewName) : NULL;

#ifdef _DEBUG
			debugLogA("CList-Events: Contact %x moved to group \"%s\".", hContact, szNewName);
#endif
			servlistMoveContact(hContact, szNewName);
			SAFE_FREE(&szNewName);
		}
	}
	return 0;
}
