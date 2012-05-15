/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
#include "commonheaders.h"

static LRESULT CALLBACK ColourPickerWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
		case WM_CREATE:
			SetWindowLongPtr(hwnd,0,0);
			SetWindowLongPtr(hwnd,sizeof(COLORREF),0);
			break;
		case CPM_SETDEFAULTCOLOUR:
			SetWindowLongPtr(hwnd,sizeof(COLORREF),lParam);
			break;
		case CPM_GETDEFAULTCOLOUR:
			return GetWindowLongPtr(hwnd,sizeof(COLORREF));
		case CPM_SETCOLOUR:
			SetWindowLongPtr(hwnd,0,lParam);
			InvalidateRect(hwnd,NULL,FALSE);
			break;
		case CPM_GETCOLOUR:
			return GetWindowLongPtr(hwnd,0);
		case WM_LBUTTONUP:
		{
            CHOOSECOLOR cc={0};
            COLORREF custColours[16]={0};
			custColours[0]=GetWindowLongPtr(hwnd,sizeof(COLORREF));
            cc.lStructSize=sizeof(CHOOSECOLOR);
            cc.hwndOwner=hwnd;
            cc.hInstance=(HWND)hMirandaInst;
            cc.rgbResult=GetWindowLongPtr(hwnd,0);
            cc.lpCustColors=custColours;
            cc.Flags=CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
            if(ChooseColor(&cc)) {
				SetWindowLongPtr(hwnd,0,cc.rgbResult);
				SendMessage(GetParent(hwnd),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(hwnd),CPN_COLOURCHANGED),(LPARAM)hwnd);
				InvalidateRect(hwnd,NULL,FALSE);
			}
			break;
		}
		case WM_ENABLE:
			InvalidateRect(hwnd,NULL,FALSE);
			break;
		case WM_NCPAINT:
		case WM_PAINT:
		{	PAINTSTRUCT ps;
			HDC hdc1;
			RECT rc;
			HBRUSH hBrush;

			hdc1=BeginPaint(hwnd,&ps);
			GetClientRect(hwnd,&rc);
			DrawEdge(hdc1,&rc,EDGE_ETCHED,BF_RECT);
			InflateRect(&rc,-2,-2);
			if(IsWindowEnabled(hwnd))
				hBrush=CreateSolidBrush(GetWindowLongPtr(hwnd,0));
			else
				hBrush=CreateHatchBrush(HS_BDIAGONAL,GetSysColor(COLOR_GRAYTEXT));
			SetBkColor(hdc1,GetSysColor(COLOR_BTNFACE));
			FillRect(hdc1,&rc,hBrush);
			DeleteObject(hBrush);
			EndPaint(hwnd,&ps);
			break;
		}
		case WM_DESTROY:
			break;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

int InitColourPicker(void)
{
	WNDCLASS wcl;

	wcl.lpfnWndProc=ColourPickerWndProc;
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=sizeof(COLORREF)*2;
	wcl.hInstance=hMirandaInst;
	wcl.hCursor=NULL;
	wcl.lpszClassName=WNDCLASS_COLOURPICKER;
	wcl.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
	wcl.hIcon=NULL;
	wcl.lpszMenuName=NULL;
	wcl.style=CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS;
	RegisterClass(&wcl);
	return 0;
}
