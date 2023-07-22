#include "stdafx.h"

extern HANDLE htuLog;

/////////////////////////////////////////////////////////////////////////////////////////
// Filters

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

/////////////////////////////////////////////////////////////////////////////////////////
// Event

void ItemData::checkCreate(HWND hwnd)
{
	if (data == nullptr) {
		if (bRtf)
			data = MTextCreateEx(htuLog, this->wtext, MTEXT_FLG_WCHAR | MTEXT_FLG_RTF);
		else
			data = MTextCreateW(htuLog, Proto_GetBaseAccountName(hContact), ptrW(TplFormatString(getTemplate(), hContact, this)));
		MTextSetParent(data, hwnd);
		MTextActivate(data, true);
	}
}

bool ItemData::isLink(POINT pt, CMStringW &url) const
{
	int cp = MTextSendMessage(0, data, EM_CHARFROMPOS, 0, LPARAM(&pt));
	if (cp == -1)
		return false;
	
	if (!isLinkChar(cp))
		return false;

	CHARRANGE sel = { cp, cp };
	for (sel.cpMin = cp; sel.cpMin >= 0; sel.cpMin--)
		if (!isLinkChar(sel.cpMin))
			break;

	for (sel.cpMax = cp + 1; isLinkChar(sel.cpMax); sel.cpMax++)
		;

	if (sel.cpMax > sel.cpMin) {
		url.Truncate(sel.cpMax - sel.cpMin + 1);

		TEXTRANGE tr = { 0 };
		tr.chrg = sel;
		tr.lpstrText = url.GetBuffer();
		int iRes = MTextSendMessage(0, data, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		if (iRes > 0)
			url.Trim();
		else
			url.Empty();
	}
	return true;
}

bool ItemData::isLinkChar(int idx) const
{
	CHARRANGE sel = { idx, idx + 1 };
	MTextSendMessage(0, data, EM_EXSETSEL, 0, LPARAM(&sel));

	CHARFORMAT2 cf = {};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_LINK;
	uint32_t res = MTextSendMessage(0, data, EM_GETCHARFORMAT, SCF_SELECTION, LPARAM(&cf));
	return ((res & CFM_LINK) && (cf.dwEffects & CFE_LINK)) || ((res & CFM_REVISED) && (cf.dwEffects & CFE_REVISED));
}

void ItemData::load(bool bFullLoad)
{
	if (!bFullLoad || bLoaded)
		return;

	dbe.cbBlob = -1;
	if (db_event_get(hEvent, &dbe))
		return;

	bLoaded = true;

	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		if (!(dbe.flags & DBEF_SENT)) {
			if (!dbe.markedRead())
				db_event_markRead(hContact, hEvent);
			Clist_RemoveEvent(hContact, hEvent);
		}
		__fallthrough;

	case EVENTTYPE_STATUSCHANGE:
		wtext = mir_utf8decodeW((char *)dbe.pBlob);
		break;

	case EVENTTYPE_FILE:
		{
			CMStringW wszFileName;
			DB::FILE_BLOB blob(dbe);
			if (blob.isOffline()) {
				wszFileName = blob.getLocalName();
			}
			else {
				wchar_t buf[MAX_PATH];
				CallService(MS_FILE_GETRECEIVEDFILESFOLDERW, hContact, (LPARAM)buf);

				wszFileName = buf;
				wszFileName.Append(blob.getName());
			}

			// if a filename contains spaces, URL will be broken
			if (wszFileName.Find(' ') != -1) {
				wchar_t wszShortPath[MAX_PATH];
				if (GetShortPathNameW(wszFileName, wszShortPath, _countof(wszShortPath))) {
					wszFileName = wszShortPath;
					wszFileName.MakeLower();
				}
			}

			wszFileName.Replace('\\', '/');
			wszFileName.Insert(0, L"file://");
			wtext = wszFileName.Detach();
		}
		break;

	default:
		wtext = DbEvent_GetTextW(&dbe, CP_ACP);
		break;
	}

	mir_free(dbe.pBlob);
	dbe.pBlob = nullptr;
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
	mir_free(wtext);
	if (data)
		MTextDestroy(data);
}

int ItemData::getTemplate() const
{
	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:         return isGrouped() ? TPL_MSG_GRP : TPL_MESSAGE;
	case EVENTTYPE_FILE:            return TPL_FILE;
	case EVENTTYPE_STATUSCHANGE:    return TPL_SIGN;
	case EVENTTYPE_AUTHREQUEST:     return TPL_AUTH;
	case EVENTTYPE_ADDED:           return TPL_ADDED;
	case EVENTTYPE_JABBER_PRESENCE: return TPL_PRESENCE;
	default:
		return TPL_OTHER;
	}
}

int ItemData::getCopyTemplate() const
{
	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:         return TPL_COPY_MESSAGE;
	case EVENTTYPE_FILE:            return TPL_COPY_FILE;
	case EVENTTYPE_STATUSCHANGE:    return TPL_COPY_SIGN;
	case EVENTTYPE_AUTHREQUEST:     return TPL_COPY_AUTH;
	case EVENTTYPE_ADDED:           return TPL_COPY_ADDED;
	case EVENTTYPE_JABBER_PRESENCE: return TPL_COPY_PRESENCE;
	default:
		return TPL_COPY_OTHER;
	}
}

void ItemData::getFontColor(int &fontId, int &colorId) const
{
	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		fontId = !(dbe.flags & DBEF_SENT) ? FONT_INMSG : FONT_OUTMSG;
		colorId = !(dbe.flags & DBEF_SENT) ? COLOR_INMSG : COLOR_OUTMSG;
		break;

	case EVENTTYPE_FILE:
		fontId = !(dbe.flags & DBEF_SENT) ? FONT_INFILE : FONT_OUTFILE;
		colorId = !(dbe.flags & DBEF_SENT) ? COLOR_INFILE : COLOR_OUTFILE;
		break;

	case EVENTTYPE_STATUSCHANGE:
		fontId = FONT_STATUS;
		colorId = COLOR_STATUS;
		break;

	case EVENTTYPE_AUTHREQUEST:
		fontId = FONT_INOTHER;
		colorId = COLOR_INOTHER;
		break;

	case EVENTTYPE_ADDED:
		fontId = FONT_INOTHER;
		colorId = COLOR_INOTHER;
		break;

	case EVENTTYPE_JABBER_PRESENCE:
		fontId = !(dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		colorId = !(dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
		break;

	default:
		fontId = !(dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		colorId = !(dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
		break;
	}
}

// Array
HistoryArray::HistoryArray() :
	pages(50),
	strings(50, wcscmp)
{
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
			wszText.AppendChar(' ');
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

	DB::ECPTR pCursor(DB::Events(hContact, hEvent));
	for (int i = 0; i < count; i++) {
		hEvent = pCursor.FetchNext();
		if (!hEvent)
			break;

		auto &p = allocateItem();
		p.hContact = hContact;
		p.hEvent = hEvent;
		p.pPrev = pPrev; pPrev = &p;
	}

	return true;
}

ItemData& HistoryArray::allocateItem()
{
	if (iLastPageCounter == HIST_BLOCK_SIZE) {
		pages.insert(new ItemBlock());
		iLastPageCounter = 0;
	}
	else if (pages.getCount() == 0)
		pages.insert(new ItemBlock);

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
	int nPages = pages.getCount();
	return (nPages == 0) ? 0 : (nPages - 1) * HIST_BLOCK_SIZE + iLastPageCounter;
}

void HistoryArray::remove(int id)
{
	int pageNo = id / HIST_BLOCK_SIZE;
	if (pageNo >= pages.getCount())
		return;

	auto &pPage = pages[pageNo];
	int offset = id % HIST_BLOCK_SIZE;

	ItemData tmp;
	memcpy(&tmp, pPage.data + offset, sizeof(ItemData));

	if (offset != HIST_BLOCK_SIZE - 1)
		memmove(&pPage.data[offset], &pPage.data[offset+1], sizeof(ItemData) * (HIST_BLOCK_SIZE - 1 - offset));

	for (int i = pageNo + 1; i < pages.getCount(); i++) {
		auto &prev = pages[i - 1], &curr = pages[i];
		memcpy(&prev.data[HIST_BLOCK_SIZE - 1], curr.data, sizeof(ItemData));
		memmove(&curr.data, &curr.data[1], sizeof(ItemData) * (HIST_BLOCK_SIZE - 1));
		memset(&curr.data[HIST_BLOCK_SIZE - 1], 0, sizeof(ItemData));
	}

	if (iLastPageCounter == 1) {
		pages.remove(pages.getCount() - 1);
		iLastPageCounter = HIST_BLOCK_SIZE;
	}
	else iLastPageCounter--;
}
