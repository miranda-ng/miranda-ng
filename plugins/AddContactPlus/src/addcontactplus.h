/*
AddContact+ plugin for Miranda NG

Copyright (C) 2007-11 Bartosz 'Dezeath' Bia³ek
Copyright (C) 2012-15 Miranda NG Team

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

#include <windows.h>
#include <commctrl.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_utils.h>
#include <m_clistint.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_addcontact.h>
#include <m_string.h>

#include "m_toptoolbar.h"
#include "m_addcontactplus.h"

#include "resource.h"
#include "version.h"

#define	ICON_ADD "AddContactPlus_Icon"

INT_PTR CALLBACK AddContactDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
extern HINSTANCE hInst;
extern HWND hAddDlg;
