#include "stdafx.h"
#include "optionsctrlimpl.h"

/*
 * OptionsCtrlImpl::Item
 */

OptionsCtrlImpl::Item::Item(OptionsCtrlImpl* pCtrl, ItemType ItemType, const TCHAR* szLabel, DWORD dwFlags, INT_PTR dwData) :
	m_pCtrl(pCtrl), m_hItem(NULL), m_nRef(1), m_ItemType(ItemType), m_strLabel(szLabel), m_dwData(dwData)
{
	m_bEnabled       = !(dwFlags & OCF_DISABLED);
	m_bDisableChilds = !(dwFlags & OCF_NODISABLECHILDS);
}

void OptionsCtrlImpl::Item::enableChilds(bool bEnable)
{
	HTREEITEM hChild = TreeView_GetChild(m_pCtrl->m_hTree, m_hItem);
	while (hChild)
	{
		Item* pItem = m_pCtrl->getItem(hChild);
		if (pItem)
			pItem->setEnabled(bEnable);

		hChild = TreeView_GetNextSibling(m_pCtrl->m_hTree, hChild);
	}
}

void OptionsCtrlImpl::Item::setLabel(const TCHAR* szLabel)
{
	m_strLabel = szLabel;

	m_pCtrl->setNodeText(m_hItem, m_strLabel.c_str());
}
