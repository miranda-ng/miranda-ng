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
		item->load(true);
		return CheckFilter(item->getWBuf(), text);
	}
	return true;
};

// Event
void ItemData::load(bool bFullLoad)
{
	if (!bFullLoad || bLoaded)
		return;

	dbe.cbBlob = db_event_getBlobSize(hEvent);
	dbe.pBlob = (PBYTE)mir_calloc(dbe.cbBlob + 1);
	if (db_event_get(hEvent, &dbe))
		return;

	bLoaded = true;

	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		if (!(dbe.flags & DBEF_SENT)) {
			if (!dbe.markedRead())
				db_event_markRead(hContact, hEvent);
			g_clistApi.pfnRemoveEvent(hContact, hEvent);
		}
		__fallthrough;

	case EVENTTYPE_STATUSCHANGE:
		wtext = mir_utf8decodeW((char *)dbe.pBlob);
		break;

	default:
		wtext = DbEvent_GetTextW(&dbe, CP_ACP);
		break;
	}
}

bool ItemData::isGrouped() const
{
	if (pPrev && g_plugin.bMsgGrouping) {
		if (!pPrev->bLoaded)
			pPrev->load(true);

		if (pPrev->hContact == hContact && (pPrev->dbe.flags & DBEF_SENT) == (dbe.flags & DBEF_SENT))
			return true;
	}
	return false;
}

ItemData::~ItemData()
{
	if (bLoaded)
		mir_free(dbe.pBlob);
	mir_free(wtext);
	if (data)
		MTextDestroy(data);
}

// Array
HistoryArray::HistoryArray() :
	pages(50),
	strings(50, wcscmp)
{
	pages.insert(new ItemBlock());
}

HistoryArray::~HistoryArray()
{
	clear();
}

void HistoryArray::clear()
{
	for (auto &str : strings)
		mir_free(str);
	strings.destroy();

	pages.destroy();
	iLastPageCounter = 0;
}

void HistoryArray::addChatEvent(SESSION_INFO *si, LOGINFO *lin)
{
	if (si == nullptr)
		return;

	CMStringW wszText;
	bool bTextUsed = Chat_GetDefaultEventDescr(si, lin, wszText);
	if (!bTextUsed && lin->ptszText) {
		if (!wszText.IsEmpty())
			wszText.Append(L": ");
		wszText.Append(g_chatApi.RemoveFormatting(lin->ptszText));
	}

	auto &p = allocateItem();
	p.hContact = si->hContact;
	p.wtext = wszText.Detach();
	p.bLoaded = true;
	p.dbe.eventType = EVENTTYPE_MESSAGE;
	p.dbe.timestamp = lin->time;

	if (lin->ptszNick) {
		p.wszNick = strings.find(lin->ptszNick);
		if (p.wszNick == nullptr) {
			p.wszNick = mir_wstrdup(lin->ptszNick);
			strings.insert(p.wszNick);
		}
	}
}

bool HistoryArray::addEvent(MCONTACT hContact, MEVENT hEvent, int count)
{
	if (count == -1)
		count = MAXINT;

	int numItems = getCount();
	auto *pPrev = (numItems == 0) ? nullptr : get(numItems - 1);

	for (int i = 0; hEvent && i < count; i++) {
		auto &p = allocateItem();
		p.hContact = hContact;
		p.hEvent = hEvent;
		p.pPrev = pPrev; pPrev = &p;

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

ItemData* HistoryArray::get(int id, bool bLoad)
{
	int pageNo = id / HIST_BLOCK_SIZE;
	if (pageNo >= pages.getCount())
		return nullptr;

	auto *p = &pages[pageNo].data[id % HIST_BLOCK_SIZE];
	if (bLoad && !p->bLoaded)
		p->load(true);
	return p;
}

int HistoryArray::getCount() const
{
	return (pages.getCount() - 1) * HIST_BLOCK_SIZE + iLastPageCounter;
}
