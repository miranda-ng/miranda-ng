/*
 Copyright (C) 2009 Ricardo Pescuma Domenecci

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

#include "commons.h"

IcolibExtraIcon::IcolibExtraIcon(int id, const char *name, const char *description, const char *descIcon,
		MIRANDAHOOKPARAM OnClick, LPARAM param) :
	BaseExtraIcon(id, name, description, descIcon, OnClick, param)
{
	char setting[512];
	mir_snprintf(setting, MAX_REGS(setting), "%s/%s", MODULE_NAME, name);
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (WPARAM) setting);
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

void IcolibExtraIcon::applyIcon(HANDLE hContact)
{
	if (!isEnabled() || hContact == NULL)
		return;

	HANDLE hImage = NULL;

	DBVARIANT dbv = { 0 };
	if (!DBGetContactSettingString(hContact, MODULE_NAME, name.c_str(), &dbv))
	{
		if (!IsEmpty(dbv.pszVal))
			hImage = GetIcon(dbv.pszVal);

		DBFreeVariant(&dbv);
	}

	ClistSetExtraIcon(hContact, hImage);
}

int IcolibExtraIcon::setIcon(int id, HANDLE hContact, void *icon)
{
	if (hContact == NULL || id != this->id)
		return -1;

	if (isEnabled())
	{
		DBVARIANT dbv = { 0 };
		if (!DBGetContactSettingString(hContact, MODULE_NAME, name.c_str(), &dbv))
		{
			if (!IsEmpty(dbv.pszVal))
				RemoveIcon(dbv.pszVal);

			DBFreeVariant(&dbv);
		}
	}

	storeIcon(hContact, icon);

	if (isEnabled())
	{
		const char *icolibName = (const char *) icon;

		HANDLE hImage;
		if (IsEmpty(icolibName))
			hImage = NULL;
		else
			hImage = AddIcon(icolibName);

		return ClistSetExtraIcon(hContact, hImage);
	}

	return 0;
}

void IcolibExtraIcon::storeIcon(HANDLE hContact, void *icon)
{
	if (hContact == NULL)
		return;

	const char *icolibName = (const char *) icon;
	if (IsEmpty(icolibName))
		icolibName = ""; // Delete don't work and I don't know why

	DBWriteContactSettingString(hContact, MODULE_NAME, name.c_str(), icolibName);
}

