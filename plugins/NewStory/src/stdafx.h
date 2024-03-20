/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#pragma warning(disable: 4458)

#define NOMINMAX

// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <tchar.h>
#include <commctrl.h>
#include <time.h>
#include <Shlwapi.h>
#include <malloc.h>

#include <map>

//Miranda headers
#include <newpluginapi.h>
#include <m_button.h>
#include <m_chat_int.h>
#include <m_clc.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_db_int.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_history.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_timezones.h>
#include <m_toptoolbar.h>
#include <m_userinfo.h>
#include <m_utils.h>

#include "m_NewStory.h"
#include "m_PluginUpdater.h"
#include "m_smileyadd.h"

#include "../../Libs/litehtml/include/cairo.h"
#include "../../Libs/litehtml/include/litehtml.h"
#include "../../Libs/litehtml/containers/windows/cairo/cairo_font.h"
#include "../../Libs/litehtml/containers/windows/cairo/windows_container.h"
#include "dib.h"

#include "resource.h"
#include "version.h"

#define MODULENAME "NewStory"
#define MODULETITLE "NewStory"

#include "utils.h"
#include "fonts.h"
#include "calendartool.h"
#include "history.h"
#include "history_array.h"
#include "history_control.h"
#include "templates.h"

void InitServices();

int OptionsInitialize(WPARAM, LPARAM);

enum
{
	HOTKEY_BOOKMARK = 1,
	HOTKEY_SEARCH = 2,
	HOTKEY_SEEK_FORWARD = 3,
	HOTKEY_SEEK_BACK = 4,
};

struct CMPlugin : public PLUGIN<CMPlugin>
{
	HANDLE m_log;
	HBRUSH hBackBrush;

	CMOption<bool> bOptVScroll, bSortAscending;

	// thesw options are a copy of static CMOption to keep performance high
	bool bMsgGrouping, bDrawEdge, bHppCompat, bDisableDelete = false;
	bool bShowType, bShowDirection;

	COLORREF clCustom[5];

	CMPlugin();

	void LoadOptions();

	int Load() override;
	int Unload() override;
};

extern CMOption<bool> g_bOptGrouping, g_bOptDrawEdge, g_bOptHppCompat, g_bShowType, g_bShowDirection;
extern CMOption<uint32_t> g_clCustom0, g_clCustom1, g_clCustom2, g_clCustom3, g_clCustom4;

extern wchar_t* months[12];
extern int g_iPixelY;
