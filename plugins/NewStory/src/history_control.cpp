#include "stdafx.h"

#define AVERAGE_ITEM_HEIGHT 100

HANDLE htuLog = 0;

static WNDPROC OldEditWndProc;
static LRESULT CALLBACK HistoryEditWndProc(HWND, UINT, WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////
// Control utilities, types and constants

NewstoryListData::NewstoryListData(HWND _1) :
	hwnd(_1),
	redrawTimer(Miranda_GetSystemWindow(), (LPARAM)this)
{
	bSortAscending = g_plugin.bSortAscending;

	redrawTimer.OnEvent = Callback(this, &NewstoryListData::OnTimer);
}

void NewstoryListData::OnContextMenu(int index, POINT pt)
{
	ItemData *item = LoadItem(index);
	if (item == nullptr)
		return;
		
	HMENU hMenu = NSMenu_Build(item);

	if (pMsgDlg != nullptr && pMsgDlg->isChat()) {
		EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED);
		Chat_CreateMenu(hMenu, pMsgDlg->getChat(), nullptr);
	}
		
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
	Menu_DestroyNestedMenu(hMenu);
}

void NewstoryListData::OnResize(int newWidth)
{
	if (newWidth == cachedWindowWidth)
		return;

	for (int i = 0; i < totalCount; i++)
		LoadItem(i)->savedHeight = -1;
}

void NewstoryListData::OnTimer(CTimer *pTimer)
{
	pTimer->Stop();

	scrollTopItem = totalCount;
	FixScrollPosition();
	InvalidateRect(hwnd, 0, FALSE);
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

	InvalidateRect(hwnd, 0, FALSE);
}

void NewstoryListData::BeginEditItem(int index, bool bReadOnly)
{
	if (hwndEditBox)
		EndEditItem(false);

	if (scrollTopItem > index)
		return;

	ItemData *item = LoadItem(index);
	if (item->dbe.eventType != EVENTTYPE_MESSAGE)
		return;

	RECT rc; GetClientRect(hwnd, &rc);
	int height = rc.bottom - rc.top;

	int top = scrollTopPixel;
	int idx = scrollTopItem;
	int itemHeight = GetItemHeight(idx);
	while (top < height) {
		if (idx == index)
			break;

		top += itemHeight;
		idx++;
		itemHeight = GetItemHeight(idx);
	}

	int fontid, colorid;
	item->getFontColor(fontid, colorid);

	uint32_t dwStyle = WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL;
	if (bReadOnly) 
		dwStyle |= ES_READONLY;

	hwndEditBox = CreateWindow(L"EDIT", item->getWBuf(), dwStyle, 0, top, rc.right - rc.left, itemHeight, hwnd, NULL, g_plugin.getInst(), NULL);
	SetWindowLongPtrW(hwndEditBox, GWLP_USERDATA, (LPARAM)item);
	OldEditWndProc = (WNDPROC)SetWindowLongPtr(hwndEditBox, GWLP_WNDPROC, (LONG_PTR)HistoryEditWndProc);
	SendMessage(hwndEditBox, WM_SETFONT, (WPARAM)g_fontTable[fontid].hfnt, 0);
	SendMessage(hwndEditBox, EM_SETMARGINS, EC_RIGHTMARGIN, 100);
	SendMessage(hwndEditBox, EM_SETSEL, 0, (LPARAM)(-1));
	ShowWindow(hwndEditBox, SW_SHOW);
	SetFocus(hwndEditBox);
}

void NewstoryListData::ClearSelection(int iFirst, int iLast)
{
	int start = min(totalCount - 1, iFirst);
	int end = min(totalCount - 1, max(0, iLast));
	if (start > end)
		std::swap(start, end);

	for (int i = start; i <= end; ++i) {
		auto *p = GetItem(i);
		p->m_bSelected = false;
	}

	InvalidateRect(hwnd, 0, FALSE);
}

void NewstoryListData::DeleteItems(void)
{
	if (IDYES != MessageBoxW(hwnd, TranslateT("Are you sure to remove selected event(s)?"), _T(MODULETITLE), MB_YESNOCANCEL | MB_ICONQUESTION))
		return;

	db_set_safety_mode(false);

	int firstSel = -1;
	for (int i = totalCount - 1; i >= 0; i--) {
		auto *p = GetItem(i);
		if (p->hEvent && p->m_bSelected) {
			db_event_delete(p->hEvent);
			items.remove(i);
			totalCount--;
			firstSel = i;
		}
	}
	db_set_safety_mode(true);

	if (firstSel != -1) {
		SetCaret(firstSel, false);
		SetSelection(firstSel, firstSel);
	}
}

void NewstoryListData::EndEditItem(bool bAccept)
{
	if (hwndEditBox == nullptr)
		return;

	if (bAccept) {
		if ((GetWindowLong(hwndEditBox, GWL_STYLE) & ES_READONLY) == 0) {
			auto *pItem = (ItemData *)GetWindowLongPtrW(hwndEditBox, GWLP_USERDATA);

			int iTextLen = GetWindowTextLengthW(hwndEditBox);
			replaceStrW(pItem->wtext, (wchar_t *)mir_alloc((iTextLen + 1) * sizeof(wchar_t)));
			GetWindowTextW(hwndEditBox, pItem->wtext, iTextLen);
			pItem->wtext[iTextLen] = 0;

			if (pItem->hContact && pItem->hEvent) {
				ptrA szUtf(mir_utf8encodeW(pItem->wtext));
				pItem->dbe.cbBlob = (int)mir_strlen(szUtf) + 1;
				pItem->dbe.pBlob = (uint8_t *)szUtf.get();
				db_event_edit(pItem->hEvent, &pItem->dbe);

				if (auto *ppro = Proto_GetInstance(pItem->hContact))
					ppro->OnEventEdited(pItem->hContact, pItem->hEvent);
			}

			MTextDestroy(pItem->data); pItem->data = 0;
			pItem->savedHeight = -1;
			pItem->checkCreate(hwnd);
		}
	}

	DestroyWindow(hwndEditBox);
	hwndEditBox = 0;
}

void NewstoryListData::EnsureVisible(int item)
{
	if (scrollTopItem >= item) {
		scrollTopItem = item;
		scrollTopPixel = 0;
	}
	else {
		RECT rc;
		GetClientRect(hwnd, &rc);
		int height = rc.bottom - rc.top;
		int top = scrollTopPixel;
		int idx = scrollTopItem;
		int itemHeight = GetItemHeight(idx);
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
}

void NewstoryListData::FixScrollPosition()
{
	EndEditItem(false);

	RECT rc;
	GetWindowRect(hwnd, &rc);
	int windowHeight = rc.bottom - rc.top;

	if (windowHeight != cachedWindowHeight || cachedMaxTopItem != scrollTopItem) {
		int maxTopItem = 0;
		int tmp = 0;
		for (maxTopItem = totalCount; (maxTopItem > 0) && (tmp < windowHeight); maxTopItem--)
			tmp += GetItemHeight(maxTopItem - 1);
		cachedMaxTopItem = maxTopItem;
		cachedWindowHeight = windowHeight;
		cachedMaxTopPixel = (windowHeight < tmp) ? windowHeight - tmp : 0;
	}

	if (scrollTopItem < 0)
		scrollTopItem = 0;

	if ((scrollTopItem > cachedMaxTopItem) ||
		((scrollTopItem == cachedMaxTopItem) && (scrollTopPixel < cachedMaxTopPixel))) {
		scrollTopItem = cachedMaxTopItem;
		scrollTopPixel = cachedMaxTopPixel;
	}

	if (g_plugin.bOptVScroll)
		RecalcScrollBar();
}

ItemData* NewstoryListData::GetItem(int idx)
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
	GetClientRect(hwnd, &rc);

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
	item->checkCreate(hwnd);

	HDC hdc = GetDC(hwnd);
	HFONT hOldFont = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);

	RECT rc; GetClientRect(hwnd, &rc);
	int width = rc.right - rc.left;

	SIZE sz;
	sz.cx = width - 6;
	MTextMeasure(hdc, &sz, (HANDLE)item->data);

	SelectObject(hdc, hOldFont);
	ReleaseDC(hwnd, hdc);
	return item->savedHeight = sz.cy + 5;
}

ItemData *NewstoryListData::LoadItem(int idx)
{
	if (totalCount == 0)
		return nullptr;

	return (bSortAscending) ? items.get(idx, true) : items.get(totalCount - 1 - idx, true);
}

int NewstoryListData::PaintItem(HDC hdc, int index, int top, int width)
{
	auto *item = LoadItem(index);
	item->savedTop = top;

	//	LOGFONT lfText;
	COLORREF clText, clBack, clLine;
	int fontid, colorid;
	item->getFontColor(fontid, colorid);

	clText = g_fontTable[fontid].cl;
	if (item->m_bSelected) {
		MTextSendMessage(0, item->data, EM_SETSEL, 0, -1);
		clText = g_colorTable[COLOR_SELTEXT].cl;
		clBack = g_colorTable[COLOR_SELBACK].cl;
		clLine = g_colorTable[COLOR_SELFRAME].cl;
	}
	else {
		MTextSendMessage(0, item->data, EM_SETSEL, 0, 0);
		clLine = g_colorTable[COLOR_FRAME].cl;
		clBack = g_colorTable[colorid].cl;
	}

	item->checkCreate(hwnd);

	SIZE sz;
	sz.cx = width - 6;
	HFONT hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
	MTextMeasure(hdc, &sz, item->data);
	SelectObject(hdc, hfnt);
	int height = sz.cy + 5;

	RECT rc;
	SetRect(&rc, 0, top, width, top + height);

	HBRUSH hbr;
	hbr = CreateSolidBrush(clBack);
	FillRect(hdc, &rc, hbr);

	SetTextColor(hdc, clText);
	SetBkMode(hdc, TRANSPARENT);

	POINT pos;
	pos.x = 3;
	pos.y = top + 2;
	hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
	MTextDisplay(hdc, pos, sz, item->data);
	SelectObject(hdc, hfnt);

	DeleteObject(hbr);

	HPEN hpn = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 1, clLine));
	MoveToEx(hdc, rc.left, rc.bottom - 1, 0);
	LineTo(hdc, rc.right, rc.bottom - 1);
	DeleteObject(SelectObject(hdc, hpn));
	return height;
}

void NewstoryListData::RecalcScrollBar()
{
	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = totalCount-1;
	si.nPage = si.nMax / 10;
	si.nPos = caret;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void NewstoryListData::ScheduleDraw()
{
	redrawTimer.Stop();
	redrawTimer.Start(100);
}

void NewstoryListData::SetCaret(int idx, bool bEnsureVisible)
{
	if (idx < totalCount) {
		caret = idx;
		if (bEnsureVisible)
			EnsureVisible(idx);
	}
}

void NewstoryListData::SetPos(int pos)
{
	caret = pos;
	SetSelection((selStart == -1) ? pos : selStart, pos);
	SetCaret(pos, true);
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
		if ((i >= start) && (i <= end))
			p->m_bSelected = true;
		else
			p->m_bSelected = false;
	}

	InvalidateRect(hwnd, 0, FALSE);
}

void NewstoryListData::ToggleBookmark()
{
	if (auto *p = GetItem(caret)) {
		if (p->dbe.flags & DBEF_BOOKMARK)
			p->dbe.flags &= ~DBEF_BOOKMARK;
		else
			p->dbe.flags |= DBEF_BOOKMARK;
		db_event_edit(p->hEvent, &p->dbe);

		p->setText();
		InvalidateRect(hwnd, 0, FALSE);
	}
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

	InvalidateRect(hwnd, 0, FALSE);
}

/////////////////////////////////////////////////////////////////////////
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

	return CallWindowProc(OldEditWndProc, hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////
// NewStory history control window procedure

LRESULT CALLBACK NewstoryListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int idx;
	POINT pt;
	NewstoryListData *data = (NewstoryListData *)GetWindowLongPtr(hwnd, 0);

	switch (msg) {
	case WM_CREATE:
		data = new NewstoryListData(hwnd);
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)data);
		if (!g_plugin.bOptVScroll)
			SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_VSCROLL);
		else
			data->RecalcScrollBar();
		break;

		// History list control messages
	case NSM_ADDEVENTS:
		if (auto *p = (ADDEVENTS *)wParam) {
			data->items.addEvent(p->hContact, p->hFirstEVent, p->eventCount);
			data->totalCount = data->items.getCount();
		}
		data->ScheduleDraw();
		break;

	case NSM_ADDCHATEVENT:
		data->items.addChatEvent((SESSION_INFO *)wParam, (LOGINFO *)lParam);
		data->totalCount++;
		data->ScheduleDraw();
		break;

	case NSM_CLEAR:
		data->items.clear();
		data->totalCount = 0;
		data->ScheduleDraw();
		break;

	case NSM_GETARRAY:
		return (LRESULT)&data->items;

	case NSM_GETCOUNT:
		return data->totalCount;

	case NSM_SELECTITEMS:
		data->AddSelection(wParam, lParam);
		return 0;

	case NSM_GETCARET:
		return data->caret;

	case NSM_FINDNEXT:
		idx = data->items.FindNext(data->caret, Filter(Filter::EVENTONLY, (wchar_t *)wParam));
		if (idx == -1 && data->caret > 0)
			idx = data->items.FindNext(-1, Filter(Filter::EVENTONLY, (wchar_t *)wParam));

		if (idx >= 0) {
			data->SetSelection(idx, idx);
			data->SetCaret(idx, true);
		}
		return idx;

	case NSM_FINDPREV:
		idx = data->items.FindPrev(data->caret, Filter(Filter::EVENTONLY, (wchar_t *)wParam));
		if (idx == -1 && data->caret != data->totalCount - 1)
			idx = data->items.FindPrev(data->totalCount, Filter(Filter::EVENTONLY, (wchar_t *)wParam));

		if (idx >= 0) {
			data->SetSelection(idx, idx);
			data->SetCaret(idx, true);
		}
		return idx;

	case NSM_SEEKTIME:
		{
			int eventCount = data->totalCount;
			for (int i = 0; i < eventCount; i++) {
				auto *p = data->GetItem(i);
				if (p->dbe.timestamp >= wParam) {
					data->SetSelection(i, i);
					data->SetCaret(i, true);
					break;
				}

				if (i == eventCount - 1) {
					data->SetSelection(i, i);
					data->SetCaret(i, true);
				}
			}
		}
		return TRUE;

	case NSM_SEEKEND:
		data->SetCaret(data->totalCount - 1, true);
		break;

	case NSM_SET_SRMM:
		data->pMsgDlg = (CSrmmBaseDialog *)lParam;
		lParam = data->pMsgDlg->m_hContact;
		__fallthrough;

	case NSM_SET_CONTACT:
		WindowList_Add(g_hNewstoryLogs, hwnd, lParam);
		break;

	case NSM_COPY:
		{
			CMStringW res;

			int eventCount = data->totalCount;
			for (int i = 0; i < eventCount; i++) {
				ItemData *p = data->GetItem(i);
				if (!p->m_bSelected)
					continue;

				if (p->m_bOfflineFile) {
					DB::EventInfo dbei(p->hEvent);
					DB::FILE_BLOB blob(dbei);
					if (p->m_bOfflineDownloaded)
						res.Append(blob.getLocalName());
					else
						res.Append(_A2T(blob.getUrl()));
					res.Append(L"\r\n");
				}
				else res.Append(ptrW(TplFormatString(p->getCopyTemplate(), p->hContact, p)));
			}

			Utils_ClipboardCopy(res);
		}
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case NSM_DOWNLOAD:
		if (auto *p = data->LoadItem(data->caret))
			Srmm_DownloadOfflineFile(p->hContact, p->hEvent, lParam);
		break;

	case NSM_SET_OPTIONS:
		data->bSortAscending = g_plugin.bSortAscending;
		data->ScheduleDraw();
		break;

	case UM_EDITEVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			auto *p = data->LoadItem(idx);
			p->load(true);
			p->setText();
			data->ScheduleDraw();
		}
		break;

	case UM_REMOVEEVENT:
		idx = data->items.find(lParam);
		if (idx != -1) {
			data->items.remove(idx);
			data->ScheduleDraw();
			data->totalCount--;
		}
		break;

	case WM_SIZE:
		data->OnResize(LOWORD(lParam));
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case WM_COMMAND:
		if (NSMenu_Process(LOWORD(wParam), data))
			return 1;
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		PAINTSTRUCT ps;
		{
			HDC hdcWindow = BeginPaint(hwnd, &ps);

			/* we get so many InvalidateRect()'s that there is no point painting,
			Windows in theory shouldn't queue up WM_PAINTs in this case but it does so
			we'll just ignore them */
			if (IsWindowVisible(hwnd)) {
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

				if (top <= height) {
					RECT rc2;
					SetRect(&rc2, 0, top, width, height);

					HBRUSH hbr = CreateSolidBrush(g_colorTable[COLOR_BACK].cl);
					FillRect(hdc, &rc2, hbr);
					DeleteObject(hbr);
				}

				if (g_plugin.bDrawEdge)
					DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

				BitBlt(hdcWindow, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);
				DeleteObject(SelectObject(hdc, hbmSave));
				DeleteDC(hdc);
			}
		}
		EndPaint(hwnd, &ps);
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
			data->SetSelection(idx, idx);
			data->SetCaret(idx, true);
			data->OnContextMenu(idx, pt);
		}
		break;

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
			case 'B':
				if (isCtrl)
					data->ToggleBookmark();
				break;

			case VK_UP:
				data->LineUp();
				break;

			case VK_DOWN:
				data->LineDown();
				break;

			case VK_PRIOR:
				if (isCtrl)
					data->ScrollTop();
				else
					data->PageUp();
				break;

			case VK_NEXT:
				if (isCtrl)
					data->ScrollBottom();
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
				data->BeginEditItem(data->caret, false);
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
					PostMessage(hwnd, NSM_COPY, 0, 0);
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
				data->SetCaret(idx, true);
			}
			else if (wParam & MK_SHIFT) {
				data->AddSelection(data->caret, idx);
				data->SetCaret(idx, true);
			}
			else {
				pt.y -= pItem->savedTop;

				CMStringW wszUrl;
				if (pItem->isLink(pt, wszUrl)) {
					Utils_OpenUrlW(wszUrl);
					return 0;
				}

				data->SetSelection(idx, idx);
				data->SetCaret(idx, true);
			}
		}
		SetFocus(hwnd);
		return 0;

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
				data->BeginEditItem(idx, true);
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
				data->LineUp();
				break;
			case SB_LINEDOWN:
				data->LineDown();
				break;
			case SB_PAGEUP:
				data->PageUp();
				break;
			case SB_PAGEDOWN:
				data->PageDown();
				break;
			case SB_BOTTOM:
				data->ScrollBottom();
				break;
			case SB_TOP:
				data->ScrollTop();
				break;
			case SB_THUMBTRACK:
				{
					SCROLLINFO si;
					si.cbSize = sizeof(si);
					si.fMask = SIF_TRACKPOS | SIF_RANGE;
					GetScrollInfo(hwnd, SB_VERT, &si);
					data->SetPos(data->totalCount * si.nTrackPos / si.nMax);
					data->FixScrollPosition();
				}
				break;

			default:
				return 0;
			}

			if (s_scrollTopItem != data->scrollTopItem || s_scrollTopPixel != data->scrollTopPixel)
				InvalidateRect(hwnd, 0, FALSE);
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
