/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

extern HANDLE htuLog;

/////////////////////////////////////////////////////////////////////////////////////////
// Filters

bool Filter::check(ItemData *item) const
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

	if (flags & (EVENTTEXT | EVENTONLY))
		return CheckFilter(item->getWBuf(), text);

	return true;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Event

ItemData::ItemData()
{
	memset(this, 0, sizeof(*this));
	m_grouping = g_plugin.bMsgGrouping ? -1 : GROUPING_NONE;
	savedHeight = -1;
}

ItemData::~ItemData()
{
	mir_free(wtext);
	if (dbe.szReplyId)
		mir_free((char*)dbe.szReplyId);
	if (data)
		MTextDestroy(data);
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isEqual(const ItemData *p1, const ItemData *p2)
{
	if (p1->hContact != p2->hContact)
		return false;
	if (p1->dbe.eventType != p2->dbe.eventType)
		return false;
	if ((p1->dbe.flags & DBEF_SENT) != (p2->dbe.flags & DBEF_SENT))
		return false;
	if (p1->dbe.timestamp / 86400 != p2->dbe.timestamp / 86400)
		return false;
	return true;
}

ItemData* ItemData::checkPrev(ItemData *pPrev)
{
	m_grouping = GROUPING_NONE;
	if (!pPrev || !g_plugin.bMsgGrouping)
		return this;

	// we don't group anything but messages
	if (!fetch())
		return this;

	if (dbe.eventType != EVENTTYPE_MESSAGE)
		return this;

	if (isEqual(this, pPrev)) {
		if (pPrev->m_grouping == GROUPING_NONE) {
			pPrev->m_grouping = GROUPING_HEAD;
			if (pPrev->m_bLoaded)
				pPrev->setText();
		}
		m_grouping = GROUPING_ITEM;
	}
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isEqualGC(const ItemData *p1, const ItemData *p2)
{
	if (p1->dbe.eventType != p2->dbe.eventType)
		return false;

	if (!p1->wszNick || !p2->wszNick)
		return false;

	if (wcscmp(p1->wszNick, p2->wszNick))
		return false;

	if (p1->dbe.timestamp / 86400 != p2->dbe.timestamp / 86400)
		return false;
	return true;
}

ItemData* ItemData::checkPrevGC(ItemData *pPrev)
{
	m_grouping = GROUPING_NONE;
	if (!pPrev || !g_plugin.bMsgGrouping)
		return this;

	if (dbe.eventType != EVENTTYPE_MESSAGE)
		return this;

	if (isEqualGC(this, pPrev)) {
		if (pPrev->m_grouping == GROUPING_NONE) {
			pPrev->m_grouping = GROUPING_HEAD;
			if (pPrev->m_bLoaded)
				pPrev->setText();
		}
		m_grouping = GROUPING_ITEM;
	}
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ItemData::checkCreate(HWND hwnd)
{
	if (data == nullptr) {
		setText();
		MTextSetParent(data, hwnd);
		MTextActivate(data, true);
	}
}

bool ItemData::isLink(POINT pt, CMStringW *pwszUrl) const
{
	int cp = MTextSendMessage(0, data, EM_CHARFROMPOS, 0, LPARAM(&pt));
	if (cp == -1)
		return false;

	if (!isLinkChar(cp))
		return false;

	if (pwszUrl) {
		CHARRANGE sel = { cp, cp };
		while (isLinkChar(sel.cpMin-1))
			sel.cpMin--;

		while (isLinkChar(sel.cpMax))
			sel.cpMax++;

		if (sel.cpMax > sel.cpMin) {
			pwszUrl->Truncate(sel.cpMax - sel.cpMin + 1);

			TEXTRANGE tr = { 0 };
			tr.chrg = sel;
			tr.lpstrText = pwszUrl->GetBuffer();
			int iRes = MTextSendMessage(0, data, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
			if (iRes > 0)
				pwszUrl->Trim();
			else
				pwszUrl->Empty();
		}
	}
	return true;
}

bool ItemData::isLinkChar(int idx) const
{
	if (idx < 0)
		return false;

	CHARRANGE sel = { idx, idx + 1 };
	MTextSendMessage(0, data, EM_EXSETSEL, 0, LPARAM(&sel));

	CHARFORMAT2 cf = {};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_LINK;
	uint32_t res = MTextSendMessage(0, data, EM_GETCHARFORMAT, SCF_SELECTION, LPARAM(&cf));
	return ((res & CFM_LINK) && (cf.dwEffects & CFE_LINK)) || ((res & CFM_REVISED) && (cf.dwEffects & CFE_REVISED));
}

bool ItemData::fetch(void)
{
	// if this event is virtual (for example, in group chats), don't try to laod it
	if (!hEvent)
		return true;

	if (!dbe) {
		if (!dbe.fetch(hEvent))
			return false;

		if (dbe.szReplyId)
			dbe.szReplyId = mir_strdup(dbe.szReplyId);
	}
	return true;
}

void ItemData::fill(int tmpl)
{
	switch (tmpl) {
	case TPL_MESSAGE:
		dbe.eventType = EVENTTYPE_MESSAGE;
		break;
	case TPL_MSG_HEAD:
		m_grouping = GROUPING_HEAD;
		dbe.eventType = EVENTTYPE_MESSAGE;
		break;
	case TPL_MSG_GRP:
		m_grouping = GROUPING_ITEM;
		dbe.eventType = EVENTTYPE_MESSAGE;
		break;
	case TPL_FILE:
		dbe.eventType = EVENTTYPE_FILE;
		break;
	case TPL_SIGN:
		dbe.eventType = EVENTTYPE_STATUSCHANGE;
		break;
	case TPL_AUTH:
		dbe.eventType = EVENTTYPE_AUTHREQUEST;
		break;
	case TPL_ADDED:
		dbe.eventType = EVENTTYPE_ADDED;
		break;
	case TPL_PRESENCE:
		dbe.eventType = EVENTTYPE_JABBER_PRESENCE;
		break;
	default:
		dbe.eventType = 9247;
		break;
	}
}

int ItemData::getTemplate() const
{
	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		switch (m_grouping) {
		case GROUPING_HEAD: return TPL_MSG_HEAD;
		case GROUPING_ITEM: return TPL_MSG_GRP;
		}
		return TPL_MESSAGE;

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

void ItemData::load(bool bFullLoad)
{
	if (!bFullLoad && m_bLoaded)
		return;

	if (!fetch())
		return;

	m_bLoaded = true;

	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		markRead();
		__fallthrough;

	case EVENTTYPE_STATUSCHANGE:
		wtext = mir_utf8decodeW((char *)dbe.pBlob);
		break;

	case EVENTTYPE_FILE:
		{
			DB::FILE_BLOB blob(dbe);
			if (blob.isOffline()) {
				m_bOfflineFile = true;
				m_bOfflineDownloaded = blob.isCompleted();

				CMStringW buf;
				buf.Append(blob.getName() ? blob.getName() : TranslateT("Unnamed"));

				if (auto *pwszDescr = blob.getDescr()) {
					buf.Append(L" - ");
					buf.Append(pwszDescr);
				}

				if (uint32_t size = blob.getSize())
					buf.AppendFormat(TranslateT(" %u KB"), size < 1024 ? 1 : unsigned(blob.getSize() / 1024));

				if (blob.getSize() > 0 && blob.getSize() == blob.getTransferred())
					buf.AppendFormat(L" [$hicon=%p$]", g_plugin.getIcon(IDI_OK));

				wtext = buf.Detach();
				markRead();
				break;
			}

			wchar_t buf[MAX_PATH];
			CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, (LPARAM)buf);

			CMStringW wszFileName = buf;
			wszFileName.Append(blob.getName());

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

	if (dbe.szReplyId)
		if (MEVENT hReply = db_event_getById(dbe.szModule, dbe.szReplyId)) {
			DB::EventInfo dbei(hReply);
			if (dbei) {
				CMStringW str(L"> ");

				if (dbei.flags & DBEF_SENT) {
					if (char *szProto = Proto_GetBaseAccountName(hContact))
						str.AppendFormat(L"%s %s: ", ptrW(Contact::GetInfo(CNF_DISPLAY, 0, szProto)).get(), TranslateT("wrote"));
				}
				else str.AppendFormat(L"%s %s: ", Clist_GetContactDisplayName(hContact, 0), TranslateT("wrote"));

				ptrW wszText(DbEvent_GetTextW(&dbei, CP_ACP));
				if (mir_wstrlen(wszText) > 43)
					wcscpy(wszText.get() + 40, L"...");
				str.Append(wszText);
				str.Append(L"\r\n");
				str.Append(wtext);

				mir_free(wtext);
				wtext = str.Detach();
			}
		}

	dbe.unload();
}

void ItemData::markRead()
{
	if (!(dbe.flags & DBEF_SENT)) {
		if (!dbe.markedRead())
			db_event_markRead(hContact, hEvent);
		Clist_RemoveEvent(-1, hEvent);
	}
}

void ItemData::setText()
{
	data = MTextCreateEx(htuLog, formatRtf().GetBuffer(), MTEXT_FLG_RTF);
	savedHeight = -1;
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

void HistoryArray::addChatEvent(SESSION_INFO *si, const LOGINFO *lin)
{
	if (si == nullptr)
		return;

	int numItems = getCount();
	auto &p = allocateItem();
	p.hContact = si->hContact;

	if (si->pMI->bDatabase && lin->hEvent) {
		p.hEvent = lin->hEvent;
		p.load();
	}
	else {
		CMStringW wszText;
		bool bTextUsed = Chat_GetDefaultEventDescr(si, lin, wszText);
		if (!bTextUsed && lin->ptszText) {
			if (!wszText.IsEmpty())
				wszText.AppendChar(' ');
			wszText.Append(g_chatApi.RemoveFormatting(lin->ptszText));
		}

		p.wtext = wszText.Detach();
		p.m_bLoaded = true;
		p.m_bHighlighted = lin->bIsHighlighted;
		p.dbe.timestamp = lin->time;
		if (lin->bIsMe)
			p.dbe.flags |= DBEF_SENT;

		switch (lin->iType) {
		case GC_EVENT_MESSAGE:
		case GC_EVENT_INFORMATION:
			p.dbe.eventType = EVENTTYPE_MESSAGE;
			break;

		case GC_EVENT_SETCONTACTSTATUS:
			p.dbe.eventType = EVENTTYPE_STATUSCHANGE;
			break;

		case GC_EVENT_JOIN:
		case GC_EVENT_PART:
		case GC_EVENT_QUIT:
			p.dbe.eventType = EVENTTYPE_JABBER_PRESENCE;
			break;

		default:
			p.dbe.eventType = EVENTTYPE_OTHER;
			break;
		}
	}

	if (lin->ptszNick) {
		p.wszNick = strings.find(lin->ptszNick);
		if (p.wszNick == nullptr) {
			p.wszNick = mir_wstrdup(lin->ptszNick);
			strings.insert(p.wszNick);
		}
		p.checkPrevGC((numItems == 0) ? nullptr : get(numItems - 1));
	}
}

bool HistoryArray::addEvent(MCONTACT hContact, MEVENT hEvent, int count)
{
	if (count == -1)
		count = MAXINT;

	int numItems = getCount();
	auto *pPrev = (numItems == 0) ? nullptr : get(numItems - 1);

	if (count == 1) {
		auto &p = allocateItem();
		p.hContact = hContact;
		p.hEvent = hEvent;
		pPrev = p.checkPrev(pPrev);
	}
	else {
		DB::ECPTR pCursor(DB::Events(hContact, hEvent));
		for (int i = 0; i < count; i++) {
			hEvent = pCursor.FetchNext();
			if (!hEvent)
				break;

			auto &p = allocateItem();
			p.hContact = hContact;
			p.hEvent = hEvent;
			pPrev = p.checkPrev(pPrev);
		}
	}

	return true;
}

void HistoryArray::addResults(const OBJLIST<SearchResult> &pArray)
{
	int numItems = getCount();
	auto *pPrev = (numItems == 0) ? nullptr : get(numItems - 1);

	for (auto &it : pArray) {
		auto &p = allocateItem();
		p.hContact = it->hContact;
		p.hEvent = it->hEvent;
		p.m_bIsResult = true;
		pPrev = p.checkPrev(pPrev);
	}
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

int HistoryArray::find(MEVENT hEvent)
{
	int i = 0;
	for (auto &it : pages)
		for (auto &p : it->data) {
			if (p.hEvent == hEvent)
				return i;
			i++;
		}

	return -1;
}

ItemData* HistoryArray::get(int id, bool bLoad) const
{
	int pageNo = id / HIST_BLOCK_SIZE;
	if (pageNo >= pages.getCount())
		return nullptr;

	auto *p = &pages[pageNo].data[id % HIST_BLOCK_SIZE];
	if (bLoad && !p->m_bLoaded)
		p->load();
	return p;
}

int HistoryArray::getCount() const
{
	int nPages = pages.getCount();
	return (nPages == 0) ? 0 : (nPages - 1) * HIST_BLOCK_SIZE + iLastPageCounter;
}

int HistoryArray::find(int id, int dir, const Filter &filter)
{
	int count = getCount();
	for (int i = id + dir; i >= 0 && i < count; i += dir)
		if (filter.check(get(i)))
			return i;

	return -1;
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
