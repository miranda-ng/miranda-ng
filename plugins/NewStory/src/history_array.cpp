/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

	if (flags & (EVENTTEXT | EVENTONLY)) {
		if (item->m_bLoaded)
			return CheckFilter(item->wtext, text);

		if (!item->fetch())
			return false;

		return CheckFilter(ptrW(item->dbe.getText()), text);
	}

	return true;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Event

ItemData::ItemData()
{
	memset(this, 0, sizeof(*this));
	savedHeight = -1;
}

ItemData::~ItemData()
{
	replaceStrW(qtext, 0);
	replaceStrW(wtext, 0);
	if (dbe.szReplyId) {
		mir_free((char *)dbe.szReplyId);
		dbe.szReplyId = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isEqual(const ItemData *p1, const ItemData *p2)
{
	if (p1->dbe.hContact != p2->dbe.hContact)
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

ItemData* ItemData::checkNext(ItemData *pPrev)
{
	m_grouping = GROUPING_NONE;
	if (!pPrev || !g_plugin.bMsgGrouping)
		return this;

	// we don't group anything but messages
	if (!fetch())
		return this;

	if (dbe.eventType != EVENTTYPE_MESSAGE)
		return this;

	pPrev->fetch();
	if (isEqual(this, pPrev)) {
		if (pPrev->m_grouping == GROUPING_NONE) {
			pPrev->m_grouping = GROUPING_HEAD;
			if (pPrev->m_bLoaded)
				pPrev->setText();
		}
		m_grouping = GROUPING_ITEM;
		if (m_bLoaded)
			setText();
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

int ItemData::calcHeight(int width)
{
	if (m_doc == nullptr)
		setText();

	int cx = width - 2;
	int xPos = 2;

	if (!pOwner->bReadOnly) {
		if (g_plugin.bShowType)	// Message type icon
			xPos += 18;

		if (g_plugin.bShowDirection)	// Message direction icon
			xPos += 18;

		if (dbe.flags & DBEF_BOOKMARK) // Bookmark icon
			xPos += 18;

		if (pOwner->m_hContact == INVALID_CONTACT_ID) // Protocol icon
			xPos += 18;

		cx -= xPos;
		if (m_bOfflineDownloaded != 0 || m_bDelivered || m_bRemoteRead) // Download completed icon
			cx -= 18;
	}

	leftOffset = xPos;
	if (m_doc && savedHeight == -1) {
		m_doc->render(cx);
		savedHeight = m_doc->height() + 5;
	}

	return xPos;
}

bool ItemData::fetch(void)
{
	// if this event is virtual (for example, in group chats), don't try to load it
	if (!dbe.getEvent())
		return false;

	if (!dbe) {
		if (!dbe.fetch())
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

const char* ItemData::getUrl() const
{
	if (auto el = m_doc->get_over_element())
		if (!mir_strcmp(el->get_tagName(), "a"))
			return el->get_attr("href");

	return nullptr;
}

void ItemData::load(bool bLoadAlways)
{
	if (!bLoadAlways && m_bLoaded)
		return;

	if (!fetch())
		return;

	m_bLoaded = true;

	switch (dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		pOwner->MarkRead(this);
		wtext = dbe.getText();
		break;

	case EVENTTYPE_STATUSCHANGE:
		wtext = mir_utf8decodeW((char *)dbe.pBlob);
		break;

	case EVENTTYPE_FILE:
		{
			DB::FILE_BLOB blob(dbe);
			if (blob.isOffline()) {
				CMStringW wszDescr, buf;

				wszDescr.Append(blob.getName() ? blob.getName() : TranslateT("Unnamed"));

				if (auto *pwszDescr = blob.getDescr()) {
					wszDescr.Append(L" - ");
					wszDescr.Append(pwszDescr);
				}

				if (uint32_t size = blob.getSize())
					wszDescr.AppendFormat(TranslateT(" %u KB"), size < 1024 ? 1 : unsigned(blob.getSize() / 1024));

				m_bOfflineFile = true;
				if (blob.isCompleted()) {
					m_bOfflineDownloaded = 100;

					auto *pwszLocalName = blob.getLocalName();
					if (g_plugin.bShowPreview && ProtoGetAvatarFileFormat(pwszLocalName) != PA_FORMAT_UNKNOWN)
						buf.AppendFormat(L"[img=file://%s]%s[/img]%s", pwszLocalName, wszDescr.c_str(), wszDescr.c_str());
					else
						buf.AppendFormat(L"[url]file://%s[/url] %s", pwszLocalName, wszDescr.c_str());
				}
				else {
					buf += wszDescr;
					m_bOfflineDownloaded = uint8_t(100.0 * blob.getTransferred() / blob.getSize());
				}

				wtext = buf.Detach();
				pOwner->MarkRead(this);
				break;
			}

			CMStringW wszFileName;
			if (!PathIsAbsoluteW(blob.getName())) {
				wchar_t buf[MAX_PATH];
				File::GetReceivedFolder(dbe.hContact, buf, _countof(buf));
				wszFileName = buf;
			}
			wszFileName.Append(blob.getName());

			wszFileName.Replace('\\', '/');
			wszFileName.Insert(0, L"[url]file://");
			wszFileName.Append(L"[/url]");
			wtext = wszFileName.Detach();
		}
		break;

	default:
		wtext = dbe.getText();
		break;
	}

	if (dbe.szReplyId)
		if (MEVENT hReply = db_event_getById(dbe.szModule, dbe.szReplyId)) {
			DB::EventInfo dbei(hReply);
			if (dbei) {
				CMStringW str, wszNick;

				wchar_t wszTime[100];
				TimeZone_PrintTimeStamp(0, dbei.timestamp, L"D t", wszTime, _countof(wszTime), 0);

				if (Contact::IsGroupChat(dbe.hContact) && dbei.szUserId)
					wszNick = Utf2T(dbei.szUserId);
				else if (dbei.flags & DBEF_SENT) {
					if (char *szProto = Proto_GetBaseAccountName(dbe.hContact))
						wszNick = ptrW(Contact::GetInfo(CNF_DISPLAY, 0, szProto));
					else
						wszNick = TranslateT("I"); // shall never happen
				}
				else wszNick = Clist_GetContactDisplayName(dbe.hContact, 0);
				
				str.AppendFormat(L"%s %s %s:\n", wszTime, wszNick.c_str(), TranslateT("wrote"));

				ptrW wszText(dbei.getText());
				if (mir_wstrlen(wszText) > 43)
					wcscpy(wszText.get() + 40, L"...");
				str.Append(wszText);
				qtext = str.Detach();
			}
		}

	dbe.unload();
}

void ItemData::setText(const wchar_t *pwszText)
{
	savedHeight = -1;

	T2Utf szFormattedBody(formatHtml(pwszText));
	if (szFormattedBody)
		m_doc = litehtml::document::createFromString(litehtml::estring(szFormattedBody.get(), litehtml::encoding::utf_8), &pOwner->webPage);
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

void HistoryArray::addChatEvent(NewstoryListData *pOwner, SESSION_INFO *si, const LOGINFO *lin)
{
	int numItems = getCount();
	auto &p = allocateItem();
	p.pOwner = pOwner;
	p.dbe.hContact = si->hContact;

	if (si->pMI->bDatabase && lin->hEvent) {
		p.dbe = lin->hEvent;
		checkGC(p, si);
	}
	else {
		CMStringW wszText;
		bool bTextUsed = Chat_GetDefaultEventDescr(si, lin, wszText);
		if (!bTextUsed && lin->ptszText) {
			if (!wszText.IsEmpty())
				wszText.AppendChar(' ');
			wszText.Append(lin->ptszText);
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
		addNick(p, lin->ptszNick);

		p.checkPrevGC((numItems == 0) ? nullptr : get(numItems - 1));
	}
}

void HistoryArray::addEvent(NewstoryListData *pOwner, MCONTACT hContact, MEVENT hEvent, int count, bool bNew)
{
	if (count == -1)
		count = MAXINT;

	int numItems = getCount();
	auto *pPrev = (numItems == 0) ? nullptr : get(numItems - 1);
	
	SESSION_INFO *si = nullptr;
	bool isChat = Contact::IsGroupChat(hContact);
	if (isChat)
		si = Chat_Find(hContact);

	if (count == 1) {
		auto &p = allocateItem();
		p.pOwner = pOwner;
		p.dbe.hContact = hContact;
		p.dbe = hEvent;
		p.m_bNew = bNew;
		if (isChat) {
			checkGC(p, si);
			pPrev = p.checkPrevGC(pPrev);
		}
		else pPrev = p.checkPrev(pPrev);
	}
	else {
		DB::ECPTR pCursor(DB::Events(hContact, hEvent));
		for (int i = 0; i < count; i++) {
			hEvent = pCursor.FetchNext();
			if (!hEvent)
				break;

			auto &p = allocateItem();
			p.pOwner = pOwner;
			p.dbe.hContact = hContact;
			p.dbe = hEvent;
			p.m_bNew = bNew;
			if (isChat) {
				checkGC(p, si);
				pPrev = p.checkPrevGC(pPrev);
			}
			else pPrev = p.checkPrev(pPrev);
		}
	}
}

void HistoryArray::addNick(ItemData &pItem, wchar_t *pwszNick)
{
	pItem.wszNick = strings.find(pwszNick);
	if (pItem.wszNick == nullptr) {
		pItem.wszNick = mir_wstrdup(pwszNick);
		strings.insert(pItem.wszNick);
	}
}

void HistoryArray::addResults(NewstoryListData *pOwner, const OBJLIST<SearchResult> &pArray)
{
	int numItems = getCount();
	auto *pPrev = (numItems == 0) ? nullptr : get(numItems - 1);

	for (auto &it : pArray) {
		auto &p = allocateItem();
		p.pOwner = pOwner;
		p.dbe.hContact = it->hContact;
		p.dbe = it->hEvent;
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

void HistoryArray::checkGC(ItemData &p, SESSION_INFO *si)
{
	p.fetch();
	if (p.dbe.szUserId) {
		Utf2T wszUser(p.dbe.szUserId);
		if (auto *pUser = g_chatApi.UM_FindUser(si, wszUser))
			addNick(p, pUser->pszNick);
		else {
			if (si == nullptr) {
				MCONTACT hContact = INVALID_CONTACT_ID;
				auto *szProto = Proto_GetBaseAccountName(p.dbe.hContact);

				if (gcCache.size() == 0) {
					ptrW wszNick(Contact::GetInfo(CNF_UNIQUEID, 0, szProto));
					gcCache[wszNick.get()] = 0;
				}
			
				auto pCache = gcCache.find(wszUser.get());
				if (pCache == gcCache.end()) {
					for (auto &cc : Contacts(szProto)) {
						ptrW wszId(Contact::GetInfo(CNF_UNIQUEID, cc));
						if (!mir_wstrcmp(wszId, wszUser)) {
							gcCache[wszId.get()] = cc;
							hContact = cc;
							break;
						}
					}
				}
				else hContact = (*pCache).second;

				if (hContact != INVALID_CONTACT_ID) {
					if (hContact == 0)
						addNick(p, ptrW(Contact::GetInfo(CNF_DISPLAY, 0, szProto)));
					else
						addNick(p, Clist_GetContactDisplayName(hContact));
					return;
				}
			}

			addNick(p, wszUser);
		}
	}
}

int HistoryArray::find(MEVENT hEvent)
{
	int i = 0;
	for (auto &it : pages)
		for (auto &p : it->data) {
			if (p.dbe.getEvent() == hEvent)
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

ItemData* HistoryArray::insert(int pos)
{
	int count = getCount();
	ItemData *pNew = &allocateItem();

	if (count > 0) {
		ItemData *pPrev = get(count - 1, false);
		for (int i = count; i >= pos; i--) {
			memcpy(pNew, pPrev, sizeof(ItemData));
			pNew = pPrev;
			pPrev = get(i - 1, false);
		}
	}

	ItemData tmp;
	memcpy(pNew, &tmp, sizeof(tmp));
	return pNew;
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

	int nPages = pages.getCount()-1;
	for (int i = pageNo + 1; i <= nPages; i++) {
		auto &prev = pages[i - 1], &curr = pages[i];
		memcpy(&prev.data[HIST_BLOCK_SIZE - 1], curr.data, sizeof(ItemData));
		memmove(&curr.data, &curr.data[1], sizeof(ItemData) * (HIST_BLOCK_SIZE - 1));
		memset(&curr.data[HIST_BLOCK_SIZE - 1], 0, sizeof(ItemData));
	}

	if (iLastPageCounter == 1) {
		pages.remove(nPages);
		iLastPageCounter = HIST_BLOCK_SIZE;
	}
	else {
		iLastPageCounter--;
		auto &pLast = pages[nPages].data[iLastPageCounter];
		memset(&pLast, 0, sizeof(ItemData));
		pLast.savedHeight = -1;
	}
}
