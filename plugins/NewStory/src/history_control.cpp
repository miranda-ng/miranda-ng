/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'F') | HKF_MIRANDA_LOCAL;
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
	webPage(*this),
	redrawTimer(Miranda_GetSystemWindow(), LPARAM(this))
{
	items.setOwner(_1);

	bSortAscending = g_plugin.bSortAscending;

	redrawTimer.OnEvent = Callback(this, &NewstoryListData::onTimer_Draw);

	iLineHeigth = GetFontHeight(g_fontTable[FONT_INMSG].lf);
}

NewstoryListData::~NewstoryListData()
{
	for (auto &it : m_protoIcons)
		IcoLib_ReleaseIcon(it.second);
}

void NewstoryListData::onTimer_Draw(CTimer *pTimer)
{
	pTimer->Stop();

	if (bWasAtBottom)
		EnsureVisible(totalCount - 1);

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::OnContextMenu(int index, POINT pt)
{
	HMENU hMenu = NSMenu_Build(this, (index == -1) ? 0 : LoadItem(index));

	if (pMsgDlg != nullptr && pMsgDlg->isChat())
		Chat_CreateMenu(hMenu, pMsgDlg->getChat(), nullptr);

	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
	Menu_DestroyNestedMenu(hMenu);
}

void NewstoryListData::OnResize()
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	int newWidth = rc.right - rc.left, newHeight = rc.bottom - rc.top;

	if (dib.width() < newWidth || dib.height() < newHeight) {
		dib.destroy();
		dib.create(newWidth, newHeight, true);
	}

	bool bDraw = false;
	if (newWidth != cachedWindowWidth) {
		cachedWindowWidth = newWidth;
		for (int i = 0; i < totalCount; i++)
			GetItem(i)->savedHeight = -1;
		bDraw = true;
	}

	if (newHeight != cachedWindowHeight) {
		cachedWindowHeight = newHeight;
		FixScrollPosition();
		bDraw = true;
	}

	if (bDraw)
		ScheduleDraw();
}

void NewstoryListData::AddChatEvent(SESSION_INFO *si, const LOGINFO *lin)
{
	if (si == nullptr)
		return;

	ScheduleDraw();
	items.addChatEvent(this, si, lin);
	totalCount++;
}

/////////////////////////////////////////////////////////////////////////////////////////

void NewstoryListData::AddEvent(MCONTACT hContact, MEVENT hFirstEvent, int iCount, bool bNew)
{
	ScheduleDraw();
	items.addEvent(this, hContact, hFirstEvent, iCount, bNew);
	totalCount = items.getCount();

	if (iCount == -1)
		if (auto *szProto = Proto_GetBaseAccountName(hContact))
			if (auto hEvent = db_get_dw(hContact, szProto, "RemoteRead"))
				RemoteRead(hContact, hEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////

void NewstoryListData::AddResults(const OBJLIST<SearchResult> &results)
{
	ScheduleDraw();
	items.addResults(this, results);
	totalCount = items.getCount();
}

void NewstoryListData::AddSelection(int iFirst, int iLast)
{
	int start = std::min(totalCount - 1, iFirst);
	int end = std::min(totalCount - 1, std::max(0, iLast));
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

	return (scrollTopItem == 0 && scrollTopPixel == 0);
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

	RECT rc; GetClientRect(m_hwnd, &rc);
	int height = rc.bottom - rc.top, width = rc.right - rc.left;

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
	if (itemHeight > height)
		dwStyle |= WS_VSCROLL;

	hwndEditBox = CreateWindow(L"EDIT", wszText, dwStyle, 0, top, width, std::min(height, itemHeight), m_hwnd, NULL, g_plugin.getInst(), NULL);
	mir_subclassWindow(hwndEditBox, HistoryEditWndProc);
	SendMessage(hwndEditBox, WM_SETFONT, (WPARAM)g_fontTable[fontid].hfnt, 0);
	SendMessage(hwndEditBox, EM_SETMARGINS, EC_RIGHTMARGIN, 100);
	if (item->dbe.eventType != EVENTTYPE_MESSAGE || !item->dbe.bSent)
		SendMessage(hwndEditBox, EM_SETREADONLY, TRUE, 0);

	ShowWindow(hwndEditBox, SW_SHOW);
	SetFocus(hwndEditBox);
	SetForegroundWindow(hwndEditBox);
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
	int start = std::min(0, iFirst);
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
	Utils_ClipboardCopy(MClipUnicode(GatherSelected(bTextOnly)) + MClipRtf(GatherSelectedRtf()));
}

void NewstoryListData::CopyPath()
{
	if (auto *pItem = GetItem(caret))
		if (pItem->completed()) {
			DB::EventInfo dbei(pItem->dbe.getEvent());
			DB::FILE_BLOB blob(dbei);
			Utils_ClipboardCopy(MClipUnicode(blob.getLocalName()));
		}
}

void NewstoryListData::CopyUrl()
{
	if (auto *pItem = GetItem(caret)) {
		if (pItem->m_bOfflineFile)
			Srmm_DownloadOfflineFile(pItem->dbe.hContact, pItem->dbe.getEvent(), OFD_COPYURL);
		else if (auto *pszUrl = pItem->getUrl())
			Utils_ClipboardCopy(MClipUnicode(Utf2T(pszUrl)));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Delete events dialog

class CDeleteEventsDlg : public CDlgBase
{
	int m_iNumEvents;
	MCONTACT m_hContact;
	CCtrlCheck chkDelHistory, chkForEveryone;

public:
	bool bDelHistory = false, bForEveryone = false;

	CDeleteEventsDlg(MCONTACT hContact, int nEvents, bool bIncoming) :
		CDlgBase(g_plugin, IDD_EMPTYHISTORY),
		m_hContact(hContact),
		m_iNumEvents(nEvents),
		chkDelHistory(this, IDC_DELSERVERHISTORY),
		chkForEveryone(this, IDC_BOTH)
	{
		if (char *szProto = Proto_GetBaseAccountName(hContact)) {
			bDelHistory = Proto_CanDeleteHistory(szProto, hContact, bIncoming);
			bForEveryone = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, hContact) & PF4_DELETEFORALL) != 0;
		}
	}

	bool OnInitDialog() override
	{
		chkDelHistory.SetState(bDelHistory);
		chkDelHistory.Enable(bDelHistory);

		if (Contact::IsGroupChat(m_hContact)) {
			chkForEveryone.SetState(true);
			chkForEveryone.Disable();
		}
		else {
			chkForEveryone.SetState(false);
			chkForEveryone.Enable(bDelHistory && bForEveryone);
		}

		if (m_iNumEvents > 1) {
			CMStringW wszText(FORMAT, TranslateT("Do you really want to delete selected items (%d)?"), m_iNumEvents);
			SetDlgItemTextW(m_hwnd, IDC_TOPLINE, wszText);
		}

		LOGFONT lf;
		HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		SendDlgItemMessage(m_hwnd, IDC_TOPLINE, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		SetFocus(GetDlgItem(m_hwnd, IDNO));
		SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return true;
	}

	bool OnApply() override
	{
		bDelHistory = chkDelHistory.IsChecked();
		bForEveryone = chkForEveryone.IsChecked();
		return true;
	}

	void OnDestroy() override
	{
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TOPLINE, WM_GETFONT, 0, 0));
	}
};

void NewstoryListData::DeleteItems(void)
{
	int nSelected = 0;
	bool bIncoming = false;

	for (int i = totalCount - 1; i >= 0; i--) {
		auto *pItem = GetItem(i);
		if (pItem->m_bSelected) {
			if (!pItem->dbe.bSent)
				bIncoming = true;
			nSelected++;
		}
	}

	CDeleteEventsDlg dlg(m_hContact, nSelected, bIncoming);
	if (IDOK == dlg.DoModal()) {
		int iSaveCount = totalCount;
		g_plugin.bDisableDelete = true;

		int firstSel = -1, flags = 0;
		if (dlg.bDelHistory)
			flags |= CDF_DEL_HISTORY;
		if (dlg.bForEveryone)
			flags |= CDF_FOR_EVERYONE;

		for (int i = totalCount - 1; i >= 0; i--) {
			auto *p = GetItem(i);
			if (!p->m_bSelected)
				continue;

			if (p->dbe.getEvent())
				db_event_delete(p->dbe.getEvent(), flags);
			items.remove(i);
			totalCount--;
			firstSel = i;
		}

		g_plugin.bDisableDelete = false;
		if (iSaveCount != totalCount)
			PostMessage(GetParent(m_hwnd), UM_UPDATE_WINDOW, 0, 0);

		if (firstSel != -1) {
			SetCaret(firstSel, false);
			SetSelection(firstSel, firstSel);
			FixScrollPosition(true);
		}
	}

	::SetFocus(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////

void NewstoryListData::DeliverEvent(MCONTACT hContact, MEVENT hEvent)
{
	bool isChanged = false, isActive = false;

	for (int i = totalCount - 1; i >= 0; i--) {
		auto *pItem = GetItem(i);
		if (pItem->dbe.hContact != hContact || !pItem->dbe.bSent)
			continue;

		if (pItem->dbe.getEvent() == hEvent)
			isActive = true;

		if (isActive && pItem->m_bNew && !pItem->m_bDelivered) {
			pItem->m_bDelivered = true;
			pItem->savedHeight = -1;
			pItem->calcHeight(cachedWindowWidth);
			isChanged = true;
		}
	}

	if (isChanged)
		InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::Download(int options)
{
	if (auto *p = LoadItem(caret))
		Srmm_DownloadOfflineFile(p->dbe.hContact, p->dbe.getEvent(), options);
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

			if (pItem->dbe.hContact && pItem->dbe.getEvent()) {
				DB::EventInfo dbei(pItem->dbe.getEvent());

				ptrA szUtf(mir_utf8encodeW(pItem->wtext));
				dbei.cbBlob = (int)mir_strlen(szUtf) + 1;
				dbei.pBlob = szUtf.detach();
				db_event_edit(pItem->dbe.getEvent(), &dbei);
			}

			pItem->m_doc = 0;
			pItem->savedHeight = -1;
			pItem->setText();
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
		if (wszText.IsEmpty())
			continue;

		RemoveBbcodes(wszText);
		ret.Append(wszText);
		ret.Append(L"\r\n\r\n");
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
	if (auto *pItem = LoadItem(index))
		return GetItemHeight(pItem);
	return 0;
}

int NewstoryListData::GetItemHeight(ItemData *pItem)
{
	if (pItem->savedHeight == -1)
		pItem->calcHeight(cachedWindowWidth);

	return pItem->savedHeight;
}

bool NewstoryListData::HasSelection() const
{
	for (int i = 0; i < totalCount; i++)
		if (auto *p = GetItem(i))
			if (p->m_bSelected)
				return true;

	return false;
}

void NewstoryListData::HitTotal(int yCurr, int yTotal)
{
	int i, y = yCurr;
	for (i = 0; i < totalCount; i++) {
		auto *pItem = GetItem(i);
		if (!pItem->m_bLoaded) {
			i = totalCount * (double(yCurr) / double(yTotal));
			y = 0;
			break;
		}
		
		int h = GetItemHeight(pItem);
		if (h > y) {
			y = -y;
			break;
		}

		y -= h;
	}

	scrollTopItem = i;
	scrollTopPixel = y;
	FixScrollPosition();
}

ItemData* NewstoryListData::LoadItem(int idx)
{
	if (totalCount == 0)
		return nullptr;

	mir_cslock lck(m_csItems);
	return (bSortAscending) ? items.get(idx, true) : items.get(totalCount - 1 - idx, true);
}

void NewstoryListData::MarkRead(ItemData *pItem)
{
	if (pMsgDlg)
		pMsgDlg->MarkEventRead(pItem->dbe);
}

void NewstoryListData::OpenFolder()
{
	if (auto *pItem = GetItem(caret)) {
		if (pItem->completed()) {
			DB::EventInfo dbei(pItem->dbe.getEvent());
			DB::FILE_BLOB blob(dbei);
			CMStringW wszFile(blob.getLocalName());
			int idx = wszFile.ReverseFind('\\');
			if (idx != -1)
				wszFile.Truncate(idx);
			::ShellExecute(nullptr, L"open", wszFile, nullptr, nullptr, SW_SHOWNORMAL);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Painting

static void recursive_set_color(element::ptr el, const web_color &fore, const background &back)
{
	el->css_w().set_bg(back);
	el->css_w().set_color(fore);

	for (auto &it : el->children()) {
		if (it->tag() == _a_ || (it->tag() == _font_ && mir_strcmp(it->get_attr("class"), "body")))
			continue;
		recursive_set_color(it, fore, back);
	}
}

void NewstoryListData::Paint(simpledib::dib &dib)
{
	int top = 0;

	for (int idx = scrollTopItem; top < cachedWindowHeight && idx < totalCount; idx++) {
		if (hwndEditBox && caret == idx)
			continue;

		auto *pItem = LoadItem(idx);
		pItem->calcHeight(cachedWindowWidth); // ensure that the item's height is calculated

		COLORREF clLine, clText, clBack;
		int fontid, colorid;
		pItem->getFontColor(fontid, colorid);

		if (pItem->m_bHighlighted) {
			clText = g_fontTable[FONT_HIGHLIGHT].cl;
			clBack = g_colorTable[pItem->m_bSelected ? COLOR_SELBACK : COLOR_HIGHLIGHT_BACK].cl;
			clLine = g_colorTable[COLOR_FRAME].cl;
		}
		else if (pItem->m_bSelected && !bReadOnly) {
			clText = g_colorTable[COLOR_SELTEXT].cl;
			clBack = g_colorTable[COLOR_SELBACK].cl;
			clLine = g_colorTable[COLOR_SELFRAME].cl;
		}
		else {
			clText = g_fontTable[fontid].cl;
			clBack = g_colorTable[colorid].cl;
			clLine = g_colorTable[COLOR_FRAME].cl;
		}

		int iItemHeigth, iOffsetY;
		if (top == 0) {
			pItem->savedTop = iOffsetY = scrollTopPixel;
			iItemHeigth = pItem->savedHeight + scrollTopPixel;
		}
		else {
			pItem->savedTop = top;
			iOffsetY = 0;
			iItemHeigth = pItem->savedHeight;
		}

		// draw item background
		HBRUSH hbr = CreateSolidBrush(clBack);
		RECT rc = { 0, top, cachedWindowWidth, top + iItemHeigth };
		FillRect(dib, &rc, hbr);
		DeleteObject(hbr);

		SetBkMode(dib, TRANSPARENT);

		// left offset of icons & text
		bool bDrawProgress = false;
		int xPos = 2, yPos = top + 2, xRight = 0;

		if (!bReadOnly) {
			HICON hIcon;

			// Protocol icon
			if (m_hContact == INVALID_CONTACT_ID) {
				if (auto *pa = Proto_GetContactAccount(pItem->dbe.hContact)) {
					if (m_protoIcons.count(pa->szModuleName))
						hIcon = m_protoIcons[pa->szModuleName];
					else {
						hIcon = Skin_LoadProtoIcon(pa->szModuleName, ID_STATUS_ONLINE);
						m_protoIcons[pa->szModuleName] = hIcon;
					}

					DrawIconEx(dib, xPos, yPos, hIcon, 16, 16, 0, 0, DI_NORMAL);
					xPos += 18;
				}
			}

			// Message type icon
			if (g_plugin.bShowType) {
				switch (pItem->dbe.eventType) {
				case EVENTTYPE_MESSAGE:
					hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
					break;
				case EVENTTYPE_FILE:
					hIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
					break;
				case EVENTTYPE_STATUSCHANGE:
					hIcon = g_plugin.getIcon(IDI_SIGNIN);
					break;
				default:
					hIcon = g_plugin.getIcon(IDI_UNKNOWN);
					break;
				}
				DrawIconEx(dib, xPos, yPos, hIcon, 16, 16, 0, 0, DI_NORMAL);
				xPos += 18;
				IcoLib_ReleaseIcon(hIcon);
			}

			// Direction icon
			if (g_plugin.bShowDirection) {
				if (pItem->dbe.bSent)
					hIcon = g_plugin.getIcon(IDI_MSGOUT);
				else
					hIcon = g_plugin.getIcon(IDI_MSGIN);
				DrawIconEx(dib, xPos, yPos, hIcon, 16, 16, 0, 0, DI_NORMAL);
				xPos += 18;
				IcoLib_ReleaseIcon(hIcon);
			}

			// Bookmark icon
			if (pItem->dbe.isBookmark) {
				DrawIconEx(dib, xPos, yPos, g_plugin.getIcon(IDI_BOOKMARK), 16, 16, 0, 0, DI_NORMAL);
				xPos += 18;
			}

			// Finished icon
			if (pItem->m_bOfflineDownloaded != 0) {
				if (pItem->completed())
					DrawIconEx(dib, cachedWindowWidth - (xRight = 18), yPos, g_plugin.getIcon(IDI_OK), 16, 16, 0, 0, DI_NORMAL);
				else
					bDrawProgress = true;
			}

			// Delivered & remote read icons
			if (pItem->m_bRemoteRead)
				DrawIconEx(dib, cachedWindowWidth - (xRight = 18), yPos, g_plugin.getIcon(IDI_REMOTEREAD), 16, 16, 0, 0, DI_NORMAL);
			else if (pItem->m_bDelivered)
				DrawIconEx(dib, cachedWindowWidth - (xRight = 18), yPos, g_plugin.getIcon(IDI_DELIVERED), 16, 16, 0, 0, DI_NORMAL);

			// Edited icon
			if (pItem->dbe.bEdited) {
				xRight += 18;
				DrawIconEx(dib, cachedWindowWidth - xRight, yPos, g_plugin.getIcon(IDI_SENDMSG), 16, 16, 0, 0, DI_NORMAL);
			}
		}

		// draw html itself
		litehtml::position clip(xPos, yPos, cachedWindowWidth - xPos - xRight, iItemHeigth);
		if (auto &pDoc = pItem->m_doc) {
			if (auto pBody = pDoc->root()->select_one("body")) {
				if (auto pBbody = pBody->select_one("[id=bbody]")) {
					litehtml::background back = pBbody->css().get_bg();
					back.m_color = litehtml::web_color(GetRValue(clBack), GetGValue(clBack), GetBValue(clBack));

					litehtml::web_color fore(GetRValue(clText), GetGValue(clText), GetBValue(clText));
					recursive_set_color(pBbody, fore, back);
				}
			}

			pDoc->draw((UINT_PTR)dib.hdc(), xPos, yPos + iOffsetY, &clip);
		}

		// draw progress
		if (bDrawProgress) {
			HPEN hpn = (HPEN)SelectObject(dib, CreatePen(PS_SOLID, 4, g_colorTable[COLOR_PROGRESS].cl));
			MoveToEx(dib, rc.left, rc.bottom - 4, 0);
			LineTo(dib, rc.left + (rc.right - rc.left) * int(pItem->m_bOfflineDownloaded) / 100, rc.bottom - 4);
			DeleteObject(SelectObject(dib, hpn));
		}

		// draw border
		HPEN hpn = (HPEN)SelectObject(dib, CreatePen(PS_SOLID, 1, clLine));
		MoveToEx(dib, rc.left, rc.bottom - 1, 0);
		LineTo(dib, rc.right, rc.bottom - 1);
		DeleteObject(SelectObject(dib, hpn));

		top += iItemHeigth;
		cachedMaxDrawnItem = idx;
	}

	if (top <= cachedWindowHeight) {
		RECT rc2;
		SetRect(&rc2, 0, top, cachedWindowWidth, cachedWindowHeight);

		HBRUSH hbr = CreateSolidBrush(g_colorTable[COLOR_BACK].cl);
		FillRect(dib, &rc2, hbr);
		DeleteObject(hbr);
	}

	if (g_plugin.bOptVScroll)
		RecalcScrollBar();
	
	if (g_plugin.bDrawEdge) {
		RECT rc = { 0, 0, cachedWindowWidth, cachedWindowHeight };
		DrawEdge(dib, &rc, BDR_SUNKENOUTER, BF_RECT);
	}
}

void NewstoryListData::Quote()
{
	if (pMsgDlg) {
		CMStringW wszText(GatherSelected(true));
		wszText.TrimRight();
		RemoveBbcodes(wszText);
		pMsgDlg->SetMessageText(Srmm_Quote(wszText));

		SetFocus(pMsgDlg->GetInput());
	}
}

void NewstoryListData::RecalcScrollBar()
{
	if (totalCount == 0)
		return;

	int yTotal = 0, yTop = 0, yVis = 0, numRec = 0;
	for (int i = 0; i < totalCount; i++) {
		if (i == scrollTopItem)
			yTop = yTotal - scrollTopPixel;

		auto *pItem = GetItem(i);
		if (pItem->m_bLoaded) {
			numRec++;

			int iHeight = GetItemHeight(pItem);
			yTotal += iHeight;
			if (i >= scrollTopItem)
				yVis += iHeight;
		}
	}

	if (numRec != totalCount) {
		double averageH = double(yTotal) / double(numRec);
		yTotal = totalCount * averageH;
		if (cachedMaxDrawnItem == totalCount - 1)
			yTop = yTotal - ((scrollTopItem == 0) ? yVis : cachedWindowHeight);
		else
			yTop = scrollTopItem * averageH;
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

void NewstoryListData::RemoteRead(MCONTACT hContact, MEVENT hEvent)
{
	bool isChanged = false, isActive = false;

	for (int i = totalCount-1; i >= 0; i--) {
		auto *pItem = GetItem(i);
		if (!pItem->m_bLoaded)
			pItem->fetch();

		if (pItem->dbe.hContact != hContact || !pItem->dbe.bSent)
			continue;

		if (pItem->dbe.getEvent() == hEvent)
			isActive = true;

		if (isActive && pItem->m_bNew && !pItem->m_bRemoteRead) {
			pItem->m_bRemoteRead = true;
			pItem->savedHeight = -1;
			pItem->calcHeight(cachedWindowWidth);
			isChanged = true;
		}
	}

	if (isChanged)
		InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::Reply()
{
	if (pMsgDlg)
		if (auto *pItem = GetItem(caret))
			pMsgDlg->SetQuoteEvent(pItem->dbe.getEvent());
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
		if (idx != caret) {
			caret = idx;
			PostMessage(GetParent(m_hwnd), UM_LOCATETIME, caret, 0);
		}

		if (bEnsureVisible)
			EnsureVisible(idx);
	}
}

void NewstoryListData::SetPos(int pos)
{
	SetSelection((selStart == -1) ? pos : selStart, pos);
	SetCaret(pos);
}

void NewstoryListData::SetSelection(int iFirst, int iLast)
{
	int start = std::min(totalCount - 1, iFirst);
	int end = std::min(totalCount - 1, std::max(0, iLast));
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
		db_event_edit(p->dbe.getEvent(), &p->dbe);

		p->setText();
	}

	InvalidateRect(m_hwnd, 0, FALSE);
}

void NewstoryListData::ToggleSelection(int iFirst, int iLast)
{
	int start = std::min(totalCount - 1, iFirst);
	int end = std::min(totalCount - 1, std::max(0, iLast));
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
	MEVENT hTopEvent = 0;
	MCONTACT hContact;
	
	if (totalCount != 0) {
		auto *pTop = GetItem(0);
		hContact = pTop->dbe.hContact;
		hTopEvent = pTop->dbe.getEvent();
		if (hTopEvent == 0)
			return;
	}
	else {
		hContact = (pMsgDlg) ? pMsgDlg->m_hContact : 0;
		hTopEvent = -1;
	}
	
	if (hContact == 0)
		return;

	bool hasRead = false;
	for (int i=0; i < totalCount; i++)
		if (GetItem(i)->m_bRemoteRead) {
			hasRead = true;
			break;
		}

	int i;
	for (i = 0; i < iCount; i++) {
		MEVENT hPrev = (hTopEvent == -1) ? db_event_last(hContact) : db_event_prev(hContact, hTopEvent);
		if (hPrev == 0)
			break;

		auto *p = items.insert(0);
		p->pOwner = this;
		p->dbe.hContact = hContact;
		p->dbe = hPrev;
		p->m_bNew = true;
		totalCount++;
	}

	ItemData *pPrev = nullptr;
	for (int j = 0; j < i + 1; j++)
		if (auto *pItem = GetItem(j)) {
			pItem->fetch();
			if (pItem->dbe.bSent)
				pItem->m_bRemoteRead = hasRead;
			pPrev = pItem->checkNext(pPrev);
		}

	caret = 0;
	CalcBottom();
	FixScrollPosition();
	InvalidateRect(m_hwnd, 0, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Navigation by coordinates

void NewstoryListData::LineUp(int iCount)
{
	if (AtTop())
		TryUp(1);
	else
		ScrollUp(iLineHeigth * iCount);
}

void NewstoryListData::LineDown(int iCount)
{
	if (!AtBottom())
		ScrollDown(iLineHeigth * iCount);
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
				if (p->dbe.getUnixtime() >= wParam) {
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

	case NSM_SET_OPTIONS:
		data->bSortAscending = g_plugin.bSortAscending;
		data->scrollTopPixel = 0;
		data->FixScrollPosition(true);
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case UM_ADD_EVENT:
		if (data->pMsgDlg == nullptr)
			data->AddEvent(wParam, lParam, 1, true);
		break;

	case UM_DELIVER_EVENT:
		data->DeliverEvent(wParam, lParam);
		break;

	case UM_REMOTE_READ:
		data->RemoteRead(wParam, lParam);
		break;

	case UM_EDIT_EVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			auto *p = data->GetItem(idx);
			p->load(true);
			p->setText();
			InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case UM_REMOVE_EVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			data->items.remove(idx);
			data->totalCount--;
			data->FixScrollPosition(true);
			InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case WM_SIZE:
		data->OnResize();
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
			HDC hdc = BeginPaint(hwnd, &ps);

			data->Paint(data->dib);

			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top, data->dib, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

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
	case WM_KEYUP:
		{
			bool isShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
			bool isCtrl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

			if (!data->bWasShift && isShift)
				data->selStart = data->caret;
			else if (data->bWasShift && !isShift)
				data->selStart = -1;

			data->bWasShift = isShift;

			if (msg == WM_KEYUP)
				break;

			int oldCaret = data->caret;
			switch (wParam) {
			case VK_UP:
				if (g_plugin.bHppCompat) {
					data->EventUp();
					if (isShift)
						data->AddSelection(data->caret, oldCaret);
				}
				else data->LineUp();
				break;

			case VK_DOWN:
				if (g_plugin.bHppCompat) {
					data->EventDown();
					if (isShift)
						data->AddSelection(oldCaret, data->caret);
				}
				else data->LineDown();
				break;

			case VK_PRIOR:
				if (isCtrl)
					data->ScrollTop();
				else {
					if (g_plugin.bHppCompat) {
						data->EventPageUp();
						if (isShift)
							data->AddSelection(data->caret, oldCaret);
					}
					else data->PageUp();
				}
				break;

			case VK_NEXT:
				if (isCtrl)
					data->ScrollBottom();
				else {
					if (g_plugin.bHppCompat) {
						data->EventPageDown();
						if (isShift)
							data->AddSelection(oldCaret, data->caret);
					}
					else data->PageDown();
				}
				break;

			case VK_HOME:
				data->ScrollTop();
				if (isShift)
					data->AddSelection(0, data->caret);
				break;

			case VK_END:
				data->ScrollBottom();
				if (isShift)
					data->AddSelection(data->caret, data->totalCount);
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

			litehtml::position::vector redraw_boxes;
			pItem->m_doc->on_lbutton_down(pt.x - pItem->leftOffset, pt.y - pItem->savedTop, pt.x, pt.y, redraw_boxes);

			if (wParam & MK_CONTROL) {
				data->ToggleSelection(idx, idx);
				data->SetCaret(idx, false);
			}
			else if (wParam & MK_SHIFT) {
				data->AddSelection(data->caret, idx);
				data->SetCaret(idx, false);
			}
			else {
				data->selStart = idx;
				data->SetSelection(idx, idx);
				data->SetCaret(idx, false);
			}
		}
		SetFocus(hwnd);
		return 0;

	case WM_LBUTTONUP:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			data->selStart = -1;

			auto *pItem = data->LoadItem(idx);
			litehtml::position::vector redraw_boxes;
			pItem->m_doc->on_lbutton_up(pt.x - pItem->leftOffset, pt.y - pItem->savedTop, pt.x, pt.y, redraw_boxes);
		}
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
				Srmm_DownloadOfflineFile(pItem->dbe.hContact, pItem->dbe.getEvent(), OFD_DOWNLOAD | OFD_RUN);
				return 0;
			}

			if (data->caret == idx) {
				data->BeginEditItem();
				return 0;
			}
		}

		SetFocus(hwnd);
		return 0;

	case WM_MOUSELEAVE:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			auto *pItem = data->LoadItem(idx);
			litehtml::position::vector redraw_boxes;
			pItem->m_doc->on_mouse_leave(redraw_boxes);
		}
		break;

	case WM_MOUSEMOVE:
		pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		idx = data->GetItemFromPixel(pt.y);
		if (idx >= 0) {
			if (data->selStart != -1) {
				data->SetSelection(data->selStart, idx);
				InvalidateRect(hwnd, 0, FALSE);
			}

			auto *pItem = data->LoadItem(idx);
			litehtml::position::vector redraw_boxes;
			pItem->m_doc->on_mouse_over(pt.x - pItem->leftOffset, pt.y - pItem->savedTop, pt.x, pt.y, redraw_boxes);
		}
		break;

	case WM_MOUSEWHEEL:
		if ((short)HIWORD(wParam) < 0)
			data->LineDown(3);
		else
			data->LineUp(3);
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
				data->HitTotal(si.nTrackPos, si.nMax);
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
		delete data;
		SetWindowLongPtr(hwnd, 0, 0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitNewstoryControl()
{
	WNDCLASS wndclass = {};
	wndclass.style = /*CS_HREDRAW | CS_VREDRAW | */CS_DBLCLKS | CS_GLOBALCLASS;
	wndclass.lpfnWndProc = NewstoryListWndProc;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = _T(NEWSTORYLIST_CLASS);
	RegisterClass(&wndclass);
}
