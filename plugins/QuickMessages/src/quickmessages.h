/*
Quick Messages plugin for Miranda IM

Copyright (C) 2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <Richedit.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_protomod.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_contacts.h>
#include <win2k.h>

#include <m_msg_buttonsbar.h>

#include "resource.h"
#include "Version.h"
#include "Utils.h"

#define PLGNAME "QuickMessages"

extern HINSTANCE hinstance;
extern HANDLE hIcolib;
extern ListData* ButtonsList[100];
extern SortedList* QuickList;
extern BOOL g_bRClickAuto;
extern BOOL g_bLClickAuto;
extern BOOL g_bQuickMenu;
extern int g_iButtonsCount;

int OptionsInit(WPARAM,LPARAM);

#define IDC_MESSAGE      1002
#define IDC_CHATMESSAGE  1009
