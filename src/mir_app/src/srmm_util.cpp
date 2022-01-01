/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

#include "stdafx.h"
#include "chat.h"

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(DWORD) CALLBACK Srmm_LogStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	LOGSTREAMDATA *lstrdat = (LOGSTREAMDATA*)dwCookie;
	if (lstrdat) {
		// create the RTF
		if (lstrdat->buffer == nullptr) {
			lstrdat->bufferOffset = 0;
			lstrdat->buffer = g_chatApi.Log_CreateRTF(lstrdat);
			lstrdat->bufferLen = (int)mir_strlen(lstrdat->buffer);
		}

		// give the RTF to the RE control
		*pcb = min(cb, LONG(lstrdat->bufferLen - lstrdat->bufferOffset));
		memcpy(pbBuff, lstrdat->buffer + lstrdat->bufferOffset, *pcb);
		lstrdat->bufferOffset += *pcb;

		// free stuff if the streaming operation is complete
		if (lstrdat->bufferOffset == lstrdat->bufferLen) {
			mir_free(lstrdat->buffer);
			lstrdat->buffer = nullptr;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_GetWindowData(MCONTACT hContact, MessageWindowData &mwd)
{
	if (hContact == 0)
		return 1;

	HWND hwnd = WindowList_Find(g_hWindowList, hContact);
	if (hwnd == nullptr)
		return 1;

	mwd.hwndWindow = hwnd;
	mwd.pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	mwd.uState = MSG_WINDOW_STATE_EXISTS;
	if (IsWindowVisible(hwnd))
		mwd.uState |= MSG_WINDOW_STATE_VISIBLE;
	if (GetForegroundWindow() == hwnd)
		mwd.uState |= MSG_WINDOW_STATE_FOCUS;
	if (IsIconic(hwnd))
		mwd.uState |= MSG_WINDOW_STATE_ICONIC;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_Broadcast(UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(g_hWindowList, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HWND) Srmm_FindWindow(MCONTACT hContact)
{
	return WindowList_Find(g_hWindowList, hContact);
}

MIR_APP_DLL(CMsgDialog*) Srmm_FindDialog(MCONTACT hContact)
{
	HWND hwnd = Srmm_FindWindow(hContact);
	return (hwnd) ? (CMsgDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// serializes all thread-unsafe operation to the first thread

struct SSTParam
{
	HWND hwnd;
	const wchar_t *wszText;
	HICON hIcon;
};

static INT_PTR CALLBACK sttSetStatusText(void *_p)
{
	SSTParam *param = (SSTParam*)_p;

	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(param->hwnd, GWLP_USERDATA);
	if (pDlg != nullptr)
		pDlg->SetStatusText(param->wszText, param->hIcon);
	return 0;
}

MIR_APP_DLL(void) Srmm_SetStatusText(MCONTACT hContact, const wchar_t *wszText, HICON hIcon)
{
	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd == nullptr)
		hwnd = Srmm_FindWindow(db_mc_getMeta(hContact));
	if (hwnd == nullptr)
		return;

	SSTParam param = { hwnd, wszText, hIcon };
	CallFunctionSync(sttSetStatusText, &param);
}
