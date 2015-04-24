#include "stdafx.h"
#include "optionsctrlimpl.h"

/*
 * OptionsCtrlImpl::RadioSiblings
 */

int OptionsCtrlImpl::RadioSiblings::join(Radio* pRadio)
{
	m_Radios.push_back(pRadio);
	++m_nRadios;

	return m_nRadios - 1;
}

bool OptionsCtrlImpl::RadioSiblings::leave(int nRadio)
{
	assert(nRadio >= 0 && nRadio < m_Radios.size() && m_Radios[nRadio]);

	m_Radios[nRadio] = NULL;
	--m_nRadios;

	return (m_nRadios == 0);
}

void OptionsCtrlImpl::RadioSiblings::setChecked(int nRadio)
{
	if (nRadio == m_nChecked)
	{
		return;
	}

	if (nRadio < 0 || nRadio >= m_Radios.size())
	{
		return;
	}

	if (m_nChecked != -1)
	{
		assert(m_Radios[m_nChecked]);

		int nOldChecked = m_nChecked;

		m_nChecked = -1;
		m_Radios[nOldChecked]->updateItem();
	}

	assert(m_Radios[nRadio]);

	m_nChecked = nRadio;
	m_Radios[m_nChecked]->updateItem();
}

/*
 * OptionsCtrlImpl::Radio
 */

bool OptionsCtrlImpl::Radio::getChildEnable(bool bChecked)
{
	return
		!m_bDisableChildsOnUncheck && m_bDisableChilds && m_bEnabled ||
		m_bDisableChildsOnUncheck && bChecked && (!m_bDisableChilds || m_bEnabled);
}

void OptionsCtrlImpl::Radio::updateItem()
{
	bool bChecked = isChecked();

	m_pCtrl->setStateImage(m_hItem, m_bEnabled ? (bChecked ? siRadioC : siRadioU) : (bChecked ? siRadioCG : siRadioUG));

	if (m_bDisableChilds || m_bDisableChildsOnUncheck)
	{
		enableChilds(getChildEnable(bChecked));
	}
}

OptionsCtrlImpl::Radio::Radio(OptionsCtrlImpl* pCtrl, Item* pParent, Radio* pSibling, const TCHAR* szLabel, DWORD dwFlags, INT_PTR dwData)
	: Item(pCtrl, itRadio, szLabel, dwFlags, dwData)
{
	m_bDisableChildsOnUncheck = bool_(dwFlags & OCF_DISABLECHILDSONUNCHECK);

	pCtrl->insertItem(pParent, this, szLabel, dwFlags, siRadioU);

	m_pSiblings = pSibling ? pSibling->m_pSiblings : new RadioSiblings();
	m_nIndex = m_pSiblings->join(this);

	if (dwFlags & OCF_CHECKED)
	{
		m_pSiblings->setChecked(m_nIndex);
	}
	else
	{
		updateItem();
	}

	if (pParent)
	{
		pParent->childAdded(this);
	}
}

OptionsCtrlImpl::Radio::~Radio()
{
	if (m_pSiblings->leave(m_nIndex))
	{
		delete m_pSiblings;
	}
}

void OptionsCtrlImpl::Radio::onToggle()
{
	if (m_bEnabled && !isChecked())
	{
		setChecked();
		m_pCtrl->setModified(this);
	}
}

void OptionsCtrlImpl::Radio::childAdded(Item* pChild)
{
	if (m_bDisableChilds || m_bDisableChildsOnUncheck)
	{
		pChild->setEnabled(getChildEnable(isChecked()));
	}
}
