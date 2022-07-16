/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

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

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <windowsx.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_awaymsg.h>
#include <m_clistint.h>
#include <m_clc.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_timezones.h>
#include <m_xstatus.h>

#include <m_tipper.h>
#include <m_fingerprint.h>
#include <m_flags.h>
#include <m_folders.h>
#include <m_metacontacts.h>
#include <m_variables.h>
#include <m_smileyadd.h>
#include <m_imgsrvc.h>

#include "resource.h"
#include "version.h"
#include "message_pump.h"
#include "mir_smileys.h"
#include "popwin.h"
#include "skin_parser.h"
#include "bitmap_func.h"
#include "options.h"
#include "str_utils.h"
#include "subst.h"
#include "preset_items.h"
#include "translations.h"

#define MODULENAME "Tipper"
#define MODULE_ITEMS "Tipper_Items"
#define DEFAULT_SKIN_FOLDER L"Skins\\Tipper"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern HFONT hFontTitle, hFontLabels, hFontValues, hFontTrayTitle;
extern COLORREF	colTitle, colLabels, colBg, colValues;

extern int iCodePage;

extern TOOLTIPSKIN skin;
extern wchar_t SKIN_FOLDER[256];

extern int ReloadFont(WPARAM wParam, LPARAM lParam);
extern IconItem iconList[];
