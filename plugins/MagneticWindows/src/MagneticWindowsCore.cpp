#include "MagneticWindowsCore.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////

TWorkingVariables Globals = {
	NULL,
	NULL,
	0,0,
	false,false
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

int Abs(int a) {
	return (a<0) ? -a : a;
}


PDockingWindow FindDockingWindow(HWND hWnd) {
	PDockingWindow i = Globals.WindowList;
	
	while (i != NULL) {
		if (i->hWnd == hWnd) return i;

		i = i->Next;
	}

	return NULL;
}


void DockWindowRect(HWND hWnd, bool Sizing, RECT& GivenRect, int SizingEdge, int MouseX = 0, int MouseY = 0) {
	POINT p;
	int W, H; 
	int XPos, YPos;
	int tmpXPos, tmpYPos;
	int tmpMouseX, tmpMouseY;

	PRectList ActRect;

	int diffX = Options.SnapWidth, diffY = Options.SnapWidth;

	RECT tmpRect = GivenRect;
	RECT frmRect = GivenRect;

	bool FoundX = false, FoundY = false;

	if (!Sizing) {
		GetCursorPos(&p);
		if (Globals.SnappedX) {
			tmpMouseX = p.x - tmpRect.left;
			OffsetRect(&tmpRect, tmpMouseX - MouseX, 0);
			OffsetRect(&GivenRect, tmpMouseX - MouseX, 0);
		} else {
			MouseX = p.x - tmpRect.left;
		}
		if (Globals.SnappedY) {
			tmpMouseY = p.y - tmpRect.top;
			OffsetRect(&tmpRect, 0, tmpMouseY - MouseY);
			OffsetRect(&GivenRect, 0, tmpMouseY - MouseY);
		} else {		
			MouseY = p.y - tmpRect.top;
		}
	}

	W = tmpRect.right - tmpRect.left;
	H = tmpRect.bottom - tmpRect.top;

	if (!Sizing) {
		ActRect = Globals.Rects;
		while (ActRect != NULL) {
			if ((tmpRect.left >= (ActRect->Rect.left - Options.SnapWidth)) &&
				(tmpRect.left <= (ActRect->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpRect.left - ActRect->Rect.left) < diffX))
			{			
				GivenRect.left = ActRect->Rect.left;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - ActRect->Rect.left);

				FoundX = true;
			} else 
				if ((ActRect != Globals.Rects) &&
				(tmpRect.left >= (ActRect->Rect.right - Options.SnapWidth)) &&
				(tmpRect.left <= (ActRect->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpRect.left - ActRect->Rect.right) < diffX))
			{			
				GivenRect.left = ActRect->Rect.right;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - ActRect->Rect.right);

				FoundX = true;
			} else
			if ((ActRect != Globals.Rects) &&
				(tmpRect.right >= (ActRect->Rect.left - Options.SnapWidth)) &&
				(tmpRect.right <= (ActRect->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpRect.right - ActRect->Rect.left) < diffX))
			{
				GivenRect.right = ActRect->Rect.left;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - ActRect->Rect.left);

				FoundX = true;
			} else
			if ((tmpRect.right >= (ActRect->Rect.right - Options.SnapWidth)) &&
				(tmpRect.right <= (ActRect->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpRect.right - ActRect->Rect.right) < diffX))
			{
				GivenRect.right = ActRect->Rect.right;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - ActRect->Rect.right);

				FoundX = true;
			}


			if ((tmpRect.top >= (ActRect->Rect.top - Options.SnapWidth)) &&
				(tmpRect.top <= (ActRect->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpRect.top - ActRect->Rect.top) < diffY))
			{
				GivenRect.top = ActRect->Rect.top;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - ActRect->Rect.top);

				FoundY = true;
			} else
			if ((ActRect != Globals.Rects) &&
				(tmpRect.top >= (ActRect->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.top <= (ActRect->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpRect.top - ActRect->Rect.bottom) < diffY))
			{
				GivenRect.top = ActRect->Rect.bottom;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - ActRect->Rect.bottom);

				FoundY = true;
			} else
			if ((ActRect != Globals.Rects) &&
				(tmpRect.bottom >= (ActRect->Rect.top - Options.SnapWidth)) &&
				(tmpRect.bottom <= (ActRect->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpRect.bottom - ActRect->Rect.top) < diffY))
			{
				GivenRect.bottom = ActRect->Rect.top;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - ActRect->Rect.top);

				FoundY = true;
			} else
			if ((tmpRect.bottom >= (ActRect->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.bottom <= (ActRect->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpRect.bottom - ActRect->Rect.bottom) < diffY))
			{
				GivenRect.bottom = ActRect->Rect.bottom;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - ActRect->Rect.bottom);

				FoundY = true;
			}

			ActRect = ActRect->Next;
		} //next rect

		Globals.SnappedX = FoundX;
		Globals.SnappedY = FoundY;
	}
	else //Sizing
	{
		if ((SizingEdge == WMSZ_LEFT) ||
			(SizingEdge == WMSZ_TOPLEFT) ||
			(SizingEdge == WMSZ_BOTTOMLEFT))
		{
			XPos = GivenRect.left;
		} else {
			XPos = GivenRect.right;
		}

		if ((SizingEdge == WMSZ_TOP) ||
			(SizingEdge == WMSZ_TOPLEFT) ||
			(SizingEdge == WMSZ_TOPRIGHT))
		{
			YPos = GivenRect.top;
		} else {
			YPos = GivenRect.bottom;
		}

		tmpXPos = XPos;
		tmpYPos = YPos;

		ActRect = Globals.Rects;
		while (ActRect != NULL) {
			if ((tmpXPos >= (ActRect->Rect.left - Options.SnapWidth)) &&
				(tmpXPos <= (ActRect->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpXPos - ActRect->Rect.left) < diffX))
			{			
				XPos = ActRect->Rect.left;

				diffX = Abs(tmpXPos - ActRect->Rect.left);
			} else
			if ((tmpXPos >= (ActRect->Rect.right - Options.SnapWidth)) &&
				(tmpXPos <= (ActRect->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < ActRect->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > ActRect->Rect.top) &&
				(Abs(tmpXPos - ActRect->Rect.right) < diffX))
			{
				XPos = ActRect->Rect.right;

				diffX = Abs(tmpXPos - ActRect->Rect.right);
			}

			if ((tmpYPos >= (ActRect->Rect.top - Options.SnapWidth)) &&
				(tmpYPos <= (ActRect->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpYPos - ActRect->Rect.top) < diffY))
			{
				YPos = ActRect->Rect.top;

				diffY = Abs(tmpYPos - ActRect->Rect.top);
			} else 
			if ((tmpYPos >= (ActRect->Rect.bottom - Options.SnapWidth)) &&
				(tmpYPos <= (ActRect->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < ActRect->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > ActRect->Rect.left) &&
				(Abs(tmpYPos - ActRect->Rect.bottom) < diffY))
			{
				YPos = ActRect->Rect.bottom;

				diffY = Abs(tmpYPos - ActRect->Rect.bottom);
			}

			ActRect = ActRect->Next;
		} //Next rect

		if ((SizingEdge == WMSZ_LEFT) ||
			(SizingEdge == WMSZ_TOPLEFT) ||
			(SizingEdge == WMSZ_BOTTOMLEFT))
		{
			GivenRect.left = XPos;
		} else {
			GivenRect.right = XPos;
		}
		if ((SizingEdge == WMSZ_TOP) ||
			(SizingEdge == WMSZ_TOPLEFT) ||
			(SizingEdge == WMSZ_TOPRIGHT))
		{
			GivenRect.top = YPos;
		} else {
			GivenRect.bottom = YPos;
		}
	}
}


void GetFrmRects(HWND ForWindow) {
	PDockingWindow i;
	PRectList Rect, l;

	Rect = Globals.Rects;
	while (Rect != NULL) {
		l = Rect;
		Rect = Rect->Next;
		free(l);
	}
	
	Rect = (PRectList)malloc(sizeof(TRectList));
	Rect->Next = NULL;
	Globals.Rects = Rect;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &(Rect->Rect), 0);
	i = Globals.WindowList;

	while (i != NULL) {
		if ((i->hWnd != ForWindow) && IsWindowVisible(i->hWnd)) {
			l = Rect;
			Rect = (PRectList)malloc(sizeof(TRectList));
			Rect->Next = NULL;
			l->Next = Rect;

			GetWindowRect(i->hWnd, &(Rect->Rect));			
		}

		i = i->Next;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Subclass Window Proc
///////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	RECT r;
	POINT p;

	PDockingWindow i = FindDockingWindow(hWnd);
	
	WNDPROC OldRun = NULL;

	if (i != NULL) {  //else we have a problem
		OldRun = i->OldWindowProc;

		if (Options.DoSnap) {
			switch (Msg) {
				case WM_ENTERSIZEMOVE: {
					if (Options.ScriverWorkAround)
						keybd_event(VK_CONTROL, 0, 0, 0);
	
					GetWindowRect(hWnd, &r);
					GetCursorPos(&p);
					Globals.MouseX = p.x - r.left;
					Globals.MouseY = p.y - r.top;
	
					GetFrmRects(hWnd);

					break;
				}
				case WM_EXITSIZEMOVE: {
					if (Options.ScriverWorkAround)
						keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

					break;
				}
				case WM_SIZING:
				case WM_MOVING: {	
					r = *((PRECT)lParam);
					if (Msg == WM_SIZING) {
						DockWindowRect(hWnd, true, r, wParam);
					} else {
						DockWindowRect(hWnd, false, r, wParam, Globals.MouseX, Globals.MouseY);
					}
	
					(*(PRECT)lParam) = r;

					if (Msg == WM_SIZING) {							
						return 1;
					}
	
					break;
				}
			} //switch
		} //if dosnap
			
		if (OldRun != NULL) {
			if (IsWindowUnicode(hWnd))  {
				return CallWindowProcW(OldRun, hWnd, Msg, wParam, lParam);
			} else {
				return CallWindowProcA(OldRun, hWnd, Msg, wParam, lParam);
			}
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// exportet Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

bool WindowOpen(HWND hWnd) {
	PDockingWindow i;

	if ((hWnd != 0) && (FindDockingWindow(hWnd) == NULL)) {
		i = (PDockingWindow)mir_alloc(sizeof(TDockingWindow));
		i->Next = Globals.WindowList;	
		i->hWnd = hWnd;
		Globals.WindowList = i;

		if (IsWindowUnicode(hWnd)) {
			i->OldWindowProc = (WNDPROC) GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)(&WindowProc));
		} else {
			i->OldWindowProc = (WNDPROC) GetWindowLongPtrA(hWnd, GWLP_WNDPROC);
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)(&WindowProc));
		}

		return true;
	}
	return false;
}

bool WindowClose(HWND hWnd) {
	PDockingWindow i, l;

	l = NULL;
	i = Globals.WindowList;

	while ((i != NULL) && (i->hWnd != hWnd)) {
		l = i;
		i = i->Next;
	}
	
	if (i != NULL) {
		if (l == NULL) {
			Globals.WindowList = i->Next;
		} else {
			l->Next = i->Next;
		}

		if (IsWindowUnicode(hWnd)) {
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR) (i->OldWindowProc));
		} else  {
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR) (i->OldWindowProc));
		}
		
		mir_free(i);

		return true;
	}

	return false;
}
bool WindowCloseAll() {
	PDockingWindow i, l;
	i = Globals.WindowList;
	while (i != NULL) {	
		l = i;
		WindowClose(i->hWnd);
		i = i->Next;
		mir_free(l);
	}
	return true;
}