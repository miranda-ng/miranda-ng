#include "_globals.h"
#include "optionsctrlimpl.h"

#include "main.h"

/*
 * OptionsCtrlImpl::Combo
 */

void OptionsCtrlImpl::Combo::enableChildsCombo()
{
	if (m_bDisableChilds || m_bDisableChildsOnIndex0)
	{
		enableChilds(getChildEnable());
	}
}

bool OptionsCtrlImpl::Combo::getChildEnable()
{
	return
		!m_bDisableChildsOnIndex0 && m_bDisableChilds && m_bEnabled ||
		m_bDisableChildsOnIndex0 && (m_nSelected != 0) && (!m_bDisableChilds || m_bEnabled);
}

int OptionsCtrlImpl::Combo::getComboSel()
{
	int nSel = SendMessage(m_hComboWnd, CB_GETCURSEL, 0, 0);

	return (nSel == CB_ERR) ? -1 : nSel;
}

ext::string OptionsCtrlImpl::Combo::getCombinedText()
{
	if (m_nSelected == -1)
	{
		return m_strLabel;
	}
	else
	{
		ext::string strTemp = m_strLabel;

		strTemp += _T(": ");
		strTemp += m_Items[m_nSelected];

		return strTemp;
	}
}

OptionsCtrlImpl::Combo::Combo(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itCombo, szLabel, dwFlags, dwData), m_hComboWnd(NULL), m_nSelected(-1)
{
	m_bDisableChildsOnIndex0 = bool_(dwFlags & OCF_DISABLECHILDSONINDEX0);

	m_pCtrl->insertItem(pParent, this, getCombinedText().c_str(), dwFlags, m_bEnabled ? siCombo : siComboG);

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::Combo::onSelect()
{
	if (!m_bEnabled || m_hComboWnd)
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

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST;

		if (hTempWnd = CreateWindowEx(
			WS_EX_CLIENTEDGE, WC_COMBOBOX, _T(""), dwStyle,
			r.left, r.top, r.right - r.left, (r.bottom - r.top) * 20,
			m_pCtrl->m_hTree, reinterpret_cast<HMENU>(ccCombo), g_hInst, NULL))
		{
			vector_each_(i, m_Items)
			{
				SendMessage(hTempWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(m_Items[i].c_str()));
			}

			if (m_nSelected != -1)
			{
				SendMessage(hTempWnd, CB_SETCURSEL, m_nSelected, 0);
			}

			SendMessage(hTempWnd, WM_SETFONT, reinterpret_cast<WPARAM>(hTreeFront), MAKELPARAM(TRUE, 0));

			m_hComboWnd = hTempWnd;
		}
	}
}

void OptionsCtrlImpl::Combo::onDeselect()
{
	if (m_hComboWnd)
	{
		RECT rToInvalidate;
		bool bValidRect = false;

		if (GetWindowRect(m_hComboWnd, &rToInvalidate))
		{
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 0);
			ScreenToClient(m_pCtrl->m_hTree, reinterpret_cast<POINT*>(&rToInvalidate) + 1);

			bValidRect = true;
		}

		m_nSelected = getComboSel();

		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());

		DestroyWindow(m_hComboWnd);
		m_hComboWnd = NULL;

		InvalidateRect(m_pCtrl->m_hTree, bValidRect ? &rToInvalidate : NULL, TRUE);

		// enable childs?
		enableChildsCombo();
	}
}

void OptionsCtrlImpl::Combo::onActivate()
{
	if (!m_hComboWnd)
	{
		onSelect();
	}

	if (m_hComboWnd)
	{
		SetFocus(m_hComboWnd);
	}
}

void OptionsCtrlImpl::Combo::onSelChanged()
{
	if (m_hComboWnd)
	{
		m_nSelected = getComboSel();

		// enable childs?
		enableChildsCombo();
	}
}

void OptionsCtrlImpl::Combo::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, bEnable ? siCombo : siComboG);

	enableChildsCombo();
}

void OptionsCtrlImpl::Combo::childAdded(Item* pChild)
{
	if (m_bDisableChilds || m_bDisableChildsOnIndex0)
	{
		pChild->setEnabled(getChildEnable());
	}
}

void OptionsCtrlImpl::Combo::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;

	// only if not editing (otherwise update when user finishes editing)
	if (!m_hComboWnd)
	{
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}
}

void OptionsCtrlImpl::Combo::addItem(const TCHAR* szItem)
{
	m_Items.push_back(szItem);
}

int OptionsCtrlImpl::Combo::getSelected()
{
	if (m_hComboWnd)
	{
		m_nSelected = getComboSel();
	}

	return m_nSelected;
}

void OptionsCtrlImpl::Combo::setSelected(int nSelect)
{
	if (nSelect < 0 || nSelect >= m_Items.size())
	{
		return;
	}

	m_nSelected = nSelect;

	if (m_hComboWnd)
	{
		SendMessage(m_hComboWnd, CB_SETCURSEL, m_nSelected, 0);
	}
	else
	{
		m_pCtrl->setNodeText(m_hItem, getCombinedText().c_str());
	}

	// enable childs?
	enableChildsCombo();
}
