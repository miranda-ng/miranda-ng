#include "stdafx.h"

HANDLE htuLog = 0;

/////////////////////////////////////////////////////////////////////////
// Control utilities, types and constants

struct NewstoryListData : public MZeroedObject
{
	HistoryArray items;

	int scrollTopItem; // topmost item
	int scrollTopPixel; // y coord of topmost item, this should be negative or zero
	int caret;
	int cachedWindowHeight;
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;

	RECT rcLastPaint;
	bool repaint;

	HWND hwndEditBox;
};

#define AVERAGE_ITEM_HEIGHT 100

static void ScrollListBy(HWND hwnd, NewstoryListData *data, int scrollItems, int scrollPixels);
static void EnsureVisible(HWND hwnd, NewstoryListData *data, int item);
static void FixScrollPosition(HWND hwnd, NewstoryListData *data);
static void RecalcScrollBar(HWND hwnd, NewstoryListData *data);
static void BeginEditItem(HWND hwnd, NewstoryListData *data, int index);
static void EndEditItem(HWND hwnd, NewstoryListData *data);
static int LayoutItem(HWND hwnd, HistoryArray *items, int index);
static int PaintItem(HDC hdc, HistoryArray *items, int index, int top, int width);

// Edit box
static WNDPROC OldEditWndProc;
static LRESULT CALLBACK HistoryEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			{
				EndEditItem(GetParent(hwnd), (NewstoryListData *)GetWindowLongPtr(GetParent(hwnd), 0));
				return 0;
			}
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
		EndEditItem(GetParent(hwnd), (NewstoryListData *)GetWindowLong(GetParent(hwnd), 0));
		return 0;
	}

	return CallWindowProc(OldEditWndProc, hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////
// Utilities

static void ScrollListBy(HWND hwnd, NewstoryListData *data, int scrollItems, int scrollPixels)
{
	if (scrollItems) {
		data->scrollTopItem += scrollItems;
		data->scrollTopPixel = 0;
	}
	else if (scrollPixels) {
		data->scrollTopPixel += scrollPixels;
		if (data->scrollTopPixel > 0) {
			while ((data->scrollTopPixel > 0) && data->scrollTopItem) {
				data->scrollTopItem--;
				int itemHeight = LayoutItem(hwnd, &data->items, data->scrollTopItem);
				data->scrollTopPixel -= itemHeight;
			}

			if (data->scrollTopPixel > 0) {
				data->scrollTopPixel = 0;
			}
		}
		else if (data->scrollTopPixel < 0) {
			int maxItem = data->items.getCount();
			int itemHeight = LayoutItem(hwnd, &data->items, data->scrollTopItem);
			while ((-data->scrollTopPixel > itemHeight) && (data->scrollTopItem < maxItem)) {
				data->scrollTopPixel += itemHeight;
				data->scrollTopItem++;
				itemHeight = LayoutItem(hwnd, &data->items, data->scrollTopItem);
			}
		}
	}

	FixScrollPosition(hwnd, data);
}

static void EnsureVisible(HWND hwnd, NewstoryListData *data, int item)
{
	if (data->scrollTopItem >= item) {
		data->scrollTopItem = item;
		data->scrollTopPixel = 0;
	}
	else {
		RECT rc; GetClientRect(hwnd, &rc);
		int	height = rc.bottom - rc.top;
		int top = data->scrollTopPixel;
		int idx = data->scrollTopItem;
		int itemHeight = LayoutItem(hwnd, &data->items, idx);
		bool found = false;
		while (top < height) {
			if (idx == item) {
				itemHeight = LayoutItem(hwnd, &data->items, idx);
				if (top + itemHeight > height)
					ScrollListBy(hwnd, data, 0, height - top - itemHeight);
				found = true;
				break;
			}
			top += itemHeight;
			idx++;
			itemHeight = LayoutItem(hwnd, &data->items, idx);
		}
		if (!found) {
			data->scrollTopItem = item;
			data->scrollTopPixel = 0;
		}
	}
	FixScrollPosition(hwnd, data);
}

static void FixScrollPosition(HWND hwnd, NewstoryListData *data)
{
	EndEditItem(hwnd, data);

	RECT rc;
	GetWindowRect(hwnd, &rc);
	int windowHeight = rc.bottom - rc.top;

	if (windowHeight != data->cachedWindowHeight) {
		int maxTopItem = 0;
		int tmp = 0;
		for (maxTopItem = data->items.getCount(); (maxTopItem > 0) && (tmp < windowHeight); maxTopItem--)
			tmp += LayoutItem(hwnd, &data->items, maxTopItem - 1);
		data->cachedMaxTopItem = maxTopItem;
		data->cachedWindowHeight = windowHeight;
		data->cachedMaxTopPixel = (windowHeight < tmp) ? windowHeight - tmp : 0;
	}

	if (data->scrollTopItem < 0) {
		data->scrollTopItem = 0;
	}

	if ((data->scrollTopItem > data->cachedMaxTopItem) ||
		((data->scrollTopItem == data->cachedMaxTopItem) && (data->scrollTopPixel < data->cachedMaxTopPixel))) {
		data->scrollTopItem = data->cachedMaxTopItem;
		data->scrollTopPixel = data->cachedMaxTopPixel;
	}
	RecalcScrollBar(hwnd, data);
}

static void RecalcScrollBar(HWND hwnd, NewstoryListData *data)
{
	SCROLLINFO si = { 0 };
	RECT clRect;
	GetClientRect(hwnd, &clRect);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = data->items.getCount() * AVERAGE_ITEM_HEIGHT;
	si.nPage = clRect.bottom;
	si.nPos = data->scrollTopItem * AVERAGE_ITEM_HEIGHT;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

static void BeginEditItem(HWND hwnd, NewstoryListData *data, int index)
{
	if (data->hwndEditBox)
		EndEditItem(hwnd, data);

	if (data->scrollTopItem > index)
		return;

	RECT rc; GetClientRect(hwnd, &rc);
	int height = rc.bottom - rc.top;

	int top = data->scrollTopPixel;
	int idx = data->scrollTopItem;
	int itemHeight = LayoutItem(hwnd, &data->items, idx);
	while (top < height) {
		if (idx == index) {
			HistoryArray::ItemData *item = data->items.get(index, ELM_DATA);

			int tpl;
			int fontid;
			int colorid;
			switch (item->dbe.eventType) {
			case EVENTTYPE_MESSAGE:
				tpl = TPL_COPY_MESSAGE;
				fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INMSG : FONT_OUTMSG;
				colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INMSG : COLOR_OUTMSG;
				break;

			case EVENTTYPE_FILE:
				tpl = TPL_COPY_FILE;
				fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INFILE : FONT_OUTFILE;
				colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INFILE : COLOR_OUTFILE;
				break;

			case EVENTTYPE_STATUSCHANGE:
				tpl = TPL_COPY_SIGN;
				fontid = FONT_STATUS;
				colorid = COLOR_STATUS;
				break;

			case EVENTTYPE_AUTHREQUEST:
				tpl = TPL_COPY_AUTH;
				fontid = FONT_INOTHER;
				colorid = COLOR_INOTHER;
				break;

			case EVENTTYPE_ADDED:
				tpl = TPL_COPY_ADDED;
				fontid = FONT_INOTHER;
				colorid = COLOR_INOTHER;
				break;

			case EVENTTYPE_JABBER_PRESENCE:
				tpl = TPL_COPY_PRESENCE;
				fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
				colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
				break;

			default:
				tpl = TPL_COPY_OTHER;
				fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
				colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
				break;
			}

			ptrW text(TplFormatString(tpl, item->hContact, item));
			data->hwndEditBox = CreateWindow(L"EDIT", text, WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, 0, top, rc.right - rc.left, itemHeight, hwnd, NULL, g_plugin.getInst(), NULL);
			OldEditWndProc = (WNDPROC)SetWindowLongPtr(data->hwndEditBox, GWLP_WNDPROC, (LONG_PTR)HistoryEditWndProc);
			SendMessage(data->hwndEditBox, WM_SETFONT, (WPARAM)fonts[fontid].hfnt, 0);
			SendMessage(data->hwndEditBox, EM_SETMARGINS, EC_RIGHTMARGIN, 100);
			SendMessage(data->hwndEditBox, EM_SETSEL, 0, (LPARAM)(-1));
			ShowWindow(data->hwndEditBox, SW_SHOW);
			SetFocus(data->hwndEditBox);
			break;
		}
		top += itemHeight;
		idx++;
		itemHeight = LayoutItem(hwnd, &data->items, idx);
	}
}

static void EndEditItem(HWND, NewstoryListData *data)
{
	DestroyWindow(data->hwndEditBox);
	data->hwndEditBox = 0;
}

static int LayoutItem(HWND hwnd, HistoryArray *items, int index)
{
	HDC hdc = GetDC(hwnd);
	RECT rc; GetClientRect(hwnd, &rc);
	int width = rc.right - rc.left;

	HistoryArray::ItemData *item = items->get(index, ELM_DATA);
	if (!item) return 0;

	int tpl;
	int fontid;
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		tpl = TPL_MESSAGE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INMSG : FONT_OUTMSG;
		break;

	case EVENTTYPE_FILE:
		tpl = TPL_FILE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INFILE : FONT_OUTFILE;
		break;

	case EVENTTYPE_STATUSCHANGE:
		tpl = TPL_SIGN;
		fontid = FONT_STATUS;
		break;

	case EVENTTYPE_AUTHREQUEST:
		tpl = TPL_AUTH;
		fontid = FONT_INOTHER;
		break;

	case EVENTTYPE_ADDED:
		tpl = TPL_ADDED;
		fontid = FONT_INOTHER;
		break;

	case EVENTTYPE_JABBER_PRESENCE:
		tpl = TPL_PRESENCE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		break;

	default:
		tpl = TPL_OTHER;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		break;
	}

	HFONT hfnt = (HFONT)SelectObject(hdc, fonts[fontid].hfnt);
	if (!item->data)
		item->data = MTextCreateW(htuLog, ptrW(TplFormatString(tpl, item->hContact, item)));

	SIZE sz;
	sz.cx = width - 6;
	MTextMeasure(hdc, &sz, (HANDLE)item->data);

	SelectObject(hdc, hfnt);

	ReleaseDC(hwnd, hdc);
	return sz.cy + 5;
}

static int PaintItem(HDC hdc, HistoryArray *items, int index, int top, int width)
{
	if (!items) return 0;
	HistoryArray::ItemData *item = items->get(index, ELM_DATA);

	//	LOGFONT lfText;
	COLORREF clText, clBack, clLine;
	int tpl;
	int fontid;
	int colorid;
	switch (item->dbe.eventType) {
	case EVENTTYPE_MESSAGE:
		tpl = TPL_MESSAGE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INMSG : FONT_OUTMSG;
		colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INMSG : COLOR_OUTMSG;
		break;

	case EVENTTYPE_FILE:
		tpl = TPL_FILE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INFILE : FONT_OUTFILE;
		colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INFILE : COLOR_OUTFILE;
		break;

	case EVENTTYPE_STATUSCHANGE:
		tpl = TPL_SIGN;
		fontid = FONT_STATUS;
		colorid = COLOR_STATUS;
		break;

	case EVENTTYPE_AUTHREQUEST:
		tpl = TPL_AUTH;
		fontid = FONT_INOTHER;
		colorid = COLOR_INOTHER;
		break;

	case EVENTTYPE_ADDED:
		tpl = TPL_ADDED;
		fontid = FONT_INOTHER;
		colorid = COLOR_INOTHER;
		break;

	case EVENTTYPE_JABBER_PRESENCE:
		tpl = TPL_PRESENCE;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
		break;

	default:
		tpl = TPL_OTHER;
		fontid = !(item->dbe.flags & DBEF_SENT) ? FONT_INOTHER : FONT_OUTOTHER;
		colorid = !(item->dbe.flags & DBEF_SENT) ? COLOR_INOTHER : COLOR_OUTOTHER;
		break;
	}
	clText = fonts[fontid].cl;
	if (item->flags & HIF_SELECTED) {
		MTextSendMessage(0, item->data, EM_SETSEL, 0, -1);
		clText = colors[COLOR_SELTEXT].cl;
		clLine = GetSysColor(COLOR_HIGHLIGHTTEXT);
		clBack = GetSysColor(COLOR_HIGHLIGHT);
	}
	else {
		MTextSendMessage(0, item->data, EM_SETSEL, 0, 0);
		clLine = colors[COLOR_SELECTED].cl;
		clBack = colors[colorid].cl;
	}

	if (!item->data) {
		wchar_t *buf = TplFormatString(tpl, item->hContact, item);
		item->data = MTextCreateW(htuLog, buf);
		mir_free(buf);
		if (!item->data)
			return 0;
	}

	SIZE sz;
	sz.cx = width - 6;
	HFONT hfnt = (HFONT)SelectObject(hdc, fonts[fontid].hfnt);
	MTextMeasure(hdc, &sz, (HANDLE)item->data);
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
	hfnt = (HFONT)SelectObject(hdc, fonts[fontid].hfnt);
	MTextDisplay(hdc, pos, sz, (HANDLE)item->data);
	SelectObject(hdc, hfnt);

	DeleteObject(hbr);

	HPEN hpn = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 1, clLine));
	MoveToEx(hdc, rc.left, rc.bottom - 1, 0);
	LineTo(hdc, rc.right, rc.bottom - 1);
	DeleteObject(SelectObject(hdc, hpn));

	return height;
}

/////////////////////////////////////////////////////////////////////////
// WndProc

LRESULT CALLBACK NewstoryListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	NewstoryListData *data = (NewstoryListData *)GetWindowLongPtr(hwnd, 0);

	switch (msg) {
	case WM_CREATE:
		data = new NewstoryListData;
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)data);
		RecalcScrollBar(hwnd, data);
		break;

	// History list control messages
	case NSM_ADDHISTORY:
		data->items.addHistory((MCONTACT)wParam);
		RecalcScrollBar(hwnd, data);
		data->scrollTopItem = data->items.getCount();
		FixScrollPosition(hwnd, data);
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case NSM_ADDEVENTS:
		{
			auto *p = (ADDEVENTS *)wParam;
			data->items.addEvent(p->hContact, p->hFirstEVent, p->eventCount);
		}
		RecalcScrollBar(hwnd, data);
		data->scrollTopItem = data->items.getCount();
		FixScrollPosition(hwnd, data);
		InvalidateRect(hwnd, 0, FALSE);
		break;

	case NSM_GETCOUNT:
		return data->items.getCount();

	case NSM_SELECTITEMS:
		{
			int start = min(data->items.getCount() - 1, max(0, wParam));
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end) {
				start ^= end;
				end ^= start;
				start ^= end;
			}
			for (int i = start; i <= end; ++i)
				data->items.get(i, ELM_NOTHING)->flags |= HIF_SELECTED;
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_TOGGLEITEMS:
		{
			int start = min(data->items.getCount() - 1, max(0, wParam));
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end) {
				start ^= end;
				end ^= start;
				start ^= end;
			}
			for (int i = start; i <= end; ++i) {
				if (data->items.get(i, ELM_NOTHING)->flags & HIF_SELECTED) {
					data->items.get(i, ELM_NOTHING)->flags &= ~HIF_SELECTED;
				}
				else {
					data->items.get(i, ELM_NOTHING)->flags |= HIF_SELECTED;
				}
			}
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_SELECTITEMS2:
		{
			int start = min(data->items.getCount() - 1, max(0, wParam));
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end) {
				start ^= end;
				end ^= start;
				start ^= end;
			}
			int count = data->items.getCount();
			for (int i = 0; i < count; ++i) {
				if ((i >= start) && (i <= end)) {
					data->items.get(i, ELM_NOTHING)->flags |= HIF_SELECTED;
				}
				else {
					data->items.get(i, ELM_NOTHING)->flags &= ~((DWORD)HIF_SELECTED);
				}
			}
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_DESELECTITEMS:
		{
			int start = min(data->items.getCount() - 1, max(0, wParam));
			int end = min(data->items.getCount() - 1, max(0, lParam));
			if (start > end) {
				start ^= end;
				end ^= start;
				start ^= end;
			}
			for (int i = start; i <= end; ++i)
				data->items.get(i, ELM_NOTHING)->flags &= ~((DWORD)HIF_SELECTED);
			InvalidateRect(hwnd, 0, FALSE);
			return 0;
		}

	case NSM_ENSUREVISIBLE:
		EnsureVisible(hwnd, data, wParam);
		return 0;

	case NSM_GETITEMFROMPIXEL:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			int height = rc.bottom - rc.top;
			DWORD count = data->items.getCount();
			DWORD current = data->scrollTopItem;
			int top = data->scrollTopPixel;
			int bottom = top + LayoutItem(hwnd, &data->items, current);
			while (top <= height) {
				if ((lParam >= top) && (lParam <= bottom))
					return current;
				if (++current >= count)
					return -1;
				top = bottom;
				bottom = top + LayoutItem(hwnd, &data->items, current);
			}
			return -1;
		}

	case NSM_SETCARET:
		if ((wParam >= 0) && (wParam < data->items.getCount())) {
			data->caret = wParam;
			if (lParam)
				SendMessage(hwnd, NSM_ENSUREVISIBLE, data->caret, 0);
		}

	case NSM_GETCARET:
		return data->caret;

	case NSM_FINDNEXT:
		{
			int id = data->items.FindNext(SendMessage(hwnd, NSM_GETCARET, 0, 0), HistoryArray::Filter(HistoryArray::Filter::EVENTONLY, (wchar_t *)wParam));
			if (id >= 0) {
				SendMessage(hwnd, NSM_SELECTITEMS2, id, id);
				SendMessage(hwnd, NSM_SETCARET, id, TRUE);
			}
			return id;
		}

	case NSM_FINDPREV:
		{
			int id = data->items.FindPrev(SendMessage(hwnd, NSM_GETCARET, 0, 0), HistoryArray::Filter(HistoryArray::Filter::EVENTONLY, (wchar_t *)wParam));
			if (id >= 0) {
				SendMessage(hwnd, NSM_SELECTITEMS2, id, id);
				SendMessage(hwnd, NSM_SETCARET, id, TRUE);
			}
			return id;
		}

	case NSM_SEEKTIME:
		{
			int eventCount = data->items.getCount();
			for (int i = 0; i < eventCount; i++) {
				auto *item = data->items.get(i, ELM_NOTHING);
				if (item->dbe.timestamp >= wParam) {
					SendMessage(hwnd, NSM_SETCARET, i, TRUE);
					break;
				}
			}
		}
		return TRUE;

	case NSM_COPY:
		{
			CMStringW res;

			int eventCount = data->items.getCount();
			for (int i = 0; i < eventCount; i++) {
				HistoryArray::ItemData *item = data->items.get(i, ELM_NOTHING);
				if (item->flags & HIF_SELECTED)
					res.Append(ptrW(TplFormatString(TPL_COPY_MESSAGE, item->hContact, item)));
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

	case WM_PRINTCLIENT:
		{
			//			PaintClc(hwnd, dat, (HDC) wParam, NULL);
			break;
		}
		/*
				case WM_NCPAINT:
				{
					RECT rc;
					GetWindowRect(hwnd, &rc);

					HDC hdc;
					hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
					FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
					ReleaseDC(hwnd, hdc);
				}
		*/
		/*
				case WM_NCPAINT:
				{
					if (wParam == 1)
						break;
					{
						POINT ptTopLeft = { 0, 0 };
						HRGN hClientRgn;
						ClientToScreen(hwnd, &ptTopLeft);
						hClientRgn = CreateRectRgn(0, 0, 1, 1);
						CombineRgn(hClientRgn, (HRGN) wParam, NULL, RGN_COPY);
						OffsetRgn(hClientRgn, -ptTopLeft.x, -ptTopLeft.y);
						InvalidateRgn(hwnd, hClientRgn, FALSE);
						DeleteObject(hClientRgn);
						UpdateWindow(hwnd);
					}
					break;
				}
		*/
	case WM_PAINT:
		{
			HDC hdcWindow;
			PAINTSTRUCT ps;
			hdcWindow = BeginPaint(hwnd, &ps);

			/* we get so many InvalidateRect()'s that there is no point painting,
			Windows in theory shouldn't queue up WM_PAINTs in this case but it does so
			we'll just ignore them */
			if (IsWindowVisible(hwnd)) {
				RECT rc;
				GetClientRect(hwnd, &rc);

				HDC hdc = CreateCompatibleDC(hdcWindow);
				HBITMAP hbmSave = (HBITMAP)SelectObject(hdc, CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top));

				GetClientRect(hwnd, &rc);
				int height = rc.bottom - rc.top;
				int width = rc.right - rc.left;
				int top = data->scrollTopPixel;
				int idx = data->scrollTopItem;
				while ((top < height) && (idx < data->items.getCount()))
					top += PaintItem(hdc, &data->items, idx++, top, width);

				if (top <= height) {
					RECT rc2;
					SetRect(&rc2, 0, top, width, height);

					HBRUSH hbr;
					hbr = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
					FillRect(hdc, &rc2, hbr);
					DeleteObject(hbr);
				}

				GetWindowRect(hwnd, &rc);
				rc.right -= rc.left; rc.left = 0;
				rc.bottom -= rc.top; rc.top = 0;
				DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

				BitBlt(hdcWindow, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);
				DeleteObject(SelectObject(hdc, hbmSave));
				DeleteDC(hdc);
			}

			EndPaint(hwnd, &ps);
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
				BeginEditItem(hwnd, data, data->caret);
				break;
			}
			break;
		}

	case WM_SYSCHAR:
	case WM_CHAR:
		if (wParam == 27) {
			if (data->hwndEditBox)
				EndEditItem(hwnd, data);
		}
		else {
			char ch = MapVirtualKey((lParam >> 16) & 0xff, 1);
			if (((ch == 'C') || (ch == VK_INSERT)) && (GetKeyState(VK_CONTROL) & 0x80)) {
				PostMessage(hwnd, NSM_COPY, 0, 0);
			}
			else if ((ch == 'A') && (GetKeyState(VK_CONTROL) & 0x80)) {
				SendMessage(hwnd, NSM_SELECTITEMS, 0, data->items.getCount());
				//				} else
				//				if (ch == VK_ESCAPE)
				//				{
				//					PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int item = SendMessage(hwnd, NSM_GETITEMFROMPIXEL, LOWORD(lParam), HIWORD(lParam));
			if (item >= 0) {
				if (data->caret != item)
					EndEditItem(hwnd, data);

				if (wParam & MK_CONTROL) {
					SendMessage(hwnd, NSM_TOGGLEITEMS, item, item);
					SendMessage(hwnd, NSM_SETCARET, item, TRUE);
				}
				else if (wParam & MK_SHIFT) {
					SendMessage(hwnd, NSM_SELECTITEMS, data->caret, item);
					SendMessage(hwnd, NSM_SETCARET, item, TRUE);
				}
				else {
					if (data->caret == item) {
						BeginEditItem(hwnd, data, item);
					}
					else {
						SendMessage(hwnd, NSM_SELECTITEMS2, item, item);
						SendMessage(hwnd, NSM_SETCARET, item, TRUE);
					}
				}
			}
		}
		SetFocus(hwnd);
		return 0;

	case WM_MOUSEWHEEL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			UINT scrollLines;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
				scrollLines = 3;
			ScrollListBy(hwnd, data, 0, (short)HIWORD(wParam) * 10 * (signed)scrollLines / WHEEL_DELTA);

			if ((s_scrollTopItem != data->scrollTopItem) || (s_scrollTopPixel != data->scrollTopPixel))
				InvalidateRect(hwnd, 0, FALSE);
		}
		return TRUE;

	case WM_VSCROLL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			switch (LOWORD(wParam)) {
			case SB_LINEUP:
				ScrollListBy(hwnd, data, 0, 10);
				break;
			case SB_LINEDOWN:
				ScrollListBy(hwnd, data, 0, -10);
				break;
			case SB_PAGEUP:
				ScrollListBy(hwnd, data, -10, 0);
				break;
			case SB_PAGEDOWN:
				ScrollListBy(hwnd, data, 10, 0);
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
						int itemHeight = LayoutItem(hwnd, &data->items, data->scrollTopItem);
						data->scrollTopPixel = -pos % AVERAGE_ITEM_HEIGHT * itemHeight / AVERAGE_ITEM_HEIGHT;
					}
					FixScrollPosition(hwnd, data);
				}
				break;

			default:
				return 0;
			}

			if ((s_scrollTopItem != data->scrollTopItem) || (s_scrollTopPixel != data->scrollTopPixel))
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
