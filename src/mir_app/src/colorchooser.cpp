/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

#include "stdafx.h"

#include "chat.h"

class CColorChooserDlg : public CDlgBase
{
	CCtrlBase m_text;

	GCSessionInfoBase *m_si;
	MODULEINFO *m_pModule;

	int  m_xPosition, m_yPosition;
	int  iCurrentHotTrack, iRows, iColumns;
	HWND m_hwndTarget, m_hwndChooser, hwndPreviousActiveWindow;
	BOOL m_bForeground, bChoosing;

	int CalculateCoordinatesToButton(POINT pt)
	{
		int iSquareRoot = (int)sqrt(static_cast<float>(m_pModule->nColorCount));
		int nCols = iSquareRoot * iSquareRoot < m_pModule->nColorCount ? iSquareRoot + 1 : iSquareRoot;

		int col = pt.x / 25;
		int row = (pt.y - 20) / 20;
		int pos = nCols * row + col;

		if (pt.y < 20 && pos >= m_pModule->nColorCount)
			pos = -1;

		return pos;
	}

	RECT CalculateButtonToCoordinates(int buttonPosition)
	{
		int iSquareRoot = (int)sqrt(static_cast<float>(m_pModule->nColorCount));
		int nCols = iSquareRoot * iSquareRoot < m_pModule->nColorCount ? iSquareRoot + 1 : iSquareRoot;

		int row = buttonPosition / nCols;
		int col = buttonPosition % nCols;

		RECT pt;
		pt.left = col * 25 + 1;
		pt.top = row * 20 + 20;
		pt.right = pt.left + 25 - 1;
		pt.bottom = pt.top + 20;

		return pt;
	}

public:
	CColorChooserDlg(SESSION_INFO *si, BOOL bFG, HWND hwndDlg, HWND hwndTarget, HWND hwndChooser) :
		CDlgBase(g_hInst, IDD_COLORCHOOSER),
		m_text(this, IDC_COLORTEXT),
		iCurrentHotTrack(-2),
		bChoosing(false)
	{
		RECT rc;
		GetWindowRect(hwndChooser, &rc);

		m_text.UseSystemColors();

		m_hwndParent = hwndDlg;
		m_hwndTarget = hwndTarget;
		m_pModule = chatApi.MM_FindModule(si->pszModule);
		m_xPosition = rc.left + 3;
		m_yPosition = IsWindowVisible(hwndChooser) ? rc.top - 1 : rc.top + 20;
		m_bForeground = bFG;
		m_hwndChooser = hwndChooser;
		m_si = si;
	}

	virtual void OnInitDialog() override
	{
		int iSquareRoot = (int)sqrt(static_cast<float>(m_pModule->nColorCount));

		iColumns = iSquareRoot * iSquareRoot == m_pModule->nColorCount ? iSquareRoot : iSquareRoot + 1;
		iRows = iSquareRoot;

		RECT rc;
		rc.top = rc.left = 100;
		rc.right = 100 + iColumns * 25 + 1;
		rc.bottom = iRows * 20 + 100 + 20;

		AdjustWindowRectEx(&rc, GetWindowLongPtr(m_hwnd, GWL_STYLE), FALSE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE));

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		m_yPosition -= height;

		m_text.SetText(m_bForeground ? TranslateT("Text color") : TranslateT("Background color"));
		SetWindowPos(m_text.GetHwnd(), NULL, 0, 0, width, 20, 0);
		SetWindowPos(m_hwnd, NULL, m_xPosition, m_yPosition, width, height, SWP_SHOWWINDOW);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
				if (iCurrentHotTrack >= 0)
					PostMessage(m_hwnd, WM_LBUTTONUP, 0, 0);
				break;
			case IDCANCEL:
				DestroyWindow(m_hwnd);
				break;
			}
			break;

		case WM_LBUTTONUP:
			if (iCurrentHotTrack >= 0 && iCurrentHotTrack < m_pModule->nColorCount && m_hwndTarget != NULL) {
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = 0;
				cf.dwEffects = 0;

				HWND hWindow = GetParent(m_hwndTarget);
				int ctrlId = GetDlgCtrlID(m_hwndChooser);

				if (m_bForeground) {
					m_si->bFGSet = true;
					m_si->iFG = iCurrentHotTrack;
					if (IsDlgButtonChecked(hWindow, ctrlId)) {
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = m_pModule->crColors[iCurrentHotTrack];
						SendMessage(m_hwndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				else {
					m_si->bBGSet = true;
					m_si->iBG = iCurrentHotTrack;
					if (IsDlgButtonChecked(hWindow, ctrlId)) {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = m_pModule->crColors[iCurrentHotTrack];
						SendMessage(m_hwndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
			}
			Close();
			break;

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				Close();
			else if ((wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE))
				hwndPreviousActiveWindow = (HWND)lParam;
			break;

		case WM_MOUSEMOVE:
			{
				HDC hdc = GetDC(m_hwnd);
				POINT pt;
				RECT rect;
				int but;

				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);

				if (iCurrentHotTrack == -2)
					return 0; // prevent focussing when not drawn yet!

				but = CalculateCoordinatesToButton(pt);

				// weird stuff
				if (but != iCurrentHotTrack) {
					if (iCurrentHotTrack >= 0) {
						rect = CalculateButtonToCoordinates(iCurrentHotTrack);
						DrawFocusRect(hdc, &rect);
						iCurrentHotTrack = -1;
					}
					iCurrentHotTrack = but;

					if (iCurrentHotTrack >= 0) {
						rect = CalculateButtonToCoordinates(iCurrentHotTrack);
						DrawFocusRect(hdc, &rect);
					}
				}
				ReleaseDC(m_hwnd, hdc);
			}
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				int iThisRow = 1;
				int iThisColumn = 0;

				RECT rc;
				GetClientRect(m_hwnd, &rc);
				rc.top += 20;

				HDC hdc = BeginPaint(m_hwnd, &ps);

				// fill background
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));

				for (int i = 0; i < m_pModule->nColorCount; i++) {
					// decide place to draw the color block in the window
					iThisColumn++;
					if (iThisColumn > iColumns) {
						iThisColumn = 1;
						iThisRow++;
					}

					if (m_bForeground && m_si->bFGSet && m_si->iFG == i || !m_bForeground && m_si->bBGSet && m_si->iBG == i) {
						rc.top = (iThisRow - 1) * 20 + 1 + 20;
						rc.left = (iThisColumn - 1) * 25 + 1 + 1;
						rc.bottom = iThisRow * 20 - 1 + 20;
						rc.right = iThisColumn * 25 - 1;

						DrawEdge(hdc, &rc, EDGE_RAISED, BF_TOP | BF_LEFT | BF_RIGHT | BF_BOTTOM);
					}

					rc.top = (iThisRow - 1) * 20 + 3 + 20;
					rc.left = (iThisColumn - 1) * 25 + 3 + 1;
					rc.bottom = iThisRow * 20 - 3 + 20;
					rc.right = iThisColumn * 25 - 3;

					FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

					HBRUSH hbr = CreateSolidBrush(m_pModule->crColors[i]);

					rc.top = (iThisRow - 1) * 20 + 4 + 20;
					rc.left = (iThisColumn - 1) * 25 + 4 + 1;
					rc.bottom = iThisRow * 20 - 4 + 20;
					rc.right = iThisColumn * 25 - 4;

					FillRect(hdc, &rc, hbr);
					DeleteObject(hbr);
				}

				EndPaint(m_hwnd, &ps);
				iCurrentHotTrack = -1;
			}
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	virtual void OnClose() override
	{
		SetFocus(m_hwndTarget);
	}
};

void ColorChooser(SESSION_INFO *si, BOOL bFG, HWND hwndDlg, HWND hwndTarget, HWND hwndChooser)
{
	CColorChooserDlg *pDialog = new CColorChooserDlg(si, bFG, hwndDlg, hwndTarget, hwndChooser);
	pDialog->Show();
}
