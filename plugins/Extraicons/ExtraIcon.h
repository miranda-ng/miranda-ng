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

#ifndef __EXTRAICON_H__
#define __EXTRAICON_H__

#include <string>

#define EXTRAICON_TYPE_GROUP -1

class ExtraIcon
{
public:
	ExtraIcon(const char *name);
	virtual ~ExtraIcon();

	virtual void rebuildIcons() =0;
	virtual void applyIcons();
	virtual void applyIcon(HANDLE hContact) =0;
	virtual void onClick(HANDLE hContact) =0;

	virtual int setIcon(int id, HANDLE hContact, void *icon) =0;
	virtual void storeIcon(HANDLE hContact, void *icon) =0;

	virtual const char *getName() const;
	virtual const char *getDescription() const =0;
	virtual const char *getDescIcon() const =0;
	virtual int getType() const =0;

	virtual int getSlot() const;
	virtual void setSlot(int slot);

	virtual int getPosition() const;
	virtual void setPosition(int position);

	virtual bool isEnabled() const;

	/// @retun <0 if this < other, 0 if this == other, >0 if this > other
	virtual int compare(const ExtraIcon *other) const;

	bool operator==(const ExtraIcon &other) const;
	bool operator!=(const ExtraIcon &other) const;
	bool operator<(const ExtraIcon &other) const;
	bool operator<=(const ExtraIcon &other) const;
	bool operator>(const ExtraIcon &other) const;
	bool operator>=(const ExtraIcon &other) const;

	virtual int ClistSetExtraIcon(HANDLE hContact, HANDLE hImage) =0;

protected:
	std::string name;

	int slot;
	int position;
};

#endif // __EXTRAICON_H__
