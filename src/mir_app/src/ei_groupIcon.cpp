/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-22 Miranda NG team

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#include "extraicons.h"

ExtraIconGroup::ExtraIconGroup(const char *_name) :
	ExtraIcon(_name),
	m_items(1)
{
	db_set_resident(EI_MODULE_NAME, _name);
}

ExtraIconGroup::~ExtraIconGroup()
{
}

void ExtraIconGroup::addExtraIcon(BaseExtraIcon *extra)
{
	m_items.insert(extra);

	CMStringW description;
	for (auto &p : m_items) {
		if (!description.IsEmpty())
			description.Append(L" / ");
		description += p->getDescription();
	}

	m_tszDescription = mir_wstrdup(description);
}

void ExtraIconGroup::rebuildIcons()
{
	for (auto &p : m_items)
		p->rebuildIcons();
}

void ExtraIconGroup::applyIcon(MCONTACT hContact)
{
	if (!isEnabled() || hContact == 0)
		return;

	m_setValidExtraIcon = false;
	m_insideApply = true;

	for (auto &p : m_items) {
		p->applyIcon(hContact);
		if (m_setValidExtraIcon) {
			m_pCurrentItem = p;
			break;
		}
	}

	m_insideApply = false;
}

int ExtraIconGroup::getPosition() const
{
	int pos = INT_MAX;
	for (auto &p : m_items)
		pos = min(pos, p->getPosition());
	return pos;
}

void ExtraIconGroup::setSlot(int slot)
{
	ExtraIcon::setSlot(slot);

	for (auto &p : m_items)
		p->setSlot(slot);
}

void ExtraIconGroup::onClick(MCONTACT hContact)
{
	if (m_pCurrentItem != nullptr)
		m_pCurrentItem->onClick(hContact);
}

int ExtraIconGroup::setIcon(MCONTACT, HANDLE)
{
	return -1;
	// return internalSetIcon(hContact, (void*)value, false);
}

int ExtraIconGroup::setIconByName(MCONTACT, const char*)
{
	return -1;
	// return internalSetIcon(hContact, (void*)value, true);
}

int ExtraIconGroup::internalSetIcon(ExtraIcon *pChild, MCONTACT hContact, HANDLE value, bool bByName)
{
	if (m_insideApply) {
		for (auto &p : m_items)
			if (p == pChild) {
				if (bByName)
					return p->setIconByName(hContact, (const char*)value);
				return p->setIcon(hContact, value);
			}

		return -1;
	}

	int currentPos = m_items.getCount();
	int storePos = m_items.getCount();
	for (int i = 0; i < m_items.getCount(); i++) {
		if (m_items[i] == pChild)
			storePos = i;

		if (m_items[i] == m_pCurrentItem)
			currentPos = i;
	}

	if (storePos == m_items.getCount())
		return -1;

	if (storePos > currentPos) {
		m_items[storePos]->storeIcon(hContact, value);
		return 0;
	}

	// Ok, we have to set the icon, but we have to assert it is a valid icon
	m_setValidExtraIcon = false;

	int ret;
	if (bByName)
		ret = m_items[storePos]->setIconByName(hContact, (const char*)value);
	else
		ret = m_items[storePos]->setIcon(hContact, (HANDLE)value);

	if (storePos < currentPos) {
		if (m_setValidExtraIcon)
			m_pCurrentItem = m_items[storePos];
	}
	else if (storePos == currentPos) {
		if (!m_setValidExtraIcon) {
			m_pCurrentItem = nullptr;

			m_insideApply = true;

			for (++storePos; storePos < m_items.getCount(); ++storePos) {
				m_items[storePos]->applyIcon(hContact);
				if (m_setValidExtraIcon) {
					m_pCurrentItem = m_items[storePos];
					break;
				}
			}

			m_insideApply = false;
		}
	}

	return ret;
}

const wchar_t* ExtraIconGroup::getDescription() const
{
	return m_tszDescription;
}

HANDLE ExtraIconGroup::getDescIcon() const
{
	for (auto &p : m_items)
		if (HANDLE ret = p->getDescIcon())
			return ret;

	return nullptr;
}

int ExtraIconGroup::getType() const
{
	return EXTRAICON_TYPE_GROUP;
}

int ExtraIconGroup::ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage)
{
	if (hImage != INVALID_HANDLE_VALUE)
		m_setValidExtraIcon = true;

	return Clist_SetExtraIcon(hContact, m_slot, hImage);
}
