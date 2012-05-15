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

#include "common.h"
#include "message_pump.h"
#include "popwin.h"
#include "options.h"
#include "str_utils.h"
#include "subst.h"

BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD) = 0;
BOOL (WINAPI *MyUpdateLayeredWindow)(HWND hwnd, HDC hdcDST, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags) = 0;
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags) = 0;
HMONITOR (WINAPI *MyMonitorFromPoint)(POINT, DWORD);
BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR, LPMONITORINFO);
HRESULT (WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind) = 0; 

unsigned int uintMessagePumpThreadId = 0;
POINT pt = {-1};
UINT WaitForContentTimerID = 0;
bool bAvatarReady = false;
bool bStatusMsgReady = false;

__inline bool IsContactTooltip(CLCINFOTIPEX *clc)
{
	return (clc->szProto || clc->swzText) == false;
}

void CALLBACK TimerProcWaitForContent(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	KillTimer(0, WaitForContentTimerID);
	WaitForContentTimerID = 0;
	bStatusMsgReady = true;	
	bAvatarReady = true;
	PostMPMessage(MUM_CREATEPOPUP, 0, 0);
}

bool NeedWaitForContent(CLCINFOTIPEX *clcitex)
{
	bool bNeedWait = false;

	if (opt.bWaitForContent && IsContactTooltip(clcitex))
	{
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)clcitex->hItem, 0);
		if (!szProto) return false;

		if (opt.bWaitForStatusMsg && !bStatusMsgReady) 
		{
			DBDeleteContactSetting(clcitex->hItem, MODULE, "TempStatusMsg");
			if (CanRetrieveStatusMsg(clcitex->hItem, szProto) &&
				CallContactService(clcitex->hItem, PSS_GETAWAYMSG, 0, 0)) 
			{
				if (WaitForContentTimerID) 
					KillTimer(0, WaitForContentTimerID);

				WaitForContentTimerID = SetTimer(NULL, 0, WAIT_TIMER_INTERVAL, TimerProcWaitForContent);
				bNeedWait = true;
			}
		}

		if (opt.bWaitForAvatar && !bAvatarReady && 
			CallProtoService(szProto, PS_GETAVATARCAPS, AF_ENABLED, 0)) 
		{
			DBVARIANT dbv;
			if (!DBGetContactSettingString(clcitex->hItem, "ContactPhoto", "File", &dbv))
			{		
				if (!strstr(dbv.pszVal, ".xml")) 
				{
					AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)clcitex->hItem, 0);
					if (!ace) 
					{
						if (WaitForContentTimerID) 
							KillTimer(0, WaitForContentTimerID);

						WaitForContentTimerID = SetTimer(NULL, 0, WAIT_TIMER_INTERVAL, TimerProcWaitForContent);
						bNeedWait = true;
					} 
					else
						bAvatarReady = true;
				} 
				else
					bAvatarReady = true;

				DBFreeVariant(&dbv);
			} 
			else
				bAvatarReady = true;
		}
		
	} 

	return bNeedWait;
}

unsigned int CALLBACK MessagePumpThread(void *param) 
{
	HWND hwndTip = 0;
	CLCINFOTIPEX *clcitex = 0;

	MSG hwndMsg = {0};
	while (GetMessage(&hwndMsg, 0, 0, 0) > 0 && !Miranda_Terminated())
	{
		if (!IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) 
		{
			switch (hwndMsg.message) 
			{
				case MUM_CREATEPOPUP:
				{
					if (!clcitex) 
					{
						if (hwndMsg.lParam) clcitex = (CLCINFOTIPEX *)hwndMsg.lParam;
						else break;
					}

					if (!NeedWaitForContent(clcitex)) 
					{
						if (hwndTip) MyDestroyWindow(hwndTip);
						hwndTip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, NULL, WS_POPUP, 0, 0, 0, 0, 0, 0, hInst, (LPVOID)clcitex);
							
						if (clcitex)
						{
							mir_free(clcitex);
							clcitex = 0;
						}

						bStatusMsgReady = false;
						bAvatarReady = false;
					}
					break;
				}	
				case MUM_DELETEPOPUP:
				{
					if (hwndTip) 
					{
						MyDestroyWindow(hwndTip);
						hwndTip = 0;
					}

					if (clcitex)
					{
						mir_free(clcitex);
						clcitex = 0;
					}

					bStatusMsgReady = false;
					bAvatarReady = false;
					break;
				}
				case MUM_GOTSTATUS:
				{
					HANDLE hContact = (HANDLE)hwndMsg.wParam;
					TCHAR *swzMsg = (TCHAR *)hwndMsg.lParam;

					if (opt.bWaitForContent && 
						bStatusMsgReady == false && 
						clcitex && 
						clcitex->hItem == hContact)
					{
						if (WaitForContentTimerID) 
						{
							KillTimer(0, WaitForContentTimerID);
							WaitForContentTimerID = 0;
						}

						if (swzMsg)
						{
							DBWriteContactSettingTString(clcitex->hItem, MODULE, "TempStatusMsg", swzMsg);	
							mir_free(swzMsg);
						}

						bStatusMsgReady = true;
						PostMPMessage(MUM_CREATEPOPUP, 0, 0);
					} 
					else if (!opt.bWaitForContent && hwndTip) 
						SendMessage(hwndTip, PUM_SETSTATUSTEXT, (WPARAM)hContact, (LPARAM)swzMsg);
					else if (swzMsg) 
						mir_free(swzMsg);

					break;
				}	
				case MUM_GOTXSTATUS:
				{
					if (hwndTip && !opt.bWaitForContent) 
						SendMessage(hwndTip, PUM_SHOWXSTATUS, hwndMsg.wParam, 0);
					break;
				}
				case MUM_GOTAVATAR:
				{
					HANDLE hContact = (HANDLE)hwndMsg.wParam;

					if (opt.bWaitForContent && 
						bAvatarReady == false && 
						clcitex && 
						clcitex->hItem == hContact) 
					{
						if (WaitForContentTimerID)
						{
							KillTimer(0, WaitForContentTimerID);
							WaitForContentTimerID = 0;
						}

						bAvatarReady = true;		
						PostMPMessage(MUM_CREATEPOPUP, 0, 0);
					} 
					else if (!opt.bWaitForContent && hwndTip) 
						SendMessage(hwndTip, PUM_SETAVATAR, hwndMsg.wParam, 0);

					break;
				}
				default:
				{
					TranslateMessage(&hwndMsg);
					DispatchMessage(&hwndMsg);
					break;
				}
			}
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
	WNDCLASSEX wcl = {0};
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = PopupWindowProc;
	wcl.hInstance = hInst;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = POP_WIN_CLASS;
	RegisterClassEx(&wcl);

	HMODULE hUserDll = LoadLibrary(_T("user32.dll"));
	if (hUserDll) 
	{
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyUpdateLayeredWindow = (BOOL (WINAPI *)(HWND hwnd, HDC hdcDST, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags))GetProcAddress(hUserDll, "UpdateLayeredWindow");
		MyAnimateWindow =(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll, "AnimateWindow");
		MyMonitorFromPoint = (HMONITOR (WINAPI*)(POINT, DWORD))GetProcAddress(hUserDll, "MonitorFromPoint");
#ifdef _UNICODE
		MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR, LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoW");
#else
		MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR, LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoA");
#endif
	}

	HMODULE hDwmapiDll = LoadLibrary(_T("dwmapi.dll"));
	if (hDwmapiDll)
		MyDwmEnableBlurBehindWindow = (HRESULT (WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");

	CloseHandle(mir_forkthreadex(MessagePumpThread, NULL, 0, &uintMessagePumpThreadId));
}

void DeinitMessagePump() 
{
	PostMPMessage(WM_QUIT, 0, 0);
	UnregisterClass(POP_WIN_CLASS, hInst);
}

INT_PTR ShowTip(WPARAM wParam, LPARAM lParam) 
{
	CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	HWND clist = (HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0);

	if (clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if (clcit->isTreeFocused == 0 && opt.bShowNoFocus == false && clist == WindowFromPoint(clcit->ptCursor)) return 0;
	if (clcit->ptCursor.x == pt.x && clcit->ptCursor.y == pt.y) return 0;
	pt.x = pt.y = 0;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->szProto = NULL;
	clcit2->swzText = NULL;

	if (wParam) // wParam is char pointer containing text - e.g. status bar tooltip
	{ 
		clcit2->swzText = a2t((char *)wParam);
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
	HWND clist = (HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0);

	if (clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if (clcit->isTreeFocused == 0 && opt.bShowNoFocus == false && clist == WindowFromPoint(clcit->ptCursor)) return 0;
	if (clcit->ptCursor.x == pt.x && clcit->ptCursor.y == pt.y) return 0;
	pt.x = pt.y = -1;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->szProto = NULL;
	clcit2->swzText = NULL;

	if (wParam) // wParam is char pointer containing text - e.g. status bar tooltip
	{ 
		clcit2->swzText = mir_tstrdup((TCHAR *)wParam);
		GetCursorPos(&clcit2->ptCursor);
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
	return 1;
}

INT_PTR HideTip(WPARAM wParam, LPARAM lParam) 
{
	//CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
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

int ProtoAck(WPARAM wParam, LPARAM lParam) 
{
	DBVARIANT dbv;
	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->result != ACKRESULT_SUCCESS)
		return 0;

	if (ack->type == ACKTYPE_AWAYMSG) 
	{
		if (!DBGetContactSettingTString(ack->hContact, "CList", "StatusMsg", &dbv))
		{
			if (dbv.ptszVal[0]) 
				PostMPMessage(MUM_GOTSTATUS, (WPARAM)ack->hContact, (LPARAM)mir_tstrdup(dbv.ptszVal));

			DBFreeVariant(&dbv);
		}
	} 
	else if (ack->type == ICQACKTYPE_XSTATUS_RESPONSE)
	{
		PostMPMessage(MUM_GOTXSTATUS, (WPARAM)ack->hContact, 0);
	}

	return 0;
}

int AvatarChanged(WPARAM wParam, LPARAM lParam) 
{
	HANDLE hContact = (HANDLE)wParam;
	PostMPMessage(MUM_GOTAVATAR, (WPARAM)hContact, 0);
	return 0;
}

int FramesShowSBTip(WPARAM wParam, LPARAM lParam) 
{
	if (opt.bStatusBarTips)
	{
		char *szProto = (char *)wParam;

		CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)mir_alloc(sizeof(CLCINFOTIPEX));
		memset(clcit2, 0, sizeof(CLCINFOTIPEX));
		clcit2->cbSize = sizeof(CLCINFOTIPEX);
		clcit2->szProto= szProto; // assume static string
		GetCursorPos(&clcit2->ptCursor);

		PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
		return 1;
	}
	return 0;
}

int FramesHideSBTip(WPARAM wParam, LPARAM lParam) 
{
	if (opt.bStatusBarTips) 
	{
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