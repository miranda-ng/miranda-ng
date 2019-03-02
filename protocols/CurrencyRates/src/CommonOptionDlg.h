#ifndef __c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__
#define __c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__

class CCurrencyRatesProviderBase;

struct CCommonDlgProcData
{
	CCommonDlgProcData(const CCurrencyRatesProviderBase* pCurrencyRatesProvider)
		: m_pCurrencyRatesProvider(pCurrencyRatesProvider), m_bFireSetingsChangedEvent(false){}

	const CCurrencyRatesProviderBase* m_pCurrencyRatesProvider;
	bool m_bFireSetingsChangedEvent;
};

void CommonOptionDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, CCommonDlgProcData& rData);

#endif//__c85fe710_f71b_4a58_9d44_3e39f6209c5f_CommonOptionDlg_h__
