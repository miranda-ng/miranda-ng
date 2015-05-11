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

#ifndef M_WWI_BIRTHDAYS_H
#define M_WWI_BIRTHDAYS_H

struct TBirthdayContact{
	MCONTACT hContact;
	HANDLE hClistIcon;
};

typedef TBirthdayContact *PBirthdayContact;

class CBirthdays
{
protected:
	int count;
	int size;
	PBirthdayContact *birthdays;
	int advancedIcon;

	void Realloc(int increaseCapacity);
	void EnsureCapacity();

	void ClearItem(int index);

public:
	CBirthdays(int initialSize = 10);
	~CBirthdays();

	int Add(MCONTACT hContact, HANDLE hClistIcon);
	int Remove(int index);
	int Remove(MCONTACT hContact);
	void Clear();
	void Destroy();

	int Index(MCONTACT hContact) const;
	int Contains(MCONTACT hContact) const;

	void SetAdvancedIconIndex(int advIcon);
	int GetAdvancedIconIndex() const;

	HANDLE GetClistIcon(MCONTACT hContact) const;

	int Count() const;
	int Size() const;
};

extern CBirthdays &birthdays;

#endif //M_WWI_BIRTHDAYS_H
