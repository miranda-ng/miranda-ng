/*
Copyright (C) 2012 Mataes

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

//Windows headers
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <tchar.h>
#include <commctrl.h>
#include <time.h>
#include <Shlwapi.h>
#include <malloc.h>

//Miranda headers
#include "newpluginapi.h"
#include "m_chat_int.h"
#include "m_clc.h"
#include "m_clistint.h"
#include "m_file.h"
#include "m_options.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_database.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_utils.h"
#include "m_history.h"
#include "m_button.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_icolib.h"
#include "m_fontservice.h"
#include "m_text.h"
#include "m_contacts.h"
#include "m_srmm_int.h"
#include <m_json.h>
#include <m_metacontacts.h>
#include <m_timezones.h>

#include "m_NewStory.h"
#include "m_smileyadd.h"
#ifndef MTEXT_NOHELPERS
#define MTEXT_NOHELPERS
#endif // MTEXT_NOHELPERS
#include "m_text.h"

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

int OptionsInitialize(WPARAM, LPARAM);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	HANDLE m_log;

	CMOption<bool> bOptVScroll;
	bool bMsgGrouping, bDrawEdge; // thesw options are a copy of static CMOption to keep performance high

	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern CMOption<bool> g_bOptGrouping, g_bOptDrawEdge;
extern wchar_t* months[12];
