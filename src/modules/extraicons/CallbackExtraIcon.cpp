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

CallbackExtraIcon::CallbackExtraIcon(int _id, const char *_name, const TCHAR *_description, const char *_descIcon,
		MIRANDAHOOK _RebuildIcons, MIRANDAHOOK _ApplyIcon, MIRANDAHOOKPARAM _OnClick, LPARAM _param) :
	BaseExtraIcon(_id, _name, _description, _descIcon, _OnClick, _param),
	RebuildIcons(_RebuildIcons), ApplyIcon(_ApplyIcon), needToRebuild(true)
{
}

CallbackExtraIcon::~CallbackExtraIcon()
{
}

int CallbackExtraIcon::getType() const
{
	return EXTRAICON_TYPE_CALLBACK;
}

void CallbackExtraIcon::rebuildIcons()
{
	if (!isEnabled()) {
		needToRebuild = true;
		return;
	}

	needToRebuild = false;
	RebuildIcons(0, 0);
}

void CallbackExtraIcon::applyIcon(MCONTACT hContact)
{
	if (!isEnabled() || hContact == NULL)
		return;

	if (needToRebuild)
		rebuildIcons();

	ApplyIcon(hContact, 0);
}

int CallbackExtraIcon::setIcon(int id, MCONTACT hContact, HANDLE icon)
{
	if (!isEnabled() || hContact == NULL || id != this->id)
		return -1;

	return ClistSetExtraIcon(hContact, icon);
}

int CallbackExtraIcon::setIconByName(int id, MCONTACT hContact, const char *icon)
{
	return -1;
}
