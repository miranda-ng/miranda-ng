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

#include "stdafx.h"

#include <tchar.h>

HMODULE hDwmapiDll = nullptr;
HRESULT(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind) = nullptr;

unsigned int uintMessagePumpThreadId = 0;
POINT pt = { -1 };
UINT WaitForContentTimerID = 0;
bool bAvatarReady = false;
bool bStatusMsgReady = false;

__inline bool IsContactTooltip(CLCINFOTIPEX *clc)
{
	return !(clc->szProto || clc->swzText);
}

void CALLBACK TimerProcWaitForContent(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, WaitForContentTimerID);
	WaitForContentTimerID = 0;
	bStatusMsgReady = true;
	bAvatarReady = true;
	PostMPMessage(MUM_CREATEPOPUP, 0, 0);
}

bool NeedWaitForContent(CLCINFOTIPEX *clcitex)
{
	bool bNeedWait = false;
	if (opt.bWaitForContent && IsContactTooltip(clcitex)) {
		MCONTACT hContact = (DWORD_PTR)clcitex->hItem;
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (!szProto) return false;

		if (opt.bWaitForStatusMsg && !bStatusMsgReady) {
			g_plugin.delSetting(hContact, "TempStatusMsg");
			if (CanRetrieveStatusMsg(hContact, szProto) && CallContactService(hContact, PS_GETAWAYMSG, 0, 0)) {
				if (WaitForContentTimerID)
					KillTimer(nullptr, WaitForContentTimerID);

				WaitForContentTimerID = SetTimer(nullptr, 0, WAIT_TIMER_INTERVAL, TimerProcWaitForContent);
				bNeedWait = true;
			}
		}

		if (opt.bWaitForAvatar && !bAvatarReady &&
			CallProtoService(szProto, PS_GETAVATARCAPS, AF_ENABLED, 0)) {
			DBVARIANT dbv;
			if (!db_get_s(hContact, "ContactPhoto", "File", &dbv)) {
				if (!strstr(dbv.pszVal, ".xml")) {
					AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hContact, 0);
					if (!ace) {
						if (WaitForContentTimerID)
							KillTimer(nullptr, WaitForContentTimerID);

						WaitForContentTimerID = SetTimer(nullptr, 0, WAIT_TIMER_INTERVAL, TimerProcWaitForContent);
						bNeedWait = true;
					}
					else
						bAvatarReady = true;
				}
				else
					bAvatarReady = true;

				db_free(&dbv);
			}
			else
				bAvatarReady = true;
		}

	}

	return bNeedWait;
}

unsigned int CALLBACK MessagePumpThread(void*)
{
	Thread_SetName("TipperYM: MessagePumpThread");

	HWND hwndTip = nullptr;
	CLCINFOTIPEX *clcitex = nullptr;
	MSG hwndMsg = {};

	while (GetMessage(&hwndMsg, nullptr, 0, 0) > 0 && !Miranda_IsTerminated()) {
		if (hwndMsg.hwnd != nullptr && IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) /* Wine fix. */
			continue;
		switch (hwndMsg.message) {
		case MUM_CREATEPOPUP:
			if (!clcitex) {
				if (hwndMsg.lParam)
					clcitex = (CLCINFOTIPEX*)hwndMsg.lParam;
				else
					break;
			}

			if (!NeedWaitForContent(clcitex)) {
				if (hwndTip)
					MyDestroyWindow(hwndTip);
				hwndTip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), (LPVOID)clcitex);

				if (clcitex) {
					mir_free(clcitex);
					clcitex = nullptr;
				}

				bStatusMsgReady = false;
				bAvatarReady = false;
			}
			break;
		case MUM_DELETEPOPUP:
			if (hwndTip) {
				MyDestroyWindow(hwndTip);
				hwndTip = nullptr;
			}

			if (clcitex) {
				mir_free(clcitex);
				clcitex = nullptr;
			}

			bStatusMsgReady = false;
			bAvatarReady = false;
			break;
		case MUM_GOTSTATUS:
			{
				MCONTACT hContact = (MCONTACT)hwndMsg.wParam;
				wchar_t *swzMsg = (wchar_t *)hwndMsg.lParam;

				if (opt.bWaitForContent && !bStatusMsgReady && clcitex && clcitex->hItem == (HANDLE)hContact) {
					if (WaitForContentTimerID) {
						KillTimer(nullptr, WaitForContentTimerID);
						WaitForContentTimerID = 0;
					}

					if (swzMsg) {
						g_plugin.setWString((DWORD_PTR)clcitex->hItem, "TempStatusMsg", swzMsg);
						mir_free(swzMsg);
					}

					bStatusMsgReady = true;
					PostMPMessage(MUM_CREATEPOPUP, 0, 0);
				}
				else if (!opt.bWaitForContent && hwndTip)
					SendMessage(hwndTip, PUM_SETSTATUSTEXT, hContact, (LPARAM)swzMsg);
				else if (swzMsg)
					mir_free(swzMsg);
			}
			break;
		case MUM_GOTXSTATUS:
			if (hwndTip && !opt.bWaitForContent)
				SendMessage(hwndTip, PUM_SHOWXSTATUS, hwndMsg.wParam, 0);
			break;
		case MUM_GOTAVATAR:
			{
				MCONTACT hContact = (MCONTACT)hwndMsg.wParam;
				if (opt.bWaitForContent && !bAvatarReady && clcitex && clcitex->hItem == (HANDLE)hContact) {
					if (WaitForContentTimerID) {
						KillTimer(nullptr, WaitForContentTimerID);
						WaitForContentTimerID = 0;
					}

					bAvatarReady = true;
					PostMPMessage(MUM_CREATEPOPUP, 0, 0);
				}
				else if (!opt.bWaitForContent && hwndTip)
					SendMessage(hwndTip, PUM_SETAVATAR, hwndMsg.wParam, 0);
			}
			break;
		default:
			TranslateMessage(&hwndMsg);
			DispatchMessage(&hwndMsg);
			break;
		}
	}

	return 0;
}

void PostMPMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PostThreadMessage(uintMessagePumpThreadId, msg, wParam, lParam);
}

void InitMessagePump()
{
	WNDCLASSEX wcl = { 0 };
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = PopupWindowProc;
	wcl.hInstance = g_plugin.getInst();
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = POP_WIN_CLASS;
	RegisterClassEx(&wcl);

	hDwmapiDll = LoadLibrary(L"dwmapi.dll");
	if (hDwmapiDll)
		MyDwmEnableBlurBehindWindow = (HRESULT(WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");

	CloseHandle(mir_forkthreadex(MessagePumpThread, nullptr, &uintMessagePumpThreadId));
}

void DeinitMessagePump()
{
	PostMPMessage(WM_QUIT, 0, 0);
	UnregisterClass(POP_WIN_CLASS, g_plugin.getInst());
	FreeLibrary(hDwmapiDll);
}

INT_PTR ShowTip(WPARAM wParam, LPARAM lParam)
{
	CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	HWND clist = g_clistApi.hwndContactTree;

	if (clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if (clcit->isTreeFocused == 0 && !opt.bShowNoFocus && clist == WindowFromPoint(clcit->ptCursor)) return 0;
	if (clcit->ptCursor.x == pt.x && clcit->ptCursor.y == pt.y) return 0;
	pt.x = pt.y = 0;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->szProto = nullptr;
	clcit2->swzText = nullptr;

	if (wParam) // wParam is char pointer containing text - e.g. status bar tooltip
	{
		clcit2->swzText = mir_a2u((char *)wParam);
		GetCursorPos(&clcit2->ptCursor);
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
	return 1;
}

int ShowTipHook(WPARAM wParam, LPARAM lParam)
{
	ShowTip(wParam, lParam);
	return 0;
}

INT_PTR ShowTipW(WPARAM wParam, LPARAM lParam)
{
	CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	HWND clist = g_clistApi.hwndContactTree;

	if (clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if (clcit->isTreeFocused == 0 && !opt.bShowNoFocus && clist == WindowFromPoint(clcit->ptCursor)) return 0;
	if (clcit->ptCursor.x == pt.x && clcit->ptCursor.y == pt.y) return 0;
	pt.x = pt.y = -1;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->szProto = nullptr;
	clcit2->swzText = nullptr;

	if (wParam) // wParam is char pointer containing text - e.g. status bar tooltip
	{
		clcit2->swzText = mir_wstrdup((wchar_t *)wParam);
		GetCursorPos(&clcit2->ptCursor);
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
	return 1;
}

INT_PTR HideTip(WPARAM, LPARAM)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		return 0;

	GetCursorPos(&pt);
	PostMPMessage(MUM_DELETEPOPUP, 0, 0);
	return 1;
}

int HideTipHook(WPARAM wParam, LPARAM lParam)
{
	HideTip(wParam, lParam);
	return 0;
}

int ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if ((ack == nullptr) || (ack->result != ACKRESULT_SUCCESS))
		return 0;

	if (ack->type == ACKTYPE_AWAYMSG) {
		wchar_t *tszMsg = (wchar_t*)ack->lParam;
		if (mir_wstrlen(tszMsg))
			PostMPMessage(MUM_GOTSTATUS, (WPARAM)ack->hContact, (LPARAM)mir_wstrdup(tszMsg));
	}
	return 0;
}

int AvatarChanged(WPARAM hContact, LPARAM)
{
	PostMPMessage(MUM_GOTAVATAR, hContact, 0);
	return 0;
}

int FramesShowSBTip(WPARAM wParam, LPARAM)
{
	if (opt.bStatusBarTips) {
		char *szProto = (char *)wParam;

		CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
		memset(clcit2, 0, sizeof(CLCINFOTIPEX));
		clcit2->cbSize = sizeof(CLCINFOTIPEX);
		clcit2->szProto = szProto; // assume static string
		GetCursorPos(&clcit2->ptCursor);

		PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
		return 1;
	}
	return 0;
}

int FramesHideSBTip(WPARAM, LPARAM)
{
	if (opt.bStatusBarTips) {
		PostMPMessage(MUM_DELETEPOPUP, 0, 0);
		return 1;
	}
	return 0;
}

BOOL MyDestroyWindow(HWND hwnd)
{
	SendMessage(hwnd, PUM_FADEOUTWINDOW, 0, 0);
	return DestroyWindow(hwnd);
}
