/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

#define SB_MYMOVE 20

//
// SmileyToolwindowType
//
class SmileyToolWindowType
{
private:
	unsigned m_NumberOfVerticalButtons;
	unsigned m_NumberOfHorizontalButtons;
	SIZE m_BitmapWidth;
	SIZE m_ButtonSize;
	unsigned m_ButtonSpace;
	unsigned m_NumberOfButtons;
	int m_WindowSizeY;

	HWND m_hwndDialog;
	HWND m_hToolTip;
	HWND m_hWndTarget;
	SmileyPackType* m_pSmileyPack;
	int m_CurrentHotTrack;
	int m_XPosition;
	int m_YPosition;
	int m_Direction;
	UINT m_TargetMessage;
	WPARAM m_TargetWParam;
	MCONTACT m_hContact;
	int rowSel;
	bool m_Choosing;

	AnimatedPack* m_AniPack;

	void InitDialog(LPARAM lParam);
	void PaintWindow(void);
	void InsertSmiley(void);
	void MouseMove(int x, int y);
	void KeyUp(WPARAM wParam, LPARAM lParam);
	void SmileySel(int but);
	void ScrollV(int action, int dist = 0);

	int GetRowSize(void) const { return m_ButtonSize.cy + m_ButtonSpace; }

	void CreateSmileyBitmap(HDC hdc);
	void CreateSmileyWinDim(void);
	RECT CalculateButtonToCoordinates(int buttonPosition, int scroll);
	int CalculateCoordinatesToButton(POINT pt, int scroll);

public:
	SmileyToolWindowType(HWND hWnd);
	LRESULT DialogProcedure(UINT msg, WPARAM wParam, LPARAM lParam);
};


LRESULT CALLBACK DlgProcSmileyToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SmileyToolWindowType *pOD = (SmileyToolWindowType*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (pOD == NULL) {
		pOD = new SmileyToolWindowType(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pOD);
	}

	LRESULT Result = pOD->DialogProcedure(msg, wParam, lParam);

	if (msg == WM_NCDESTROY) {
		delete pOD;
		Result = FALSE;
	}

	return Result;
}


SmileyToolWindowType::SmileyToolWindowType(HWND hWnd) 
{ 
	m_hwndDialog = hWnd; 
	rowSel = -1; 
	m_AniPack = NULL; 

	m_NumberOfVerticalButtons = 0;
	m_NumberOfHorizontalButtons = 0;
	m_BitmapWidth.cx = 0;
	m_BitmapWidth.cy = 0;
	m_ButtonSize.cx = 0;
	m_ButtonSize.cy = 0;
	m_ButtonSpace = 1;
	m_NumberOfButtons = 0;
}


LRESULT SmileyToolWindowType::DialogProcedure(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = FALSE;

	switch (msg) {
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE) 
			DestroyWindow(m_hwndDialog);
		break;

	case WM_PAINT:
		PaintWindow();
		break;

	case WM_TIMER:
		if (m_AniPack) m_AniPack->ProcessTimerTick(m_hwndDialog);
		break;

	case WM_DESTROY:
		KillTimer(m_hwndDialog, 1);
		if (m_AniPack) delete m_AniPack;
		m_AniPack = NULL;
		DestroyWindow(m_hToolTip);
		PostQuitMessage(0);
		if (m_Choosing) 
			SetFocus(m_hWndTarget);
		break;

	case WM_KEYUP:
		KeyUp(wParam, lParam);
		break;

	case WM_CREATE:
		InitDialog(lParam);
		break;

	case WM_VSCROLL:
		ScrollV(LOWORD(wParam));
		break;

	case WM_MOUSEMOVE:
		MouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONUP:
		InsertSmiley();
		break;

	case WM_MOUSEWHEEL:
		ScrollV(SB_MYMOVE, ((short)HIWORD(wParam))/-120);
		MouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	default:
		Result = DefWindowProc(m_hwndDialog, msg, wParam, lParam);
		break;
	}

	return Result;
}

struct smlsrvstruct
{
	smlsrvstruct(SmileyType *tsml, MCONTACT thContact)
		: sml(tsml), hContact(thContact) {}
	SmileyType *sml;
	MCONTACT hContact;
};

void CALLBACK smileyServiceCallback(void* arg)
{
	smlsrvstruct* p = (smlsrvstruct*)arg;
	p->sml->CallSmileyService(p->hContact);
	delete p;
}

void SmileyToolWindowType::InsertSmiley(void)
{
	if (m_CurrentHotTrack >= 0 && m_hWndTarget != NULL) {
		SmileyType *sml = m_pSmileyPack->GetSmiley(m_CurrentHotTrack);

		if (sml->IsService()) {
			smlsrvstruct* p = new smlsrvstruct(sml, m_hContact);
			CallFunctionAsync(smileyServiceCallback, p);
		}
		else {
			CMString insertText;

			if (opt.SurroundSmileyWithSpaces) insertText = ' ';
			insertText += sml->GetInsertText();
			if (opt.SurroundSmileyWithSpaces) insertText += ' ';

			SendMessage(m_hWndTarget, m_TargetMessage, m_TargetWParam, (LPARAM) insertText.c_str());
		}
		m_Choosing = true;
		DestroyWindow(m_hwndDialog);
	}
	else if (m_hWndTarget == NULL)
		DestroyWindow(m_hwndDialog);
}

void SmileyToolWindowType::SmileySel(int but)
{
	if (but != m_CurrentHotTrack) {
		SCROLLINFO si; 
		si.cbSize = sizeof (si);
		si.fMask  = SIF_POS;
		si.nPos = 0;
		GetScrollInfo (m_hwndDialog, SB_VERT, &si);

		HDC hdc = GetDC(m_hwndDialog);
		if (m_CurrentHotTrack >= 0) {
			RECT rect = CalculateButtonToCoordinates(m_CurrentHotTrack, si.nPos);
			DrawFocusRect(hdc, &rect);
			m_CurrentHotTrack = -1;
			SendMessage(m_hToolTip, TTM_ACTIVATE, FALSE, 0);
		}
		m_CurrentHotTrack = but;
		if (m_CurrentHotTrack >= 0) {
			TOOLINFO ti = {0};
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			ti.hwnd = m_hwndDialog;
			ti.uId = (UINT_PTR)m_hwndDialog;

			const CMString& toolText = m_pSmileyPack->GetSmiley(m_CurrentHotTrack)->GetToolText();
			ti.lpszText = const_cast<TCHAR*>(toolText.c_str()); 
			SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
			SendMessage(m_hToolTip, TTM_ACTIVATE, TRUE, 0);

			RECT rect = CalculateButtonToCoordinates(m_CurrentHotTrack, si.nPos);
			DrawFocusRect(hdc, &rect);
			if (m_AniPack) m_AniPack->SetSel(rect);
		}
		ReleaseDC(m_hwndDialog, hdc);
	}
}


void SmileyToolWindowType::ScrollV(int action, int dist)
{ 
	SCROLLINFO si; 
	si.cbSize = sizeof (si);
	si.fMask  = SIF_ALL;
	GetScrollInfo (m_hwndDialog, SB_VERT, &si);

	// Save the position for comparison later on
	int yPos = si.nPos;
	switch (action) {
	// user clicked the HOME keyboard key
	case SB_TOP:
		si.nPos = si.nMin;
		break;

	// user clicked the END keyboard key
	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;

	// user clicked the top arrow
	case SB_LINEUP:
		si.nPos -= 1;
		break;

	// user clicked the bottom arrow
	case SB_LINEDOWN:
		si.nPos += 1;
		break;

	// user clicked the scroll bar shaft above the scroll box
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;

	// user clicked the scroll bar shaft below the scroll box
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;

	// user dragged the scroll box
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;

	// user dragged the scroll box
	case SB_MYMOVE:
		si.nPos += dist;
		break;
	}
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	si.fMask = SIF_POS;
	SetScrollInfo (m_hwndDialog, SB_VERT, &si, TRUE);
	GetScrollInfo (m_hwndDialog, SB_VERT, &si);
	// If the position has changed, scroll window and update it
	if (si.nPos != yPos) {                    
		if (m_AniPack)
			m_AniPack->SetOffset(si.nPos*GetRowSize());

		ScrollWindowEx(m_hwndDialog, 0, (yPos - si.nPos) * GetRowSize(), 
			NULL, NULL, NULL, NULL, SW_INVALIDATE);

		UpdateWindow (m_hwndDialog);
	}
} 


void SmileyToolWindowType::MouseMove(int xposition, int yposition)
{
	if (m_CurrentHotTrack == -2) return; //prevent focussing when not drawn yet!
	//    SetFocus(m_hwndDialog);


	SCROLLINFO si; 
	si.cbSize = sizeof (si);
	si.fMask  = SIF_POS;
	si.nPos = 0;
	GetScrollInfo (m_hwndDialog, SB_VERT, &si);

	POINT pt = { xposition, yposition };  
	int but = CalculateCoordinatesToButton(pt, si.nPos);
	SmileySel(but);
}



void SmileyToolWindowType::KeyUp(WPARAM wParam, LPARAM lParam)
{
	int colSel = -1, numKey = -1;
	int but = m_CurrentHotTrack;

	switch(wParam)
	{
	case VK_END:
		but = m_NumberOfButtons-1;
		break;

	case VK_HOME:
		but = 0;
		break;

	case VK_LEFT:
		but -= (opt.IEViewStyle ? 1 : m_NumberOfVerticalButtons) * LOWORD(lParam);
		break;

	case VK_UP:
		but -= (opt.IEViewStyle ? m_NumberOfHorizontalButtons : 1) * LOWORD(lParam);
		break;

	case VK_RIGHT:
		but += (opt.IEViewStyle ? 1 : m_NumberOfVerticalButtons) * LOWORD(lParam);
		break;

	case VK_DOWN:
		but += (opt.IEViewStyle ? m_NumberOfHorizontalButtons : 1) * LOWORD(lParam);
		break;

	case VK_SPACE:
	case VK_RETURN:
		if (but != -1) InsertSmiley();
		return;

	case VK_ESCAPE:
		DestroyWindow(m_hwndDialog);
		return;

	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
		if ((GetKeyState(VK_NUMLOCK) & 1) != 0)  
			numKey = (int)wParam - VK_NUMPAD1;
		else 
		{
			rowSel = -1;
			return;
		}
		break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		numKey = (int)wParam - '1';
		break;

	default:
		rowSel = -1;
		return;
	}

	if (numKey != -1) {
		if (rowSel == -1) { 
			rowSel = numKey;
			but = (opt.IEViewStyle ? m_NumberOfHorizontalButtons : 1) * rowSel;
		}
		else {
			colSel = numKey;
			if (opt.IEViewStyle)
				but = colSel + m_NumberOfHorizontalButtons * rowSel;
			else
				but = rowSel + m_NumberOfVerticalButtons * colSel;
		}
	}

	if (but < 0) but = 0;
	if (but >= (int)m_NumberOfButtons) but = m_NumberOfButtons-1;

	SmileySel(but);
	if (colSel != -1)
		InsertSmiley();
}


void SmileyToolWindowType::InitDialog(LPARAM lParam)
{
	LPCREATESTRUCT createStruct = (LPCREATESTRUCT)lParam;
	SmileyToolWindowParam* stwp = (SmileyToolWindowParam*) createStruct->lpCreateParams;

	m_pSmileyPack = stwp->pSmileyPack;
	m_XPosition = stwp->xPosition;
	m_YPosition = stwp->yPosition;
	m_hWndTarget = stwp->hWndTarget;
	m_TargetMessage = stwp->targetMessage;
	m_TargetWParam = stwp->targetWParam;
	m_Direction = stwp->direction;
	m_hContact = stwp->hContact;

	m_CurrentHotTrack = -2;
	m_Choosing = false;

	CreateSmileyWinDim();

	int width = m_BitmapWidth.cx;
	int height =  m_BitmapWidth.cy;

	const int colsz = GetRowSize(); 
	const int heightn = m_WindowSizeY;

	SCROLLINFO si;

	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nMax   = height / colsz - 1; 
	si.nPage  = heightn / colsz; 
	si.nPos   = 0; 
	SetScrollInfo(m_hwndDialog, SB_VERT, &si, TRUE); 

	if (GetWindowLongPtr(m_hwndDialog, GWL_STYLE) & WS_VSCROLL)
		width += GetSystemMetrics(SM_CXVSCROLL);

	RECT rc = { 0, 0, width, heightn };
	AdjustWindowRectEx(&rc, GetWindowLongPtr(m_hwndDialog, GWL_STYLE), 
		FALSE, GetWindowLongPtr(m_hwndDialog, GWL_EXSTYLE));

	width = rc.right - rc.left;
	height =  rc.bottom - rc.top;

	switch (m_Direction) {
	case 1: 
		m_XPosition-=width;
		break;
	case 2:
		m_XPosition-=width;
		m_YPosition-=height;
		break;
	case 3:
		m_YPosition-=height;
		break;
	}

	// Get screen dimentions
	int xoScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int yoScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);

	int xScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int yScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	if (xScreen == 0) xScreen = GetSystemMetrics(SM_CXSCREEN);
	if (yScreen == 0) yScreen = GetSystemMetrics(SM_CYSCREEN);

	xScreen += xoScreen;
	yScreen += yoScreen;

	// Prevent window from opening off-screen
	if (m_YPosition + height > yScreen) m_YPosition = yScreen - height;
	if (m_XPosition + width  > xScreen) m_XPosition = xScreen - width;
	if (m_YPosition < yoScreen) m_YPosition = yoScreen;
	if (m_XPosition < xoScreen) m_XPosition = xoScreen;

	// Move window to desired location
	SetWindowPos(m_hwndDialog, NULL, m_XPosition, m_YPosition, 
		width, height, SWP_NOZORDER);

	m_AniPack = new AnimatedPack(m_hwndDialog, height, m_ButtonSize, opt.SelWndBkgClr);

	SmileyPackType::SmileyVectorType &sml = m_pSmileyPack->GetSmileyList();
	for (unsigned i=0; i<m_NumberOfButtons; i++) 
		if (!sml[i].IsHidden())
			m_AniPack->Add(&sml[i], CalculateButtonToCoordinates(i, 0), opt.IEViewStyle);

	m_AniPack->SetOffset(0);

	if (opt.AnimateSel) SetTimer(m_hwndDialog, 1, 100, NULL);

	//add tooltips
	m_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), 
		TTS_NOPREFIX | WS_POPUP, 0, 0, 0, 0, m_hwndDialog, NULL, g_hInst, NULL);
	TOOLINFO ti = {0};
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = m_hwndDialog;
	ti.uId = (UINT_PTR)m_hwndDialog;
	ti.lpszText = TranslateT("d'Oh!");
	SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}


void SmileyToolWindowType::PaintWindow(void)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS;
	si.nPos = 0;
	GetScrollInfo(m_hwndDialog, SB_VERT, &si);

	PAINTSTRUCT ps; 
	HDC hdc = BeginPaint(m_hwndDialog, &ps); 

	HBITMAP hBmp = CreateCompatibleBitmap(hdc, m_BitmapWidth.cx, m_BitmapWidth.cy);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HANDLE hOld = SelectObject(hdcMem, hBmp);

	CreateSmileyBitmap(hdcMem);

	if (m_AniPack)
		m_AniPack->Draw(hdcMem);

	BitBlt(hdc, 0, 0, m_BitmapWidth.cx, m_WindowSizeY, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hOld);    
	DeleteObject(hBmp);	
	DeleteDC(hdcMem);

	if (m_CurrentHotTrack == -2)
		m_CurrentHotTrack = -1;

	EndPaint(m_hwndDialog, &ps); 
}


void SmileyToolWindowType::CreateSmileyWinDim(void)
{
	m_NumberOfButtons = m_pSmileyPack->VisibleSmileyCount();

	if (m_NumberOfButtons == 0) return;

	// Find largest smiley
	if (m_pSmileyPack->selec.x == 0 || m_pSmileyPack->selec.y == 0)
	{
		if (opt.ScaleAllSmileys)
		{
			m_pSmileyPack->GetSmiley(0)->GetSize(m_ButtonSize);
			++m_ButtonSize.cx; ++m_ButtonSize.cy; 
		}
		else
		{
			m_ButtonSize.cx = 0;
			m_ButtonSize.cy = 0;
			SmileyPackType::SmileyVectorType &sml = m_pSmileyPack->GetSmileyList();
			for (unsigned i=0; i<m_NumberOfButtons; i++)
			{
				SIZE smsz;
				sml[i].GetSize(smsz);

				if (m_ButtonSize.cx < smsz.cx) m_ButtonSize.cx = smsz.cx;
				if (m_ButtonSize.cy < smsz.cy) m_ButtonSize.cy = smsz.cy;
			}
		}
	}
	else
	{
		m_ButtonSize = *(SIZE*)&m_pSmileyPack->selec;
	}

	if (m_pSmileyPack->win.x == 0 || m_pSmileyPack->win.y == 0)
	{
		if (opt.IEViewStyle)
		{
			// All integer square root
			unsigned i;
			for (i=1; i*i<m_NumberOfButtons; i++) ;
			m_NumberOfHorizontalButtons = min(i, 350 / (m_ButtonSize.cx + m_ButtonSpace));

			m_NumberOfVerticalButtons = m_NumberOfButtons / m_NumberOfHorizontalButtons + 
				(m_NumberOfButtons % m_NumberOfHorizontalButtons != 0);
		}
		else
		{
			const int nh = min(10u, GetSystemMetrics(SM_CXSCREEN) / ((m_ButtonSize.cx + m_ButtonSpace) * 2));

			m_NumberOfVerticalButtons = m_NumberOfButtons / nh + (m_NumberOfButtons % nh != 0);
			if (m_NumberOfVerticalButtons < 5) m_NumberOfVerticalButtons = 5;

			m_NumberOfHorizontalButtons = m_NumberOfButtons / m_NumberOfVerticalButtons + 
				(m_NumberOfButtons % m_NumberOfVerticalButtons != 0);
		}
	}
	else
	{
		m_NumberOfHorizontalButtons = m_pSmileyPack->win.x;
		m_NumberOfVerticalButtons = m_NumberOfButtons / m_NumberOfHorizontalButtons + 
			(m_NumberOfButtons % m_NumberOfHorizontalButtons != 0);
	}

	m_BitmapWidth.cx = m_NumberOfHorizontalButtons * (m_ButtonSize.cx + m_ButtonSpace) + m_ButtonSpace; 
	m_BitmapWidth.cy = m_NumberOfVerticalButtons * (m_ButtonSize.cy + m_ButtonSpace) + m_ButtonSpace;

	const int colsz = m_ButtonSize.cy + m_ButtonSpace;
	int wndsz = min((int)m_BitmapWidth.cy, GetSystemMetrics(SM_CYSCREEN) / 2);
	if (opt.IEViewStyle) wndsz = min(wndsz, 250);

	if (m_pSmileyPack->win.x != 0 && m_pSmileyPack->win.y != 0)
		wndsz = min(wndsz, m_pSmileyPack->win.y * (m_ButtonSize.cy + (int)m_ButtonSpace) + (int)m_ButtonSpace);

	m_WindowSizeY = wndsz - (wndsz % colsz) + m_ButtonSpace;
}


void SmileyToolWindowType::CreateSmileyBitmap(HDC hdc)
{
	const RECT rc = { 0, 0, m_BitmapWidth.cx, m_WindowSizeY };

	SetBkColor(hdc, opt.SelWndBkgClr);
	const HBRUSH hBkgBrush = CreateSolidBrush(opt.SelWndBkgClr);
	FillRect(hdc, &rc, hBkgBrush);
	DeleteObject(hBkgBrush);

	if (opt.IEViewStyle)
	{
		HPEN hpen = CreatePen(PS_DOT, 1, 0);
		HGDIOBJ hOldPen = SelectObject(hdc, hpen);

		POINT pts[2] = { {0, 0}, {m_BitmapWidth.cx, 0} };

		for (unsigned i=0; i<=m_NumberOfVerticalButtons; i++)
		{
			pts[0].y = pts[1].y = i * (m_ButtonSize.cy + m_ButtonSpace);
			if (pts[0].y > m_WindowSizeY) break;
			Polyline(hdc, pts, 2);
		}

		pts[0].y = 0; pts[1].y = m_BitmapWidth.cy;
		for (unsigned j=0; j<=m_NumberOfHorizontalButtons; j++)
		{
			pts[0].x = pts[1].x = j * (m_ButtonSize.cx + m_ButtonSpace);
			Polyline(hdc, pts, 2);
		}

		SelectObject(hdc, hOldPen);
		DeleteObject(hpen);
	}
}


RECT SmileyToolWindowType::CalculateButtonToCoordinates(int buttonPosition, int scroll)
{ 
	int row, rowpos;

	if (opt.IEViewStyle)
	{
		row = buttonPosition / m_NumberOfHorizontalButtons;
		rowpos = buttonPosition % m_NumberOfHorizontalButtons;
	}
	else
	{
		row = buttonPosition % m_NumberOfVerticalButtons;
		rowpos = buttonPosition / m_NumberOfVerticalButtons;
	}

	RECT pt;
	pt.left = rowpos * (m_ButtonSize.cx + m_ButtonSpace) + m_ButtonSpace;
	pt.top  = (row - scroll) * (m_ButtonSize.cy + m_ButtonSpace) + m_ButtonSpace;
	pt.right = pt.left + m_ButtonSize.cx;
	pt.bottom = pt.top + m_ButtonSize.cy;

	return pt;
}


int SmileyToolWindowType::CalculateCoordinatesToButton(POINT pt, int scroll)
{
	const int rowpos = (pt.x - m_ButtonSpace) / (m_ButtonSize.cx + m_ButtonSpace);
	const int row = (pt.y - m_ButtonSpace) / (m_ButtonSize.cy + m_ButtonSpace) + scroll;

	int pos;
	if (opt.IEViewStyle)
		pos = m_NumberOfHorizontalButtons * row + rowpos;
	else
		pos = m_NumberOfVerticalButtons * rowpos + row;

	if (pos >= (int)m_NumberOfButtons) pos = -1;

	return pos;
}

void __cdecl SmileyToolThread(void *arg)
{
	SmileyToolWindowParam* stwp = (SmileyToolWindowParam*)arg;
	if (stwp->pSmileyPack && stwp->pSmileyPack->VisibleSmileyCount())
	{
		WNDCLASSEX wndclass;
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = CS_SAVEBITS;
		wndclass.lpfnWndProc   = DlgProcSmileyToolWindow;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 4;
		wndclass.hInstance     = g_hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = CreateSolidBrush(opt.SelWndBkgClr);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = _T("SmileyTool");
		wndclass.hIconSm       = NULL;
		RegisterClassEx(&wndclass);

		CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOPARENTNOTIFY, _T("SmileyTool"), NULL, 
			WS_BORDER | WS_POPUP | WS_VISIBLE, 
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			((SmileyToolWindowParam*)arg)->hWndParent, NULL, g_hInst, arg);

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		UnregisterClass(_T("SmileyTool"), g_hInst);
	}
	delete stwp;
}
