/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_clui.h>
#include <m_fontservice.h>
#include <m_avatars.h>
#include <m_xstatus.h>
#include <m_icolib.h>
#include <m_cluiframes.h>
#include <win2k.h>

#include <m_proto_listeningto.h>
#include <m_listeningto.h>
#include <m_statusplugins.h>
#include <m_awaymsg.h>
#include <m_simplestatusmsg.h>
#include <m_smileyadd.h>
#include <m_mydetails.h>
#include <m_skin_eng.h>

#include "../utils/mir_smileys.h"
#include "../utils/mir_options.h"

#include "resource.h"
#include "Version.h"
#include "data.h"
#include "options.h"
#include "frame.h"

#define MODULE_NAME "MyDetails"

#define SETTING_FRAME_VISIBLE "FrameVisible"
#define SETTING_DEFAULT_NICK "DefaultNick"

extern HINSTANCE hInst;
extern bool g_bFramesExist, g_bAvsExist;

#define PS_GETMYNICKNAMEMAXLENGTH "/GetMyNicknameMaxLength"

INT_PTR PluginCommand_SetMyNicknameUI(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_SetMyNickname(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_GetMyNickname(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_SetMyAvatarUI(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_SetMyAvatar(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_GetMyAvatar(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_SetMyStatusMessageUI(WPARAM wParam, LPARAM lParam);
INT_PTR PluginCommand_CycleThroughtProtocols(WPARAM wParam, LPARAM lParam);

// Helper
static __inline int DRAW_TEXT(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol)
{
	if (!opts.replace_smileys)
		return DrawText(hDC, lpString, nCount, lpRect, uFormat);

	return Smileys_DrawText(hDC, lpString, nCount, lpRect, uFormat | (opts.resize_smileys ? DT_RESIZE_SMILEYS : 0),
		opts.use_contact_list_smileys ? "clist" : protocol, NULL);
}

#endif // __COMMONS_H__
