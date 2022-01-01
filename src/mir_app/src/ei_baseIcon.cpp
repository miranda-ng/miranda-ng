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

BaseExtraIcon::BaseExtraIcon(const char *name, const wchar_t *description, HANDLE descIcon, MIRANDAHOOKPARAM OnClick, LPARAM param) :
	ExtraIcon(name),
	m_OnClick(OnClick),
	m_onClickParam(param),
	m_tszDescription(mir_wstrdup(description)),
	m_hDescIcon(descIcon)
{
	if (!IcoLib_IsManaged((HICON)descIcon))
		m_hDescIcon = IcoLib_GetIconHandle((const char *)descIcon);

	m_id = registeredExtraIcons.getCount() + 1;
}

BaseExtraIcon::~BaseExtraIcon()
{
}

void BaseExtraIcon::setOnClick(MIRANDAHOOKPARAM pFunc, LPARAM pParam)
{
	m_OnClick = pFunc;
	m_onClickParam = pParam;
}

const wchar_t* BaseExtraIcon::getDescription() const
{
	return TranslateW_LP(m_tszDescription, m_pPlugin);
}

HANDLE BaseExtraIcon::getDescIcon() const
{
	return m_hDescIcon;
}

void BaseExtraIcon::onClick(MCONTACT hContact)
{
	if (m_OnClick != nullptr)
		m_OnClick(hContact, (LPARAM)ConvertToClistSlot(m_slot), m_onClickParam);
}

int BaseExtraIcon::ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage)
{
	if (m_pParent)
		return m_pParent->ClistSetExtraIcon(hContact, hImage);
	return Clist_SetExtraIcon(hContact, m_slot, hImage);
}
