#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////

struct TWindowData
{
	HWND hWnd;
	RECT Rect;
};

static LIST<TWindowData> arWindows(10, HandleKeySortT);

TWorkingVariables Globals =
{
	0, 0, false, false
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

int Abs(int a)
{
	return (a < 0) ? -a : a;
}

void DockWindowRect(HWND hWnd, bool Sizing, RECT &GivenRect, int SizingEdge, int MouseX = 0, int MouseY = 0)
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
		int i = 0; 
		for (auto &it : arWindows) {
			if (it->hWnd == hWnd)
				continue;

			if ((tmpRect.left >= (it->Rect.left - Options.SnapWidth)) &&
				(tmpRect.left <= (it->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpRect.left - it->Rect.left) < diffX))
			{
				GivenRect.left = it->Rect.left;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - it->Rect.left);

				FoundX = true;
			}
			else if (i != 0 &&
				(tmpRect.left >= (it->Rect.right - Options.SnapWidth)) &&
				(tmpRect.left <= (it->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpRect.left - it->Rect.right) < diffX))
			{
				GivenRect.left = it->Rect.right;
				GivenRect.right = GivenRect.left + W;

				diffX = Abs(tmpRect.left - it->Rect.right);

				FoundX = true;
			}
			else if (i != 0 &&
				(tmpRect.right >= (it->Rect.left - Options.SnapWidth)) &&
				(tmpRect.right <= (it->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpRect.right - it->Rect.left) < diffX))
			{
				GivenRect.right = it->Rect.left;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - it->Rect.left);

				FoundX = true;
			}
			else if ((tmpRect.right >= (it->Rect.right - Options.SnapWidth)) &&
				(tmpRect.right <= (it->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpRect.right - it->Rect.right) < diffX))
			{
				GivenRect.right = it->Rect.right;
				GivenRect.left = GivenRect.right - W;

				diffX = Abs(tmpRect.right - it->Rect.right);

				FoundX = true;
			}

			if ((tmpRect.top >= (it->Rect.top - Options.SnapWidth)) &&
				(tmpRect.top <= (it->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpRect.top - it->Rect.top) < diffY))
			{
				GivenRect.top = it->Rect.top;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - it->Rect.top);

				FoundY = true;
			}
			else if (i != 0 &&
				(tmpRect.top >= (it->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.top <= (it->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpRect.top - it->Rect.bottom) < diffY))
			{
				GivenRect.top = it->Rect.bottom;
				GivenRect.bottom = GivenRect.top + H;

				diffY = Abs(tmpRect.top - it->Rect.bottom);

				FoundY = true;
			}
			else if (i != 0 &&
				(tmpRect.bottom >= (it->Rect.top - Options.SnapWidth)) &&
				(tmpRect.bottom <= (it->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpRect.bottom - it->Rect.top) < diffY))
			{
				GivenRect.bottom = it->Rect.top;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - it->Rect.top);

				FoundY = true;
			}
			else if ((tmpRect.bottom >= (it->Rect.bottom - Options.SnapWidth)) &&
				(tmpRect.bottom <= (it->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpRect.bottom - it->Rect.bottom) < diffY)) {
				GivenRect.bottom = it->Rect.bottom;
				GivenRect.top = GivenRect.bottom - H;

				diffY = Abs(tmpRect.bottom - it->Rect.bottom);

				FoundY = true;
			}
			i++;
		}

		Globals.SnappedX = FoundX;
		Globals.SnappedY = FoundY;
	}
	else { // Sizing
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

		for (auto &it : arWindows) {
			if (it->hWnd == hWnd)
				continue;

			if ((tmpXPos >= (it->Rect.left - Options.SnapWidth)) &&
				(tmpXPos <= (it->Rect.left + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpXPos - it->Rect.left) < diffX))
			{
				XPos = it->Rect.left;
				diffX = Abs(tmpXPos - it->Rect.left);
			}
			else if ((tmpXPos >= (it->Rect.right - Options.SnapWidth)) &&
				(tmpXPos <= (it->Rect.right + Options.SnapWidth)) &&
				((tmpRect.top - Options.SnapWidth) < it->Rect.bottom) &&
				((tmpRect.bottom + Options.SnapWidth) > it->Rect.top) &&
				(Abs(tmpXPos - it->Rect.right) < diffX))
			{
				XPos = it->Rect.right;
				diffX = Abs(tmpXPos - it->Rect.right);
			}

			if ((tmpYPos >= (it->Rect.top - Options.SnapWidth)) &&
				(tmpYPos <= (it->Rect.top + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpYPos - it->Rect.top) < diffY))
			{
				YPos = it->Rect.top;
				diffY = Abs(tmpYPos - it->Rect.top);
			}
			else if ((tmpYPos >= (it->Rect.bottom - Options.SnapWidth)) &&
				(tmpYPos <= (it->Rect.bottom + Options.SnapWidth)) &&
				((tmpRect.left - Options.SnapWidth) < it->Rect.right) &&
				((tmpRect.right + Options.SnapWidth) > it->Rect.left) &&
				(Abs(tmpYPos - it->Rect.bottom) < diffY))
			{
				YPos = it->Rect.bottom;
				diffY = Abs(tmpYPos - it->Rect.bottom);
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

	for (int i = 1; i < arWindows.getCount(); i++) {
		TWindowData *it = arWindows[i];
		if (it->hWnd != ForWindow && IsWindowVisible(it->hWnd))
			GetWindowRect(it->hWnd, &it->Rect);
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
	for (auto &it : arWindows)
		mir_free(it);
	arWindows.destroy();
}
