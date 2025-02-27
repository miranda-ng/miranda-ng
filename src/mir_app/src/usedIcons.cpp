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

struct Icon
{
	ptrA name;
	int refCount = 0;
	HANDLE hImage = INVALID_HANDLE_VALUE;

	Icon(const char *icolibName) :
		name(mir_strdup(icolibName))
	{}
};

static int SortFunc(const Icon *p1, const Icon *p2)
{
	return mir_strcmp(p1->name, p2->name);
}

static OBJLIST<Icon> usedIcons(50, SortFunc);

static Icon* FindIcon(const char *icolibName)
{
	Icon *icon = usedIcons.find((Icon*)&icolibName);
	if (icon == nullptr)
		usedIcons.insert(icon = new Icon(icolibName));

	if (icon->hImage == INVALID_HANDLE_VALUE) {
		HICON hIcon = IcoLib_GetIcon(icon->name);
		if (hIcon != nullptr) {
			icon->hImage = ExtraIcon_AddIcon(hIcon);
			IcoLib_ReleaseIcon(hIcon);
		}
	}

	return icon;
}

HANDLE GetIcon(const char *icolibName)
{
	return FindIcon(icolibName)->hImage;
}

HANDLE AddIcon(const char *icolibName)
{
	Icon *icon = FindIcon(icolibName);
	icon->refCount++;
	return icon->hImage;
}

void RemoveIcon(const char *icolibName)
{
	Icon *icon = usedIcons.find((Icon *)&icolibName);
	if (icon != nullptr)
		icon->refCount--;
}

void ResetIcons()
{
	for (auto &it : usedIcons.rev_iter()) {
		if (it->refCount <= 0)
			usedIcons.removeItem(&it);
		else
			it->hImage = INVALID_HANDLE_VALUE;
	}
}
