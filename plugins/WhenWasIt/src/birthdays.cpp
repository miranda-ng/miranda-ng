/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

CBirthdays &birthdays = CBirthdays();

CBirthdays::CBirthdays(int initialSize)
{
	count = 0;
	size = 0;
	birthdays = NULL;
	advancedIcon = CLIST_ICON;

	Realloc(initialSize);
}

CBirthdays::~CBirthdays()
{
	Destroy();
}

void CBirthdays::Destroy()
{
	if (birthdays) {
		Clear();
		free(birthdays);
		birthdays = NULL;
	}
}

void CBirthdays::Clear()
{
	for (int i = 0; i < Count(); i++)
		ClearItem(i);

	count = 0;
}

void CBirthdays::ClearItem(int index)
{
	free(birthdays[index]);
	birthdays[index] = NULL;
}

int CBirthdays::Count() const
{
	return count;
}

int CBirthdays::Size() const
{
	return size;
}

void CBirthdays::EnsureCapacity()
{
	if (count >= size)
		Realloc(size / 2);
}

void CBirthdays::Realloc(int increaseCapacity)
{
	size += increaseCapacity;
	birthdays = (PBirthdayContact *)realloc(birthdays, size * sizeof(PBirthdayContact));
}

int CBirthdays::Add(MCONTACT hContact, HANDLE hClistIcon)
{
	if (!Contains(hContact)) {
		EnsureCapacity();
		TBirthdayContact *item = (TBirthdayContact *)malloc(sizeof(TBirthdayContact));
		item->hContact = hContact;
		item->hClistIcon = hClistIcon;
		birthdays[count++] = item;
		return 0;
	}
	return -1;
}

int CBirthdays::Remove(int index)
{
	if ((index >= 0) && (index < count)) {
		for (int i = index + 1; i < count; i++)
			birthdays[i - 1] = birthdays[i];

		ClearItem(count--);
		return 0;
	}
	return -1;
}

int CBirthdays::Remove(MCONTACT hContact)
{
	return Remove(Index(hContact));
}

int CBirthdays::Contains(MCONTACT hContact) const
{
	for (int i = 0; i < count; i++)
		if (birthdays[i]->hContact == hContact)
			return TRUE;

	return FALSE;
}

int CBirthdays::Index(MCONTACT hContact) const
{
	for (int i = 0; i < count; i++)
		if (birthdays[i]->hContact == hContact)
			return i;

	return -1;
}

void CBirthdays::SetAdvancedIconIndex(int advIcon)
{
	advancedIcon = advIcon;
}

int CBirthdays::GetAdvancedIconIndex() const
{
	return advancedIcon;
}

HANDLE CBirthdays::GetClistIcon(MCONTACT hContact) const
{
	int index = Index(hContact);
	if ((index >= 0) && (index < count))
		return birthdays[index]->hClistIcon;

	return INVALID_HANDLE_VALUE;
}
