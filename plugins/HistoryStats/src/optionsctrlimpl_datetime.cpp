#include "stdafx.h"
#include "optionsctrlimpl.h"

#include "main.h"

/*
 * OptionsCtrlImpl::DateTime
 */

ext::string OptionsCtrlImpl::DateTime::getDTFormatString(const ext::string& strFormat)
{
	ext::string strOut, strPart;

	for (int i = 0; i < strFormat.length(); ++i) {
		if (strFormat[i] == '%' && i < strFormat.length() - 1) {
			++i;

			bool bSharp = (strFormat[i] == '#');
			ext::string strCode;

			if (bSharp && i < strFormat.length() - 1)
				++i;

			switch (strFormat[i]) {
			case 'a':
				strCode = _T("ddd");
				break;

			case 'A':
				strCode = _T("dddd");
				break;

			case 'b':
				strCode = _T("MMM");
				break;

			case 'B':
				strCode = _T("MMMM");
				break;

			case 'd':
				strCode = bSharp ? _T("d") : _T("dd");
				break;

			case 'H':
				strCode = bSharp ? _T("H") : _T("HH");
				break;

			case 'I':
				strCode = bSharp ? _T("h") : _T("hh");
				break;

			case 'm':
				strCode = bSharp ? _T("M") : _T("MM");
				break;

			case 'M':
				strCode = bSharp ? _T("m") : _T("mm");
				break;

			case 'p':
				strCode = _T("tt"); // MEMO: seems not to work if current locale is 24-hour
				break;

			case 'y':
				strCode = _T("yy");
				break;

			case 'Y':
				strCode = _T("yyyy");
				break;

			case '%':
				strPart += _T("%");
				break;
			}

			if (!strCode.empty()) {
				if (!strPart.empty()) {
					strOut += _T("'");
					strOut += strPart;
					strOut += _T("'");
					strPart = _T("");
				}

				strOut += strCode;
			}
		}
		else {
			strPart += strFormat[i];

			if (strFormat[i] == '\'')
				strPart += _T("'");
		}
	}

	if (!strPart.empty()) {
		strOut += _T("'");
		strOut += strPart;
		strOut += _T("'");
	}

	return strOut;
}

SYSTEMTIME OptionsCtrlImpl::DateTime::toSystemTime(time_t timestamp)
{
	SYSTEMTIME st;
	FILETIME ft;
	LONGLONG ll = Int32x32To64(timestamp, 10000000) + 116444736000000000;

	ft.dwLowDateTime = static_cast<DWORD>(ll);
	ft.dwHighDateTime = static_cast<DWORD>(ll >> 32);

	FileTimeToSystemTime(&ft, &st);

	return st;
}

time_t OptionsCtrlImpl::DateTime::fromSystemTime(const SYSTEMTIME& st)
{
	FILETIME ft;
	LONGLONG ll;

	SystemTimeToFileTime(&st, &ft);

	ll = (static_cast<LONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

	return static_cast<time_t>((ll - 116444736000000000) / 10000000);
}

void OptionsCtrlImpl::DateTime::enableChildsDateTime()
{
	if (m_bDisableChilds || m_bDisableChildsOnNone) {
		enableChilds(getChildEnable());
	}
}

bool OptionsCtrlImpl::DateTime::getChildEnable()
{
	return
		!m_bDisableChildsOnNone && m_bDisableChilds && m_bEnabled ||
		m_bDisableChildsOnNone && !m_bNone && (!m_bDisableChilds || m_bEnabled);
}

time_t OptionsCtrlImpl::DateTime::getTimestampValue()
{
	SYSTEMTIME st;

	if (SendMessage(m_hDateTimeWnd, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st)) == GDT_VALID) {
		return fromSystemTime(st);
	}
	else {
		return 24 * 60 * 60;
	}
}

bool OptionsCtrlImpl::DateTime::getTimestampNone()
{
	if (!m_bAllowNone) {
		return false;
	}

	SYSTEMTIME st;

	return (SendMessage(m_hDateTimeWnd, DTM_GETSYSTEMTIME, 0, reinterpret_cast<LPARAM>(&st)) != GDT_VALID);
}

ext::string OptionsCtrlImpl::DateTime::getCombinedText()
{
	ext::string strTemp = m_strLabel;

	strTemp += _T(": ");

	if (m_bNone)
		strTemp += TranslateT("none");
	else
		strTemp += utils::timestampToString(m_timestamp, m_strFormat.c_str());

	return strTemp;
}

OptionsCtrlImpl::DateTime::DateTime(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, const TCHAR* szFormat, time_t timestamp, DWORD dwFlags, INT_PTR dwData)
: Item(pCtrl, itDateTime, szLabel, dwFlags, dwData), m_hDateTimeWnd(NULL), m_strFormat(szFormat), m_timestamp(timestamp)
{
	m_bDisableChildsOnNone = bool_(dwFlags & OCF_DISABLECHILDSONNONE);
	m_bAllowNone = bool_(dwFlags & OCF_ALLOWNONE);
	m_bNone = m_bAllowNone && bool_(dwFlags & OCF_NONE);
	m_strFormatDT = getDTFormatString(m_strFormat);

	m_pCtrl->insertItem(pParent, this, getCombinedText().c_str(), dwFlags, m_bEnabled ? siDateTime : siDateTimeG);

	if (pParent) {
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::DateTime::onSelect()
{
	if (!m_bEnabled || m_hDateTimeWnd) {
		return;
	}

	m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());

	HFONT hTreeFront = reinterpret_cast<HFONT>(SendMessage(m_pCtrl->m_hTree, WM_GETFONT, 0, 0));
	RECT r;

	if (m_pCtrl->getItemFreeRect(m_hItem, r)) {
		r.top -= 2;
		r.bottom += 2;

		if (r.left + 50 > r.right) {
			r.left = r.right - 50;
		}

		HWND hTempWnd;

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | (m_bAllowNone ? DTS_SHOWNONE : 0);

		if (hTempWnd = CreateWindowEx(
			WS_EX_CLIENTEDGE, DATETIMEPICK_CLASS, _T(""), dwStyle,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			m_pCtrl->m_hTree, reinterpret_cast<HMENU>(ccDateTime), g_hInst, NULL)) {
			// restrict to dates a timestamp can hold (with 1 day less to avoid timezone issues)
			SYSTEMTIME stMinMax[2] = { toSystemTime(0x00000000 + 24 * 60 * 60), toSystemTime(0x7FFFFFFF - 24 * 60 * 60) };

			SendMessage(hTempWnd, DTM_SETRANGE, GDTR_MIN | GDTR_MAX, reinterpret_cast<LPARAM>(stMinMax));

			// set format string
			SendMessage(hTempWnd, DTM_SETFORMAT, 0, reinterpret_cast<LPARAM>(m_strFormatDT.c_str()));

			// set timestamp
			if (m_bAllowNone && m_bNone) {
				SendMessage(hTempWnd, DTM_SETSYSTEMTIME, GDT_NONE, 0);
			}
			else {
				SYSTEMTIME st = toSystemTime(m_timestamp);

				SendMessage(hTempWnd, DTM_SETSYSTEMTIME, GDT_VALID, reinterpret_cast<LPARAM>(&st));
			}

			SendMessage(hTempWnd, WM_SETFONT, reinterpret_cast<WPARAM>(hTreeFront), MAKELPARAM(TRUE, 0));

			m_hDateTimeWnd = hTempWnd;
		}
	}
}

void OptionsCtrlImpl::DateTime::onDeselect()
{
	if (m_hDateTimeWnd) {
		RECT rToInvalidate;
		bool bValidRect = false;

		if (GetWindowRect(m_hDateTimeWnd, &rToInvalidate)) {
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 0);
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 1);

			bValidRect = true;
		}

		m_timestamp = getTimestampValue();
		m_bNone = getTimestampNone();

		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());

		DestroyWindow(m_hDateTimeWnd);
		m_hDateTimeWnd = NULL;

		InvalidateRect(m_pCtrl->m_hTree, bValidRect ? &rToInvalidate : NULL, TRUE);

		// enable childs?
		enableChildsDateTime();
	}
}

void OptionsCtrlImpl::DateTime::onActivate()
{
	if (!m_hDateTimeWnd) {
		onSelect();
	}

	if (m_hDateTimeWnd) {
		SetFocus(m_hDateTimeWnd);
	}
}

void OptionsCtrlImpl::DateTime::onDateTimeChange()
{
	if (m_hDateTimeWnd) {
		m_timestamp = getTimestampValue();
		m_bNone = getTimestampNone();

		// enable childs?
		enableChildsDateTime();
	}
}

void OptionsCtrlImpl::DateTime::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, bEnable ? siDateTime : siDateTimeG);

	enableChildsDateTime();
}

void OptionsCtrlImpl::DateTime::childAdded(Item* pChild)
{
	if (m_bDisableChilds || m_bDisableChildsOnNone) {
		pChild->setEnabled(getChildEnable());
	}
}

void OptionsCtrlImpl::DateTime::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;

	// only if not editing (otherwise update when user finishes editing)
	if (!m_hDateTimeWnd) {
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}
}

bool OptionsCtrlImpl::DateTime::isNone()
{
	if (m_hDateTimeWnd) {
		m_timestamp = getTimestampValue();
		m_bNone = getTimestampNone();
	}

	return m_bNone;
}

void OptionsCtrlImpl::DateTime::setNone()
{
	if (!m_bAllowNone) {
		return;
	}

	m_bNone = true;

	if (m_hDateTimeWnd) {
		SendMessage(m_hDateTimeWnd, DTM_SETSYSTEMTIME, GDT_NONE, 0);
	}
	else {
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}

	// enable childs?
	enableChildsDateTime();
}

time_t OptionsCtrlImpl::DateTime::getTimestamp()
{
	if (m_hDateTimeWnd) {
		m_timestamp = getTimestampValue();
		//m_bNone = getTimestampNone();/// @note : what was this supposed to be?
	}

	return m_timestamp;
}

void OptionsCtrlImpl::DateTime::setTimestamp(time_t timestamp)
{
	m_bNone = false;
	m_timestamp = timestamp;

	if (m_hDateTimeWnd) {
		SYSTEMTIME st = toSystemTime(timestamp);

		SendMessage(m_hDateTimeWnd, DTM_SETSYSTEMTIME, GDT_VALID, reinterpret_cast<LPARAM>(&st));
	}
	else {
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}

	// enable childs?
	enableChildsDateTime();
}

bool OptionsCtrlImpl::DateTime::isMonthCalVisible()
{
	return (m_hDateTimeWnd && SendMessage(m_hDateTimeWnd, DTM_GETMONTHCAL, 0, 0));
}
