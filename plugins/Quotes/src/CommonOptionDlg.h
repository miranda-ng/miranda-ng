#ifndef __c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__
#define __c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__

class CQuotesProviderBase;

struct CCommonDlgProcData
{
	CCommonDlgProcData(const CQuotesProviderBase* pQuotesProvider)
		: m_pQuotesProvider(pQuotesProvider), m_bFireSetingsChangedEvent(false){}

	const CQuotesProviderBase* m_pQuotesProvider;
	bool m_bFireSetingsChangedEvent;
};

void CommonOptionDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, CCommonDlgProcData& rData);

#endif//__c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__
