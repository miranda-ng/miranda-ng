#include "_globals.h"
#include "optionsctrlimpl.h"

/*
 * OptionsCtrlImpl::Group
 */

OptionsCtrlImpl::Group::Group(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itGroup, szLabel, dwFlags, dwData)
{
	m_bDrawLine = bool_(dwFlags & OCF_DRAWLINE);

	pCtrl->insertItem(pParent, this, szLabel, dwFlags, m_bEnabled ? siFolder : siFolderG);

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

void OptionsCtrlImpl::Group::setEnabled(bool bEnable)
{
	m_bEnabled = bEnable;

	m_pCtrl->setStateImage(m_hItem, m_bEnabled ? siFolder : siFolderG);

	if (m_bDisableChilds)
	{
		enableChilds(m_bEnabled);
	}
}

void OptionsCtrlImpl::Group::childAdded(Item* pChild)
{
	if (m_bDisableChilds)
	{
		pChild->setEnabled(m_bEnabled);
	}
}
