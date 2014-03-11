#include "_globals.h"
#include "optionsctrl.h"

#include "utils.h"

/*
 * OptionsCtrl
 */

HANDLE OptionsCtrl::insertGroup(HANDLE hParent, const TCHAR* szLabel, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCGROUP ocg;
	ocg.dwFlags = dwFlags;
	ocg.szLabel = const_cast<TCHAR*>(szLabel);
	ocg.dwData = dwData;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTGROUP, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&ocg)));
}

HANDLE OptionsCtrl::insertCheck(HANDLE hParent, const TCHAR* szLabel, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCCHECK occ;
	occ.dwFlags = dwFlags;
	occ.szLabel = const_cast<TCHAR*>(szLabel);
	occ.dwData = dwData;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTCHECK, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&occ)));
}

HANDLE OptionsCtrl::insertRadio(HANDLE hParent, HANDLE hSibling, const TCHAR* szLabel, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCRADIO ocr;
	ocr.dwFlags = dwFlags;
	ocr.szLabel = const_cast<TCHAR*>(szLabel);
	ocr.dwData = dwData;
	ocr.hSibling = hSibling;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTRADIO, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&ocr)));
}

HANDLE OptionsCtrl::insertEdit(HANDLE hParent, const TCHAR* szLabel, const TCHAR* szEdit /* = _T("") */, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCEDIT oce;
	oce.dwFlags = dwFlags;
	oce.szLabel = const_cast<TCHAR*>(szLabel);
	oce.dwData = dwData;
	oce.szEdit = const_cast<TCHAR*>(szEdit);

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTEDIT, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&oce)));
}

HANDLE OptionsCtrl::insertCombo(HANDLE hParent, const TCHAR* szLabel, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCCOMBO occ;
	occ.dwFlags = dwFlags;
	occ.szLabel = const_cast<TCHAR*>(szLabel);
	occ.dwData = dwData;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTCOMBO, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&occ)));
}

HANDLE OptionsCtrl::insertButton(HANDLE hParent, const TCHAR* szLabel, const TCHAR* szButton, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCBUTTON ocb;
	ocb.dwFlags = dwFlags;
	ocb.szLabel = const_cast<TCHAR*>(szLabel);
	ocb.dwData = dwData;
	ocb.szButton = const_cast<TCHAR*>(szButton);

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTBUTTON, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&ocb)));
}

HANDLE OptionsCtrl::insertDateTime(HANDLE hParent, const TCHAR* szLabel, DWORD dwDateTime, const TCHAR* szFormat /* = _T("%Y-%m-%d") */, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCDATETIME ocdt;
	ocdt.dwFlags = dwFlags;
	ocdt.szLabel = const_cast<TCHAR*>(szLabel);
	ocdt.dwData = dwData;
	ocdt.szFormat = const_cast<TCHAR*>(szFormat);
	ocdt.dwDateTime = dwDateTime;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTDATETIME, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&ocdt)));
}

HANDLE OptionsCtrl::insertColor(HANDLE hParent, const TCHAR* szLabel, COLORREF crColor /* = 0 */, DWORD dwFlags /* = 0 */, INT_PTR dwData /* = 0 */)
{
	OCCOLOR occ;
	occ.dwFlags = dwFlags;
	occ.szLabel = const_cast<TCHAR*>(szLabel);
	occ.dwData = dwData;
	occ.crColor = crColor;

	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_INSERTCOLOR, reinterpret_cast<WPARAM>(hParent), reinterpret_cast<LPARAM>(&occ)));
}

const TCHAR* OptionsCtrl::getItemLabel(HANDLE hItem)
{
	return reinterpret_cast<const TCHAR*>(SendMessage(m_hOptWnd, OCM_GETITEMLABEL, reinterpret_cast<WPARAM>(hItem), 0));
}

void OptionsCtrl::setItemLabel(HANDLE hItem, const TCHAR* szLabel)
{
	SendMessage(m_hOptWnd, OCM_SETITEMLABEL, reinterpret_cast<WPARAM>(hItem), reinterpret_cast<LPARAM>(szLabel));
}

bool OptionsCtrl::isItemEnabled(HANDLE hItem)
{
	return bool_(SendMessage(m_hOptWnd, OCM_ISITEMENABLED, reinterpret_cast<WPARAM>(hItem), 0));
}

void OptionsCtrl::enableItem(HANDLE hItem, bool bEnable)
{
	SendMessage(m_hOptWnd, OCM_ENABLEITEM, reinterpret_cast<WPARAM>(hItem), BOOL_(bEnable));
}

DWORD OptionsCtrl::getItemData(HANDLE hItem)
{
	return SendMessage(m_hOptWnd, OCM_GETITEMDATA, reinterpret_cast<WPARAM>(hItem), 0);
}

void OptionsCtrl::setItemData(HANDLE hItem, INT_PTR dwData)
{
	SendMessage(m_hOptWnd, OCM_SETITEMDATA, reinterpret_cast<WPARAM>(hItem), dwData);
}

bool OptionsCtrl::isItemChecked(HANDLE hItem)
{
	return bool_(SendMessage(m_hOptWnd, OCM_ISITEMCHECKED, reinterpret_cast<WPARAM>(hItem), 0));
}

void OptionsCtrl::checkItem(HANDLE hItem, bool bCheck)
{
	SendMessage(m_hOptWnd, OCM_CHECKITEM, reinterpret_cast<WPARAM>(hItem), BOOL_(bCheck));
}

int OptionsCtrl::getRadioChecked(HANDLE hRadio)
{
	return SendMessage(m_hOptWnd, OCM_GETRADIOCHECKED, reinterpret_cast<WPARAM>(hRadio), 0);
}

void OptionsCtrl::setRadioChecked(HANDLE hRadio, int nCheck)
{
	SendMessage(m_hOptWnd, OCM_SETRADIOCHECKED, reinterpret_cast<WPARAM>(hRadio), nCheck);
}

int OptionsCtrl::getEditNumber(HANDLE hEdit)
{
	return SendMessage(m_hOptWnd, OCM_GETEDITNUMBER, reinterpret_cast<WPARAM>(hEdit), 0);
}

void OptionsCtrl::setEditNumber(HANDLE hEdit, int nNumber)
{
	SendMessage(m_hOptWnd, OCM_SETEDITNUMBER, reinterpret_cast<WPARAM>(hEdit), nNumber);
}

const TCHAR* OptionsCtrl::getEditString(HANDLE hEdit)
{
	return reinterpret_cast<const TCHAR*>(SendMessage(m_hOptWnd, OCM_GETEDITSTRING, reinterpret_cast<WPARAM>(hEdit), 0));
}

void OptionsCtrl::setEditString(HANDLE hEdit, const TCHAR* szString)
{
	SendMessage(m_hOptWnd, OCM_SETEDITSTRING, reinterpret_cast<WPARAM>(hEdit), reinterpret_cast<LPARAM>(szString));
}

void OptionsCtrl::addComboItem(HANDLE hCombo, const TCHAR* szItem)
{
	SendMessage(m_hOptWnd, OCM_ADDCOMBOITEM, reinterpret_cast<WPARAM>(hCombo), reinterpret_cast<LPARAM>(szItem));
}

int OptionsCtrl::getComboSelected(HANDLE hCombo)
{
	return SendMessage(m_hOptWnd, OCM_GETCOMBOSELECTED, reinterpret_cast<WPARAM>(hCombo), 0);
}

void OptionsCtrl::setComboSelected(HANDLE hCombo, int nSelect)
{
	SendMessage(m_hOptWnd, OCM_SETCOMBOSELECTED, reinterpret_cast<WPARAM>(hCombo), nSelect);
}

void OptionsCtrl::ensureVisible(HANDLE hItem)
{
	SendMessage(m_hOptWnd, OCM_ENSUREVISIBLE, reinterpret_cast<WPARAM>(hItem), 0);
}

void OptionsCtrl::deleteAllItems()
{
	SendMessage(m_hOptWnd, OCM_DELETEALLITEMS, 0, 0);
}

HANDLE OptionsCtrl::getSelection()
{
	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_GETSELECTION, 0, 0));
}

void OptionsCtrl::selectItem(HANDLE hItem)
{
	SendMessage(m_hOptWnd, OCM_SELECTITEM, reinterpret_cast<WPARAM>(hItem), 0);
}

HANDLE OptionsCtrl::getFirstItem()
{
	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_GETITEM, 0, OCGI_FIRST));
}

HANDLE OptionsCtrl::getNextItem(HANDLE hItem)
{
	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_GETITEM, reinterpret_cast<WPARAM>(hItem), OCGI_NEXT));
}

HANDLE OptionsCtrl::getPrevItem(HANDLE hItem)
{
	return reinterpret_cast<HANDLE>(SendMessage(m_hOptWnd, OCM_GETITEM, reinterpret_cast<WPARAM>(hItem), OCGI_PREV));
}

void OptionsCtrl::setRedraw(bool bRedraw)
{
	SendMessage(m_hOptWnd, WM_SETREDRAW, BOOL_(bRedraw), 0);
}

void OptionsCtrl::deleteItem(HANDLE hItem)
{
	SendMessage(m_hOptWnd, OCM_DELETEITEM, reinterpret_cast<WPARAM>(hItem), 0);
}

void OptionsCtrl::moveItem(HANDLE& hItem, HANDLE hInsertAfter)
{
	SendMessage(m_hOptWnd, OCM_MOVEITEM, reinterpret_cast<WPARAM>(&hItem), reinterpret_cast<LPARAM>(hInsertAfter));
}

int OptionsCtrl::getScrollPos(int nBar)
{
	return SendMessage(m_hOptWnd, OCM_GETSCROLLPOS, nBar, 0);
}

void OptionsCtrl::setScrollPos(int nBar, int nPos)
{
	SendMessage(m_hOptWnd, OCM_SETSCROLLPOS, nBar, nPos);
}

bool OptionsCtrl::isDateTimeNone(HANDLE hDateTime)
{
	return bool_(SendMessage(m_hOptWnd, OCM_ISDATETIMENONE, reinterpret_cast<WPARAM>(hDateTime), 0));
}

void OptionsCtrl::setDateTimeNone(HANDLE hDateTime)
{
	SendMessage(m_hOptWnd, OCM_SETDATETIMENONE, reinterpret_cast<LPARAM>(hDateTime), 0);
}

DWORD OptionsCtrl::getDateTime(HANDLE hDateTime, bool* pbNone /* = NULL */)
{
	BOOL bMyNone = FALSE;
	DWORD dwResult = SendMessage(m_hOptWnd, OCM_GETDATETIME, reinterpret_cast<WPARAM>(hDateTime), reinterpret_cast<LPARAM>(&bMyNone));

	if (pbNone)
		*pbNone = bool_(bMyNone);

	return dwResult;
}

void OptionsCtrl::setDateTime(HANDLE hDateTime, DWORD dwDateTime)
{
	SendMessage(m_hOptWnd, OCM_SETDATETIME, reinterpret_cast<WPARAM>(hDateTime), dwDateTime);
}


ext::string OptionsCtrl::getDateTimeStr(HANDLE hDateTime)
{
	bool bNone = false;
	DWORD dwTimestamp = getDateTime(hDateTime, &bNone);

	return bNone ? _T("") : utils::formatDate(dwTimestamp);
}

void OptionsCtrl::setDateTimeStr(HANDLE hDateTime, const ext::string& strDateTime)
{
	if (strDateTime.empty())
		setDateTimeNone(hDateTime);
	else
		setDateTime(hDateTime, utils::parseDate(strDateTime));
}

COLORREF OptionsCtrl::getColor(HANDLE hColor)
{
	return SendMessage(m_hOptWnd, OCM_GETITEMCOLOR, reinterpret_cast<WPARAM>(hColor), 0);
}

void OptionsCtrl::setColor(HANDLE hColor, COLORREF crColor)
{
	SendMessage(m_hOptWnd, OCM_SETITEMCOLOR, reinterpret_cast<WPARAM>(hColor), crColor);
}
