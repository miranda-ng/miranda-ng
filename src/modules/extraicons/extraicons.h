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

#ifndef __COMMONS_H__
# define __COMMONS_H__

#define MODULE_NAME "ExtraIcons"

// Global Variables
extern HINSTANCE hInst;

#define FREE(_m_)		if (_m_ != NULL) { free(_m_); _m_ = NULL; }

#define ICON_SIZE 16

#include "Extraicon.h"

extern LIST<BaseExtraIcon> registeredExtraIcons;
extern LIST<ExtraIcon> extraIconsByHandle, extraIconsBySlot;
void RebuildListsBasedOnGroups(LIST<ExtraIconGroup> &groups);
ExtraIcon * GetExtraIconBySlot(int slot);

int GetNumberOfSlots();
int ConvertToClistSlot(int slot);

int Clist_SetExtraIcon(MCONTACT hContact, int slot, HANDLE hImage);

void DefaultExtraIcons_Load();

HANDLE ExtraIcon_Add(HICON hIcon);

void fnReloadExtraIcons();
void fnSetAllExtraIcons(MCONTACT hContact);

static inline BOOL IsEmpty(const char *str)
{
	return str == NULL || str[0] == 0;
}

static inline int MIN(int a, int b)
{
	if (a <= b)
		return a;
	return b;
}

static inline int MAX(int a, int b)
{
	if (a >= b)
		return a;
	return b;
}

#endif // __COMMONS_H__
