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

#ifndef _MESSAGE_PUMP_INC
#define _MESSAGE_PUMP_INC

#define  WAIT_TIMER_INTERVAL	500

#define MUM_CREATEPOPUP			(WM_USER + 0x011)
#define MUM_DELETEPOPUP			(WM_USER + 0x012)
#define MUM_GOTSTATUS			(WM_USER + 0x013)
#define MUM_GOTAVATAR			(WM_USER + 0x014)
#define MUM_GOTXSTATUS			(WM_USER + 0x015)

#define DWM_BB_ENABLE					0x00000001
#define DWM_BB_BLURREGION				0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED	0x00000004

struct DWM_BLURBEHIND
{
	uint32_t dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
};

extern HRESULT(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

void InitMessagePump();
void DeinitMessagePump();

INT_PTR ShowTip(WPARAM wParam, LPARAM lParam);
INT_PTR ShowTipW(WPARAM wParam, LPARAM lParam);
INT_PTR HideTip(WPARAM wParam, LPARAM lParam);

int ShowTipHook(WPARAM wParam, LPARAM lParam);
int HideTipHook(WPARAM wParam, LPARAM lParam);

int FramesShowSBTip(WPARAM wParam, LPARAM lParam);
int FramesHideSBTip(WPARAM wParam, LPARAM lParam);

int ProtoAck(WPARAM wParam, LPARAM lParam);
int AvatarChanged(WPARAM wParam, LPARAM lParam);

BOOL MyDestroyWindow(HWND hwnd);
void PostMPMessage(UINT msg, WPARAM, LPARAM);


#endif
