#include "stdafx.h"

class CNewStoryLogWindow : public CSrmmLogWindow
{
	HWND m_hwnd = nullptr;

public:
	CNewStoryLogWindow(CMsgDialog &pDlg) :
		CSrmmLogWindow(pDlg)
	{
	}

	void Attach() override
	{
		m_hwnd = ::CreateWindow(_T(NEWSTORYLIST_CLASS), L"NewStory", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 300, 150, m_pDlg.GetHwnd(), 0, m_pDlg.GetInst(), 0);
	}

	void Detach() override
	{
		::DestroyWindow(m_hwnd);
	}

	//////////////////////////////////////////////////////////////////////////////////////

	bool AtBottom() override
	{
		return false;
	}

	void Clear() override
	{
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
		if (count != -1) {
			ADDEVENTS tmp = { m_pDlg.m_hContact, hDbEventFirst, count };
			SendMessage(m_hwnd, NSM_ADDEVENTS, (LPARAM)&tmp, 0);
		}
		else SendMessage(m_hwnd, NSM_ADDHISTORY, m_pDlg.m_hContact, 0);
	}

	void LogEvents(DBEVENTINFO *dbei_s, bool bAppend)
	{
	}

	void LogEvents(LOGINFO *pLog, bool) override
	{
	}

	void Resize() override
	{
		RECT rc;
		GetClientRect(GetDlgItem(m_pDlg.GetHwnd(), IDC_SRMM_LOG), &rc);

		::SetWindowPos(m_hwnd, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	void ScrollToBottom() override
	{
	}
};

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg)
{
	return new CNewStoryLogWindow(pDlg);
}
