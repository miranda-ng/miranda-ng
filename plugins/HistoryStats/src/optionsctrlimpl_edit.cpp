#include "stdafx.h"
#include "optionsctrlimpl.h"

#include "main.h"

/*
 * OptionsCtrlImpl::Edit
 */

ext::string OptionsCtrlImpl::Edit::getEditText()
{
	int nLen = GetWindowTextLength(m_hEditWnd);
	TCHAR* szBuf = new TCHAR[nLen + 1];

	GetWindowText(m_hEditWnd, szBuf, nLen + 1);

	ext::string strTemp = szBuf;

	delete[] szBuf;

	return strTemp;
}

ext::string OptionsCtrlImpl::Edit::getCombinedText()
{
	if (m_strEdit.empty())
	{
		return m_strLabel;
	}
	else
	{
		ext::string strTemp = m_strLabel;

		strTemp += _T(": ");
		strTemp += m_strEdit;

		return strTemp;
	}
}

OptionsCtrlImpl::Edit::Edit(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, const TCHAR* szEdit, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itEdit, szLabel, dwFlags, dwData), m_hEditWnd(NULL)
{
	m_strEdit = szEdit;
	m_bNumber = bool_(dwFlags & OCF_NUMBER);
	
	m_pCtrl->insertItem(pParent, this, getCombinedText().c_str(), dwFlags, m_bEnabled ? siEdit : siEditG);

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::Edit::onSelect()
{
	if (!m_bEnabled || m_hEditWnd)
	{
		return;
	}

	m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());

	HFONT hTreeFront = reinterpret_cast<HFONT>(SendMessage(m_pCtrl->m_hTree, WM_GETFONT, 0, 0));
	RECT r;

	if (m_pCtrl->getItemFreeRect(m_hItem, r))
	{
		r.top -= 2;
		r.bottom += 2;

		if (r.left + 50 > r.right)
		{
			r.left = r.right - 50;
		}

		HWND hTempWnd;
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;

		if (m_bNumber)
		{
			dwStyle |= ES_NUMBER | ES_RIGHT;
		}

		if (hTempWnd = CreateWindowEx(
			WS_EX_CLIENTEDGE, WC_EDIT, m_strEdit.c_str(), dwStyle,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			m_pCtrl->m_hTree, reinterpret_cast<HMENU>(ccEdit), g_hInst, NULL))
		{
			SendMessage(hTempWnd, WM_SETFONT, reinterpret_cast<WPARAM>(hTreeFront), MAKELPARAM(TRUE, 0));

			m_hEditWnd = hTempWnd;
		}
	}
}

void OptionsCtrlImpl::Edit::onDeselect()
{
	if (m_hEditWnd)
	{
		RECT rToInvalidate;
		bool bValidRect = false;

		if (GetWindowRect(m_hEditWnd, &rToInvalidate))
		{
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 0);
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 1);

			bValidRect = true;
		}

		m_strEdit = getEditText();

		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());

		DestroyWindow(m_hEditWnd);
		m_hEditWnd = NULL;

		InvalidateRect(m_pCtrl->m_hTree, bValidRect ? &rToInvalidate : NULL, TRUE);
	}
}

void OptionsCtrlImpl::Edit::onActivate()
{
	if (!m_hEditWnd)
	{
		onSelect();
	}

	if (m_hEditWnd)
	{
		SetFocus(m_hEditWnd);
		SendMessage(m_hEditWnd, EM_SETSEL, 0, -1);
	}
}

void OptionsCtrlImpl::Edit::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, bEnable ? siEdit : siEditG);

	if (m_bDisableChilds)
	{
		enableChilds(m_bEnabled);
	}
}

void OptionsCtrlImpl::Edit::childAdded(Item* pChild)
{
	if (m_bDisableChilds)
	{
		pChild->setEnabled(m_bEnabled);
	}
}

const TCHAR* OptionsCtrlImpl::Edit::getString()
{
	if (m_hEditWnd)
	{
		m_strEdit = getEditText();
	}

	return m_strEdit.c_str();
}

void OptionsCtrlImpl::Edit::setString(const TCHAR* szString)
{
	m_strEdit = szString;

	if (m_hEditWnd)
	{
		SetWindowText(m_hEditWnd, m_strEdit.c_str());
	}
	else
	{
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}
}

void OptionsCtrlImpl::Edit::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;

	// only if not editing (otherwise update when user finishes editing)
	if (!m_hEditWnd)
	{
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}
}
