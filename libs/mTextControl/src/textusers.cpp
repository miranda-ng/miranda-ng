/*
Miranda Text Control - Plugin for Miranda IM
Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

HANDLE htuDefault = nullptr;

static TextUser *textUserFirst = nullptr;
static TextUser *textUserLast = nullptr;

MTEXTCONTROL_DLL(HANDLE) MTextRegister(const char *userTitle, DWORD options)
{
	TextUser *textUserNew = new TextUser;
	textUserNew->name = new char[mir_strlen(userTitle) + 1];
	mir_strcpy(textUserNew->name, userTitle);
	textUserNew->options =
		(db_get_dw(0, MODULENAME, userTitle, options) & MTEXT_FANCY_MASK) | (textUserNew->options & MTEXT_SYSTEM_MASK);
	db_set_dw(0, MODULENAME, userTitle, textUserNew->options);
	textUserNew->prev = textUserLast;
	textUserNew->next = nullptr;
	if (textUserLast) {
		textUserLast->next = textUserNew;
		textUserLast = textUserNew;
	}
	else textUserFirst = textUserLast = textUserNew;

	return textUserNew;
}

DWORD TextUserGetOptions(HANDLE userHandle)
{
	if (!userHandle) return 0;
	return ((TextUser *)userHandle)->options;
}

void TextUserSetOptions(HANDLE userHandle, uint32_t options)
{
	if (!userHandle) return;
	((TextUser *)userHandle)->options = options;
}

void TextUsersSave()
{
	for (TextUser *textUser = textUserFirst; textUser; textUser = textUser->next)
		db_set_dw(0, MODULENAME, textUser->name, textUser->options);
}

void TextUsersReset()
{
	for (TextUser *textUser = textUserFirst; textUser; textUser = textUser->next)
		textUser->options = (db_get_dw(0, MODULENAME, textUser->name, 0) & MTEXT_FANCY_MASK) | (textUser->options&MTEXT_SYSTEM_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadTextUsers()
{
	htuDefault = MTextRegister("Text Controls", MTEXT_FANCY_MASK);
}

void UnloadTextUsers()
{
	while (textUserFirst) {
		delete[] textUserFirst->name;
		TextUser *next = textUserFirst->next;
		delete[] textUserFirst;
		textUserFirst = next;
	}
}
