#include "stdafx.h"

// Event
bool HistoryArray::ItemData::load(EventLoadMode mode)
{
	if (mode == ELM_NOTHING)
		return true;

	if ((mode == ELM_INFO) && !dbeOk) {
		dbeOk = true;
		dbe.cbBlob = 0;
		dbe.pBlob = 0;
		db_event_get(hEvent, &dbe);
		return true;
	}

	if ((mode == ELM_DATA) && (!dbeOk || !dbe.cbBlob)) {
		dbeOk = true;
		dbe.cbBlob = db_event_getBlobSize(hEvent);
		dbe.pBlob = (PBYTE)calloc(dbe.cbBlob + 1, 1);
		db_event_get(hEvent, &dbe);

		int aLength = 0;
		atext = 0;
		wtext = 0;

		switch (dbe.eventType) {
		case EVENTTYPE_STATUSCHANGE:
		case EVENTTYPE_MESSAGE:
			{
				atext = (char *)dbe.pBlob;
				atext_del = false;
				aLength = lstrlenA(atext);
				if (dbe.cbBlob > (DWORD)aLength + 1) {
					wtext = (WCHAR *)(dbe.pBlob + aLength + 1);
					wtext_del = false;
				}
				break;
			}

		case EVENTTYPE_AUTHREQUEST:
			{
				atext = new char[512];
				atext_del = true;
				if ((dbe.cbBlob > 8) && *(dbe.pBlob + 8)) {
					mir_snprintf(atext, 512, ("%s requested authorization"), dbe.pBlob + 8);
				}
				else {
					mir_snprintf(atext, 512, ("%d requested authorization"), *(DWORD*)(dbe.pBlob));
				}
				aLength = lstrlenA(atext);
				break;
			}

		case EVENTTYPE_ADDED:
			{
				atext = new char[512];
				atext_del = true;
				if ((dbe.cbBlob > 8) && *(dbe.pBlob + 8)) {
					mir_snprintf(atext, 512, ("%s added you to the contact list"), dbe.pBlob + 8);
				}
				else {
					mir_snprintf(atext, 512, ("%d added you to the contact list"), *(DWORD*)(dbe.pBlob));
				}
				aLength = lstrlenA(atext);
				break;
			}
		}

		if (atext && !wtext) {
			#ifdef UNICODE
			int bufSize = MultiByteToWideChar(CP_ACP, 0, atext, aLength + 1, 0, 0);
			wtext = new WCHAR[bufSize + 1];
			MultiByteToWideChar(CP_ACP, 0, atext, aLength + 1, wtext, bufSize);
			wtext_del = true;
			#else
			this->wtext = 0;
			wtext_del = false;
			#endif
		}
		else
			if (!atext && wtext) {
				// strange situation, really :) I'll fix this later
			}
			else
				if (!atext && !wtext) {
					atext = "";
					atext_del = false;
					wtext = L"";
					wtext_del = false;
				}

		return true;
	}

	return false;
}

HistoryArray::ItemData::~ItemData()
{
	if (dbeOk && dbe.pBlob) {
		free(dbe.pBlob);
		dbe.pBlob = 0;
	}
	if (wtext && wtext_del) delete[] wtext;
	if (atext && atext_del) delete[] atext;
	if (data) MTextDestroy(data);
}

// Array
HistoryArray::HistoryArray()
{
	head = tail = 0;
	caching = false;
	caching_complete = true;
	InitializeCriticalSection(&csItems);
}

HistoryArray::~HistoryArray()
{
	DeleteCriticalSection(&csItems);
	clear();
}

bool HistoryArray::allocateBlock(int count)
{
	ItemBlock *newBlock = new ItemBlock;
	newBlock->items = new ItemData[count];
	newBlock->count = count;
	newBlock->prev = tail;
	newBlock->next = 0;

	if (tail) {
		tail->next = newBlock;
	}
	else {
		head = newBlock;
	}
	tail = newBlock;

	return true;
}

void HistoryArray::clear()
{
	while (head) {
		ItemBlock *next = head->next;
		//		for (int i = 0; i < head->count; ++i)
		//			destroyEvent(head->items[i]);
		delete[] head->items;
		head = next;
	}

	head = tail = 0;
	preBlock = 0;
	preIndex = 0;
}

bool HistoryArray::addHistory(MCONTACT hContact, EventLoadMode)
{
	int count = db_event_count(hContact);
	allocateBlock(count);

	int i = 0;
	MEVENT hEvent = db_event_first(hContact);
	while (hEvent) {
		tail->items[i].hContact = hContact;
		tail->items[i].hEvent = hEvent;

		++i;
		hEvent = db_event_next(hEvent, 0);
	}
	return true;
}

bool HistoryArray::addEvent(MCONTACT hContact, MEVENT hEvent, EventLoadMode mode)
{
	allocateBlock(1);
	tail->items[0].hContact = hContact;
	tail->items[0].hEvent = hEvent;
	if (mode != ELM_NOTHING)
		tail->items[0].load(mode);

	return true;
}
/*
bool HistoryArray::preloadEvents(int count)
{
	for (int i = 0; i < count; ++i)
	{
		preBlock->items[preIndex].load(ELM_DATA);
		if (++preIndex == preBlock->count)
		{
			preBlock = preBlock->next;
			if (!preBlock)
				return false;
			preIndex = 0;
		}
	}
	return true;
}
*/
HistoryArray::ItemData *HistoryArray::get(int id, EventLoadMode mode)
{
	caching = false;
	if (caching) EnterCriticalSection(&csItems);
	int offset = 0;
	for (ItemBlock *p = head; p; p = p->next) {
		if (id < offset + p->count) {
			if (mode != ELM_NOTHING)
				p->items[id - offset].load(mode);

			if (caching) {
				if (caching_complete) caching = false;
				LeaveCriticalSection(&csItems);
			}
			return p->items + id - offset;
		}
		offset += p->count;
	}
	if (caching) {
		if (caching_complete) caching = false;
		LeaveCriticalSection(&csItems);
	}
	return 0;
}

///////////////////////////////////////////////////////////
// Cache data
void HistoryArray::CacheThreadFunc(void *arg)
{
	HistoryArray *_this = (HistoryArray *)arg;
	_this->caching_complete = true;
	_endthread();
}
