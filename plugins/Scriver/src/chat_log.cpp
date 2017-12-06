/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#include "stdafx.h"

#ifndef EM_GETSCROLLPOS
#define EM_GETSCROLLPOS	(WM_USER+221)
#endif

void CChatRoomDlg::StreamInEvents(LOGINFO* lin, bool bRedraw)
{
	if (m_hwnd == nullptr || lin == nullptr || m_si == nullptr)
		return;

	if (!bRedraw && (m_si->iType == GCW_CHATROOM || m_si->iType == GCW_PRIVMESS) && m_bFilterEnabled && !(m_iLogFilterFlags & lin->iType))
		return;

	LOGSTREAMDATA streamData;
	memset(&streamData, 0, sizeof(streamData));
	streamData.hwnd = m_log.GetHwnd();
	streamData.si = m_si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;
	streamData.isFirst = bRedraw ? 1 : m_log.GetRichTextLength() == 0;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = Srmm_LogStreamCallback;
	stream.dwCookie = (DWORD_PTR)&streamData;

	SCROLLINFO scroll;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(m_log.GetHwnd(), SB_VERT, &scroll);

	POINT point = { 0 };
	m_log.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	CHARRANGE oldsel, sel, newsel;
	m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		m_log.SendMsg(WM_SETREDRAW, FALSE, 0);

	// set the insertion point at the bottom
	sel.cpMin = sel.cpMax = m_log.GetRichTextLength();
	m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	// get the number of pixels per logical inch
	bool bFlag = false;
	if (bRedraw) {
		m_log.SendMsg(WM_SETREDRAW, FALSE, 0);
		bFlag = true;
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	m_log.SendMsg(EM_STREAMIN, wp, (LPARAM)&stream);

	// do smileys
	if (g_dat.smileyAddInstalled && (bRedraw || (lin->ptszText && lin->iType != GC_EVENT_JOIN && lin->iType != GC_EVENT_NICK && lin->iType != GC_EVENT_ADDSTATUS && lin->iType != GC_EVENT_REMOVESTATUS))) {
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = { sizeof(sm) };
		sm.hwndRichEditControl = m_log.GetHwnd();
		sm.Protocolname = m_si->pszModule;
		sm.rangeToReplace = bRedraw ? nullptr : &newsel;
		sm.flags = 0;
		sm.disableRedraw = TRUE;
		sm.hContact = m_hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if (bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50)
		ScrollToBottom();
	else
		m_log.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldsel);
		m_log.SendMsg(WM_SETREDRAW, TRUE, 0);
		InvalidateRect(m_log.GetHwnd(), nullptr, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = m_log.GetRichTextLength();
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		m_log.SendMsg(WM_SETREDRAW, TRUE, 0);
		InvalidateRect(m_log.GetHwnd(), nullptr, TRUE);
	}
}
