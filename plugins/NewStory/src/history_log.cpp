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

class CNewStoryLogWindow : public CSimpleLogWindow
{
	HWND m_hwnd = nullptr;
	NewstoryListData *m_histCtrl;

public:
	CNewStoryLogWindow(CMsgDialog &pDlg) :
		CSimpleLogWindow(pDlg)
	{}

	void Attach() override
	{
		RECT rc;
		GetClientRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rc);

		m_hwnd = ::CreateWindowW(_T(NEWSTORYLIST_CLASS), L"NewStory", WS_VISIBLE | WS_CHILD | WS_TABSTOP,
			0, 0, rc.left - rc.right, rc.bottom - rc.top, m_pDlg.GetHwnd(), 0, m_pDlg.GetInst(), 0);

		m_histCtrl = (NewstoryListData *)GetWindowLongPtr(m_hwnd, 0);
		m_histCtrl->SetDialog(&m_pDlg);
	}

	void Detach() override
	{
		WindowList_Remove(g_hNewstoryLogs, m_pDlg.GetHwnd());
		::DestroyWindow(m_hwnd);
	}

	//////////////////////////////////////////////////////////////////////////////////////

	bool AtBottom() override
	{
		return m_histCtrl->AtBottom();
	}

	void Clear() override
	{
		m_histCtrl->Clear();
	}

	HWND GetHwnd() override
	{
		return m_hwnd;
	}

	wchar_t* GetSelectedText() override
	{
		return m_histCtrl->GatherSelected(true).Detach();
	}

	int GetType() override
	{
		return 1;
	}

	void LogEvents(MEVENT hDbEvent, int count, bool bAppend) override
	{
		if (!hDbEvent)
			return;

		if (!bAppend)
			Clear();

		m_histCtrl->AddEvent(m_pDlg.m_hContact, hDbEvent, count, true);
	}

	void LogChatEvent(const LOGINFO &lin) override
	{
		m_histCtrl->AddChatEvent(m_pDlg.getChat(), &lin);
	}

	void Resize() override
	{
		bool bottomScroll = AtBottom();

		RECT rc;
		GetWindowRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rc);

		POINT pt = { rc.left, rc.top };
		ScreenToClient(GetParent(m_hwnd), &pt);

		::SetWindowPos(m_hwnd, 0, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);

		if (bottomScroll)
			ScrollToBottom();
	}

	void ScrollToBottom() override
	{
		m_histCtrl->ScrollBottom();
	}
};

CSrmmLogWindow *__cdecl NewStory_Stub(CMsgDialog &pDlg)
{
	return new CNewStoryLogWindow(pDlg);
}
