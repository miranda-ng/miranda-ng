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

#include "..\..\core\commonheaders.h"

#include "IcolibExtraIcon.h"

#include "extraicons.h"
#include "usedIcons.h"

IcolibExtraIcon::IcolibExtraIcon(int _id, const char *_name, const TCHAR *_description, const char *_descIcon,
		MIRANDAHOOKPARAM _OnClick, LPARAM _param) :
	BaseExtraIcon(_id, _name, _description, _descIcon, _OnClick, _param)
{
	char setting[512];
	mir_snprintf(setting, SIZEOF(setting), "%s/%s", MODULE_NAME, _name);
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

	HANDLE hImage = INVALID_HANDLE_VALUE;

	DBVARIANT dbv = { 0 };
	if ( !DBGetContactSettingString(hContact, MODULE_NAME, name.c_str(), &dbv)) {
		if (!IsEmpty(dbv.pszVal))
			hImage = GetIcon(dbv.pszVal);

		db_free(&dbv);
	}

	ClistSetExtraIcon(hContact, hImage);
}

int IcolibExtraIcon::setIcon(int id, HANDLE hContact, HANDLE hIcoLib)
{
	if (hContact == NULL || id != this->id)
		return -1;

	if (hIcoLib == INVALID_HANDLE_VALUE)
		hIcoLib = NULL;

	if (isEnabled()) {
		DBVARIANT dbv = { 0 };
		if ( !DBGetContactSettingString(hContact, MODULE_NAME, name.c_str(), &dbv)) {
			if (!IsEmpty(dbv.pszVal))
				RemoveIcon(dbv.pszVal);

			db_free(&dbv);
		}
	}

	storeIcon(hContact, "");

	if (isEnabled()) {
		HANDLE hImage;
		if (hIcoLib == NULL)
			hImage = INVALID_HANDLE_VALUE;
		else
			hImage = AddIcon(hIcoLib);

		return ClistSetExtraIcon(hContact, hImage);
	}

	return 0;
}

int IcolibExtraIcon::setIconByName(int id, HANDLE hContact, const char *icon)
{
	if (hContact == NULL || id != this->id)
		return -1;

	if (icon == INVALID_HANDLE_VALUE)
		icon = NULL;

	if (isEnabled()) {
		DBVARIANT dbv = { 0 };
		if ( !DBGetContactSettingString(hContact, MODULE_NAME, name.c_str(), &dbv)) {
			if (!IsEmpty(dbv.pszVal))
				RemoveIcon(dbv.pszVal);

			db_free(&dbv);
		}
	}

	storeIcon(hContact, (char*)icon);

	if (isEnabled()) {
		HANDLE hImage;
		if ( IsEmpty(icon))
			hImage = INVALID_HANDLE_VALUE;
		else
			hImage = AddIcon(icon);

		return ClistSetExtraIcon(hContact, hImage);
	}

	return 0;
}

void IcolibExtraIcon::storeIcon(HANDLE hContact, void *icon)
{
	if (hContact == NULL)
		return;

	const char *icolibName = (const char *) icon;
	if ( IsEmpty(icolibName))
		db_unset(hContact, MODULE_NAME, name.c_str());
	else
		db_set_s(hContact, MODULE_NAME, name.c_str(), icolibName);
}
