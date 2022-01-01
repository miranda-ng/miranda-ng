/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

#undef FASTCALL

#define TSAPI __stdcall
#define FASTCALL __fastcall

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <Richedit.h>

#include <malloc.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_timezones.h>
#include <m_cluiframes.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_tipper.h>
#include <m_xstatus.h>
#include <m_extraicons.h>
#include <m_variables.h>

#include <m_metacontacts.h>
#include <m_cln_skinedit.h>

#include "resource.h"
#include "version.h"
#include "extbackg.h"
#include "clc.h"
#include "config.h"
#include "clist.h"
#include "alphablend.h"
#include "rowheight_funcs.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

// shared vars

extern LONG g_cxsmIcon, g_cysmIcon;
extern LIST<StatusItems_t> arStatusItems;
extern ImageItem *g_glyphItem;
extern HIMAGELIST hCListImages;

extern CLIST_INTERFACE coreCli;

typedef  int  (__cdecl *pfnDrawAvatar)(HDC hdcOrig, HDC hdcMem, RECT *rc, ClcContact *contact, int y, struct ClcData *dat, int selected, uint16_t cstatus, int rowHeight);

BOOL __forceinline GetItemByStatus(int status, StatusItems_t *retitem);

void DrawAlpha(HDC hdcwnd, PRECT rc, uint32_t basecolor, int alpha, uint32_t basecolor2, BOOL transparent, uint8_t FLG_GRADIENT, uint8_t FLG_CORNER, uint32_t BORDERSTYLE, ImageItem *item);

void CustomizeButton(HWND hWnd, bool bIsSkinned, bool bIsThemed, bool bIsFlat = false, bool bIsTTButton = false);

/////////////////////////////////////////////////////////////////////////////////////////
// Base class for Clist Nicer options

class CRowItemsBaseDlg : public CDlgBase
{
	void OnFinish(CDlgBase *)
	{
		Clist_ClcOptionsChanged();
		PostMessage(g_clistApi.hwndContactList, CLUIINTM_REDRAW, 0, 0);
	}

public:
	CRowItemsBaseDlg(int iDlg) :
		CDlgBase(g_plugin, iDlg)
	{
		m_OnFinishWizard = Callback(this, &CRowItemsBaseDlg::OnFinish);
	}
};
