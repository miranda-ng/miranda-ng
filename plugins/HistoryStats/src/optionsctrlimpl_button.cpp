#include "_globals.h"
#include "optionsctrlimpl.h"

#include "main.h"

/*
 * OptionsCtrlImpl::Button
 */

OptionsCtrlImpl::Button::Button(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, const TCHAR* szButton, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itButton, szLabel, dwFlags, dwData)
	, m_hButtonWnd(NULL)
	, m_strButton(szButton)
{
	m_pCtrl->insertItem(pParent, this, m_strLabel.c_str(), dwFlags, m_bEnabled ? siButton : siButtonG);

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::Button::onSelect()
{
	if (!m_bEnabled || m_hButtonWnd)
	{
		return;
	}

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

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_TEXT | BS_CENTER | BS_VCENTER;

		if (hTempWnd = CreateWindowEx(
			0, WC_BUTTON, m_strButton.c_str(), dwStyle,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			m_pCtrl->m_hTree, reinterpret_cast<HMENU>(ccButton), g_hInst, NULL))
		{
			SendMessage(hTempWnd, WM_SETFONT, reinterpret_cast<WPARAM>(hTreeFront), MAKELPARAM(TRUE, 0));

			m_hButtonWnd = hTempWnd;
		}
	}
}

void OptionsCtrlImpl::Button::onDeselect()
{
	if (m_hButtonWnd)
	{
		RECT rToInvalidate;
		bool bValidRect = false;

		if (GetWindowRect(m_hButtonWnd, &rToInvalidate))
		{
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 0);
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 1);

			bValidRect = true;
		}

		m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());

		DestroyWindow(m_hButtonWnd);
		m_hButtonWnd = NULL;

		InvalidateRect(m_pCtrl->m_hTree, bValidRect ? &rToInvalidate : NULL, TRUE);
	}
}

void OptionsCtrlImpl::Button::onActivate()
{
	if (!m_hButtonWnd)
	{
		onSelect();
	}

	if (m_hButtonWnd)
	{
		SetFocus(m_hButtonWnd);
	}
}

void OptionsCtrlImpl::Button::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, bEnable ? siButton : siButtonG);

	if (m_bDisableChilds)
	{
		enableChilds(m_bEnabled);
	}
}

void OptionsCtrlImpl::Button::childAdded(Item* pChild)
{
	if (m_bDisableChilds)
	{
		pChild->setEnabled(m_bEnabled);
	}
}

void OptionsCtrlImpl::Button::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;
	
	// only if not showing button (otherwise update when button disappears)
	if (!m_hButtonWnd)
	{
		m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());
	}
}
