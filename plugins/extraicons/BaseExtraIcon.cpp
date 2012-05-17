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

BaseExtraIcon::BaseExtraIcon(int id, const char *name, const char *description, const char *descIcon,
		MIRANDAHOOKPARAM OnClick, LPARAM param) :
	ExtraIcon(name), id(id), description(description), descIcon(descIcon), OnClick(OnClick), onClickParam(param)
{
}

BaseExtraIcon::~BaseExtraIcon()
{
}

void BaseExtraIcon::setOnClick(MIRANDAHOOKPARAM OnClick, LPARAM param)
{
	this->OnClick = OnClick;
	this->onClickParam = param;
}

int BaseExtraIcon::getID() const
{
	return id;
}

const char *BaseExtraIcon::getDescription() const
{
	return description.c_str();
}

void BaseExtraIcon::setDescription(const char *desc)
{
	description = desc;
}

const char *BaseExtraIcon::getDescIcon() const
{
	return descIcon.c_str();
}

void BaseExtraIcon::setDescIcon(const char *icon)
{
	descIcon = icon;
}

void BaseExtraIcon::onClick(HANDLE hContact)
{
	if (OnClick == NULL)
		return;

	OnClick((WPARAM) hContact, (LPARAM) ConvertToClistSlot(slot), onClickParam);
}

int BaseExtraIcon::ClistSetExtraIcon(HANDLE hContact, HANDLE hImage)
{
	ExtraIcon *tmp = extraIconsByHandle[id - 1];
	if (tmp != this)
		return tmp->ClistSetExtraIcon(hContact, hImage);
	else
		return Clist_SetExtraIcon(hContact, slot, hImage);
}

