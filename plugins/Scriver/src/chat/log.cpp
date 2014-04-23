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

#include "../commonheaders.h"

#ifndef EM_GETSCROLLPOS
#define EM_GETSCROLLPOS	(WM_USER+221)
#endif

static DWORD CALLBACK Log_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	LOGSTREAMDATA *lstrdat = (LOGSTREAMDATA*)dwCookie;
	if (lstrdat) {
		// create the RTF
		if (lstrdat->buffer == NULL) {
			lstrdat->bufferOffset = 0;
			lstrdat->buffer = pci->Log_CreateRTF(lstrdat);
			lstrdat->bufferLen = lstrlenA(lstrdat->buffer);
		}

		// give the RTF to the RE control
		*pcb = min(cb, lstrdat->bufferLen - lstrdat->bufferOffset);
		CopyMemory(pbBuff, lstrdat->buffer + lstrdat->bufferOffset, *pcb);
		lstrdat->bufferOffset += *pcb;

		// free stuff if the streaming operation is complete
		if (lstrdat->bufferOffset == lstrdat->bufferLen) {
			mir_free(lstrdat->buffer);
			lstrdat->buffer = NULL;
		}
	}

	return 0;
}

void Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO *si, BOOL bRedraw)
{
	if (hwndDlg == 0 || lin == 0 || si == 0)
		return;

	HWND hwndRich = GetDlgItem(hwndDlg, IDC_CHAT_LOG);

	LOGSTREAMDATA streamData;
	ZeroMemory(&streamData, sizeof(streamData));
	streamData.hwnd = hwndRich;
	streamData.si = si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;
	streamData.isFirst = bRedraw ? 1 : (GetRichTextLength(hwndRich, CP_ACP, FALSE) == 0);

	if (!bRedraw && (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS) && si->bFilterEnabled && !(si->iLogFilterFlags & lin->iType))
		return;

	BOOL bFlag = FALSE;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = Log_StreamCallback;
	stream.dwCookie = (DWORD_PTR)& streamData;

	SCROLLINFO scroll;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(hwndRich, SB_VERT, &scroll);

	POINT point = { 0 };
	SendMessage(hwndRich, EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	CHARRANGE oldsel, sel, newsel;
	SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);

	// set the insertion point at the bottom
	sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich, CP_ACP, FALSE);
	SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&sel);
	SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	// get the number of pixels per logical inch
	if (bRedraw) {
		SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);
		bFlag = TRUE;
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	SendMessage(hwndRich, EM_STREAMIN, wp, (LPARAM)&stream);

	// do smileys
	if (g_dat.smileyAddInstalled && (bRedraw || (lin->ptszText && lin->iType != GC_EVENT_JOIN 
		&& lin->iType != GC_EVENT_NICK && lin->iType != GC_EVENT_ADDSTATUS && lin->iType != GC_EVENT_REMOVESTATUS)))
	{
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = { sizeof(sm) };
		sm.hwndRichEditControl = hwndRich;
		sm.Protocolname = si->pszModule;
		sm.rangeToReplace = bRedraw ? NULL : &newsel;
		sm.flags = 0;
		sm.disableRedraw = TRUE;
		sm.hContact = si->hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if (bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50)
		SendMessage(GetParent(hwndRich), GC_SCROLLTOBOTTOM, 0, 0);
	else
		SendMessage(hwndRich, EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)& oldsel);
		SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwndRich, NULL, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich, CP_ACP, FALSE);
		SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)& sel);
		SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwndRich, NULL, TRUE);
	}
}
