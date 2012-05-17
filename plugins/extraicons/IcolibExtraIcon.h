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

#ifndef __ICOLIBEXTRAICON_H__
#define __ICOLIBEXTRAICON_H__

#include "BaseExtraIcon.h"

class IcolibExtraIcon : public BaseExtraIcon
{
public:
	IcolibExtraIcon(int id, const char *name, const char *description, const char *descIcon, MIRANDAHOOKPARAM OnClick,
			LPARAM param);
	virtual ~IcolibExtraIcon();

	virtual int getType() const;

	virtual void rebuildIcons();
	virtual void applyIcon(HANDLE hContact);

	virtual int setIcon(int id, HANDLE hContact, void *icon);
	virtual void storeIcon(HANDLE hContact, void *icon);
};

#endif // __ICOLIBEXTRAICON_H__
