/*
AddContact+ plugin for Miranda NG

Copyright (C) 2007-11 Bartosz 'Dezeath' Białek
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <windows.h>
#include <commctrl.h>
#include <limits.h>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_clistint.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_contacts.h>

#include "m_toptoolbar.h"
#include "m_addcontactplus.h"

#include "resource.h"
#include "version.h"

#define MODULENAME "AddContact"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#define	ICON_ADD "AddContactPlus_Icon"

INT_PTR CALLBACK AddContactDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
extern HWND hAddDlg;
