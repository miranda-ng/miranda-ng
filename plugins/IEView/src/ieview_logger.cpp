/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_ERRMSG 25366

/////////////////////////////////////////////////////////////////////////////////////////
// CIeviewLogWindow class

class CIeviewLogWindow : public CSrmmLogWindow
{
	HWND m_hwnd = nullptr;

public:
	CIeviewLogWindow(CMsgDialog &pDlg) :
		CSrmmLogWindow(pDlg)
	{
	}

	void Attach() override
	{
		IEVIEWWINDOW ieWindow = {};
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = m_pDlg.GetHwnd();
		ieWindow.cx = 200;
		ieWindow.cy = 200;
		HandleIEWindow(0, LPARAM(&ieWindow));
		m_hwnd = ieWindow.hwnd;
	}

	void Detach() override
	{
		IEVIEWWINDOW ieWindow = {};
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwnd;
		HandleIEWindow(0, LPARAM(&ieWindow));
	}

	//////////////////////////////////////////////////////////////////////////////////////

	bool AtBottom() override
	{
		return false;
	}
	
	void Clear() override
	{
		IEVIEWEVENT event = {};
		event.iType = IEE_CLEAR_LOG;
		event.hwnd = m_hwnd;
		event.hContact = m_pDlg.m_hContact;
		HandleIEEvent(0, LPARAM(&event));
	}

	HWND GetHwnd() override
	{
		return m_hwnd;
	}
	
	wchar_t* GetSelection() override
	{
		auto *view = IEView::get(m_hwnd);
		return (view != nullptr) ? view->selection() : nullptr;
	}

	int GetType() override
	{
		return 1;
	}

	void LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) override
	{
		if (!bAppend)
			Clear();

		IEVIEWEVENT event = {};
		event.iType = IEE_LOG_DB_EVENTS;
		event.hwnd = m_hwnd;
		event.hDbEventFirst = hDbEventFirst;
		event.hContact = m_pDlg.m_hContact;
		event.count = count;
		HandleIEEvent(0, LPARAM(&event));
	}

	void LogEvents(LOGINFO *pLog, bool bRedraw) override
	{
		IEVIEWEVENTDATA ied = {};
		ied.dwFlags = IEEDF_UNICODE_NICK;

		IEVIEWEVENT event = {};
		event.hwnd = m_hwnd;
		event.hContact = m_pDlg.m_hContact;
		event.codepage = CP_UTF8;
		event.iType = IEE_LOG_MEM_EVENTS;
		event.eventData = &ied;
		event.count = 1;

		while (pLog) {
			ied.szNick.w = pLog->ptszNick;
			ied.szText.w = pLog->ptszText;
			ied.time = pLog->time;
			ied.bIsMe = pLog->bIsMe;

			switch (pLog->iType) {
			case GC_EVENT_MESSAGE:
				ied.iType = IEED_GC_EVENT_MESSAGE;
				ied.dwData = IEEDD_GC_SHOW_NICK;
				break;
			case GC_EVENT_ACTION:
				ied.iType = IEED_GC_EVENT_ACTION;
				break;
			case GC_EVENT_JOIN:
				ied.iType = IEED_GC_EVENT_JOIN;
				break;
			case GC_EVENT_PART:
				ied.iType = IEED_GC_EVENT_PART;
				break;
			case GC_EVENT_QUIT:
				ied.iType = IEED_GC_EVENT_QUIT;
				break;
			case GC_EVENT_NICK:
				ied.iType = IEED_GC_EVENT_NICK;
				break;
			case GC_EVENT_KICK:
				ied.iType = IEED_GC_EVENT_KICK;
				break;
			case GC_EVENT_NOTICE:
				ied.iType = IEED_GC_EVENT_NOTICE;
				break;
			case GC_EVENT_TOPIC:
				ied.iType = IEED_GC_EVENT_TOPIC;
				break;
			case GC_EVENT_INFORMATION:
				ied.iType = IEED_GC_EVENT_INFORMATION;
				break;
			case GC_EVENT_ADDSTATUS:
				ied.iType = IEED_GC_EVENT_ADDSTATUS;
				break;
			case GC_EVENT_REMOVESTATUS:
				ied.iType = IEED_GC_EVENT_REMOVESTATUS;
				break;
			}

			ied.dwData |= IEEDD_GC_SHOW_TIME | IEEDD_GC_SHOW_ICON;
			ied.dwFlags = IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;
			HandleIEEvent(0, LPARAM(&event));

			pLog = pLog->prev;
		}

		if (bRedraw)
			ScrollToBottom();
	}

	void Resize() override
	{
		RECT rcRichEdit;
		GetWindowRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rcRichEdit);

		POINT pt = { rcRichEdit.left, rcRichEdit.top };
		ScreenToClient(GetParent(m_hwnd), &pt);

		IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = m_hwnd;
		ieWindow.hwnd = m_hwnd;
		ieWindow.x = pt.x;
		ieWindow.y = pt.y;
		ieWindow.cx = rcRichEdit.right - rcRichEdit.left;
		ieWindow.cy = rcRichEdit.bottom - rcRichEdit.top;
		if (ieWindow.cx != 0 && ieWindow.cy != 0)
			HandleIEWindow(0, LPARAM(&ieWindow));
	}

	void ScrollToBottom() override
	{
		auto *view = IEView::get(m_hwnd);
		if (view != nullptr)
			view->scrollToBottom();
	}
};

CSrmmLogWindow *logBuilder(CMsgDialog &pDlg)
{
	return new CIeviewLogWindow(pDlg);
}
