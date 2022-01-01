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

ExtraIcon::ExtraIcon(const char *name) :
	m_szName(mir_strdup(name))
{
}

ExtraIcon::~ExtraIcon()
{
}

const char *ExtraIcon::getName() const
{
	return m_szName;
}

int ExtraIcon::getSlot() const
{
	return m_slot;
}

void ExtraIcon::setSlot(int slot)
{
	m_slot = slot;
}

int ExtraIcon::getPosition() const
{
	return m_position;
}

void ExtraIcon::setPosition(int position)
{
	m_position = position;
}

bool ExtraIcon::isEnabled() const
{
	return m_slot >= 0;
}

void ExtraIcon::doApply(MCONTACT hContact)
{
	if (m_pParent)
		m_pParent->applyIcon(hContact);
	else
		applyIcon(hContact);
}

void ExtraIcon::applyIcons()
{
	if (!isEnabled())
		return;

	for (auto &hContact : Contacts()) {
		// Clear to assert that it will be cleared
		Clist_SetExtraIcon(hContact, m_slot, INVALID_HANDLE_VALUE);
		doApply(hContact);
	}
}
