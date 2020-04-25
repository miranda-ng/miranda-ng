#include "stdafx.h"

bool Filter::check(ItemData *item)
{
	if (!item) return false;
	if (!(flags & EVENTONLY)) {
		if (item->dbe.flags & DBEF_SENT) {
			if (!(flags & OUTGOING))
				return false;
		}
		else {
			if (!(flags & INCOMING))
				return false;
		}
		switch (item->dbe.eventType) {
		case EVENTTYPE_MESSAGE:
			if (!(flags & MESSAGES))
				return false;
			break;
		case EVENTTYPE_FILE:
			if (!(flags & FILES))
				return false;
			break;
		case EVENTTYPE_STATUSCHANGE:
			if (!(flags & STATUS))
				return false;
			break;
		default:
			if (!(flags & OTHER))
				return false;
		}
	}
	if (flags & (EVENTTEXT | EVENTONLY)) {
		item->loadInline(ItemData::ELM_DATA);
		return CheckFilter(item->getWBuf(), text);
	}
	return true;
};

// Event
bool ItemData::load(EventLoadMode mode)
{
	if (mode == ItemData::ELM_NOTHING)
		return true;

	if ((mode == ItemData::ELM_INFO) && !dbeOk) {
		dbeOk = true;
		dbe.cbBlob = 0;
		dbe.pBlob = 0;
		db_event_get(hEvent, &dbe);
		return true;
	}

	if ((mode == ItemData::ELM_DATA) && (!dbeOk || !dbe.cbBlob)) {
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
				mir_snwprintf(wtext, 512, L"%s requested authorization", Utf2T((char*)dbe.pBlob + 8).get());
			}
			else {
				mir_snwprintf(wtext, 512, L"%d requested authorization", *(DWORD *)(dbe.pBlob));
			}
			break;

		case EVENTTYPE_ADDED:
			wtext = new wchar_t[512];
			wtext_del = true;
			if ((dbe.cbBlob > 8) && *(dbe.pBlob + 8)) {
				mir_snwprintf(wtext, 512, L"%s added you to the contact list", Utf2T((char *)dbe.pBlob + 8).get());
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

ItemData::~ItemData()
{
	if (dbeOk && dbe.pBlob) {
		mir_free(dbe.pBlob);
		dbe.pBlob = 0;
	}
	if (wtext && wtext_del) delete[] wtext;
	if (data) MTextDestroy(data);
}

// Array
HistoryArray::HistoryArray() :
	pages(50)
{
	pages.insert(new ItemBlock());
}

HistoryArray::~HistoryArray()
{
	clear();
}

void HistoryArray::clear()
{
	pages.destroy();
	iLastPageCounter = 0;
}

void HistoryArray::addChatEvent(MCONTACT hContact, LOGINFO *pEvent)
{
	auto &p = allocateItem();
	p.hContact = hContact;
	p.wtext = mir_wstrdup(pEvent->ptszText);
	p.dbeOk = true;
	p.dbe.pBlob = (BYTE *)p.wtext;
	p.dbe.eventType = EVENTTYPE_MESSAGE;
	p.dbe.timestamp = pEvent->time;
}

bool HistoryArray::addEvent(MCONTACT hContact, MEVENT hEvent, int count, ItemData::EventLoadMode mode)
{
	if (count == -1)
		count = MAXINT;

	for (int i = 0; hEvent && i < count; i++) {
		auto &p = allocateItem();
		p.hContact = hContact;
		p.hEvent = hEvent;

		if (mode != ItemData::ELM_NOTHING)
			p.load(mode);
		
		hEvent = db_event_next(hContact, hEvent);
	}

	return true;
}

ItemData& HistoryArray::allocateItem()
{
	if (iLastPageCounter == HIST_BLOCK_SIZE - 1) {
		pages.insert(new ItemBlock());
		iLastPageCounter = 0;
	}

	auto &p = pages[pages.getCount() - 1];
	return p.data[iLastPageCounter++];
}

/*
bool HistoryArray::preloadEvents(int count)
{
	for (int i = 0; i < count; ++i)
	{
		preBlock->items[preIndex].load(ItemData::ELM_DATA);
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

ItemData* HistoryArray::get(int id, ItemData::EventLoadMode mode)
{
	int pageNo = id / HIST_BLOCK_SIZE;
	if (pageNo >= pages.getCount())
		return nullptr;

	return &pages[pageNo].data[id % HIST_BLOCK_SIZE];
}

int HistoryArray::getCount() const
{
	return (pages.getCount() - 1) * HIST_BLOCK_SIZE + iLastPageCounter;
}
