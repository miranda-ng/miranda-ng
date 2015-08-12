/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

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
	ExtraIcon(_name), m_setValidExtraIcon(false), m_insideApply(false),
	m_items(1)
{
	db_set_resident(MODULE_NAME, _name);
}

ExtraIconGroup::~ExtraIconGroup()
{
}

void ExtraIconGroup::addExtraIcon(BaseExtraIcon *extra)
{
	m_items.insert(extra);

	CMString description;
	for (int i = 0; i < m_items.getCount(); i++) {
		if (i > 0)
			description += _T(" / ");
		description += m_items[i]->getDescription();
	}

	m_tszDescription = mir_tstrdup(description);
}

void ExtraIconGroup::rebuildIcons()
{
	for (int i = 0; i < m_items.getCount(); i++)
		m_items[i]->rebuildIcons();
}

void ExtraIconGroup::applyIcon(MCONTACT hContact)
{
	if (!isEnabled() || hContact == NULL)
		return;

	m_setValidExtraIcon = false;
	m_insideApply = true;

	int i;
	for (i = 0; i < m_items.getCount(); i++) {
		m_items[i]->applyIcon(hContact);
		if (m_setValidExtraIcon)
			break;
	}

	m_insideApply = false;

	db_set_dw(hContact, MODULE_NAME, m_szName, m_setValidExtraIcon ? m_items[i]->getID() : 0);
}

int ExtraIconGroup::getPosition() const
{
	int pos = INT_MAX;
	for (int i = 0; i < m_items.getCount(); i++)
		pos = MIN(pos, m_items[i]->getPosition());
	return pos;
}

void ExtraIconGroup::setSlot(int slot)
{
	ExtraIcon::setSlot(slot);

	for (int i = 0; i < m_items.getCount(); i++)
		m_items[i]->setSlot(slot);
}

ExtraIcon * ExtraIconGroup::getCurrentItem(MCONTACT hContact) const
{
	int id = (int)db_get_dw(hContact, MODULE_NAME, m_szName, 0);
	if (id < 1)
		return NULL;

	for (int i = 0; i < m_items.getCount(); i++)
		if (id == m_items[i]->getID())
			return m_items[i];

	return NULL;
}

void ExtraIconGroup::onClick(MCONTACT hContact)
{
	ExtraIcon *extra = getCurrentItem(hContact);
	if (extra != NULL)
		extra->onClick(hContact);
}

int ExtraIconGroup::setIcon(int id, MCONTACT hContact, HANDLE value)
{
	return internalSetIcon(id, hContact, (void*)value, false);
}

int ExtraIconGroup::setIconByName(int id, MCONTACT hContact, const char *value)
{
	return internalSetIcon(id, hContact, (void*)value, true);
}

int ExtraIconGroup::internalSetIcon(int id, MCONTACT hContact, void *value, bool bByName)
{
	if (m_insideApply) {
		for (int i=0; i < m_items.getCount(); i++)
			if (m_items[i]->getID() == id) {
				if (bByName)
					return m_items[i]->setIconByName(id, hContact, (const char*)value);
				return m_items[i]->setIcon(id, hContact, (HANDLE)value);
			}

		return -1;
	}

	ExtraIcon *current = getCurrentItem(hContact);
	int currentPos = m_items.getCount();
	int storePos = m_items.getCount();
	for (int i=0; i < m_items.getCount(); i++) {
		if (m_items[i]->getID() == id)
			storePos = i;

		if (m_items[i] == current)
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
		ret = m_items[storePos]->setIconByName(id, hContact, (const char*)value);
	else
		ret = m_items[storePos]->setIcon(id, hContact, (HANDLE)value);

	if (storePos < currentPos) {
		if (m_setValidExtraIcon)
			db_set_dw(hContact, MODULE_NAME, m_szName, m_items[storePos]->getID());
	}
	else if (storePos == currentPos) {
		if (!m_setValidExtraIcon) {
			db_set_dw(hContact, MODULE_NAME, m_szName, 0);

			m_insideApply = true;

			for (++storePos; storePos < m_items.getCount(); ++storePos) {
				m_items[storePos]->applyIcon(hContact);
				if (m_setValidExtraIcon)
					break;
			}

			m_insideApply = false;

			if (m_setValidExtraIcon && storePos < m_items.getCount())
				db_set_dw(hContact, MODULE_NAME, m_szName, m_items[storePos]->getID());
		}
	}

	return ret;
}

const TCHAR* ExtraIconGroup::getDescription() const
{
	return m_tszDescription;
}

const char *ExtraIconGroup::getDescIcon() const
{
	for (int i = 0; i < m_items.getCount(); i++)
		if (!IsEmpty(m_items[i]->getDescIcon()))
			return m_items[i]->getDescIcon();

	return "";
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
