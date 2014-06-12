#ifndef SNAPPING_WINDOWS_H
#define SNAPPING_WINDOWS_H

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

#define CallSnappingWindowProc(hwnd,nMessage,wParam,lParam) {static struct SnapWindowProc_t SnapInfo;														\
								if ((nMessage == WM_MOVING) || (nMessage == WM_NCLBUTTONDOWN) || (nMessage == WM_SYSCOMMAND) || (nMessage == WM_SIZING)){	\
								SnapInfo.hWnd = hwnd;																										\
								SnapInfo.wParam = wParam;																									\
								SnapInfo.lParam = lParam;																									\
								CallService("Utils/SnapWindowProc",(WPARAM)&SnapInfo,nMessage);																\
								if (nMessage == WM_SIZING) return 1;}}

#endif //SNAPPING_WINDOWS_H