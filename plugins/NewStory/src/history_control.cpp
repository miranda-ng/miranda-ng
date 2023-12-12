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

#define AVERAGE_ITEM_HEIGHT 100

HANDLE htuLog = 0;

void InitHotkeys()
{
	HOTKEYDESC hkd = {};
	hkd.szSection.a = MODULENAME;

	hkd.szDescription.a = LPGEN("Toggle bookmark");
	hkd.pszName = "ns_bookmark";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'B') | HKF_MIRANDA_LOCAL;
	hkd.lParam = HOTKEY_BOOKMARK;
	g_plugin.addHotkey(&hkd);

	hkd.szDescription.a = LPGEN("Search");
	hkd.pszName = "ns_search";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'S') | HKF_MIRANDA_LOCAL;
	hkd.lParam = HOTKEY_SEARCH;
	g_plugin.addHotkey(&hkd);

	hkd.szDescription.a = LPGEN("Search forward");
	hkd.pszName = "ns_seek_forward";
	hkd.DefHotKey = HOTKEYCODE(0, VK_F3) | HKF_MIRANDA_LOCAL;
	hkd.lParam = HOTKEY_SEEK_FORWARD;
	g_plugin.addHotkey(&hkd);

	hkd.szDescription.a = LPGEN("Search backward");
	hkd.pszName = "ns_seek_back";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT, VK_F3) | HKF_MIRANDA_LOCAL;
	hkd.lParam = HOTKEY_SEEK_BACK;
	g_plugin.addHotkey(&hkd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Control utilities, types and constants

NewstoryListData::NewstoryListData(HWND _1) :
	m_hwnd(_1),
	redrawTimer(Miranda_GetSystemWindow(), (LPARAM)this)
{
	bSortAscending = g_plugin.bSortAscending;

	redrawTimer.OnEvent = Callback(this, &NewstoryListData::OnTimer);
}

void NewstoryListData::OnContextMenu(int index, POINT pt)
{
	HMENU hMenu = NSMenu_Build(this, (index == -1) ? 0 : LoadItem(index));

	if (pMsgDlg != nullptr && pMsgDlg->isChat())
		Chat_CreateMenu(hMenu, pMsgDlg->getChat(), nullptr);

	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
	Menu_DestroyNestedMenu(hMenu);
}

void NewstoryListData::OnResize(int newWidth, int newHeight)
{
	bool bDraw = false;
	if (newWidth != cachedWindowWidth) {
		cachedWindowWidth = newWidth;
		for (int i = 0; i < totalCount; i++)
			LoadItem(i)->savedHeight = -1;
		bDraw = true;
	}

	if (newHeight != cachedWindowHeight) {
		cachedWindowHeight = newHeight;
		FixScrollPosition(true);
		bDraw = true;
	}

	if (bDraw)
		InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::OnTimer(CTimer *pTimer)
{
	pTimer->Stop();

	if (bWasAtBottom)
		EnsureVisible(totalCount - 1);

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::AddChatEvent(SESSION_INFO *si, const LOGINFO *lin)
{
	ScheduleDraw();
	items.addChatEvent(si, lin);
	totalCount++;
}

void NewstoryListData::AddEvent(MCONTACT hContact, MEVENT hFirstEvent, int iCount)
{
	ScheduleDraw();
	items.addEvent(hContact, hFirstEvent, iCount);
	totalCount = items.getCount();
}

void NewstoryListData::AddResults(const OBJLIST<SearchResult> &results)
{
	ScheduleDraw();
	items.addResults(results);
	totalCount = items.getCount();
}

void NewstoryListData::AddSelection(int iFirst, int iLast)
{
	int start = min(totalCount - 1, iFirst);
	int end = min(totalCount - 1, max(0, iLast));
	if (start > end)
		std::swap(start, end);

	for (int i = start; i <= end; ++i)
		if (auto *p = GetItem(i))
			p->m_bSelected = true;

	InvalidateRect(m_hwnd, 0, FALSE);
}

bool NewstoryListData::AtBottom(void) const
{
	if (scrollTopItem > cachedMaxTopItem)
		return true;

	if (scrollTopItem == cachedMaxTopItem && cachedMaxTopPixel >= scrollTopPixel)
		return true;

	return false;
}

bool NewstoryListData::AtTop(void) const
{
	if (scrollTopItem < 0)
		return true;

	if (scrollTopItem == 0 && scrollTopPixel == 0)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Edit box window procedure

static LRESULT CALLBACK HistoryEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto *pData = (NewstoryListData *)GetWindowLongPtr(GetParent(hwnd), 0);

	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			pData->EndEditItem(true);
			return 0;
		case VK_ESCAPE:
			pData->EndEditItem(false);
			return 0;
		}
		break;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg2 = (MSG *)lParam;
			if (msg2->message == WM_KEYDOWN && msg2->wParam == VK_TAB)
				return 0;
			if (msg2->message == WM_CHAR && msg2->wParam == '\t')
				return 0;
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		pData->EndEditItem(false);
		return 0;
	}

	return mir_callNextSubclass(hwnd, HistoryEditWndProc, msg, wParam, lParam);
}

void NewstoryListData::BeginEditItem()
{
	if (hwndEditBox)
		EndEditItem(false);

	if (scrollTopItem > caret)
		return;

	ItemData *item = LoadItem(caret);
	if (item->dbe.eventType != EVENTTYPE_MESSAGE)
		return;

	RECT rc; GetClientRect(m_hwnd, &rc);
	int height = rc.bottom - rc.top;

	int top = scrollTopPixel;
	int idx = scrollTopItem;
	int itemHeight = GetItemHeight(idx);
	while (top < height) {
		if (idx == caret)
			break;

		top += itemHeight;
		idx++;
		itemHeight = GetItemHeight(idx);
	}

	int fontid, colorid;
	item->getFontColor(fontid, colorid);

	// #4012 make sure that both single & double CRLF are now double
	CMStringW wszText(item->getWBuf());
	wszText.Replace(L"\r\n", L"\n");
	wszText.Replace(L"\n", L"\r\n");

	uint32_t dwStyle = WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL;
	hwndEditBox = CreateWindow(L"EDIT", wszText, dwStyle, 0, top, rc.right - rc.left, itemHeight, m_hwnd, NULL, g_plugin.getInst(), NULL);
	mir_subclassWindow(hwndEditBox, HistoryEditWndProc);
	SendMessage(hwndEditBox, WM_SETFONT, (WPARAM)g_fontTable[fontid].hfnt, 0);
	SendMessage(hwndEditBox, EM_SETMARGINS, EC_RIGHTMARGIN, 100);
	ShowWindow(hwndEditBox, SW_SHOW);
	SetFocus(hwndEditBox);
}

/////////////////////////////////////////////////////////////////////////////////////////

void NewstoryListData::CalcBottom()
{
	int maxTopItem = totalCount, tmp = 0;
	while (maxTopItem > 0 && tmp < cachedWindowHeight)
		tmp += GetItemHeight(--maxTopItem);
	cachedMaxTopItem = maxTopItem;
	cachedMaxTopPixel = (cachedWindowHeight < tmp) ? cachedWindowHeight - tmp : 0;
}

void NewstoryListData::Clear()
{
	items.clear();
	totalCount = 0;
	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ClearSelection(int iFirst, int iLast)
{
	int start = min(0, iFirst);
	int end = (iLast <= 0) ? totalCount - 1 : iLast;
	if (start > end)
		std::swap(start, end);

	for (int i = start; i <= end; ++i)
		if (auto *pItem = GetItem(i))
			pItem->m_bSelected = false;

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::Copy(bool bTextOnly)
{
	Utils_ClipboardCopy(GatherSelected(bTextOnly));
}

void NewstoryListData::CopyUrl()
{
	if (auto *pItem = GetItem(caret)) {
		if (pItem->m_bOfflineDownloaded) {
			DB::EventInfo dbei(pItem->hEvent);
			DB::FILE_BLOB blob(dbei);
			Utils_ClipboardCopy(blob.getLocalName());
		}
		else Srmm_DownloadOfflineFile(pItem->hContact, pItem->hEvent, OFD_COPYURL);
	}
}

void NewstoryListData::DeleteItems(void)
{
	if (IDYES != MessageBoxW(m_hwnd, TranslateT("Are you sure to remove selected event(s)?"), _T(MODULETITLE), MB_YESNOCANCEL | MB_ICONQUESTION))
		return;

	g_plugin.bDisableDelete = true;

	int firstSel = -1;
	for (int i = totalCount - 1; i >= 0; i--) {
		auto *p = GetItem(i);
		if (!p->m_bSelected)
			continue;

		if (p->hEvent)
			db_event_delete(p->hEvent);
		items.remove(i);
		totalCount--;
		firstSel = i;
	}

	g_plugin.bDisableDelete = false;

	if (firstSel != -1) {
		SetCaret(firstSel, false);
		SetSelection(firstSel, firstSel);
		FixScrollPosition(true);
	}
}

void NewstoryListData::Download(int options)
{
	if (auto *p = LoadItem(caret))
		Srmm_DownloadOfflineFile(p->hContact, p->hEvent, options);
}

void NewstoryListData::EndEditItem(bool bAccept)
{
	if (hwndEditBox == nullptr)
		return;

	if (bAccept) {
		if ((GetWindowLong(hwndEditBox, GWL_STYLE) & ES_READONLY) == 0) {
			auto *pItem = GetItem(caret);

			int iTextLen = GetWindowTextLengthW(hwndEditBox);
			mir_free(pItem->wtext);
			pItem->wtext = (wchar_t *)mir_alloc((iTextLen + 1) * sizeof(wchar_t));
			GetWindowTextW(hwndEditBox, pItem->wtext, iTextLen+1);
			pItem->wtext[iTextLen] = 0;

			if (pItem->hContact && pItem->hEvent) {
				DBEVENTINFO dbei = pItem->dbe;

				ptrA szUtf(mir_utf8encodeW(pItem->wtext));
				dbei.cbBlob = (int)mir_strlen(szUtf) + 1;
				dbei.pBlob = (uint8_t *)szUtf.get();
				db_event_edit(pItem->hEvent, &dbei);
			}

			MTextDestroy(pItem->data); pItem->data = 0;
			pItem->savedHeight = -1;
			pItem->checkCreate(m_hwnd);
		}
	}

	DestroyWindow(hwndEditBox);
	hwndEditBox = nullptr;
}

void NewstoryListData::EnsureVisible(int item)
{
	if (scrollTopItem >= item) {
		scrollTopItem = item;
		scrollTopPixel = 0;
	}
	else {
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		int height = rc.bottom - rc.top;
		int idx = scrollTopItem;
		int itemHeight = GetItemHeight(idx);
		int top = itemHeight + scrollTopPixel;
		bool found = false;
		while (top < height) {
			if (idx == item) {
				itemHeight = GetItemHeight(idx);
				found = true;
				break;
			}
			top += itemHeight;
			idx++;
			itemHeight = GetItemHeight(idx);
		}
		if (!found) {
			scrollTopItem = item;
			scrollTopPixel = 0;
		}
	}
	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

int NewstoryListData::FindNext(const wchar_t *pwszText)
{
	int idx = items.FindNext(caret, Filter(Filter::EVENTONLY, pwszText));
	if (idx == -1 && caret > 0)
		idx = items.FindNext(-1, Filter(Filter::EVENTONLY, pwszText));

	if (idx >= 0) {
		SetSelection(idx, idx);
		SetCaret(idx);
	}
	return idx;
}

int NewstoryListData::FindPrev(const wchar_t *pwszText)
{
	int idx = items.FindPrev(caret, Filter(Filter::EVENTONLY, pwszText));
	if (idx == -1 && caret != totalCount - 1)
		idx = items.FindPrev(totalCount, Filter(Filter::EVENTONLY, pwszText));

	if (idx >= 0) {
		SetSelection(idx, idx);
		SetCaret(idx);
	}
	return idx;
}

void NewstoryListData::FixScrollPosition(bool bForce)
{
	EndEditItem(false);

	if (bForce || cachedMaxTopItem != scrollTopItem)
		CalcBottom();

	if (scrollTopItem < 0)
		scrollTopItem = 0;

	if (bForce || scrollTopItem > cachedMaxTopItem || (scrollTopItem == cachedMaxTopItem && scrollTopPixel < cachedMaxTopPixel)) {
		scrollTopItem = cachedMaxTopItem;
		scrollTopPixel = cachedMaxTopPixel;
	}
}

CMStringW NewstoryListData::GatherSelected(bool bTextOnly)
{
	CMStringW ret;

	int eventCount = totalCount;
	for (int i = 0; i < eventCount; i++) {
		ItemData *p = GetItem(i);
		if (!p->m_bSelected)
			continue;

		CMStringW wszText(bTextOnly ? p->wtext : p->formatString());
		RemoveBbcodes(wszText);
		ret.Append(wszText);
		ret.Append(L"\r\n");
	}

	return ret;
}

ItemData* NewstoryListData::GetItem(int idx) const
{
	if (totalCount == 0)
		return nullptr;

	return (bSortAscending) ? items.get(idx, false) : items.get(totalCount - 1 - idx, false);
}

int NewstoryListData::GetItemFromPixel(int yPos)
{
	if (!totalCount)
		return -1;

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	int height = rc.bottom - rc.top;
	int current = scrollTopItem;
	int top = scrollTopPixel;
	int bottom = top + GetItemHeight(current);
	while (top <= height) {
		if (yPos >= top && yPos <= bottom)
			return current;
		if (++current >= totalCount)
			break;
		top = bottom;
		bottom = top + GetItemHeight(current);
	}

	return -1;
}

int NewstoryListData::GetItemHeight(int index)
{
	auto *item = LoadItem(index);
	if (!item)
		return 0;

	if (item->savedHeight >= 0)
		return item->savedHeight;

	int fontid, colorid;
	item->getFontColor(fontid, colorid);
	item->checkCreate(m_hwnd);

	HDC hdc = GetDC(m_hwnd);
	HFONT hOldFont = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);

	RECT rc; GetClientRect(m_hwnd, &rc);
	int width = rc.right - rc.left;

	SIZE sz = { width - 6, 0 };
	MTextMeasure(hdc, &sz, item->data);

	SelectObject(hdc, hOldFont);
	ReleaseDC(m_hwnd, hdc);
	return item->savedHeight = sz.cy + 5;
}

bool NewstoryListData::HasSelection() const
{
	for (int i = 0; i < totalCount; i++)
		if (auto *p = GetItem(i))
			if (p->m_bSelected)
				return true;

	return false;
}

void NewstoryListData::HitTotal(int y)
{
	int i = 0;
	while (i < totalCount && y > 0)
		y -= GetItemHeight(i++);

	scrollTopItem = i;
	scrollTopPixel = y;
	FixScrollPosition();
}

ItemData* NewstoryListData::LoadItem(int idx)
{
	if (totalCount == 0)
		return nullptr;

	return (bSortAscending) ? items.get(idx, true) : items.get(totalCount - 1 - idx, true);
}

void NewstoryListData::OpenFolder()
{
	if (auto *pItem = GetItem(caret)) {
		if (pItem->m_bOfflineDownloaded) {
			DB::EventInfo dbei(pItem->hEvent);
			DB::FILE_BLOB blob(dbei);
			CMStringW wszFile(blob.getLocalName());
			int idx = wszFile.ReverseFind('\\');
			if (idx != -1)
				wszFile.Truncate(idx);
			::ShellExecute(nullptr, L"open", wszFile, nullptr, nullptr, SW_SHOWNORMAL);
		}
	}
}

int NewstoryListData::PaintItem(HDC hdc, int index, int top, int width)
{
	auto *item = LoadItem(index);
	item->savedTop = top;

	// remove any selections that might be created by the BBCodes parser
	MTextSendMessage(0, item->data, EM_SETSEL, 0, 0);

	//	LOGFONT lfText;
	COLORREF clText, clBack, clLine;
	int fontid, colorid;
	item->getFontColor(fontid, colorid);

	clText = g_fontTable[fontid].cl;
	if (item->m_bHighlighted) {
		clText = g_fontTable[FONT_HIGHLIGHT].cl;
		clBack = g_colorTable[COLOR_HIGHLIGHT_BACK].cl;
		clLine = g_colorTable[COLOR_FRAME].cl;
	}
	else if (item->m_bSelected) {
		clText = g_colorTable[COLOR_SELTEXT].cl;
		clBack = g_colorTable[COLOR_SELBACK].cl;
		clLine = g_colorTable[COLOR_SELFRAME].cl;
	}
	else {
		clLine = g_colorTable[COLOR_FRAME].cl;
		clBack = g_colorTable[colorid].cl;
	}

	item->checkCreate(m_hwnd);

	SIZE sz;
	sz.cx = width - 6;
	HFONT hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
	MTextMeasure(hdc, &sz, item->data);
	SelectObject(hdc, hfnt);
	int height = sz.cy + 5;

	RECT rc;
	SetRect(&rc, 0, top, width, top + height);

	HBRUSH hbr = CreateSolidBrush(clBack);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);

	SetTextColor(hdc, clText);
	SetBkMode(hdc, TRANSPARENT);

	POINT pos;
	pos.x = 3;
	pos.y = top + 2;
	hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
	MTextDisplay(hdc, pos, sz, item->data);
	SelectObject(hdc, hfnt);

	HPEN hpn = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 1, clLine));
	MoveToEx(hdc, rc.left, rc.bottom - 1, 0);
	LineTo(hdc, rc.right, rc.bottom - 1);
	DeleteObject(SelectObject(hdc, hpn));
	return height;
}

void NewstoryListData::RecalcScrollBar()
{
	if (totalCount == 0)
		return;

	int yTotal = 0, yTop = 0;
	for (int i = 0; i < totalCount; i++) {
		if (i == scrollTopItem)
			yTop = yTotal - scrollTopPixel;
		yTotal += GetItemHeight(i);
	}

	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = yTotal;
	si.nPage = cachedWindowHeight;
	si.nPos = yTop;

	if (si.nPos != cachedScrollbarPos || si.nMax != cachedScrollbarMax) {
		cachedScrollbarPos = si.nPos;
		cachedScrollbarMax = si.nMax;
		SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
	}
}

void NewstoryListData::Quote()
{
	if (pMsgDlg) {
		CMStringW wszText(GatherSelected(true));
		RemoveBbcodes(wszText);
		pMsgDlg->SetMessageText(Srmm_Quote(wszText));

		SetFocus(pMsgDlg->GetInput());
	}
}

void NewstoryListData::Reply()
{
	if (pMsgDlg)
		if (auto *pItem = GetItem(caret))
			pMsgDlg->SetQuoteEvent(pItem->hEvent);
}

void NewstoryListData::ScheduleDraw()
{
	bWasAtBottom = AtBottom();

	redrawTimer.Stop();
	redrawTimer.Start(30);
}

void NewstoryListData::SetCaret(int idx, bool bEnsureVisible)
{
	if (idx < totalCount) {
		caret = idx;
		if (bEnsureVisible)
			EnsureVisible(idx);
	}
}
void NewstoryListData::SetContact(MCONTACT hContact)
{
	WindowList_Add(g_hNewstoryLogs, m_hwnd, hContact);
}

void NewstoryListData::SetDialog(CSrmmBaseDialog *pDlg)
{
	if (pMsgDlg = pDlg)
		SetContact(pDlg->m_hContact);
}

void NewstoryListData::SetPos(int pos)
{
	SetSelection((selStart == -1) ? pos : selStart, pos);
	SetCaret(pos);
}

void NewstoryListData::SetSelection(int iFirst, int iLast)
{
	int start = min(totalCount - 1, iFirst);
	int end = min(totalCount - 1, max(0, iLast));
	if (start > end)
		std::swap(start, end);

	int count = totalCount;
	for (int i = 0; i < count; ++i) {
		auto *p = GetItem(i);
		if (i >= start && i <= end)
			p->m_bSelected = true;
		else
			p->m_bSelected = false;
	}

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ToggleBookmark()
{
	int eventCount = totalCount;
	for (int i = 0; i < eventCount; i++) {
		ItemData *p = GetItem(i);
		if (!p->m_bSelected)
			continue;

		if (p->dbe.flags & DBEF_BOOKMARK)
			p->dbe.flags &= ~DBEF_BOOKMARK;
		else
			p->dbe.flags |= DBEF_BOOKMARK;
		db_event_edit(p->hEvent, &p->dbe);

		p->setText();
	}

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ToggleSelection(int iFirst, int iLast)
{
	int start = min(totalCount - 1, iFirst);
	int end = min(totalCount - 1, max(0, iLast));
	if (start > end)
		std::swap(start, end);

	for (int i = start; i <= end; ++i) {
		auto *p = GetItem(i);
		p->m_bSelected = !p->m_bSelected;
	}

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::TryUp(int iCount)
{
	if (totalCount == 0)
		return;

	auto *pTop = GetItem(0);
	MCONTACT hContact = pTop->hContact;
	if (pTop->hEvent == 0 || hContact == 0)
		return;
	
	int i;
	for (i = 0; i < iCount; i++) {
		MEVENT hPrev = db_event_prev(hContact, pTop->hEvent);
		if (hPrev == 0)
			break;

		auto *p = items.insert(0);
		p->hContact = hContact;
		p->hEvent = hPrev;
		totalCount++;
	}

	ItemData *pPrev = nullptr;
	for (int j = 0; j < i + 1; j++) {
		auto *pItem = GetItem(j);
		pPrev = pItem->checkNext(pPrev);
	}

	caret = 0;
	CalcBottom();
	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Navigation by coordinates

void NewstoryListData::LineUp()
{
	if (AtTop())
		TryUp(1);
	else
		ScrollUp(10);
}

void NewstoryListData::LineDown()
{
	if (!AtBottom())
		ScrollDown(10);
}

void NewstoryListData::PageUp()
{
	if (AtTop())
		TryUp(10);
	else
		ScrollUp(cachedWindowHeight);
}

void NewstoryListData::PageDown()
{
	if (!AtBottom())
		ScrollDown(cachedWindowHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Navigation by events

void NewstoryListData::EventUp()
{
	if (caret == 0)
		TryUp(1);
	else
		SetPos(caret - 1);
}

void NewstoryListData::EventDown()
{
	if (caret < totalCount-1)
		SetPos(caret + 1);
}

void NewstoryListData::EventPageUp()
{
	if (caret >= 10)
		SetPos(caret - 10);
	else
		TryUp(caret == 10 ? 1 : 10 - caret);
}

void NewstoryListData::EventPageDown()
{
	if (caret < totalCount - 10)
		SetPos(caret + 10);
	else
		SetPos(totalCount - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Common navigation functions

void NewstoryListData::ScrollBottom()
{
	if (!totalCount)
		return;

	scrollTopItem = cachedMaxTopItem;
	scrollTopPixel = cachedMaxTopPixel;
	FixScrollPosition(true);
	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ScrollDown(int deltaY)
{
	int iHeight = GetItemHeight(scrollTopItem) + scrollTopPixel;
	if (iHeight > deltaY)
		scrollTopPixel -= deltaY;
	else {
		deltaY -= iHeight;

		bool bFound = false;
		for (int i = scrollTopItem + 1; i < totalCount; i++) {
			iHeight = GetItemHeight(i);
			if (iHeight > deltaY) {
				scrollTopPixel = -deltaY;
				scrollTopItem = i;
				bFound = true;
				break;
			}
			deltaY -= iHeight;
		}
		if (!bFound)
			scrollTopItem = scrollTopPixel = 0;
	}

	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ScrollTop()
{
	scrollTopItem = scrollTopPixel = 0;
	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ScrollUp(int deltaY)
{
	int reserveY = -scrollTopPixel; // distance in pixels between the top event beginning and the window top coordinate

	if (reserveY >= deltaY)
		scrollTopPixel += deltaY;    // stay on the same event, just move up
	else {
		deltaY -= reserveY;    // move to the appropriate event first, then calculate the gap

		bool bFound = false;
		for (int i = scrollTopItem - 1; i >= 0; i--) {
			int iHeight = GetItemHeight(i);
			if (iHeight > deltaY) {
				scrollTopPixel = deltaY - iHeight;
				scrollTopItem = i;
				bFound = true;
				break;
			}
			deltaY -= iHeight;
		}

		if (!bFound)
			scrollTopItem = scrollTopPixel = 0;
	}
	
	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// NewStory history control window procedure

LRESULT CALLBACK NewstoryListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int idx;
	POINT pt;
	NewstoryListData *data = (NewstoryListData *)GetWindowLongPtr(hwnd, 0);

	MSG message = { hwnd, msg, wParam, lParam };
	switch (Hotkey_Check(&message, MODULENAME)) {
	case HOTKEY_SEEK_FORWARD:
		PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 1);
		break;
	case HOTKEY_SEEK_BACK:
		PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDC_FINDPREV, BN_CLICKED), 1);
		break;
	case HOTKEY_SEARCH:
		PostMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(IDC_SEARCH, BN_CLICKED), 1);
		break;
	case HOTKEY_BOOKMARK:
		data->ToggleBookmark();
		return 0;
	}

	switch (msg) {
	case WM_CREATE:
		data = new NewstoryListData(hwnd);
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)data);
		if (!g_plugin.bOptVScroll)
			SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_VSCROLL);
		break;

	// History list control messages
	case NSM_SELECTITEMS:
		data->AddSelection(wParam, lParam);
		return 0;

	case NSM_SEEKTIME:
		{
			int eventCount = data->totalCount;
			for (int i = 0; i < eventCount; i++) {
				auto *p = data->GetItem(i);
				if (p->dbe.timestamp >= wParam) {
					data->SetSelection(i, i);
					data->SetCaret(i);
					break;
				}

				if (i == eventCount - 1) {
					data->SetSelection(i, i);
					data->SetCaret(i);
				}
			}
		}
		return TRUE;

	case NSM_ADDEVENT:
		data->AddEvent(wParam, lParam, 1);
		break;

	case NSM_SET_OPTIONS:
		data->bSortAscending = g_plugin.bSortAscending;
		data->scrollTopPixel = 0;
		data->FixScrollPosition(true);
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case UM_ADDEVENT:
		if (data->pMsgDlg == nullptr)
			data->AddEvent(wParam, lParam, 1);
		break;

	case UM_EDITEVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			auto *p = data->GetItem(idx);
			p->load(true);
			p->setText();
			InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case UM_REMOVEEVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			data->items.remove(idx);
			data->totalCount--;
			data->FixScrollPosition(true);
			InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case WM_SIZE:
		data->OnResize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		if (NSMenu_Process(LOWORD(wParam), data))
			return 1;
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		/* we get so many InvalidateRect()'s that there is no point painting,
		Windows in theory shouldn't queue up WM_PAINTs in this case but it does so
		we'll just ignore them */
		if (IsWindowVisible(hwnd)) {
			PAINTSTRUCT ps;
			HDC hdcWindow = BeginPaint(hwnd, &ps);

			RECT rc;
			GetClientRect(hwnd, &rc);

			HDC hdc = CreateCompatibleDC(hdcWindow);
			HBITMAP hbmSave = (HBITMAP)SelectObject(hdc, CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top));

			int height = rc.bottom - rc.top;
			int width = rc.right - rc.left;
			int top = data->scrollTopPixel;
			idx = data->scrollTopItem;
			while ((top < height) && (idx < data->totalCount))
				top += data->PaintItem(hdc, idx++, top, width);
			data->cachedMaxDrawnItem = idx;

			if (top <= height) {
				RECT rc2;
				SetRect(&rc2, 0, top, width, height);

				HBRUSH hbr = CreateSolidBrush(g_colorTable[COLOR_BACK].cl);
				FillRect(hdc, &rc2, hbr);
				DeleteObject(hbr);
			}

			if (g_plugin.bOptVScroll)
				data->RecalcScrollBar();
			if (g_plugin.bDrawEdge)
				DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

			BitBlt(hdcWindow, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(hdc, hbmSave));
			DeleteDC(hdc);
			EndPaint(hwnd, &ps);
		}
		break;

	case WM_CONTEXTMENU:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if (pt.x == -1 && pt.y == -1)
			GetCursorPos(&pt);

		POINT pt2 = pt;
		ScreenToClient(hwnd, &pt2);

		idx = data->GetItemFromPixel(pt2.y);
		if (idx != -1) {
			if (data->caret != idx)
				data->EndEditItem(false);
			data->SetCaret(idx);
			if (!data->HasSelection())
				data->SetSelection(idx, idx);
		}
		data->OnContextMenu(idx, pt);
		break;

	case WM_KILLFOCUS:
		if (wParam && (HWND)wParam != data->hwndEditBox)
			data->EndEditItem(false);
		if (data->pMsgDlg && ((HWND)wParam == data->pMsgDlg->GetInput() || (HWND)wParam == data->pMsgDlg->GetHwnd()))
			data->ClearSelection(0, -1);
		return 0;

	case WM_SETFOCUS:
		return 0;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg2 = (MSG *)lParam;
			if (msg2->message == WM_KEYDOWN) {
				if (msg2->wParam == VK_TAB)
					return 0;
				if (msg2->wParam == VK_ESCAPE && !data->hwndEditBox)
					return 0;
			}
			else if (msg2->message == WM_CHAR) {
				if (msg2->wParam == '\t')
					return 0;
				if (msg2->wParam == 27 && !data->hwndEditBox)
					return 0;
			}
		}
		return DLGC_WANTMESSAGE;

	case WM_KEYDOWN:
		{
			bool isShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
			bool isCtrl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

			if (!data->bWasShift && isShift)
				data->selStart = data->caret;
			else if (data->bWasShift && !isShift)
				data->selStart = -1;
			data->bWasShift = isShift;

			switch (wParam) {
			case VK_UP:
				if (g_plugin.bHppCompat)
					data->EventUp();
				else
					data->LineUp();
				break;

			case VK_DOWN:
				if (g_plugin.bHppCompat)
					data->EventDown();
				else
					data->LineDown();
				break;

			case VK_PRIOR:
				if (isCtrl)
					data->ScrollTop();
				else if (g_plugin.bHppCompat)
					data->EventPageUp();
				else
					data->PageUp();
				break;

			case VK_NEXT:
				if (isCtrl)
					data->ScrollBottom();
				else if (g_plugin.bHppCompat)
					data->EventPageDown();
				else
					data->PageDown();
				break;

			case VK_HOME:
				data->ScrollTop();
				break;

			case VK_END:
				data->ScrollBottom();
				break;

			case VK_F2:
				data->BeginEditItem();
				break;

			case VK_ESCAPE:
				if (data->hwndEditBox)
					data->EndEditItem(false);
				break;

			case VK_DELETE:
				data->DeleteItems();
				break;

			case VK_INSERT:
			case 'C':
				if (isCtrl)
					data->Copy();
				break;

			case 'A':
				if (isCtrl)
					data->AddSelection(0, data->totalCount);
				break;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			if (data->caret != idx)
				data->EndEditItem(false);

			auto *pItem = data->LoadItem(idx);

			if (wParam & MK_CONTROL) {
				data->ToggleSelection(idx, idx);
				data->SetCaret(idx);
			}
			else if (wParam & MK_SHIFT) {
				data->AddSelection(data->caret, idx);
				data->SetCaret(idx);
			}
			else {
				pt.y -= pItem->savedTop;

				CMStringW wszUrl;
				if (pItem->isLink(pt, &wszUrl)) {
					Utils_OpenUrlW(wszUrl);
					return 0;
				}

				data->selStart = idx;
				data->SetSelection(idx, idx);
				data->SetCaret(idx);
			}
		}
		SetFocus(hwnd);
		return 0;

	case WM_LBUTTONUP:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0)
			data->selStart = -1;
		break;

	case WM_LBUTTONDBLCLK:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			if (data->caret != idx)
				data->EndEditItem(false);

			auto *pItem = data->LoadItem(idx);
			pt.y -= pItem->savedTop;

			if (pItem->m_bOfflineFile) {
				Srmm_DownloadOfflineFile(pItem->hContact, pItem->hEvent, OFD_DOWNLOAD | OFD_RUN);
				return 0;
			}

			if (data->caret == idx) {
				data->BeginEditItem();
				return 0;
			}
		}

		SetFocus(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			auto *pItem = data->LoadItem(idx);
			MTextSendMessage(hwnd, pItem->data, msg, wParam, lParam);

			HCURSOR hOldCursor = GetCursor();
			HCURSOR hNewCursor = LoadCursor(0, (pItem->isLink(pt) || pItem->m_bOfflineFile) ? IDC_HAND : IDC_ARROW);
			if (hOldCursor != hNewCursor)
				SetCursor(hNewCursor);

			if (data->selStart != -1) {
				data->SetSelection(data->selStart, idx);
				InvalidateRect(hwnd, 0, FALSE);
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if ((short)HIWORD(wParam) < 0)
			data->LineDown();
		else
			data->LineUp();
		return TRUE;

	case WM_VSCROLL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			switch (LOWORD(wParam)) {
			case SB_LINEUP:
				if (g_plugin.bHppCompat)
					data->EventUp();
				else
					data->LineUp();
				break;

			case SB_LINEDOWN:
				if (g_plugin.bHppCompat)
					data->EventDown();
				else
					data->LineDown();
				break;

			case SB_PAGEUP:
				if (g_plugin.bHppCompat)
					data->EventPageUp();
				else
					data->PageUp();
				break;

			case SB_PAGEDOWN:
				if (g_plugin.bHppCompat)
					data->EventPageDown();
				else
					data->PageDown();
				break;

			case SB_BOTTOM:
				data->ScrollBottom();
				break;

			case SB_TOP:
				data->ScrollTop();
				break;

			case SB_THUMBTRACK:
				SCROLLINFO si;
				si.cbSize = sizeof(si);
				si.fMask = SIF_ALL;
				GetScrollInfo(hwnd, SB_VERT, &si);
				data->HitTotal(si.nTrackPos);
				break;

			default:
				return 0;
			}

			if (s_scrollTopItem != data->scrollTopItem || s_scrollTopPixel != data->scrollTopPixel)
				InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
		if (lParam == INT_PTR(data->hwndEditBox)) {
			SetBkColor((HDC)wParam, g_colorTable[COLOR_SELBACK].cl);
			return (LRESULT)g_plugin.hBackBrush;
		}
		break;

	case WM_DESTROY:
		WindowList_Add(g_hNewstoryLogs, hwnd);
		delete data;
		SetWindowLongPtr(hwnd, 0, 0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitNewstoryControl()
{
	htuLog = MTextRegister("Newstory", MTEXT_FANCY_DEFAULT | MTEXT_SYSTEM_HICONS | MTEXT_FANCY_SMILEYS);

	WNDCLASS wndclass = {};
	wndclass.style = /*CS_HREDRAW | CS_VREDRAW | */CS_DBLCLKS | CS_GLOBALCLASS;
	wndclass.lpfnWndProc = NewstoryListWndProc;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = _T(NEWSTORYLIST_CLASS);
	RegisterClass(&wndclass);
}
