#include "_globals.h"
#include "optionsctrlimpl.h"

/*
 * OptionsCtrlImpl::Check
 */

int OptionsCtrlImpl::Check::getStateImage()
{
	return m_bEnabled ? (m_bChecked ? siCheckC : siCheckU) : (m_bChecked ? siCheckCG : siCheckUG);
}

bool OptionsCtrlImpl::Check::getChildEnable()
{
	return
		!m_bDisableChildsOnUncheck && m_bDisableChilds && m_bEnabled ||
		m_bDisableChildsOnUncheck && m_bChecked && (!m_bDisableChilds || m_bEnabled);
}

void OptionsCtrlImpl::Check::updateItem()
{
	m_pCtrl->setStateImage(m_hItem, m_bEnabled ? (m_bChecked ? siCheckC : siCheckU) : (m_bChecked ? siCheckCG : siCheckUG));

	if (m_bDisableChilds || m_bDisableChildsOnUncheck)
		enableChilds(getChildEnable());
}

OptionsCtrlImpl::Check::Check(OptionsCtrlImpl* pCtrl, Item* pParent, const TCHAR* szLabel, DWORD dwFlags, INT_PTR dwData) :
	Item(pCtrl, itCheck, szLabel, dwFlags, dwData)
{
	m_bChecked = bool_(dwFlags & OCF_CHECKED);
	m_bDisableChildsOnUncheck = bool_(dwFlags & OCF_DISABLECHILDSONUNCHECK);

	pCtrl->insertItem(pParent, this, szLabel, dwFlags, getStateImage());

	if (pParent)
		pParent->childAdded(this);
}

void OptionsCtrlImpl::Check::onToggle()
{
	if (m_bEnabled) {
		setChecked(!m_bChecked);
		m_pCtrl->setModified(this);
	}
}

void OptionsCtrlImpl::Check::childAdded(Item* pChild)
{
	if (m_bDisableChilds || m_bDisableChildsOnUncheck)
		pChild->setEnabled(getChildEnable());
}
