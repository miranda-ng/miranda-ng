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

struct Icon
{
	char *name;
	int refCount;
	HANDLE hImage;

	Icon(const char *icolibName) :
		name( mir_strdup(icolibName)), refCount(0), hImage(INVALID_HANDLE_VALUE)
	{
	}

	~Icon()
	{	mir_free(name);
	}
};

static int SortFunc(const Icon *p1, const Icon *p2)
{
	return mir_strcmp(p1->name, p2->name);
}

static OBJLIST<Icon> usedIcons(50, SortFunc);

static Icon* FindIcon(const char *icolibName)
{
	Icon *icon = usedIcons.find((Icon*)&icolibName);
	if (icon == NULL)
		usedIcons.insert(icon = new Icon(icolibName));

	if (icon->hImage == INVALID_HANDLE_VALUE) {
		HICON hIcon = Skin_GetIcon(icon->name);
		if (hIcon != NULL) {
			icon->hImage = ExtraIcon_Add(hIcon);
			Skin_ReleaseIcon(hIcon);
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
	Icon *icon = usedIcons.find((Icon*)&icolibName);
	if (icon != NULL)
		icon->refCount--;
}

void ResetIcons()
{
	for (int i = usedIcons.getCount()-1; i >= 0; i--) {
		Icon &p = usedIcons[i];
		if (p.refCount <= 0)
			usedIcons.remove(i);
		else
			p.hImage = INVALID_HANDLE_VALUE;
	}
}
