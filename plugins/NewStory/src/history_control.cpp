#include "stdafx.h"

#define AVERAGE_ITEM_HEIGHT 100

HANDLE htuLog = 0;

static WNDPROC OldEditWndProc;
static wchar_t wszDelete[] = LPGENW("Are you sure to remove all events from history?");

static LRESULT CALLBACK HistoryEditWndProc(HWND, UINT, WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////
// Control utilities, types and constants

struct NewstoryListData : public MZeroedObject
{
	NewstoryListData(HWND _1) :
		hwnd(_1),
		redrawTimer(Miranda_GetSystemWindow(), (LPARAM)this)
	{
		redrawTimer.OnEvent = Callback(this, &NewstoryListData::OnTimer);
	}

	HistoryArray items;

	int scrollTopItem; // topmost item
	int scrollTopPixel; // y coord of topmost item, this should be negative or zero
	int caret;
	int cachedWindowHeight;
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;

	RECT rcLastPaint;
	bool repaint;

	HWND hwnd;
	HWND hwndEditBox;

	CTimer redrawTimer;
	CSrmmBaseDialog *pMsgDlg = nullptr;

	void OnContextMenu(int index, POINT pt)
	{
		ItemData* item = items[index];
		if (item == nullptr)
			return;
		
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXTMENU));
		TranslateMenu(hMenu);

		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		UINT ret;
		if (pMsgDlg != nullptr && pMsgDlg->isChat()) {
			EnableMenuItem(hSubMenu, 2, MF_BYPOSITION | MF_GRAYED);

			pMsgDlg->m_bInMenu = true;
			ret = Chat_CreateMenu(hwnd, hSubMenu, pt, pMsgDlg->getChat(), nullptr);
			pMsgDlg->m_bInMenu = false;
		}
		else ret = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);

		switch(ret) {
		case ID_CONTEXT_COPY:
			SendMessage(hwnd, NSM_COPY, 0, 0);
			break;

		case ID_CONTEXT_EDIT:
			BeginEditItem(index);
			break;

		case ID_CONTEXT_DELETE:
			break;

		case ID_CONTEXT_SELECTALL:
			SendMessage(hwnd, NSM_SELECTITEMS, 0, items.getCount() - 1);
			break;

		default:
			if (pMsgDlg != nullptr) {
				PostMessage(pMsgDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
				Chat_DoEventHook(pMsgDlg->getChat(), GC_USER_LOGMENU, nullptr, nullptr, ret);
			}
		}

		DestroyMenu(hMenu);
	}

	void OnTimer(CTimer *pTimer)
	{
		pTimer->Stop();

		scrollTopItem = items.getCount();
		FixScrollPosition();
		InvalidateRect(hwnd, 0, FALSE);
	}

	void BeginEditItem(int index)
	{
		if (hwndEditBox)
			EndEditItem();

		if (scrollTopItem > index)
			return;

		RECT rc; GetClientRect(hwnd, &rc);
		int height = rc.bottom - rc.top;

		int top = scrollTopPixel;
		int idx = scrollTopItem;
		int itemHeight = LayoutItem(idx);
		while (top < height) {
			if (idx == index) {
				ItemData *item = items[index];

				int fontid, colorid;
				item->getFontColor(fontid, colorid);

				ptrW text(TplFormatString(item->getCopyTemplate(), item->hContact, item));
				hwndEditBox = CreateWindow(L"EDIT", text, WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, 0, top, rc.right - rc.left, itemHeight, hwnd, NULL, g_plugin.getInst(), NULL);
				OldEditWndProc = (WNDPROC)SetWindowLongPtr(hwndEditBox, GWLP_WNDPROC, (LONG_PTR)HistoryEditWndProc);
				SendMessage(hwndEditBox, WM_SETFONT, (WPARAM)g_fontTable[fontid].hfnt, 0);
				SendMessage(hwndEditBox, EM_SETMARGINS, EC_RIGHTMARGIN, 100);
				SendMessage(hwndEditBox, EM_SETSEL, 0, (LPARAM)(-1));
				ShowWindow(hwndEditBox, SW_SHOW);
				SetFocus(hwndEditBox);
				break;
			}
			top += itemHeight;
			idx++;
			itemHeight = LayoutItem(idx);
		}
	}

	void EndEditItem()
	{
		DestroyWindow(hwndEditBox);
		hwndEditBox = 0;
	}

	void EnsureVisible(int item)
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
			int itemHeight = LayoutItem(idx);
			bool found = false;
			while (top < height) {
				if (idx == item) {
					itemHeight = LayoutItem(idx);
					if (top + itemHeight > height)
						ScrollListBy(0, height - top - itemHeight);
					found = true;
					break;
				}
				top += itemHeight;
				idx++;
				itemHeight = LayoutItem(idx);
			}
			if (!found) {
				scrollTopItem = item;
				scrollTopPixel = 0;
			}
		}
		FixScrollPosition();
	}

	void FixScrollPosition()
	{
		EndEditItem();

		RECT rc;
		GetWindowRect(hwnd, &rc);
		int windowHeight = rc.bottom - rc.top;

		if (windowHeight != cachedWindowHeight || cachedMaxTopItem != scrollTopItem) {
			int maxTopItem = 0;
			int tmp = 0;
			for (maxTopItem = items.getCount(); (maxTopItem > 0) && (tmp < windowHeight); maxTopItem--)
				tmp += LayoutItem(maxTopItem - 1);
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

	int GetItemFromPixel(int yPos)
	{
		int count = items.getCount();
		if (!count)
			return -1;
		
		RECT rc;
		GetClientRect(hwnd, &rc);

		int height = rc.bottom - rc.top;
		int current = scrollTopItem;
		int top = scrollTopPixel;
		int bottom = top + LayoutItem(current);
		while (top <= height) {
			if (yPos >= top && yPos <= bottom)
				return current;
			if (++current >= count)
				break;
			top = bottom;
			bottom = top + LayoutItem(current);
		}

		return -1;
	}

	int LayoutItem(int index)
	{
		HDC hdc = GetDC(hwnd);
		RECT rc; GetClientRect(hwnd, &rc);
		int width = rc.right - rc.left;

		ItemData *item = items[index];
		if (!item) {
			DeleteDC(hdc);
			return 0;
		}

		int fontid, colorid;
		item->getFontColor(fontid, colorid);
		item->checkCreate(hwnd);

		HFONT hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);

		SIZE sz;
		sz.cx = width - 6;
		MTextMeasure(hdc, &sz, (HANDLE)item->data);

		SelectObject(hdc, hfnt);

		ReleaseDC(hwnd, hdc);
		return sz.cy + 5;
	}

	int PaintItem(HDC hdc, int index, int top, int width)
	{
		auto *item = items[index];
		item->savedTop = top;

		//	LOGFONT lfText;
		COLORREF clText, clBack, clLine;
		int fontid, colorid;
		item->getFontColor(fontid, colorid);

		clText = g_fontTable[fontid].cl;
		if (item->bSelected) {
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

	void RecalcScrollBar()
	{
		SCROLLINFO si = { 0 };
		RECT clRect;
		GetClientRect(hwnd, &clRect);
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = items.getCount() * AVERAGE_ITEM_HEIGHT;
		si.nPage = clRect.bottom;
		si.nPos = scrollTopItem * AVERAGE_ITEM_HEIGHT;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	}

	void ScrollListBy(int scrollItems, int scrollPixels)
	{
		if (scrollItems) {
			scrollTopItem += scrollItems;
			scrollTopPixel = 0;
		}
		else if (scrollPixels) {
			scrollTopPixel += scrollPixels;
			if (scrollTopPixel > 0) {
				while ((scrollTopPixel > 0) && scrollTopItem) {
					scrollTopItem--;
					int itemHeight = LayoutItem(scrollTopItem);
					scrollTopPixel -= itemHeight;
				}

				if (scrollTopPixel > 0) {
					scrollTopPixel = 0;
				}
			}
			else if (scrollTopPixel < 0) {
				int maxItem = items.getCount();
				int itemHeight = LayoutItem(scrollTopItem);
				while ((-scrollTopPixel > itemHeight) && (scrollTopItem < maxItem)) {
					scrollTopPixel += itemHeight;
					scrollTopItem++;
					itemHeight = LayoutItem(scrollTopItem);
				}
			}
		}

		FixScrollPosition();
	}
};

// Edit box
static LRESULT CALLBACK HistoryEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto *pData = (NewstoryListData *)GetWindowLongPtr(GetParent(hwnd), 0);

	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			pData->EndEditItem();
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
		pData->EndEditItem();
		return 0;
	}

	return CallWindowProc(OldEditWndProc, hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////
// WndProc

LRESULT CALLBACK NewstoryListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int idx;
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
		if (auto *p = (ADDEVENTS *)wParam)
			data->items.addEvent(p->hContact, p->hFirstEVent, p->eventCount);

		data->redrawTimer.Stop();
		data->redrawTimer.Start(100);
		break;

	case NSM_ADDCHATEVENT:
		data->items.addChatEvent((SESSION_INFO *)wParam, (LOGINFO*)lParam);

		data->redrawTimer.Stop();
		data->redrawTimer.Start(100);
		break;

	case NSM_CLEAR:
		data->items.clear();

		data->redrawTimer.Stop();
		data->redrawTimer.Start(100);
		break;

	case NSM_GETARRAY:
		return (LRESULT)&data->items;

	case NSM_GETCOUNT:
		return data->items.getCount();

	case NSM_SELECTITEMS:
		{
			int start = min(data->items.getCount() - 1, wParam);
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end)
				std::swap(start, end);

			for (int i = start; i <= end; ++i) {
				auto *p = data->items.get(i, false);
				p->bSelected = true;
			}
			
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_TOGGLEITEMS:
		{
			int start = min(data->items.getCount() - 1, wParam);
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end)
				std::swap(start, end);

			for (int i = start; i <= end; ++i) {
				auto *p = data->items.get(i, false);
				p->bSelected = !p->bSelected;
			}
			
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_SELECTITEMS2:
		{
			int start = min(data->items.getCount() - 1, wParam);
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end)
				std::swap(start, end);

			int count = data->items.getCount();
			for (int i = 0; i < count; ++i) {
				auto *p = data->items.get(i, false);
				if ((i >= start) && (i <= end)) 
					p->bSelected = true;
				else
					p->bSelected = false;
			}
			
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_DESELECTITEMS:
		{
			int start = min(data->items.getCount() - 1, wParam);
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end) {
				start ^= end;
				end ^= start;
				start ^= end;
			}
			for (int i = start; i <= end; ++i) {
				auto *p = data->items.get(i, false);
				p->bSelected = false;
			}
			
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_ENSUREVISIBLE:
		data->EnsureVisible(wParam);
		return 0;

	case NSM_GETITEMFROMPIXEL:
		return data->GetItemFromPixel(lParam);

	case NSM_SETCARET:
		if (wParam < data->items.getCount()) {
			data->caret = wParam;
			if (lParam)
				SendMessage(hwnd, NSM_ENSUREVISIBLE, data->caret, 0);
		}

	case NSM_GETCARET:
		return data->caret;

	case NSM_FINDNEXT:
		idx = data->items.FindNext(data->caret, Filter(Filter::EVENTONLY, (wchar_t *)wParam));
		if (idx >= 0) {
			SendMessage(hwnd, NSM_SELECTITEMS2, idx, idx);
			SendMessage(hwnd, NSM_SETCARET, idx, TRUE);
		}
		return idx;

	case NSM_FINDPREV:
		idx = data->items.FindPrev(data->caret, Filter(Filter::EVENTONLY, (wchar_t *)wParam));
		if (idx >= 0) {
			SendMessage(hwnd, NSM_SELECTITEMS2, idx, idx);
			SendMessage(hwnd, NSM_SETCARET, idx, TRUE);
		}
		return idx;

	case NSM_SEEKTIME:
		{
			int eventCount = data->items.getCount();
			for (int i = 0; i < eventCount; i++) {
				auto *p = data->items.get(i, false);
				if (p->dbe.timestamp >= wParam) {
					SendMessage(hwnd, NSM_SELECTITEMS2, i, i);
					SendMessage(hwnd, NSM_SETCARET, i, TRUE);
					break;
				}

				if (i == eventCount - 1) {
					SendMessage(hwnd, NSM_SELECTITEMS2, i, i);
					SendMessage(hwnd, NSM_SETCARET, i, TRUE);
				}
			}
		}
		return TRUE;

	case NSM_SEEKEND:
		SendMessage(hwnd, NSM_SETCARET, data->items.getCount() - 1, 1);
		break;

	case NSM_SET_SRMM:
		data->pMsgDlg = (CSrmmBaseDialog *)lParam;
		break;

	case NSM_DELETE:
		if (IDYES == MessageBoxW(hwnd, TranslateW(wszDelete), _T(MODULETITLE), MB_YESNOCANCEL | MB_ICONQUESTION)) {
			db_set_safety_mode(false);

			int eventCount = data->items.getCount();
			for (int i = eventCount - 1; i >= 0; i--) {
				auto *p = data->items.get(i, false);
				if (p->hEvent && p->hContact)
					db_event_delete(p->hEvent);
			}
			db_set_safety_mode(true);

			data->items.reset();

			InvalidateRect(hwnd, 0, FALSE);
		}
		break;

	case NSM_COPY:
		{
			CMStringW res;

			int eventCount = data->items.getCount();
			for (int i = 0; i < eventCount; i++) {
				ItemData *p = data->items.get(i, false);
				if (p->bSelected)
					res.Append(ptrW(TplFormatString(p->getCopyTemplate(), p->hContact, p)));
			}

			CopyText(hwnd, res);
		}
		__fallthrough;
		// End of history list control messages

	case WM_SIZE:
		InvalidateRect(hwnd, 0, FALSE);
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
				while ((top < height) && (idx < data->items.getCount()))
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
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1)
				GetCursorPos(&pt);

			POINT pt2 = pt;
			ScreenToClient(hwnd, &pt2);

			int index = data->GetItemFromPixel(pt2.y);
			if (index != -1)
				data->OnContextMenu(index, pt);
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
		switch (wParam) {
		case VK_UP:
			SendMessage(hwnd, NSM_SELECTITEMS2, data->caret - 1, data->caret - 1);
			SendMessage(hwnd, NSM_SETCARET, data->caret - 1, TRUE);
			break;

		case VK_DOWN:
			SendMessage(hwnd, NSM_SELECTITEMS2, data->caret + 1, data->caret + 1);
			SendMessage(hwnd, NSM_SETCARET, data->caret + 1, TRUE);
			break;

		case VK_PRIOR:
			break;

		case VK_NEXT:
			break;

		case VK_HOME:
			break;

		case VK_END:
			break;

		case VK_F2:
			data->BeginEditItem(data->caret);
			break;
		}
		break;

	case WM_SYSCHAR:
	case WM_CHAR:
		if (wParam == 27) {
			if (data->hwndEditBox)
				data->EndEditItem();
		}
		else {
			char ch = MapVirtualKey((lParam >> 16) & 0xff, 1);
			if (((ch == 'C') || (ch == VK_INSERT)) && (GetKeyState(VK_CONTROL) & 0x80)) {
				PostMessage(hwnd, NSM_COPY, 0, 0);
			}
			else if ((ch == 'A') && (GetKeyState(VK_CONTROL) & 0x80)) {
				SendMessage(hwnd, NSM_SELECTITEMS, 0, data->items.getCount());
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			int item = data->GetItemFromPixel(pt.y);
			if (item >= 0) {
				if (data->caret != item)
					data->EndEditItem();

				if (wParam & MK_CONTROL) {
					SendMessage(hwnd, NSM_TOGGLEITEMS, item, item);
					SendMessage(hwnd, NSM_SETCARET, item, TRUE);
				}
				else if (wParam & MK_SHIFT) {
					SendMessage(hwnd, NSM_SELECTITEMS, data->caret, item);
					SendMessage(hwnd, NSM_SETCARET, item, TRUE);
				}
				else {
					auto *pItem = data->items[item];
					pt.y -= pItem->savedTop;
					if (pItem->isLink(pt)) {
						Utils_OpenUrlW(pItem->getWBuf());
						return 0;
					}
					
					if (data->caret == item) {
						data->BeginEditItem(item);
						return 0;
					}

					SendMessage(hwnd, NSM_SELECTITEMS2, item, item);
					SendMessage(hwnd, NSM_SETCARET, item, TRUE);
				}
			}
		}
		SetFocus(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			int item = data->GetItemFromPixel(pt.y);
			if (item >= 0) {
				auto *pItem = data->items[item];
				MTextSendMessage(hwnd, pItem->data, msg, wParam, lParam);
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			UINT scrollLines;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
				scrollLines = 3;
			data->ScrollListBy(0, (short)HIWORD(wParam) * 10 * (signed)scrollLines / WHEEL_DELTA);

			if (s_scrollTopItem != data->scrollTopItem || s_scrollTopPixel != data->scrollTopPixel)
				InvalidateRect(hwnd, 0, FALSE);
		}
		return TRUE;

	case WM_VSCROLL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			switch (LOWORD(wParam)) {
			case SB_LINEUP:
				data->ScrollListBy(0, 10);
				break;
			case SB_LINEDOWN:
				data->ScrollListBy(0, -10);
				break;
			case SB_PAGEUP:
				data->ScrollListBy(-10, 0);
				break;
			case SB_PAGEDOWN:
				data->ScrollListBy(10, 0);
				break;
			case SB_BOTTOM:
				data->scrollTopItem = data->items.getCount() - 1;
				data->scrollTopPixel = 0;
				break;
			case SB_TOP:
				data->scrollTopItem = 0;
				data->scrollTopPixel = 0;
				break;
			case SB_THUMBTRACK:
				{
					SCROLLINFO si;
					si.cbSize = sizeof(si);
					si.fMask = SIF_TRACKPOS | SIF_RANGE;
					GetScrollInfo(hwnd, SB_VERT, &si);
					int pos = si.nTrackPos;

					if (pos == si.nMax) {
						data->scrollTopItem = data->items.getCount();
						data->scrollTopPixel = -1000;
					}
					else {
						data->scrollTopItem = pos / AVERAGE_ITEM_HEIGHT;
						int itemHeight = data->LayoutItem(data->scrollTopItem);
						data->scrollTopPixel = -pos % AVERAGE_ITEM_HEIGHT * itemHeight / AVERAGE_ITEM_HEIGHT;
					}
					data->FixScrollPosition();
				}
				break;

			default:
				return 0;
			}

			if (s_scrollTopItem != data->scrollTopItem || s_scrollTopPixel != data->scrollTopPixel)
				InvalidateRect(hwnd, 0, FALSE);
			break;
		}

	case WM_DESTROY:
		delete data;
		SetWindowLongPtr(hwnd, 0, 0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitNewstoryControl()
{
	htuLog = MTextRegister("Newstory", MTEXT_FANCY_DEFAULT | MTEXT_SYSTEM_HICONS);

	WNDCLASS wndclass = {};
	wndclass.style = /*CS_HREDRAW | CS_VREDRAW | */CS_DBLCLKS | CS_GLOBALCLASS;
	wndclass.lpfnWndProc = NewstoryListWndProc;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = _T(NEWSTORYLIST_CLASS);
	RegisterClass(&wndclass);
}
