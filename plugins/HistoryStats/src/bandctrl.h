#if !defined(HISTORYSTATS_GUARD_BANDCTRL_H)
#define HISTORYSTATS_GUARD_BANDCTRL_H

#include "_globals.h"
#include "bandctrldefs.h"

/*
 * BandCtrl
 */ 

class BandCtrl
	: public BandCtrlDefs
	, private pattern::NotCopyable<BandCtrl>
{
private:
	HWND m_hBandWnd;

public:
	explicit BandCtrl(HWND hBandWnd = NULL) : m_hBandWnd(hBandWnd) { }
	~BandCtrl() { }

public:
	const BandCtrl& operator <<(HWND hBandWnd) { m_hBandWnd = hBandWnd; return *this; }
	operator HWND() { return m_hBandWnd; }

public:
	void setLayout(int nLayout);
	HANDLE addButton(DWORD dwFlags, HICON hIcon, INT_PTR dwData, const TCHAR* szTooltip = NULL, const TCHAR* szText = NULL);
	bool isButtonChecked(HANDLE hButton);
    void checkButton(HANDLE hButton, bool bCheck);
	DWORD getButtonData(HANDLE hButton);
	void setButtonData(HANDLE hButton, INT_PTR dwData);
	bool isButtonVisisble(HANDLE hButton);
	void showButton(HANDLE hButton, bool bShow);
	RECT getButtonRect(HANDLE hButton);
	bool isButtonEnabled(HANDLE hButton);
	void enableButton(HANDLE hButton, bool bEnable);
};

#endif // HISTORYSTATS_GUARD_BANDCTRL_H
