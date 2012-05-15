/* 
Copyright (C) 2007 Dmitry Titkov (C) 2010 tico-tico, Mataes

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

#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

#ifndef __TRANSLITSWIITCHER_H__
#define __TRANSLITSWIITCHER_H__

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <time.h>

#include "newpluginapi.h"
#include "m_options.h"
#include "m_langpack.h"
#include "m_database.h"
#include "m_hotkeys.h"
#include "m_protosvc.h"
#include "win2k.h"
#include <m_icolib.h>
#include "m_msg_buttonsbar.h"
#include "m_popup.h"

#include "..\version.h"
#include "..\resource.h"
#include "m_smileyadd.h"
#include "m_ieview.h"
#include "m_popup2.h"

#define IDC_MESSAGE				1002
#define IDC_CHATMESSAGE			1009

VOID SwitchLayout(BOOL);
VOID TranslitLayout(BOOL);
VOID InvertCase(BOOL);
int OnButtonPressed(WPARAM wParam, LPARAM lParam);

// {0286947D-3140-4222-B5AD-2C92315E1C1E}
#define MIID_TS { 0x286947d, 0x3140, 0x4222, { 0xb5, 0xad, 0x2c, 0x92, 0x31, 0x5e, 0x1c, 0x1e } }

#endif
