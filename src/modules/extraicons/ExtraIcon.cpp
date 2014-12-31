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

#include "..\..\core\commonheaders.h"

#include "extraicons.h"

ExtraIcon::ExtraIcon(const char *name) :
	szName(mir_strdup(name)), slot(-1), position(1000), hLangpack(0)
{
}

ExtraIcon::~ExtraIcon()
{
}

const char *ExtraIcon::getName() const
{
	return szName;
}

int ExtraIcon::getSlot() const
{
	return slot;
}

void ExtraIcon::setSlot(int slot)
{
	this->slot = slot;
}

int ExtraIcon::getPosition() const
{
	return position;
}

void ExtraIcon::setPosition(int position)
{
	this->position = position;
}

bool ExtraIcon::isEnabled() const
{
	return slot >= 0;
}

void ExtraIcon::applyIcons()
{
	if (!isEnabled())
		return;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		// Clear to assert that it will be cleared
		Clist_SetExtraIcon(hContact, slot, INVALID_HANDLE_VALUE);
		applyIcon(hContact);
	}
}
