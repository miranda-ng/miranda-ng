#include "MagneticWindowsCore.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////

struct TWindowData
{
	HWND hWnd;
	RECT Rect;
};

static LIST<TWindowData> arWindows(10, HandleKeySortT);

TWorkingVariables Globals = {
	0, 0,
	false, false
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

int Abs(int a) {
	return (a<0) ? -a : a;
}

void DockWindowRect(HWND hWnd, bool Sizing, RECT& GivenRect, int SizingEdge, int MouseX = 0, int MouseY = 0)
{
	POINT p;
	int XPos, YPos;
	int tmpXPos, tmpYPos;
	int tmpMouseX, tmpMouseY;

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
		}
		else MouseX = p.x - tmpRect.left;

		if (Globals.SnappedY) {
			tmpMouseY = p.y - tmpRect.top;
			OffsetRect(&tmpRect, 0, tmpMouseY - MouseY);
			OffsetRect(&GivenRect, 0, tmpMouseY - MouseY);
		}
		else MouseY = p.y - tmpRect.top;
	}

	int W = tmpRect.right - tmpRect.left;
	int H = tmpRect.bottom - tmpRect.top;

	if (!Sizing) {
		for (int i=0; i < arWindows.getCount(); i++) {
			TWindowData *p = arWindows[i];
			if (p->hWnd == hWnd)
				continue;

			if ((tmpRect.left >= (p->Rect.left - Options.SnapWidth)) &&
				(tmpRect.left <= (p->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpRect.left - p->Rect.left) < diffX))
			{			
				GivenRect.left = p->Rect.left;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - p->Rect.left);

				FoundX = true;
			}
			else if (i != 0 &&
				(tmpRect.left >= (p->Rect.right - Options.SnapWidth)) &&
				(tmpRect.left <= (p->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpRect.left - p->Rect.right) < diffX))
			{			
				GivenRect.left = p->Rect.right;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - p->Rect.right);

				FoundX = true;
			}
			else if (i != 0 &&
				(tmpRect.right >= (p->Rect.left - Options.SnapWidth)) &&
				(tmpRect.right <= (p->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpRect.right - p->Rect.left) < diffX))
			{
				GivenRect.right = p->Rect.left;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - p->Rect.left);

				FoundX = true;
			}
			else if ((tmpRect.right >= (p->Rect.right - Options.SnapWidth)) &&
				(tmpRect.right <= (p->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpRect.right - p->Rect.right) < diffX))
			{
				GivenRect.right = p->Rect.right;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - p->Rect.right);

				FoundX = true;
			}

			if ((tmpRect.top >= (p->Rect.top - Options.SnapWidth)) &&
				(tmpRect.top <= (p->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpRect.top - p->Rect.top) < diffY))
			{
				GivenRect.top = p->Rect.top;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - p->Rect.top);

				FoundY = true;
			}
			else if (i != 0 &&
				(tmpRect.top >= (p->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.top <= (p->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpRect.top - p->Rect.bottom) < diffY))
			{
				GivenRect.top = p->Rect.bottom;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - p->Rect.bottom);

				FoundY = true;
			}
			else if (i != 0 &&
				(tmpRect.bottom >= (p->Rect.top - Options.SnapWidth)) &&
				(tmpRect.bottom <= (p->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpRect.bottom - p->Rect.top) < diffY))
			{
				GivenRect.bottom = p->Rect.top;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - p->Rect.top);

				FoundY = true;
			}
			else if ((tmpRect.bottom >= (p->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.bottom <= (p->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpRect.bottom - p->Rect.bottom) < diffY))
			{
				GivenRect.bottom = p->Rect.bottom;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - p->Rect.bottom);

				FoundY = true;
			}
		}

		Globals.SnappedX = FoundX;
		Globals.SnappedY = FoundY;
	}
	else //Sizing
	{
		if (SizingEdge == WMSZ_LEFT || SizingEdge == WMSZ_TOPLEFT || SizingEdge == WMSZ_BOTTOMLEFT)
			XPos = GivenRect.left;
		else
			XPos = GivenRect.right;

		if (SizingEdge == WMSZ_TOP || SizingEdge == WMSZ_TOPLEFT || SizingEdge == WMSZ_TOPRIGHT)
			YPos = GivenRect.top;
		else
			YPos = GivenRect.bottom;

		tmpXPos = XPos;
		tmpYPos = YPos;

		for (int i=0; i < arWindows.getCount(); i++) {
			TWindowData *p = arWindows[i];
			if (p->hWnd == hWnd)
				continue;

			if ((tmpXPos >= (p->Rect.left - Options.SnapWidth)) &&
				(tmpXPos <= (p->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpXPos - p->Rect.left) < diffX))
			{			
				XPos = p->Rect.left;
				diffX = Abs(tmpXPos - p->Rect.left);
			}
			else if ((tmpXPos >= (p->Rect.right - Options.SnapWidth)) &&
				(tmpXPos <= (p->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < p->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > p->Rect.top) &&
				(Abs(tmpXPos - p->Rect.right) < diffX))
			{
				XPos = p->Rect.right;
				diffX = Abs(tmpXPos - p->Rect.right);
			}

			if ((tmpYPos >= (p->Rect.top - Options.SnapWidth)) &&
				(tmpYPos <= (p->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpYPos - p->Rect.top) < diffY))
			{
				YPos = p->Rect.top;
				diffY = Abs(tmpYPos - p->Rect.top);
			}
			else if ((tmpYPos >= (p->Rect.bottom - Options.SnapWidth)) &&
				(tmpYPos <= (p->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < p->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > p->Rect.left) &&
				(Abs(tmpYPos - p->Rect.bottom) < diffY))
			{
				YPos = p->Rect.bottom;
				diffY = Abs(tmpYPos - p->Rect.bottom);
			}
		}

		if (SizingEdge == WMSZ_LEFT || SizingEdge == WMSZ_TOPLEFT || SizingEdge == WMSZ_BOTTOMLEFT)
			GivenRect.left = XPos;
		else
			GivenRect.right = XPos;

		if (SizingEdge == WMSZ_TOP || SizingEdge == WMSZ_TOPLEFT || SizingEdge == WMSZ_TOPRIGHT)
			GivenRect.top = YPos;
		else
			GivenRect.bottom = YPos;
	}
}

void GetFrmRects(HWND ForWindow)
{
	SystemParametersInfo(SPI_GETWORKAREA, 0, &arWindows[0]->Rect, 0);

	for (int i=1; i < arWindows.getCount(); i++) {
		TWindowData *p = arWindows[i];
		if (p->hWnd != ForWindow && IsWindowVisible(p->hWnd))
			GetWindowRect(p->hWnd, &p->Rect);			
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Subclass Window Proc
///////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	POINT p;

	if (Options.DoSnap) {
		switch (Msg) {
		case WM_ENTERSIZEMOVE:
			if (Options.ScriverWorkAround)
				keybd_event(VK_CONTROL, 0, 0, 0);

			GetWindowRect(hWnd, &r);
			GetCursorPos(&p);
			Globals.MouseX = p.x - r.left;
			Globals.MouseY = p.y - r.top;
			GetFrmRects(hWnd);
			break;

		case WM_EXITSIZEMOVE:
			if (Options.ScriverWorkAround)
				keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
			break;

		case WM_SIZING:
		case WM_MOVING:
			r = *((PRECT)lParam);
			if (Msg == WM_SIZING)
				DockWindowRect(hWnd, true, r, wParam);
			else
				DockWindowRect(hWnd, false, r, wParam, Globals.MouseX, Globals.MouseY);

			(*(PRECT)lParam) = r;

			if (Msg == WM_SIZING)
				return 1;

			break;
		} 
	}

	return mir_callNextSubclass(hWnd, WindowProc, Msg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// exportet Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

void WindowStart()
{
	TWindowData *p = (TWindowData*)mir_calloc(sizeof(TWindowData));
	arWindows.insert(p);
}

bool WindowOpen(HWND hWnd)
{
	if (hWnd == 0)
		return false;

	TWindowData *p = (TWindowData*)mir_alloc(sizeof(TWindowData));
	p->hWnd = hWnd;
	GetWindowRect(hWnd, &p->Rect);
	arWindows.insert(p);

	mir_subclassWindow(hWnd, WindowProc);
	return true;
}

bool WindowClose(HWND hWnd)
{
	if (hWnd == 0)
		return false;

	mir_unsubclassWindow(hWnd, WindowProc);
	int idx = arWindows.indexOf((TWindowData*)&hWnd);
	if (idx != -1) {
		TWindowData *p = arWindows[idx];
		arWindows.remove(idx);
		mir_free(p);
	}
	return true;
}

void WindowCloseAll()
{
	for (int i=0; i < arWindows.getCount(); i++)
		mir_free(arWindows[i]);
	arWindows.destroy();
}
