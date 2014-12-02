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

#ifndef __TRANSLITSWIITCHER_H__
#define __TRANSLITSWIITCHER_H__

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <win2k.h>
#include <m_icolib.h>
#include <m_popup.h>
#include <m_string.h>

#include <m_msg_buttonsbar.h>
#include <m_smileyadd.h>
#include <m_ieview.h>

#include "version.h"
#include "resource.h"

#define IDC_MESSAGE				1002
#define IDC_CHATMESSAGE			1009

VOID SwitchLayout(bool);
VOID TranslitLayout(bool);
VOID InvertCase(bool);
int OnButtonPressed(WPARAM wParam, LPARAM lParam);


#endif
