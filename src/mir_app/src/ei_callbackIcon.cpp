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

CallbackExtraIcon::CallbackExtraIcon(const char *_name, const wchar_t *_description, HANDLE _descIcon,
		MIRANDAHOOK _RebuildIcons, MIRANDAHOOK _ApplyIcon, MIRANDAHOOKPARAM _OnClick, LPARAM _param) :
	BaseExtraIcon(_name, _description, _descIcon, _OnClick, _param),
	m_pfnRebuildIcons(_RebuildIcons), m_pfnApplyIcon(_ApplyIcon), m_needToRebuild(true)
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
		m_needToRebuild = true;
		return;
	}

	m_needToRebuild = false;
	m_pfnRebuildIcons(0, 0);
}

void CallbackExtraIcon::applyIcon(MCONTACT hContact)
{
	if (!isEnabled() || hContact == 0)
		return;

	if (m_needToRebuild)
		rebuildIcons();

	m_pfnApplyIcon(hContact, 0);
}

int CallbackExtraIcon::setIcon(MCONTACT hContact, HANDLE icon)
{
	if (!isEnabled() || hContact == 0)
		return -1;

	return ClistSetExtraIcon(hContact, icon);
}

int CallbackExtraIcon::setIconByName(MCONTACT, const char*)
{
	return -1;
}
