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
		dbe.pBlob = (PBYTE)mir_calloc(dbe.cbBlob + 1);
		db_event_get(hEvent, &dbe);

		wtext = 0;

		switch (dbe.eventType) {
		case EVENTTYPE_STATUSCHANGE:
		case EVENTTYPE_MESSAGE:
			wtext = mir_utf8decodeW((char*)dbe.pBlob);
			wtext_del = false;
			break;

		case EVENTTYPE_JABBER_PRESENCE:
			wtext = DbEvent_GetTextW(&dbe, CP_ACP);
			wtext_del = false;
			break;

		case EVENTTYPE_AUTHREQUEST:
			wtext = new wchar_t[512];
			wtext_del = true;
			if ((dbe.cbBlob > 8) && *(dbe.pBlob + 8)) {
				mir_snwprintf(wtext, 512, L"%s requested authorization", dbe.pBlob + 8);
			}
			else {
				mir_snwprintf(wtext, 512, L"%d requested authorization", *(DWORD *)(dbe.pBlob));
			}
			break;

		case EVENTTYPE_ADDED:
			wtext = new wchar_t[512];
			wtext_del = true;
			if ((dbe.cbBlob > 8) && *(dbe.pBlob + 8)) {
				mir_snwprintf(wtext, 512, L"%s added you to the contact list", dbe.pBlob + 8);
			}
			else {
				mir_snwprintf(wtext, 512, L"%d added you to the contact list", *(DWORD *)(dbe.pBlob));
			}
			break;
		}

		return true;
	}

	return false;
}

HistoryArray::ItemData::~ItemData()
{
	if (dbeOk && dbe.pBlob) {
		mir_free(dbe.pBlob);
		dbe.pBlob = 0;
	}
	if (wtext && wtext_del) delete[] wtext;
	if (data) MTextDestroy(data);
}

// Array
HistoryArray::HistoryArray()
{
}

HistoryArray::~HistoryArray()
{
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
		hEvent = db_event_next(hContact, hEvent);
	}
	return true;
}

bool HistoryArray::addEvent(MCONTACT hContact, MEVENT hEvent, int count, EventLoadMode mode)
{
	allocateBlock(count);

	for (int i = 0; i < count; i++) {
		tail->items[i].hContact = hContact;
		tail->items[i].hEvent = hEvent;
		if (mode != ELM_NOTHING)
			tail->items[i].load(mode);
		hEvent = db_event_next(hContact, hEvent);
	}

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
	int offset = 0;
	for (ItemBlock *p = head; p; p = p->next) {
		if (id < offset + p->count) {
			if (mode != ELM_NOTHING)
				p->items[id - offset].load(mode);

			return p->items + id - offset;
		}
		offset += p->count;
	}
	return 0;
}
