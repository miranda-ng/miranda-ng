#include "_globals.h"
#include "bandctrl.h"

/*
 * BandCtrl
 */

void BandCtrl::setLayout(int nLayout)
{
	SendMessage(m_hBandWnd, BCM_SETLAYOUT, nLayout, 0);
}

HANDLE BandCtrl::addButton(DWORD dwFlags, HICON hIcon, INT_PTR dwData, const TCHAR *szTooltip, const TCHAR *szText)
{
	BCBUTTON bcb;

	bcb.dwFlags = dwFlags | BCF_ICON | BCF_DATA | (szTooltip ? BCF_TOOLTIP : 0) | (szText ? BCF_TEXT : 0);
	bcb.hIcon = hIcon;
	bcb.dwData = dwData;
	bcb.m_szTooltip = szTooltip;
	bcb.m_szText = szText;

	return reinterpret_cast<HANDLE>(SendMessage(m_hBandWnd, BCM_ADDBUTTON, 0, reinterpret_cast<LPARAM>(&bcb)));
}

bool BandCtrl::isButtonChecked(HANDLE hButton)
{
	return bool_(SendMessage(m_hBandWnd, BCM_ISBUTTONCHECKED, reinterpret_cast<WPARAM>(hButton), 0));
}

void BandCtrl::checkButton(HANDLE hButton, bool bCheck)
{
	SendMessage(m_hBandWnd, BCM_CHECKBUTTON, reinterpret_cast<WPARAM>(hButton), BOOL_(bCheck));
}

DWORD BandCtrl::getButtonData(HANDLE hButton)
{
	return SendMessage(m_hBandWnd, BCM_GETBUTTONDATA, reinterpret_cast<WPARAM>(hButton), 0);
}

void BandCtrl::setButtonData(HANDLE hButton, INT_PTR dwData)
{
	SendMessage(m_hBandWnd, BCM_SETBUTTONDATA, reinterpret_cast<WPARAM>(hButton), dwData);
}

bool BandCtrl::isButtonVisisble(HANDLE hButton)
{
	return bool_(SendMessage(m_hBandWnd, BCM_ISBUTTONVISIBLE, reinterpret_cast<WPARAM>(hButton), 0));
}

void BandCtrl::showButton(HANDLE hButton, bool bShow)
{
	SendMessage(m_hBandWnd, BCM_SHOWBUTTON, reinterpret_cast<WPARAM>(hButton), BOOL_(bShow));
}

RECT BandCtrl::getButtonRect(HANDLE hButton)
{
	RECT rButton;

	SendMessage(m_hBandWnd, BCM_GETBUTTONRECT, reinterpret_cast<WPARAM>(hButton), reinterpret_cast<LPARAM>(&rButton));

	return rButton;
}

bool BandCtrl::isButtonEnabled(HANDLE hButton)
{
	return bool_(SendMessage(m_hBandWnd, BCM_ISBUTTONENABLED, reinterpret_cast<WPARAM>(hButton), 0));
}

void BandCtrl::enableButton(HANDLE hButton, bool bEnable)
{
	SendMessage(m_hBandWnd, BCM_ENABLEBUTTON, reinterpret_cast<WPARAM>(hButton), BOOL_(bEnable));
}
