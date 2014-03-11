#include "_globals.h"
#include "optionsctrlimpl.h"

#include "main.h"

/*
 * OptionsCtrlImpl::Color
 */

COLORREF OptionsCtrlImpl::Color::getColorValue()
{
	return SendMessage(m_hColorWnd, CPM_GETCOLOUR, 0, 0);
}

OptionsCtrlImpl::Color::Color(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, COLORREF crColor, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itColor, szLabel, dwFlags, dwData)
	, m_hColorWnd(NULL)
	, m_crColor(crColor)
{	
	m_pCtrl->insertItem(pParent, this, m_strLabel.c_str(), dwFlags, m_bEnabled ? siColor : siColorG);

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::Color::onSelect()
{
	if (!m_bEnabled || m_hColorWnd)
	{
		return;
	}

	HFONT hTreeFront = reinterpret_cast<HFONT>(SendMessage(m_pCtrl->m_hTree, WM_GETFONT, 0, 0));
	RECT r;

	if (m_pCtrl->getItemFreeRect(m_hItem, r))
	{
		r.top -= 1;
		r.bottom += 1;

		if (r.left + 40 > r.right)
		{
			r.left = r.right - 40;
		}

		if (r.right - 40 > r.left)
		{
			r.right = r.left + 40;
		}

		HWND hTempWnd;

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

		if (hTempWnd = CreateWindowEx(
			0, _T(WNDCLASS_COLOURPICKER), NULL, dwStyle,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			m_pCtrl->m_hTree, reinterpret_cast<HMENU>(ccColor), g_hInst, NULL))
		{
			SendMessage(hTempWnd, CPM_SETCOLOUR, 0, m_crColor);

			m_hColorWnd = hTempWnd;
		}
	}
}

void OptionsCtrlImpl::Color::onDeselect()
{
	if (m_hColorWnd)
	{
		RECT rToInvalidate;
		bool bValidRect = false;

		if (GetWindowRect(m_hColorWnd, &rToInvalidate))
		{
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 0);
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 1);

			bValidRect = true;
		}

		m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());

		DestroyWindow(m_hColorWnd);
		m_hColorWnd = NULL;

		InvalidateRect(m_pCtrl->m_hTree, bValidRect ? &rToInvalidate : NULL, TRUE);
	}
}

void OptionsCtrlImpl::Color::onActivate()
{
	if (!m_hColorWnd)
	{
		onSelect();
	}

	if (m_hColorWnd)
	{
		SetFocus(m_hColorWnd);
	}
}

void OptionsCtrlImpl::Color::onColorChange()
{
	if (m_hColorWnd)
	{
		m_crColor = getColorValue();
	}
}

void OptionsCtrlImpl::Color::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, bEnable ? siColor : siColorG);

	if (m_bDisableChilds)
	{
		enableChilds(m_bEnabled);
	}
}

void OptionsCtrlImpl::Color::childAdded(Item* pChild)
{
	if (m_bDisableChilds)
	{
		pChild->setEnabled(m_bEnabled);
	}
}

void OptionsCtrlImpl::Color::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;
	
	// only if not showing button (otherwise update when button disappears)
	if (!m_hColorWnd)
	{
		m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());
	}
}

COLORREF OptionsCtrlImpl::Color::getColor()
{
	if (m_hColorWnd)
	{
		m_crColor = getColorValue();
	}

	return m_crColor;
}

void OptionsCtrlImpl::Color::setColor(COLORREF crColor)
{
	m_crColor = crColor;

	if (m_hColorWnd)
	{
		SendMessage(m_hColorWnd, CPM_SETCOLOUR, 0, crColor);
	}
}
