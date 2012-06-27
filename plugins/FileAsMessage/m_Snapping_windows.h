#ifndef SNAPPING_WINDOWS_H
#define SNAPPING_WINDOWS_H

/*
If you want to use SnappingWindows in you plugin you should add this code in WindowProc:

BOOL CALLBACK YouWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CallSnappingWindowProc(hwnd,msg,wParam,lParam);
	//
	switch (msg)
	{
		case:.....
		...........
	}
	return ......
}


*/

struct SnapWindowProc_t
{
	HWND hWnd;
	//
	SIZE m_szMoveOffset;
	WPARAM wParam;
	LPARAM lParam;
	//
	int Reserved1;
	int Reserved2;
	int Reserved3;
};

#define CallSnappingWindowProc(hwnd,nMessage,wParam,lParam) {static struct SnapWindowProc_t SnapInfo;\
								if ((nMessage == WM_MOVING) || (nMessage == WM_NCLBUTTONDOWN) || (nMessage == WM_SYSCOMMAND)){\
								SnapInfo.hWnd = hwnd;\
								SnapInfo.wParam = wParam;\
								SnapInfo.lParam = lParam;\
								CallService("Utils/SnapWindowProc",(WPARAM)&SnapInfo,nMessage);}}

#endif //SNAPPING_WINDOWS_H