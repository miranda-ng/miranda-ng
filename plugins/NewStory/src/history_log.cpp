#include "stdafx.h"

class CNewStoryLogWindow : public CSimpleLogWindow
{
	HWND m_hwnd = nullptr;
	NewstoryListData *m_histCtrl;

public:
	CNewStoryLogWindow(CMsgDialog &pDlg) :
		CSimpleLogWindow(pDlg)
	{
	}

	void Attach() override
	{
		RECT rc;
		GetClientRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rc);

		m_hwnd = ::CreateWindowW(_T(NEWSTORYLIST_CLASS), L"NewStory", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
			0, 0, rc.left - rc.right, rc.bottom - rc.top, m_pDlg.GetHwnd(), 0, m_pDlg.GetInst(), 0);

		SendMessage(m_hwnd, NSM_SET_SRMM, 0, (LPARAM)&m_pDlg);
		m_histCtrl = (NewstoryListData *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
	}

	void Detach() override
	{
		::DestroyWindow(m_hwnd);
	}

	//////////////////////////////////////////////////////////////////////////////////////

	bool AtBottom() override
	{
		int totalCount = SendMessage(m_hwnd, NSM_GETCOUNT, 0, 0);
		int caret = SendMessage(m_hwnd, NSM_GETCARET, 0, 0);
		return caret >= totalCount - 1;
	}

	void Clear() override
	{
		m_histCtrl->Clear();
	}

	HWND GetHwnd() override
	{
		return m_hwnd;
	}

	wchar_t* GetSelection() override
	{
		return nullptr;
	}

	int GetType() override
	{
		return 1;
	}

	void LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) override
	{
		if (!bAppend)
			Clear();

		m_histCtrl->AddEvent(m_pDlg.m_hContact, hDbEventFirst, count);
	}

	void LogChatEvent(const LOGINFO &lin) override
	{
		m_histCtrl->AddChatEvent(m_pDlg.getChat(), &lin);
	}

	void Resize() override
	{
		RECT rc;
		GetWindowRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rc);

		POINT pt = { rc.left, rc.top };
		ScreenToClient(GetParent(m_hwnd), &pt);

		::SetWindowPos(m_hwnd, 0, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	void ScrollToBottom() override
	{
		::SendMessage(m_hwnd, NSM_SEEKEND, 0, 0);
	}
};

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg)
{
	return new CNewStoryLogWindow(pDlg);
}
