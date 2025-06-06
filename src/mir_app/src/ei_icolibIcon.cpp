/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-25 Miranda NG team

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
#include "usedIcons.h"

#include "IcoLib.h"

IcolibExtraIcon::IcolibExtraIcon(const char *_name, const wchar_t *_description, HANDLE _descIcon, MIRANDAHOOKPARAM _OnClick, LPARAM _param) :
	BaseExtraIcon(_name, _description, _descIcon, _OnClick, _param)
{
	db_set_resident(EI_MODULE_NAME, _name);
}

IcolibExtraIcon::~IcolibExtraIcon()
{
}

int IcolibExtraIcon::getType() const
{
	return EXTRAICON_TYPE_ICOLIB;
}

void IcolibExtraIcon::rebuildIcons()
{
}

void IcolibExtraIcon::applyIcon(MCONTACT hContact)
{
	if (!isEnabled() || hContact == 0)
		return;

	HANDLE hImage = INVALID_HANDLE_VALUE;

	ptrA szIconName(db_get_sa(hContact, EI_MODULE_NAME, m_szName));
	if (!IsEmpty(szIconName))
		hImage = GetIcon(szIconName);

	ClistSetExtraIcon(hContact, hImage);
}

int IcolibExtraIcon::setIcon(MCONTACT hContact, HANDLE hIcoLib)
{
	if (hContact == 0)
		return -1;

	if (hIcoLib == INVALID_HANDLE_VALUE)
		hIcoLib = nullptr;

	if (isEnabled()) {
		ptrA szIconName(db_get_sa(hContact, EI_MODULE_NAME, m_szName));
		if (!IsEmpty(szIconName))
			RemoveIcon(szIconName);
	}

	IcolibItem *p = (IcolibItem*)hIcoLib;
	char *szName = (p) ? p->name : nullptr;
	storeIcon(hContact, szName);

	if (isEnabled())
		return ClistSetExtraIcon(hContact, (hIcoLib == nullptr) ? INVALID_HANDLE_VALUE : AddIcon(szName));

	return 0;
}

int IcolibExtraIcon::setIconByName(MCONTACT hContact, const char *icon)
{
	if (hContact == 0)
		return -1;

	if (icon == INVALID_HANDLE_VALUE)
		icon = nullptr;

	if (isEnabled()) {
		ptrA szIconName(db_get_sa(hContact, EI_MODULE_NAME, m_szName));
		if (!IsEmpty(szIconName))
			RemoveIcon(szIconName);
	}

	storeIcon(hContact, (char*)icon);

	if (isEnabled())
		return ClistSetExtraIcon(hContact, (IsEmpty(icon)) ? INVALID_HANDLE_VALUE : AddIcon(icon));

	return 0;
}

void IcolibExtraIcon::storeIcon(MCONTACT hContact, void *icon)
{
	if (hContact == 0)
		return;

	const char *icolibName = (const char *)icon;
	if (IsEmpty(icolibName))
		db_unset(hContact, EI_MODULE_NAME, m_szName);
	else
		db_set_s(hContact, EI_MODULE_NAME, m_szName, icolibName);
}
