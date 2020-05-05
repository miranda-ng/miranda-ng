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

		int tpl;
		int fontid;
		switch (item->dbe.eventType) {
		case EVENTTYPE_MESSAGE:
			tpl = item->isGrouped() ? TPL_MSG_GRP : TPL_MESSAGE;
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

		HFONT hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
		if (!item->data)
			item->data = MTextCreateW(htuLog, ptrW(TplFormatString(tpl, item->hContact, item)));

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

		//	LOGFONT lfText;
		COLORREF clText, clBack, clLine;
		int tpl;
		int fontid;
		int colorid;
		switch (item->dbe.eventType) {
		case EVENTTYPE_MESSAGE:
			tpl = item->isGrouped() ? TPL_MSG_GRP : TPL_MESSAGE;
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

		if (!item->data) {
			item->data = MTextCreateW(htuLog, ptrW(TplFormatString(tpl, item->hContact, item)));
			if (!item->data)
				return 0;
		}

		SIZE sz;
		sz.cx = width - 6;
		HFONT hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
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
		hfnt = (HFONT)SelectObject(hdc, g_fontTable[fontid].hfnt);
		MTextDisplay(hdc, pos, sz, (HANDLE)item->data);
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
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			int height = rc.bottom - rc.top;
			int count = data->items.getCount();
			int current = data->scrollTopItem;
			int top = data->scrollTopPixel;
			int bottom = top + data->LayoutItem(current);
			while (top <= height) {
				if ((lParam >= top) && (lParam <= bottom))
					return current;
				if (++current >= count)
					return -1;
				top = bottom;
				bottom = top + data->LayoutItem(current);
			}
			return -1;
		}

	case NSM_SETCARET:
		if (wParam < data->items.getCount()) {
			data->caret = wParam;
			if (lParam)
				SendMessage(hwnd, NSM_ENSUREVISIBLE, data->caret, 0);
		}

	case NSM_GETCARET:
		return data->caret;

	case NSM_FINDNEXT:
		{
			int id = data->items.FindNext(SendMessage(hwnd, NSM_GETCARET, 0, 0), Filter(Filter::EVENTONLY, (wchar_t *)wParam));
			if (id >= 0) {
				SendMessage(hwnd, NSM_SELECTITEMS2, id, id);
				SendMessage(hwnd, NSM_SETCARET, id, TRUE);
			}
			return id;
		}

	case NSM_FINDPREV:
		{
			int id = data->items.FindPrev(SendMessage(hwnd, NSM_GETCARET, 0, 0), Filter(Filter::EVENTONLY, (wchar_t *)wParam));
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
					res.Append(ptrW(TplFormatString(TPL_COPY_MESSAGE, p->hContact, p)));
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
		{
			PAINTSTRUCT ps;
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
				int idx = data->scrollTopItem;
				while ((top < height) && (idx < data->items.getCount()))
					top += data->PaintItem(hdc, idx++, top, width);

				if (top <= height) {
					RECT rc2;
					SetRect(&rc2, 0, top, width, height);

					HBRUSH hbr;
					hbr = CreateSolidBrush(g_colorTable[COLOR_BACK].cl);
					FillRect(hdc, &rc2, hbr);
					DeleteObject(hbr);
				}

				if (g_plugin.bDrawEdge)
					DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

				BitBlt(hdcWindow, 0, 0, rc.right, rc.bottom, hdc, 0, 0, SRCCOPY);
				DeleteObject(SelectObject(hdc, hbmSave));
				DeleteDC(hdc);
			}

			EndPaint(hwnd, &ps);
		}
		break;


	case WM_CONTEXTMENU:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1)
				GetCursorPos(&pt);

			POINT pt2 = pt;
			ScreenToClient(hwnd, &pt2);

			int index = SendMessage(hwnd, NSM_GETITEMFROMPIXEL, pt2.x, pt2.y);
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
				data->BeginEditItem(data->caret);
				break;
			}
			break;
		}

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
			int item = SendMessage(hwnd, NSM_GETITEMFROMPIXEL, LOWORD(lParam), HIWORD(lParam));
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

	case WM_MOUSEWHEEL:
		{
			int s_scrollTopItem = data->scrollTopItem;
			int s_scrollTopPixel = data->scrollTopPixel;

			UINT scrollLines;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
				scrollLines = 3;
			data->ScrollListBy(0, (short)HIWORD(wParam) * 10 * (signed)scrollLines / WHEEL_DELTA);

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
