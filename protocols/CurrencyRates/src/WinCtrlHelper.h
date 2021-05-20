#ifndef __a05d6852_4497_4f28_85e1_48a15a170738_WinCtrlHelper_h__
#define __a05d6852_4497_4f28_85e1_48a15a170738_WinCtrlHelper_h__

class ICurrencyRatesProvider;

inline CMStringW get_window_text(HWND hWnd)
{
	int cBytes = ::GetWindowTextLength(hWnd);

	std::vector<wchar_t> aBuf(cBytes + 1);
	LPTSTR pBuffer = &*(aBuf.begin());
	::GetWindowText(hWnd, pBuffer, cBytes + 1);

	return CMStringW(pBuffer);
}

inline void prepare_edit_ctrl_for_error(HWND hwndEdit)
{
	::SetFocus(hwndEdit);
	::SendMessage(hwndEdit, EM_SETSEL, 0, -1);
	::SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
}

void show_variable_list(HWND hwndParent, const ICurrencyRatesProvider *pProvider);

inline int CurrencyRates_MessageBox(HWND hWnd, LPCTSTR pszText, UINT nType = MB_OK)
{
	return ::MessageBox(hWnd, pszText, _A2T(MIRANDANAME), nType);
}

inline void spin_set_range(HWND hwndSpin, short nLower, short nUpper)
{
	::SendMessage(hwndSpin, UDM_SETRANGE, 0, MAKELPARAM(nUpper, nLower));
}


#endif //__a05d6852_4497_4f28_85e1_48a15a170738_WinCtrlHelper_h__
