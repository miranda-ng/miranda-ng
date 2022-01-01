/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

static COLORREF colorTable[] =
{
	RGB(0,0,0), RGB(0,0,128), RGB(0,128,128), RGB(128,0,128),
	RGB(0,128,0), RGB(128,128,0), RGB(128,0,0), RGB(128,128,128),
	RGB(192,192,192), RGB(0,0,255), RGB(0,255,255), RGB(255,0,255), 
	RGB(0,255,0), RGB(255,255,0), RGB(255,0,0), RGB(255,255,255)
};

MIR_APP_DLL(COLORREF*) Srmm_GetColorTable(int *pSize)
{
	if (pSize != nullptr)
		*pSize = _countof(colorTable);
	return colorTable;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CColorChooserDlg : public CDlgBase
{
	CCtrlBase m_text;

	CSrmmBaseDialog *m_pDlg;

	int  m_xPosition, m_yPosition;
	int  iCurrentHotTrack, iRows, iColumns;
	HWND m_hwndTarget, m_hwndChooser;
	BOOL m_bForeground, bChoosing;

	int CalculateCoordinatesToButton(POINT pt)
	{
		int iSquareRoot = (int)sqrt(static_cast<float>(16));
		int nCols = iSquareRoot * iSquareRoot < 16 ? iSquareRoot + 1 : iSquareRoot;

		int col = pt.x / 25;
		int row = (pt.y - 20) / 20;
		int pos = nCols * row + col;

		if (pt.y < 20 && pos >= 16)
			pos = -1;

		return pos;
	}

	RECT CalculateButtonToCoordinates(int buttonPosition)
	{
		int iSquareRoot = (int)sqrt(static_cast<float>(16));
		int nCols = iSquareRoot * iSquareRoot < 16 ? iSquareRoot + 1 : iSquareRoot;

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
	CColorChooserDlg(CSrmmBaseDialog *pDlg, BOOL bFG, HWND hwndDlg, HWND hwndTarget, HWND hwndChooser) :
		CDlgBase(g_plugin, IDD_COLORCHOOSER),
		m_text(this, IDC_COLORTEXT),
		m_pDlg(pDlg),
		m_hwndTarget(hwndTarget),
		m_hwndChooser(hwndChooser),
		m_bForeground(bFG),
		iCurrentHotTrack(-2),
		bChoosing(false)
	{
		RECT rc;
		GetWindowRect(hwndChooser, &rc);

		m_text.UseSystemColors();

		m_hwndParent = hwndDlg;
		m_xPosition = rc.left + 3;
		m_yPosition = IsWindowVisible(hwndChooser) ? rc.top - 1 : rc.top + 20;

		int iSquareRoot = (int)sqrt(static_cast<float>(16));
		iColumns = iSquareRoot * iSquareRoot == 16 ? iSquareRoot : iSquareRoot + 1;
		iRows = iSquareRoot;
	}

	bool OnInitDialog() override
	{
		RECT rc;
		rc.top = rc.left = 100;
		rc.right = 100 + iColumns * 25 + 1;
		rc.bottom = iRows * 20 + 100 + 20;

		AdjustWindowRectEx(&rc, GetWindowLongPtr(m_hwnd, GWL_STYLE), FALSE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE));

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		m_yPosition -= height;

		m_text.SetText(m_bForeground ? TranslateT("Text color") : TranslateT("Background color"));
		SetWindowPos(m_text.GetHwnd(), nullptr, 0, 0, width, 20, 0);
		SetWindowPos(m_hwnd, nullptr, m_xPosition, m_yPosition, width, height, SWP_SHOWWINDOW);
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
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
			if (iCurrentHotTrack >= 0 && iCurrentHotTrack < 16 && m_hwndTarget != nullptr) {
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = 0;
				cf.dwEffects = 0;

				HWND hWindow = GetParent(m_hwndTarget);
				int ctrlId = GetDlgCtrlID(m_hwndChooser);

				if (m_bForeground) {
					m_pDlg->m_bFGSet = true;
					m_pDlg->m_iFG = colorTable[iCurrentHotTrack];
					if (IsDlgButtonChecked(hWindow, ctrlId)) {
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = colorTable[iCurrentHotTrack];
						SendMessage(m_hwndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				else {
					m_pDlg->m_bBGSet = true;
					m_pDlg->m_iBG = colorTable[iCurrentHotTrack];
					if (IsDlgButtonChecked(hWindow, ctrlId)) {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = colorTable[iCurrentHotTrack];
						SendMessage(m_hwndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
			}
			Close();
			break;

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				Close();
			break;

		case WM_MOUSEMOVE:
			if (iCurrentHotTrack == -2)
				return 0; // prevent focussing when not drawn yet!
			{
				HDC hdc = GetDC(m_hwnd);

				// weird stuff
				POINT pt;
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				int but = CalculateCoordinatesToButton(pt);
				if (but != iCurrentHotTrack) {
					if (iCurrentHotTrack >= 0) {
						RECT rect = CalculateButtonToCoordinates(iCurrentHotTrack);
						DrawFocusRect(hdc, &rect);
						iCurrentHotTrack = -1;
					}
					iCurrentHotTrack = but;

					if (iCurrentHotTrack >= 0) {
						RECT rect = CalculateButtonToCoordinates(iCurrentHotTrack);
						DrawFocusRect(hdc, &rect);
					}
				}
				ReleaseDC(m_hwnd, hdc);
			}
			break;

		case WM_PAINT:
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			rc.top += 20;
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(m_hwnd, &ps);

				// fill background
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));

				int iThisRow = 1;
				int iThisColumn = 0;
				for (unsigned int i = 0; i < 16; i++) {
					// decide place to draw the color block in the window
					iThisColumn++;
					if (iThisColumn > iColumns) {
						iThisColumn = 1;
						iThisRow++;
					}

					if (m_bForeground && m_pDlg->m_bFGSet && m_pDlg->m_iFG == i || !m_bForeground && m_pDlg->m_bBGSet && m_pDlg->m_iBG == i) {
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

					rc.top = (iThisRow - 1) * 20 + 4 + 20;
					rc.left = (iThisColumn - 1) * 25 + 4 + 1;
					rc.bottom = iThisRow * 20 - 4 + 20;
					rc.right = iThisColumn * 25 - 4;

					HBRUSH hbr = CreateSolidBrush(colorTable[i]);
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

	void OnDestroy() override
	{
		SetFocus(m_hwndTarget);
	}
};

void CSrmmBaseDialog::ShowColorChooser(int iCtrlId)
{
	CColorChooserDlg *pDialog = new CColorChooserDlg(this, iCtrlId == IDC_SRMM_COLOR, m_hwnd, m_message.GetHwnd(), GetDlgItem(m_hwnd, iCtrlId));
	pDialog->Show();
}
