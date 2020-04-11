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
		m_hwnd = ::CreateWindowW(_T(NEWSTORYLIST_CLASS), L"", WS_TABSTOP, 0, 0, 300, 150, m_pDlg.GetHwnd(), (HMENU)IDC_SRMM_LOG, m_pDlg.GetInst(), 0);
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
	}

	void LogEvents(DBEVENTINFO *dbei_s, bool bAppend)
	{
	}

	void LogEvents(LOGINFO *pLog, bool) override
	{
	}

	void Resize() override
	{
	}

	void ScrollToBottom() override
	{
	}
};

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg)
{
	return new CNewStoryLogWindow(pDlg);
}
