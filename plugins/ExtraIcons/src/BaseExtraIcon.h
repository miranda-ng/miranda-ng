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

#ifndef __BASEEXTRAICON_H__
#define __BASEEXTRAICON_H__

#include "ExtraIcon.h"

class BaseExtraIcon : public ExtraIcon
{
public:
	BaseExtraIcon(int id, const char *name, const char *description, const char *descIcon, MIRANDAHOOKPARAM OnClick,
			LPARAM param);
	virtual ~BaseExtraIcon();

	virtual int getID() const;
	virtual const char *getDescription() const;
	virtual void setDescription(const char *desc);
	virtual const char *getDescIcon() const;
	virtual void setDescIcon(const char *icon);
	virtual int getType() const =0;

	virtual void onClick(HANDLE hContact);
	virtual void setOnClick(MIRANDAHOOKPARAM OnClick, LPARAM param);

	virtual int ClistSetExtraIcon(HANDLE hContact, HANDLE hImage);

protected:
	int id;
	std::string description;
	std::string descIcon;
	MIRANDAHOOKPARAM OnClick;
	LPARAM onClickParam;
};

#endif // __BASEEXTRAICON_H__
